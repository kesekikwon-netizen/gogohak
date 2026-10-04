#pragma once

#include "core/TilePrint.h"

#include <QImage>
#include <QPageSize>
#include <QSet>
#include <QWidget>

#include <functional>

class QPainter;

// 도면 인쇄 미리보기: 나눈 칸을 그리고, 누르면 그 장을 이번 인쇄에서 빼거나 다시 넣는다.
// 「조용한 도판」 (2차 보드 판 5) draws the sheets on the oat desk with ink edges, hatched overlaps and a skipped
// sheet washed out and numbered 「· 이번에는 뺌」; the stock look keeps its blue sheets.
class KaTilePreview : public QWidget {
public:
  explicit KaTilePreview(QWidget* parent);

  std::function<void(int)> onToggle;

  void setContent(const QImage& thumb, const TilePlan& plan, const QSet<int>& skipped, bool tiled,
                  const QSizeF& drawingMm, const QPageSize& sheet, const TileFit& fit);
  // 좌표에 있는 장. 겹침 칸에서는 가운데가 더 가까운 장을 고른다.
  int sheetAt(const QPointF& pos) const;

protected:
  void paintEvent(QPaintEvent*) override;
  void mousePressEvent(QMouseEvent* event) override;

private:
  QRectF box() const;
  void layoutTiles(double* k, QPointF* origin) const;
  static QRectF mapped(const QRectF& mm, double k, const QPointF& origin);
  void paintTiles(QPainter& p) const;
  void paintSheet(QPainter& p) const;
  void paintPlateTiles(QPainter& p, double k, const QPointF& origin) const;

  QImage m_thumb;
  TilePlan m_plan;
  QSet<int> m_skipped;
  bool m_tiled = false;
  QSizeF m_drawingMm;
  QPageSize m_sheet;
  TileFit m_fit;
  bool m_plate = false;  // 새 모양 when the window was built
};
