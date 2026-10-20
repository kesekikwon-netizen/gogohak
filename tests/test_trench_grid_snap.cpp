// 「좌표격자에 맞추기」 (사용자 요청 2026-10-20): ㄱ·ㄴ자 trenches laid on the map's coordinate grid. The grid
// turns the trenches, every outer corner sits on a grid node, trenches step by whole grid cells and never come
// closer than 1 m, and the plan is the fewest whole Ls reaching the target. A grid too coarse for that says so.
#include <cmath>
#include <memory>

#include <QtTest>
#include <QCoreApplication>

#include "core/CanvasGridMath.h"
#include "core/TrenchGridGenerator.h"
#include "core/TrenchGridSnap.h"
#include "core/TrenchPlanCache.h"

#include <ogr_api.h>
#include <ogr_geometry.h>

namespace {
struct GeometryDeleter {
  void operator()(OGRGeometry* geometry) const { OGRGeometryFactory::destroyGeometry(geometry); }
};
using Geometry = std::unique_ptr<OGRGeometry, GeometryDeleter>;

QByteArray squareWkb(double x0, double y0, double x1, double y1) {
  OGRLinearRing ring;
  ring.addPoint(x0, y0);
  ring.addPoint(x1, y0);
  ring.addPoint(x1, y1);
  ring.addPoint(x0, y1);
  ring.addPoint(x0, y0);
  OGRPolygon polygon;
  polygon.addRing(&ring);
  QByteArray wkb(int(polygon.WkbSize()), '\0');
  polygon.exportToWkb(reinterpret_cast<unsigned char*>(wkb.data()));
  return wkb;
}

OGRPolygon cellPolygon(const TrenchGridGenerator::Cell& cell) {
  OGRLinearRing ring;
  for (const auto& point : cell.ring) ring.addPoint(point.first, point.second);
  OGRPolygon polygon;
  polygon.addRing(&ring);
  return polygon;
}

bool onNode(const TrenchGridSnap::Grid& grid, double x, double y) {
  double u = 0.0, v = 0.0;
  CanvasGridMath::toGrid(CanvasGridMath::Frame{grid.originX, grid.originY, grid.rotationDeg}, x, y, &u, &v);
  return std::abs(u / grid.step - std::round(u / grid.step)) < 1e-6 &&
         std::abs(v / grid.step - std::round(v / grid.step)) < 1e-6;
}

// Whole, inside, at least 1 m apart, outer corners on nodes, the fewest reaching the target.
void checkPlan(const TrenchGridGenerator::RatioFill& plan, const QByteArray& areaWkb, const TrenchGridSnap::Grid& grid,
               double targetPct) {
  QVERIFY2(!plan.cells.empty(), qPrintable(plan.error));
  QVERIFY(TrenchGridGenerator::cellsValid(plan.cells));
  QVERIFY(TrenchGridGenerator::meetsTarget(plan.cells, plan.areaM2, targetPct));
  OGRGeometry* raw = nullptr;
  OGRGeometryFactory::createFromWkb(areaWkb.constData(), nullptr, &raw, size_t(areaWkb.size()));
  Geometry area(raw);
  std::vector<OGRPolygon> polygons;
  for (const auto& cell : plan.cells) {
    QCOMPARE(cell.arm, 15.0);
    QVERIFY2(onNode(grid, cell.ring[0].first, cell.ring[0].second),
             qPrintable(QStringLiteral("%1 corner %2, %3").arg(cell.name).arg(cell.ring[0].first, 0, 'f', 3)
                            .arg(cell.ring[0].second, 0, 'f', 3)));
    polygons.push_back(cellPolygon(cell));
    QVERIFY(area->Contains(&polygons.back()));
  }
  for (size_t i = 0; i < polygons.size(); ++i)
    for (size_t j = i + 1; j < polygons.size(); ++j)
      QVERIFY2(polygons[i].Distance(&polygons[j]) >= 1.0 - 1e-9,
               qPrintable(QStringLiteral("%1 and %2 touch").arg(plan.cells[i].name, plan.cells[j].name)));
  QVERIFY(plan.balk >= 1.0 - 1e-9);
}
}  // namespace

class TestTrenchGridSnap : public QObject {
  Q_OBJECT
private slots:
  void cornersSitOnGridNodes_data();
  void cornersSitOnGridNodes();
  void turnedGridTurnsTheTrenches();
  void coarseGridSaysWhyTenPercentDoesNotFit();
  void planCacheKeepsGridPlansApart();
};

