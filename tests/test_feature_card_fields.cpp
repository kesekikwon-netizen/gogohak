// The 「선택한 유구」 card also shows and edits 조사 상태·조사자·조사일, and Enter in one of its boxes
// only finishes that box (user 2026-10-03 「선택한 유구의 속성정보를 수정할수있게하라」, answer 「진행」,
// docs/intent/2026-10-03-edit-record-while-drawing.md). tests/test_feature_card.cpp is at its 300-line limit.
#include <QtTest>
#include <QDir>
#include <QFrame>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QSignalSpy>
#include <memory>

#include "app/KaFeatureCard.h"
#include "app/KaTheme.h"
#include "core/FeaturePresets.h"
#include "core/LayerOps.h"

#include <qgsapplication.h>
#include <qgsfeature.h>
#include <qgsgeometry.h>
#include <qgspointxy.h>
#include <qgsrectangle.h>
#include <qgsvectordataprovider.h>
#include <qgsvectorlayer.h>

namespace {
std::unique_ptr<QgsVectorLayer> recordedFeature(int count = 1) {
  auto layer = std::make_unique<QgsVectorLayer>(
      QStringLiteral("Polygon?crs=EPSG:5187&field=kind:string&field=period:string&field=feature_no:string"
                     "&field=note:string&field=status:string&field=surveyor:string&field=surv_date:string"),
      QStringLiteral("유구"), QStringLiteral("memory"));
  LayerOps::markSurveyLayer(layer.get(), QStringLiteral("feature_poly"));
  QgsFeature f(layer->fields());
  f.setAttribute(QStringLiteral("feature_no"), QStringLiteral("1호"));
  f.setAttribute(QStringLiteral("status"), QStringLiteral("조사 중"));
  f.setAttribute(QStringLiteral("surveyor"), QStringLiteral("권영인"));
  f.setAttribute(QStringLiteral("surv_date"), QStringLiteral("2026-10-03"));
  f.setGeometry(QgsGeometry::fromRect(QgsRectangle(200000, 450000, 200010, 450005)));
  for (int i = 0; i < count; ++i) layer->dataProvider()->addFeature(f);
  return layer;
}

QgsFeatureId firstId(QgsVectorLayer* layer) {
  QgsFeature f;
  layer->getFeatures().nextFeature(f);
  return f.id();
}
}  // namespace

class TestFeatureCardFields : public QObject {
  Q_OBJECT
 private slots:
  void surveyStatusSurveyorAndDateAreOnTheCard() {
    auto layer = recordedFeature();
    KaFeatureCard card;
    card.setFeature(layer.get(), firstId(layer.get()));
    QCOMPARE(card.rowText(QStringLiteral("status")), QStringLiteral("조사 중"));
    QCOMPARE(card.rowText(QStringLiteral("surveyor")), QStringLiteral("권영인"));
    QCOMPARE(card.rowText(QStringLiteral("surv_date")), QStringLiteral("2026-10-03"));
  }

  // Typing in a box and pressing Enter writes that one value into the edit buffer (one undo step);
  // nothing else happens, so a drawing in progress goes on.
  void enterFinishesOnlyThatBox() {
    auto layer = recordedFeature();
    QVERIFY(layer->startEditing());
    KaFeatureCard card;
    card.show();
    card.setFeature(layer.get(), firstId(layer.get()));
    QLineEdit* surveyor = nullptr;
    for (QLineEdit* edit : card.findChildren<QLineEdit*>())
      if (edit->text() == QStringLiteral("권영인")) surveyor = edit;
    QVERIFY(surveyor);
    QSignalSpy edited(&card, &KaFeatureCard::edited);
    QSignalSpy entered(&card, &KaFeatureCard::valueEntered);
    surveyor->setFocus();
    surveyor->setText(QStringLiteral("김조사"));  // typed (QTest cannot type Hangul)
    QTest::keyClick(surveyor, Qt::Key_Return);
    QCOMPARE(edited.count(), 1);
    QCOMPARE(entered.count(), 1);  // while drawing the window hands the keys back to the map
    QCOMPARE(layer->getFeature(firstId(layer.get())).attribute(QStringLiteral("surveyor")).toString(), QStringLiteral("김조사"));
    layer->rollBack();
  }

