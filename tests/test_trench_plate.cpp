// 「조용한 도판」 시굴격자 속성 (2차 보드 판 5 새 표시): the 시굴 비율 on a 0 … 2×목표 bar with the 목표 marked, and a dial
// beside 회전(방위) whose needle shows the turn from north. 새 모양 only; the stock window keeps its sentence.
#include <QtTest>

#include <QDoubleSpinBox>
#include <QLabel>

#include <qgsapplication.h>
#include <qgsgeometry.h>
#include <qgsrectangle.h>

#include "app/KaAzimuthDial.h"
#include "app/KaTargetMeter.h"
#include "app/KaTrenchDialog.h"

namespace {
QByteArray squareWkb(double size) { return QgsGeometry::fromRect(QgsRectangle(0, 0, size, size)).asWkb(); }
bool ink(const QColor& c) { return c.lightness() < 70; }
}  // namespace

class TestTrenchPlate : public QObject {
  Q_OBJECT
private slots:
  void cleanup() { qApp->setProperty("kaPaperLook", QVariant()); }

  // 10.2 % against 목표 10 %: the fill reaches just past the middle, where the 목표 mark stands.
  void meterFillsToTheRatioAndMarksTheTarget() {
    KaTargetMeter meter;
    meter.resize(512, meter.sizeHint().height());
    meter.setTarget(10.0);
    meter.setValue(10.2);
    const QImage image = meter.grab().toImage();
    QVERIFY(ink(image.pixelColor(128, 2)));    // filled
    QVERIFY(!ink(image.pixelColor(384, 2)));   // the rest of the track
    QVERIFY(ink(image.pixelColor(256, 8)));    // the 목표 mark below the track
  }

  // The needle points along the turn: up at 0°, right at 90°.
  void dialNeedleFollowsTheTurn() {
    KaAzimuthDial dial;
    dial.resize(40, 40);
    dial.setAzimuth(0.0);
    QImage image = dial.grab().toImage();
    QVERIFY(ink(image.pixelColor(20, 13)));
    QVERIFY(!ink(image.pixelColor(27, 20)));
    dial.setAzimuth(90.0);
    image = dial.grab().toImage();
    QVERIFY(ink(image.pixelColor(27, 20)));
    QVERIFY(!ink(image.pixelColor(20, 13)));
    QCOMPARE(KaAzimuthDial::note(0.0), QStringLiteral("위가 북쪽입니다"));
    QCOMPARE(KaAzimuthDial::note(90.0), QStringLiteral("북에서 시계 방향 90°"));
  }

  void paperWindowCarriesTheMeterAndTheDial() {
    qApp->setProperty("kaPaperLook", true);
    KaTrenchDialog dialog;
    dialog.setArea(squareWkb(100), 10000.0);
    auto* dial = dialog.findChild<KaAzimuthDial*>();
    QVERIFY(dialog.findChild<KaTargetMeter*>() && dial);
    QDoubleSpinBox* azimuth = nullptr;
    for (auto* spin : dialog.findChildren<QDoubleSpinBox*>())
      if (spin->suffix().contains(QStringLiteral("°"))) azimuth = spin;
    azimuth->setValue(30.0);
    QCOMPARE(dial->azimuth(), 30.0);
    QTRY_VERIFY_WITH_TIMEOUT(!dialog.planPending(), 20000);
    qApp->setProperty("kaPaperLook", QVariant());
    KaTrenchDialog stock;
    QVERIFY(!stock.findChild<KaTargetMeter*>() && !stock.findChild<KaAzimuthDial*>());
  }
};

int main(int argc, char** argv) {
  QgsApplication app(argc, argv, false);
  const QString prefix = qEnvironmentVariable(
      "QGIS_PREFIX_PATH", QFile::exists(QStringLiteral("A:/OSGeo4W/apps/qgis-dev"))
                              ? QStringLiteral("A:/OSGeo4W/apps/qgis-dev")
                              : QStringLiteral("C:/OSGeo4W/apps/qgis-dev"));
  QgsApplication::setPrefixPath(prefix, true);
  QgsApplication::setPluginPath(prefix + QStringLiteral("/plugins"));
  QgsApplication::initQgis();
  TestTrenchPlate tc;
  const int rc = QTest::qExec(&tc, argc, argv);
  QgsApplication::exitQgis();
  return rc;
}

#include "test_trench_plate.moc"
