#include "KaDrawRowRules.h"

#include "KaTheme.h"

#include <QAction>
#include <QList>
#include <QToolBar>

namespace KaDrawRowRules {

void apply(QToolBar* bar) {
  if (!bar || !KaTheme::displayOptions().paperLook) return;
  // Where a cluster starts: the 자석 widgets after 도형선택, 쉽게그리기, and the first of the sketch buttons.
  QList<QAction*> starts;
  bool sketchSeen = false;
  const QList<QAction*> actions = bar->actions();
  for (int i = 0; i < actions.size(); ++i) {
    QAction* action = actions.at(i);
    if (action->isSeparator()) continue;
    const QString tool = action->property("kaSubTool").toString();
    if (tool == QLatin1String("select")) {
      int next = i + 1;
      while (next < actions.size() && actions.at(next)->isSeparator()) ++next;  // a rule already placed
      if (next < actions.size()) starts << actions.at(next);
    } else if (tool == QLatin1String("easy")) starts << action;
    else if (action->property("kaSketch").isValid() && !sketchSeen) {
      sketchSeen = true;
      starts << action;
    }
  }
  for (QAction* start : starts) {
    const QList<QAction*> now = bar->actions();
    const int at = now.indexOf(start);
    if (at > 0 && !now.at(at - 1)->isSeparator()) bar->insertSeparator(start);  // never two in a row
  }
}

}  // namespace KaDrawRowRules
