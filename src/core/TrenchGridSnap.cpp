#include "TrenchGridSnap.h"
#include "CanvasGridMath.h"
#include "TrenchLShape.h"
#include "TrenchTerrain.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <memory>
#include <type_traits>
#include <vector>

#include <QLocale>

#include <ogr_api.h>
#include <ogr_geometry.h>

namespace TrenchGridSnap {
namespace {

using TrenchGridGenerator::Cell;
using TrenchGridGenerator::RatioFill;

constexpr size_t kMaxCells = 200000;
constexpr double kExtraPitch = 40.0;  // trenches at most this much (m) farther apart than one arm
constexpr int kMaxCellsApart = 60;
constexpr int kMaxShifted = 8;  // rows are shifted only for repeats of up to this many cells

struct GeometryDeleter {
  void operator()(OGRGeometry* geometry) const { OGRGeometryFactory::destroyGeometry(geometry); }
};
using Geometry = std::unique_ptr<OGRGeometry, GeometryDeleter>;
struct PreparedDeleter {
  void operator()(OGRPreparedGeometryH prepared) const { OGRDestroyPreparedGeometry(prepared); }
};
using Prepared = std::unique_ptr<std::remove_pointer_t<OGRPreparedGeometryH>, PreparedDeleter>;

// A trench every `a` cells along a row, rows `b` cells apart, each row shifted `s` cells more.
struct Lattice {
  int a = 1, b = 1, s = 0;
};

OGRPolygon polygonOf(const Cell& cell) {
  OGRLinearRing ring;
  for (const auto& point : cell.ring) ring.addPoint(point.first, point.second);
  OGRPolygon polygon;
  polygon.addRing(&ring);
  return polygon;
}

// Lays one L with its outer corner on grid node (cu, cv).
struct Placer {
  Grid grid;
  CanvasGridMath::Frame frame;
  double width = 2.0, arm = 15.0;
  int bend = 0;
  bool east = false, north = false;  // where the bend puts the outer corner in the L's square

  Placer(const Grid& g, double w, double a, int b)
      : grid(g), frame{g.originX, g.originY, g.rotationDeg}, width(w), arm(a), bend(b), east(b == 1 || b == 2),
        north(b == 1 || b == 3) {}

