#pragma once
#include "core/ChecklistEngine.h"

#include <QVector>
#include <QWidget>

// 「조용한 도판」 필수 도면 5종 (2차 보드 검수 목록): a paper tile per drawing, ink-framed when its rule
// passed, dashed with 「없음」 when it failed. Display only; the drawing rules decide. The strip exists
// only in 새 모양, so it paints with the board's colours (the 검수·제출 window does not link the theme).
// The names it shows are the properties "present" and "missing"; no rule result hides it.
class KaSubmitDrawingStrip final : public QWidget {
public:
  explicit KaSubmitDrawingStrip(QWidget* parent);
  void setResults(const QVector<CheckResult>& results);

protected:
  void paintEvent(QPaintEvent* event) override;
};
