#include "core/SnapTraceOps.h"
#include "core/KaLogExcept.h"
#include "core/LayerOps.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <QColor>
#include <queue>

#include <qgsabstractgeometry.h>
#include <qgscoordinatetransform.h>
#include <qgsfeature.h>
#include <qgsfeatureiterator.h>
#include <qgsfeaturerequest.h>
#include <qgsgeometry.h>
#include <qgsmapcanvas.h>
#include <qgsmaplayer.h>
#include <qgsmapsettings.h>
#include <qgspoint.h>
#include <qgsproject.h>
#include <qgsrectangle.h>
#include <qgsvectorlayer.h>
#include <qgsvertexid.h>
#include <qgsvertexmarker.h>

namespace SnapTraceOps {
namespace {

QList<QgsVectorLayer*> eligibleLayers(QgsMapCanvas* canvas) {
  QList<QgsVectorLayer*> out;
  if (!canvas) return out;
  QgsProject* proj = QgsProject::instance();
  if (proj && LayerOps::readSnapSettings(proj).target == LayerOps::SnapTarget::CurrentLayer) {
    if (auto* cur = qobject_cast<QgsVectorLayer*>(canvas->currentLayer())) {
      if (cur->isValid() && LayerOps::isSnapSourceLayer(cur)) out.append(cur);
      return out;
    }
  }
  QList<QgsMapLayer*> raw = canvas->layers();
  if (raw.isEmpty() && proj) raw = proj->mapLayers().values();
  for (QgsMapLayer* ml : raw) {
    auto* vl = qobject_cast<QgsVectorLayer*>(ml);
    if (vl && vl->isValid() && LayerOps::isSnapSourceLayer(vl) && !out.contains(vl)) out.append(vl);
  }
  return out;
}

bool toCanvasAndFetch(QgsVectorLayer* vl, const QgsRectangle& canvasBox, const QgsCoordinateReferenceSystem& canvasCrs,
                      QVector<QgsGeometry>& out) {
  if (!vl) return false;
  QgsRectangle reqBox = canvasBox;
  const bool needXf = canvasCrs.isValid() && vl->crs().isValid() && vl->crs() != canvasCrs;
  QgsCoordinateTransform toLayer, toCanvas;
  if (needXf) {
    const auto ctx =
        QgsProject::instance() ? QgsProject::instance()->transformContext() : QgsCoordinateTransformContext();
    toLayer = QgsCoordinateTransform(canvasCrs, vl->crs(), ctx);
    toCanvas = QgsCoordinateTransform(vl->crs(), canvasCrs, ctx);
    toLayer.setBallparkTransformsAreAppropriate(true);
    toCanvas.setBallparkTransformsAreAppropriate(true);
    try {
      reqBox = toLayer.transformBoundingBox(canvasBox);
    } catch (...) {
      KA_LOG_EXCEPT();
    }
  }
  QgsFeature feat;
  QgsFeatureIterator it = vl->getFeatures(QgsFeatureRequest().setFilterRect(reqBox).setNoAttributes());
  while (it.nextFeature(feat)) {
    if (!feat.hasGeometry()) continue;
    QgsGeometry g = feat.geometry();
    if (needXf) {
      try {
        if (g.transform(toCanvas) != Qgis::GeometryOperationResult::Success) continue;
      } catch (...) {
        KA_LOG_EXCEPT();
        continue;
      }
    }
    out.append(g);
  }
  return !out.isEmpty();
}

QgsPointXY projectOnSegment(const QgsPointXY& p, const QgsPointXY& a, const QgsPointXY& b) {
  const double dx = b.x() - a.x(), dy = b.y() - a.y(), lenSq = dx * dx + dy * dy;
  if (lenSq <= 1e-12) return a;
  const double t = std::clamp(((p.x() - a.x()) * dx + (p.y() - a.y()) * dy) / lenSq, 0.0, 1.0);
  return QgsPointXY(a.x() + t * dx, a.y() + t * dy);
}

}  // namespace

QVector<QgsPointXY> nearbyVertices(QgsMapCanvas* canvas, const QgsPointXY& cursorMapPt, const QgsPointXY* activeSnapPt,
                                   double radiusPx, int maxCount) {
  QVector<QgsPointXY> result;
  if (!canvas || radiusPx <= 0.0 || maxCount <= 0 || !std::isfinite(cursorMapPt.x()) || !std::isfinite(cursorMapPt.y()))
    return result;
  const double mupp = canvas->mapUnitsPerPixel() > 0.0 ? canvas->mapUnitsPerPixel() : 0.05;
  const double radiusMap = radiusPx * mupp, radiusSq = radiusMap * radiusMap;
  const double dedupSq = std::pow(std::max(mupp * 2.5, 0.01), 2);
  const QgsRectangle box(cursorMapPt.x() - radiusMap, cursorMapPt.y() - radiusMap, cursorMapPt.x() + radiusMap,
                         cursorMapPt.y() + radiusMap);
  const QgsCoordinateReferenceSystem canvasCrs = canvas->mapSettings().destinationCrs();
  QVector<QPair<double, QgsPointXY>> cands;
  for (QgsVectorLayer* vl : eligibleLayers(canvas)) {
    QVector<QgsGeometry> geoms;
    if (!toCanvasAndFetch(vl, box, canvasCrs, geoms)) continue;
    for (const QgsGeometry& geom : geoms) {
      for (auto vit = geom.vertices_begin(); vit != geom.vertices_end(); ++vit) {
        const QgsPointXY pt((*vit).x(), (*vit).y());
        const double dSq = cursorMapPt.sqrDist(pt);
        if (dSq > radiusSq || (activeSnapPt && activeSnapPt->sqrDist(pt) <= dedupSq)) continue;
        const bool dup =
            std::any_of(cands.begin(), cands.end(), [&](const auto& c) { return c.second.sqrDist(pt) <= dedupSq; });
        if (!dup) cands.append({dSq, pt});
      }
    }
  }
  std::sort(cands.begin(), cands.end(), [](const auto& a, const auto& b) { return a.first < b.first; });
  const int take = std::min(static_cast<int>(cands.size()), maxCount);
  for (int i = 0; i < take; ++i)
    result.append(cands[i].second);
  return result;
}

void clearPreviewMarkers(QList<QgsVertexMarker*>& markers) {
  qDeleteAll(markers);
  markers.clear();
}

void syncPreviewMarkers(QgsMapCanvas* canvas, QList<QgsVertexMarker*>& markers, const QVector<QgsPointXY>& points) {
  if (!canvas || points.isEmpty()) {
    clearPreviewMarkers(markers);
    return;
  }
  while (markers.size() > points.size())
    delete markers.takeLast();
  while (markers.size() < points.size()) {
    auto* m = new QgsVertexMarker(canvas);
    m->setIconType(QgsVertexMarker::ICON_CIRCLE);
    m->setIconSize(9);
    m->setPenWidth(2);
    m->setColor(QColor(217, 119, 6, 230));
    m->setFillColor(QColor(255, 251, 235, 210));
    markers.append(m);
  }
  for (int i = 0; i < points.size(); ++i) {
    markers[i]->setCenter(points[i]);
    markers[i]->show();
  }
}

QVector<QgsPointXY> traceBetween(QgsMapCanvas* canvas, const QgsPointXY& fromMapPt, const QgsPointXY& toMapPt,
                                 int maxIntermediate) {
  if (!canvas || maxIntermediate <= 0) return {};
  const double chord = fromMapPt.distance(toMapPt);
  const double mupp = canvas->mapUnitsPerPixel() > 0.0 ? canvas->mapUnitsPerPixel() : 0.05;
  const double snapTol = std::max(mupp * 16.0, 0.25), mergeTol = std::max(mupp * 0.4, 0.005);
  const double mergeSq = mergeTol * mergeTol, skipSq = std::pow(std::max(mupp * 2.0, 0.02), 2);
  if (!std::isfinite(chord) || chord <= snapTol * 0.5) return {};

  QgsRectangle canvasBox(fromMapPt, toMapPt);
  canvasBox.grow(std::max(chord * 0.6, snapTol * 2.0));
  const QgsCoordinateReferenceSystem canvasCrs = canvas->mapSettings().destinationCrs();
  QVector<QgsPointXY> bestPath;
  double bestDist = std::numeric_limits<double>::infinity();

  for (QgsVectorLayer* vl : eligibleLayers(canvas)) {
    QVector<QgsGeometry> geoms;
    if (!toCanvasAndFetch(vl, canvasBox, canvasCrs, geoms)) continue;
    QVector<QgsPointXY> nodes;
    QVector<QVector<QPair<int, double>>> adj;
    QVector<QPair<int, int>> segs;
    auto addNode = [&](const QgsPointXY& p) -> int {
      for (int i = 0; i < nodes.size(); ++i)
        if (nodes[i].sqrDist(p) <= mergeSq) return i;
      nodes.append(p);
      adj.emplaceBack();
      return nodes.size() - 1;
    };
    auto addEdge = [&](int u, int v) {
      if (u == v) return;
      const double w = nodes[u].distance(nodes[v]);
      adj[u].append({v, w});
      adj[v].append({u, w});
      segs.append({u, v});
    };
    for (const QgsGeometry& geom : geoms) {
      if (nodes.size() >= 600) break;
      const QgsAbstractGeometry* ag = geom.constGet();
      if (!ag) continue;
      for (int p = 0; p < ag->partCount(); ++p) {
        for (int r = 0; r < ag->ringCount(p); ++r) {
          const int n = ag->vertexCount(p, r);
          for (int v = 0; v + 1 < n; ++v) {
            const QgsPoint p0 = ag->vertexAt(QgsVertexId(p, r, v));
            const QgsPoint p1 = ag->vertexAt(QgsVertexId(p, r, v + 1));
            const QgsPointXY a(p0.x(), p0.y()), b(p1.x(), p1.y());
            if (canvasBox.contains(a) || canvasBox.contains(b)) addEdge(addNode(a), addNode(b));
          }
        }
      }
    }
    if (nodes.size() < 3) continue;

    auto attach = [&](const QgsPointXY& target) -> int {
      const double vTol = std::max(mergeTol * 4.0, mupp * 4.0);
      int bestNode = -1, bestSeg = -1;
      double bestNodeDist = snapTol, bestSegDist = snapTol;
      QgsPointXY bestProj;
      for (int i = 0; i < nodes.size(); ++i) {
        if (const double d = nodes[i].distance(target); d <= bestNodeDist) {
          bestNodeDist = d;
          bestNode = i;
        }
      }
      if (bestNode >= 0 && bestNodeDist <= vTol) return bestNode;
      for (int i = 0; i < segs.size(); ++i) {
        const QgsPointXY proj = projectOnSegment(target, nodes[segs[i].first], nodes[segs[i].second]);
        if (const double d = proj.distance(target); d < bestSegDist) {
          bestSegDist = d;
          bestSeg = i;
          bestProj = proj;
        }
      }
      if (bestSeg < 0) return bestNode;
      const auto [u, v] = segs[bestSeg];
      if (bestProj.distance(nodes[u]) <= vTol) return u;
      if (bestProj.distance(nodes[v]) <= vTol) return v;
      const int w = addNode(bestProj);
      addEdge(w, u);
      addEdge(w, v);
      return w;
    };

    const int s = attach(fromMapPt), t = attach(toMapPt);
    if (s < 0 || t < 0 || s == t) continue;
    QVector<double> dist(nodes.size(), std::numeric_limits<double>::infinity());
    QVector<int> parent(nodes.size(), -1);
    using Item = QPair<double, int>;
    std::priority_queue<Item, QVector<Item>, std::greater<Item>> pq;
    pq.push({dist[s] = 0.0, s});
    while (!pq.empty()) {
      const auto [d, u] = pq.top();
      pq.pop();
      if (u == t) break;
      if (d > dist[u]) continue;
      for (const auto& [v, w] : adj[u]) {
        if (d + w < dist[v]) {
          parent[v] = u;
          pq.push({dist[v] = d + w, v});
        }
      }
    }
    if (!std::isfinite(dist[t]) || dist[t] > chord * 2.2 || dist[t] >= bestDist) continue;
    QVector<int> chain;
    for (int cur = parent[t]; cur >= 0 && cur != s; cur = parent[cur])
      chain.prepend(cur);
    QVector<QgsPointXY> mids;
    for (int idx : chain)
      if (nodes[idx].sqrDist(fromMapPt) > skipSq && nodes[idx].sqrDist(toMapPt) > skipSq) mids.append(nodes[idx]);
    if (!mids.isEmpty() && mids.size() <= maxIntermediate) {
      bestDist = dist[t];
      bestPath = mids;
    }
  }
  return bestPath;
}

}  // namespace SnapTraceOps
