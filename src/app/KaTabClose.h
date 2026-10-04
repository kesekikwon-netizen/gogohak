#pragma once

class QTabWidget;

// 「도면 ✕」 (2차 보드 판 4): in 새 모양 a closable view tab carries a clear muted ✕ in place of the style's faint cross
// (the theme may not use url() images). Tabs without a close button (홈, 지도) keep none; stock keeps the style's
// own cross. A click asks the tab widget to close that tab, as the style's button did.
namespace KaTabClose {
// Puts the ✕ on every closable tab now and each time the current tab changes (a new studio tab opens current).
void follow(QTabWidget* tabs);
}  // namespace KaTabClose
