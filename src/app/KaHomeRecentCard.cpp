#include "KaHomeRecentCard.h"

#include "KaHomeRowDelegate.h"
#include "KaHomeText.h"
#include "KaIcons.h"
#include "KaRecoverySnapshots.h"
#include "KaTheme.h"
#include "core/SurveyFacts.h"

#include <QAbstractItemView>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QSettings>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>

namespace {

QLabel* label(const QString& text, const char* name, QWidget* parent) {
  auto* l = new QLabel(text, parent);
  l->setObjectName(QString::fromLatin1(name));
  return l;
}

// 구역 · 유구 · 검수 as dot states; empty when the counts are unknown or the file is gone.
QList<int> dotsFor(const RecentSurveys::Item& survey, const SurveyFacts::Facts& facts) {
  if (!survey.available || !facts.hasCounts()) return {};
  const int check = !facts.hasCheck()     ? KaHomeRecentCard::DotTodo
                    : facts.errors > 0    ? KaHomeRecentCard::DotDanger
                    : facts.warnings > 0  ? KaHomeRecentCard::DotWarn
                                          : KaHomeRecentCard::DotOk;
  return {facts.areas > 0 ? KaHomeRecentCard::DotOk : KaHomeRecentCard::DotTodo,
          facts.features > 0 ? KaHomeRecentCard::DotOk : KaHomeRecentCard::DotTodo, check};
}

}  // namespace

KaHomeRecentCard::KaHomeRecentCard(QWidget* parent) : QFrame(parent) {
  setObjectName(QStringLiteral("startRecentCard"));
  // 2차 보드 판 2: six plain columns with numbers, no icon column, the right-click hint under the table.
  m_plate = KaTheme::displayOptions().paperLook;
  auto* rows = new QVBoxLayout(this);
  rows->setContentsMargins(0, 0, 0, 6);
  rows->setSpacing(0);
  auto* header = new QHBoxLayout;
  header->setContentsMargins(18, 14, 18, 12);
  header->setSpacing(10);
  header->addWidget(label(QStringLiteral("최근 조사"), "startRecentTitle", this));
  m_count = label(QString(), "startRecentCount", this);
  header->addWidget(m_count);
  header->addStretch(1);
  // Same look as the count; its own name so it can be found and restyled.
  if (!m_plate) header->addWidget(label(QStringLiteral("오른쪽 클릭: 목록에서 제거"), "startRecentCount", this));
  m_filter = new QLineEdit(this);
  m_filter->setObjectName(QStringLiteral("startRecentFilter"));
  m_filter->setPlaceholderText(QStringLiteral("이름·폴더로 찾기"));
  m_filter->setClearButtonEnabled(true);
  m_filter->setFixedWidth(260);
  m_filter->addAction(QIcon(KaIcons::glyphPixmap(QStringLiteral("search"), KaTheme::tokens().inkMuted, 14,
                                                 devicePixelRatioF())),
                      QLineEdit::LeadingPosition);
  connect(m_filter, &QLineEdit::textChanged, this, &KaHomeRecentCard::applyFilter);
  header->addWidget(m_filter);
  rows->addLayout(header);

  m_empty = label(QStringLiteral("아직 최근 조사가 없습니다. 새 조사 또는 조사 열기로 시작하세요."),
                  "recentEmptyHint", this);
  m_empty->setWordWrap(true);
  m_empty->setContentsMargins(18, 8, 18, 8);
  rows->addWidget(m_empty);

  m_table = new QTableWidget(0, m_plate ? 6 : 4, this);
  m_table->setObjectName(QStringLiteral("recentSurveyList"));
  m_table->setAccessibleName(QStringLiteral("최근 조사 목록"));
  if (m_plate)
    m_table->setHorizontalHeaderLabels({QStringLiteral("조사"), QStringLiteral("상태"), QStringLiteral("조사구역"),
                                        QStringLiteral("유구"), QStringLiteral("검수"), QStringLiteral("마지막 열림")});
  else
    m_table->setHorizontalHeaderLabels({QStringLiteral("조사"), QStringLiteral("상태"),
                                        QStringLiteral("구역 · 유구 · 검수"), QStringLiteral("마지막 열림")});
  m_table->horizontalHeader()->setVisible(true);
  m_table->horizontalHeader()->setHighlightSections(false);
  m_table->horizontalHeader()->setSectionsClickable(false);
  m_table->verticalHeader()->setVisible(false);
  m_table->verticalHeader()->setDefaultSectionSize(kRowHeight);
  m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_table->setSelectionMode(QAbstractItemView::SingleSelection);
  m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_table->setShowGrid(false);
  m_table->setFocusPolicy(Qt::StrongFocus);
  m_table->setMouseTracking(true);
  m_table->viewport()->setCursor(Qt::PointingHandCursor);  // a row opens on a click
  m_table->setContextMenuPolicy(Qt::CustomContextMenu);
  auto* delegate = new KaHomeRowDelegate(m_table);
  for (int column : {SurveyColumn, StateColumn, DotsColumn})
    if (!m_plate || column != DotsColumn) m_table->setItemDelegateForColumn(column, delegate);
  auto* columns = m_table->horizontalHeader();
  columns->setSectionResizeMode(SurveyColumn, QHeaderView::Stretch);
  if (m_plate) columns->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  for (int column = StateColumn; column < m_table->columnCount(); ++column)
    columns->setSectionResizeMode(column, QHeaderView::ResizeToContents);
  connect(m_table, &QTableWidget::cellActivated, this, [this](int row, int) { openRow(row); });
  connect(m_table, &QTableWidget::cellClicked, this, [this](int row, int) { openRow(row); });
  connect(m_table, &QTableWidget::customContextMenuRequested, this, &KaHomeRecentCard::showMenu);
  rows->addWidget(m_table, 1);
  if (m_plate) {
    auto* note = label(QStringLiteral("목록에서 빼려면 조사 위에서 오른쪽 단추를 누르세요."), "startRecentNote", this);
    note->setContentsMargins(18, 8, 18, 4);
    rows->addWidget(note);
  }
}

