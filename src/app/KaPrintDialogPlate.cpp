// 「조용한 도판」 도면 인쇄 (2차 보드 판 5, 860 × 640): the stock column rearranged into the board's two columns.
#include "KaPrintDialog.h"
#include "KaTilePreview.h"

#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLocale>
#include <QPushButton>
#include <QRadioButton>
#include <QVBoxLayout>

#include <algorithm>
#include <cmath>

namespace {
QString decimal(double value) {
  QString text = QLocale(QLocale::Korean).toString(value, 'f', 1);
  if (text.endsWith(QLatin1String(".0"))) text.chop(2);
  return text;
}

// Ledger values: figures in mono, the words around them muted (설계 2.2 Ledger).
QString figure(const QString& text) {
  return QStringLiteral("<span style=\"font-family:Consolas,monospace\">%1</span>").arg(text.toHtmlEscaped());
}
QString soft(const QString& text) {
  return QStringLiteral("<span style=\"color:#73726C\">%1</span>").arg(text.toHtmlEscaped());
}

// One ledger row, 24 px: the word, a dotted leader, the value.
QWidget* ledgerRow(QWidget* parent, const QString& word, const char* valueName, QLabel** value,
                   QLabel** wordLabel = nullptr) {
  auto* row = new QWidget(parent);
  row->setFixedHeight(24);
  auto* line = new QHBoxLayout(row);
  line->setContentsMargins(0, 0, 0, 0);
  line->setSpacing(6);
  auto* name = new QLabel(word, row);
  name->setProperty("kaFormWord", true);
  auto* leader = new QFrame(row);
  leader->setObjectName(QStringLiteral("printLedgerLeader"));
  leader->setFixedHeight(12);
  *value = new QLabel(row);
  (*value)->setObjectName(QString::fromLatin1(valueName));
  (*value)->setTextFormat(Qt::RichText);
  line->addWidget(name);
  line->addWidget(leader, 1, Qt::AlignVCenter);
  line->addWidget(*value);
  if (wordLabel) *wordLabel = name;
  return row;
}
}  // namespace

