// 좌표격자를 도면에 (사용자 요청 2026-10-20): the drawing's coordinate grid can follow the map's coordinate grid,
// the one the 시굴격자 is laid on: same step, lines through the same nodes, drawn as full lines with the 도곽 numbers.
#include <QtTest>
#include <QFile>
#include <QImage>

#include <qgsapplication.h>
#include <qgslayoutexporter.h>
#include <qgslayoutitemmap.h>
#include <qgslayoutitemmapgrid.h>
#include <qgsprintlayout.h>
#include <qgsproject.h>

#include "core/LayoutService.h"

namespace {
constexpr double kDpi = 200.0;

QgsLayoutItemMap* addMap(QgsPrintLayout& layout) {
  auto* map = new QgsLayoutItemMap(&layout);
  layout.addLayoutItem(map);
  map->attemptSetSceneRect(QRectF(10., 10., 160., 160.));
  map->setCrs(QgsCoordinateReferenceSystem(QStringLiteral("EPSG:5186")));
  map->setKeepLayerSet(true);
  map->setFrameEnabled(false);
  map->setExtent(QgsRectangle(219700, 450000, 219760, 450060));  // 60 m over 160 mm
  return map;
}

// Dark pixels down the map at map x (EPSG:5186 metres).
int inkAt(const QImage& page, double mapX) {
  const double mm = 10.0 + (mapX - 219700.0) / 60.0 * 160.0;
  const int column = qRound(mm / 25.4 * kDpi);
  int dark = 0;
  for (int row = qRound(30.0 / 25.4 * kDpi); row < qRound(150.0 / 25.4 * kDpi); ++row)
    for (int dx = -2; dx <= 2; ++dx)
      if (qGray(page.pixel(column + dx, row)) < 200) {
        ++dark;
        break;
      }
  return dark;
}
}  // namespace

class LayoutMapGridTest : public QObject {
  Q_OBJECT
private slots:
  void gridFollowsTheMapsStepAndNodes() {
    QgsProject project;
    QgsPrintLayout layout(&project);
    layout.initializeDefaults();
    QgsLayoutItemMap* map = addMap(layout);
    // The map grid runs 15 m apart through 219707 / 450002, not through the CRS origin.
    LayoutService::applyMapCoordinateGrid(map, 15.0, 219707.0, 450002.0, true);
    QCOMPARE(map->grids()->size(), 1);
    QgsLayoutItemMapGrid* grid = map->grids()->asList().front();
    QCOMPARE(grid->intervalX(), 15.0);
    QCOMPARE(grid->intervalY(), 15.0);
    QCOMPARE(grid->offsetX(), 2.0);
    QCOMPARE(grid->offsetY(), 2.0);
    QCOMPARE(grid->style(), Qgis::MapGridStyle::Lines);
    QVERIFY(grid->annotationEnabled());

    // On paper: a line at 219722, none between the lines (219714.5).
    QgsLayoutExporter exporter(&layout);
    const QImage page = exporter.renderPageToImage(0, QSize(), kDpi);
    QVERIFY(!page.isNull());
    QVERIFY2(inkAt(page, 219722.0) > 200, qPrintable(QString::number(inkAt(page, 219722.0))));
    QVERIFY2(inkAt(page, 219714.5) < 20, qPrintable(QString::number(inkAt(page, 219714.5))));
  }

  // Turning the plain 도곽 grid back on puts it back at the CRS origin with crosses.
  void plainFrameGridStaysOnTheCrsOrigin() {
    QgsProject project;
    QgsPrintLayout layout(&project);
    layout.initializeDefaults();
    QgsLayoutItemMap* map = addMap(layout);
    LayoutService::applyMapCoordinateGrid(map, 15.0, 219707.0, 450002.0, true);
    LayoutService::applySurveyFrameGrid(map, 20.0, true, true);
    QCOMPARE(map->grids()->size(), 1);
    QgsLayoutItemMapGrid* grid = map->grids()->asList().front();
    QCOMPARE(grid->offsetX(), 0.0);
    QCOMPARE(grid->style(), Qgis::MapGridStyle::LineCrosses);
  }
};

int main(int argc, char** argv) {
  QgsApplication app(argc, argv, false);
  const QString prefix = qEnvironmentVariable(
      "QGIS_PREFIX_PATH", QFile::exists(QStringLiteral("A:/OSGeo4W/apps/qgis-dev")) ? QStringLiteral("A:/OSGeo4W/apps/qgis-dev")
                                                                                    : QStringLiteral("C:/OSGeo4W/apps/qgis-dev"));
  QgsApplication::setPrefixPath(prefix, true);
  QgsApplication::initQgis();
  LayoutMapGridTest test;
  const int result = QTest::qExec(&test, argc, argv);
  QgsApplication::exitQgis();
  return result;
}
#include "test_layout_map_grid.moc"
