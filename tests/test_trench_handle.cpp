// 시굴격자 트렌치를 쉽게 옮기기 (사용자 요청 2026-10-20): with the coordinate grid on, a dragged trench lands with its
// outer corner (first vertex) on a grid node; the arrow keys move the picked trench, or the whole grid, one grid cell
// (1 m without a grid) as one Ctrl+Z step, and the map does not pan; a trench can be picked from outside the tool.
#include <QtTest>

#include "app/KaCanvasGridOverlay.h"
#include "app/KaTrenchMoveTool.h"
#include "core/LayerOps.h"

#include <qgsapplication.h>
#include <qgsgeometry.h>
#include <qgsmapcanvas.h>
#include <qgsmapmouseevent.h>
#include <qgsmaptopixel.h>
#include <qgsproject.h>
#include <qgsvectordataprovider.h>
#include <qgsvectorlayer.h>

namespace {
struct Canvas {
  QgsMapCanvas canvas;
  KaCanvasGridOverlay* grid = nullptr;
  Canvas() {
    canvas.setRenderFlag(false);
    canvas.setDestinationCrs(QgsCoordinateReferenceSystem(QStringLiteral("EPSG:5186")));
    canvas.resize(640, 480);
    canvas.mapSettings().setOutputSize(QSize(640, 480));
    canvas.setExtent(QgsRectangle(200000, 450000, 200100, 450075));
    grid = new KaCanvasGridOverlay(&canvas);
    KaCanvasGridOverlay::Config config;
    config.enabled = true;
    config.stepMeters = 5.0;
    grid->setConfig(config);
  }
};

// One ㄴ-shaped trench whose outer corner (first vertex) is off the 5 m nodes.
QgsVectorLayer* lLayer(const QStringList& wkts) {
  auto* layer = new QgsVectorLayer(
      QStringLiteral("Polygon?crs=EPSG:5186&field=name:string&field=width:double&field=length:double"),
      QStringLiteral("시굴격자"), QStringLiteral("memory"));
  LayerOps::markSurveyLayer(layer, QStringLiteral("trial_trench"));
  int n = 1;
  for (const QString& wkt : wkts) {
    QgsFeature feature(layer->fields());
    feature.setGeometry(QgsGeometry::fromWkt(wkt));
    feature.setAttribute(QStringLiteral("name"), QStringLiteral("Tr-%1").arg(n++));
    layer->dataProvider()->addFeature(feature);
  }
  layer->updateExtents();
  QgsProject::instance()->addMapLayer(layer);
  return layer;
}

QString lAt(double x, double y) {
  return QStringLiteral("POLYGON((%1 %2,%3 %2,%3 %4,%5 %4,%5 %6,%1 %6,%1 %2))")
      .arg(x, 0, 'f', 3).arg(y, 0, 'f', 3).arg(x + 15, 0, 'f', 3).arg(y + 2, 0, 'f', 3).arg(x + 2, 0, 'f', 3)
      .arg(y + 15, 0, 'f', 3);
}

void mouse(QgsMapTool& tool, QgsMapCanvas* canvas, QEvent::Type type, const QgsPointXY& at) {
  const QgsPointXY px = canvas->getCoordinateTransform()->transform(at);
  const Qt::MouseButtons held = type == QEvent::MouseButtonRelease ? Qt::NoButton : Qt::MouseButtons(Qt::LeftButton);
  QgsMapMouseEvent event(canvas, type, QPoint(qRound(px.x()), qRound(px.y())),
                         type == QEvent::MouseMove ? Qt::NoButton : Qt::LeftButton, held);
  event.setMapPoint(at);
  if (type == QEvent::MouseButtonPress) tool.canvasPressEvent(&event);
  else if (type == QEvent::MouseMove) tool.canvasMoveEvent(&event);
  else tool.canvasReleaseEvent(&event);
}

QgsPointXY cornerOf(QgsVectorLayer* layer, QgsFeatureId fid) {
  const QgsGeometry geometry = layer->getFeature(fid).geometry();
  return QgsPointXY(geometry.vertexAt(0));
}
}  // namespace

class TestTrenchHandle : public QObject {
  Q_OBJECT
private slots:
  void cleanup() { QgsProject::instance()->removeAllMapLayers(); }

