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

// 발굴조사보고서 고찰 및 학술논문용 전문 모식도(Schematic Diagram)·차트(Chart)·그래프(Graph)
// 및 DBpia 학술문헌·DBpia AI 교차검증 섹션 HTML을 생성한다 (QTextBrowser/HWP A4 뷰어 100% 호환).
QString renderVisualsAndDbpiaSectionHtml(const HwpSurveyReportService::SurveySnapshot& snap,
                                         const DbpiaSummary& dbpia);

// DBpia 수집 문헌 및 DBpia AI 응답을 한글(.hwpx / .hwp) 문서 본문·표·누름틀에 주입한다.
void appendDbpiaAndVisualSummaryToDoc(const HwpSurveyReportService::SurveySnapshot& snap,
                                      const DbpiaSummary& dbpia,
                                      HwpSurveyReportService::ReportDocumentSession* session);

}  // namespace HwpAcademicVisuals
