#include "KaDrawingStudioPlate.h"

#include <QApplication>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QToolButton>
#include <QVBoxLayout>

namespace KaDrawingStudioPlate {

QWidget* inspectorColumn(QScrollArea* scroll) {
  if (!qApp || !qApp->property("kaPaperLook").toBool()) return scroll;
  QWidget* content = scroll->widget();
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

}  // namespace KaDrawingStudioPlate
