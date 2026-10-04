// 「조용한 도판」 기록 카드 (2차 보드 판 3): the card as a ledger on the page, the cadastral parcel as a locked record,
// the period band, a control point's coordinates and a live switch of the look. Moved out of
// tests/test_feature_card_fields.cpp to keep both under the 300-line limit.
#include <QtTest>
#include <QDir>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <memory>

#include "app/KaFeatureCard.h"
#include "app/KaFeatureFormDialog.h"
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

class TestFeatureCardPlate : public QObject {
  Q_OBJECT
 private slots:
  // 「조용한 도판」 기록 카드 (2차 보드 판 3 B1): no box on the panel, the shape's swatch before the name, the record
  // in one line under it, one 「자동 계산」 head over the 면적·둘레 ledger, figures without boxes.
  void cardIsALedgerOnThePageInPaper() {
    KaTheme::DisplayOptions paper;
    paper.paperLook = true;
    KaTheme::setDisplayOptions(qApp, paper);
    auto layer = recordedFeature();
    const QgsFeatureId id = firstId(layer.get());
    layer->dataProvider()->changeAttributeValues(
        {{id, {{layer->fields().lookupField(QStringLiteral("kind")), QStringLiteral("주거지")},
               {layer->fields().lookupField(QStringLiteral("period")), QStringLiteral("청동기")}}}});
    QWidget host;
    host.setAutoFillBackground(true);
    auto* card = new KaFeatureCard(&host);
    (new QHBoxLayout(&host))->addWidget(card);
    host.resize(300, 640);
    host.show();
    QVERIFY(QTest::qWaitForWindowExposed(&host));
    card->setFeature(layer.get(), id);
    QCoreApplication::processEvents();
    auto* swatch = card->findChild<QLabel*>(QStringLiteral("kaFeatureCardSwatch"));
    auto* sub = card->findChild<QLabel*>(QStringLiteral("kaFeatureCardSub"));
    auto* head = card->findChild<QWidget*>(QStringLiteral("kaMeasureHead"));
    const QImage image = host.grab().toImage();
    const auto at = [&](QWidget* w, const QPoint& p) { return image.pixelColor(w->mapTo(&host, p) * image.devicePixelRatio()).name(); };
    const QString ground = at(card, QPoint(card->width() / 2, -2));
    QFrame* box = card->findChildren<QFrame*>(QStringLiteral("kaMeasureField")).value(0);
    const QString boxFace = box ? at(box, QPoint(4, box->height() / 2)) : QString();
    const QString cardEdge = at(card, QPoint(0, card->height() / 2));
    KaTheme::setDisplayOptions(qApp, KaTheme::DisplayOptions());
    QVERIFY(swatch && swatch->isVisible() && swatch->pixmap().size() == QSize(28, 20));  // spec 4.1: 유구 표지 28 × 20
    QVERIFY(sub && sub->isVisible());
    QCOMPARE(sub->text(), QStringLiteral("유구 면 · 주거지 · 청동기"));
    QVERIFY(!card->findChild<QLabel*>(QStringLiteral("kaFeatureCardKind"))->isVisible());
    QVERIFY(head && head->isVisible());
    auto* number = card->findChild<QLineEdit*>(QStringLiteral("kaFeatureCardNumber"));
    QVERIFY2(head->mapTo(card, QPoint()).y() < number->mapTo(card, QPoint()).y(), "the ledger comes right under the name");
    QCOMPARE(cardEdge, ground);  // no box around the card
    QCOMPARE(boxFace, ground);   // the figure has no box of its own
  }

  // 참조 자료 (2차 보드 판 3 B8): a parcel is named by its 지번, 「지적도 · 참조 자료」 under it, and 지번·지목
  // read as locked rows (the record is never written).
  void parcelReadsAsALockedRecordInPaper() {
    KaTheme::DisplayOptions paper;
    paper.paperLook = true;
    KaTheme::setDisplayOptions(nullptr, paper);
    QgsVectorLayer parcels(QStringLiteral("MultiPolygon?crs=EPSG:5186&field=PNU:string&field=JIBUN:string"),
                           QStringLiteral("지적도 · 조사 주변 5km"), QStringLiteral("memory"));
    LayerOps::markCadastralLayer(&parcels);  // as the cadastral download marks its layer
    QgsFeature f(parcels.fields());
    f.setAttribute(QStringLiteral("PNU"), QStringLiteral("5011025021116160001"));
    f.setAttribute(QStringLiteral("JIBUN"), QStringLiteral("1616-1 과"));
    f.setGeometry(QgsGeometry::fromRect(QgsRectangle(200000, 450000, 200010, 450005)));
    parcels.dataProvider()->addFeature(f);
    KaFeatureCard card;
    card.setFeature(&parcels, firstId(&parcels));
    const auto text = [&card](const char* name) {
      auto* label = card.findChild<QLabel*>(QString::fromLatin1(name));
      return label ? label->text() : QStringLiteral("(없음)");
    };
    const QString name = text("kaFeatureCardName"), sub = text("kaFeatureCardSub");
    const QString jibun = text("kaLockedJibun"), jimok = text("kaLockedJimok");
    KaTheme::setDisplayOptions(nullptr, KaTheme::DisplayOptions());
    QCOMPARE(name, QStringLiteral("1616-1"));
    QCOMPARE(sub, QStringLiteral("지적도 · 참조 자료"));
    QCOMPARE(jibun, QStringLiteral("1616-1"));
    QCOMPARE(jimok, QStringLiteral("과"));
    parcels.dataProvider()->changeAttributeValues({{firstId(&parcels), {{parcels.fields().lookupField(QStringLiteral("JIBUN")), QString()}}}});
    KaTheme::setDisplayOptions(nullptr, paper);
    card.setFeature(&parcels, firstId(&parcels));
    const QString emptyJibun = text("kaLockedJibun");
    KaTheme::setDisplayOptions(nullptr, KaTheme::DisplayOptions());
    QCOMPARE(emptyJibun, QStringLiteral("—"));  // nothing recorded reads as a dash, not a blank
  }

