#pragma once

#include <QString>
#include <QWidget>

// 「조용한 도판」 방위 다이얼 (2차 보드 판 5 시굴격자 속성 「회전(방위)」 옆): a 40 px dial with north at the top and a
// needle turned clockwise by the grid's azimuth. Display only; the value is typed in the box beside it.
class KaAzimuthDial final : public QWidget {
  Q_OBJECT
public:
  explicit KaAzimuthDial(QWidget* parent = nullptr);

  void setAzimuth(double degrees);
  double azimuth() const { return m_azimuth; }
  QSize sizeHint() const override { return {40, 40}; }
  // 「위가 북쪽입니다」 at 0°, otherwise 「북에서 시계 방향 n°」.
  static QString note(double degrees);

protected:
  void paintEvent(QPaintEvent* event) override;

private:
  double m_azimuth = 0.0;
};
