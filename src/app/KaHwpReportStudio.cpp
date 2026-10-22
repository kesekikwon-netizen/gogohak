#include "KaHwpReportStudio.h"
#include "core/HwpSurveyReportService.h"
#include "core/LocalReportRag.h"
#include <QCheckBox>
#include <QDir>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QMessageBox>
#include <QPushButton>
#include <QShortcut>
#include <QSplitter>
#include <QTabBar>
#include <QTabWidget>
#include <QTableWidget>
#include <QTextBrowser>
#include <QVBoxLayout>
#include <qgsproject.h>

KaHwpReportStudio::KaHwpReportStudio(QWidget* parent)
    : QDialog(parent),
      m_session(std::make_unique<HwpSurveyReportService::ReportDocumentSession>()) {
  setWindowTitle(QStringLiteral("Strata · 한글 발굴조사보고서 스튜디오 (OpenHwpSDK .hwpx / .hwp)"));
  resize(1180, 780);
  auto* root = new QVBoxLayout(this);
  root->setContentsMargins(14, 14, 14, 14);
  root->setSpacing(10);

  auto* topBar = new QHBoxLayout();
  auto* btnRefresh = new QPushButton(QStringLiteral("GIS 현황으로 보고서 새로 생성"), this);
  auto* btnOpenTpl = new QPushButton(QStringLiteral("기존 한글 양식(.hwp/.hwpx) 열기"), this);
  auto* btnSaveHwpx = new QPushButton(QStringLiteral("한글 표준(.hwpx) 저장"), this);
  auto* btnSaveHwp5 = new QPushButton(QStringLiteral("한글 5.0(.hwp) 저장"), this);
  m_ragSlotCheck = new QCheckBox(QStringLiteral("발굴보고서 RAG 슬롯 연결 (인덱싱 완료 후 체크)"), this);
  btnSaveHwpx->setStyleSheet(QStringLiteral("QPushButton{background:#1d4ed8;color:#fff;font-weight:600;padding:6px 14px;border-radius:6px;}"));
  btnSaveHwp5->setStyleSheet(QStringLiteral("QPushButton{background:#0f766e;color:#fff;font-weight:600;padding:6px 14px;border-radius:6px;}"));
  topBar->addWidget(btnRefresh);
  topBar->addWidget(btnOpenTpl);
  topBar->addWidget(m_ragSlotCheck);
  topBar->addStretch(1);
  topBar->addWidget(btnSaveHwpx);
  topBar->addWidget(btnSaveHwp5);
  root->addLayout(topBar);

  m_summaryLabel = new QLabel(QStringLiteral("GIS 프로젝트 현황을 불러오는 중..."), this);
  m_summaryLabel->setStyleSheet(QStringLiteral("color:#334155;background:#f8fafc;border:1px solid #cbd5e1;border-radius:6px;padding:8px;"));
  root->addWidget(m_summaryLabel);

  auto* split = new QSplitter(Qt::Horizontal, this);
  auto* leftPane = new QWidget(split);
  auto* leftLayout = new QVBoxLayout(leftPane);
  leftLayout->setContentsMargins(0, 0, 6, 0);
  leftLayout->setSpacing(8);

  auto* reqTitle = new QLabel(QStringLiteral("1. 보고서 작성·수정 요청 (원하는 내용을 직접 지시하세요)"), leftPane);
  reqTitle->setStyleSheet(QStringLiteral("font-weight:700;font-size:11pt;color:#0f172a;"));
  leftLayout->addWidget(reqTitle);

  auto* quickBar = new QHBoxLayout();
  for (const QString& qp : {QStringLiteral("입지 및 고고학적 배경 단락을 작성해줘"),
                            QStringLiteral("주변 유적 공간분석 상세 서술을 추가해줘"),
                            QStringLiteral("종합 고찰에 조사 의의와 향후 과제를 보강해줘")}) {
    auto* chip = new QPushButton(qp.left(14) + QStringLiteral("…"), leftPane);
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
  m_promptEdit->setPlaceholderText(QStringLiteral("여기에 작성·수정 요청을 입력하세요 (예: 고찰에 신라 고분군과의 관계를 추가해줘 / '시굴'을 '정밀발굴'로 바꿔줘)"));
  m_promptEdit->setStyleSheet(QStringLiteral("QLineEdit{padding:8px 10px;border:2px solid #0f766e;border-radius:6px;font-size:10pt;}"));
  auto* btnSend = new QPushButton(QStringLiteral("문서에 요청 반영"), leftPane);
  btnSend->setObjectName(QStringLiteral("btnSendHwpPrompt"));
  btnSend->setStyleSheet(QStringLiteral("QPushButton{background:#0f766e;color:#fff;font-weight:700;padding:8px 16px;border-radius:6px;}"));
  inputRow->addWidget(m_promptEdit, 1);
  inputRow->addWidget(btnSend);
  leftLayout->addLayout(inputRow);

  auto* fieldTitle = new QLabel(QStringLiteral("2. 한글 누름틀(Field) 표 직접 편집"), leftPane);
  fieldTitle->setStyleSheet(QStringLiteral("font-weight:700;font-size:10.5pt;color:#1e293b;margin-top:6px;"));
  leftLayout->addWidget(fieldTitle);

  m_fieldTable = new QTableWidget(0, 2, leftPane);
  m_fieldTable->setHorizontalHeaderLabels({QStringLiteral("누름틀 이름"), QStringLiteral("내용 (수정 시 문서 즉시 반영)")});
  m_fieldTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
  m_fieldTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
  leftLayout->addWidget(m_fieldTable, 2);

  auto* rightPane = new QWidget(split);
  auto* rightLayout = new QVBoxLayout(rightPane);
  rightLayout->setContentsMargins(6, 0, 0, 0);
  auto* previewTitle = new QLabel(QStringLiteral("한글(.hwpx / .hwp) A4 문서 조판 실시간 미리보기"), rightPane);
  previewTitle->setStyleSheet(QStringLiteral("font-weight:700;font-size:11pt;color:#0f172a;"));
  rightLayout->addWidget(previewTitle);
  m_previewView = new QTextBrowser(rightPane);
  m_previewView->setOpenExternalLinks(false);
  rightLayout->addWidget(m_previewView, 1);

  split->addWidget(leftPane);
  split->addWidget(rightPane);
  split->setStretchFactor(0, 5);
  split->setStretchFactor(1, 6);
  root->addWidget(split, 1);

  connect(btnRefresh, &QPushButton::clicked, this, &KaHwpReportStudio::refreshFromActiveProject);
  connect(btnOpenTpl, &QPushButton::clicked, this, &KaHwpReportStudio::onOpenTemplateClicked);
  connect(btnSaveHwpx, &QPushButton::clicked, this, &KaHwpReportStudio::onSaveHwpxClicked);
  connect(btnSaveHwp5, &QPushButton::clicked, this, &KaHwpReportStudio::onSaveHwp5Clicked);
  connect(m_fieldTable, &QTableWidget::cellChanged, this, &KaHwpReportStudio::onFieldCellChanged);
  connect(btnSend, &QPushButton::clicked, this, &KaHwpReportStudio::onSendPromptClicked);
  connect(m_promptEdit, &QLineEdit::returnPressed, this, &KaHwpReportStudio::onSendPromptClicked);

  refreshFromActiveProject();
  appendChatMessage(QStringLiteral("안내"),
                    QStringLiteral("아래 초록색 입력창에 보고서에 들어갈 내용이나 수정 지시를 입력하고 [문서에 요청 반영] 또는 Enter를 누르면 우측 한글(.hwpx/.hwp) 문서에 즉시 반영됩니다."));
}

KaHwpReportStudio::~KaHwpReportStudio() = default;

void KaHwpReportStudio::appendChatMessage(const QString& role, const QString& text) {
  if (!m_chatView) return;
  const QString color = (role == QLatin1String("요청")) ? QStringLiteral("#0f766e") : QStringLiteral("#1d4ed8");
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
  appendChatMessage(QStringLiteral("요청"), req);
  const auto snap = HwpSurveyReportService::ReportDocumentSession::collectFromProject(
      QgsProject::instance(), m_ragSlotCheck && m_ragSlotCheck->isChecked());
  QString reply;
  if (m_session->applyUserRequest(req, snap, &reply)) {
    appendChatMessage(QStringLiteral("반영 완료"), reply);
    syncUiFromSession();
  } else {
    appendChatMessage(QStringLiteral("오류"), QStringLiteral("요청을 문서에 반영하지 못했습니다."));
  }
}

void KaHwpReportStudio::refreshFromActiveProject() {
  const bool useRag = m_ragSlotCheck && m_ragSlotCheck->isChecked();
  const auto ragHook = [](const HwpSurveyReportService::SurveySnapshot& snap) -> QString {
    const auto hits = strata::core::LocalReportRag::search(snap.surveyTitle.toStdString(), "", 3);
    if (hits.empty()) return QStringLiteral("[RAG 검색 완료 — 일치하는 보고서 본문 없음]");
    QStringList lines;
    for (const auto& h : hits)
      lines << QStringLiteral("• %1 (p.%2): %3").arg(QString::fromStdString(h.report_name)).arg(h.page_no).arg(QString::fromStdString(h.snippet).left(180));
    return lines.join(QStringLiteral("\n"));
  };
  const auto snap = HwpSurveyReportService::ReportDocumentSession::collectFromProject(QgsProject::instance(), useRag, ragHook);
  QString err;
  if (!m_session->buildStandardReport(snap, &err)) {
    QMessageBox::warning(this, QStringLiteral("보고서 생성 오류"), err);
    return;
  }
  m_summaryLabel->setText(QStringLiteral("조사명: %1  |  조사구역: %2개소 (%3 ㎡)  |  반경 내 주변유적: %4개소  |  좌표계: %5")
                              .arg(snap.surveyTitle).arg(snap.surveyPolygonCount).arg(qRound(snap.totalAreaSqm))
                              .arg(snap.nearbySites.size()).arg(snap.surveyCrs));
  syncUiFromSession();
}

void KaHwpReportStudio::onOpenTemplateClicked() {
  const QString path = QFileDialog::getOpenFileName(this, QStringLiteral("한글 보고서 양식 파일 열기"), QDir::homePath(), QStringLiteral("한글 문서 (*.hwpx *.hwp)"));
  if (path.isEmpty()) return;
  const auto snap = HwpSurveyReportService::ReportDocumentSession::collectFromProject(QgsProject::instance(), m_ragSlotCheck && m_ragSlotCheck->isChecked());
  QString err;
  if (!m_session->openTemplateAndFill(std::filesystem::path(path.toStdString()), snap, &err)) {
    QMessageBox::warning(this, QStringLiteral("양식 열기 실패"), err);
    return;
  }
  syncUiFromSession();
}

void KaHwpReportStudio::onSaveHwpxClicked() {
  const QString path = QFileDialog::getSaveFileName(this, QStringLiteral("한글 표준(.hwpx)으로 저장"), QDir::home().filePath(QStringLiteral("발굴조사보고서.hwpx")), QStringLiteral("한글 표준 문서 (*.hwpx)"));
  if (path.isEmpty()) return;
  QString err;
  if (!m_session->saveAs(std::filesystem::path(path.toStdString()), QStringLiteral("HWPX"), &err)) {
    QMessageBox::warning(this, QStringLiteral("저장 실패"), err);
    return;
  }
  QMessageBox::information(this, QStringLiteral("저장 완료"), QStringLiteral("한글 표준(.hwpx) 보고서를 저장했습니다:\n%1").arg(path));
}

void KaHwpReportStudio::onSaveHwp5Clicked() {
  const QString path = QFileDialog::getSaveFileName(this, QStringLiteral("한글 5.0(.hwp)으로 저장"), QDir::home().filePath(QStringLiteral("발굴조사보고서.hwp")), QStringLiteral("한글 5.0 문서 (*.hwp)"));
  if (path.isEmpty()) return;
  QString err;
  if (!m_session->saveAs(std::filesystem::path(path.toStdString()), QStringLiteral("HWP"), &err)) {
    QMessageBox::warning(this, QStringLiteral("저장 실패"), err);
    return;
  }
  QMessageBox::information(this, QStringLiteral("저장 완료"), QStringLiteral("한글 5.0(.hwp) 보고서를 저장했습니다:\n%1").arg(path));
}

void KaHwpReportStudio::onFieldCellChanged(int row, int column) {
  if (m_updatingTable || column != 1) return;
  QTableWidgetItem *nameItem = m_fieldTable->item(row, 0), *valItem = m_fieldTable->item(row, 1);
  if (nameItem && valItem && m_session->setFieldValue(nameItem->text(), valItem->text()))
    m_previewView->setHtml(m_session->renderPreviewHtml());
}

void KaHwpReportStudio::syncUiFromSession() {
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
  m_previewView->setHtml(m_session->renderPreviewHtml());
}

void KaHwpReportStudio::attach(QMainWindow* mainWindow) {
  if (!mainWindow) return;
  auto* tabs = mainWindow->findChild<QTabWidget*>(QStringLiteral("viewTabs"));
  auto* studioPage = new KaHwpReportStudio(tabs ? static_cast<QWidget*>(tabs) : mainWindow);
  studioPage->setObjectName(QStringLiteral("kaHwpReportStudio"));
  studioPage->setWindowFlags(Qt::Widget);
  if (tabs) {
    const int tabIdx = tabs->addTab(studioPage, QStringLiteral("한글 보고서 (.hwpx/.hwp)"));
    if (QTabBar* bar = tabs->tabBar()) bar->setTabButton(tabIdx, QTabBar::RightSide, nullptr);
  }
  auto openStudio = [tabs, studioPage]() {
    studioPage->refreshFromActiveProject();
    if (tabs && tabs->indexOf(studioPage) >= 0) {
      tabs->setCurrentWidget(studioPage);
      if (auto* edit = studioPage->findChild<QLineEdit*>(QStringLiteral("hwpPromptInput"))) edit->setFocus();
    } else {
      studioPage->setWindowFlags(Qt::Dialog);
      studioPage->show();
    }
  };
  auto* sc = new QShortcut(QKeySequence(QStringLiteral("Ctrl+G")), mainWindow);
  QObject::connect(sc, &QShortcut::activated, mainWindow, openStudio);
  if (QWidget* corner = mainWindow->findChild<QWidget*>(QStringLiteral("viewTabsCorner"))) {
    if (corner->layout()) {
      auto* btn = new QPushButton(QStringLiteral("한글 보고서 요청·작성 (Ctrl+G)"), corner);
      btn->setObjectName(QStringLiteral("btnOpenHwpStudio"));
      btn->setCursor(Qt::PointingHandCursor);
      btn->setStyleSheet(QStringLiteral("QPushButton{background:#0f766e;color:#fff;font-weight:600;padding:5px 12px;border-radius:6px;border:none;}QPushButton:hover{background:#115e59;}"));
      QObject::connect(btn, &QPushButton::clicked, mainWindow, openStudio);
      corner->layout()->addWidget(btn);
    }
  }
}
