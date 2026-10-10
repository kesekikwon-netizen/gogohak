#include "core/ParcelClipboard.h"

#include "core/CadastralImport.h"
#include "core/LayerOps.h"

#include <qgscoordinatetransform.h>
#include <qgsexception.h>
#include <qgsproject.h>
#include <qgsvectordataprovider.h>
#include <qgsvectorlayer.h>
#include <qgsvectorlayerutils.h>
#include <qgswkbtypes.h>

namespace {
const QString kCopyLayerName = QStringLiteral("지적도 · 복사한 필지");
const QString kCopyProperty = QStringLiteral("ka_hgis/cadastral_copy");
const QString kColorProperty = QStringLiteral("ka_hgis/cadastral_color");

// A multipolygon gives one polygon per part; a single polygon gives itself.
QList<QgsGeometry> partsOf(const QgsGeometry& geometry) {
  if (!QgsWkbTypes::isMultiType(geometry.wkbType())) return {geometry};
  QList<QgsGeometry> parts;
  for (const QgsGeometry& part : geometry.asGeometryCollection())
    parts.append(part);
  return parts;
}

// The attributes of `parcel` laid out in the fields of the copy layer; a field it lacks stays empty.
QgsAttributes attributesFor(const ParcelClipboard::Parcel& parcel, const QgsFields& fields) {
  QgsAttributes out(fields.count());
  for (int i = 0; i < fields.count(); ++i) {
    const int from = parcel.fields.lookupField(fields.at(i).name());
    if (from >= 0 && from < parcel.attributes.size()) out[i] = parcel.attributes.at(from);
  }
  return out;
}

// 「지적도 · 복사한 필지」 줄을 처음 붙일 때 만든다. 지적도 역할이라 고칠 수 없고, 받은 지적도와 같은 모양이다.
QgsVectorLayer* makeCopyLayer(QgsProject* project, const ParcelClipboard::Parcel& first) {
  // 붙일 때 조각마다 한 줄로 나누므로 줄은 단일 폴리곤이다.
  auto* layer = new QgsVectorLayer(QStringLiteral("Polygon?crs=%1").arg(first.crs.authid()), kCopyLayerName,
                                   QStringLiteral("memory"));
  if (!layer->isValid()) {
    delete layer;
    return nullptr;
  }
  QList<QgsField> fields;
  // 받은 지적도(GPKG)의 fid 칸은 옮기지 않는다. 같은 값이 되풀이되어 저장할 때 조사 파일에 들어가지 못한다.
  for (const QgsField& field : first.fields) {
    if (field.name().compare(QStringLiteral("fid"), Qt::CaseInsensitive) != 0) fields.append(field);
  }
  layer->dataProvider()->addAttributes(fields);
  layer->updateFields();
  LayerOps::markCadastralLayer(layer);
  layer->setCustomProperty(kCopyProperty, true);
  CadastralImport::applyStyle(layer, first.color, first.labels);
  project->addMapLayer(layer, false);
  LayerOps::placeCadastralLayer(project, layer);
  return layer;
}
}  // namespace

namespace ParcelClipboard {

QList<Parcel> copyFeatures(const QList<QPair<QgsVectorLayer*, QgsFeatureId>>& picked) {
  QList<Parcel> parcels;
  for (const auto& pick : picked) {
    QgsVectorLayer* layer = pick.first;
    // 조사 도형은 복사하지 않는다. 지적 필지(받은 지적도와 그 복사본)만 담는다.
    if (!layer || !layer->isValid() || !LayerOps::isCadastralLayer(layer)) continue;
    const QgsFeature feature = layer->getFeature(pick.second);
    if (!feature.hasGeometry() ||
        QgsWkbTypes::geometryType(feature.geometry().wkbType()) != Qgis::GeometryType::Polygon)
      continue;
    Parcel parcel;
    parcel.geometry = feature.geometry();
    parcel.crs = layer->crs();
    parcel.fields = layer->fields();
    parcel.attributes = feature.attributes();
    parcel.color = QColor(layer->customProperty(kColorProperty, QStringLiteral("#000000")).toString());
    parcel.labels = layer->labelsEnabled();
    parcels.append(parcel);
  }
  return parcels;
}

QgsVectorLayer* copyLayer(const QgsProject* project) {
  if (!project) return nullptr;
  for (QgsMapLayer* layer : project->mapLayers()) {
    if (layer->customProperty(kCopyProperty).toBool()) return qobject_cast<QgsVectorLayer*>(layer);
  }
  return nullptr;
}

int paste(QgsProject* project, const QList<Parcel>& parcels, QString* error) {
  if (error) error->clear();
  if (!project || parcels.isEmpty()) {
    if (error) *error = QStringLiteral("복사한 지적 필지가 없습니다. 도형선택으로 필지를 고른 뒤 Ctrl+C 하세요.");
    return 0;
  }
  QgsVectorLayer* layer = copyLayer(project);
  if (!layer) layer = makeCopyLayer(project, parcels.first());
  // 편집 버퍼에 담아 둔다. 조사 파일을 저장할 때 커밋되고, 그때 조사 파일 안으로 들어간다.
  if (!layer || (!layer->isEditable() && !layer->startEditing())) {
    if (error) *error = QStringLiteral("「지적도 · 복사한 필지」 줄에 붙이지 못했습니다.");
    return 0;
  }
  int added = 0;
  for (const Parcel& parcel : parcels) {
    QgsGeometry geometry = parcel.geometry;
    if (parcel.crs != layer->crs()) {
      // 좌표계가 달라도 같은 자리에 놓는다: 붙이는 줄의 좌표계로 옮긴다.
      try {
        const QgsCoordinateTransform toLayer(parcel.crs, layer->crs(), project->transformContext());
        if (geometry.transform(toLayer) != Qgis::GeometryOperationResult::Success) continue;
      } catch (const QgsCsException&) {
        continue;
      }
    }
    // 여러 조각인 필지는 조각마다 한 줄로 넣는다(GPKG 다중 도형 함정).
    for (const QgsGeometry& part : partsOf(geometry)) {
      QgsFeature feature(layer->fields());
      feature.setGeometry(part);
      feature.setAttributes(attributesFor(parcel, layer->fields()));
      for (QgsFeature& made : QgsVectorLayerUtils::makeFeatureCompatible(feature, layer)) {
        if (layer->addFeature(made)) ++added;
      }
    }
  }
  if (added > 0)
    project->setDirty(true);
  else if (error)
    *error = QStringLiteral("붙일 수 있는 필지가 없었습니다.");
  return added;
}

}  // namespace ParcelClipboard
