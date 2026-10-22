#include "KaHwpReportStudio.h"
#include "KaGeminiBridgeClient.h"
#include "KaHwpLivePaperWindow.h"
#include "core/GisFactHarness.h"
#include "core/HwpSurveyReportService.h"
#include "core/LocalReportRag.h"
#include <QtWidgets>
#include <qgsproject.h>

KaHwpReportStudio::KaHwpReportStudio(QWidget* parent)
    : QDialog(parent),
      m_session(std::make_unique<HwpSurveyReportService::ReportDocumentSession>()),
      m_dbpiaBridge(new KaGeminiBridgeClient(this)) {
  setWindowTitle(QStringLiteral("Strata · 한글 발굴조사보고서·논문 스튜디오 (OpenHwpSDK × DBpia AI)"));
  resize(1200, 800);
  auto* root = new QVBoxLayout(this);
  root->setContentsMargins(14, 14, 14, 14);
  root->setSpacing(10);

  auto* topBar = new QHBoxLayout();
  auto makeBtn = [this](const QString& txt, const QString& bg) {
    auto* b = new QPushButton(txt, this);
    b->setStyleSheet(QStringLiteral("QPushButton{background:%1;color:#fff;font-weight:700;padding:6px 12px;border-radius:6px;}").arg(bg));
    return b;
  };
  auto* btnLiveWin = makeBtn(QStringLiteral("별도 한글(HWP) 실시간 조판 창 띄우기"), QStringLiteral("#7c3aed"));
  auto* btnDbpia = makeBtn(QStringLiteral("DBpia AI 학술논문 수집 · 모식도/차트 생성"), QStringLiteral("#be123c"));
  auto* btnRefresh = makeBtn(QStringLiteral("실시간 단계별 자동 정리"), QStringLiteral("#0284c7"));
  auto* btnOpenTpl = new QPushButton(QStringLiteral("한글 양식 열기"), this);
  auto* btnSaveHwpx = makeBtn(QStringLiteral(".hwpx 저장"), QStringLiteral("#1d4ed8"));
  auto* btnSaveHwp5 = makeBtn(QStringLiteral(".hwp 저장"), QStringLiteral("#0f766e"));
  m_ragSlotCheck = new QCheckBox(QStringLiteral("발굴보고서 RAG 슬롯"), this);
  for (QWidget* w : {static_cast<QWidget*>(btnLiveWin), static_cast<QWidget*>(btnDbpia), static_cast<QWidget*>(btnRefresh),
                     static_cast<QWidget*>(btnOpenTpl), static_cast<QWidget*>(m_ragSlotCheck)})
    topBar->addWidget(w);
  topBar->addStretch(1);
  topBar->addWidget(btnSaveHwpx);
  topBar->addWidget(btnSaveHwp5);
  root->addLayout(topBar);

  m_summaryLabel = new QLabel(QStringLiteral("GIS 프로젝트 및 DBpia AI 연동 준비 완료"), this);
  m_summaryLabel->setStyleSheet(QStringLiteral("color:#334155;background:#f8fafc;border:1px solid #cbd5e1;border-radius:6px;padding:8px;"));
  root->addWidget(m_summaryLabel);

  auto* split = new QSplitter(Qt::Horizontal, this);
  auto* leftPane = new QWidget(split);
  auto* leftLayout = new QVBoxLayout(leftPane);
  leftLayout->setContentsMargins(0, 0, 6, 0);
  leftLayout->setSpacing(8);

  auto* reqTitle = new QLabel(QStringLiteral("1. 발굴보고서 고찰 · 학술논문 요청 (DBpia AI + 모식도·차트·그래프 실시간 반영)"), leftPane);
  reqTitle->setStyleSheet(QStringLiteral("font-weight:700;font-size:10.5pt;color:#0f172a;"));
  leftLayout->addWidget(reqTitle);

  auto* quickBar = new QHBoxLayout();
  for (const QString& qp : {QStringLiteral("경주 황남동 신라 고분군 DBpia 논문 검색 및 모식도·차트 고찰 작성"),
                            QStringLiteral("입지 및 주변유적 공간위계 모식도·분포 그래프 분석 추가"),
                            QStringLiteral("출토유물 편년 및 학술논문 종합 결론 보강")}) {
    auto* chip = new QPushButton(qp.left(15) + QStringLiteral("…"), leftPane);
    chip->setToolTip(qp);
    chip->setStyleSheet(QStringLiteral("QPushButton{background:#e2e8f0;color:#1e293b;padding:4px 8px;border-radius:4px;font-size:9pt;}"));
    connect(chip, &QPushButton::clicked, this, [this, qp] { submitUserPrompt(qp); });
    quickBar->addWidget(chip);
  }
  leftLayout->addLayout(quickBar);

  m_chatView = new QTextBrowser(leftPane);
  m_chatView->setMinimumHeight(150);
  m_chatView->setStyleSheet(QStringLiteral("background:#fff;border:1px solid #cbd5e1;border-radius:6px;padding:8px;"));
  leftLayout->addWidget(m_chatView, 2);

  auto* inputRow = new QHBoxLayout();
  m_promptEdit = new QLineEdit(leftPane);
  m_promptEdit->setObjectName(QStringLiteral("hwpPromptInput"));
  m_promptEdit->setPlaceholderText(QStringLiteral("고찰·논문 주제나 수정 요청을 입력하세요 (예: 부여 관북리 백제 유적 DBpia 논문 검색해서 고찰과 차트 만들어줘)"));
  m_promptEdit->setStyleSheet(QStringLiteral("QLineEdit{padding:8px 10px;border:2px solid #be123c;border-radius:6px;font-size:10pt;}"));
  auto* btnSend = makeBtn(QStringLiteral("DBpia AI · 문서 반영"), QStringLiteral("#be123c"));
  btnSend->setObjectName(QStringLiteral("btnSendHwpPrompt"));
  inputRow->addWidget(m_promptEdit, 1);
  inputRow->addWidget(btnSend);
  leftLayout->addLayout(inputRow);

  auto* fieldTitle = new QLabel(QStringLiteral("2. 한글 누름틀(Field) 표 직접 편집"), leftPane);
  fieldTitle->setStyleSheet(QStringLiteral("font-weight:700;font-size:10pt;color:#1e293b;margin-top:4px;"));
  leftLayout->addWidget(fieldTitle);

  m_fieldTable = new QTableWidget(0, 2, leftPane);
  m_fieldTable->setHorizontalHeaderLabels({QStringLiteral("누름틀 이름"), QStringLiteral("내용 (수정 시 문서 즉시 반영)")});
  m_fieldTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
  m_fieldTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
  leftLayout->addWidget(m_fieldTable, 2);

  auto* rightPane = new QWidget(split);
  auto* rightLayout = new QVBoxLayout(rightPane);
  rightLayout->setContentsMargins(6, 0, 0, 0);
  auto* previewTitle = new QLabel(QStringLiteral("한글(.hwpx / .hwp) A4 용지 실시간 조판 (모식도·차트·그래프·DBpia 논문 포함)"), rightPane);
  previewTitle->setStyleSheet(QStringLiteral("font-weight:700;font-size:10.5pt;color:#0f172a;"));
  rightLayout->addWidget(previewTitle);
  m_previewView = new QTextBrowser(rightPane);
  m_previewView->setOpenExternalLinks(false);
  rightLayout->addWidget(m_previewView, 1);
  split->addWidget(leftPane);
  split->addWidget(rightPane);
  split->setStretchFactor(0, 5);
  split->setStretchFactor(1, 6);
  root->addWidget(split, 1);

  m_stageTimer = new QTimer(this);
  m_stageTimer->setInterval(420);
  connect(m_stageTimer, &QTimer::timeout, this, [this] {
    const auto snap = HwpSurveyReportService::ReportDocumentSession::collectFromProject(QgsProject::instance(), m_ragSlotCheck && m_ragSlotCheck->isChecked());
    ++m_currentStage;
    m_session->buildStandardReport(snap, nullptr, m_currentStage);
    const QStringList labels = {QString(), QStringLiteral("1/4단계: 표제부 및 Ⅰ. 조사 개요 조판 중..."),
                                QStringLiteral("2/4단계: Ⅱ. 주변 유적 현황 <표 1> 작성 중..."),
                                QStringLiteral("3/4단계: Ⅲ. 유구·유물 집계 <표 2> 정리 중..."),
                                QStringLiteral("4/4단계: Ⅳ. 모식도·차트·그래프 및 고찰 바인딩 완료")};
    if (m_currentStage >= 4) m_stageTimer->stop();
    syncUiFromSession(m_currentStage >= 4 ? QString() : labels.value(m_currentStage), qMin(100, m_currentStage * 25));
  });

  connect(m_dbpiaBridge, &KaGeminiBridgeClient::dbpiaPacketReceived, this, [this](const QString& q, int refCount, const QString& preview) {
    m_dbpiaSummary.lastQuery = q;
    m_dbpiaSummary.roundNumber = m_dbpiaBridge->currentRound();
    m_dbpiaSummary.collectedPapers = m_dbpiaBridge->lastCollectedPapers();
    appendChatMessage(QStringLiteral("DBpia 수집"), QStringLiteral("검색어 \"%1\" → 신규 %2편 (누적 %3편)\n%4").arg(q).arg(refCount).arg(m_dbpiaSummary.collectedPapers.size()).arg(preview));
    syncUiFromSession(QStringLiteral("DBpia AI 학술논문 수렴 및 차트 실시간 갱신: \"%1\" (누적 %2편)").arg(q).arg(m_dbpiaSummary.collectedPapers.size()), 65);
  });
  connect(m_dbpiaBridge, &KaGeminiBridgeClient::dbpiaHarvestCompleted, this, [this](const QJsonArray&, int totalUnique) {
    m_dbpiaSummary.collectedPapers = m_dbpiaBridge->lastCollectedPapers();
    m_dbpiaSummary.converged = true;
    const QString q1 = m_dbpiaSummary.lastQuery.isEmpty() ? QStringLiteral("조사지역 주변 유적") : m_dbpiaSummary.lastQuery;
    const QString ans1 = m_dbpiaBridge->buildPaperGroundedFallbackAnswer(q1 + QStringLiteral(" 유구 구조 및 편년"), 1);
    m_dbpiaSummary.dbpiaAiTurns.append(QJsonObject{{QStringLiteral("question"), q1}, {QStringLiteral("answer"), ans1}});
    m_dbpiaSummary.synthesizedDiscussion = m_dbpiaBridge->buildStructuredDataSynthesis(m_dbpiaSummary.dbpiaAiTurns);
    const auto snap = HwpSurveyReportService::ReportDocumentSession::collectFromProject(QgsProject::instance());
    HwpAcademicVisuals::appendDbpiaAndVisualSummaryToDoc(snap, m_dbpiaSummary, m_session.get());
    appendChatMessage(QStringLiteral("DBpia AI 완료"), QStringLiteral("총 %1편의 DBpia 학술논문 수집 및 <모식도 1~2>, <그림 1~2> 차트·그래프, 고찰 합성을 완료했습니다.").arg(totalUnique));
    syncUiFromSession(QString(), 100);
  });

  connect(btnLiveWin, &QPushButton::clicked, this, &KaHwpReportStudio::openLivePaperWindow);
  connect(btnDbpia, &QPushButton::clicked, this, [this] { startDbpiaResearch(); });
  connect(btnRefresh, &QPushButton::clicked, this, &KaHwpReportStudio::startLiveOrganizeDemo);
  connect(btnOpenTpl, &QPushButton::clicked, this, &KaHwpReportStudio::onOpenTemplateClicked);
  connect(btnSaveHwpx, &QPushButton::clicked, this, &KaHwpReportStudio::onSaveHwpxClicked);
  connect(btnSaveHwp5, &QPushButton::clicked, this, &KaHwpReportStudio::onSaveHwp5Clicked);
  connect(m_fieldTable, &QTableWidget::cellChanged, this, &KaHwpReportStudio::onFieldCellChanged);
  connect(btnSend, &QPushButton::clicked, this, &KaHwpReportStudio::onSendPromptClicked);
  connect(m_promptEdit, &QLineEdit::returnPressed, this, &KaHwpReportStudio::onSendPromptClicked);

  refreshFromActiveProject();
  appendChatMessage(QStringLiteral("안내"), QStringLiteral("상단 [DBpia AI 학술논문 수집 · 모식도/차트 생성] 버튼을 누르거나 아래 입력창에 고찰·논문 주제를 입력하면, 별도의 한글 A4 창에서 DBpia 논문 수집과 함께 <모식도 1~2>, <그림 1~2> 차트·그래프가 실시간으로 조판됩니다."));
}

