#pragma once
// AI(제미나이) 하네스용 GIS 실측 원장(G 카드)과 AI 지도 명령 허용 목록.
// - 원장은 이 앱이 실제로 계산·보유한 값만 담는다. 없는 자료는 uncollectedDomains 로 잠근다.
// - AI 명령은 허용 목록 액션과 허용 범위 파라미터만 통과시킨다. 참조·지적 레이어 변형은 거부한다.
// 지도·레이어를 바꾸지 않는다(읽기 전용).
#include "NearbyHeritageExport.h"

#include <QJsonObject>
#include <QString>
#include <QStringList>
#include <QVector>

class QgsProject;

namespace GisFactHarness {

struct Card {
  QString id;           // "G1", "G2", ...
  QString kind;         // "survey_area" | "nearby_heritage" | "soil_terrain"
  QString subject;      // "조사구역" 또는 유적명
  QString siteName;     // 주변유적이면 유적명
  QString datasetName;  // 예: "매장유산유존지역"
  QString direction;    // 8방위 또는 "조사구역 내"
  QString radiusLabel;  // "500m 이내" 등
  QString attribute;    // "면적", "이격거리", "분포지형"
  QString value;        // 문자열로 고정(반올림 포함)
  QString unit;         // "㎡", "m", ""
  int distanceMeters = -1;
  QString quote;  // 사람이 읽는 실측 문장
};

struct Ledger {
  QString workCrs;
  QString targetSiteName;
  double surveyAreaM2 = -1.0;
  QVector<Card> cards;
  QStringList uncollectedDomains;  // 값 카드가 없는 도메인: "soil", "paleo_landform", "dem", "geology"
  QStringList dbpiaQueries;
  QString error;
  bool ok() const { return error.isEmpty(); }
};

struct CommandCheck {
  bool allowed = false;
  QString reason;
};

// 주변유적 수집 결과를 G 카드로 바꾼다. firstNumber 는 첫 카드 번호(G<n>).
QVector<Card> cardsFromNearby(const QVector<NearbyHeritageExport::Item>& items, int firstNumber);

// 주변유적 실명으로 결정적 학술 검색어를 만든다(가까운 순, 중복 제거). AI 를 쓰지 않는다.
QStringList queriesFromNearby(const QVector<NearbyHeritageExport::Item>& items, int maxQueries = 8);

// 열린 프로젝트에서 원장을 만든다. 조사구역이 없으면 error 를 채운다.
// maxRadiusMeters 가 0 이면 지도에 그려진 주변 경계(없으면 1000m)를 쓴다(NearbyHeritageExport::collect).
Ledger buildLedger(QgsProject* project, const QString& targetSiteName = QString(), double maxRadiusMeters = 0.0);

QJsonObject ledgerToJson(const Ledger& ledger);

// 허용 액션 이름 목록.
QStringList allowedActions();

// AI 명령 {"action": "...", "params": {...}} 검사. 지도를 바꾸지 않는다.
CommandCheck validateCommand(const QJsonObject& command, QgsProject* project);

// 반경 허용 범위(m). 사람 검토 대상 기본값.
constexpr double kMinRadiusMeters = 100.0;
constexpr double kMaxRadiusMeters = 5000.0;

}  // namespace GisFactHarness
