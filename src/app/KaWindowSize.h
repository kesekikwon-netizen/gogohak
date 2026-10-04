#pragma once
#include <QCoreApplication>
#include <QVariant>

// 「조용한 도판」 window widths (docs/design/2026-10-04-quiet-plate: S 440 · M 560 · L 860). The stock look keeps
// each window's own width. Reads the application's "kaPaperLook" property (set by KaTheme::setDisplayOptions)
// so dialogs built into test programs without the theme sources still compile and link.
namespace KaWindowSize {

enum Size { Small, Medium, Large };

inline int width(Size size, int stockWidth) {
  if (!qApp || !qApp->property("kaPaperLook").toBool())
    return stockWidth;
  return size == Small ? 440 : size == Medium ? 560 : 860;
}

}  // namespace KaWindowSize
