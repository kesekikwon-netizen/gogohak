// 「조용한 도판」 조사구역 그리기 설정 (2차 보드 판 5, 440): eight plain swatches in two rows of four with the chosen
// one ringed and named under 「외곽선 색」, the four widths joined in one row and 「직접」 mm on its own row. Stock keeps
// its one row of named swatches (tests/test_survey_point_dialogs.cpp, which is at its line limit).
#include "app/KaSurveyAreaDialog.h"
#include "app/KaTheme.h"
#include "core/LayerOps.h"

#include <QApplication>
#include <QDir>
#include <QDoubleSpinBox>
#include <QFontDatabase>
#include <QLabel>
#include <QPushButton>
#include <QTemporaryDir>
#include <QtTest>

#include <qgsapplication.h>
#include <qgscoordinatereferencesystem.h>
#include <qgslayertree.h>
#include <qgsmapcanvas.h>
#include <qgsproject.h>
#include <qgsvectorfilewriter.h>
#include <qgsvectorlayer.h>

class TestSurveyAreaPlate : public QObject {
  Q_OBJECT
private slots:
  void cleanup() { KaTheme::setDisplayOptions(qApp, KaTheme::DisplayOptions()); }

  void paperWindowReadsLikeTheBoard() {
    KaTheme::DisplayOptions paper;
    paper.paperLook = true;
    KaTheme::setDisplayOptions(qApp, paper);
    KaSurveyAreaDialog dialog(nullptr);
    dialog.setAttribute(Qt::WA_DontShowOnScreen);
    dialog.show();
    QApplication::processEvents();
    QCOMPARE(dialog.minimumWidth(), 440);
    const auto swatches = dialog.findChildren<QPushButton*>(QStringLiteral("surveyAreaColor"));
    QCOMPARE(swatches.size(), 8);
    QCOMPARE(swatches.at(3)->y(), swatches.at(0)->y());  // two rows of four
    QVERIFY(swatches.at(4)->y() > swatches.at(0)->y());
    for (QPushButton* swatch : swatches) QVERIFY2(swatch->text().isEmpty() && !swatch->accessibleName().isEmpty(),
                                                  qPrintable(swatch->text()));
    auto* name = dialog.findChild<QLabel*>(QStringLiteral("surveyAreaColorName"));
    QVERIFY(name);
    QCOMPARE(name->text(), QStringLiteral("주황"));  // the first area is orange
    swatches.at(0)->click();
    QCOMPARE(name->text(), swatches.at(0)->accessibleName());
    const auto widths = dialog.findChildren<QPushButton*>(QStringLiteral("surveyAreaWidth"));
    QCOMPARE(widths.size(), 4);
    for (int i = 1; i < widths.size(); ++i)  // joined: each starts where the one before ends
      QVERIFY2(qAbs(widths.at(i)->x() - (widths.at(i - 1)->x() + widths.at(i - 1)->width())) <= 1,
               qPrintable(QString::number(widths.at(i)->x())));
    const auto after = [&widths] {
      QList<bool> marks;
      for (QPushButton* w : widths) marks << w->property("kaAfterChecked").toBool();
      return marks;
    };
    QCOMPARE(after(), (QList<bool>{false, false, true, false}));  // 기본 1.5 (the second) is chosen
    widths.at(3)->click();
    QCOMPARE(after(), (QList<bool>{false, false, false, false}));
    auto* own = dialog.findChild<QDoubleSpinBox*>();
    QVERIFY(own->y() > widths.first()->y() + widths.first()->height() / 2);  // 「직접」 on its own row
    const QString out = qEnvironmentVariable("KA_HGIS_QA_OUTPUT_DIR");  // a picture for a look, as in the stock test
    if (!out.isEmpty()) QVERIFY(dialog.grab().save(QDir(out).filePath(QStringLiteral("survey-area-dialog-paper.png"))));
  }

  void uncheckedSurveyAreaIsNotContinuedUnlessPickedAndEditMakesItVisible() {
    QTemporaryDir dir;
    const QString survey = dir.filePath(QStringLiteral("조사.gpkg"));
    QgsVectorLayer shape(QStringLiteral("Polygon?crs=EPSG:5187"), QStringLiteral("area"), QStringLiteral("memory"));
    QgsVectorFileWriter::SaveVectorOptions options;
    options.driverName = QStringLiteral("GPKG");
    options.layerName = QStringLiteral("survey_area");
    options.actionOnExistingFile = QgsVectorFileWriter::CreateOrOverwriteFile;
    QVERIFY(QgsVectorFileWriter::writeAsVectorFormatV3(&shape, survey, QgsCoordinateTransformContext(), options) ==
            QgsVectorFileWriter::NoError);
    QgsProject project;
    auto* own = new QgsVectorLayer(survey + QStringLiteral("|layername=survey_area"), QStringLiteral("임하리사지"), QStringLiteral("ogr"));
    LayerOps::markSurveyLayer(own, QStringLiteral("survey_area"));
    QVERIFY(own->isValid());
    project.addMapLayer(own, false);
    QgsLayerTreeGroup* group = project.layerTreeRoot()->addGroup(QStringLiteral("조사 데이터"));
    QgsLayerTreeLayer* node = group->addLayer(own);
    node->setItemVisibilityChecked(false);
    QCOMPARE(KaSurveyAreaDialog::layerToContinue(&project, survey, nullptr), static_cast<QgsVectorLayer*>(nullptr));
    QCOMPARE(KaSurveyAreaDialog::layerToContinue(&project, survey, own), own);
    group->setItemVisibilityChecked(false);
    QgsMapCanvas canvas;
    KaSurveyAreaDialog::ensureLayerVisibleOnCanvas(&project, own, &canvas);
    QVERIFY(node->isVisible());
  }
};

int main(int argc, char** argv) {
  QgsApplication app(argc, argv, true);
#ifdef Q_OS_WIN
  if (QGuiApplication::platformName() == QLatin1String("offscreen")) {  // Korean for the QA picture
    const QDir windows(qEnvironmentVariable("WINDIR", QStringLiteral("C:/Windows")));
    for (const QString& file : {QStringLiteral("malgun.ttf"), QStringLiteral("malgunbd.ttf")})
      QFontDatabase::addApplicationFont(windows.filePath(QStringLiteral("Fonts/") + file));
  }
#endif
  QgsApplication::setPrefixPath(qEnvironmentVariable("QGIS_PREFIX_PATH"), true);
  QgsApplication::initQgis();
  KaTheme::apply(&app);
  int code = 0;
  {
    TestSurveyAreaPlate test;
    code = QTest::qExec(&test, argc, argv);
  }
  QgsApplication::exitQgis();
  return code;
}

#include "test_survey_area_plate.moc"
