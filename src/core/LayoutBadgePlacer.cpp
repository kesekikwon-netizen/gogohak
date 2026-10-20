#include "LayoutBadgePlacer.h"

#include <QLineF>
#include <QList>
#include <QRectF>

#include <qgsgeometryengine.h>
#include <qgslayout.h>
#include <qgslayoutitemlegend.h>
#include <qgslayoutitemmap.h>

#include <algorithm>
#include <cmath>
#include <memory>
#include <numeric>
#include <vector>

namespace {
constexpr double kPi = 3.14159265358979323846;
// Centres closer than this share of a step cover each other.
constexpr double kClearShare = 0.96;
// Keep the circle outline (0.15 mm) and a hair of paper inside the frame.
constexpr double kEdgeMarginMm = 0.3;
}  // namespace

LayoutBadgePlacer::LayoutBadgePlacer(const QTransform& mapToScene, const QPainterPath& allowedScene,
                                     double stepMapUnits)
    : m_toScene(mapToScene),
      m_allowed(allowedScene),
      m_limited(!allowedScene.isEmpty()),
      m_step(std::isfinite(stepMapUnits) && stepMapUnits > 0.0 ? stepMapUnits : 1.0) {
  const double stepMm = QLineF(m_toScene.map(QPointF(0.0, 0.0)), m_toScene.map(QPointF(m_step, 0.0))).length();
  if (stepMm > 1e-9) m_unitsPerMm = m_step / stepMm;
  if (!m_limited) return;
  const QRectF box = m_allowed.boundingRect();
  const double reachMm = std::hypot(box.width(), box.height());
  if (stepMm > 1e-9 && std::isfinite(reachMm))
    m_reachRings = std::clamp(static_cast<int>(std::ceil(reachMm / stepMm)) + 1, 1, kMaxRings);
}

int LayoutBadgePlacer::slotsOnRing(int ring) {
  const double half = std::min(0.999, 1.0 / (2.0 * double(std::max(1, ring))));
  const double count = kPi / std::asin(half);
  return std::max(4, static_cast<int>(std::floor(count + 1e-6)));
}

quint64 LayoutBadgePlacer::cellKey(qint64 cx, qint64 cy) const {
  return (quint64(quint32(qint32(cx))) << 32) | quint64(quint32(qint32(cy)));
}

QRectF LayoutBadgePlacer::boxAt(const QgsPointXY& point) const {
  const QSizeF box = m_boxesMm.value(m_current) * m_unitsPerMm;
  return QRectF(point.x() - box.width() * 0.5, point.y() - box.height() * 0.5, box.width(), box.height());
}

bool LayoutBadgePlacer::isFree(const QgsPointXY& point) const {
  if (!m_boxesMm.isEmpty()) {
    const QRectF box = boxAt(point);
    for (const QRectF& other : m_placedBoxes)
      if (box.intersects(other)) return false;
    return true;
  }
  const qint64 cx = qint64(std::floor(point.x() / m_step));
  const qint64 cy = qint64(std::floor(point.y() / m_step));
  const double clear = m_step * kClearShare;
  for (qint64 dx = -1; dx <= 1; ++dx) {
    for (qint64 dy = -1; dy <= 1; ++dy) {
      const auto cell = m_cells.constFind(cellKey(cx + dx, cy + dy));
      if (cell == m_cells.cend()) continue;
      for (const QgsPointXY& other : *cell)
        if (point.distance(other) < clear) return false;
    }
  }
  return true;
}

bool LayoutBadgePlacer::fits(const QgsPointXY& point, double badgeDiameterMm) const {
  if (!m_limited) return true;
  const QPointF centre = m_toScene.map(QPointF(point.x(), point.y()));
  if (!m_boxesMm.isEmpty()) {
    const QSizeF box = m_boxesMm.value(m_current);
    const double w = box.width() * 0.5 + kEdgeMarginMm, h = box.height() * 0.5 + kEdgeMarginMm;
    return m_allowed.contains(QRectF(centre.x() - w, centre.y() - h, 2.0 * w, 2.0 * h));
  }
  const double r = std::max(0.5, badgeDiameterMm * 0.5) + kEdgeMarginMm;
  return m_allowed.contains(QRectF(centre.x() - r, centre.y() - r, 2.0 * r, 2.0 * r));
}

void LayoutBadgePlacer::occupy(const QgsPointXY& point) {
  const qint64 cx = qint64(std::floor(point.x() / m_step));
  const qint64 cy = qint64(std::floor(point.y() / m_step));
  m_cells[cellKey(cx, cy)].append(point);
  if (!m_boxesMm.isEmpty()) m_placedBoxes.append(boxAt(point));
  ++m_count;
}

