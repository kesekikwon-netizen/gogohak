#include "KaRecordPlate.h"
#include "KaElevationFigure.h"
#include "KaThemeFonts.h"
#include "KaWindowSize.h"

#include <QApplication>
#include <QDialog>
#include <QDialogButtonBox>
#include <QEvent>
#include <QFormLayout>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

#include <iterator>

namespace {
const char* const kSections[] = {"식별", "층위", "표고", "조사", "사진 · 비고"};
constexpr int kHalfBox = 244 - 96;  // the board's 244 px half cell less its 96 px word

struct Place {
  const char* field;
  int section;
  bool across;  // the whole row instead of half
};
// The board's order; a field not named here goes to 식별 after these.
const Place kPlaces[] = {{"kind", 0, false},      {"period", 0, false},    {"feature_no", 0, false},
                         {"survey_name", 0, false}, {"site_name", 0, false}, {"layer_no", 1, false},
                         {"relation", 1, true},   {"top_el", 2, false},    {"bottom_el", 2, false},
                         {"depth_m", 2, false},   {"status", 3, false},    {"surv_date", 3, false},
                         {"surveyor", 3, false},  {"photo", 4, true},      {"note", 4, true}};

struct Entry {
  QString field;
  QLabel* word = nullptr;
  QWidget* box = nullptr;
  bool across = false;
};

// 「식별」 ─────: an 11 px section word with a hairline running to the right edge.
QHBoxLayout* sectionHead(QWidget* parent, const QString& word) {
  auto* head = new QHBoxLayout;
  head->setSpacing(8);
  auto* label = new QLabel(word, parent);
  label->setObjectName(QStringLiteral("recordSection"));
  auto* rule = new QFrame(parent);
  rule->setObjectName(QStringLiteral("recordRule"));
  rule->setFixedHeight(1);
  head->addWidget(label);
  head->addWidget(rule, 1, Qt::AlignVCenter);
  return head;
}

// A 96 px word and its box.
QHBoxLayout* cell(const Entry& entry) {
  auto* line = new QHBoxLayout;
  line->setSpacing(0);
  entry.word->setFixedWidth(96);
  entry.word->setProperty("kaFormWord", true);
  const bool lines = entry.box->inherits("QPlainTextEdit");  // 비고: the word stays at the first line
  if (lines) entry.word->setContentsMargins(0, 6, 0, 0);
  line->addWidget(entry.word, 0, lines ? Qt::AlignTop : Qt::Alignment());
  line->addWidget(entry.box, 1);
  return line;
}

// Keeps the 「m」 at the right end of its box.
class UnitKeeper final : public QObject {
public:
  using QObject::QObject;
  bool eventFilter(QObject* watched, QEvent* event) override {
    if (event->type() == QEvent::Resize)
      if (auto* box = qobject_cast<QLineEdit*>(watched))
        if (auto* unit = box->findChild<QLabel*>(QStringLiteral("recordUnit")))
          unit->setGeometry(box->width() - 22, 0, 14, box->height());
    return false;
  }
};

// 표고 boxes (보드: mono, right-aligned, 「m」 inside the box).
void metreBox(QWidget* box) {
  auto* edit = qobject_cast<QLineEdit*>(box);
  if (!edit) return;
  edit->setProperty("kaRecordMetres", true);
  edit->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  QFont mono = edit->font();
  mono.setFamilies(KaTheme::monoFontStack());
  mono.setStyleHint(QFont::Monospace);
  edit->setFont(mono);
  edit->setTextMargins(0, 0, 18, 0);
  auto* unit = new QLabel(QStringLiteral("m"), edit);
  unit->setObjectName(QStringLiteral("recordUnit"));
  unit->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  unit->setAttribute(Qt::WA_TransparentForMouseEvents);
  edit->installEventFilter(new UnitKeeper(edit));
}

// Half fields two to a row, the across ones on a row of their own.
void addRows(QVBoxLayout* rows, const QList<Entry>& entries) {
  QHBoxLayout* pending = nullptr;  // a half row still waiting for its second field
  for (const Entry& entry : entries) {
    if (!entry.across) entry.box->setFixedWidth(kHalfBox);
    if (pending && !entry.across) {
      delete pending->takeAt(pending->count() - 1);  // the second field takes the room held for it
      pending->addLayout(cell(entry));
      pending = nullptr;
      continue;
    }
    auto* row = new QHBoxLayout;
    row->setSpacing(24);
    row->addLayout(cell(entry), entry.across ? 1 : 0);
    if (!entry.across) row->addStretch(1);
    rows->addLayout(row);
    pending = entry.across ? nullptr : row;
  }
}

QLineEdit* lineOf(const QList<Entry>& entries, const char* field) {
  for (const Entry& entry : entries)
    if (entry.field == QLatin1String(field)) return qobject_cast<QLineEdit*>(entry.box);
  return nullptr;
}

// 상면 · 바닥 on the left with the section picture beside them, 깊이 under them (보드 표고 칸).
void addElevations(QDialog* dialog, QVBoxLayout* rows, const QList<Entry>& entries) {
  QLineEdit* top = lineOf(entries, "top_el");
  QLineEdit* bottom = lineOf(entries, "bottom_el");
  for (const Entry& entry : entries) metreBox(entry.box);
  if (!top || !bottom) {
    addRows(rows, entries);
    return;
  }
  QLineEdit* depth = lineOf(entries, "depth_m");
  auto* pair = new QHBoxLayout;
  pair->setSpacing(24);
  auto* levels = new QVBoxLayout;
  levels->setSpacing(12);
  QList<Entry> rest;
  for (const Entry& entry : entries) {
    if (entry.box == top || entry.box == bottom) {
      entry.box->setFixedWidth(kHalfBox);
      levels->addLayout(cell(entry));
    } else {
      rest << entry;
    }
  }
  pair->addLayout(levels, 1);
  auto* figure = new KaElevationFigure(dialog);
  pair->addWidget(figure, 0, Qt::AlignVCenter);  // its own 244: the boxes line up with 깊이 under them
  rows->addLayout(pair);
  addRows(rows, rest);
  const auto redraw = [figure, top, bottom, depth] {
    figure->setLevels(top->text(), bottom->text(), depth ? depth->text() : QString());
  };
  for (QLineEdit* box : {top, bottom, depth})
    if (box) QObject::connect(box, &QLineEdit::textChanged, figure, redraw);
  redraw();
}
}  // namespace

