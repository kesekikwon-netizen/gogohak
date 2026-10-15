// 도형선택에서 Ctrl+C · Ctrl+V 로 지적 필지를 같은 자리에
// 복사한다(docs/intent/2026-10-10-cadastral-own-section-copy-paste.md). 복사·붙이기 자체는 test_parcel_copy.cpp.
#include <QApplication>
#include <QKeyEvent>
#include <QtTest>

#include <qgsapplication.h>
#include <qgsmapcanvas.h>
#include <qgsmaptoolpan.h>
#include <qgsproject.h>
#include <qgsrectangle.h>
#include <qgsvectorlayer.h>

#include "app/KaCaptureMapTool.h"
#include "app/KaFeatureSelectTool.h"
#include "app/KaParcelCopyKeys.h"
#include "core/LayerOps.h"
#include "core/ParcelClipboard.h"
#include "core/SnapTraceOps.h"
#include "parcel_copy_fixture.h"
#include <QFile>
#include <qgsmapmouseevent.h>
#include <qgsmaptopixel.h>

using namespace ParcelCopyFixture;

class ParcelCopyKeysTest : public QObject {
  Q_OBJECT
 private slots:
  void holdingCtrlVPastesOnce() {
    QgsProject* project = QgsProject::instance();
    project->clear();
    QgsVectorLayer* received = receivedMap(*project, QStringLiteral("EPSG:5187"), square(200000, 450000, 10));
    QgsMapCanvas canvas;
    canvas.setRenderFlag(false);
    canvas.setLayers({received});
    KaParcelCopyKeys keys(&canvas);
    KaFeatureSelectTool tool(&canvas);
    canvas.setMapTool(&tool);
    received->selectByIds({firstFid(received)});
    pressCtrl(canvas, Qt::Key_C);
    pressCtrl(canvas, Qt::Key_V);
    for (int i = 0; i < 5; ++i) {  // 키를 누르고 있으면 오는 반복 입력
      QKeyEvent repeat(QEvent::KeyPress, Qt::Key_V, Qt::ControlModifier, QString(), true);
      QApplication::sendEvent(&canvas, &repeat);
    }
    QCOMPARE(int(featuresOf(ParcelClipboard::copyLayer(project)).size()), 1);
    canvas.setLayers({});
    project->clear();
  }

  void ctrlCAndCtrlVInSelectToolPasteWhereTheParcelWas() {
    QgsProject* project = QgsProject::instance();
    project->clear();
    QgsVectorLayer* received = receivedMap(*project, QStringLiteral("EPSG:5187"), square(200000, 450000, 10));

    QgsMapCanvas canvas;
    canvas.setRenderFlag(false);
    canvas.setDestinationCrs(QgsCoordinateReferenceSystem(QStringLiteral("EPSG:5187")));
    canvas.resize(640, 480);
    canvas.setExtent(QgsRectangle(199990, 449990, 200020, 450020));
    canvas.setLayers({received});
    KaParcelCopyKeys keys(&canvas);
    KaFeatureSelectTool tool(&canvas);
    canvas.setMapTool(&tool);
    received->selectByIds({firstFid(received)});

    pressCtrl(canvas, Qt::Key_C);
    pressCtrl(canvas, Qt::Key_V);

    QgsVectorLayer* copy = ParcelClipboard::copyLayer(project);
    QVERIFY2(copy, "Ctrl+V in 도형선택 pastes the copied parcel");
    QCOMPARE(int(featuresOf(copy).size()), 1);
    canvas.setLayers({});
    project->clear();
  }

  void ctrlCAndCtrlVDoNothingOutsideSelectTool() {
    QgsProject* project = QgsProject::instance();
    project->clear();
    QgsVectorLayer* received = receivedMap(*project, QStringLiteral("EPSG:5187"), square(200000, 450000, 10));
    addParcel(received, square(200020, 450000, 10), QStringLiteral("2222222222222222222"));
    QgsFeatureId parcelA = -1;  // 왼쪽 필지
    QgsFeatureId parcelB = -1;  // 오른쪽 필지
    QgsFeature feature;
    for (auto it = received->getFeatures(); it.nextFeature(feature);)
      (feature.geometry().boundingBox().xMinimum() < 200010 ? parcelA : parcelB) = feature.id();

    QgsMapCanvas canvas;
    canvas.setRenderFlag(false);
    canvas.setDestinationCrs(QgsCoordinateReferenceSystem(QStringLiteral("EPSG:5187")));
    canvas.resize(640, 480);
    canvas.setExtent(QgsRectangle(199990, 449990, 200040, 450020));
    canvas.setLayers({received});
    KaParcelCopyKeys keys(&canvas);
    KaFeatureSelectTool tool(&canvas);
    canvas.setMapTool(&tool);
    received->selectByIds({parcelA});
    pressCtrl(canvas, Qt::Key_C);  // 도형선택에서는 A를 담는다
    pressCtrl(canvas, Qt::Key_V);
    QVERIFY2(ParcelClipboard::copyLayer(project), "the select tool copies and pastes A first");

    QgsMapToolPan pan(&canvas);
    canvas.setMapTool(&pan);
    received->selectByIds({parcelB});
    pressCtrl(canvas, Qt::Key_C);  // 도형선택이 아니므로 B를 담지 않는다
    pressCtrl(canvas, Qt::Key_V);  // 붙이지도 않는다

    canvas.setMapTool(&tool);
    pressCtrl(canvas, Qt::Key_V);  // 담긴 것은 아직 A뿐이다
    QgsVectorLayer* copy = ParcelClipboard::copyLayer(project);
    QVERIFY(copy);
    const QList<QgsFeature> copies = featuresOf(copy);
    QCOMPARE(int(copies.size()), 2);  // A 두 번, B는 없다
    for (const QgsFeature& copied : copies)
      QVERIFY(copied.geometry().centroid().distance(square(200000, 450000, 10).centroid()) < 0.01);
    canvas.setLayers({});
    project->clear();
  }

