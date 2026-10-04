#pragma once
#include <QWidget>

// 「조용한 도판」 경도 눈금자 in 「새 조사」 (2차 보드, 512 × 72): 124°–132°E degree ticks, the 128° ink line
// between 중부원점 | 동부원점, place ticks (제주 … 원주 | 대구 … 울산) and a clay triangle with
// 「지금 지도 중심 126.4°」 when the map centre is known. Display only; the origin is still chosen below.
class KaLongitudeRuler final : public QWidget {
  Q_OBJECT
public:
  // centre: WGS84 longitude of the map centre, NaN when unknown.
  explicit KaLongitudeRuler(double centre, QWidget* parent = nullptr);
  double centre() const { return m_centre; }
  // Horizontal position of a longitude, clamped to the ruler's 124°–132° span.
  double xFor(double longitude) const;
  QSize sizeHint() const override { return {512, 72}; }

protected:
  void paintEvent(QPaintEvent* event) override;

private:
  double m_centre = 0.0;
};