namespace KaRecordPlate {

void arrange(QDialog* dialog, QFormLayout* form) {
  if (!qApp || !qApp->property("kaPaperLook").toBool() || dialog->layout() != form) return;
  QLabel* tip = nullptr;
  QDialogButtonBox* buttons = nullptr;
  QList<Entry> found;
  for (int row = 0; row < form->rowCount(); ++row) {
    if (QLayoutItem* item = form->itemAt(row, QFormLayout::SpanningRole)) {
      if (auto* box = qobject_cast<QDialogButtonBox*>(item->widget())) buttons = box;
      else if (auto* note = qobject_cast<QLabel*>(item->widget())) tip = note;
      continue;
    }
    QLayoutItem* word = form->itemAt(row, QFormLayout::LabelRole);
    QLayoutItem* box = form->itemAt(row, QFormLayout::FieldRole);
    auto* label = word ? qobject_cast<QLabel*>(word->widget()) : nullptr;
    if (!label || !box || !box->widget()) return;  // not the record form this was made for
    found << Entry{box->widget()->property("kaRecordField").toString(), label, box->widget()};
  }
  QList<Entry> sections[std::size(kSections)];
  for (const Place& place : kPlaces)
    for (Entry& entry : found)
      if (entry.box && entry.field == QLatin1String(place.field)) {
        entry.across = place.across;
        sections[place.section] << entry;
        entry.box = nullptr;  // placed
      }
  for (const Entry& entry : found)
    if (entry.box) sections[0] << entry;
  delete form;  // the words and boxes stay in the dialog and go into the sections below

  dialog->setProperty("kaPlate", true);
  dialog->setFixedWidth(KaWindowSize::width(KaWindowSize::Medium, 420));
  auto* root = new QVBoxLayout(dialog);
  root->setContentsMargins(24, 24, 24, 16);
  root->setSpacing(0);
  if (tip) {
    tip->setProperty("kaFormNote", true);
    root->addWidget(tip);
  }
  for (std::size_t at = 0; at < std::size(kSections); ++at) {
    if (sections[at].isEmpty()) continue;
    root->addSpacing(16);
    root->addLayout(sectionHead(dialog, QString::fromUtf8(kSections[at])));
    root->addSpacing(8);
    auto* rows = new QVBoxLayout;
    rows->setSpacing(12);
    if (at == 2) addElevations(dialog, rows, sections[at]);
    else addRows(rows, sections[at]);
    root->addLayout(rows);
  }
  if (buttons) {
    if (QPushButton* cancel = buttons->button(QDialogButtonBox::Cancel)) cancel->setText(QStringLiteral("취소"));
    root->addSpacing(24);
    root->addWidget(buttons);
  }
}

}  // namespace KaRecordPlate
