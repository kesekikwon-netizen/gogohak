#pragma once

#include <QColor>
#include <QList>
#include <QPair>
#include <QString>

#include <qgsattributes.h>
#include <qgscoordinatereferencesystem.h>
#include <qgsfeatureid.h>
#include <qgsfields.h>
#include <qgsgeometry.h>

class QgsProject;
class QgsVectorLayer;

// 도형선택에서 고른 지적 필지를 복사해 두었다가 같은 자리에 붙인다 (Ctrl+C · Ctrl+V).
//
// 붙인 필지는 「지적도 · 복사한 필지」 줄에 든다. 지적도 역할이라 고칠 수 없고 자석이 붙는다.
// 복사본은 받은 지적도 줄과 따로 가진 도형이라 원본 줄을 지워도 남고, 조사 파일을 저장하면 같이 저장된다.
namespace ParcelClipboard {

// A parcel as it was copied: its geometry in the CRS of the layer it came from, its attributes
// and the look of that layer.
struct Parcel {
  QgsGeometry geometry;
  QgsCoordinateReferenceSystem crs;
  QgsFields fields;
  QgsAttributes attributes;
  QColor color = Qt::black;
  bool labels = true;
};

// The cadastral parcels among the picked (layer, feature id) pairs. Survey shapes are never copied.
QList<Parcel> copyFeatures(const QList<QPair<QgsVectorLayer*, QgsFeatureId>>& picked);

// Adds the parcels to 「지적도 · 복사한 필지」 at their own coordinates; the layer is made on the first
// paste. Returns the number of features added. When nothing is added, `error` says why.
int paste(QgsProject* project, const QList<Parcel>& parcels, QString* error = nullptr);

// The copy layer of `project`, or nullptr when nothing has been pasted yet.
QgsVectorLayer* copyLayer(const QgsProject* project);

}  // namespace ParcelClipboard