  // 「새 모양」 switched while a record is shown (화면 보기): the card is rebuilt in the other look at once.
  void cardFollowsALiveSwitch() {
    auto layer = recordedFeature();
    KaFeatureCard card;
    card.show();
    QVERIFY(QTest::qWaitForWindowExposed(&card));
    card.setFeature(layer.get(), firstId(layer.get()));
    QVERIFY(!card.findChild<QFrame*>(QStringLiteral("kaMeasureHead")));
    KaTheme::DisplayOptions paper;
    paper.paperLook = true;
    KaTheme::setDisplayOptions(qApp, paper);
    QCoreApplication::processEvents();
    const bool headIn = card.findChild<QFrame*>(QStringLiteral("kaMeasureHead")) != nullptr;
    const bool swatchIn = card.findChild<QLabel*>(QStringLiteral("kaFeatureCardSwatch"))->isVisibleTo(&card);
    KaTheme::setDisplayOptions(qApp, KaTheme::DisplayOptions());
    QCoreApplication::processEvents();
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    QVERIFY(headIn && swatchIn);
    QVERIFY(!card.findChild<QFrame*>(QStringLiteral("kaMeasureHead")));
    QVERIFY(!card.findChild<QLabel*>(QStringLiteral("kaFeatureCardSwatch"))->isVisibleTo(&card));
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

  // 이름·번호 (2차 보드 판 5, 440): two plain lines on top, 96 px words, and the next number filled in with
  // 「다음 번호를 미리 넣었습니다」 beside it until a number is typed.
  void nameNumberWindowReadsLikeTheBoardInPaper() {
    auto layer = recordedFeature();
    qApp->setProperty("kaPaperLook", true);
    KaFeatureFormDialog form(layer.get(), nullptr, firstId(layer.get()));
    qApp->setProperty("kaPaperLook", QVariant());
    QCOMPARE(form.minimumWidth(), 440);
    QCOMPARE(form.findChild<QLabel*>(QStringLiteral("kaFeatureFormTip"))->text(),
             QStringLiteral("이름과 번호는 나중에 넣어도 됩니다.") + QLatin1Char(0x0A) +
                 QStringLiteral("건너뛰어도 그린 도형은 그대로 남습니다."));
    auto* filled = form.findChild<QLabel*>(QStringLiteral("kaFeatureFormNumberFilled"));
    QVERIFY(filled && filled->isVisibleTo(&form));
    QCOMPARE(filled->text(), QStringLiteral("다음 번호를 미리 넣었습니다"));
    QTest::keyClicks(form.findChild<QLineEdit*>(QStringLiteral("kaFeatureFormNumber")), QStringLiteral("7"));
    QVERIFY(!filled->isVisibleTo(&form));
    KaFeatureFormDialog stock(layer.get(), nullptr, firstId(layer.get()));
    QVERIFY(!stock.findChild<QLabel*>(QStringLiteral("kaFeatureFormNumberFilled")));
    QVERIFY(stock.findChild<QLabel*>(QStringLiteral("kaFeatureFormTip"))->text().startsWith(QStringLiteral("선택입니다")));
  }
};

int main(int argc, char** argv) {
  QgsApplication app(argc, argv, true);
  QgsApplication::setPrefixPath(qEnvironmentVariable("QGIS_PREFIX_PATH", QStringLiteral("A:/OSGeo4W/apps/qgis-dev")),
                                true);
  QgsApplication::initQgis();
  FeaturePresets::instance().ensureLoaded();
  TestFeatureCardPlate tc;
  const int rc = QTest::qExec(&tc, argc, argv);
  QgsApplication::exitQgis();
  return rc;
}

#include "test_feature_card_plate.moc"
