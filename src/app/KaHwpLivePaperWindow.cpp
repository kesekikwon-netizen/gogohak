#include "KaHwpLivePaperWindow.h"
#include "KaGeminiBridgeClient.h"
#include "KaHwpReportStudio.h"
#include "KaIcons.h"
#include "KaTheme.h"
#include "core/GisFactHarness.h"
#include "core/LocalReportRag.h"

#include <QtWidgets>
#include <qgsproject.h>

KaHwpLivePaperWindow::KaHwpLivePaperWindow(QWidget* parent)
    : QDialog(parent, Qt::Window | Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint) {
  setWindowTitle(QStringLiteral("Strata · 한글(HWPX / HWP) A4 조판 창"));
  resize(920, 960);

  auto* root = new QVBoxLayout(this);
  root->setContentsMargins(10, 10, 10, 10);
  root->setSpacing(8);

  auto* topBar = new QHBoxLayout();
  m_statusLabel = new QLabel(QStringLiteral("한글(HWPX/HWP) A4 조판 준비됨"), this);
  m_progressBar = new QProgressBar(this);
  m_progressBar->setRange(0, 100);
  m_progressBar->setValue(100);
  m_progressBar->setFixedWidth(130);

  auto* btnZoomOut = new QPushButton(QStringLiteral("축소 -"), this);
  m_zoomLabel = new QLabel(QStringLiteral("100%"), this);
  auto* btnZoomIn = new QPushButton(QStringLiteral("확대 +"), this);
  m_fieldMarkCheck = new QCheckBox(QStringLiteral("누름틀 표시"), this);
  m_fieldMarkCheck->setChecked(true);
  auto* btnSaveHwpx = new QPushButton(QStringLiteral(".hwpx 저장"), this);
  auto* btnSaveHwp5 = new QPushButton(QStringLiteral(".hwp 저장"), this);

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
  root->addWidget(m_paperBrowser, 1);

  auto* bottomBar = new QHBoxLayout();
  m_livePromptEdit = new QLineEdit(this);
  m_livePromptEdit->setPlaceholderText(QStringLiteral("작성·비교·치환 지시를 입력하세요"));
  auto* btnSend = new QPushButton(QStringLiteral("문서 반영"), this);
  btnSend->setObjectName(QStringLiteral("primary"));
  btnSend->setProperty("class", QStringLiteral("primary"));
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
  connect(m_fieldMarkCheck, &QCheckBox::toggled, this, [this](bool) { if (m_onRefresh) m_onRefresh(); });
  auto sendFn = [this] {
    if (!m_livePromptEdit || m_livePromptEdit->text().trimmed().isEmpty()) return;
    const QString txt = m_livePromptEdit->text().trimmed();
    m_livePromptEdit->clear();
    if (m_onPrompt) m_onPrompt(txt);
  };
  connect(btnSend, &QPushButton::clicked, this, sendFn);
  connect(m_livePromptEdit, &QLineEdit::returnPressed, this, sendFn);
  connect(btnSaveHwpx, &QPushButton::clicked, this, [this] { if (auto* p = parentWidget()) QMetaObject::invokeMethod(p, "onSaveHwpxClicked"); });
  connect(btnSaveHwp5, &QPushButton::clicked, this, [this] { if (auto* p = parentWidget()) QMetaObject::invokeMethod(p, "onSaveHwp5Clicked"); });
}

KaHwpLivePaperWindow::~KaHwpLivePaperWindow() = default;
bool KaHwpLivePaperWindow::showFieldMarks() const { return !m_fieldMarkCheck || m_fieldMarkCheck->isChecked(); }

