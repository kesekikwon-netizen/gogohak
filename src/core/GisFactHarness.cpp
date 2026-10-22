#include "GisFactHarness.h"

#include "HeritageStyle.h"
#include "KaLogExcept.h"
#include "LayerFeatures.h"
#include "LayerOps.h"
#include "LayerRole.h"
#include "PaleoLandformService.h"
#include "SoilMapService.h"

#include <algorithm>
#include <QHash>
#include <QJsonArray>
#include <QRegularExpression>
#include <QSet>
#include <qgscoordinatetransform.h>
#include <qgsdistancearea.h>
#include <qgsgeometry.h>
#include <qgsproject.h>
#include <qgsvectorlayer.h>

namespace GisFactHarness {
namespace {

const QStringList kSpatialActions = {QStringLiteral("collect_nearby_sites"), QStringLiteral("add_distance_ring"),
                                     QStringLiteral("download_soil_map"), QStringLiteral("seed_paleo_landform"),
                                     QStringLiteral("generate_trench_grid")};

// 액션별 허용 파라미터. 목록 밖 키(경로·코드 등)는 거부한다.
QStringList allowedParams(const QString& action) {
  if (action == QLatin1String("collect_nearby_sites") || action == QLatin1String("add_distance_ring"))
    return {QStringLiteral("radiusMeters")};
  if (action == QLatin1String("generate_trench_grid")) return {QStringLiteral("targetLayerId")};
  return {};
}

QgsGeometry surveyUnion(QgsVectorLayer* survey) {
  QVector<QgsGeometry> parts;
  QgsFeature f;
  QgsFeatureIterator it = survey->getFeatures(QgsFeatureRequest().setNoAttributes());
  while (it.nextFeature(f)) {
    if (f.hasGeometry() && !f.geometry().isEmpty()) parts.push_back(f.geometry());
  }
  return parts.isEmpty() ? QgsGeometry() : QgsGeometry::unaryUnion(parts);
}

// 투영 좌표계(5186/5187 등)는 평면 면적, 지리 좌표계는 타원체 면적(㎡)으로 잰다.
double areaSquareMeters(QgsProject* project, QgsVectorLayer* survey, const QgsGeometry& area) {
  if (area.isEmpty()) return -1.0;
  if (!survey->crs().isGeographic()) return area.area();
  QgsDistanceArea da;
  da.setSourceCrs(survey->crs(), project->transformContext());
  da.setEllipsoid(survey->crs().ellipsoidAcronym().isEmpty() ? QStringLiteral("EPSG:7019")
                                                             : survey->crs().ellipsoidAcronym());
  return da.convertAreaMeasurement(da.measureArea(area), Qgis::AreaUnit::SquareMeters);
}

// 조사구역과 겹치는 흙토람 분포지형 이름(공식 이름, 중복 제거).
QStringList soilTerrainNames(QgsProject* project, QgsVectorLayer* soil, QgsVectorLayer* survey,
                             const QgsGeometry& area) {
  QStringList names;
  const int field = soil->fields().indexOf(QStringLiteral("soil_type_geo"));
  if (field < 0 || area.isEmpty()) return names;
  QgsGeometry probe = area;
  if (soil->crs() != survey->crs()) {
    try {
      QgsCoordinateTransform ct(survey->crs(), soil->crs(), project);
      ct.setBallparkTransformsAreAppropriate(true);
      if (probe.transform(ct) != Qgis::GeometryOperationResult::Success) return names;
    } catch (...) {
      KA_LOG_EXCEPT();  // 토양만 건너뛴다. 면적·주변유적 카드는 그대로 둔다.
      return names;
    }
  }
  QgsFeatureRequest request(probe.boundingBox());
  request.setSubsetOfAttributes(QgsAttributeList{field});
  QgsFeature f;
  QgsFeatureIterator it = soil->getFeatures(request);
  while (it.nextFeature(f)) {
    if (!f.hasGeometry() || !f.geometry().intersects(probe)) continue;
    const QString name = SoilMapService::terrainName(f.attribute(field).toString());
    if (!name.isEmpty() && !names.contains(name)) names << name;
  }
  return names;
}

Card makeCard(int number, const QString& kind, const QString& subject, const QString& attribute, const QString& value,
              const QString& unit, const QString& quote) {
  Card c;
  c.id = QStringLiteral("G%1").arg(number);
  c.kind = kind, c.subject = subject, c.attribute = attribute, c.value = value, c.unit = unit, c.quote = quote;
  return c;
}

QString cleanSiteQueryBase(QString s) {
  static const QRegularExpression reDrop(QStringLiteral(
      R"(\([^)]*\)|\[[^\]]*\]|(?:산\s*)?\d+(?:-\d+)?\s*번지(?:\s*일원|\s*내)?|\d+\s*(?:통|호|지구|구역|구간)|(?:공동주택|단독주택|근린생활시설|공용주차장|주차장|창고시설|공장|건물|도로개설|정비사업|태양광발전시설|소규모|문화재|임시활용\s*등을\s*위한)(?:\s*(?:신축|증축|조성|개설|설치|부지|사업|공사|내|일원))*|(?:신축|증축|조성|개설|부지)\s*(?:내|일원)?|\b(?:일원|부지내|부지|내)\b)"));
  static const QRegularExpression reTrail(QStringLiteral(R"(([가-힣])(?:[0-9]+|[ⅠⅡⅢⅣⅤⅥⅦⅧⅨⅩ]+)(?=\s|$))"));
  static const QRegularExpression reComp(
      QStringLiteral(R"(([가-힣]{2,})(유적|고분군|고분|사지|산성|읍성|유물산포지|패총|가마터|전랑지|건물지))"));
  static const QRegularExpression reCity(QStringLiteral(R"(^([가-힣]{2,3})(?:시|군)\s+)"));
  static const QRegularExpression reSkip(
      QStringLiteral(R"(^(?:[가-하A-Z0-9]{1,2}\s*(?:지구|구역|구간|지역)|조사구역|유적|시굴조사|발굴조사|지표조사)$)"));
  s.replace(reDrop, QStringLiteral(" "));
  s.replace(reTrail, QStringLiteral("\\1"));
  s.replace(reComp, QStringLiteral("\\1 \\2"));
  s.replace(reCity, QStringLiteral("\\1 "));
  s = s.simplified();
  return (s.size() < 3 || reSkip.match(s).hasMatch()) ? QString() : s;
}

}  // namespace

QVector<Card> cardsFromNearby(const QVector<NearbyHeritageExport::Item>& items, int firstNumber) {
  // 이름 없는(무명 라벨) 유적과 행정 규제 구역(현상변경허용기준)은 빼고, 같은 이름은 가장 가까운 하나로 합친다.
  QVector<NearbyHeritageExport::Item> kept;
  QHash<QString, int> indexByName;
  for (const auto& item : items) {
    if (item.datasetName.contains(QStringLiteral("현상변경"))) continue;
    const QString name = item.siteName.trimmed();
    if (name.isEmpty() || name == HeritageStyle::unnamedLabel()) continue;
    const auto found = indexByName.constFind(name);
    if (found == indexByName.constEnd()) {
      indexByName.insert(name, kept.size());
      kept.push_back(item);
    } else if (item.distanceMeters < kept[found.value()].distanceMeters) {
      kept[found.value()] = item;
    }
  }
  QVector<Card> cards;
  int number = firstNumber;
  for (const auto& item : kept) {
    Card c = makeCard(number++, QStringLiteral("nearby_heritage"), item.siteName.trimmed(), QStringLiteral("이격거리"),
                      QString::number(item.distanceMeters), QStringLiteral("m"), QString());
    c.siteName = c.subject, c.datasetName = item.datasetName, c.direction = item.direction;
    c.radiusLabel = item.radiusLabel, c.distanceMeters = item.distanceMeters;
    const QString where =
        item.distanceMeters <= 0
            ? QStringLiteral("조사구역 내 (0m)")
            : QStringLiteral("조사구역에서 %1쪽으로 약 %2m").arg(item.direction).arg(item.distanceMeters);
    c.quote = QStringLiteral("%1(%2): %3 (%4)").arg(c.siteName, item.datasetName, where, item.radiusLabel);
    cards.push_back(c);
  }
  return cards;
}

QStringList queriesFromNearby(const QVector<NearbyHeritageExport::Item>& items, int maxQueries) {
  QVector<NearbyHeritageExport::Item> sorted = items;
  std::stable_sort(sorted.begin(), sorted.end(),
                   [](const auto& a, const auto& b) { return a.distanceMeters < b.distanceMeters; });
  QStringList queries;
  QSet<QString> seen;
  for (const auto& item : sorted) {
    if (item.datasetName.contains(QStringLiteral("현상변경"))) continue;
    const QString raw = item.siteName.trimmed();
    if (raw.isEmpty() || raw == HeritageStyle::unnamedLabel()) continue;
    const QString base = cleanSiteQueryBase(raw);
    if (base.isEmpty() || seen.contains(base)) continue;
    seen.insert(base);
    queries << (base.endsWith(QStringLiteral("조사")) ? base : QStringLiteral("%1 발굴조사").arg(base));
    if (queries.size() >= maxQueries) break;
  }
  return queries;
}

Ledger buildLedger(QgsProject* project, const QString& targetSiteName, double maxRadiusMeters) {
  Ledger ledger;
  ledger.targetSiteName = targetSiteName.trimmed();
  if (!project) {
    ledger.error = QStringLiteral("열린 프로젝트가 없습니다.");
    return ledger;
  }
  try {
    QgsVectorLayer* survey = LayerOps::findByLayerKey(project, QStringLiteral("survey_area"));
    if (!survey || !LayerFeatures::any(survey)) {
      ledger.error = QStringLiteral("조사구역(survey_area) 도형이 없어 실측 원장을 만들 수 없습니다.");
      return ledger;
    }
    ledger.workCrs = survey->crs().authid();
    const QgsGeometry area = surveyUnion(survey);
    ledger.surveyAreaM2 = areaSquareMeters(project, survey, area);
    int number = 1;
    if (ledger.surveyAreaM2 > 0) {
      const QString value = QString::number(ledger.surveyAreaM2, 'f', 1);
      ledger.cards.push_back(
          makeCard(number++, QStringLiteral("survey_area"), QStringLiteral("조사구역"), QStringLiteral("면적"), value,
                   QStringLiteral("㎡"),
                   QStringLiteral("조사구역(survey_area) 실측 면적 %1㎡ (작업 좌표계 %2)").arg(value, ledger.workCrs)));
    }
    const auto nearby = NearbyHeritageExport::collect(project, survey, maxRadiusMeters);
    if (nearby.ok()) {
      const auto cards = cardsFromNearby(nearby.items, number);
      number += cards.size();
      ledger.cards += cards;
      ledger.dbpiaQueries = queriesFromNearby(nearby.items);
    }
    const qint64 nFeat = LayerFeatures::count(LayerOps::findByLayerKey(project, QStringLiteral("feature_poly")));
    const qint64 nArt = LayerFeatures::count(LayerOps::findByLayerKey(project, QStringLiteral("artifact_point")));
    const qint64 nTr = LayerFeatures::count(LayerOps::findByLayerKey(project, QStringLiteral("trial_trench")));
    if (nFeat > 0 || nArt > 0 || nTr > 0) {
      const QString val = QStringLiteral("트렌치 %1개·유구 %2기·유물 %3점").arg(nTr).arg(nFeat).arg(nArt);
      ledger.cards.push_back(makeCard(number++, QStringLiteral("survey_features"), QStringLiteral("조사구역 내부 검출"),
                                      QStringLiteral("검출현황"), val, QString(),
                                      QStringLiteral("조사구역 내부 도메인 레이어 실측 현황: %1").arg(val)));
    }
    // 값 카드가 있는 도메인만 연다. 레이어가 있어도 값 카드가 없으면 잠근다(AI 가 추측해 쓰지 못하게).
    bool soilCard = false;
    QgsVectorLayer* soil = PaleoLandformService::findSoilTerrainLayer(project);
    if (soil && LayerFeatures::any(soil)) {
      const QStringList names = soilTerrainNames(project, soil, survey, area);
      if (!names.isEmpty()) {
        const QString joined = names.join(QStringLiteral("·"));
        ledger.cards.push_back(makeCard(number++, QStringLiteral("soil_terrain"), QStringLiteral("조사구역"),
                                        QStringLiteral("분포지형"), joined, QString(),
                                        QStringLiteral("흙토람 정밀토양도 분포지형(조사구역과 겹침): %1").arg(joined)));
        soilCard = true;
      }
    }
    if (!soilCard) ledger.uncollectedDomains << QStringLiteral("soil");
    // 고지형·DEM(표고·경사)·지질은 아직 값 카드를 만들지 않으므로 항상 잠근다.
    ledger.uncollectedDomains << QStringLiteral("paleo_landform") << QStringLiteral("dem") << QStringLiteral("geology");
  } catch (...) {
    KA_LOG_EXCEPT();
    ledger.error = QStringLiteral("실측 원장을 만드는 중 오류가 났습니다.");
  }
  return ledger;
}

QJsonObject ledgerToJson(const Ledger& ledger) {
  QJsonArray cards;
  for (const Card& c : ledger.cards) {
    QJsonObject o{{"id", c.id},           {"kind", c.kind}, {"subject", c.subject}, {"attribute", c.attribute},
                  {"value", c.value},     {"unit", c.unit}, {"quote", c.quote},     {"confidence", "verified"},
                  {"crs", ledger.workCrs}};
    if (c.kind == QLatin1String("nearby_heritage")) {
      o.insert("siteName", c.siteName);
      o.insert("datasetName", c.datasetName);
      o.insert("direction", c.direction);
      o.insert("radiusLabel", c.radiusLabel);
      o.insert("distanceMeters", c.distanceMeters);
    }
    cards.append(o);
  }
  return QJsonObject{{"schemaVersion", "1.0"},
                     {"workCrs", ledger.workCrs},
                     {"targetSiteName", ledger.targetSiteName},
                     {"surveyAreaM2", ledger.surveyAreaM2},
                     {"cards", cards},
                     {"uncollectedDomains", QJsonArray::fromStringList(ledger.uncollectedDomains)},
                     {"dbpiaQueries", QJsonArray::fromStringList(ledger.dbpiaQueries)}};
}

QStringList allowedActions() {
  return kSpatialActions + QStringList{QStringLiteral("run_checklist")};
}

CommandCheck validateCommand(const QJsonObject& command, QgsProject* project) {
  CommandCheck check;
  const QString action = command.value(QStringLiteral("action")).toString();
  const QJsonObject params = command.value(QStringLiteral("params")).toObject();
  if (!allowedActions().contains(action)) {
    check.reason = QStringLiteral("허용 목록에 없는 명령입니다: %1").arg(action);
    return check;
  }
  const QStringList permitted = allowedParams(action);
  for (auto it = params.constBegin(); it != params.constEnd(); ++it) {
    if (!permitted.contains(it.key())) {
      check.reason = QStringLiteral("%1 명령에 허용되지 않은 값입니다: %2").arg(action, it.key());
      return check;
    }
  }
  if (params.contains(QStringLiteral("radiusMeters"))) {
    const QJsonValue v = params.value(QStringLiteral("radiusMeters"));
    const double r = v.toDouble(-1.0);
    if (!v.isDouble() || r < kMinRadiusMeters || r > kMaxRadiusMeters) {
      check.reason = QStringLiteral("반경은 %1~%2m 숫자만 허용합니다.").arg(kMinRadiusMeters).arg(kMaxRadiusMeters);
      return check;
    }
  }
  if (params.contains(QStringLiteral("targetLayerId"))) {
    QgsMapLayer* target =
        project ? project->mapLayer(params.value(QStringLiteral("targetLayerId")).toString()) : nullptr;
    if (!target || LayerRole::resolve(target) != LayerRole::Kind::Survey) {
      check.reason = QStringLiteral("조사 데이터가 아닌 레이어(참조·지적)는 바꿀 수 없습니다.");
      return check;
    }
  }
  if (kSpatialActions.contains(action)) {
    QgsVectorLayer* survey = project ? LayerOps::findByLayerKey(project, QStringLiteral("survey_area")) : nullptr;
    if (!survey || !LayerFeatures::any(survey)) {
      check.reason = QStringLiteral("조사구역(survey_area) 도형이 없어 공간 명령을 실행할 수 없습니다.");
      return check;
    }
  }
  check.allowed = true;
  return check;
}

}  // namespace GisFactHarness