  // 「조용한 도판」 수치 대장: the 면적 and 둘레 rows show only the figure (20 px); the 「자동 계산」 words move
  // into the tooltip, so the card keeps its width (stock look keeps the lock and the words).
  void measureFigureStandsAlone() {
    KaTheme::DisplayOptions paper;
    paper.paperLook = true;
    KaTheme::setDisplayOptions(qApp, paper);
    auto layer = recordedFeature();
    KaFeatureCard card;
    card.show();
    card.setFeature(layer.get(), firstId(layer.get()));
    const auto boxes = card.findChildren<QFrame*>(QStringLiteral("kaMeasureField"));
    KaTheme::setDisplayOptions(qApp, KaTheme::DisplayOptions());
    QCOMPARE(boxes.size(), 2);
    for (QFrame* box : boxes) {
      QVERIFY(!box->findChild<QLabel*>(QStringLiteral("kaMeasureAuto"))->isVisibleTo(box));
      QVERIFY(box->findChild<QLabel*>(QStringLiteral("kaMeasureValue"))->toolTip().contains(QStringLiteral("자동 계산")));
    }
  }

  // 시대 띠: under 시대 the card shows the ten period cells with the recorded one marked (새 모양 only).
  void periodBandMarksTheRecordedPeriodInPaper() {
    auto layer = recordedFeature();
    const int period = layer->fields().indexOf(QStringLiteral("period"));
    QVERIFY(layer->dataProvider()->changeAttributeValues({{firstId(layer.get()), {{period, QStringLiteral("청동기")}}}}));
    {
      KaFeatureCard stock;
      stock.setFeature(layer.get(), firstId(layer.get()));
      QVERIFY(!stock.findChild<QWidget*>(QStringLiteral("kaPeriodBand")));
    }
    KaTheme::DisplayOptions paper;
    paper.paperLook = true;
    KaTheme::setDisplayOptions(qApp, paper);
    KaFeatureCard card;
    card.show();
    card.setFeature(layer.get(), firstId(layer.get()));
    QCoreApplication::processEvents();  // a row added to a shown card appears on the next event turn
    auto* band = card.findChild<QWidget*>(QStringLiteral("kaPeriodBand"));
    KaTheme::setDisplayOptions(qApp, KaTheme::DisplayOptions());
    QVERIFY2(band, "no 시대 띠 widget");
    QVERIFY(band->isVisibleTo(&card));
    QCOMPARE(band->property("periodIndex").toInt(), FeaturePresets::instance().periodOrder(QStringLiteral("청동기")));
  }