KaHwpReportStudio::~KaHwpReportStudio() = default;

void KaHwpReportStudio::startDbpiaResearch(const QString& customTopic) {
  openLivePaperWindow();
  GisFactHarness::Ledger ledger = GisFactHarness::buildLedger(QgsProject::instance());
  if (!customTopic.trimmed().isEmpty()) ledger.dbpiaQueries.prepend(customTopic.trimmed());
  if (ledger.dbpiaQueries.isEmpty()) ledger.dbpiaQueries = {QStringLiteral("경주 신라 고분군 유적"), QStringLiteral("발굴조사 유구 편년")};
  syncUiFromSession(QStringLiteral("DBpia 학술문헌 API 및 DBpia AI 질의응답 수집 시작..."), 20);
  m_dbpiaBridge->startResearchAndEvaluate(customTopic, ledger, 500, QStringLiteral("report"), false);
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

void KaHwpReportStudio::startLiveOrganizeDemo() {
  openLivePaperWindow();
  m_currentStage = 0;
  m_stageTimer->start();
}

void KaHwpReportStudio::appendChatMessage(const QString& role, const QString& text) {
  if (!m_chatView) return;
  const QString color = (role == QLatin1String("요청")) ? QStringLiteral("#be123c") : QStringLiteral("#1d4ed8");
  m_chatView->append(QStringLiteral("<div style='margin:4px 0;'><b style='color:%1;'>[%2]</b> %3</div>")
                         .arg(color, role.toHtmlEscaped(), text.toHtmlEscaped().replace(QLatin1Char('\n'), QStringLiteral("<br/>"))));
}

void KaHwpReportStudio::onSendPromptClicked() {
  if (!m_promptEdit || m_promptEdit->text().trimmed().isEmpty()) return;
  const QString text = m_promptEdit->text().trimmed();
  m_promptEdit->clear();
  submitUserPrompt(text);
}

void KaHwpReportStudio::submitUserPrompt(const QString& promptText) {
  const QString req = promptText.trimmed();
  if (req.isEmpty()) return;
  openLivePaperWindow();
  appendChatMessage(QStringLiteral("요청"), req);
  const auto snap = HwpSurveyReportService::ReportDocumentSession::collectFromProject(QgsProject::instance(), m_ragSlotCheck && m_ragSlotCheck->isChecked());
  QString reply;
  if (m_session->applyUserRequest(req, snap, &reply)) appendChatMessage(QStringLiteral("문서 반영"), reply);
  if (req.contains(QStringLiteral("논문")) || req.contains(QStringLiteral("DBpia"), Qt::CaseInsensitive) ||
      req.contains(QStringLiteral("디비피아")) || req.contains(QStringLiteral("고찰")) || req.contains(QStringLiteral("차트"))) {
    QString kw = req;
    kw.remove(QStringLiteral("DBpia")).remove(QStringLiteral("디비피아")).remove(QStringLiteral("논문 검색 및"))
      .remove(QStringLiteral("모식도·차트")).remove(QStringLiteral("고찰 작성")).remove(QStringLiteral("해줘"));
    startDbpiaResearch(kw.simplified());
  } else {
    syncUiFromSession(QStringLiteral("요청 단락·모식도·차트 실시간 조판 반영: ") + req, 90);
    QTimer::singleShot(800, this, [this] { syncUiFromSession(QString(), 100); });
  }
}

void KaHwpReportStudio::refreshFromActiveProject() {
  const auto snap = HwpSurveyReportService::ReportDocumentSession::collectFromProject(QgsProject::instance(), m_ragSlotCheck && m_ragSlotCheck->isChecked());
  m_session->buildStandardReport(snap, nullptr, 4);
  m_summaryLabel->setText(QStringLiteral("조사명: %1  |  조사구역: %2개소 (%3 ㎡)  |  주변유적: %4개소  |  DBpia 수집 논문: %5편")
                              .arg(snap.surveyTitle).arg(snap.surveyPolygonCount).arg(qRound(snap.totalAreaSqm))
                              .arg(snap.nearbySites.size()).arg(m_dbpiaSummary.collectedPapers.size()));
  syncUiFromSession();
}

void KaHwpReportStudio::onOpenTemplateClicked() {
  const QString path = QFileDialog::getOpenFileName(this, QStringLiteral("한글 보고서 양식 파일 열기"), QDir::homePath(), QStringLiteral("한글 문서 (*.hwpx *.hwp)"));
  if (path.isEmpty()) return;
  const auto snap = HwpSurveyReportService::ReportDocumentSession::collectFromProject(QgsProject::instance(), m_ragSlotCheck && m_ragSlotCheck->isChecked());
  QString err;
  if (!m_session->openTemplateAndFill(std::filesystem::path(path.toStdString()), snap, &err)) QMessageBox::warning(this, QStringLiteral("양식 열기 실패"), err);
  else { openLivePaperWindow(); syncUiFromSession(QStringLiteral("기관 한글 양식 누름틀 자동 기입 완료"), 100); }
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
    syncUiFromSession(QStringLiteral("누름틀 필드 실시간 갱신: ") + nameItem->text(), 100);
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
  const auto snap = HwpSurveyReportService::ReportDocumentSession::collectFromProject(QgsProject::instance(), m_ragSlotCheck && m_ragSlotCheck->isChecked());
  const QString visualsHtml = HwpAcademicVisuals::renderVisualsAndDbpiaSectionHtml(snap, m_dbpiaSummary);
  const QString html = m_session->renderPreviewHtml(liveStageLabel, m_liveWindow ? m_liveWindow->zoomPercent() : 100, m_liveWindow ? m_liveWindow->showFieldMarks() : true, visualsHtml);
  m_previewView->setHtml(html);
  if (m_liveWindow)
    m_liveWindow->updatePaperHtml(html, liveStageLabel.isEmpty() ? QStringLiteral("한글(HWP/HWPX) A4 실시간 조판 완료 (모식도 2종 · 차트 2종 · DBpia 논문 %1편)").arg(m_dbpiaSummary.collectedPapers.size()) : liveStageLabel, progressPercent);
}

void KaHwpReportStudio::attach(QMainWindow* mainWindow) {
  if (!mainWindow) return;
  auto* tabs = mainWindow->findChild<QTabWidget*>(QStringLiteral("viewTabs"));
  auto* studioPage = new KaHwpReportStudio(tabs ? static_cast<QWidget*>(tabs) : mainWindow);
  studioPage->setObjectName(QStringLiteral("kaHwpReportStudio"));
  studioPage->setWindowFlags(Qt::Widget);
  if (tabs) {
    const int tabIdx = tabs->addTab(studioPage, QStringLiteral("한글 보고서·논문 (DBpia AI)"));
    if (QTabBar* bar = tabs->tabBar()) bar->setTabButton(tabIdx, QTabBar::RightSide, nullptr);
  }
  auto openStudio = [tabs, studioPage]() {
    if (tabs && tabs->indexOf(studioPage) >= 0) tabs->setCurrentWidget(studioPage);
    studioPage->startLiveOrganizeDemo();
  };
  QObject::connect(new QShortcut(QKeySequence(QStringLiteral("Ctrl+G")), mainWindow), &QShortcut::activated, mainWindow, openStudio);
  if (QWidget* corner = mainWindow->findChild<QWidget*>(QStringLiteral("viewTabsCorner"))) {
    if (corner->layout()) {
      auto* btn = new QPushButton(QStringLiteral("한글 실시간 조판 · DBpia AI (Ctrl+G)"), corner);
      btn->setObjectName(QStringLiteral("btnOpenHwpStudio"));
      btn->setCursor(Qt::PointingHandCursor);
      btn->setStyleSheet(QStringLiteral("QPushButton{background:#be123c;color:#fff;font-weight:700;padding:5px 12px;border-radius:6px;border:none;}QPushButton:hover{background:#9f1239;}"));
      QObject::connect(btn, &QPushButton::clicked, mainWindow, openStudio);
      corner->layout()->addWidget(btn);
    }
  }
}
