#pragma once

#include "HwpSurveyReportService.h"

#include <QJsonArray>
#include <QString>

namespace HwpAcademicVisuals {

struct DbpiaSummary {
  QString lastQuery;
  int roundNumber = 0;
  bool converged = false;
  QJsonArray collectedPapers;
  QJsonArray dbpiaAiTurns;
  QString synthesizedDiscussion;
};

// A4 용지 하단에 들어가는 층위 대비도(<그림 1>), 최단거리 분포 그래프(<그림 2>),
// 학술문헌 및 각주 목록 HTML을 생성한다 (KaTheme 종이·먹·황토 톤 준수).
QString renderVisualsAndDbpiaSectionHtml(const HwpSurveyReportService::SurveySnapshot& snap,
                                         const DbpiaSummary& dbpia);

// 가운데 영역 [검수] 보기용: 문장·항목별 사실근거(G·R·L) 및 미확인(○/◌) 검수 보고서 HTML
QString renderVerificationReportHtml(const HwpSurveyReportService::SurveySnapshot& snap,
                                     const DbpiaSummary& dbpia);

// 가운데 영역 [비교] 보기용: 조사구역 내 유구 집계 및 지역(경주시 등) 동종 유구 RAG 비교표·도표 HTML
QString renderFeatureComparisonHtml(const HwpSurveyReportService::SurveySnapshot& snap,
                                    const QString& featureKeyword = {});

// DBpia 수집 문헌 및 시각 요약문을 한글(.hwpx / .hwp) 문서 본문·누름틀에 주입한다.
void appendDbpiaAndVisualSummaryToDoc(const HwpSurveyReportService::SurveySnapshot& snap,
                                      const DbpiaSummary& dbpia,
                                      HwpSurveyReportService::ReportDocumentSession* session);

}  // namespace HwpAcademicVisuals