  // 기준점: the card shows the geodetic line and X / Y / Z as mono figures on dotted leaders (새 모양 only).
  void controlPointShowsItsCoordinatesInPaper() {
    QgsVectorLayer layer(QStringLiteral("Point?crs=EPSG:5186&field=point_id:string&field=x:double&field=y:double&field=z:double"
                                        "&field=datum:string&field=ellipsoid:string&field=projection:string&field=origin:string"),
                         QStringLiteral("기준점"), QStringLiteral("memory"));
    LayerOps::markSurveyLayer(&layer, QStringLiteral("control_points"));
    QgsFeature f(layer.fields());
    const QList<QPair<QString, QVariant>> values = {
        {QStringLiteral("point_id"), QStringLiteral("GCP1")}, {QStringLiteral("x"), 147962.418},
        {QStringLiteral("y"), 98131.207}, {QStringLiteral("z"), 152.36}, {QStringLiteral("datum"), QStringLiteral("세계측지계")},
        {QStringLiteral("ellipsoid"), QStringLiteral("GRS80")}, {QStringLiteral("projection"), QStringLiteral("TM")},
        {QStringLiteral("origin"), QStringLiteral("중부원점")}};
    for (const auto& [field, value] : values) f.setAttribute(field, value);
    f.setGeometry(QgsGeometry::fromPointXY(QgsPointXY(147962.418, 98131.207)));
    QVERIFY(layer.dataProvider()->addFeature(f));
    {
      KaFeatureCard stock;
      stock.setFeature(&layer, firstId(&layer));
      QVERIFY(!stock.findChild<QLabel*>(QStringLiteral("kaControlX")));
    }
    KaTheme::DisplayOptions paper;
    paper.paperLook = true;
    KaTheme::setDisplayOptions(qApp, paper);
    KaFeatureCard card;
    card.setFeature(&layer, firstId(&layer));
    const QString output = qEnvironmentVariable("KA_HGIS_QA_OUTPUT_DIR");  // opt-in look check
    if (!output.isEmpty() && QDir(output).exists()) {
      card.resize(272, 420);
      card.show();
      QCoreApplication::processEvents();
      QVERIFY(card.grab().save(QDir(output).filePath(QStringLiteral("control-point-card.png"))));
    }
    KaTheme::setDisplayOptions(qApp, KaTheme::DisplayOptions());
    const auto text = [&card](const char* name) {
      auto* label = card.findChild<QLabel*>(QString::fromLatin1(name));
      return label ? label->text() : QStringLiteral("(없음)");
    };
    QCOMPARE(text("kaControlMeta"), QStringLiteral("세계측지계 · GRS80 · TM · 중부원점"));
    QCOMPARE(text("kaControlX"), QStringLiteral("147,962.418"));
    QCOMPARE(text("kaControlY"), QStringLiteral("98,131.207"));
    QCOMPARE(text("kaControlZ"), QStringLiteral("EL. 152.36"));
    // A point with no coordinates says so instead of 0.000.
    QgsFeature empty(layer.fields());
    empty.setAttribute(QStringLiteral("point_id"), QStringLiteral("GCP9"));
    QVERIFY(layer.dataProvider()->addFeature(empty));
    KaTheme::setDisplayOptions(qApp, paper);
    QgsFeatureId emptyId = FID_NULL;
    QgsFeature each;
    for (QgsFeatureIterator it = layer.getFeatures(); it.nextFeature(each);)
      if (each.attribute(QStringLiteral("point_id")).toString() == QLatin1String("GCP9")) emptyId = each.id();
    card.setFeature(&layer, emptyId);
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);  // the previous point's rows
    KaTheme::setDisplayOptions(qApp, KaTheme::DisplayOptions());
    QCOMPARE(text("kaControlX"), QStringLiteral("—"));
  }

  // Text still being typed when the card shows another record goes to its own record, never to
  // the next one and never lost (code review: the card is no longer locked while drawing).
  void typedTextStaysWithItsRecord() {
    auto layer = recordedFeature(2);
    QVERIFY(layer->startEditing());
    const QList<QgsFeatureId> ids(layer->allFeatureIds().cbegin(), layer->allFeatureIds().cend());
    QCOMPARE(ids.size(), 2);
    KaFeatureCard card;
    card.show();
    card.setFeature(layer.get(), ids.at(0));
    auto* note = card.findChild<QPlainTextEdit*>(QStringLiteral("kaFeatureCardNote"));
    QLineEdit* surveyor = nullptr;
    for (QLineEdit* edit : card.findChildren<QLineEdit*>())
      if (edit->text() == QStringLiteral("권영인")) surveyor = edit;
    QVERIFY(note && surveyor);
    surveyor->setText(QStringLiteral("김조사"));
    note->setFocus();
    note->setPlainText(QStringLiteral("북벽 교란"));
    card.setFeature(layer.get(), ids.at(1));
    const QgsFeature first = layer->getFeature(ids.at(0));
    const QgsFeature second = layer->getFeature(ids.at(1));
    QCOMPARE(first.attribute(QStringLiteral("note")).toString(), QStringLiteral("북벽 교란"));
    QVERIFY(second.attribute(QStringLiteral("note")).toString().isEmpty());
    QCOMPARE(second.attribute(QStringLiteral("surveyor")).toString(), QStringLiteral("권영인"));
    layer->rollBack();
  }
};

int main(int argc, char** argv) {
  QgsApplication app(argc, argv, true);
  QgsApplication::setPrefixPath(qEnvironmentVariable("QGIS_PREFIX_PATH", QStringLiteral("A:/OSGeo4W/apps/qgis-dev")),
                                true);
  QgsApplication::initQgis();
  FeaturePresets::instance().ensureLoaded();
  TestFeatureCardFields tc;
  const int rc = QTest::qExec(&tc, argc, argv);
  QgsApplication::exitQgis();
  return rc;
}

#include "test_feature_card_fields.moc"
