#include "KaHomeTitleBlock.h"

#include "KaHomeText.h"
#include "core/SurveyFacts.h"

#include <QEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QStyle>
#include <QVBoxLayout>

namespace {

QLabel* label(const QString& text, const char* name, QWidget* parent) {
  auto* l = new QLabel(text, parent);
  l->setObjectName(QString::fromLatin1(name));
  return l;
}

QFrame* rule(Qt::Orientation orientation, QWidget* parent) {
  auto* line = new QFrame(parent);
  line->setObjectName(QStringLiteral("startTitleRule"));
  if (orientation == Qt::Vertical) line->setFixedWidth(1);
  else line->setFixedHeight(1);
  return line;
}

}  // namespace

KaHomeTitleBlock::KaHomeTitleBlock(QWidget* parent) : QFrame(parent) {
  setObjectName(QStringLiteral("startTitleBlock"));
  // One frame of three rows (보완 H2): the caption row over the whole width, the name beside the button, the cells.
  m_outer = new QVBoxLayout(this);
  m_outer->setContentsMargins(0, 0, 0, 0);
  m_outer->setSpacing(0);
  auto* tagBox = new QWidget(this);
  tagBox->setMinimumHeight(32);
  auto* tagRow = new QHBoxLayout(tagBox);
  tagRow->setContentsMargins(24, 0, 24, 0);
  tagRow->addWidget(label(QStringLiteral("이어서 작업"), "startTitleTag", tagBox));
  tagRow->addStretch(1);
  m_state = label(QString(), "startTitleState", tagBox);
  m_state->setTextFormat(Qt::RichText);
  tagRow->addWidget(m_state);
  m_outer->addWidget(tagBox);
  m_outer->addWidget(rule(Qt::Horizontal, this));
  auto* nameBox = new QWidget(this);
  nameBox->setMinimumHeight(92);
  auto* nameRow = new QHBoxLayout(nameBox);
  nameRow->setContentsMargins(24, 0, 0, 0);
  nameRow->setSpacing(0);
  m_name = label(QString(), "startTitleName", nameBox);
  m_name->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);  // a long name must not widen the window
  nameRow->addWidget(m_name, 1, Qt::AlignVCenter);
  nameRow->addWidget(rule(Qt::Vertical, nameBox));
  auto* buttonCell = new QWidget(nameBox);
  buttonCell->setMinimumWidth(320);
  auto* buttonRow = new QHBoxLayout(buttonCell);
  buttonRow->setContentsMargins(24, 0, 24, 0);
  m_resume = new QPushButton(QStringLiteral("이어서 열기  →"), buttonCell);
  m_resume->setObjectName(QStringLiteral("startContinueBtn"));
  buttonRow->addWidget(m_resume, 1, Qt::AlignVCenter);
  nameRow->addWidget(buttonCell);
  m_outer->addWidget(nameBox);
  m_outer->addWidget(rule(Qt::Horizontal, this));
  auto* cellsBox = new QWidget(this);
  cellsBox->setMinimumHeight(64);
  auto* cells = new QHBoxLayout(cellsBox);
  cells->setContentsMargins(0, 0, 0, 0);
  cells->setSpacing(0);
  m_folder = cell("폴더", "startCellFolder", cells);
  m_folder->installEventFilter(this);  // shortened again whenever the cell's width changes (showFolder)
  m_when = cell("마지막 열림", "startCellWhen", cells);
  m_crs = cell("작업 좌표계", "startCellCrs", cells);
  m_areas = cell("조사구역", "startCellAreas", cells);
  m_features = cell("유구", "startCellFeatures", cells);
  m_check = cell("검수", "startCellCheck", cells);
  m_outer->addWidget(cellsBox);
}

QLabel* KaHomeTitleBlock::cell(const char* caption, const char* name, QLayout* row) {
  if (row->count() > 0) row->addWidget(rule(Qt::Vertical, this));
  auto* box = new QWidget(this);
  // The six cells share the block's width; a long folder never asks the window for more (사용자 신고 2026-10-05).
  box->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
  auto* lines = new QVBoxLayout(box);
  lines->setContentsMargins(24, 10, 16, 12);
  lines->setSpacing(2);
  lines->addWidget(label(QString::fromUtf8(caption), "startCellCaption", box));
  QLabel* value = label(QString(), name, box);
  lines->addWidget(value);
  row->addWidget(box);
  return value;
}

