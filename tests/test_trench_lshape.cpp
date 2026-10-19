// ㄱ·ㄴ자 시굴트렌치 (사용자 요청 2026-10-19): two 15 m arms, 2 m wide, laid inside the survey area
// at the 시굴 10% area like the straight grid. One trench is one closed 7-point polygon, so it moves,
// deletes and saves as one feature; width × length (centreline) is its area.
#include <cmath>
#include <memory>

#include <QtTest>
#include <QCoreApplication>
#include <QTemporaryDir>

#include "core/TrenchGridGenerator.h"
#include "core/TrenchPlanCache.h"

#include <gdal_priv.h>
#include <ogr_api.h>
#include <ogr_geometry.h>
#include <ogrsf_frmts.h>

namespace {
struct GeometryDeleter {
  void operator()(OGRGeometry* geometry) const { OGRGeometryFactory::destroyGeometry(geometry); }
};
using Geometry = std::unique_ptr<OGRGeometry, GeometryDeleter>;
using Dataset = std::unique_ptr<GDALDataset, decltype(&GDALClose)>;

QByteArray wktWkb(const QString& wkt) {
  OGRGeometry* geometry = nullptr;
  OGRGeometryFactory::createFromWkt(wkt.toLatin1().constData(), nullptr, &geometry);
  Geometry owner(geometry);
  if (!owner) return {};
  QByteArray result(int(owner->WkbSize()), '\0');
  owner->exportToWkb(wkbNDR, reinterpret_cast<unsigned char*>(result.data()));
  return result;
}

QByteArray squareWkb(double x0, double y0, double x1, double y1) {
  return wktWkb(QStringLiteral("POLYGON((%1 %2,%3 %2,%3 %4,%1 %4,%1 %2))")
                    .arg(x0, 0, 'f', 6).arg(y0, 0, 'f', 6).arg(x1, 0, 'f', 6).arg(y1, 0, 'f', 6));
}

Geometry readGeometry(const QByteArray& wkb) {
  OGRGeometry* geometry = nullptr;
  OGRGeometryFactory::createFromWkb(wkb.constData(), nullptr, &geometry, size_t(wkb.size()));
  return Geometry(geometry);
}

OGRPolygon cellPolygon(const TrenchGridGenerator::Cell& cell) {
  OGRLinearRing ring;
  for (const auto& point : cell.ring) ring.addPoint(point.first, point.second);
  OGRPolygon polygon;
  polygon.addRing(&ring);
  return polygon;
}

double edge(const TrenchGridGenerator::Cell& cell, int from, int to) {
  return std::hypot(cell.ring[to].first - cell.ring[from].first, cell.ring[to].second - cell.ring[from].second);
}

TrenchGridGenerator::Spec lSpec() {
  TrenchGridGenerator::Spec spec;
  spec.originX = 200000.0;
  spec.originY = 450000.0;
  spec.trenchWidth = 2.0;
  spec.armLength = 15.0;
  spec.balkWidth = 5.0;
  return spec;
}
}  // namespace

class TestTrenchLShape : public QObject {
  Q_OBJECT
private slots:
  void oneLHasTwoFifteenMetreArms();
  void tenPercentFillStaysInsideWithoutOverlap_data();
  void tenPercentFillStaysInsideWithoutOverlap();
  void tenPercentFillSpreadsOverTheArea();
  void brokenLIsRejected();
  void lTrenchSavesAsOnePolygon();
  void planCacheKeepsStraightAndLApart();
};

// At 0° the corner is south-west: one arm runs east, the other north (ㄴ).
void TestTrenchLShape::oneLHasTwoFifteenMetreArms() {
  const auto cells = TrenchGridGenerator::build(lSpec());
  QCOMPARE(int(cells.size()), 1);
  const auto& cell = cells.front();
  QCOMPARE(int(cell.ring.size()), 7);
  QCOMPARE(cell.arm, 15.0);
  QCOMPARE(cell.width, 2.0);
  QVERIFY(std::abs(cell.length - 28.0) < 1e-9);  // centreline 14 + 14: width × length = area
  QVERIFY(std::abs(edge(cell, 0, 1) - 15.0) < 1e-9);
  QVERIFY(std::abs(edge(cell, 0, 5) - 15.0) < 1e-9);
  QVERIFY(cell.ring[1].first > cell.ring[0].first);    // east arm
  QVERIFY(cell.ring[5].second > cell.ring[0].second);  // north arm
  OGRPolygon polygon = cellPolygon(cell);
  QVERIFY(polygon.IsValid());
  QVERIFY2(std::abs(polygon.get_Area() - 56.0) < 1e-6, qPrintable(QString::number(polygon.get_Area())));
  QVERIFY(std::abs(TrenchGridGenerator::totalArea(cells) - 56.0) < 1e-9);
  QVERIFY(TrenchGridGenerator::cellsValid(cells));
}

