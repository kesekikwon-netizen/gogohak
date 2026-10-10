// 받은 지적도의 필지를 도형선택에서 Ctrl+C 하고 Ctrl+V 하면 같은 자리에 「지적도 · 복사한 필지」 줄이
// 생긴다(docs/intent/2026-10-10-cadastral-own-section-copy-paste.md). 복사본은 원본과 따로 가진 도형이라
// 받은 지적도 줄을 지워도 남고, 조사 파일을 저장한 뒤 다시 열어도 남는다. 키 입력은 test_parcel_copy_keys.cpp.
#include <QtTest>

#include <qgsapplication.h>
#include <qgscoordinatetransform.h>
#include <qgsproject.h>
#include <qgsvectorlayer.h>
#include <qgswkbtypes.h>

#include "core/LayerOps.h"
#include "core/ParcelClipboard.h"
#include "core/SurveyProjectFactory.h"
#include "core/SurveyStorage.h"
#include "parcel_copy_fixture.h"

using namespace ParcelCopyFixture;

class ParcelCopyTest : public QObject {
  Q_OBJECT
 private slots:
  void pasteKeepsTheSamePlaceAndAttributes() {
    QgsProject project;
    const QgsGeometry parcel = square(200000, 450000, 10);
    QgsVectorLayer* received = receivedMap(project, QStringLiteral("EPSG:5187"), parcel);

    const auto parcels = ParcelClipboard::copyFeatures({{received, firstFid(received)}});
    QCOMPARE(int(parcels.size()), 1);
    QCOMPARE(ParcelClipboard::paste(&project, parcels), 1);

    QgsVectorLayer* copy = ParcelClipboard::copyLayer(&project);
    QVERIFY(copy);
    QCOMPARE(copy->name(), kCopyName);
    QVERIFY(LayerOps::isCadastralLayer(copy));  // 지적도 역할: 고칠 수 없고 자석이 붙는다
    QCOMPARE(copy->crs().authid(), QStringLiteral("EPSG:5187"));
    const QList<QgsFeature> copies = featuresOf(copy);
    QCOMPARE(int(copies.size()), 1);
    QVERIFY(copies.first().geometry().isTopologicallyEqual(parcel));
    QCOMPARE(copies.first().attribute(QStringLiteral("PNU")).toString(), kPnu);
    QCOMPARE(copies.first().attribute(QStringLiteral("JIBUN")).toString(), QStringLiteral("1-1"));
  }

  void secondPasteAddsToTheSameCopyLayer() {
    QgsProject project;
    QgsVectorLayer* received = receivedMap(project, QStringLiteral("EPSG:5187"), square(200000, 450000, 10));
    const auto parcels = ParcelClipboard::copyFeatures({{received, firstFid(received)}});
    QCOMPARE(ParcelClipboard::paste(&project, parcels), 1);
    QCOMPARE(ParcelClipboard::paste(&project, parcels), 1);
    QCOMPARE(copyLayerCount(project), 1);
    QCOMPARE(int(featuresOf(ParcelClipboard::copyLayer(&project)).size()), 2);
  }

  void multipartParcelBecomesOneFeaturePerPart() {
    QgsProject project;
    const QgsGeometry parcel = QgsGeometry::fromWkt(
        QStringLiteral("MULTIPOLYGON(((200000 450000, 200010 450000, 200010 450010, 200000 450010, 200000 450000)),"
                       "((200020 450000, 200030 450000, 200030 450010, 200020 450010, 200020 450000)))"));
    QgsVectorLayer* received = receivedMap(project, QStringLiteral("EPSG:5187"), parcel);
    QCOMPARE(ParcelClipboard::paste(&project, ParcelClipboard::copyFeatures({{received, firstFid(received)}})), 2);

    QVERIFY(!QgsWkbTypes::isMultiType(ParcelClipboard::copyLayer(&project)->wkbType()));  // 조각을 다시 감싸지 않는다
    const QList<QgsFeature> copies = featuresOf(ParcelClipboard::copyLayer(&project));
    QCOMPARE(int(copies.size()), 2);
    double area = 0;
    for (const QgsFeature& copy : copies) {
      QCOMPARE(int(copy.geometry().asGeometryCollection().size()), 1);  // 조각마다 한 줄
      area += copy.geometry().area();
    }
    QVERIFY(qAbs(area - parcel.area()) < 1e-6);
  }