void KaPrintDialog::arrangePlate(QVBoxLayout* root, QHBoxLayout* pickRow, QHBoxLayout* paperRow) {
  // Empty the stock column; the widgets stay and move. Child layouts come back without a parent.
  const auto empty = [](QLayout* layout) {
    while (QLayoutItem* item = layout->takeAt(0))
      if (!item->layout()) delete item;
  };
  QLayout* buttons = root->itemAt(root->count() - 1)->layout();
  empty(root);
  empty(pickRow);
  delete pickRow;
  root->setContentsMargins(24, 24, 24, 16);
  root->setSpacing(16);

  // Left: the form with 96 px words, the ledger at the foot.
  for (int row = 0; row < m_form->rowCount(); ++row)
    if (QLayoutItem* item = m_form->itemAt(row, QFormLayout::LabelRole))
      if (auto* word = qobject_cast<QLabel*>(item->widget())) {
        word->setFixedWidth(96);
        word->setProperty("kaFormWord", true);
      }
  m_form->setHorizontalSpacing(0);
  m_form->setVerticalSpacing(12);
  if (auto* line = qobject_cast<QBoxLayout*>(m_properties->parentWidget()->layout())) {
    line->setDirection(QBoxLayout::TopToBottom);  // 「프린터 속성…」 under the list, as a small word
    line->setSpacing(6);
    line->setAlignment(m_properties, Qt::AlignLeft);
  }
  for (QPushButton* small : {m_properties, m_pickAll}) small->setProperty("kaPlate", true);
  // 「A4 | A3」 joined words stand in for the list, which stays as the one record of the choice.
  m_sheet->hide();
  auto* joined = new QHBoxLayout;
  joined->setSpacing(0);
  auto* words = new QButtonGroup(this);
  for (Sheet sheet : {SheetA4, SheetA3}) {
    auto* word = new QPushButton(sheet == SheetA4 ? QStringLiteral("A4") : QStringLiteral("A3"), this);
    word->setObjectName(sheet == SheetA4 ? QStringLiteral("printSheetA4") : QStringLiteral("printSheetA3"));
    word->setCheckable(true);
    word->setAutoDefault(false);  // Enter stays 「인쇄」
    word->setChecked(m_sheet->currentIndex() == int(sheet));
    words->addButton(word, int(sheet));
    joined->addWidget(word);
  }
  paperRow->insertLayout(0, joined);
  setTabOrder(m_output, words->button(SheetA4));  // where the hidden list stood
  setTabOrder(words->button(SheetA4), words->button(SheetA3));
  setTabOrder(words->button(SheetA3), m_overlap);
  connect(words, &QButtonGroup::idClicked, m_sheet, &QComboBox::setCurrentIndex);
  connect(m_sheet, &QComboBox::currentIndexChanged, words, [words](int index) {
    if (QAbstractButton* word = words->button(index)) word->setChecked(true);
  });
  m_overlap->setFixedWidth(88);
  m_overlapLabel->setProperty("kaFormWord", true);
  auto* settings = new QWidget(this);
  settings->setObjectName(QStringLiteral("printSettings"));
  int widest = 0;  // the board's 380 px column, wider only when a 「방법」 line would be cut off
  for (QRadioButton* method : {m_modeTiles, m_modeFit}) {
    method->ensurePolished();
    widest = std::max(widest, method->sizeHint().width());
  }
  settings->setFixedWidth(std::max(380, 96 + widest));
  auto* left = new QVBoxLayout(settings);
  left->setContentsMargins(0, 0, 0, 0);
  left->setSpacing(0);
  left->addLayout(m_form);
  left->addStretch(1);
  left->addWidget(m_ledger = buildLedger());

  // Right: 「미리보기」 and what is shown, the two-line hint, the preview, then what this print leaves out.
  auto* right = new QVBoxLayout;
  right->setSpacing(0);
  auto* head = new QHBoxLayout;
  auto* word = new QLabel(QStringLiteral("미리보기"), this);
  word->setProperty("kaFormWord", true);
  m_previewHead = new QLabel(this);
  m_previewHead->setObjectName(QStringLiteral("printPreviewHead"));
  m_previewHead->setTextFormat(Qt::RichText);
  head->addWidget(word);
  head->addStretch(1);
  head->addWidget(m_previewHead);
  right->addLayout(head);
  m_pickHint->setObjectName(QStringLiteral("printPickHint"));
  m_pickHint->setText(QStringLiteral("미리보기에서 장을 누르면 이번 인쇄에서 빼거나 다시 넣습니다.") + QLatin1Char(0x0A) +
                      QStringLiteral("잘못 나온 장만 다시 찍을 때 쓰세요."));
  right->addSpacing(2);
  right->addWidget(m_pickHint);
  right->addSpacing(8);
  right->addWidget(m_summary);
  // The warn wash and text of the paper look (KaThemePaper.cpp) instead of the stock amber box.
  m_warning->setStyleSheet(QStringLiteral(
      "QLabel#printWarning { color: #7A5A00; background: #F6ECC8; border: none; border-radius: 4px; padding: 6px 8px; }"));
  right->addWidget(m_warning);
  right->addWidget(m_preview, 1);
  right->addSpacing(8);
  auto* skipRow = new QHBoxLayout;
  skipRow->setSpacing(8);
  m_skipNote = new QLabel(this);
  m_skipNote->setObjectName(QStringLiteral("printSkipNote"));
  m_skipNote->hide();
  skipRow->addWidget(m_skipNote, 1);
  skipRow->addWidget(m_pickAll, 0, Qt::AlignTop);
  right->addLayout(skipRow);

  auto* columns = new QHBoxLayout;
  columns->setSpacing(24);
  columns->addWidget(settings);
  columns->addLayout(right, 1);
  root->addLayout(columns, 1);
  root->addLayout(buttons);
}

