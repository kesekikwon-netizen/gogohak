#pragma once

#include <QWidget>

// 「조용한 도판」 시굴 비율 bar (2차 보드 판 5 시굴격자 속성): a 4 px track from 0 to twice the 목표, filled in ink up to
// the value, a 12 px ink mark at the 목표 with 「목표 10%」 beside it, 「0」 and 「20%」 at the ends. Display only.
class KaTargetMeter final : public QWidget {
  Q_OBJECT
public:
  explicit KaTargetMeter(QWidget* parent = nullptr);

  void setValue(double percent);
  void setTarget(double percent);
  double value() const { return m_value; }
  double target() const { return m_target; }
  QSize sizeHint() const override { return {512, 26}; }

protected:
  void paintEvent(QPaintEvent* event) override;

private:
  double m_value = 0.0;
  double m_target = 10.0;
};
