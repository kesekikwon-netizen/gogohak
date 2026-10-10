#pragma once

class QToolBar;

// 그리기 줄 (보완 M3): in 새 모양 a 1 px rule stands between the clusters of the drawing row
// (도형선택 | 자석 … | 그리는 도구 | 완료 · 연속 그리기), the rest of the row follows after it. Asking again changes nothing.
namespace KaDrawRowRules {
void apply(QToolBar* bar);
}  // namespace KaDrawRowRules
