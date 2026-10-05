#include "TrenchTerrain.h"

#include <QtMath>

#include <qgscoordinatetransform.h>
#include <qgsexception.h>
#include <qgsfeatureiterator.h>
#include <qgsfeaturerequest.h>
#include <qgsgeometry.h>
#include <qgsproject.h>
#include <qgsvectorlayer.h>

#include <algorithm>
#include <cmath>

namespace TrenchTerrain {

namespace {
constexpr double kReachM = 30.0;           // contours just outside the area still tell its slope
constexpr double kStepM = 5.0;             // one sample every 5 m along a contour
constexpr std::size_t kMaxSamples = 4000;  // enough for a stable plane, quick on a 1:5000 sheet
}  // namespace

std::vector<TrenchGridGenerator::ElevSample> contourSamples(const QgsProject* project, const QByteArray& areaWkb,
                                                             const QString& areaCrs) {
  std::vector<TrenchGridGenerator::ElevSample> out;
  if (!project || areaWkb.isEmpty()) return out;
  QgsGeometry area;
  area.fromWkb(areaWkb);
  if (area.isNull() || area.isEmpty()) return out;
  const QgsCoordinateReferenceSystem crs = areaCrs.isEmpty() ? project->crs() : QgsCoordinateReferenceSystem(areaCrs);
  const QgsGeometry reach = area.buffer(kReachM, 8);
  for (QgsMapLayer* map : project->mapLayers()) {
    auto* layer = qobject_cast<QgsVectorLayer*>(map);
    if (!layer || !layer->isValid() || layer->geometryType() != Qgis::GeometryType::Line) continue;
    const int field = layer->fields().lookupField(QStringLiteral("elevation"));
    if (field < 0) continue;  // roads, buildings: no 표고
    QgsGeometry reachInLayer = reach;
    const QgsCoordinateTransform toLayer(crs, layer->crs(), project);
    const QgsCoordinateTransform toArea(layer->crs(), crs, project);
    try {
      reachInLayer.transform(toLayer);
    } catch (const QgsCsException&) {
      continue;
    }
    QgsFeatureRequest request(reachInLayer.boundingBox());
    request.setSubsetOfAttributes(QgsAttributeList{field});
    QgsFeatureIterator it = layer->getFeatures(request);
    QgsFeature feature;
    while (it.nextFeature(feature)) {
      const QVariant value = feature.attribute(field);
      bool ok = false;
      const double z = value.toDouble(&ok);
      if (value.isNull() || !ok || !std::isfinite(z)) continue;
      QgsGeometry line = feature.geometry();
      try {
        line.transform(toArea);
      } catch (const QgsCsException&) {
        continue;
      }
      const QgsGeometry near = line.intersection(reach).densifyByDistance(kStepM);
      for (auto v = near.vertices_begin(); v != near.vertices_end(); ++v) {
        out.push_back({(*v).x(), (*v).y(), z});
        if (out.size() >= kMaxSamples) return out;
      }
    }
  }
  return out;
}

TrenchGridGenerator::SlopeAspect contourAspect(const QgsProject* project, const QByteArray& areaWkb,
                                               const QString& areaCrs) {
  return TrenchGridGenerator::upslopeAspect(contourSamples(project, areaWkb, areaCrs));
}

void numberFromHighSide(std::vector<TrenchGridGenerator::Cell>& cells, double azimuthDeg, const QString& prefix) {
  const double rad = qDegreesToRadians(azimuthDeg);
  const double ux = std::sin(rad), uy = std::cos(rad);   // along the azimuth (north at 0°)
  const double vx = std::cos(rad), vy = -std::sin(rad);  // across it (east at 0°)
  struct Key {
    double along;
    double across;
    std::size_t at;
  };
  std::vector<Key> keys;
  keys.reserve(cells.size());
  for (std::size_t i = 0; i < cells.size(); ++i) {
    const auto& r = cells[i].ring;
    const double cx = (r[0].first + r[1].first + r[2].first + r[3].first) / 4.0;
    const double cy = (r[0].second + r[1].second + r[2].second + r[3].second) / 4.0;
    // Centimetres: cells of one row share a value exactly, so the row reads left to right.
    keys.push_back({std::round((cx * ux + cy * uy) * 100.0), cx * vx + cy * vy, i});
  }
  std::sort(keys.begin(), keys.end(), [](const Key& a, const Key& b) {
    return a.along != b.along ? a.along > b.along : a.across < b.across;
  });
  std::vector<TrenchGridGenerator::Cell> ordered;
  ordered.reserve(cells.size());
  for (const Key& key : keys) {
    ordered.push_back(cells[key.at]);
    ordered.back().name = prefix + QString::number(ordered.size());
  }
  cells = std::move(ordered);
}

}  // namespace TrenchTerrain