  void copyStaysWhenItsSourceIsRemoved() {
    QgsProject project;
    QgsVectorLayer* received = receivedMap(project, QStringLiteral("EPSG:5187"), square(200000, 450000, 10));
    const QString receivedId = received->id();
    QCOMPARE(ParcelClipboard::paste(&project, ParcelClipboard::copyFeatures({{received, firstFid(received)}})), 1);

    project.removeMapLayer(receivedId);  // 받은 지적도 줄을 지운다
    QgsVectorLayer* copy = ParcelClipboard::copyLayer(&project);
    QVERIFY(copy);
    QCOMPARE(int(featuresOf(copy).size()), 1);
  }

  void surveyShapesAreNotCopied() {
    QgsProject project;
    QgsVectorLayer* received = receivedMap(project, QStringLiteral("EPSG:5187"), square(200000, 450000, 10));
    QCOMPARE(int(ParcelClipboard::copyFeatures({{received, firstFid(received)}}).size()), 1);

    auto* survey = new QgsVectorLayer(QStringLiteral("Polygon?crs=EPSG:5187&field=kind:string"),
                                      QStringLiteral("유구 면"), QStringLiteral("memory"));
    project.addMapLayer(survey, false);
    LayerOps::markSurveyLayer(survey, QStringLiteral("feature_poly"));
    addParcel(survey, square(200020, 450020, 5), QString());
    QVERIFY(ParcelClipboard::copyFeatures({{survey, firstFid(survey)}}).isEmpty());
  }

  void pasteWithNothingCopiedSaysWhy() {
    QgsProject project;
    QgsVectorLayer* received = receivedMap(project, QStringLiteral("EPSG:5187"), square(200000, 450000, 10));
    QCOMPARE(ParcelClipboard::paste(&project, ParcelClipboard::copyFeatures({{received, firstFid(received)}})), 1);

    QString error;
    QCOMPARE(ParcelClipboard::paste(&project, {}, &error), 0);
    QVERIFY2(!error.isEmpty(), "the user is told why nothing was pasted");
  }

  void pasteKeepsTheSamePlaceAcrossCoordinateSystems() {
    QgsProject project;
    const QgsGeometry place = square(200000, 450000, 10);  // 5187 좌표
    QgsVectorLayer* east = receivedMap(project, QStringLiteral("EPSG:5187"), place);
    // 같은 땅을 5186 좌표로 적은 받은 지적도. 숫자는 수백 km 다르다.
    const QgsCoordinateTransform toCentral(QgsCoordinateReferenceSystem(QStringLiteral("EPSG:5187")),
                                           QgsCoordinateReferenceSystem(QStringLiteral("EPSG:5186")),
                                           project.transformContext());
    QgsGeometry placeIn5186 = place;
    placeIn5186.transform(toCentral);
    QgsVectorLayer* central = receivedMap(project, QStringLiteral("EPSG:5186"), placeIn5186);

    const auto parcels = ParcelClipboard::copyFeatures({{east, firstFid(east)}, {central, firstFid(central)}});
    QCOMPARE(ParcelClipboard::paste(&project, parcels), 2);

    QgsVectorLayer* copy = ParcelClipboard::copyLayer(&project);
    QVERIFY(copy);
    QCOMPARE(copy->crs().authid(), QStringLiteral("EPSG:5187"));  // 먼저 복사한 필지의 좌표계
    const QList<QgsFeature> copies = featuresOf(copy);
    QCOMPARE(int(copies.size()), 2);
    for (const QgsFeature& copied : copies)
      QVERIFY(copied.geometry().centroid().distance(place.centroid()) < 0.01);  // 1 cm 안에서 같은 자리
  }

