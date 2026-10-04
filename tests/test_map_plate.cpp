// 「조용한 도판」 map screen (2차 보드 판 1): the tab-row survey mark, the status-bar work CRS and the address
// search and the scale bar in 새 모양; stock keeps its own words and box.
#include <QtTest>

#include <QApplication>
#include <QFontMetrics>
#include <QStandardPaths>
#include <QImage>
#include <QLabel>
#include <QLineEdit>
#include <QToolButton>

#include <qgsapplication.h>
#include <qgscoordinatereferencesystem.h>
#include <qgsmapcanvas.h>

#include "app/KaAppBar.h"
#include "app/KaBasemapQuickCard.h"
#include "app/KaMapControls.h"
#include "app/KaStatusBar.h"
#include "app/KaSurveyBadge.h"
#include "app/KaTheme.h"

namespace {

void paper(bool on) {
  KaTheme::DisplayOptions options;
  options.paperLook = on;
  KaTheme::setDisplayOptions(qApp, options);
}

// The widget alone on a transparent sheet (grab() would paint an opaque window ground under it).
QImage alone(QWidget& widget) {
  QImage image(widget.size(), QImage::Format_ARGB32_Premultiplied);
  image.fill(Qt::transparent);
  widget.render(&image, QPoint(), QRegion(), QWidget::DrawChildren);
  return image;
}

int countNear(const QImage& image, const QColor& want, int tolerance) {
  int n = 0;
  for (int y = 0; y < image.height(); ++y)
    for (int x = 0; x < image.width(); ++x) {
      const QColor c = image.pixelColor(x, y);
      if (c.alpha() > 200 && qAbs(c.red() - want.red()) <= tolerance && qAbs(c.green() - want.green()) <= tolerance &&
          qAbs(c.blue() - want.blue()) <= tolerance)
        ++n;
    }
  return n;
}

}  // namespace

class TestMapPlate : public QObject {
  Q_OBJECT
 private slots:
  void cleanup() { paper(false); }

  // 탭 줄 오른쪽: 「광령리1 ● 저장 안 됨」 — the name in ink, a small clay dot, no pill; saved reads 「저장됨」.
  void surveyMarkIsNameAndDotInPaper() {
    paper(true);
    QCOMPARE(KaSurveyBadge::textFor(QStringLiteral("광령리1"), true), QStringLiteral("광령리1 ● 저장 안 됨"));
    QCOMPARE(KaSurveyBadge::textFor(QStringLiteral("광령리1"), false), QStringLiteral("광령리1 저장됨"));
    KaSurveyBadge badge;
    badge.setSurvey(QStringLiteral("광령리1"), true);
    badge.resize(badge.sizeHint());
    const QImage image = badge.grab().toImage();
    QVERIFY2(countNear(image, QColor(0xD9, 0x77, 0x57), 24) >= 6, "the clay dot");
    QCOMPARE(countNear(image, KaTheme::tokens().warnSurface, 6), 0);  // no warn pill behind the words
    QVERIFY(countNear(image, KaTheme::tokens().ink, 40) > 20);        // the name and state in ink
    paper(false);
    QCOMPARE(KaSurveyBadge::textFor(QStringLiteral("광령리1"), true), QStringLiteral("광령리1 · 저장 안 됨"));
  }

  // 상태줄: 「작업 5186 · 중부원점 ▾」 names the origin and says it opens a choice; stock keeps 「작업 5186」.
  void workCrsNamesTheOriginInPaper() {
    paper(true);
    KaStatusBar bar;
    auto* crs = bar.findChild<QToolButton*>(QStringLiteral("crsButton"));
    QVERIFY(crs);
    bar.setWorkCrs(QStringLiteral("EPSG:5186"));
    QCOMPARE(crs->text(), QStringLiteral("작업 5186 · 중부원점 ▾"));
    bar.setWorkCrs(QStringLiteral("EPSG:5187"));
    QCOMPARE(crs->text(), QStringLiteral("작업 5187 · 동부원점 ▾"));
    paper(false);
    bar.setWorkCrs(QStringLiteral("EPSG:5186"));
    QCOMPARE(crs->text(), QStringLiteral("작업 5186"));
  }

  // 주소 찾기: the board's short 「주소·지번 찾기」; the example and Ctrl+F move to the tip.
  void addressSearchIsShortInPaper() {
    paper(true);
    KaAppBar bar;
    auto* search = bar.findChild<QLineEdit*>(QStringLiteral("appBarSearch"));
    QVERIFY(search);
    QCOMPARE(search->placeholderText(), QStringLiteral("주소·지번 찾기"));
    QVERIFY(search->toolTip().contains(QStringLiteral("하회종가길 40")) && search->toolTip().contains(QStringLiteral("Ctrl+F")));
  }

