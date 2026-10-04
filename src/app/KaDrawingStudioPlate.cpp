#include "KaDrawingStudioPlate.h"

#include <QApplication>
#include <QEvent>
#include <QHBoxLayout>
#include <QBoxLayout>
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
  if (content) wordTiles(content);
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
  print->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);  // 「인쇄」 keeps its own width; PDF takes the rest
  print->setMinimumWidth(88);
  row->addWidget(pdf, 1);
  row->addWidget(print);
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

void wordTiles(QWidget* content) {
  if (!paperLook()) return;
  const QStringList words{QStringLiteral("축척 글자"), QStringLiteral("좌표계"), QStringLiteral("좌표 격자")};
  for (QToolButton* tile : content->findChildren<QToolButton*>()) {
    // 축척 글자 · 좌표계 by their tooltip word; 좌표 격자 by name, its tooltip follows the grid's state
    if (!words.contains(tile->toolTip()) && tile->objectName() != QLatin1String("btnGridSettings")) continue;
    tile->setToolButtonStyle(Qt::ToolButtonTextOnly);
    tile->setMinimumWidth(0);
    tile->setFixedHeight(32);
    auto* row = qobject_cast<QBoxLayout*>(tile->parentWidget() ? tile->parentWidget()->layout() : nullptr);
    for (int i = 0; row && i < row->count(); ++i) {  // the row the tile sits in, inside its parent's layout
      QBoxLayout* line = qobject_cast<QBoxLayout*>(row->itemAt(i)->layout());
      QBoxLayout* holder = row->indexOf(tile) >= 0 ? row : (line && line->indexOf(tile) >= 0 ? line : nullptr);
      if (!holder) continue;
      holder->setStretchFactor(tile, 1);
      for (int at = 0; at < holder->count(); ++at)  // the trailing stretch gives the room back to the words
        if (QSpacerItem* gap = holder->itemAt(at)->spacerItem()) gap->changeSize(0, 0, QSizePolicy::Fixed, QSizePolicy::Minimum);
      holder->invalidate();
      break;
    }
  }
}

void setWord(QToolButton* button) {
  button->setText(paperLook() ? QStringLiteral("세트 ▾") : QStringLiteral("세트"));
  button->installEventFilter(new SetWordFollower(button));
}

}  // namespace KaDrawingStudioPlate
