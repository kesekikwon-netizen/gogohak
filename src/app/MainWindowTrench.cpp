// 시굴격자 and the map: the coordinate grid the trench window may lay ㄱ·ㄴ자 on, and handing a trench picked
// with 「도형 선택」 to the trench tool (사용자 요청 2026-10-20).
#include "MainWindow.h"
#include "KaCanvasGridOverlay.h"
#include "KaFeatureSelectTool.h"
#include "KaTrenchDialog.h"
#include "KaTrenchMoveTool.h"
#include "core/LayerOps.h"

#include <qgsmapcanvas.h>
#include <qgsproject.h>
#include <qgsvectorlayer.h>

// 「좌표격자에 맞추기」 (사용자 요청 2026-10-20): the trench window may lay ㄱ·ㄴ자 on the map's coordinate grid
// when it is a metre grid drawn in the survey area's CRS.
void MainWindow::syncTrenchCoordinateGrid(const QString& areaCrs) {
#if KA_HGIS_HAS_QGIS
  if (!m_trenchDlg) return;
  const bool usable = m_canvas && m_mapGrid && m_mapGrid->isEnabled() &&
                      m_mapGrid->config().type == KaCanvasGridOverlay::Type::ProjectedMeters &&
                      (areaCrs.isEmpty() || m_canvas->mapSettings().destinationCrs().authid() == areaCrs);
  const KaCanvasGridOverlay::Config cfg = m_mapGrid ? m_mapGrid->config() : KaCanvasGridOverlay::Config();
  m_trenchDlg->setCoordinateGrid(
      usable, TrenchGridSnap::Grid{cfg.originX, cfg.originY, cfg.rotationDeg, usable ? m_mapGrid->stepMeters() : 0.0});
#else
  Q_UNUSED(areaCrs);
#endif
}

// 「도형 선택」 picked one 시굴격자 trench: the trench tool moves it whole (drag, arrow keys, Delete) instead of
// showing its vertices one by one.
void MainWindow::pickTrenchFromSelection() {
#if KA_HGIS_HAS_QGIS
  auto* vl = LayerOps::findByLayerKey(QgsProject::instance(), QStringLiteral("trial_trench"));
  if (!vl || vl->selectedFeatureCount() != 1 || !m_canvas || m_canvas->mapTool() != m_featureSelectTool) return;
  const QgsFeatureId fid = *vl->selectedFeatureIds().constBegin();
  vl->removeSelection();
  activateTrenchTool(true);
  if (m_trenchMoveTool) m_trenchMoveTool->selectTrench(fid);
#endif
}
