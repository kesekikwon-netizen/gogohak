// 「조용한 도판」 새 조사: a longitude ruler above the two origin buttons — 124°–132°E, the 128° line between
// 중부원점 and 동부원점, place ticks and a clay mark at the map centre. 새 모양 only; display only.
#include <QtTest>
#include <cmath>
#include <limits>
#include "app/KaLongitudeRuler.h"
#include "app/KaNewSurveyDialog.h"
#include "app/KaTheme.h"

class TestLongitudeRuler : public QObject {
  Q_OBJECT
private slots:
  void stockDialogHasNoRuler() {
    KaNewSurveyDialog dialog(QStringLiteral("EPSG:5186"), 126.4);
    QVERIFY(!dialog.findChild<KaLongitudeRuler*>());
  }
  void paperDialogShowsTheRulerWithTheMapCentre() {
    KaTheme::DisplayOptions paper;
    paper.paperLook = true;
    KaTheme::setDisplayOptions(qApp, paper);
    KaNewSurveyDialog dialog(QStringLiteral("EPSG:5186"), 126.4);
    KaTheme::setDisplayOptions(qApp, KaTheme::DisplayOptions());
    auto* ruler = dialog.findChild<KaLongitudeRuler*>(QStringLiteral("newSurveyLongitudeRuler"));
    QVERIFY(ruler);
    QCOMPARE(ruler->centre(), 126.4);
  }
  void the128LineSitsHalfwayAndOutsideIsClamped() {
    KaLongitudeRuler ruler(std::numeric_limits<double>::quiet_NaN());
    ruler.resize(512, 72);
    QCOMPARE(qRound(ruler.xFor(128.0)), ruler.width() / 2);
    QVERIFY(ruler.xFor(124.0) < ruler.xFor(126.0));
    QCOMPARE(ruler.xFor(120.0), ruler.xFor(124.0));
  }
};

QTEST_MAIN(TestLongitudeRuler)
#include "test_longitude_ruler.moc"
