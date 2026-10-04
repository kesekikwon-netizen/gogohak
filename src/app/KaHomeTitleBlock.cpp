#include "KaHomeTitleBlock.h"

#include "KaHomeText.h"
#include "core/SurveyFacts.h"

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
  auto* outer = new QHBoxLayout(this);
  outer->setContentsMargins(0, 0, 0, 0);
  outer->setSpacing(0);
  m_left = new QVBoxLayout;
  m_left->setContentsMargins(0, 0, 0, 0);
  m_left->setSpacing(0);
  auto* head = new QVBoxLayout;
  head->setContentsMargins(24, 14, 24, 14);
  head->setSpacing(2);
  auto* tagRow = new QHBoxLayout;
  tagRow->addWidget(label(QStringLiteral("이어서 작업"), "startTitleTag", this));
  tagRow->addStretch(1);
  m_state = label(QString(), "startTitleState", this);
  m_state->setTextFormat(Qt::RichText);
  tagRow->addWidget(m_state);
  head->addLayout(tagRow);
  m_name = label(QString(), "startTitleName", this);
  head->addWidget(m_name);
  m_left->addLayout(head);
  m_left->addWidget(rule(Qt::Horizontal, this));
  auto* cells = new QHBoxLayout;
  cells->setContentsMargins(0, 0, 0, 0);
  cells->setSpacing(0);
  m_folder = cell("폴더", "startCellFolder", cells);
  m_when = cell("마지막 열림", "startCellWhen", cells);
  m_crs = cell("작업 좌표계", "startCellCrs", cells);
  m_areas = cell("조사구역", "startCellAreas", cells);
  m_features = cell("유구", "startCellFeatures", cells);
  m_check = cell("검수", "startCellCheck", cells);
  m_left->addLayout(cells);
  outer->addLayout(m_left, 1);
  outer->addWidget(rule(Qt::Vertical, this));
  auto* right = new QVBoxLayout;
  right->setContentsMargins(24, 24, 24, 24);
  m_resume = new QPushButton(QStringLiteral("이어서 열기  →"), this);
  m_resume->setObjectName(QStringLiteral("startContinueBtn"));
  m_resume->setMinimumWidth(300);
  right->addWidget(m_resume);
  right->addStretch(1);
  outer->addLayout(right, 0);
}

QLabel* KaHomeTitleBlock::cell(const char* caption, const char* name, QLayout* row) {
  if (row->count() > 0) row->addWidget(rule(Qt::Vertical, this));
  auto* box = new QWidget(this);
  auto* lines = new QVBoxLayout(box);
  lines->setContentsMargins(24, 10, 16, 12);
  lines->setSpacing(2);
  lines->addWidget(label(QString::fromUtf8(caption), "startCellCaption", box));
  QLabel* value = label(QString(), name, box);
  lines->addWidget(value);
  row->addWidget(box);
  return value;
}

void KaHomeTitleBlock::addNote(QWidget* note) {
  m_left->addWidget(rule(Qt::Horizontal, this));
  auto* row = new QHBoxLayout;
  row->setContentsMargins(24, 8, 16, 8);  // 2차 보드: a row under the cells, in from the edge like them
  row->addWidget(note);
  m_left->addLayout(row);
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
  m_folder->setText(KaHomeText::folder(item.path));
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
