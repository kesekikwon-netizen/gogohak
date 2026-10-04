#pragma once
#include <QWidget>

// 「조용한 도판」 경도 눈금자 in 「새 조사」 (2차 보드 판 5, 512 × 88): 126°–130°E with tenth-degree ticks, the 128° ink
// line between 중부원점 | 동부원점, the towns (제주 … 춘천 | 대구 … 울산) over the axis and a clay line and triangle at
// the map centre when it is known. Display only; the origin is still chosen below.
class KaLongitudeRuler final : public QWidget {
  Q_OBJECT
public:
  // centre: WGS84 longitude of the map centre, NaN when unknown.
  explicit KaLongitudeRuler(double centre, QWidget* parent = nullptr);
  double centre() const { return m_centre; }
  // Horizontal position of a longitude, clamped to the ruler's 126°–130° span.
  double xFor(double longitude) const;
  QSize sizeHint() const override { return {512, 88}; }

protected:
  void paintEvent(QPaintEvent* event) override;

private:
  double m_centre = 0.0;
};
