#pragma once

#include <QStringList>
#include <QWidget>

class QLabel;

// 받는 순서 (2차 보드 판 5 「수치지형도 내려받기」): the steps on one line, each a circle with its name under it. A done
// step is a ✓ in an ink circle, the current one an ink square, a later one a grey number; the line between them is ink
// up to the current step and grey after it. Display only.
class KaStageRail final : public QWidget {
public:
  explicit KaStageRail(const QStringList& stages, QWidget* parent = nullptr);

  const QStringList& stages() const { return m_stages; }
  int current() const { return m_current; }
  // 0-based; stages().size() means every step is done.
  void setCurrent(int index);
  QSize sizeHint() const override;

  // In 새 모양 puts a rail of trail's steps (written 「a → b → c」) right after trail in its layout and hides trail;
  // in stock does nothing and returns nullptr.
  static KaStageRail* replace(QLabel* trail);

protected:
  void paintEvent(QPaintEvent* event) override;

private:
  QStringList m_stages;
  int m_current = 0;
};
