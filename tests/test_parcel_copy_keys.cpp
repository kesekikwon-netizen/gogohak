// 도형선택에서 Ctrl+C · Ctrl+V 로 지적 필지를 같은 자리에 복사한다(docs/intent/2026-10-10-cadastral-own-section-copy-paste.md).
// 복사·붙이기 자체는 test_parcel_copy.cpp.
#include <QApplication>
#include <QKeyEvent>
#include <QtTest>

#include <qgsapplication.h>
#include <qgsmapcanvas.h>
#include <qgsmaptoolpan.h>
#include <qgsproject.h>
#include <qgsrectangle.h>
#include <qgsvectorlayer.h>

#include "app/KaFeatureSelectTool.h"
#include "app/KaParcelCopyKeys.h"
#include "core/ParcelClipboard.h"
#include "parcel_copy_fixture.h"

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