QgsPointXY LayoutBadgePlacer::place(const QgsPointXY& origin, double badgeDiameterMm) {
  return place(origin, badgeDiameterMm, nullptr, QgsRectangle());
}

QgsPointXY LayoutBadgePlacer::place(const QgsPointXY& origin, double badgeDiameterMm, const QgsGeometryEngine* site,
                                    const QgsRectangle& siteBox) {
  QgsPointXY chosen = origin;
  bool found = false;
  bool haveFree = false;
  QgsPointXY firstFree;
  // With a site area, the nearest free slot off the site is only kept until a
  // free slot inside the site turns up.
  bool haveOutside = false;
  QgsPointXY nearestOutside;
  auto inside = [&](const QgsPointXY& candidate) {
    if (!siteBox.contains(candidate)) return false;
    const QgsPoint point(candidate);
    return site->intersects(&point);
  };
  auto consider = [&](const QgsPointXY& candidate) {
    if (!isFree(candidate)) return false;
    if (fits(candidate, badgeDiameterMm)) {
      if (!site || inside(candidate)) {
        chosen = candidate;
        return true;
      }
      if (!haveOutside) {
        haveOutside = true;
        nearestOutside = candidate;
      }
      return false;
    }
    if (!haveFree) {
      haveFree = true;
      firstFree = candidate;
    }
    return false;
  };
  found = consider(origin);
  // Each placed badge blocks at most a few spots of any ring, while ring n
  // holds about 2*pi*n spots, so this many rings always has a free slot. On a
  // limited paper the search also reaches across it, so a badge whose site is
  // under a legend card wider than the base rings still gets clear of it.
  int rings = std::min(kMaxRings, std::max(kBaseRings + m_count, m_reachRings));
  // Distance to the far corner of the site area.
  const double siteReach = site ? std::hypot(std::max(std::abs(siteBox.xMinimum() - origin.x()),
                                                      std::abs(siteBox.xMaximum() - origin.x())),
                                             std::max(std::abs(siteBox.yMinimum() - origin.y()),
                                                      std::abs(siteBox.yMaximum() - origin.y())))
                                : 0.0;
  if (site) rings = std::min(kMaxRings, std::max(rings, static_cast<int>(std::ceil(siteReach / m_step))));
  for (int ring = 1; !found && ring <= rings; ++ring) {
    // Off-site slots are only worth taking near the site; stop once the rings
    // pass the site area with an off-site slot in hand.
    if (site && haveOutside && m_step * ring > siteReach) break;
    const int count = slotsOnRing(ring);
    const double radius = m_step * ring;
    for (int slot = 0; slot < count && !found; ++slot) {
      const double angle = slot * (2.0 * kPi / double(count));
      found = consider(QgsPointXY(origin.x() + std::cos(angle) * radius,
                                  origin.y() + std::sin(angle) * radius));
    }
  }
  if (!found && haveOutside) {
    chosen = nearestOutside;
    found = true;
  }
  // No slot is both clear and fully on the paper: keep the circles apart
  // (overlap is never allowed) at the nearest clear slot.
  if (!found)
    chosen = haveFree ? firstFree : QgsPointXY(origin.x(), origin.y() + m_step * (rings + 1));
  occupy(chosen);
  return chosen;
}

