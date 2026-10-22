#pragma once
// Strata(ka-hgis) <-> DBpia 다중 라운드 /goal 수렴 루프 및 제미나이 브리지 비동기 클라이언트.
#include "core/GisFactHarness.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QObject>
#include <QSet>
#include <QString>
#include <QStringList>

class QNetworkAccessManager;

class KaGeminiBridgeClient : public QObject {
  Q_OBJECT
 public:
  explicit KaGeminiBridgeClient(QObject* parent = nullptr);

  bool isBusy() const { return m_busy; }
  QString lastMarkdownReply() const { return m_lastReply; }
  void setLastMarkdownReply(const QString& reply) { m_lastReply = reply; }
  QJsonArray lastCollectedPapers() const { return m_collectedPapers; }
  QJsonArray lastDbpiaPackets() const { return m_dbpiaPackets; }
  int currentRound() const { return m_roundNumber; }
  void mergeAiReferences(const QJsonArray& aiRefs);
  QString buildPaperGroundedFallbackAnswer(const QString& question, int turnIndex) const;
  QString buildStructuredDataSynthesis(const QJsonArray& dbpiaAiTurns) const;

  void startResearchAndEvaluate(const QString& userRequest, const GisFactHarness::Ledger& ledger,
                                int radiusMeters, const QString& docType, bool autoExportDocx,
                                const QString& mapSnapshotPath = QString());
  void cancelActiveJob();

 signals:
  void debugLogged(const QString& stage, const QString& detail);
  void dbpiaPacketReceived(const QString& query, int refCount, const QString& summaryPreview);
  void roundCompleted(int roundNumber, int newInRound, int totalUniquePapers, bool converged);
  void dbpiaHarvestCompleted(const QJsonArray& dbpiaPackets, int totalUniquePapers);
  void evaluationFinished(const QString& markdownReply, int totalPapers);

 private:
  void initDbpiaDirectSession();
  void runNextDbpiaQuery();
  void finishRoundOrContinue();
  void harvestFollowUpKeywords(const QString& text);
  static QStringList buildCandidatePool(const GisFactHarness::Ledger& ledger, QString* outRegionPrefix);
  static QString formatPlctTypeBreakdown(const QString& raw);

  QNetworkAccessManager* m_nam = nullptr;
  bool m_busy = false;
  quint64 m_jobSeq = 0;

  QString m_userRequest;
  GisFactHarness::Ledger m_ledger;
  int m_radiusMeters = 500;
  QString m_mapSnapshotPath;

  QString m_dbpiaSearchKey;
  QString m_regionPrefix;
  QStringList m_candidatePool;
  QStringList m_discoveredPool;
  QSet<QString> m_executedQueries;
  QStringList m_pendingQueries;
  int m_queryIndex = 0;
  int m_roundNumber = 1;
  int m_newInRound = 0;
  QSet<QString> m_seenNodeIds;
  QJsonArray m_dbpiaPackets;
  QJsonArray m_collectedPapers;
  QString m_lastReply;
};
