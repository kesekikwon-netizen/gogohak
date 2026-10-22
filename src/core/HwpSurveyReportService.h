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

struct SiteEvidenceRow {
  QString datasetName;
  QString siteName;
  int distanceMeters = 0;
  QString direction;
  QString eraStatus = QStringLiteral("○");
  QString featureStatus = QStringLiteral("○");
  QString strataStatus = QStringLiteral("○");
  QString reportStatus = QStringLiteral("○");  // "✓" | "◌" | "○"
  QString reportName;
  int pageNo = 0;
  int strataPageNo = 0;
  QString eraSummary;
  QString featureSummary;
  QString strataSummary;
  QString relationToSurvey;  // ⑤ 조사구역과의 연관성
};

struct SurveySnapshot {
  QString surveyTitle;
  QString surveyCrs;
  QString regionName;
  double radiusMeters = 500.0;
  double totalAreaSqm = 0.0;
  int surveyPolygonCount = 0;
  int regionTotalReports = 0;
  int regionTextReports = 0;
  int regionScanReports = 0;
  QVector<DomainLayerCount> domainCounts;
  QVector<NearbyHeritageExport::Item> nearbySites;
  QVector<SiteEvidenceRow> siteEntries;
  QString discussionDraft;
  QString ragCitationSlotText;
};

struct FieldEntry {
  QString name;
  QString value;
};

using RagSlotCallback = std::function<QString(const SurveySnapshot& snapshot)>;

class ReportDocumentSession {
 public:
  ReportDocumentSession();
  ~ReportDocumentSession();

  ReportDocumentSession(const ReportDocumentSession&) = delete;
  ReportDocumentSession& operator=(const ReportDocumentSession&) = delete;

  static SurveySnapshot collectFromProject(QgsProject* project,
                                           bool enableRagSlot = false,
                                           const RagSlotCallback& ragSlot = {},
                                           double radiusMeters = 500.0);

  // 주변유적 목록(nearbySites)과 조사지역명(regionName)에 대해 로컬 발굴조사보고서 RAG 증거(✓/◌/○)와
  // ⑤ 조사구역과의 연관성을 채운다.
  static void populateRagEvidence(SurveySnapshot* snapshot, bool queryRagDb = true);

  bool buildStandardReport(const SurveySnapshot& snapshot,
                           QString* errorOut = nullptr,
                           int maxStage = 4);

  bool openTemplateAndFill(const std::filesystem::path& templatePath,
                           const SurveySnapshot& snapshot,
                           QString* errorOut = nullptr);

  bool saveAs(const std::filesystem::path& outputPath,
              const QString& format,
              QString* errorOut = nullptr);

  QVector<FieldEntry> fields() const;
  bool setFieldValue(const QString& fieldName, const QString& value);

  bool applyUserRequest(const QString& userPrompt,
                        const SurveySnapshot& snapshot,
                        QString* assistantReplyOut = nullptr);

  QString renderPreviewHtml(const QString& liveStageLabel = {},
                            int zoomPercent = 100,
                            bool showFieldMarks = true,
                            const QString& extraVisualsHtml = {}) const;

 private:
  struct Impl;
  Impl* m_impl = nullptr;
};

}  // namespace HwpSurveyReportService
