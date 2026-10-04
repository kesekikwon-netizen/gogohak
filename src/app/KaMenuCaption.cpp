#include "KaMenuCaption.h"

#include <QApplication>
#include <QLabel>
#include <QMenu>
#include <QWidgetAction>

namespace {
constexpr const char* kCaption = "kaMenuCaption";
}  // namespace

namespace KaMenuCaption {

QAction* insert(QMenu* menu, const QString& text, QAction* before) {
  auto* word = new QLabel(text);
  word->setObjectName(QStringLiteral("kaMenuCaption"));  // 11 px muted, 24 px high (QSS)
  auto* caption = new QWidgetAction(menu);
  caption->setDefaultWidget(word);
  caption->setEnabled(false);  // a heading, never an item to pick
  caption->setProperty(kCaption, true);
  menu->insertAction(before, caption);
  return caption;
}

void follow(QMenu* menu) {
  const bool paper = qApp && qApp->property("kaPaperLook").toBool();
  for (QAction* action : menu->actions()) {
    if (action->property(kCaption).toBool()) action->setVisible(paper);
    else if (!action->isSeparator()) action->setIconVisibleInMenu(!paper);
  }
}

void install(QMenu* menu, const QList<QPair<QString, QAction*>>& groups) {
  for (const auto& group : groups) insert(menu, group.first, group.second);
  QObject::connect(menu, &QMenu::aboutToShow, menu, [menu] { follow(menu); });
  follow(menu);
}

}  // namespace KaMenuCaption