void TestTrenchLShape::tenPercentFillStaysInsideWithoutOverlap_data() {
  QTest::addColumn<QByteArray>("areaWkb");
  QTest::addColumn<double>("azimuth");
  QTest::newRow("5187-square") << squareWkb(200000, 450000, 200200, 450200) << 0.0;
  QTest::newRow("rotated-slope") << squareWkb(200000, 450000, 200200, 450200) << 30.0;
  QTest::newRow("concave") << wktWkb(QStringLiteral("POLYGON((0 0,160 0,160 60,60 60,60 160,0 160,0 0))")) << 0.0;
}

void TestTrenchLShape::tenPercentFillStaysInsideWithoutOverlap() {
  QFETCH(QByteArray, areaWkb);
  QFETCH(double, azimuth);
  const auto plan = TrenchGridGenerator::buildForTargetRatio(areaWkb, 10.0, 2.0, azimuth, 15.0);
  QVERIFY2(!plan.cells.empty(), qPrintable(plan.error));
  QVERIFY(plan.error.isEmpty());
  QVERIFY(TrenchGridGenerator::cellsValid(plan.cells));
  Geometry area = readGeometry(areaWkb);
  QVERIFY(area);
  Geometry united;
  double sum = 0.0, longest = 0.0;
  for (const auto& cell : plan.cells) {
    QCOMPARE(int(cell.ring.size()), 7);
    QCOMPARE(cell.width, 2.0);
    QVERIFY2(cell.arm <= 15.0 + 1e-9 && cell.arm >= 13.0, qPrintable(QString::number(cell.arm)));
    QVERIFY(std::abs(edge(cell, 0, 1) - cell.arm) < 1e-7 && std::abs(edge(cell, 0, 5) - cell.arm) < 1e-7);
    OGRPolygon polygon = cellPolygon(cell);
    Geometry outside(polygon.Difference(area.get()));
    QVERIFY(outside);
    QVERIFY2(OGR_G_Area(OGRGeometry::ToHandle(outside.get())) < 1e-7, "the whole trench stays in the area");
    sum += polygon.get_Area();
    longest = std::max(longest, cell.arm);
    united.reset(united ? united->Union(&polygon) : polygon.clone());
    QVERIFY(united);
  }
  const double unionArea = OGR_G_Area(OGRGeometry::ToHandle(united.get()));
  const double targetArea = OGR_G_Area(OGRGeometry::ToHandle(area.get())) * 0.10;
  QVERIFY2(std::abs(sum - unionArea) < 1e-6, "trenches must not overlap");
  QVERIFY2(std::abs(unionArea - targetArea) < std::max(1e-6, targetArea * 1e-8),
           qPrintable(QStringLiteral("actual %1, target %2").arg(unionArea, 0, 'g', 17).arg(targetArea, 0, 'g', 17)));
  QVERIFY(std::abs(TrenchGridGenerator::totalArea(plan.cells) - targetArea) < std::max(1e-6, targetArea * 1e-8));
  QVERIFY(std::abs(plan.ratioPct - 10.0) < 1e-7);
  QCOMPARE(plan.arm, longest);
  qInfo() << "ㄱ·ㄴ자" << plan.cells.size() << "개 · 팔" << plan.arm << "m · 둑" << plan.balk << "m";
}

// The same count with wider gaps is kept, so the Ls reach every edge of the area instead of
// crowding the corner the grid starts from (1 m gaps left a 50 m empty strip).
void TestTrenchLShape::tenPercentFillSpreadsOverTheArea() {
  const auto plan = TrenchGridGenerator::buildForTargetRatio(squareWkb(0, 0, 200, 200), 10.0, 2.0, 0.0, 15.0);
  QVERIFY2(!plan.cells.empty(), qPrintable(plan.error));
  double minX = 1e300, maxX = -1e300, minY = 1e300, maxY = -1e300;
  for (const auto& cell : plan.cells)
    for (const auto& point : cell.ring) {
      minX = std::min(minX, point.first);
      maxX = std::max(maxX, point.first);
      minY = std::min(minY, point.second);
      maxY = std::max(maxY, point.second);
    }
  QVERIFY2(maxX - minX > 180.0 && maxY - minY > 180.0,
           qPrintable(QStringLiteral("spread %1 × %2 m").arg(maxX - minX).arg(maxY - minY)));
}