  void snapTraceShowsNearbyVerticesOnlyWhenCloseAndFillsSkippedVertex() {
    QgsProject* project = QgsProject::instance();
    project->clear();
    auto* cad = new QgsVectorLayer(QStringLiteral("LineString?crs=EPSG:5187"), QStringLiteral("사업자도면"),
                                   QStringLiteral("memory"));
    LayerOps::markReferenceLayer(cad);
    cad->setCustomProperty(QStringLiteral("ka_hgis/cad_drawing"), QStringLiteral("cad1"));
    QgsFeature f(cad->fields());
    f.setGeometry(QgsGeometry::fromPolylineXY(
        {QgsPointXY(200000, 450000), QgsPointXY(200010, 450001.2), QgsPointXY(200020, 450000)}));
    cad->dataProvider()->addFeature(f);
    cad->updateExtents();
    project->addMapLayer(cad);

    QgsMapCanvas canvas;
    canvas.setRenderFlag(false);
    canvas.setDestinationCrs(QgsCoordinateReferenceSystem(QStringLiteral("EPSG:5187")));
    canvas.resize(640, 480);
    canvas.mapSettings().setOutputSize(QSize(640, 480));
    canvas.setExtent(QgsRectangle(199980, 449980, 200060, 450040));
    canvas.setLayers({cad});
    auto px = [&](double x, double y) {
      const QgsPointXY p = canvas.getCoordinateTransform()->transform(x, y);
      return QPoint(qRound(p.x()), qRound(p.y()));
    };

    KaCaptureMapTool tool(&canvas);
    tool.setMode(KaCaptureMapTool::Mode::Line);
    tool.setSnapTrace(true);

    QgsMapMouseEvent farMove(&canvas, QEvent::MouseMove, px(200050, 450035), Qt::NoButton, Qt::NoButton);
    tool.canvasMoveEvent(&farMove);
    QCOMPARE(tool.previewMarkerCount(), 0);  // 마우스가 멀리 있을 때는 자석점 미리보기를 숨겨 어지럽지 않음

    QgsMapMouseEvent nearMove(&canvas, QEvent::MouseMove, px(200010, 450002), Qt::NoButton, Qt::NoButton);
    tool.canvasMoveEvent(&nearMove);
    QVERIFY2(tool.previewMarkerCount() >= 1, "마우스가 가까이 갔을 때 주변 자석점이 미리 표시되어야 한다");

    QgsMapMouseEvent p1(&canvas, QEvent::MouseButtonPress, px(200000, 450000), Qt::LeftButton, Qt::LeftButton);
    tool.canvasPressEvent(&p1);
    QgsMapMouseEvent p2(&canvas, QEvent::MouseButtonPress, px(200020, 450000), Qt::LeftButton, Qt::LeftButton);
    tool.canvasPressEvent(&p2);
    QCOMPARE(tool.pointCount(), 3);  // 빠뜨린 중간 자석점 (200010, 450001.2)가 자동으로 채워짐

    QFile sketchSrc(QStringLiteral("src/app/KaDrawSketchTools.cpp"));
    if (sketchSrc.open(QIODevice::ReadOnly | QIODevice::Text)) {
      const QString code = QString::fromUtf8(sketchSrc.readAll());
      QVERIFY(code.contains(QLatin1String("actSnapTrace")) && code.contains(QStringLiteral("선 따라 자석")));
    }
    canvas.setLayers({});
    project->clear();
  }
};

int main(int argc, char** argv) {
  QgsApplication app(argc, argv, false);
  QgsApplication::setPrefixPath(qEnvironmentVariable("QGIS_PREFIX_PATH"), true);
  QgsApplication::initQgis();
  ParcelCopyKeysTest test;
  const int rc = QTest::qExec(&test, argc, argv);
  QgsApplication::exitQgis();
  return rc;
}
#include "test_parcel_copy_keys.moc"