void KaHomeRecentCard::setItems(const QVector<RecentSurveys::Item>& items, QSettings& settings) {
  m_table->setRowCount(0);
  m_empty->setVisible(items.isEmpty());
  m_table->setVisible(!items.isEmpty());
  m_count->setText(items.isEmpty() ? QString() : QStringLiteral("%1개").arg(items.size()));
  const QColor muted = KaTheme::tokens().inkMuted;
  int row = 0;
  for (const RecentSurveys::Item& survey : items) {
    const QString snapshot =
        survey.available ? KaRecoverySnapshots::unsavedSnapshotFor(settings, survey.path) : QString();
    // Read-only record; never opens the file here (the probe runs from the page, if enabled).
    const SurveyFacts::Facts facts = survey.available ? SurveyFacts::lookup(settings, survey.path)
                                                      : SurveyFacts::Facts{};
    const KaHomeText::StateChip state = KaHomeText::stateChip(survey, snapshot);
    const QString tip = survey.available
        ? QStringLiteral("누르면 이 조사를 엽니다.\n%1").arg(survey.path)
        : QStringLiteral("지금 찾을 수 없습니다. USB·네트워크 드라이브를 연결하면 다시 열 수 있습니다.\n%1")
              .arg(survey.path);
    auto* name = new QTableWidgetItem(survey.name);
    name->setData(kPathRole, survey.path);
    name->setData(kSnapshotRole, snapshot);
    name->setData(kFolderRole, KaHomeText::folder(survey.path));
    name->setData(kThumbRole, m_plate ? (survey.available ? QStringLiteral("none") : QStringLiteral("none-missing"))
                              : survey.available ? QStringLiteral("survey_thumb") : QStringLiteral("missing"));
    name->setToolTip(tip);
    auto* chip = new QTableWidgetItem(state.text);
    chip->setData(kToneRole, state.tone);
    chip->setToolTip(state.tip);
    auto* dots = new QTableWidgetItem;
    dots->setData(kDotsRole, QVariant::fromValue(dotsFor(survey, facts)));
    dots->setToolTip(KaHomeText::dotsTip(facts, survey.available));
    auto* when = new QTableWidgetItem(survey.available ? KaHomeText::when(survey.lastOpenedMs)
                                                       : QStringLiteral("찾을 수 없음"));
    when->setForeground(muted);
    when->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    when->setToolTip(tip);
    m_table->insertRow(row);
    int column = 0;
    if (m_plate) {
      delete dots;
      for (auto* item : {name, chip}) m_table->setItem(row, column++, item);
      for (QTableWidgetItem* cell : plateCells(survey, facts)) m_table->setItem(row, column++, cell);
      m_table->setItem(row, column, when);
    } else {
      for (auto* item : {name, chip, dots, when}) m_table->setItem(row, column++, item);
    }
    m_table->setRowHeight(row, kRowHeight);
    ++row;
  }
  applyFilter(m_filter->text());
}