  Cell at(long long cu, long long cv) const {
    TrenchGridGenerator::Spec spec;
    CanvasGridMath::toMap(frame, double(cu) * grid.step - (east ? arm : 0.0), double(cv) * grid.step - (north ? arm : 0.0),
                          &spec.originX, &spec.originY);
    spec.azimuthDeg = grid.rotationDeg;
    spec.trenchWidth = width;
    spec.armLength = arm;
    spec.armBend = bend;
    const std::vector<Cell> one = TrenchLShape::build(spec);
    return one.empty() ? Cell{} : one.front();
  }
};

// Closest approach of a trench to its neighbours in the lattice; stops early below kMinBalk.
double clearance(const Placer& placer, const Lattice& lattice) {
  const OGRPolygon base = polygonOf(placer.at(0, 0));
  double closest = std::numeric_limits<double>::infinity();
  for (int m = -3; m <= 3; ++m) {
    for (int n = -3; n <= 3; ++n) {
      const long long du = static_cast<long long>(n) * lattice.a + static_cast<long long>(m) * lattice.s;
      const long long dv = static_cast<long long>(m) * lattice.b;
      if ((du == 0 && dv == 0) || std::abs(double(du)) * placer.grid.step >= placer.arm + kMinBalk ||
          std::abs(double(dv)) * placer.grid.step >= placer.arm + kMinBalk)
        continue;  // the same trench, or squares already kMinBalk apart
      const OGRPolygon other = polygonOf(placer.at(du, dv));
      closest = std::min(closest, base.Distance(&other));
      if (closest < kMinBalk - 1e-9) return closest;
    }
  }
  return closest;
}

// The area prepared once, and the grid nodes an outer corner can take with its square inside the
// area's envelope.
struct Nodes {
  Prepared prepared;
  long long cuMin = 0, cuMax = -1, cvMin = 0, cvMax = -1;
};

Nodes nodesFor(const OGRGeometry* area, const Placer& placer) {
  Nodes nodes;
  nodes.prepared.reset(OGRCreatePreparedGeometry(OGRGeometry::ToHandle(const_cast<OGRGeometry*>(area))));
  OGREnvelope env;
  area->getEnvelope(&env);
  double uMin = std::numeric_limits<double>::infinity(), uMax = -uMin, vMin = uMin, vMax = -uMin;
  for (double x : {env.MinX, env.MaxX}) {
    for (double y : {env.MinY, env.MaxY}) {
      double u = 0.0, v = 0.0;
      CanvasGridMath::toGrid(placer.frame, x, y, &u, &v);
      uMin = std::min(uMin, u), uMax = std::max(uMax, u), vMin = std::min(vMin, v), vMax = std::max(vMax, v);
    }
  }
  const double step = placer.grid.step, du = placer.east ? placer.arm : 0.0, dv = placer.north ? placer.arm : 0.0;
  nodes.cuMin = static_cast<long long>(std::ceil((uMin + du) / step));
  nodes.cuMax = static_cast<long long>(std::floor((uMax - placer.arm + du) / step));
  nodes.cvMin = static_cast<long long>(std::ceil((vMin + dv) / step));
  nodes.cvMax = static_cast<long long>(std::floor((vMax - placer.arm + dv) / step));
  return nodes;
}

// Whole Ls inside the area on this lattice, anchored at the first node; empty with *tooMany when
// the lattice is too fine for the area.
std::vector<Cell> fill(const Placer& placer, const Nodes& nodes, const Lattice& lattice, bool* tooMany) {
  std::vector<Cell> out;
  const double rows = double(nodes.cvMax - nodes.cvMin) / lattice.b + 1.0;
  const double perRow = double(nodes.cuMax - nodes.cuMin) / lattice.a + 1.0;
  *tooMany = rows * perRow > double(kMaxCells);
  if (*tooMany || rows < 1.0 || perRow < 1.0) return out;
  for (long long m = 0; nodes.cvMin + m * lattice.b <= nodes.cvMax; ++m) {
    const long long shift = (m * lattice.s) % lattice.a;
    for (long long cu = nodes.cuMin + shift; cu <= nodes.cuMax; cu += lattice.a) {
      Cell cell = placer.at(cu, nodes.cvMin + m * lattice.b);
      if (cell.ring.empty()) continue;
      OGRPolygon polygon = polygonOf(cell);
      if (OGRPreparedGeometryContains(nodes.prepared.get(), OGRGeometry::ToHandle(&polygon)))
        out.push_back(std::move(cell));
    }
  }
  return out;
}

}  // namespace

RatioFill buildForTargetRatio(const QByteArray& areaWkb, double targetPct, double width, double armLength, int armBend,
                              const Grid& grid) {
  RatioFill best;
  const Placer placer(grid, width, armLength, armBend);
  if (!std::isfinite(grid.step) || !(grid.step > 0.0) || !std::isfinite(grid.originX) ||
      !std::isfinite(grid.originY) || !std::isfinite(grid.rotationDeg)) {
    best.error = QStringLiteral("좌표격자를 읽지 못했습니다. 미터 좌표격자를 켠 뒤 다시 적용하세요.");
    return best;
  }
  if (placer.at(0, 0).ring.empty() || !std::isfinite(targetPct) || targetPct <= 0.0 || targetPct > 100.0) {
    best.error = QStringLiteral("폭은 0 초과 2 m 이하, 팔 길이는 폭보다 길고 20 m 이하, 목표 비율은 0 초과 100% 이하의 "
                                "유효한 값이어야 합니다.");
    return best;
  }
  if (const QString problem = TrenchGridGenerator::areaProblem(areaWkb); !problem.isEmpty()) {
    best.error = problem;
    return best;
  }
  OGRGeometry* raw = nullptr;
  OGRGeometryFactory::createFromWkb(areaWkb.constData(), nullptr, &raw, static_cast<size_t>(areaWkb.size()));
  const Geometry area(raw);
  const double areaM2 = OGR_G_Area(OGRGeometry::ToHandle(area.get()));
  const double targetArea = areaM2 * (targetPct / 100.0);
  if (!std::isfinite(areaM2) || !(areaM2 > 1e-6)) {
    best.error = QStringLiteral("조사구역 면적과 목표 면적을 계산하지 못했습니다.");
    return best;
  }
  best.areaM2 = areaM2;
  const Nodes nodes = nodesFor(area.get(), placer);
  const double oneL = width * (2.0 * armLength - width);
  const int maxApart = std::clamp(int(std::ceil((armLength + kExtraPitch) / grid.step)), 1, kMaxCellsApart);

  // Fewest whole Ls reaching the target; of the same count the widest lattice, so they spread.
  // Wider rows only remove Ls: once a row spacing falls short, wider ones are not tried.
  double bestArea = std::numeric_limits<double>::infinity(), bestSpread = 0.0, mostBelow = 0.0;
  for (int a = 1; a <= maxApart; ++a) {
    for (int s = 0; s < (a <= kMaxShifted ? a : 1); ++s) {
      for (int b = 1; b <= maxApart; ++b) {
        const Lattice lattice{a, b, s};
        const double gap = clearance(placer, lattice);
        if (gap < kMinBalk - 1e-9) continue;
        bool tooMany = false;
        std::vector<Cell> cells = fill(placer, nodes, lattice, &tooMany);
        if (tooMany) continue;
        const double candidateArea = double(cells.size()) * oneL;
        if (candidateArea < targetArea) {
          mostBelow = std::max(mostBelow, candidateArea);
          break;
        }
        const bool fewer = candidateArea < bestArea - oneL * 0.5;
        if (fewer || (candidateArea < bestArea + oneL * 0.5 && double(a) * b > bestSpread)) {
          bestArea = candidateArea;
          bestSpread = double(a) * b;
          best.cells = std::move(cells);
          best.balk = gap;
        }
      }
    }
  }
  if (best.cells.empty()) {
    best.error = QStringLiteral("좌표격자 %1 m 칸에 맞추면 트렌치끼리 %2 m 이상 띄워 놓을 수 있는 것이 최대 %3%라 목표 "
                                "%4%를 채울 수 없습니다. 좌표격자 간격을 줄이거나(예: 5 m) 「좌표격자에 맞추기」를 "
                                "끄세요. 기존 격자는 유지됩니다.")
                     .arg(QLocale().toString(grid.step, 'g', 6), QLocale().toString(kMinBalk, 'g', 3),
                          QLocale().toString(mostBelow / areaM2 * 100.0, 'f', 1), QLocale().toString(targetPct, 'g', 4));
    return best;
  }
  const QString prefix = targetPct <= 3.5 ? QStringLiteral("Sp-") : QStringLiteral("Tr-");
  TrenchTerrain::numberFromHighSide(best.cells, grid.rotationDeg, prefix);
  // A lattice moves in whole rows and columns; Ls it holds beyond the target are left out, spread
  // over the numbering, so the plan is the fewest Ls that still reach it.
  const auto surplus = static_cast<size_t>(std::floor((bestArea - targetArea) / oneL + 1e-9));
  if (surplus > 0 && surplus < best.cells.size()) {
    std::vector<Cell> kept;
    const size_t count = best.cells.size();
    size_t next = 0;
    for (size_t i = 0; i < count; ++i) {
      if (next < surplus && i == (2 * next + 1) * count / (2 * surplus)) {
        ++next;
        continue;
      }
      kept.push_back(std::move(best.cells[i]));
    }
    best.cells = std::move(kept);
    TrenchTerrain::numberFromHighSide(best.cells, grid.rotationDeg, prefix);
  }
  best.arm = armLength;
  best.length = best.cells.front().length;
  best.azimuthDeg = grid.rotationDeg;
  best.ratioPct = TrenchGridGenerator::totalArea(best.cells) / areaM2 * 100.0;
  return best;
}

}  // namespace TrenchGridSnap
