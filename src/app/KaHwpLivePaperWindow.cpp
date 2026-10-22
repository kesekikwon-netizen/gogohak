#include "KaHwpLivePaperWindow.h"

#include <QCheckBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QProgressBar>
#include <QPushButton>
#include <QScrollBar>
#include <QTextBrowser>
#include <QVBoxLayout>

KaHwpLivePaperWindow::KaHwpLivePaperWindow(QWidget* parent)
    : QDialog(parent, Qt::Window | Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint) {
  setWindowTitle(QStringLiteral("한글(HWP / HWPX) 실시간 A4 조판 창 — OpenHwpSDK Live Viewer"));
  resize(920, 980);

  auto* root = new QVBoxLayout(this);
  root->setContentsMargins(10, 10, 10, 10);
  root->setSpacing(8);

  auto* topBar = new QHBoxLayout();
  m_statusLabel = new QLabel(QStringLiteral("실시간 한글(HWP) A4 조판 대기 중"), this);
  m_statusLabel->setStyleSheet(
      QStringLiteral("font-weight:700;color:#0f766e;background:#f0fdfa;border:1px solid #99f6e4;border-radius:6px;padding:6px 10px;"));
  m_progressBar = new QProgressBar(this);
  m_progressBar->setRange(0, 100);
  m_progressBar->setValue(100);
  m_progressBar->setFixedWidth(130);

  auto* btnZoomOut = new QPushButton(QStringLiteral("축소 -"), this);
  m_zoomLabel = new QLabel(QStringLiteral("100%"), this);
  m_zoomLabel->setStyleSheet(QStringLiteral("font-weight:700;padding:0 6px;"));
  auto* btnZoomIn = new QPushButton(QStringLiteral("확대 +"), this);
  m_fieldMarkCheck = new QCheckBox(QStringLiteral("누름틀 꺾쇠 표시"), this);
  m_fieldMarkCheck->setChecked(true);

  auto* btnSaveHwpx = new QPushButton(QStringLiteral(".hwpx 저장"), this);
  btnSaveHwpx->setStyleSheet(QStringLiteral("QPushButton{background:#1d4ed8;color:#fff;font-weight:600;padding:5px 10px;border-radius:5px;}"));
  auto* btnSaveHwp5 = new QPushButton(QStringLiteral(".hwp 저장"), this);
  btnSaveHwp5->setStyleSheet(QStringLiteral("QPushButton{background:#0f766e;color:#fff;font-weight:600;padding:5px 10px;border-radius:5px;}"));

  topBar->addWidget(m_statusLabel, 1);
  topBar->addWidget(m_progressBar);
  topBar->addWidget(btnZoomOut);
  topBar->addWidget(m_zoomLabel);
  topBar->addWidget(btnZoomIn);
  topBar->addWidget(m_fieldMarkCheck);
  topBar->addWidget(btnSaveHwpx);
  topBar->addWidget(btnSaveHwp5);
  root->addLayout(topBar);

  m_paperBrowser = new QTextBrowser(this);
  m_paperBrowser->setOpenExternalLinks(false);
  m_paperBrowser->setStyleSheet(QStringLiteral("background:#525659;border:1px solid #334155;border-radius:6px;"));
  root->addWidget(m_paperBrowser, 1);

  auto* bottomBar = new QHBoxLayout();
  m_livePromptEdit = new QLineEdit(this);
  m_livePromptEdit->setPlaceholderText(
      QStringLiteral("이 별도 창에서 바로 작성·수정 지시를 입력하세요 (예: 입지 및 고고학적 배경 작성해줘 / '시굴'을 '정밀발굴'로 바꿔줘)"));
  m_livePromptEdit->setStyleSheet(
      QStringLiteral("QLineEdit{padding:8px 10px;border:2px solid #0f766e;border-radius:6px;font-size:10pt;}"));
  auto* btnSend = new QPushButton(QStringLiteral("실시간 문서 정리·반영"), this);
  btnSend->setStyleSheet(
      QStringLiteral("QPushButton{background:#0f766e;color:#fff;font-weight:700;padding:8px 14px;border-radius:6px;}"));
  bottomBar->addWidget(m_livePromptEdit, 1);
  bottomBar->addWidget(btnSend);
  root->addLayout(bottomBar);

  connect(btnZoomOut, &QPushButton::clicked, this, [this] {
    m_zoomPercent = qMax(70, m_zoomPercent - 15);
    m_zoomLabel->setText(QStringLiteral("%1%").arg(m_zoomPercent));
    if (m_onRefresh) m_onRefresh();
  });
  connect(btnZoomIn, &QPushButton::clicked, this, [this] {
    m_zoomPercent = qMin(160, m_zoomPercent + 15);
    m_zoomLabel->setText(QStringLiteral("%1%").arg(m_zoomPercent));
    if (m_onRefresh) m_onRefresh();
  });
  connect(m_fieldMarkCheck, &QCheckBox::toggled, this, [this](bool) {
    if (m_onRefresh) m_onRefresh();
  });
  auto sendFn = [this] {
    if (!m_livePromptEdit || m_livePromptEdit->text().trimmed().isEmpty()) return;
    const QString txt = m_livePromptEdit->text().trimmed();
    m_livePromptEdit->clear();
    if (m_onPrompt) m_onPrompt(txt);
  };
  connect(btnSend, &QPushButton::clicked, this, sendFn);
  connect(m_livePromptEdit, &QLineEdit::returnPressed, this, sendFn);
  connect(btnSaveHwpx, &QPushButton::clicked, this, [this] {
    if (auto* p = parentWidget()) QMetaObject::invokeMethod(p, "onSaveHwpxClicked");
  });
  connect(btnSaveHwp5, &QPushButton::clicked, this, [this] {
    if (auto* p = parentWidget()) QMetaObject::invokeMethod(p, "onSaveHwp5Clicked");
  });
}

KaHwpLivePaperWindow::~KaHwpLivePaperWindow() = default;

bool KaHwpLivePaperWindow::showFieldMarks() const {
  return !m_fieldMarkCheck || m_fieldMarkCheck->isChecked();
}

void KaHwpLivePaperWindow::updatePaperHtml(const QString& html, const QString& statusText, int progressPercent) {
  if (m_statusLabel && !statusText.isEmpty()) m_statusLabel->setText(statusText);
  if (m_progressBar) m_progressBar->setValue(qBound(0, progressPercent, 100));
  if (m_paperBrowser) {
    m_paperBrowser->setHtml(html);
    if (progressPercent < 100) {
      if (QScrollBar* sb = m_paperBrowser->verticalScrollBar()) sb->setValue(sb->maximum());
    }
  }
}

void KaHwpLivePaperWindow::setActionCallbacks(std::function<void(const QString&)> onPromptSubmitted,
                                              std::function<void()> onRefreshClicked,
                                              std::function<void()> onSaveHwpxClicked,
                                              std::function<void()> onSaveHwp5Clicked) {
  Q_UNUSED(onSaveHwpxClicked);
  Q_UNUSED(onSaveHwp5Clicked);
  m_onPrompt = std::move(onPromptSubmitted);
  m_onRefresh = std::move(onRefreshClicked);
}
