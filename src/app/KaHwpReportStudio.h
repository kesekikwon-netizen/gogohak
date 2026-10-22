#pragma once

#include <QDialog>
#include <memory>

class QCheckBox;
class QLabel;
class QMainWindow;
class QTableWidget;
class QTextBrowser;

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

private slots:
  void onOpenTemplateClicked();
  void onSaveHwpxClicked();
  void onSaveHwp5Clicked();
  void onFieldCellChanged(int row, int column);

private:
  void syncUiFromSession();

  std::unique_ptr<HwpSurveyReportService::ReportDocumentSession> m_session;
  QLabel* m_summaryLabel = nullptr;
  QCheckBox* m_ragSlotCheck = nullptr;
  QTableWidget* m_fieldTable = nullptr;
  QTextBrowser* m_previewView = nullptr;
  bool m_updatingTable = false;
};
