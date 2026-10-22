#pragma once

#include <QMap>
#include <QString>
#include <QStringList>
#include <QVector>
#include <filesystem>
#include <functional>

#include "NearbyHeritageExport.h"

class QgsProject;

namespace HwpSurveyReportService {

struct DomainLayerCount {
  QString layerKey;
  QString label;
  int featureCount = 0;
};

struct SurveySnapshot {
  QString surveyTitle;
  QString surveyCrs;
  double totalAreaSqm = 0.0;
  int surveyPolygonCount = 0;
  QVector<DomainLayerCount> domainCounts;
  QVector<NearbyHeritageExport::Item> nearbySites;
  QString discussionDraft;
  QString ragCitationSlotText;
};

struct FieldEntry {
  QString name;
  QString value;
};

// RAG 연결 루트(콜백 슬롯): 현재는 RAG 인덱싱 완료 전이므로 기본적으로 호출하지 않고,
// 사용자가 활성화했을 때만 이 슬롯을 통해 원문 인용문을 주입한다.
using RagSlotCallback = std::function<QString(const SurveySnapshot& snapshot)>;

class ReportDocumentSession {
public:
  ReportDocumentSession();
  ~ReportDocumentSession();

  ReportDocumentSession(const ReportDocumentSession&) = delete;
  ReportDocumentSession& operator=(const ReportDocumentSession&) = delete;

  // 현재 GIS 프로젝트에서 조사구역·유구·반경 내 주변유적 현황을 수집한다.
  static SurveySnapshot collectFromProject(QgsProject* project,
                                           bool enableRagSlot = false,
                                           const RagSlotCallback& ragSlot = {});

  // 수집된 GIS 현황으로 표준 발굴조사보고서 한글 문서(표·누름틀 포함)를 생성한다.
  // maxStage (1~4)를 지정하면 단계별로 실시간 조판되는 모습을 보여줄 수 있다.
  bool buildStandardReport(const SurveySnapshot& snapshot,
                           QString* errorOut = nullptr,
                           int maxStage = 4);

  // 기존 한글 양식(.hwp / .hwpx)을 열고 누름틀 필드에 GIS 데이터를 자동으로 채운다.
  bool openTemplateAndFill(const std::filesystem::path& templatePath,
                           const SurveySnapshot& snapshot,
                           QString* errorOut = nullptr);

  // 현재 한글 문서를 .hwpx 또는 .hwp(HWP 5.0) 포맷으로 저장한다.
  bool saveAs(const std::filesystem::path& outputPath,
              const QString& format,
              QString* errorOut = nullptr);

  // 누름틀 필드 목록 및 값 조회/수정
  QVector<FieldEntry> fields() const;
  bool setFieldValue(const QString& fieldName, const QString& value);

  // 사용자의 자연어 작성·수정 요청을 받아 한글 문서(단락·표·누름틀)에 즉시 반영한다.
  bool applyUserRequest(const QString& userPrompt,
                        const SurveySnapshot& snapshot,
                        QString* assistantReplyOut = nullptr);

  // 실제 한글(HWP) A4 용지 조판 미리보기용 HTML 렌더링 (다중 쪽·여백 마크·실시간 하이라이트 지원)
  QString renderPreviewHtml(const QString& liveStageLabel = {},
                            int zoomPercent = 100,
                            bool showFieldMarks = true) const;

private:
  struct Impl;
  Impl* m_impl = nullptr;
};

}  // namespace HwpSurveyReportService