// 용지 · 배열 · 붙인 크기 · 축척 between an ink rule and a hairline (보드: 「A3 세로 · 6장」 「가로 3 × 세로 2」).
QWidget* KaPrintDialog::buildLedger() {
  auto* ledger = new QFrame(this);
  ledger->setObjectName(QStringLiteral("printLedger"));
  auto* rows = new QVBoxLayout(ledger);
  rows->setContentsMargins(0, 4, 0, 4);
  rows->setSpacing(0);
  rows->addWidget(ledgerRow(ledger, QStringLiteral("용지"), "printLedgerSheet", &m_ledgerSheet));
  rows->addWidget(m_ledgerGridRow = ledgerRow(ledger, QStringLiteral("배열"), "printLedgerGrid", &m_ledgerGrid));
  rows->addWidget(ledgerRow(ledger, QStringLiteral("붙인 크기"), "printLedgerSize", &m_ledgerSize, &m_ledgerSizeWord));
  rows->addWidget(m_ledgerScaleRow = ledgerRow(ledger, QStringLiteral("축척"), "printLedgerScale", &m_ledgerScale));
  return ledger;
}

void KaPrintDialog::refreshPlate() {
  if (!m_plate) return;
  if (m_drawingMm.isEmpty()) {  // nothing can be printed: only the reason stays, no empty rows or bars
    for (QWidget* idle : {m_ledger, static_cast<QWidget*>(m_previewHead), static_cast<QWidget*>(m_warning),
                          static_cast<QWidget*>(m_skipNote), static_cast<QWidget*>(m_pickHint),
                          static_cast<QWidget*>(m_pickAll)})
      idle->hide();
    return;
  }
  const bool tiled = isTiled();
  const bool planned = tiled && m_plan.ok;
  const QString sheet = m_sheet->currentText();
  const QString turn = (tiled ? m_plan.landscape : m_fit.landscape) ? QStringLiteral("가로") : QStringLiteral("세로");
  const QString scale = paperScaleText();
  const int count = int(m_plan.sheets.size());
  const int chosen = int(chosenSheets().size());
  m_ledgerSheet->setText(figure(sheet) + QLatin1Char(' ') +
                         soft(planned ? QStringLiteral("%1 · %2장").arg(turn).arg(count)
                                      : QStringLiteral("%1 · 한 장").arg(turn)));
  m_ledgerGridRow->setVisible(planned);
  m_ledgerGrid->setText(soft(QStringLiteral("가로")) + QLatin1Char(' ') + figure(QString::number(m_plan.cols)) +
                        QStringLiteral(" × ") + soft(QStringLiteral("세로")) + QLatin1Char(' ') +
                        figure(QString::number(m_plan.rows)));
  m_ledgerSizeWord->setText(tiled ? QStringLiteral("붙인 크기") : QStringLiteral("인쇄 크기"));
  if (tiled)
    m_ledgerSize->setText(
        figure(QStringLiteral("%1 × %2").arg(decimal(m_plan.outputMm.width() / 10.0), decimal(m_plan.outputMm.height() / 10.0))) +
        QLatin1Char(' ') + soft(QStringLiteral("cm")));
  else
    m_ledgerSize->setText(m_fit.actualSize
                              ? soft(QStringLiteral("실제 크기")) + QLatin1Char(' ') + figure(QStringLiteral("100%"))
                              : figure(QStringLiteral("%1%").arg(int(std::lround(m_fit.ratio * 100.0)))) +
                                    soft(QStringLiteral("로 줄임")));
  m_ledgerScaleRow->setVisible(!scale.isEmpty());
  m_ledgerScale->setText(figure(scale));
  QString head = QStringLiteral("도면 · ") + figure(sheet) + QLatin1Char(' ') + turn;
  if (planned) head += QStringLiteral(" · %1장 중 %2장").arg(count).arg(chosen);
  else if (!tiled) head += QStringLiteral(" · 한 장");
  if (!scale.isEmpty()) head += QStringLiteral(" · ") + figure(scale);
  m_previewHead->setText(head);
  m_skipNote->setVisible(planned && !m_skipped.isEmpty());
  QString skipped = QStringLiteral("이번에는 %1장만 찍습니다 (미리보기에서 뺀 장 %2장)").arg(chosen).arg(m_skipped.size());
  if (m_overview->isChecked()) skipped += QLatin1Char(0x0A) + QStringLiteral("안내 장은 다시 찍지 않습니다");
  m_skipNote->setText(skipped);
  // The ledger carries the facts; the bold line is left for a drawing or a plan that cannot be printed.
  m_summary->setVisible(m_drawingMm.isEmpty() || (tiled && !m_plan.ok));
}
