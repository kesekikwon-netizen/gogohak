#pragma once

#include "core/HwpAcademicVisuals.h"

#include <QDialog>
#include <memory>

class KaGeminiBridgeClient;
class KaHwpLivePaperWindow;
class QCheckBox;
class QLabel;
class QLineEdit;
class QMainWindow;
class QTableWidget;
class QTextBrowser;
class QTimer;

namespace HwpSurveyReportService {
class ReportDocumentSession;
}

class KaHwpReportStudio : public QDialog {
  Q_OBJECT
public:
  explicit KaHwpReportStudio(QWidget* parent = nullptr);
  ~KaHwpReportStudio() override;

  static void attach(QMainWindow* mainWindow);

  void refreshFromActiveProject();
  void startLiveOrganizeDemo();
  void startDbpiaResearch(const QString& customTopic = {});
  void openLivePaperWindow();
  void submitUserPrompt(const QString& promptText);

public slots:
  void onOpenTemplateClicked();
  void onSaveHwpxClicked();
  void onSaveHwp5Clicked();
  void onFieldCellChanged(int row, int column);
  void onSendPromptClicked();

private:
  void syncUiFromSession(const QString& liveStageLabel = {}, int progressPercent = 100);
  void appendChatMessage(const QString& role, const QString& text);

  std::unique_ptr<HwpSurveyReportService::ReportDocumentSession> m_session;
  KaGeminiBridgeClient* m_dbpiaBridge = nullptr;
  HwpAcademicVisuals::DbpiaSummary m_dbpiaSummary;
  KaHwpLivePaperWindow* m_liveWindow = nullptr;
  QTimer* m_stageTimer = nullptr;
  int m_currentStage = 4;
  QLabel* m_summaryLabel = nullptr;
  QCheckBox* m_ragSlotCheck = nullptr;
  QTextBrowser* m_chatView = nullptr;
  QLineEdit* m_promptEdit = nullptr;
  QTableWidget* m_fieldTable = nullptr;
  QTextBrowser* m_previewView = nullptr;
  bool m_updatingTable = false;
};
