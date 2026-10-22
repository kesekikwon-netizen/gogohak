#pragma once

#include <QDialog>
#include <functional>

class QCheckBox;
class QLabel;
class QLineEdit;
class QProgressBar;
class QTextBrowser;

class KaHwpLivePaperWindow : public QDialog {
  Q_OBJECT
public:
  explicit KaHwpLivePaperWindow(QWidget* parent = nullptr);
  ~KaHwpLivePaperWindow() override;

  void updatePaperHtml(const QString& html, const QString& statusText = {}, int progressPercent = 100);
  int zoomPercent() const { return m_zoomPercent; }
  bool showFieldMarks() const;

  void setActionCallbacks(std::function<void(const QString&)> onPromptSubmitted,
                          std::function<void()> onRefreshClicked,
                          std::function<void()> onSaveHwpxClicked,
                          std::function<void()> onSaveHwp5Clicked);

private:
  QLabel* m_statusLabel = nullptr;
  QProgressBar* m_progressBar = nullptr;
  QLabel* m_zoomLabel = nullptr;
  QCheckBox* m_fieldMarkCheck = nullptr;
  QTextBrowser* m_paperBrowser = nullptr;
  QLineEdit* m_livePromptEdit = nullptr;
  int m_zoomPercent = 100;

  std::function<void(const QString&)> m_onPrompt;
  std::function<void()> m_onRefresh;
};