  // 축척 막대: figures at both ends and the middle, the unit after the end figure.
  void scaleTicksHaveAMiddleFigure() {
    QCOMPARE(KaMapScaleBar::ticks(20.0), (QStringList{QStringLiteral("0"), QStringLiteral("10"), QStringLiteral("20")}));
    QCOMPARE(KaMapScaleBar::ticks(5.0), (QStringList{QStringLiteral("0"), QStringLiteral("2.5"), QStringLiteral("5")}));
    QCOMPARE(KaMapScaleBar::ticks(2000.0), (QStringList{QStringLiteral("0"), QStringLiteral("1"), QStringLiteral("2")}));
    QCOMPARE(KaMapScaleBar::unit(20.0), QStringLiteral("m"));
    QCOMPARE(KaMapScaleBar::unit(2000.0), QStringLiteral("km"));
  }

  // 축척 막대 (새 모양): no white box, the 도북 arrow at the left; stock keeps the white box.
  void scaleBarHasNoBoxAndNorthFirstInPaper() {
    QWidget host;
    host.resize(800, 600);
    QgsMapCanvas canvas(&host);
    canvas.resize(800, 600);
    canvas.setDestinationCrs(QgsCoordinateReferenceSystem(QStringLiteral("EPSG:5186")));
    host.show();
    QVERIFY(QTest::qWaitForWindowExposed(&host));
    canvas.setExtent(QgsRectangle(200000, 400000, 200200, 400150));
    paper(true);
    KaMapScaleBar bar(&canvas, &host);
    QTRY_VERIFY(bar.isVisible() && bar.lengthMetres() > 0);
    const QImage image = alone(bar);
    QVERIFY2(image.pixelColor(image.width() / 2, 1).alpha() < 30, "no white box behind the bar");
    int ink = 0;
    for (int y = 0; y < image.height(); ++y)
      for (int x = 0; x < 14; ++x)
        if (image.pixelColor(x, y).alpha() > 200 && image.pixelColor(x, y).lightness() < 80) ++ink;
    QVERIFY2(ink > 10, qPrintable(QString::number(ink)));  // the arrow's ink half comes first
    paper(false);
    KaMapScaleBar stock(&canvas, &host);
    QTRY_VERIFY(stock.isVisible());
    QVERIFY(alone(stock).pixelColor(stock.width() / 2, 1).alpha() > 200);
  }

  // 배경 지도 (새 모양): a small caption with a rule, no box, icon or note; three plain buttons, the chosen one oat,
  // 「옛 지도 ▾」 because it opens the old-map list.
  void basemapButtonsArePlainInPaper() {
    paper(true);
    QWidget host;
    host.setAutoFillBackground(true);
    auto* card = new KaBasemapQuickCard(&host);
    card->setChecked(QStringLiteral("terrain"), true);
    card->resize(272, card->sizeHint().height());
    host.resize(card->size());
    host.show();
    QVERIFY(QTest::qWaitForWindowExposed(&host));
    QVERIFY(card->findChild<QLabel*>(QStringLiteral("basemapQuickNote"))->isHidden());
    QVERIFY(card->findChild<QFrame*>(QStringLiteral("basemapQuickRule")));
    QToolButton* old = card->button(QStringLiteral("old_map"));
    QCOMPARE(old->text(), QStringLiteral("옛 지도 ▾"));
    QCOMPARE(old->accessibleName(), QStringLiteral("옛 지도"));
    for (const QString& id : KaBasemapQuickCard::ids()) {
      QVERIFY2(card->button(id)->icon().isNull(), qPrintable(id));
      QVERIFY2(card->button(id)->width() >= (card->width() - 8) / 3 - 1, qPrintable(id));  // the three share the row
    }
    const QImage image = host.grab().toImage();
    const auto face = [&](const QString& id) {
      QToolButton* b = card->button(id);
      return image.pixelColor(b->mapTo(&host, QPoint(4, b->height() / 2)) * image.devicePixelRatio()).name();
    };
    QCOMPARE(face(QStringLiteral("terrain")), KaTheme::tokens().tileShown.name());
    QCOMPARE(face(QStringLiteral("satellite")), KaTheme::tokens().surface.name());
  }

  // The board's mark is 13 px with 8 px between the name and the state; readers hear it without the dot.
  void surveyMarkIsBoardSizedAndReadable() {
    paper(true);
    KaSurveyBadge badge;
    badge.setSurvey(QStringLiteral("광령리1"), true);
    QFont font = badge.font();
    font.setPixelSize(13);
    font.setWeight(QFont::Normal);
    const QFontMetrics fm(font);
    QCOMPARE(badge.sizeHint().width(),
             fm.horizontalAdvance(QStringLiteral("광령리1")) + 8 + 12 + fm.horizontalAdvance(QStringLiteral("저장 안 됨")) + 8);
    QCOMPARE(badge.accessibleName(), QStringLiteral("광령리1 저장 안 됨"));
  }

