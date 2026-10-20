// KaTrenchMoveTool: the grid-node drop, arrow-key moves and picking a trench from outside the tool
// (시굴격자 트렌치를 쉽게 옮기기, 사용자 요청 2026-10-20).
#include "KaTrenchMoveTool.h"
#include "KaCanvasGridOverlay.h"
#include "core/CanvasGridMath.h"

#include <QKeyEvent>

#include <qgsfeature.h>
#include <qgsfeatureiterator.h>
#include <qgsgeometry.h>
#include <qgsvectorlayer.h>

void KaTrenchMoveTool::selectTrench(QgsFeatureId fid) {
  if (!m_layer || FID_IS_NULL(fid)) return;
  const QgsFeature feature = m_layer->getFeature(fid);
  if (!feature.isValid()) return;
  setMode(Mode::Single);
  m_selFid = fid;
  m_selName = feature.attribute(QStringLiteral("name")).toString();
  rebuildSingleRubber(fid, QgsPointXY(0, 0));
  emit statusMessage(QStringLiteral("%1 선택 — 끌어서 이동, 화살표 = 한 칸씩, Delete = 삭제")
                         .arg(m_selName.isEmpty() ? QStringLiteral("트렌치") : m_selName));
}

QgsPointXY KaTrenchMoveTool::dragDelta(const QgsPointXY& cursor) const {
  if (m_grid && m_grid->isEnabled() && m_layer) {
    // The dragged trench's outer corner (its first vertex; the first trench's for the whole grid)
    // lands on the nearest grid node, so a trench off the grid comes onto it.
    QgsFeature feature;
    if (m_mode == Mode::Single && !FID_IS_NULL(m_selFid)) feature = m_layer->getFeature(m_selFid);
    else m_layer->getFeatures().nextFeature(feature);
    if (feature.hasGeometry()) {
      const QgsPointXY corner(feature.geometry().vertexAt(0));
      const QgsPointXY target = m_grid->snapToGrid(
          QgsPointXY(corner.x() + cursor.x() - m_from.x(), corner.y() + cursor.y() - m_from.y()));
      return QgsPointXY(target.x() - corner.x(), target.y() - corner.y());
    }
  }
  // Whole snap steps from the snapped click, not from the raw click (F046).
  const QgsPointXY from = snapPt(m_from), to = snapPt(cursor);
  return QgsPointXY(to.x() - from.x(), to.y() - from.y());
}

bool KaTrenchMoveTool::nudge(QKeyEvent* e) {
  if (!e || !m_layer) return false;
  int across = 0, along = 0;
  switch (e->key()) {
    case Qt::Key_Left: across = -1; break;
    case Qt::Key_Right: across = 1; break;
    case Qt::Key_Up: along = 1; break;
    case Qt::Key_Down: along = -1; break;
    default: return false;
  }
  if (m_mode == Mode::Single && FID_IS_NULL(m_selFid)) return false;
  // One grid cell along the (turned) grid's axes; 1 m east-west / north-south without a grid.
  const bool onGrid = m_grid && m_grid->isEnabled();
  const double step = onGrid ? m_grid->stepMeters() : 1.0;
  double dx = across * step, dy = along * step;
  if (onGrid) CanvasGridMath::toMap({0.0, 0.0, m_grid->config().rotationDeg}, across * step, along * step, &dx, &dy);
  const bool single = m_mode == Mode::Single;
  if (applyTranslate(single ? QgsFeatureIds{m_selFid} : QgsFeatureIds{}, dx, dy)) {
    if (single) rebuildSingleRubber(m_selFid, QgsPointXY(0, 0));
    emit statusMessage(QStringLiteral("%1 한 칸 옮김 — Ctrl+Z로 되돌립니다.")
                           .arg(single ? (m_selName.isEmpty() ? QStringLiteral("트렌치") : m_selName)
                                       : QStringLiteral("시굴격자 전체")));
  }
  // QgsMapCanvas pans on an arrow key unless the map tool ignores the event: ignored = used here.
  e->ignore();
  return true;
}
