#include "KaDrawingStudioPlate.h"

#include <QApplication>
#include <QEvent>
#include <QHBoxLayout>
#include <QLayout>
#include <QScrollArea>
#include <QSpacerItem>
#include <QToolButton>
#include <QVBoxLayout>

namespace {
bool paperLook() { return qApp && qApp->property("kaPaperLook").toBool(); }

// Rewrites the set word whenever the look changes (KaTheme sends StyleChange to every widget on a switch).
class SetWordFollower final : public QObject {
public:
  using QObject::QObject;
  bool eventFilter(QObject* watched, QEvent* event) override {
    if (event->type() == QEvent::StyleChange)
      if (auto* button = qobject_cast<QToolButton*>(watched))
        button->setText(paperLook() ? QStringLiteral("세트 ▾") : QStringLiteral("세트"));
    return false;
  }
};
}  // namespace

namespace KaDrawingStudioPlate {

QWidget* inspectorColumn(QScrollArea* scroll) {
  if (!qApp || !qApp->property("kaPaperLook").toBool()) return scroll;
  QWidget* content = scroll->widget();
  if (content) joinScaleTable(content);  // the panel is built: share the scale presets' edges
  auto* pdf = content ? content->findChild<QToolButton*>(QStringLiteral("btnPrimary")) : nullptr;
  auto* print = content ? content->findChild<QToolButton*>(QStringLiteral("btnPrint")) : nullptr;
  if (!pdf || !print) return scroll;
  auto* column = new QWidget;
  column->setObjectName(QStringLiteral("studioInspectorColumn"));
  auto* stack = new QVBoxLayout(column);
  stack->setContentsMargins(0, 0, 0, 0);
  stack->setSpacing(0);
  stack->addWidget(scroll, 1);
  auto* foot = new QWidget(column);
  foot->setObjectName(QStringLiteral("studioOutputFoot"));  // the hairline over it (QSS)
  foot->setAttribute(Qt::WA_StyledBackground);
  auto* row = new QHBoxLayout(foot);
  row->setContentsMargins(16, 12, 16, 12);
  row->setSpacing(8);
  for (QToolButton* button : {pdf, print}) {
    button->setParent(foot);  // out of the card's row; the row's layout lets go of it
    button->show();
  }
  row->addWidget(pdf, 2);
  row->addWidget(print, 1);
  stack->addWidget(foot);
  column->setMinimumWidth(scroll->minimumWidth());
  column->setMaximumWidth(scroll->maximumWidth());
  return column;
}

QWidget* bottomStrip(QWidget* strip) {
  if (!qApp || !qApp->property("kaPaperLook").toBool() || !strip->layout()) return strip;
  strip->setProperty("kaPlate", true);
  strip->layout()->setContentsMargins(0, 0, 0, 0);
  strip->layout()->setSpacing(0);
  bool first = true;
  for (int i = 0; i < strip->layout()->count(); ++i) {
    auto* cell = qobject_cast<QToolButton*>(strip->layout()->itemAt(i)->widget());
    if (!cell) continue;
    cell->setFixedSize(96, 52);
    cell->setText(cell->text().replace(QLatin1Char(0x0A), QLatin1Char(' ')));  // one line under the icon
    cell->setProperty("kaCell", first ? QStringLiteral("first") : QStringLiteral("rest"));  // hairline before the rest
    first = false;
  }
  return strip;
}

int chipsPerRow() { return paperLook() ? 7 : 3; }

int chipGap(int stockGap) { return paperLook() ? 0 : stockGap; }

void joinScaleTable(QWidget* content) {
  if (!paperLook()) return;
  const auto chips = content->findChildren<QToolButton*>(QStringLiteral("scaleChip"));
  if (chips.isEmpty() || !chips.first()->parentWidget()->layout()) return;
  QLayout* column = chips.first()->parentWidget()->layout();
  int rows = 0;
  for (int i = 0; i < column->count(); ++i) {
    QLayout* row = column->itemAt(i)->layout();
    if (!row) continue;
    int cols = 0;
    for (int c = 0; c < row->count(); ++c) {
      auto* chip = qobject_cast<QToolButton*>(row->itemAt(c)->widget());
      if (!chip || chip->objectName() != QLatin1String("scaleChip")) continue;
      chip->setProperty("kaRow", rows == 0 ? QStringLiteral("first") : QStringLiteral("rest"));
      chip->setProperty("kaCol", cols++ == 0 ? QStringLiteral("first") : QStringLiteral("rest"));
    }
    if (cols == 0) continue;
    if (rows++ > 0 && i > 0)  // the gap before a later row of the table closes
      if (QSpacerItem* gap = column->itemAt(i - 1)->spacerItem()) gap->changeSize(0, 0, QSizePolicy::Minimum, QSizePolicy::Fixed);
  }
  column->invalidate();
}

void setWord(QToolButton* button) {
  button->setText(paperLook() ? QStringLiteral("세트 ▾") : QStringLiteral("세트"));
  button->installEventFilter(new SetWordFollower(button));
}

}  // namespace KaDrawingStudioPlate