void TestTrenchLShape::brokenLIsRejected() {
  const auto valid = TrenchGridGenerator::build(lSpec());
  QVERIFY(TrenchGridGenerator::cellsValid(valid));
  for (int kind = 0; kind < 4; ++kind) {
    auto broken = valid;
    if (kind == 0) broken[0].length = 30.0;           // area no longer width × length
    if (kind == 1) broken[0].ring[3].first += 0.5;    // inner corner moved
    if (kind == 2) broken[0].arm = 14.0;              // arms do not match the attribute
    if (kind == 3) broken[0].ring.pop_back();         // ring not closed
    QVERIFY2(!TrenchGridGenerator::cellsValid(broken), qPrintable(QString::number(kind)));
  }
  auto tooLong = lSpec();
  tooLong.armLength = 20.5;  // each arm stays within the 20 m trench limit
  QVERIFY(TrenchGridGenerator::build(tooLong).empty());
  auto tooShort = lSpec();
  tooShort.armLength = 2.0;  // an arm no longer than the width is not an L
  QVERIFY(TrenchGridGenerator::build(tooShort).empty());
}

void TestTrenchLShape::lTrenchSavesAsOnePolygon() {
  QTemporaryDir dir;
  QVERIFY(dir.isValid());
  const QString path = dir.filePath(QStringLiteral("survey.gpkg"));
  GDALAllRegister();
  {
    GDALDriver* driver = GetGDALDriverManager()->GetDriverByName("GPKG");
    QVERIFY(driver);
    Dataset seed(driver->Create(path.toUtf8().constData(), 0, 0, 0, GDT_Unknown, nullptr), &GDALClose);
    QVERIFY(seed);
  }
  auto spec = lSpec();
  spec.cols = 2;
  const auto cells = TrenchGridGenerator::build(spec);
  QCOMPARE(int(cells.size()), 2);
  QString error;
  QVERIFY2(TrenchGridGenerator::writeGpkg(path, QStringLiteral("trial_trench"), cells, QStringLiteral("EPSG:5187"), &error),
           qPrintable(error));
  Dataset ds(static_cast<GDALDataset*>(GDALOpenEx(path.toUtf8().constData(), GDAL_OF_VECTOR, nullptr, nullptr, nullptr)),
             &GDALClose);
  QVERIFY(ds);
  OGRLayer* layer = ds->GetLayerByName("trial_trench");
  QVERIFY(layer);
  QCOMPARE(int(layer->GetFeatureCount()), 2);
  layer->ResetReading();
  while (OGRFeature* raw = layer->GetNextFeature()) {
    std::unique_ptr<OGRFeature, decltype(&OGRFeature::DestroyFeature)> feature(raw, &OGRFeature::DestroyFeature);
    const auto* polygon = feature->GetGeometryRef()->toPolygon();
    QCOMPARE(polygon->getExteriorRing()->getNumPoints(), 7);
    QVERIFY(std::abs(polygon->get_Area() - 56.0) < 1e-6);
    QVERIFY(std::abs(feature->GetFieldAsDouble("width") * feature->GetFieldAsDouble("length") - 56.0) < 1e-9);
  }
}

void TestTrenchLShape::planCacheKeepsStraightAndLApart() {
  TrenchPlanCache::clear();
  const QByteArray area = squareWkb(200000, 450000, 200200, 450200);
  const auto straight = TrenchPlanCache::ratioPlan(area, 10.0, 2.0, 0.0);
  const auto lShaped = TrenchPlanCache::ratioPlan(area, 10.0, 2.0, 0.0, 15.0);
  QVERIFY(!straight.cells.empty() && !lShaped.cells.empty());
  QCOMPARE(int(straight.cells.front().ring.size()), 5);
  QCOMPARE(int(lShaped.cells.front().ring.size()), 7);
  QCOMPARE(TrenchPlanCache::computeCount(), 2);
  TrenchGridGenerator::RatioFill known;
  QVERIFY(TrenchPlanCache::lookup(area, 10.0, 2.0, 0.0, &known, 15.0));
  QCOMPARE(int(known.cells.front().ring.size()), 7);
}

int main(int argc, char** argv) {
  QCoreApplication app(argc, argv);
  TestTrenchLShape tc;
  return QTest::qExec(&tc, argc, argv);
}

#include "test_trench_lshape.moc"
