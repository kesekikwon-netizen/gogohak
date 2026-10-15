#pragma once

#include "core/KaLogExcept.h"
#include "core/LayerOps.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <queue>

#include <qgsabstractgeometry.h>
#include <qgscoordinatetransform.h>
#include <qgsfeature.h>
#include <qgsfeatureiterator.h>
#include <qgsfeaturerequest.h>
#include <qgsgeometry.h>
#include <qgslayertree.h>
#include <qgsmapcanvas.h>
#include <qgsmaplayer.h>
#include <qgsmapsettings.h>
#include <qgspoint.h>
#include <qgsproject.h>
#include <qgsrectangle.h>
#include <qgsvectorlayer.h>
#include <qgsvertexid.h>

namespace SnapTraceOps::detail {

inline QList<QgsVectorLayer*> eligibleLayers(QgsMapCanvas* canvas, QgsVectorLayer* excludeIfOnly = nullptr) {
  QList<QgsVectorLayer*> cadRef, survey;
  QgsProject* proj = QgsProject::instance();
  QList<QgsMapLayer*> raw = canvas ? canvas->layers() : QList<QgsMapLayer*>();
  if (proj) {
    QgsLayerTree* root = proj->layerTreeRoot();
    for (QgsMapLayer* ml : proj->mapLayers().values()) {
      if (!ml || raw.contains(ml)) continue;
      if (auto* node = root ? root->findLayer(ml->id()) : nullptr; !node || node->isVisible()) raw.append(ml);
    }
  }
  for (QgsMapLayer* ml : raw) {
    auto* vl = qobject_cast<QgsVectorLayer*>(ml);
    if (!vl || !vl->isValid() || !LayerOps::isSnapSourceLayer(vl) || vl == excludeIfOnly) continue;
    if (LayerOps::isCadastralLayer(vl) || LayerOps::isReferenceLayer(vl))
      cadRef.append(vl);
    else
      survey.append(vl);
  }
  cadRef.append(survey);
  return cadRef;
}

inline bool toCanvasAndFetch(QgsVectorLayer* vl, const QgsRectangle& canvasBox,
                             const QgsCoordinateReferenceSystem& canvasCrs, QgsVectorLayer* exLayer, QgsFeatureId exFid,
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
    if ((vl == exLayer && feat.id() == exFid) || !feat.hasGeometry()) continue;
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

inline QgsPointXY projectOnSegment(const QgsPointXY& p, const QgsPointXY& a, const QgsPointXY& b) {
  const double dx = b.x() - a.x(), dy = b.y() - a.y(), lenSq = dx * dx + dy * dy;
  if (lenSq <= 1e-12) return a;
  const double t = std::clamp(((p.x() - a.x()) * dx + (p.y() - a.y()) * dy) / lenSq, 0.0, 1.0);
  return QgsPointXY(a.x() + t * dx, a.y() + t * dy);
}

struct BoundaryGraph {
  double mergeTol = 0.01, mergeSq = 0.0001;
  QVector<QgsPointXY> nodes;
  QVector<QVector<QPair<int, double>>> adj;
  QVector<QPair<int, int>> segs;

  int addNode(const QgsPointXY& p) {
    for (int i = 0; i < nodes.size(); ++i)
      if (nodes[i].sqrDist(p) <= mergeSq) return i;
    nodes.append(p);
    adj.emplaceBack();
    return nodes.size() - 1;
  }
  void addEdge(int u, int v) {
    if (u == v) return;
    const double w = nodes[u].distance(nodes[v]);
    adj[u].append({v, w});
    adj[v].append({u, w});
    segs.append({u, v});
  }
  void addGeometries(const QVector<QgsGeometry>& geoms, const QgsRectangle& box) {
    for (const QgsGeometry& geom : geoms) {
      if (nodes.size() >= 900) break;
      const QgsAbstractGeometry* ag = geom.constGet();
      if (!ag) continue;
      for (int p = 0; p < ag->partCount(); ++p)
        for (int r = 0; r < ag->ringCount(p); ++r)
          for (int v = 0, n = ag->vertexCount(p, r); v + 1 < n; ++v) {
            const QgsPoint p0 = ag->vertexAt(QgsVertexId(p, r, v)), p1 = ag->vertexAt(QgsVertexId(p, r, v + 1));
            const QgsPointXY a(p0.x(), p0.y()), b(p1.x(), p1.y());
            if (box.contains(a) || box.contains(b)) addEdge(addNode(a), addNode(b));
          }
    }
    const int origSegs = segs.size();
    for (int s = 0; s < origSegs; ++s) {
      const auto [u, v] = segs[s];
      for (int w = 0; w < nodes.size(); ++w) {
        if (w == u || w == v) continue;
        if (projectOnSegment(nodes[w], nodes[u], nodes[v]).sqrDist(nodes[w]) <= mergeSq * 4.0) {
          addEdge(u, w);
          addEdge(w, v);
        }
      }
    }
  }
  QgsPointXY snapPoint(const QgsPointXY& target, double vTol, double sTol, bool* moved) {
    int bestN = -1, bestS = -1;
    double bestND = vTol, bestSD = sTol;
    QgsPointXY bestProj = target;
    for (int i = 0; i < nodes.size(); ++i)
      if (const double d = nodes[i].distance(target); d <= bestND) {
        bestND = d;
        bestN = i;
      }
    for (int i = 0; i < segs.size(); ++i) {
      const QgsPointXY proj = projectOnSegment(target, nodes[segs[i].first], nodes[segs[i].second]);
      if (const double d = proj.distance(target); d < bestSD) {
        bestSD = d;
        bestS = i;
        bestProj = proj;
      }
    }
    QgsPointXY res = target;
    if (bestN >= 0 && bestND <= std::max(bestSD * 2.2, vTol * 0.45))
      res = nodes[bestN];
    else if (bestS >= 0)
      res = bestProj;
    if (moved) *moved = (res.sqrDist(target) > mergeSq);
    return res;
  }
  int attach(const QgsPointXY& target, double snapTol, double vTol) {
    int bestN = -1, bestS = -1;
    double bestND = snapTol, bestSD = snapTol;
    QgsPointXY bestProj;
    for (int i = 0; i < nodes.size(); ++i)
      if (const double d = nodes[i].distance(target); d <= bestND) {
        bestND = d;
        bestN = i;
      }
    if (bestN >= 0 && bestND <= vTol) return bestN;
    for (int i = 0; i < segs.size(); ++i) {
      const QgsPointXY proj = projectOnSegment(target, nodes[segs[i].first], nodes[segs[i].second]);
      if (const double d = proj.distance(target); d < bestSD) {
        bestSD = d;
        bestS = i;
        bestProj = proj;
      }
    }
    if (bestS < 0) return bestN;
    const auto [u, v] = segs[bestS];
    if (bestProj.distance(nodes[u]) <= vTol) return u;
    if (bestProj.distance(nodes[v]) <= vTol) return v;
    const int w = addNode(bestProj);
    addEdge(w, u);
    addEdge(w, v);
    return w;
  }
  QVector<QgsPointXY> trace(const QgsPointXY& fromPt, const QgsPointXY& toPt, double snapTol, double vTol,
                            double skipSq, int maxMids) {
    const double chord = fromPt.distance(toPt);
    if (nodes.size() < 3 || !std::isfinite(chord) || chord <= mergeTol * 2.0) return {};
    const int s = attach(fromPt, snapTol, vTol), t = attach(toPt, snapTol, vTol);
    if (s < 0 || t < 0 || s == t) return {};
    const double corridorSq = std::pow(std::max(snapTol * 1.5, chord * 0.45), 2);
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
        if (projectOnSegment(nodes[v], fromPt, toPt).sqrDist(nodes[v]) > corridorSq) continue;
        if (d + w < dist[v]) {
          parent[v] = u;
          pq.push({dist[v] = d + w, v});
        }
      }
    }
    if (!std::isfinite(dist[t]) || dist[t] > chord * 2.1) return {};
    QVector<int> chain;
    for (int cur = parent[t]; cur >= 0 && cur != s; cur = parent[cur])
      chain.prepend(cur);
    QVector<QgsPointXY> mids;
    for (int idx : chain)
      if (nodes[idx].sqrDist(fromPt) > skipSq && nodes[idx].sqrDist(toPt) > skipSq) mids.append(nodes[idx]);
    return mids.size() <= maxMids ? mids : QVector<QgsPointXY>{};
  }
};

}  // namespace SnapTraceOps::detail