void KaHwpLivePaperWindow::updatePaperHtml(const QString& html, const QString& statusText, int progressPercent) {
  if (m_statusLabel && !statusText.isEmpty()) m_statusLabel->setText(statusText);
  if (m_progressBar) m_progressBar->setValue(qBound(0, progressPercent, 100));
  if (m_paperBrowser) {
    m_paperBrowser->setHtml(html);
    if (progressPercent < 100)
      if (QScrollBar* sb = m_paperBrowser->verticalScrollBar()) sb->setValue(sb->maximum());
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

void KaHwpReportStudio::updateStepRail(int activeStep) {
  if (!m_stepList) return;
  m_stepList->clear();
  const QStringList steps = {QStringLiteral("1. 근거 확정 (GIS 최단거리·방위)"), QStringLiteral("2. 보고서 원문 대조 (RAG)"),
                             QStringLiteral("3. 학술문헌 수렴 (DBpia)"),          QStringLiteral("4. 사실·인용 교차검증 (G·R·L)"),
                             QStringLiteral("5. 본문·각주·연관성 집필"),          QStringLiteral("6. 층위 대비도·비교표 조판")};
  for (int i = 0; i < steps.size(); ++i)
    m_stepList->addItem(((i + 1 < activeStep) ? QStringLiteral("✓ ") : ((i + 1 == activeStep) ? QStringLiteral("• ") : QStringLiteral("○ "))) + steps[i]);
}

void KaHwpReportStudio::populateEvidenceTable() {
  if (!m_evidenceTable) return;
  m_evidenceTable->setRowCount(m_cachedSnap.siteEntries.size());
  for (int i = 0; i < m_cachedSnap.siteEntries.size(); ++i) {
    const auto& e = m_cachedSnap.siteEntries[i];
    const QString rptBadge = e.reportStatus == QStringLiteral("✓") ? QStringLiteral("✓ p.%1").arg(e.pageNo)
                             : (e.reportStatus == QStringLiteral("◌") ? QStringLiteral("◌ 스캔") : QStringLiteral("○ 없음"));
    const QStringList cols = {QString::number(i + 1), e.siteName, QStringLiteral("%1m").arg(e.distanceMeters),
                              e.direction, e.eraStatus, e.featureStatus, e.strataStatus, rptBadge};
    for (int c = 0; c < cols.size(); ++c) {
      auto* item = new QTableWidgetItem(cols[c]);
      if (c != 1) item->setTextAlignment(Qt::AlignCenter);
      m_evidenceTable->setItem(i, c, item);
    }
  }
  if (!m_cachedSnap.siteEntries.isEmpty()) m_evidenceTable->selectRow(0);
  else onSiteRowSelected(-1);
}

void KaHwpReportStudio::onSiteRowSelected(int row) {
  if (!m_siteDetailView) return;
  if (row < 0 || row >= m_cachedSnap.siteEntries.size()) {
    m_siteDetailView->setHtml(QStringLiteral("<div style='color:#6E6A63;font-size:8.8pt;'>지도에 주변유적을 올리거나 행을 선택하면 원문 발췌와 <b>⑤ 조사구역과의 연관성</b>이 표시됩니다.</div>"));
    return;
  }
  const auto& e = m_cachedSnap.siteEntries[row];
  m_siteDetailView->setHtml(QStringLiteral(
      "<div style='font-size:8.6pt;line-height:1.55;color:#1F1E1B;'>"
      "<b>[%1]</b> 조사구역 경계 기준 <b>%2 %3m</b> · 보고서: <b>%4</b><br/>"
      "• <b>층위·토층(③):</b> %5<br/>• <b>유구·시대(④):</b> [%6] %7<br/>"
      "• <b style='color:#B5573A;'>⑤ 조사구역과의 연관성:</b> %8</div>")
      .arg(e.siteName.toHtmlEscaped(), e.direction.toHtmlEscaped()).arg(e.distanceMeters)
      .arg(e.reportName.isEmpty() ? QStringLiteral("원문 없음(○)") : QStringLiteral("%1 (%2)").arg(e.reportName.toHtmlEscaped(), e.reportStatus))
      .arg(e.strataSummary.toHtmlEscaped(), e.eraSummary.toHtmlEscaped(), e.featureSummary.toHtmlEscaped(), e.relationToSurvey.toHtmlEscaped()));
}

void KaHwpReportStudio::refreshFromActiveProject(bool forceReload) {
  if (!m_hasCachedSnap || forceReload) {
    m_cachedSnap = HwpSurveyReportService::ReportDocumentSession::collectFromProject(
        QgsProject::instance(), m_ragSlotCheck && m_ragSlotCheck->isChecked(), {}, m_radiusMeters);
    if (m_regionCombo && !m_regionCombo->currentData().toString().isEmpty()) {
      m_cachedSnap.regionName = m_regionCombo->currentData().toString();
      const auto rs = strata::core::LocalReportRag::queryRegionStats(m_cachedSnap.regionName.toStdString());
      m_cachedSnap.regionTotalReports = rs.total_reports, m_cachedSnap.regionTextReports = rs.text_reports, m_cachedSnap.regionScanReports = rs.scan_reports;
    }
    m_hasCachedSnap = true;
  }
  if (m_compareView) m_compareView->clear();
  m_session->buildStandardReport(m_cachedSnap, nullptr, 4);
  m_summaryLabel->setText(QStringLiteral("조사명: %1 · 면적 %2㎡ (%3) · 반경 %4m 주변유적 %5개소")
                              .arg(m_cachedSnap.surveyTitle).arg(qRound(m_cachedSnap.totalAreaSqm))
                              .arg(m_cachedSnap.surveyCrs).arg(qRound(m_radiusMeters)).arg(m_cachedSnap.siteEntries.size()));
  m_connStatusLabel->setText(QStringLiteral("연결 상태 — 발굴조사보고서 RAG(%1): 총 %2권 (본문 %3권 · 스캔 %4권) | DBpia 문헌: %5편")
                                 .arg(m_cachedSnap.regionName.isEmpty() ? QStringLiteral("경주시") : m_cachedSnap.regionName)
                                 .arg(m_cachedSnap.regionTotalReports).arg(m_cachedSnap.regionTextReports)
                                 .arg(m_cachedSnap.regionScanReports).arg(m_dbpiaSummary.collectedPapers.size()));
  populateEvidenceTable();
  syncUiFromSession();
}

void KaHwpReportStudio::startDbpiaResearch(const QString& customTopic) {
  GisFactHarness::Ledger ledger = GisFactHarness::buildLedger(QgsProject::instance());
  if (!customTopic.trimmed().isEmpty()) ledger.dbpiaQueries.prepend(customTopic.trimmed());
  if (ledger.dbpiaQueries.isEmpty()) ledger.dbpiaQueries = {m_cachedSnap.regionName + QStringLiteral(" 발굴조사 유구 편년")};
  updateStepRail(3);
  m_dbpiaBridge->startResearchAndEvaluate(customTopic, ledger, qRound(m_radiusMeters), m_isThesisMode ? QStringLiteral("thesis") : QStringLiteral("report"), false);
}

void KaHwpReportStudio::openLivePaperWindow() {
  if (!m_liveWindow) {
    m_liveWindow = new KaHwpLivePaperWindow(this);
    m_liveWindow->setActionCallbacks([this](const QString& txt) { submitUserPrompt(txt); }, [this] { syncUiFromSession(); },
                                     [this] { onSaveHwpxClicked(); }, [this] { onSaveHwp5Clicked(); });
  }
  m_liveWindow->show(); m_liveWindow->raise(); m_liveWindow->activateWindow();
  syncUiFromSession();
}

void KaHwpReportStudio::startLiveOrganizeDemo() { refreshFromActiveProject(true); m_currentStage = 0; m_stageTimer->start(); }

void KaHwpReportStudio::appendChatMessage(const QString& role, const QString& text) {
  if (m_chatView)
    m_chatView->append(QStringLiteral("<div style='margin:3px 0;font-size:8.6pt;color:#1F1E1B;'><b>[%1]</b> %2</div>")
                           .arg(role.toHtmlEscaped(), text.toHtmlEscaped().replace(QLatin1Char('\n'), QStringLiteral("<br/>"))));
}

void KaHwpReportStudio::onSendPromptClicked() {
  const QString text = m_promptEdit ? m_promptEdit->text().trimmed() : QString();
  if (text.isEmpty()) { startLiveOrganizeDemo(); return; }
  m_promptEdit->clear();
  submitUserPrompt(text);
}

void KaHwpReportStudio::submitUserPrompt(const QString& promptText) {
  const QString req = promptText.trimmed();
  if (req.isEmpty()) return;
  appendChatMessage(QStringLiteral("요청"), req);
  QString reply;
  if (m_session->applyUserRequest(req, m_cachedSnap, &reply)) appendChatMessage(QStringLiteral("반영"), reply);
  if (m_compareView) m_compareView->setHtml(HwpAcademicVisuals::renderFeatureComparisonHtml(m_cachedSnap, req));
  if (req.contains(QStringLiteral("비교")) || req.contains(QStringLiteral("현황")) || req.contains(QStringLiteral("분포")) || req.contains(QStringLiteral("변화")))
    m_centerStack->setCurrentWidget(m_compareView);
  if (req.contains(QStringLiteral("논문")) || req.contains(QStringLiteral("DBpia"), Qt::CaseInsensitive)) startDbpiaResearch(req);
  else syncUiFromSession(QStringLiteral("요청 반영 완료: ") + req, 100);
}

void KaHwpReportStudio::onOpenTemplateClicked() {
  const QString path = QFileDialog::getOpenFileName(this, QStringLiteral("한글 보고서 양식 열기"), QDir::homePath(), QStringLiteral("한글 문서 (*.hwpx *.hwp)"));
  QString err;
  if (!path.isEmpty() && !m_session->openTemplateAndFill(std::filesystem::path(path.toStdString()), m_cachedSnap, &err)) QMessageBox::warning(this, QStringLiteral("양식 열기 실패"), err);
  else if (!path.isEmpty()) syncUiFromSession(QStringLiteral("기관 한글 양식 누름틀 자동 기입 완료"), 100);
}

void KaHwpReportStudio::onSaveHwpxClicked() {
  const QString path = QFileDialog::getSaveFileName(this, QStringLiteral("한글 표준(.hwpx)으로 저장"), QDir::home().filePath(QStringLiteral("발굴조사보고서.hwpx")), QStringLiteral("한글 표준 문서 (*.hwpx)"));
  QString err;
  if (!path.isEmpty() && !m_session->saveAs(std::filesystem::path(path.toStdString()), QStringLiteral("HWPX"), &err)) QMessageBox::warning(this, QStringLiteral("저장 실패"), err);
}

void KaHwpReportStudio::onSaveHwp5Clicked() {
  const QString path = QFileDialog::getSaveFileName(this, QStringLiteral("한글 5.0(.hwp)으로 저장"), QDir::home().filePath(QStringLiteral("발굴조사보고서.hwp")), QStringLiteral("한글 5.0 문서 (*.hwp)"));
  QString err;
  if (!path.isEmpty() && !m_session->saveAs(std::filesystem::path(path.toStdString()), QStringLiteral("HWP"), &err)) QMessageBox::warning(this, QStringLiteral("저장 실패"), err);
}

void KaHwpReportStudio::onFieldCellChanged(int row, int column) {
  if (m_updatingTable || column != 1) return;
  QTableWidgetItem *nameItem = m_fieldTable->item(row, 0), *valItem = m_fieldTable->item(row, 1);
  if (nameItem && valItem && m_session->setFieldValue(nameItem->text(), valItem->text()))
    syncUiFromSession(QStringLiteral("누름틀 갱신: ") + nameItem->text(), 100);
}

void KaHwpReportStudio::syncUiFromSession(const QString& liveStageLabel, int progressPercent) {
  m_updatingTable = true;
  const auto list = m_session->fields();
  m_fieldTable->setRowCount(list.size());
  for (int i = 0; i < list.size(); ++i) {
    auto* kItem = new QTableWidgetItem(list[i].name);
    kItem->setFlags(kItem->flags() & ~Qt::ItemIsEditable);
    m_fieldTable->setItem(i, 0, kItem);
    m_fieldTable->setItem(i, 1, new QTableWidgetItem(list[i].value));
  }
  m_updatingTable = false;
  const int zoom = (m_zoomCombo && m_zoomCombo->currentIndex() == 0) ? 90 : ((m_zoomCombo && m_zoomCombo->currentIndex() == 2) ? 120 : 100);
  const QString visualsHtml = HwpAcademicVisuals::renderVisualsAndDbpiaSectionHtml(m_cachedSnap, m_dbpiaSummary);
  const QString html = m_session->renderPreviewHtml(liveStageLabel, zoom, true, visualsHtml);
  m_previewView->setHtml(html);
  if (m_verifyView) m_verifyView->setHtml(HwpAcademicVisuals::renderVerificationReportHtml(m_cachedSnap, m_dbpiaSummary));
  if (m_compareView && m_compareView->toPlainText().isEmpty())
    m_compareView->setHtml(HwpAcademicVisuals::renderFeatureComparisonHtml(m_cachedSnap));
  if (m_liveWindow) m_liveWindow->updatePaperHtml(html, liveStageLabel, progressPercent);
}

void KaHwpReportStudio::attach(QMainWindow* mainWindow) {
  if (!mainWindow) return;
  auto* tabs = mainWindow->findChild<QTabWidget*>(QStringLiteral("viewTabs"));
  auto* studioPage = new KaHwpReportStudio(tabs ? static_cast<QWidget*>(tabs) : mainWindow);
  studioPage->setObjectName(QStringLiteral("kaHwpReportStudio"));
  studioPage->setWindowFlags(Qt::Widget);
  studioPage->hide();
  auto openStudio = [tabs, studioPage]() {
    if (!tabs) return;
    if (tabs->indexOf(studioPage) < 0)
      tabs->addTab(studioPage, KaIcons::icon(QStringLiteral("note"), KaTheme::tokens().inkMuted), QStringLiteral("보고서"));
    studioPage->show();
    tabs->setCurrentWidget(studioPage);
    studioPage->refreshFromActiveProject(true);
  };
  auto* actReport = new QAction(KaIcons::icon(QStringLiteral("note")), QStringLiteral("보고서 작성 (Ctrl+G)"), mainWindow);
  actReport->setObjectName(QStringLiteral("actionReportStudio"));
  actReport->setShortcut(QKeySequence(QStringLiteral("Ctrl+G")));
  actReport->setShortcutContext(Qt::WindowShortcut);
  QObject::connect(actReport, &QAction::triggered, mainWindow, openStudio);
  mainWindow->addAction(actReport);
  for (auto* btn : mainWindow->findChildren<QToolButton*>())
    if (btn->text() == QStringLiteral("더보기") && btn->menu()) btn->menu()->addAction(actReport);
}
