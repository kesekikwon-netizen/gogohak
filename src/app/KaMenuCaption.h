#pragma once

#include <QList>
#include <QPair>
#include <QString>

class QAction;
class QMenu;

// 「조용한 도판」 menu group captions (2차 보드 판 5 더보기: 「불러오기·배경」「화면」「계정과 키」「상태와 정보」).
// A caption is an 11 px muted row that is shown only in 새 모양; there the items also drop their icons. Stock keeps
// its icons and shows no captions. Call follow() each time the menu opens so a switch of the look is picked up.
namespace KaMenuCaption {
// Puts a caption row before `before` (at the end when null) and returns its action.
QAction* insert(QMenu* menu, const QString& text, QAction* before = nullptr);
// Shows the captions and hides the item icons in 새 모양, and the other way round in stock.
void follow(QMenu* menu);
// Puts each caption before its group's first item and calls follow() each time the menu opens.
void install(QMenu* menu, const QList<QPair<QString, QAction*>>& groups);
}  // namespace KaMenuCaption
