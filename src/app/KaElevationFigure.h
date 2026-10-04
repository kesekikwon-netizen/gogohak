#pragma once

#include <QString>
#include <QWidget>

// 「조용한 도판」 표고 그림 (2차 보드 판 5 「도형 속성」 표고 칸 옆, 244 × 76): a pit in section, the 상면 and 바닥
// levels as dashed guides with 「상면 EL. 152.84」 · 「바닥 EL. 152.42」, and the depth on a bracket at the right.
// Display only; the numbers are typed in the boxes beside it. Unknown values show 「—」.
class KaElevationFigure final : public QWidget {
  Q_OBJECT
public:
  explicit KaElevationFigure(QWidget* parent = nullptr);

  // The boxes' text as typed; an empty depth is 상면 − 바닥 when both are numbers.
  void setLevels(const QString& top, const QString& bottom, const QString& depth);
  QSize sizeHint() const override { return {244, 76}; }
  // 「EL. 152.84」 (two decimals) or 「EL. —」 when the text is not a number.
  static QString level(const QString& text);

protected:
  void paintEvent(QPaintEvent* event) override;

private:
  QString m_top;
  QString m_bottom;
  QString m_depth;
};
