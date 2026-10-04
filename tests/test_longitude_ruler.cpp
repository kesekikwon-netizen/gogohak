// 「조용한 도판」 새 조사: a longitude ruler above the two origin buttons — 124°–132°E, the 128° line between
// 중부원점 and 동부원점, place ticks and a clay mark at the map centre. 새 모양 only; display only.
#include <QtTest>
#include <QLabel>
#include <QPushButton>
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
    ruler.resize(512, 88);
    QCOMPARE(qRound(ruler.xFor(128.0)), ruler.width() / 2);
    QVERIFY(ruler.xFor(126.0) < ruler.xFor(127.0));
    QCOMPARE(ruler.xFor(120.0), ruler.xFor(126.0));
  }

  // 2차 보드 판 5: 126°–130° across the full width, the map centre as a clay line and triangle over the axis
  // and named only for screen readers (nothing written in the top-left corner).
  void boardSpanAndClayCentre() {
    KaLongitudeRuler ruler(126.4);
    ruler.resize(512, ruler.height());
    QCOMPARE(qRound(ruler.xFor(126.0)), 0);
    QCOMPARE(qRound(ruler.xFor(130.0)), ruler.width());
    QVERIFY2(ruler.accessibleName().contains(QStringLiteral("지금 지도 중심은 126.4도")),
             qPrintable(ruler.accessibleName()));
    KaTheme::DisplayOptions paper;  // the ruler is only shown in 새 모양, where the accent is clay
    paper.paperLook = true;
    KaTheme::setDisplayOptions(qApp, paper);
    const QImage image = ruler.grab().toImage();
    KaTheme::setDisplayOptions(qApp, KaTheme::DisplayOptions());
    const QColor line = image.pixelColor(qRound(ruler.xFor(126.4) - 0.5), 30);
    QVERIFY2(line.red() > line.green() + 40, qPrintable(line.name()));
    int corner = 0;  // ink in the top-left corner, left of 「중부원점」
    for (int y = 0; y < 14; ++y)
      for (int x = 0; x < 120; ++x)
        if (image.pixelColor(x, y).lightness() < 140) ++corner;
    QCOMPARE(corner, 0);
  }

  // 2차 보드 판 5 새 조사: 「작업 좌표계 … 동경 128°를 경계로 나뉩니다」 over the ruler, 「추천」 inside the
  // recommended origin, and three short lines (the zone, the boundary rule, later change and 5179).
  void paperDialogReadsLikeTheBoard() {
    KaTheme::DisplayOptions paper;
    paper.paperLook = true;
    KaTheme::setDisplayOptions(qApp, paper);
    KaNewSurveyDialog dialog(QStringLiteral("EPSG:5187"), 126.4);
    KaTheme::setDisplayOptions(qApp, KaTheme::DisplayOptions());
    const auto text = [&dialog](const char* name) {
      auto* label = dialog.findChild<QLabel*>(QString::fromLatin1(name));
      return label ? label->text() : QStringLiteral("(없음)");
    };
    QCOMPARE(text("newSurveyCrsBoundary"), QStringLiteral("동경 128°를 경계로 나뉩니다"));
    QCOMPARE(text("newSurveyCrsSuggestion"), QStringLiteral("지금 지도 중심(동경 126.4°)은 중부원점(5186) 구역입니다."));
    QCOMPARE(text("newSurveyCrsHint"), QStringLiteral("경계 가까운 곳은 발주처·측량 성과의 원점을 따르세요."));
    QCOMPARE(text("newSurveyTip"), QStringLiteral("작업 좌표계는 나중에 화면 아래에서 바꿀 수 있습니다.\n제출 파일은 언제나 5179로 바뀝니다."));
    auto* central = dialog.findChild<QPushButton*>(QStringLiteral("newSurveyCrs5186"));
    auto* east = dialog.findChild<QPushButton*>(QStringLiteral("newSurveyCrs5187"));
    QVERIFY(central->findChild<QLabel*>(QStringLiteral("newSurveyCrsRecommend")));
    QVERIFY(!east->findChild<QLabel*>(QStringLiteral("newSurveyCrsRecommend")));
  }

  // The place names stand above the axis with ticks down to it (2차 보드), not under the degrees.
  void placeNamesStandAboveTheAxis() {
    KaLongitudeRuler ruler(126.4);
    ruler.resize(512, ruler.height());
    const QImage image = ruler.grab().toImage();
    const int boundary = qRound(ruler.xFor(128.0));
    int words = 0;  // ink letters in the band above the axis (the 128° line itself does not count)
    for (int y = 18; y <= 28; ++y)
      for (int x = 0; x < image.width(); ++x)
        if (qAbs(x - boundary) > 2 && image.pixelColor(x, y).lightness() < 60) ++words;
    QVERIFY2(words > 40, qPrintable(QString::number(words)));
  }
};

QTEST_MAIN(TestLongitudeRuler)
#include "test_longitude_ruler.moc"