void TestTrenchGridSnap::cornersSitOnGridNodes_data() {
  QTest::addColumn<double>("step");
  QTest::addColumn<int>("bend");
  QTest::newRow("5 m ㄴ") << 5.0 << 0;
  QTest::newRow("10 m ㄱ") << 10.0 << 1;  // the arm is not a whole number of cells: the outer corner still is on a node
  QTest::newRow("1 m ㄴ 좌우 반대") << 1.0 << 2;
}

void TestTrenchGridSnap::cornersSitOnGridNodes() {
  QFETCH(double, step);
  QFETCH(int, bend);
  const QByteArray area = squareWkb(219700.3, 450000.7, 219900.3, 450200.7);  // not on any node
  const TrenchGridSnap::Grid grid{0.0, 0.0, 0.0, step};
  const auto plan = TrenchGridSnap::buildForTargetRatio(area, 10.0, 2.0, 15.0, bend, grid);
  checkPlan(plan, area, grid, 10.0);
  QVERIFY(plan.ratioPct >= 10.0 - 1e-9);
  QCOMPARE(plan.azimuthDeg, 0.0);
}

void TestTrenchGridSnap::turnedGridTurnsTheTrenches() {
  const QByteArray area = squareWkb(200000, 450000, 200200, 450200);
  const TrenchGridSnap::Grid grid{200003.0, 450007.0, 30.0, 5.0};
  const auto plan = TrenchGridSnap::buildForTargetRatio(area, 10.0, 2.0, 15.0, 0, grid);
  checkPlan(plan, area, grid, 10.0);
  QCOMPARE(plan.azimuthDeg, 30.0);
  // ㄴ: ring[0] → ring[1] runs along the grid's turned east axis.
  const auto& r = plan.cells.front().ring;
  const double turn = std::atan2(-(r[1].second - r[0].second), r[1].first - r[0].first) * 180.0 / 3.14159265358979323846;
  QVERIFY2(std::abs(turn - 30.0) < 1e-6, qPrintable(QString::number(turn)));
}

// A 15 m grid holds one 15 m L per cell; Ls that never touch fill at most every third cell (under 8.3%).
void TestTrenchGridSnap::coarseGridSaysWhyTenPercentDoesNotFit() {
  const QByteArray area = squareWkb(219700, 450000, 219910, 450210);
  const TrenchGridSnap::Grid grid{0.0, 0.0, 0.0, 15.0};
  const auto tooMuch = TrenchGridSnap::buildForTargetRatio(area, 10.0, 2.0, 15.0, 0, grid);
  QVERIFY(tooMuch.cells.empty());
  QVERIFY2(tooMuch.error.contains(QStringLiteral("15 m")) && tooMuch.error.contains(QStringLiteral("5 m")),
           qPrintable(tooMuch.error));
  const auto seven = TrenchGridSnap::buildForTargetRatio(area, 7.0, 2.0, 15.0, 0, grid);
  checkPlan(seven, area, grid, 7.0);
}

void TestTrenchGridSnap::planCacheKeepsGridPlansApart() {
  TrenchPlanCache::clear();
  const QByteArray area = squareWkb(200000, 450000, 200200, 450200);
  const TrenchGridSnap::Grid five{0.0, 0.0, 0.0, 5.0};
  const TrenchGridSnap::Grid ten{0.0, 0.0, 0.0, 10.0};
  const auto free = TrenchPlanCache::ratioPlan(area, 10.0, 2.0, 0.0, 15.0, 0);
  const auto onFive = TrenchPlanCache::ratioPlan(area, 10.0, 2.0, 0.0, 15.0, 0, &five);
  const auto onTen = TrenchPlanCache::ratioPlan(area, 10.0, 2.0, 0.0, 15.0, 0, &ten);
  QCOMPARE(TrenchPlanCache::computeCount(), 3);
  QVERIFY(!onFive.cells.empty() && !onTen.cells.empty());
  const auto direct = TrenchGridSnap::buildForTargetRatio(area, 10.0, 2.0, 15.0, 0, ten);
  QVERIFY(onTen.cells.front().ring == direct.cells.front().ring);
  TrenchGridGenerator::RatioFill known;
  QVERIFY(TrenchPlanCache::lookup(area, 10.0, 2.0, 0.0, &known, 15.0, 0, &five));
  QVERIFY(known.cells.front().ring == onFive.cells.front().ring);
  QVERIFY(free.cells.size() > 0);
}

int main(int argc, char** argv) {
  QCoreApplication app(argc, argv);
  TestTrenchGridSnap tc;
  return QTest::qExec(&tc, argc, argv);
}

#include "test_trench_grid_snap.moc"
