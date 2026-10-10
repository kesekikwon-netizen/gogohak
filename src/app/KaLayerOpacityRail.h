#pragma once

#include <QFrame>
#include <QPointer>

class QLabel;
class QMouseEvent;
class QSlider;
class QShowEvent;

// 맵·도면만들기 캔버스 안쪽 왼쪽 위에 붙는 세로 막대. 투명도와 밝기 두 줄이다.
// 레이어 카드 밖, 레이어 창 바로 오른쪽(캔버스 좌상)에 둔다. 가리면 빈 곳을 잡고 끌어 옮긴다.
class KaLayerOpacityRail : public QFrame {
  Q_OBJECT
public:
  // placementKey: 끌어 놓은 자리를 이 이름으로 기억한다(지도 "map", 도면 "drawing"). 비면 기억하지 않는다.
  explicit KaLayerOpacityRail(QWidget* host, const QString& placementKey = QString());

  QSlider* slider() const { return m_slider; }
  void setPercent(int percent, bool enabled);
  int percent() const;
  // 밝기: -255~255, 0이 원본. 그림(래스터)일 때만 쓴다.
  void setBrightness(int value, bool enabled);
  int brightness() const;
  // 지금 조절하는 레이어 이름. adjustable=false 면 막대를 접고 안내 한 줄만 둔다.
  void setTarget(const QString& layerName, bool adjustable);
  // 그리기 안내 띠(KaDrawGuideBand)가 보이는 동안 그만큼 아래로 내려 앉는다. 0이면 10,10.
  void setTopInset(int px);
  int topInset() const { return m_topInset; }

signals:
  void percentChanged(int percent);
  void brightnessChanged(int value);

protected:
  bool eventFilter(QObject* watched, QEvent* event) override;
  void showEvent(QShowEvent* event) override;
  void mousePressEvent(QMouseEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;

private:
  void reposition();
  QPoint keptInside(const QPoint& wanted) const;

  QPointer<QWidget> m_host;
  QSlider* m_slider = nullptr;
  QLabel* m_title = nullptr;
  QLabel* m_value = nullptr;
  QSlider* m_bright = nullptr;
  QLabel* m_brightTitle = nullptr;
  QLabel* m_brightValue = nullptr;
  QLabel* m_target = nullptr;
  bool m_adjustable = false;
  int m_topInset = 0;
  QString m_placementKey;
  bool m_placed = false;  // the user dragged it: m_placedPos replaces the 10,10 corner
  QPoint m_placedPos;
  bool m_dragging = false;
  QPoint m_grab;  // where the press landed inside the card
};
