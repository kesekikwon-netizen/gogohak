#include "NearbyHeritageExport.h"
#include "HeritageImport.h"
#include "HeritageLayoutNumbers.h"
#include "HeritageStyle.h"
#include "KaLogExcept.h"
#include "LayerFeatures.h"
#include "LayerOps.h"
#include <algorithm>
#include <cmath>
#include <cpl_error.h>
#include <gdal_priv.h>
#include <ogrsf_frmts.h>
#include <QDir>
#include <QFileInfo>
#include <QMap>
#include <QSaveFile>
#include <QTemporaryDir>
#include <qgscoordinatetransform.h>
#include <qgsfeatureiterator.h>
#include <qgsgeometry.h>
#include <qgslayertree.h>
#include <qgslayertreelayer.h>
#include <qgslinestring.h>
#include <qgsproject.h>
#include <qgsvectorlayer.h>

namespace NearbyHeritageExport {
namespace {

QgsGeometry polygonsOnly(const QgsGeometry& g) {
  if (g.isEmpty() || g.type() == Qgis::GeometryType::Polygon) return g;
  QVector<QgsGeometry> polys;
  for (const QgsGeometry& part : g.asGeometryCollection())
    if (part.type() == Qgis::GeometryType::Polygon) polys << part;
  return polys.isEmpty() ? QgsGeometry() : QgsGeometry::collectGeometry(polys);
}

QgsGeometry unionPolygonsInCrs(QgsVectorLayer* source, const QgsCoordinateReferenceSystem& dest, QgsProject* project,
                               QString* errorOut) {
  const bool needXf = source->crs().isValid() && dest.isValid() && source->crs() != dest;
  QgsCoordinateTransform xf(source->crs(), dest,
                            project ? project->transformContext() : QgsCoordinateTransformContext());
  if (needXf) xf.setBallparkTransformsAreAppropriate(true);
  QgsFeatureIterator it = source->getFeatures();
  QgsFeature f;
  QVector<QgsGeometry> parts;
  while (it.nextFeature(f)) {
    QgsGeometry g = f.geometry();
    if (g.isEmpty()) continue;
    if (needXf) {
      try {
        if (g.transform(xf) != Qgis::GeometryOperationResult::Success) continue;
      } catch (...) {
        KA_LOG_EXCEPT();
        continue;
      }
    }
    parts << g;
  }
  QgsGeometry poly = polygonsOnly(parts.isEmpty() ? QgsGeometry() : QgsGeometry::unaryUnion(parts));
  if (poly.isEmpty()) {
    if (errorOut) *errorOut = QStringLiteral("조사구역 면 도형이 없습니다. 조사구역을 먼저 그리세요.");
    return {};
  }
  poly.removeDuplicateNodes(0.05);
  const QgsGeometry valid = polygonsOnly(poly.makeValid());
  return valid.isEmpty() ? poly : valid;
}

std::optional<HeritageDataset> datasetForLayer(const QgsProject* project, const QgsVectorLayer* layer) {
  if (!layer) return std::nullopt;
  if (auto tagged = HeritageLayoutNumbers::taggedDataset(layer)) return tagged;
  if (project && project->layerTreeRoot())
    for (QgsLayerTreeNode* node = project->layerTreeRoot()->findLayer(layer->id()); node; node = node->parent())
      if (auto ds = HeritageStyle::fromLayerName(node->name())) return ds;
  const QString name = layer->name().trimmed();
  if (auto ds = HeritageStyle::fromLayerName(name)) return ds;
  for (HeritageDataset ds : HeritageStyle::allDatasets())
    if (const QString base = HeritageStyle::layerName(ds); !base.isEmpty() && name.startsWith(base)) return ds;
  if (name.contains(QStringLiteral("지정유산")) || name.contains(QStringLiteral("등록문화유산")) ||
      name.contains(QStringLiteral("보호구역")))
    return HeritageDataset::DesignatedHeritage;
  return std::nullopt;
}

QList<QgsVectorLayer*> heritageLayersInProject(QgsProject* project) {
  QList<QgsVectorLayer*> out;
  if (!project) return out;
  QSet<QString> seen;
  auto tryAdd = [&](QgsMapLayer* ml) {
    auto* vl = qobject_cast<QgsVectorLayer*>(ml);
    if (!vl || !vl->isValid() || seen.contains(vl->id()) || !datasetForLayer(project, vl).has_value()) return;
    seen.insert(vl->id());
    out.append(vl);
  };
  if (project->layerTreeRoot())
    for (QgsLayerTreeLayer* node : project->layerTreeRoot()->findLayers())
      tryAdd(node->layer());
  for (QgsMapLayer* ml : project->mapLayers())
    tryAdd(ml);
  return out;
}

double computeAzimuthDeg(const QgsGeometry& sitePoly, const QgsGeometry& target) {
  const QgsGeometry line = sitePoly.shortestLine(target);
  if (const QgsLineString* ls = line.isEmpty() ? nullptr : qgsgeometry_cast<const QgsLineString*>(line.constGet());
      ls && ls->numPoints() >= 2) {
    const QgsPointXY p1(ls->startPoint().x(), ls->startPoint().y()), p2(ls->endPoint().x(), ls->endPoint().y());
    if (p1.distance(p2) > 1e-4) return std::fmod(std::fmod(p1.azimuth(p2), 360.0) + 360.0, 360.0);
  }
  const QgsGeometry c1 = sitePoly.pointOnSurface(), c2 = target.pointOnSurface();
  return (!c1.isEmpty() && !c2.isEmpty())
             ? std::fmod(std::fmod(c1.asPoint().azimuth(c2.asPoint()), 360.0) + 360.0, 360.0)
             : 0.0;
}
}  // namespace

QgsVectorLayer* resolveSourceLayer(QgsProject* project, QgsVectorLayer* preferred) {
  auto isUsable = [](QgsVectorLayer* vl) {
    return vl && vl->isValid() && vl->geometryType() == Qgis::GeometryType::Polygon &&
           !LayerOps::isReferenceLayer(vl) && !LayerOps::layerKeyOf(vl).startsWith(QLatin1String("user:buffer_")) &&
           LayerFeatures::any(vl);
  };
  if (isUsable(preferred)) return preferred;
  const QgsLayerTree* root = project ? project->layerTreeRoot() : nullptr;
  QgsVectorLayer* fallback = preferred;
  for (QgsVectorLayer* vl : LayerOps::surveyAreaLayers(project)) {
    if (!isUsable(vl)) continue;
    if (const QgsLayerTreeLayer* n = root ? root->findLayer(vl->id()) : nullptr; !n || n->isVisible()) return vl;
    if (fallback == preferred) fallback = vl;
  }
  return fallback;
}

double activeMaxRadiusMeters(const QgsProject* project) {
  auto* mut = const_cast<QgsProject*>(project);
  const QgsLayerTree* root = project ? project->layerTreeRoot() : nullptr;
  auto hasRing = [&](const QString& key, bool requireVisible) {
    QgsMapLayer* ml = mut ? LayerOps::findByLayerKey(mut, key) : nullptr;
    const QgsLayerTreeLayer* n = (ml && root) ? root->findLayer(ml->id()) : nullptr;
    return ml && (!requireVisible || !n || n->isVisible());
  };
  if (hasRing(QStringLiteral("user:buffer_ring_1000"), true)) return 1000.0;
  if (hasRing(QStringLiteral("user:buffer_ring_500"), true)) return 500.0;
  return hasRing(QStringLiteral("user:buffer_ring_500"), false) &&
                 !hasRing(QStringLiteral("user:buffer_ring_1000"), false)
             ? 500.0
             : 1000.0;
}

QString cardinalDirection8(double azimuthDegrees) {
  const double norm = std::fmod(std::fmod(azimuthDegrees, 360.0) + 360.0, 360.0);
  static const char* const kDirs[8] = {"북", "북동", "동", "남동", "남", "남서", "서", "북서"};
  return QString::fromUtf8(kDirs[static_cast<int>(std::floor((norm + 22.5) / 45.0)) % 8]);
}

Result collect(QgsProject* project, QgsVectorLayer* sourceLayer, double maxRadiusMeters) {
  Result out;
  if (!project) {
    out.error = QStringLiteral("열린 조사가 없습니다.");
    return out;
  }
  QgsVectorLayer* source = resolveSourceLayer(project, sourceLayer);
  if (!source || !source->isValid() || source->geometryType() != Qgis::GeometryType::Polygon) {
    out.error = QStringLiteral("조사구역 면 레이어를 먼저 그리거나 선택하세요.");
    return out;
  }
  const QList<QgsVectorLayer*> heritageLayers = heritageLayersInProject(project);
  if (heritageLayers.isEmpty()) {
    out.error = QStringLiteral("지도에 주변유적(유산) 레이어가 없습니다. 먼저 「유산」을 눌러 자료를 받아 오세요.");
    return out;
  }
  out.maxRadiusMeters = (maxRadiusMeters >= 1.0) ? maxRadiusMeters : activeMaxRadiusMeters(project);
  QgsCoordinateReferenceSystem dest = project->crs();
  if (!dest.isValid() || dest.isGeographic()) dest = source->crs();
  if (!dest.isValid() || dest.isGeographic()) dest = QgsCoordinateReferenceSystem(QStringLiteral("EPSG:5187"));

  const QgsGeometry sitePoly = unionPolygonsInCrs(source, dest, project, &out.error);
  if (sitePoly.isEmpty()) return out;

  for (QgsVectorLayer* layer : heritageLayers) {
    const auto ds = datasetForLayer(project, layer);
    if (!ds) continue;
    const QString datasetName = HeritageStyle::layerName(*ds), nameField = HeritageImport::chooseNameField(layer);
    const bool needXf = layer->crs().isValid() && layer->crs() != dest;
    QgsCoordinateTransform xf(layer->crs(), dest, project->transformContext());
    if (needXf) xf.setBallparkTransformsAreAppropriate(true);
    QMap<QString, QPair<QString, QVector<QgsGeometry>>> grouped;
    QgsFeatureIterator it = layer->getFeatures();
    QgsFeature f;
    while (it.nextFeature(f)) {
      QgsGeometry g = f.geometry();
      if (g.isEmpty()) continue;
      if (needXf) {
        try {
          if (g.transform(xf) != Qgis::GeometryOperationResult::Success) continue;
        } catch (...) {
          KA_LOG_EXCEPT();
          continue;
        }
      }
      const QString rawName = nameField.isEmpty() ? QString() : f.attribute(nameField).toString().trimmed();
      const QString key = rawName.isEmpty() ? QStringLiteral("__unnamed_%1").arg(f.id()) : rawName;
      auto& entry = grouped[key];
      if (entry.first.isEmpty()) entry.first = rawName.isEmpty() ? HeritageStyle::unnamedLabel() : rawName;
      entry.second.append(g);
    }
    for (auto git = grouped.cbegin(); git != grouped.cend(); ++git) {
      const QgsGeometry g = (git->second.size() == 1) ? git->second.first() : QgsGeometry::unaryUnion(git->second);
      const double rawDist = g.isEmpty() ? -1.0 : sitePoly.distance(g);
      if (rawDist < 0.0 || !std::isfinite(rawDist) || rawDist > out.maxRadiusMeters) continue;

      Item item{datasetName, layer->name(), git->first, {}, {}, 0, -1.0, {}};
      if (sitePoly.intersects(g) || rawDist < 0.05) {
        item.direction = QStringLiteral("조사구역 내");
        item.radiusLabel = QStringLiteral("조사구역 내");
        item.description = QStringLiteral("조사구역 내 (0m)");
      } else {
        item.distanceMeters = std::max(1, qRound(rawDist));
        item.radiusLabel = (rawDist <= 500.0) ? QStringLiteral("500m 이내")
                                              : QStringLiteral("%1m 이내").arg(qRound(out.maxRadiusMeters));
        const double az = computeAzimuthDeg(sitePoly, g);
        item.azimuthDeg = std::round(az * 10.0) / 10.0;
        item.direction = cardinalDirection8(az);
        item.description = QStringLiteral("조사구역에서 %1쪽으로 약 %2m").arg(item.direction).arg(item.distanceMeters);
      }
      out.items.append(item);
    }
  }
  std::stable_sort(out.items.begin(), out.items.end(), [](const Item& a, const Item& b) {
    if (a.distanceMeters != b.distanceMeters) return a.distanceMeters < b.distanceMeters;
    return a.datasetName != b.datasetName ? a.datasetName < b.datasetName : a.siteName < b.siteName;
  });
  return out;
}

bool writeXlsx(const QString& path, const QVector<Item>& items, QString* errorOut) {
  QTemporaryDir tempDir;
  if (path.trimmed().isEmpty() || !tempDir.isValid()) {
    if (errorOut) *errorOut = QStringLiteral("엑셀 저장 경로 또는 임시 폴더가 유효하지 않습니다.");
    return false;
  }
  const QString scratchXlsx = tempDir.filePath(QStringLiteral("nearby_heritage.xlsx"));
  GDALAllRegister();
  CPLErrorReset();
  GDALDriver* driver = GetGDALDriverManager()->GetDriverByName("XLSX");
  GDALDataset* ds = driver ? driver->Create(scratchXlsx.toUtf8().constData(), 0, 0, 0, GDT_Unknown, nullptr) : nullptr;
  OGRLayer* sheet = ds ? ds->CreateLayer("주변유적_현황", nullptr, wkbNone, nullptr) : nullptr;
  auto fail = [&](const QString& msg) {
    if (ds) GDALClose(ds);
    if (errorOut) *errorOut = msg;
    return false;
  };
  if (!sheet) return fail(QStringLiteral("엑셀 시트를 만들지 못했습니다."));
  const QPair<const char*, OGRFieldType> kCols[] = {
      {"연번", OFTInteger},    {"자료구분", OFTString},   {"레이어명", OFTString},
      {"유적명", OFTString},   {"반경구분", OFTString},   {"방향", OFTString},
      {"거리(m)", OFTInteger}, {"방위각(도)", OFTString}, {"위치설명", OFTString}};
  for (const auto& col : kCols) {
    OGRFieldDefn fd(col.first, col.second);
    if (sheet->CreateField(&fd) != OGRERR_NONE)
      return fail(QStringLiteral("엑셀 열(%1) 생성에 실패했습니다.").arg(QString::fromUtf8(col.first)));
  }
  for (int i = 0; i < items.size(); ++i) {
    const Item& it = items.at(i);
    OGRFeature* feat = OGRFeature::CreateFeature(sheet->GetLayerDefn());
    const QString az = (it.azimuthDeg < 0.0) ? QStringLiteral("-") : QString::number(it.azimuthDeg, 'f', 1);
    feat->SetField(0, i + 1);
    const QString strs[] = {it.datasetName, it.layerName, it.siteName, it.radiusLabel, it.direction};
    for (int c = 0; c < 5; ++c)
      feat->SetField(c + 1, strs[c].toUtf8().constData());
    feat->SetField(6, it.distanceMeters);
    feat->SetField(7, az.toUtf8().constData());
    feat->SetField(8, it.description.toUtf8().constData());
    const OGRErr err = sheet->CreateFeature(feat);
    OGRFeature::DestroyFeature(feat);
    if (err != OGRERR_NONE) return fail(QStringLiteral("엑셀 행 기록에 실패했습니다."));
  }
  GDALClose(ds);
  ds = nullptr;
  QFile src(scratchXlsx);
  QDir().mkpath(QFileInfo(path).absolutePath());
  QSaveFile dst(path);
  if (!src.open(QIODevice::ReadOnly) || !dst.open(QIODevice::WriteOnly))
    return fail(QStringLiteral("엑셀 저장 경로를 열지 못했습니다: %1").arg(path));
  const QByteArray bytes = src.readAll();
  if (dst.write(bytes) != bytes.size() || !dst.commit())
    return fail(QStringLiteral("엑셀 파일을 저장하지 못했습니다: %1").arg(path));
  return true;
}

Result exportToXlsx(QgsProject* project, const QString& path, QgsVectorLayer* sourceLayer, double maxRadiusMeters) {
  Result res = collect(project, sourceLayer, maxRadiusMeters);
  if (res.ok()) writeXlsx(path, res.items, &res.error);
  return res;
}
}  // namespace NearbyHeritageExport