  // 「새 모양」 switched while the app runs (화면 보기): the words follow without a restart.
  void wordsFollowALiveSwitch() {
    paper(true);
    KaStatusBar bar;
    bar.setWorkCrs(QStringLiteral("EPSG:5186"));
    KaAppBar app;
    KaSurveyBadge badge;
    badge.setSurvey(QStringLiteral("광령리1"), true);
    for (QWidget* w : std::initializer_list<QWidget*>{&bar, &app, &badge}) w->show();
    auto* crs = bar.findChild<QToolButton*>(QStringLiteral("crsButton"));
    auto* search = app.findChild<QLineEdit*>(QStringLiteral("appBarSearch"));
    QCOMPARE(crs->accessibleName(), QStringLiteral("작업 5186 · 중부원점"));
    paper(false);
    QCoreApplication::processEvents();
    QCOMPARE(crs->text(), QStringLiteral("작업 5186"));
    QCOMPARE(search->placeholderText(), QStringLiteral("도로명·지번 (예: 하회종가길 40)  Ctrl+F"));
    QCOMPARE(badge.text(), QStringLiteral("광령리1 · 저장 안 됨"));
    paper(true);
    QCoreApplication::processEvents();
    QCOMPARE(crs->text(), QStringLiteral("작업 5186 · 중부원점 ▾"));
    QCOMPARE(search->placeholderText(), QStringLiteral("주소·지번 찾기"));
    QCOMPARE(badge.text(), QStringLiteral("광령리1 ● 저장 안 됨"));
  }

  void basemapSectionFollowsALiveSwitch() {
    paper(true);
    KaBasemapQuickCard card;
    card.show();
    auto* note = card.findChild<QLabel*>(QStringLiteral("basemapQuickNote"));
    QToolButton* old = card.button(QStringLiteral("old_map"));
    paper(false);
    QCoreApplication::processEvents();
    QVERIFY(!note->isHidden());
    QVERIFY(!card.button(QStringLiteral("satellite"))->icon().isNull());
    QCOMPARE(old->text(), QStringLiteral("옛 지도"));
    QVERIFY(!card.property("kaPlate").toBool());
    paper(true);
    QCoreApplication::processEvents();
    QVERIFY(note->isHidden());
    QVERIFY(card.button(QStringLiteral("satellite"))->icon().isNull());
    QCOMPARE(old->text(), QStringLiteral("옛 지도 ▾"));
    QVERIFY(card.property("kaPlate").toBool());
  }

  // 축척 막대: a three-figure end (100) keeps a space before its unit; a turned map keeps the whole arrow.
  void scaleUnitClearsTheEndFigureAndArrowFitsTurned() {
    QWidget host;
    host.resize(800, 600);
    QgsMapCanvas canvas(&host);
    canvas.resize(800, 600);
    canvas.setDestinationCrs(QgsCoordinateReferenceSystem(QStringLiteral("EPSG:5186")));
    host.show();
    QVERIFY(QTest::qWaitForWindowExposed(&host));
    canvas.setExtent(QgsRectangle(200000, 400000, 200800, 400600));  // 1 m a pixel: a 100 m bar
    paper(true);
    KaMapScaleBar bar(&canvas, &host);
    QTRY_COMPARE(bar.lengthMetres(), 100.0);
    QImage image = alone(bar);
    const auto ink = [&image](int x, int y) {
      const QColor c = image.pixelColor(x, y);
      return c.alpha() > 200 && c.lightness() < 120;
    };
    int barRight = 0;  // the bar's right outline on its middle row
    for (int x = 0; x < image.width(); ++x)
      if (ink(x, image.height() - 6)) barRight = x;
    QList<int> columns;  // inked columns of the figures right of the bar's end
    for (int x = barRight + 1; x < image.width(); ++x)
      for (int y = 0; y < image.height() - 10; ++y)
        if (ink(x, y)) {
          columns << x;
          break;
        }
    int widest = 0;
    for (int i = 1; i < columns.size(); ++i) widest = qMax(widest, columns.at(i) - columns.at(i - 1) - 1);
    QVERIFY2(widest >= 5, qPrintable(QString::number(widest)));  // a space's width, not a squeeze
    canvas.setRotation(90.0);
    QTRY_VERIFY(!alone(bar).isNull());
    image = alone(bar);
    for (int y = 0; y < image.height(); ++y) QVERIFY2(image.pixelColor(0, y).alpha() < 30, qPrintable(QString::number(y)));
  }
};

int main(int argc, char** argv) {
  QgsApplication app(argc, argv, true);
  QgsApplication::setPrefixPath(qEnvironmentVariable("QGIS_PREFIX_PATH", "A:/OSGeo4W/apps/qgis-dev"), true);
  QgsApplication::initQgis();
  QStandardPaths::setTestModeEnabled(true);
  KaTheme::apply(&app);
  TestMapPlate test;
  const int result = QTest::qExec(&test, argc, argv);
  QgsApplication::exitQgis();
  return result;
}

#include "test_map_plate.moc"