// A long folder is shortened in the middle to the cell's width, never cut off at the edge (2026-10-10).
void KaHomeTitleBlock::showFolder() {
  // Not on screen yet (no real width): the whole folder until it shows.
  const int width = m_folder->isVisible() ? m_folder->contentsRect().width() : 0;
  m_folder->setText(width > 0 ? m_folder->fontMetrics().elidedText(m_folderText, Qt::ElideMiddle, width) : m_folderText);
}

bool KaHomeTitleBlock::eventFilter(QObject* watched, QEvent* event) {
  if (watched == m_folder && (event->type() == QEvent::Resize || event->type() == QEvent::Show || event->type() == QEvent::FontChange))
    showFolder();
  if (m_noteBox && (event->type() == QEvent::ShowToParent || event->type() == QEvent::HideToParent)) {
    if (auto* note = qobject_cast<QWidget*>(watched); note && note->parentWidget() == m_noteBox)
      m_noteBox->setVisible(event->type() == QEvent::ShowToParent);
  }
  return QFrame::eventFilter(watched, event);
}

// The note row (the recovery line) exists only while the note shows, so no empty band hangs under the cells.
void KaHomeTitleBlock::addNote(QWidget* note) {
  if (!m_noteBox) {
    m_noteBox = new QWidget(this);
    auto* column = new QVBoxLayout(m_noteBox);
    column->setContentsMargins(0, 0, 0, 0);
    column->setSpacing(0);
    column->addWidget(rule(Qt::Horizontal, m_noteBox));
    m_noteRow = new QHBoxLayout;
    m_noteRow->setContentsMargins(24, 8, 16, 8);  // 2차 보드: a row under the cells, in from the edge like them
    column->addLayout(m_noteRow);
    m_outer->addWidget(m_noteBox);
  }
  note->setSizePolicy(QSizePolicy::Ignored, note->sizePolicy().verticalPolicy());
  m_noteRow->addWidget(note);
  note->installEventFilter(this);
  m_noteBox->setVisible(!note->isHidden());
}

QString KaHomeTitleBlock::crsText(const QString& authId) {
  if (authId.endsWith(QLatin1String("5186"))) return QStringLiteral("5186 중부원점");
  if (authId.endsWith(QLatin1String("5187"))) return QStringLiteral("5187 동부원점");
  return authId.isEmpty() ? QStringLiteral("—") : authId.section(QLatin1Char(':'), -1);
}

void KaHomeTitleBlock::setSurvey(const RecentSurveys::Item& item, const SurveyFacts::Facts& facts, bool unsaved) {
  m_name->setText(item.name);
  // 「● 저장 안 됨」: the attention clay dot (as 연결 상태's 「● 설정 필요」), the words in ink.
  m_state->setText(unsaved ? QStringLiteral("<span style=\"color:#D97757\">●</span>&nbsp;저장 안 됨") : QStringLiteral("저장됨"));
  m_state->setAccessibleName(unsaved ? QStringLiteral("저장 안 됨") : QStringLiteral("저장됨"));
  m_state->setProperty("kaTone", unsaved ? QStringLiteral("needsYou") : QString());
  m_folderText = KaHomeText::folderTail(KaHomeText::folder(item.path));
  showFolder();
  m_folder->setToolTip(item.path);
  m_when->setText(KaHomeText::when(item.lastOpenedMs));
  m_crs->setText(crsText(facts.known ? facts.workCrs : QString()));
  const bool counted = facts.hasCounts();
  m_areas->setText(counted ? QStringLiteral("%1 곳").arg(facts.areas) : QStringLiteral("—"));
  m_features->setText(counted ? QStringLiteral("%1 기").arg(facts.features) : QStringLiteral("—"));
  const bool checked = facts.hasCheck();
  m_check->setText(!checked ? QStringLiteral("검수 전")
                   : facts.errors + facts.warnings == 0
                       ? QStringLiteral("통과")
                       : QStringLiteral("오류 %1 · 주의 %2").arg(facts.errors).arg(facts.warnings));
  m_check->setProperty("kaTone", checked && facts.errors > 0 ? QStringLiteral("block") : QString());
  for (QLabel* toned : {m_state, m_check}) {
    toned->style()->unpolish(toned);
    toned->style()->polish(toned);
  }
}
