// Shared fixture of the parcel copy tests (test_parcel_copy.cpp, test_parcel_copy_keys.cpp; split for the
// 300-line limit): a received cadastral map with one parcel, kept in memory or saved as a GPKG.
#pragma once

#include <QApplication>
#include <QKeyEvent>
#include <QTemporaryDir>
#include <QtTest>

#include <qgsfeature.h>
#include <qgsgeometry.h>
#include <qgsmapcanvas.h>
#include <qgsproject.h>
#include <qgsrectangle.h>
#include <qgsvectorfilewriter.h>
#include <qgsvectorlayer.h>

#include "core/LayerOps.h"

namespace ParcelCopyFixture {
inline const QString kSource = QStringLiteral("지적도 · 조사 주변 5km");
inline const QString kCopyName = QStringLiteral("지적도 · 복사한 필지");
inline const QString kPnu = QStringLiteral("1111011800100010000");

inline QgsGeometry square(double x, double y, double size) {
  return QgsGeometry::fromRect(QgsRectangle(x, y, x + size, y + size));
}

inline void addParcel(QgsVectorLayer* layer, const QgsGeometry& geometry, const QString& pnu) {
  QgsFeature feature(layer->fields());
  feature.setGeometry(geometry);
  feature.setAttribute(QStringLiteral("PNU"), pnu);
  feature.setAttribute(QStringLiteral("JIBUN"), QStringLiteral("1-1"));
  QVERIFY(layer->startEditing());
  QVERIFY(layer->addFeature(feature));
  QVERIFY(layer->commitChanges());
}

// 받은 지적도: a cadastral layer of parcels that carry PNU and JIBUN, added to `project`.
inline QgsVectorLayer* receivedMap(QgsProject& project, const QString& crs, const QgsGeometry& parcel) {
  auto* layer =
      new QgsVectorLayer(QStringLiteral("MultiPolygon?crs=%1&field=PNU:string(19)&field=JIBUN:string").arg(crs),
                         kSource, QStringLiteral("memory"));
  project.addMapLayer(layer, false);
  LayerOps::markCadastralLayer(layer);
  addParcel(layer, parcel, kPnu);
  return layer;
}

inline QList<QgsFeature> featuresOf(QgsVectorLayer* layer) {
  QList<QgsFeature> features;
  if (!layer) return features;
  QgsFeature feature;
  for (auto it = layer->getFeatures(); it.nextFeature(feature);)
    features.append(feature);
  return features;
}

inline QgsFeatureId firstFid(QgsVectorLayer* layer) {
  return *layer->allFeatureIds().constBegin();
}

// Ctrl+<key> reaching the map canvas, as a keyboard press does.
inline void pressCtrl(QgsMapCanvas& canvas, Qt::Key key) {
  QKeyEvent press(QEvent::KeyPress, key, Qt::ControlModifier);
  QApplication::sendEvent(&canvas, &press);
}

// 받은 지적도를 앱처럼 GPKG 파일로 저장해 둔다 (ogr 로 열면 fid 칸이 있다). 저장한 파일 경로를 돌려준다.
inline QString savedMap(const QTemporaryDir& dir, const QgsGeometry& parcel) {
  const QString outside = dir.filePath(QStringLiteral("received.gpkg"));
  QgsProject scratch;
  QgsVectorLayer memoryMap(QStringLiteral("MultiPolygon?crs=EPSG:5187&field=PNU:string(19)&field=JIBUN:string"),
                           kSource, QStringLiteral("memory"));
  addParcel(&memoryMap, parcel, kPnu);
  QgsVectorFileWriter::SaveVectorOptions options;
  options.driverName = QStringLiteral("GPKG");
  options.layerName = QStringLiteral("cadastral");
  options.fileEncoding = QStringLiteral("UTF-8");
  QString error;
  if (QgsVectorFileWriter::writeAsVectorFormatV3(&memoryMap, outside, scratch.transformContext(), options, &error) !=
      QgsVectorFileWriter::NoError)
    return QString();
  return outside;
}

inline int copyLayerCount(const QgsProject& project) {
  return static_cast<int>(project.mapLayersByName(kCopyName).size());
}
}  // namespace ParcelCopyFixture