  void gridDragPutsTheCornerOnANode() {
    Canvas c;
    auto* layer = lLayer({lAt(200011.3, 450012.6)});
    const QgsFeatureId fid = *layer->allFeatureIds().constBegin();
    KaTrenchMoveTool tool(&c.canvas);
    tool.setLayer(layer);
    tool.setGridOverlay(c.grid);
    tool.setMode(KaTrenchMoveTool::Mode::Single);
    mouse(tool, &c.canvas, QEvent::MouseButtonPress, QgsPointXY(200012.0, 450020.0));
    mouse(tool, &c.canvas, QEvent::MouseMove, QgsPointXY(200019.2, 450023.1));
    mouse(tool, &c.canvas, QEvent::MouseButtonRelease, QgsPointXY(200019.2, 450023.1));
    const QgsPointXY corner = cornerOf(layer, fid);  // 200018.5, 450015.7 rounded to the 5 m nodes
    QVERIFY2(qAbs(corner.x() - 200020.0) < 1e-6 && qAbs(corner.y() - 450015.0) < 1e-6,
             qPrintable(QStringLiteral("%1 %2").arg(corner.x(), 0, 'f', 3).arg(corner.y(), 0, 'f', 3)));
    QCOMPARE(layer->undoStack()->count(), 1);
  }

  void arrowKeysMoveThePickedTrenchOneCell() {
    Canvas c;
    auto* layer = lLayer({lAt(200020, 450015), lAt(200050, 450015)});
    QgsFeatureId first = FID_NULL;  // the trench at 200020 (feature ids come in no set order)
    for (QgsFeatureId fid : layer->allFeatureIds())
      if (cornerOf(layer, fid) == QgsPointXY(200020, 450015)) first = fid;
    KaTrenchMoveTool tool(&c.canvas);
    tool.setLayer(layer);
    tool.setGridOverlay(c.grid);
    tool.setMode(KaTrenchMoveTool::Mode::Single);
    c.canvas.setMapTool(&tool);
    tool.selectTrench(first);
    QCOMPARE(tool.selectedTrench(), first);
    const QgsRectangle extent = c.canvas.extent();
    QTest::keyClick(&c.canvas, Qt::Key_Right);
    QTest::keyClick(&c.canvas, Qt::Key_Up);
    QCOMPARE(cornerOf(layer, first).toString(3), QgsPointXY(200025, 450020).toString(3));
    QCOMPARE(c.canvas.extent(), extent);  // the map did not pan
    for (QgsFeatureId fid : layer->allFeatureIds())
      if (fid != first) QCOMPARE(cornerOf(layer, fid), QgsPointXY(200050, 450015));  // only the picked one moved
    QCOMPARE(layer->undoStack()->count(), 2);
    layer->undoStack()->undo();
    QCOMPARE(cornerOf(layer, first), QgsPointXY(200025, 450015));
  }

  void arrowKeysMoveTheWholeGridWithoutAGridOneMetre() {
    Canvas c;
    c.grid->setEnabled(false);
    auto* layer = lLayer({lAt(200020, 450015), lAt(200050, 450015)});
    KaTrenchMoveTool tool(&c.canvas);
    tool.setLayer(layer);
    tool.setGridOverlay(c.grid);
    tool.setMode(KaTrenchMoveTool::Mode::Whole);
    c.canvas.setMapTool(&tool);
    QTest::keyClick(&c.canvas, Qt::Key_Left);
    QList<QgsPointXY> corners;
    for (QgsFeatureId fid : layer->allFeatureIds()) corners << cornerOf(layer, fid);
    QVERIFY(corners.contains(QgsPointXY(200019, 450015)) && corners.contains(QgsPointXY(200049, 450015)));
    QCOMPARE(layer->undoStack()->count(), 1);
  }
};

int main(int argc, char** argv) {
  QgsApplication app(argc, argv, true);
  const QString prefix = qEnvironmentVariable(
      "QGIS_PREFIX_PATH", QFile::exists(QStringLiteral("A:/OSGeo4W/apps/qgis-dev"))
                              ? QStringLiteral("A:/OSGeo4W/apps/qgis-dev")
                              : QStringLiteral("C:/OSGeo4W/apps/qgis-dev"));
  QgsApplication::setPrefixPath(prefix, true);
  QgsApplication::setPluginPath(prefix + QStringLiteral("/plugins"));
  QgsApplication::initQgis();
  TestTrenchHandle tc;
  const int rc = QTest::qExec(&tc, argc, argv);
  QgsApplication::exitQgis();
  return rc;
}

#include "test_trench_handle.moc"
