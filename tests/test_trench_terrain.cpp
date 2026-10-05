// 시굴격자 사면 방향 (사용자 요청 2026-10-05): without a DEM the slope of the survey area comes from the 수치지형도
// contours (their 표고 field), so 시굴 trenches run from the high side to the low side across the contours.
#include <QtTest>

#include "core/TrenchGridGenerator.h"
#include "core/TrenchTerrain.h"

#include <qgsapplication.h>
#include <qgsfeature.h>
#include <qgsgeometry.h>
#include <qgsproject.h>
#include <qgsvectordataprovider.h>
#include <qgsvectorlayer.h>

#include <cmath>

namespace {
const QgsRectangle kArea(203050, 374030, 203150, 374090);  // a 100 × 60 m survey area (EPSG:5187)

// Straight contours every 5 m with the 표고 rising 1 m per line: east–west lines rising north, or north–south
// lines rising east.
QgsVectorLayer* contours(bool risingNorth, const QString& field = QStringLiteral("elevation")) {
  auto* layer = new QgsVectorLayer(QStringLiteral("LineString?crs=EPSG:5187&field=%1:double").arg(field),
                                   QStringLiteral("수치지형도"), QStringLiteral("memory"));
  QgsFeatureList features;
  for (int k = 0; k <= 40; ++k) {
    QgsFeature f(layer->fields());
    const double along = 5.0 * k;
    f.setGeometry(risingNorth ? QgsGeometry::fromPolylineXY({{202950, 373950 + along}, {203250, 373950 + along}})
                              : QgsGeometry::fromPolylineXY({{202950 + along, 373950}, {202950 + along, 374150}}));
    f.setAttribute(0, 100.0 + k);
    features << f;
  }
  layer->dataProvider()->addFeatures(features);
  return layer;
}

QByteArray area() { return QgsGeometry::fromRect(kArea).asWkb(); }
double turn(double a, double b) { return std::abs(std::remainder(a - b, 360.0)); }
}  // namespace

class TestTrenchTerrain : public QObject {
  Q_OBJECT
private slots:
  void contoursRisingNorthPointNorth() {
    QgsProject project;
    project.addMapLayer(contours(true));
    const auto samples = TrenchTerrain::contourSamples(&project, area(), QStringLiteral("EPSG:5187"));
    QVERIFY(samples.size() >= 3);
    const auto aspect = TrenchGridGenerator::upslopeAspect(samples);
    QVERIFY(aspect.valid);
    QVERIFY2(turn(aspect.azimuthDeg, 0.0) < 2.0, qPrintable(QString::number(aspect.azimuthDeg)));
  }

  void contoursRisingEastPointEast() {
    QgsProject project;
    project.addMapLayer(contours(false));
    const auto aspect =
        TrenchGridGenerator::upslopeAspect(TrenchTerrain::contourSamples(&project, area(), QStringLiteral("EPSG:5187")));
    QVERIFY(aspect.valid);
    QVERIFY2(turn(aspect.azimuthDeg, 90.0) < 2.0, qPrintable(QString::number(aspect.azimuthDeg)));
  }

  // Lines without a 표고 (roads, buildings) are not contours.
  void linesWithoutElevationAreNotContours() {
    QgsProject project;
    project.addMapLayer(contours(true, QStringLiteral("name_kr")));
    QVERIFY(TrenchTerrain::contourSamples(&project, area(), QStringLiteral("EPSG:5187")).empty());
  }
};

int main(int argc, char** argv) {
  QgsApplication app(argc, argv, false);
  const QString prefix = qEnvironmentVariable(
      "QGIS_PREFIX_PATH", QFile::exists(QStringLiteral("A:/OSGeo4W/apps/qgis-dev"))
                              ? QStringLiteral("A:/OSGeo4W/apps/qgis-dev")
                              : QStringLiteral("C:/OSGeo4W/apps/qgis-dev"));
  QgsApplication::setPrefixPath(prefix, true);
  QgsApplication::initQgis();
  TestTrenchTerrain tc;
  const int rc = QTest::qExec(&tc, argc, argv);
  QgsApplication::exitQgis();
  return rc;
}

#include "test_trench_terrain.moc"