// 조사구역 · 유구 · 검수 as words and numbers: 「—」 when not counted, 「검수 전」 before a check.
QList<QTableWidgetItem*> KaHomeRecentCard::plateCells(const RecentSurveys::Item& survey,
                                                       const SurveyFacts::Facts& facts) const {
  const bool counted = survey.available && facts.hasCounts();
  auto* areas = new QTableWidgetItem(counted ? QString::number(facts.areas) : QStringLiteral("—"));
  auto* features = new QTableWidgetItem(counted ? QString::number(facts.features) : QStringLiteral("—"));
  for (auto* number : {areas, features}) number->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
  const bool checked = survey.available && facts.hasCheck();
  auto* check = new QTableWidgetItem(!survey.available ? QStringLiteral("—")
                                     : !checked ? QStringLiteral("검수 전")
                                     : facts.errors + facts.warnings == 0
                                         ? QStringLiteral("통과")
                                         : QStringLiteral("오류 %1 · 주의 %2").arg(facts.errors).arg(facts.warnings));
  check->setForeground(checked && facts.errors > 0 ? KaTheme::tokens().danger : KaTheme::tokens().inkMuted);
  return {areas, features, check};
}

void KaHomeRecentCard::applyFilter(const QString& text) {
  const QString needle = text.trimmed();
  for (int row = 0; row < m_table->rowCount(); ++row) {
    const QTableWidgetItem* name = m_table->item(row, SurveyColumn);
    const QTableWidgetItem* state = m_table->item(row, StateColumn);
    const QString path = name ? name->data(kPathRole).toString() : QString();
    const bool match = needle.isEmpty() ||
                       (name && name->text().contains(needle, Qt::CaseInsensitive)) ||
                       (state && state->text().contains(needle, Qt::CaseInsensitive)) ||
                       path.contains(needle, Qt::CaseInsensitive);
    m_table->setRowHidden(row, !match);
  }
}

void KaHomeRecentCard::openRow(int row) {
  if (row < 0) return;
  const QTableWidgetItem* name = m_table->item(row, SurveyColumn);
  const QString path = name ? name->data(kPathRole).toString() : QString();
  if (!path.isEmpty()) emit openRequested(path);
}

void KaHomeRecentCard::showMenu(const QPoint& pos) {
  const QTableWidgetItem* hit = m_table->itemAt(pos);
  const QTableWidgetItem* name = hit ? m_table->item(hit->row(), SurveyColumn) : nullptr;
  const QString path = name ? name->data(kPathRole).toString() : QString();
  if (path.isEmpty()) return;
  const QString snapshot = name->data(kSnapshotRole).toString();
  QMenu menu(this);
  QAction* openCopy = nullptr;
  QAction* openFolder = nullptr;
  QAction* dismiss = nullptr;
  if (!snapshot.isEmpty()) {
    openCopy = menu.addAction(QStringLiteral("복구 사본 열기"));
    openFolder = menu.addAction(QStringLiteral("복구 사본 폴더 열기"));
    dismiss = menu.addAction(QStringLiteral("복구 사본 표시 숨기기"));
    menu.addSeparator();
  }
  QAction* forget = menu.addAction(QStringLiteral("목록에서 제거"));
  QAction* chosen = menu.exec(m_table->viewport()->mapToGlobal(pos));
  if (!chosen) return;
  if (chosen == forget) emit forgetRequested(path);
  else if (chosen == openCopy) emit openRequested(snapshot);
  else if (chosen == openFolder) emit recoveryFolderRequested(snapshot);
  else if (chosen == dismiss) emit recoveryDismissed(path);
}