  void copySurvivesSaveAndReopen() {
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QString error;
    // 조사 파일 (앱이 만드는 방식 그대로) 과 받은 지적도 원본 (조사 파일 바깥의 GPKG).
    const QString survey = SurveyProjectFactory::createNewSurvey(dir.path(), QStringLiteral("copy"), &error);
    QVERIFY2(!survey.isEmpty(), qPrintable(error));
    const QString outside = savedMap(dir, square(200000, 450000, 10));
    QVERIFY(!outside.isEmpty());

    QgsProject project;
    auto* received =
        new QgsVectorLayer(outside + QStringLiteral("|layername=cadastral"), kSource, QStringLiteral("ogr"));
    QVERIFY(received->isValid());
    project.addMapLayer(received, false);
    LayerOps::markCadastralLayer(received);
    QCOMPARE(ParcelClipboard::paste(&project, ParcelClipboard::copyFeatures({{received, firstFid(received)}})), 1);

    // 저장: 편집 중인 복사본을 커밋하고, 바깥 지적도는 조사 파일로 옮기지 않은 채 복사본만 들인다.
    QVERIFY(ParcelClipboard::copyLayer(&project)->commitChanges());
    const SurveyStorage::AbsorbResult absorbed = SurveyStorage::absorbExternalVectors(&project, survey);
    QVERIFY2(absorbed.skippedReference.contains(kSource), qPrintable(absorbed.skippedReference.join(',')));
    QVERIFY2(absorbed.imported.contains(kCopyName), qPrintable(absorbed.imported.join(',')));

    // 프로젝트를 저장하고 다시 연다.
    const QString projectPath = dir.filePath(QStringLiteral("copy.qgz"));
    QVERIFY(project.write(projectPath));
    QgsProject reopened;
    QVERIFY(reopened.read(projectPath));
    QgsVectorLayer* reopenedCopy = ParcelClipboard::copyLayer(&reopened);
    QVERIFY2(reopenedCopy, "the copy comes back when the survey is reopened");
    QVERIFY(LayerOps::isCadastralLayer(reopenedCopy));
    QCOMPARE(int(featuresOf(reopenedCopy).size()), 1);

    // 받은 지적도 줄을 지워도 복사본은 남는다.
    for (QgsMapLayer* layer : reopened.mapLayersByName(kSource))
      reopened.removeMapLayer(layer->id());
    QVERIFY(ParcelClipboard::copyLayer(&reopened));
    QCOMPARE(int(featuresOf(ParcelClipboard::copyLayer(&reopened)).size()), 1);
  }

  void pastingASavedMapTwiceStillSaves() {
    // 받은 지적도(GPKG)의 여러 조각 필지를 두 번 붙여도 저장·다시 열기에서 하나도 빠지지 않는다.
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QString error;
    const QString survey = SurveyProjectFactory::createNewSurvey(dir.path(), QStringLiteral("twice"), &error);
    QVERIFY2(!survey.isEmpty(), qPrintable(error));
    const QString outside = savedMap(
        dir, QgsGeometry::fromWkt(QStringLiteral(
                 "MULTIPOLYGON(((200000 450000, 200010 450000, 200010 450010, 200000 450010, 200000 450000)),"
                 "((200020 450000, 200030 450000, 200030 450010, 200020 450010, 200020 450000)))")));
    QVERIFY(!outside.isEmpty());

    QgsProject project;
    auto* received =
        new QgsVectorLayer(outside + QStringLiteral("|layername=cadastral"), kSource, QStringLiteral("ogr"));
    QVERIFY(received->isValid());
    project.addMapLayer(received, false);
    LayerOps::markCadastralLayer(received);
    const auto parcels = ParcelClipboard::copyFeatures({{received, firstFid(received)}});
    QCOMPARE(ParcelClipboard::paste(&project, parcels), 2);
    QCOMPARE(ParcelClipboard::paste(&project, parcels), 2);

    QVERIFY(ParcelClipboard::copyLayer(&project)->commitChanges());
    const SurveyStorage::AbsorbResult absorbed = SurveyStorage::absorbExternalVectors(&project, survey);
    QVERIFY2(absorbed.imported.contains(kCopyName), qPrintable(absorbed.failed.join(',')));
    const QString projectPath = dir.filePath(QStringLiteral("twice.qgz"));
    QVERIFY(project.write(projectPath));
    QgsProject reopened;
    QVERIFY(reopened.read(projectPath));
    QCOMPARE(int(featuresOf(ParcelClipboard::copyLayer(&reopened)).size()), 4);
  }

};

int main(int argc, char** argv) {
  QgsApplication app(argc, argv, false);
  QgsApplication::setPrefixPath(qEnvironmentVariable("QGIS_PREFIX_PATH"), true);
  QgsApplication::initQgis();
  ParcelCopyTest test;
  const int rc = QTest::qExec(&test, argc, argv);
  QgsApplication::exitQgis();
  return rc;
}
#include "test_parcel_copy.moc"