QVector<QgsPointXY> LayoutBadgePlacer::placeAll(const QVector<QgsPointXY>& origins,
                                                const QVector<double>& badgeDiametersMm,
                                                const QVector<QgsGeometry>& siteAreas) {
  const int n = origins.size();
  QVector<QgsPointXY> placed(n);
  // Prepared site areas (polygons only); points and lines keep the ring search.
  std::vector<std::unique_ptr<QgsGeometryEngine>> engines(n);
  QVector<QgsRectangle> boxes(n);
  QVector<double> areas(n, 0.0);
  for (int i = 0; i < n && i < siteAreas.size(); ++i) {
    const QgsGeometry& area = siteAreas.at(i);
    if (area.isEmpty() || area.type() != Qgis::GeometryType::Polygon) continue;
    engines[i].reset(QgsGeometry::createGeometryEngine(area.constGet()));
    engines[i]->prepareGeometry();
    boxes[i] = area.boundingBox();
    areas[i] = area.area();
  }
  auto placeOne = [&](int i) {
    m_current = i;
    return place(origins.at(i), badgeDiametersMm.value(i), engines[i].get(), boxes.at(i));
  };
  // Line length to a slot: none inside the badge's own area.
  auto cost = [&](int i, const QgsPointXY& at) {
    if (engines[i] && boxes.at(i).contains(at)) {
      const QgsPoint point(at);
      if (engines[i]->intersects(&point)) return 0.0;
    }
    return origins.at(i).distance(at);
  };
  // Crowds: sites chained closer than the clear distance (union-find over step cells).
  QVector<int> root(n);
  std::iota(root.begin(), root.end(), 0);
  auto find = [&root](int i) {
    while (root[i] != i) i = root[i] = root[root[i]];
    return i;
  };
  QHash<quint64, QVector<int>> cells;
  for (int i = 0; i < n; ++i) {
    const qint64 cx = qint64(std::floor(origins.at(i).x() / m_step));
    const qint64 cy = qint64(std::floor(origins.at(i).y() / m_step));
    for (qint64 dx = -1; dx <= 1; ++dx) {
      for (qint64 dy = -1; dy <= 1; ++dy) {
        for (int j : cells.value(cellKey(cx + dx, cy + dy)))
          if (origins.at(i).distance(origins.at(j)) < m_step * kClearShare) root[find(i)] = find(j);
      }
    }
    cells[cellKey(cx, cy)].append(i);
  }
  QVector<QVector<int>> crowds(n);
  for (int i = 0; i < n; ++i) crowds[find(i)].append(i);
  // Lone sites first, so no crowd takes a site's own place; then the crowded
  // ones in number order, each on its site or the nearest free slot around it.
  for (int i = 0; i < n; ++i)
    if (crowds.at(find(i)).size() == 1) placed[i] = placeOne(i);
  // Small areas have the fewest slots on their site, so they choose first.
  QVector<int> crowded;
  for (int i = 0; i < n; ++i)
    if (crowds.at(find(i)).size() > 1) crowded.append(i);
  std::stable_sort(crowded.begin(), crowded.end(), [&](int a, int b) {
    const bool aArea = engines[a] != nullptr, bArea = engines[b] != nullptr;
    if (aArea != bArea) return !aArea;
    return aArea && areas.at(a) < areas.at(b);
  });
  for (int i : crowded) placed[i] = placeOne(i);
  // Two badges trade places whenever that shortens their leader lines. Crossing
  // lines always get shorter by trading, so none stay crossed; a badge on its
  // own site never gains from a trade, so it stays there.
  // Name boxes differ in width: a trade could put a long name over a neighbour.
  bool traded = m_boxesMm.isEmpty();
  for (int pass = 0; traded && pass < kMaxRings; ++pass) {
    traded = false;
    for (int a = 0; a < n; ++a) {
      for (int b = a + 1; b < n; ++b) {
        const double now = cost(a, placed.at(a)) + cost(b, placed.at(b));
        if (now <= m_step * 1e-6) continue;
        const double then = cost(a, placed.at(b)) + cost(b, placed.at(a));
        if (then >= now - m_step * 1e-6) continue;
        if (!fits(placed.at(b), badgeDiametersMm.value(a)) || !fits(placed.at(a), badgeDiametersMm.value(b))) continue;
        std::swap(placed[a], placed[b]);
        traded = true;
      }
    }
  }
  return placed;
}

QVector<QgsPointXY> LayoutBadgePlacer::placeAllBoxes(const QVector<QgsPointXY>& origins,
                                                     const QVector<QSizeF>& boxesMm,
                                                     const QVector<QgsGeometry>& siteAreas) {
  m_boxesMm = boxesMm;
  QVector<double> widest;
  for (const QSizeF& box : boxesMm) widest.append(std::max(box.width(), box.height()));
  return placeAll(origins, widest, siteAreas);
}

QVector<QPolygonF> LayoutBadgePlacer::legendBlocks(QgsLayoutItemMap* map, bool exporting) {
  QVector<QPolygonF> blocks;
  if (!map || !map->layout()) return blocks;
  const QRectF frame = map->sceneBoundingRect();
  QList<QgsLayoutItemLegend*> legends;
  map->layout()->layoutItems(legends);
  for (QgsLayoutItemLegend* legend : legends) {
    if (!legend || !legend->isVisible()) continue;
    if (exporting && legend->excludeFromExports()) continue;
    if (!legend->sceneBoundingRect().intersects(frame)) continue;
    blocks.append(legend->mapToScene(legend->rect()));
  }
  return blocks;
}

QPainterPath LayoutBadgePlacer::withoutBlocks(const QPainterPath& visibleScene,
                                              const QVector<QPolygonF>& blocks) {
  QPainterPath allowed = visibleScene;
  for (const QPolygonF& block : blocks) {
    QPainterPath cut;
    cut.addPolygon(block);
    cut.closeSubpath();
    allowed = allowed.subtracted(cut);
  }
  return allowed;
}
