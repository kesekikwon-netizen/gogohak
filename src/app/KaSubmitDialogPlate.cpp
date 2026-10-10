// 「조용한 도판」 검수·제출 (2차 보드 판 3, spec 4.3 Row 4): the result as a serif headline with the rules' name and
// what to do next under it, 오류 · 주의 · 통과 as mono figures at the top right, and what the package holds in one
// mono line above the buttons. Only built in 새 모양 (the window is modal, so the look cannot change while it is open).
#include "KaSubmitDialog.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QStyle>
#include <QVBoxLayout>

QWidget* KaSubmitDialog::buildPlateHead() {
  auto* head = new QWidget(this);
  auto* row = new QHBoxLayout(head);
  row->setContentsMargins(0, 0, 0, 0);
  row->setSpacing(24);
  auto* words = new QVBoxLayout;
  words->setSpacing(4);
  m_headline = new QLabel(head);
  m_headline->setObjectName(QStringLiteral("submitHeadline"));
  m_headline->setWordWrap(true);
  m_subline = new QLabel(head);
  m_subline->setObjectName(QStringLiteral("submitSubline"));
  m_subline->setWordWrap(true);
  words->addWidget(m_headline);
  words->addWidget(m_subline);
  row->addLayout(words, 1);
  auto* counts = new QFrame(head);
  counts->setObjectName(QStringLiteral("submitCounts"));
  auto* cells = new QHBoxLayout(counts);
  cells->setContentsMargins(0, 0, 0, 0);
  cells->setSpacing(0);
  const char* cellNames[][2] = {{"오류", "submitCountError"}, {"주의", "submitCountWarn"}, {"통과", "submitCountPass"}};
  for (int i = 0; i < 3; ++i) {
    if (i > 0) {
      auto* rule = new QFrame(counts);
      rule->setObjectName(QStringLiteral("submitCountRule"));
      rule->setFixedWidth(1);
      cells->addWidget(rule);
    }
    auto* cell = new QVBoxLayout;
    cell->setContentsMargins(12, 6, 24, 6);
    auto* caption = new QLabel(QString::fromUtf8(cellNames[i][0]), counts);
    caption->setObjectName(QStringLiteral("submitCountCaption"));
    auto* figure = new QHBoxLayout;
    figure->setSpacing(4);
    m_counts[i] = new QLabel(counts);
    m_counts[i]->setObjectName(QString::fromLatin1(cellNames[i][1]));
    auto* unit = new QLabel(QStringLiteral("건"), counts);
    unit->setObjectName(QStringLiteral("submitCountUnit"));
    figure->addWidget(m_counts[i], 0, Qt::AlignBaseline);
    figure->addWidget(unit, 0, Qt::AlignBaseline);
    figure->addStretch(1);
    cell->addWidget(caption);
    cell->addLayout(figure);
    cells->addLayout(cell);
  }
  row->addWidget(counts, 0, Qt::AlignTop);
  return head;
}

QWidget* KaSubmitDialog::buildPlateFoot() {
  auto* line = new QLabel(QStringLiteral("EPSG:5179 SHP · 조사도면.pdf · MANIFEST.sha256"), this);
  line->setObjectName(QStringLiteral("submitPackageLine"));
  line->setToolTip(QStringLiteral("제출 꾸러미에 들어가는 것입니다. 검수 오류가 남아 있어도 만들 수 있고, "
                                  "도면 용지가 없으면 조사도면.pdf만 빠집니다."));
  return line;
}

void KaSubmitDialog::showPlateResult(int errors, int warns, int passed) {
  const bool noRules = !m_rulesLoaded || m_results.isEmpty();
  m_headline->setText(noRules      ? QStringLiteral("검수 규칙 파일을 찾지 못했습니다")
                      : errors > 0 ? QStringLiteral("오류 %1건이 남아 있습니다").arg(errors)
                      : warns > 0  ? QStringLiteral("오류가 없습니다")
                                   : QStringLiteral("오류와 주의 항목이 없습니다"));
  // What to do next follows the headline: nothing to add with a missing rule file (the reason below says where
  // it was looked for) or when everything passed.
  const QString next = noRules      ? QString()
                       : errors > 0 ? QStringLiteral("「위치 보기」나 옆 단추로 고치거나, 그대로 제출 꾸러미를 만들 수 있습니다.")
                       : warns > 0  ? QStringLiteral("주의 항목은 확인한 뒤 진행할 수 있습니다.")
                                    : QString();
  QStringList parts{noRules ? QString() : m_rulesTitle, next};
  parts.removeAll(QString());
  m_subline->setText(parts.join(QStringLiteral(" · ")));
  const int numbers[] = {errors, warns, passed};
  for (int i = 0; i < 3; ++i) m_counts[i]->setText(QString::number(numbers[i]));
  m_counts[0]->setProperty("zero", errors == 0);  // the blocking red only when something blocks
  m_counts[0]->style()->unpolish(m_counts[0]);
  m_counts[0]->style()->polish(m_counts[0]);
  m_blockReason->setVisible(noRules);  // only where the rule file was looked for
}
