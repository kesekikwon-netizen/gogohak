// 「조용한 도판」 시작 화면 지층 (새 모양): bedrock and readable notices from the first frame, the strata timeline, the
// 12 px card corners and the strata reaching a wider card's edge. Moved out of tests/test_startup_splash.cpp to keep
// both under the 300-line limit.
#include "app/KaSplashStrata.h"
#include "app/KaStartupSplash.h"

#include <QApplication>
#include <QDir>
#include <QImage>
#include <QPainter>
#include <QSignalSpy>
#include <QtTest>
#include <QWidget>

#include <cmath>

namespace {

// Saves a frame for visual QA when KA_STARTUP_QA_OUTPUT_DIR is set.
void saveFrame(QWidget& widget, const QString& name) {
  const QString output = qEnvironmentVariable("KA_STARTUP_QA_OUTPUT_DIR");
  if (output.isEmpty()) return;
  QDir().mkpath(output);
  widget.grab().save(QDir(output).filePath(name));
}

double luminance(const QColor& c) {
  const auto lin = [](double v) { return v <= 0.04045 ? v / 12.92 : std::pow((v + 0.055) / 1.055, 2.4); };
  return 0.2126 * lin(c.redF()) + 0.7152 * lin(c.greenF()) + 0.0722 * lin(c.blueF());
}

// Contrast of ink (with its alpha) painted over an opaque background.
double inkContrast(const QColor& ink, const QColor& background) {
  const double a = ink.alphaF();
  const QColor text = QColor::fromRgbF(float(ink.redF() * a + background.redF() * (1 - a)),
                                       float(ink.greenF() * a + background.greenF() * (1 - a)),
                                       float(ink.blueF() * a + background.blueF() * (1 - a)));
  const double x = luminance(text), y = luminance(background);
  return (qMax(x, y) + 0.05) / (qMin(x, y) + 0.05);
}

}  // namespace

class StartupStrataTest : public QObject {
  Q_OBJECT
private slots:
  // 새 모양 notice: the app is busy until it is ready, so the bedrock, the name and the two notice
  // lines are in the first frame, and the notice ink reads on the bedrock. The strata above it
  // stack up only once the app is ready (user 2026-10-04: 지층이 아래부터 한 겹씩 쌓인다).
  void strataNoticeShowsBedrockAndReadableNoticesFromTheFirstFrame() {
    KaStartupSplash splash(nullptr, 3000, true);
    splash.show();
    QVERIFY(QTest::qWaitForWindowExposed(&splash));
    saveFrame(splash, QStringLiteral("startup-strata-first.png"));
    const QImage frame = splash.grab().toImage();
    const QRectF card = splash.cardRect();
    const double unit = card.height() / 400.0;
    const double dpr = frame.devicePixelRatio();
    const auto at = [&](double x, double y) { return frame.pixelColor(int((card.left() + x * unit) * dpr), int((card.top() + y * unit) * dpr)); };
    QCOMPARE(at(672, 380), KaSplashStrata::bedrock());  // beside the notice lines
    QVERIFY(inkContrast(KaSplashStrata::noticeInk(), KaSplashStrata::bedrock()) >= 4.5);
    QCOMPARE(at(350, 20), KaSplashStrata::paper());
    QCOMPARE(at(350, 300), KaSplashStrata::paper());  // the strata have not stacked yet
  }

  // 2차 보드 시작 화면: once the app is ready four boundaries are inked bottom up, the staff grows, the pit,
  // ground line, peg and Ⅰ–Ⅴ come in, stratum Ⅲ fills with clay, hairlines cross where the 기준점 lands
  // and fade to the triangle alone. Reduced motion shows the finished picture at once.
  void strataTimelineRunsInOrderAndReducedMotionSkipsIt() {
    for (int i = 0; i < 4; ++i) {
      QCOMPARE(KaSplashStrata::boundaryReveal(i, 0.4 + 0.35 * i), 0.0);
      QCOMPARE(KaSplashStrata::boundaryReveal(i, 0.75 + 0.35 * i), 1.0);
    }
    QCOMPARE(KaSplashStrata::pitReveal(1.9), 0.0);
    QCOMPARE(KaSplashStrata::pitReveal(2.4), 1.0);
    QCOMPARE(KaSplashStrata::clayReveal(2.6), 0.0);
    QCOMPARE(KaSplashStrata::clayReveal(3.2), 1.0);
    QCOMPARE(KaSplashStrata::markerReveal(3.2), 0.0);
    QCOMPARE(KaSplashStrata::markerReveal(3.6), 1.0);
    QVERIFY(KaSplashStrata::crossVisibility(3.6) > 0.9);
    QCOMPARE(KaSplashStrata::crossVisibility(KaSplashStrata::kTimelineSeconds), 0.0);
    // The 기준점's lower edge at (236, 238.6) is ink once it has landed, paper before.
    const auto markerEdge = [](const char* reduced) {
      qputenv("KA_HGIS_REDUCED_MOTION", reduced);
      KaStartupSplash splash(nullptr, 3000, true);
      splash.show();
      if (!QTest::qWaitForWindowExposed(&splash)) return QColor();
      saveFrame(splash, QStringLiteral("startup-strata-motion-%1.png").arg(QLatin1String(reduced)));
      const QRectF card = splash.cardRect();
      const double unit = card.height() / 400.0;
      const QImage frame = splash.grab().toImage();
      return frame.pixelColor(int((card.left() + 236 * unit) * frame.devicePixelRatio()), int((card.top() + 238.6 * unit) * frame.devicePixelRatio()));
    };
    const QColor waiting = markerEdge("0");
    const QColor finished = markerEdge("1");
    qunsetenv("KA_HGIS_REDUCED_MOTION");
    QCOMPARE(waiting, KaSplashStrata::paper());
    QVERIFY2(finished.isValid() && finished.lightness() < 110, qPrintable(finished.name()));
  }

  // The app is usually ready only near the end of the reading time; the drawing still gets its time
  // (user 2026-10-04 「모션그래픽이 안나와」): the home screen waits for it.
  void motionGetsItsTimeWhenTheAppIsReadyLate() {
    KaStartupSplash splash(nullptr, 300, true);
    QSignalSpy ready(&splash, &KaStartupSplash::readyToShow);
    splash.show();
    QTest::qWait(400);  // the reading time is already over when the app becomes ready
    splash.markReady();
    QTest::qWait(1000);
    QCOMPARE(ready.count(), 0);
    QTRY_COMPARE_WITH_TIMEOUT(ready.count(), 1, 5000);
  }

  void strataCardHasTwelvePixelCorners() {  // 12 px corners (blue card: 16): (4,4) is inside a 12 px arc, outside a 16 px one
    KaStartupSplash splash(nullptr, 3000, true); splash.show();
    QVERIFY(QTest::qWaitForWindowExposed(&splash));
    const QImage frame = splash.grab().toImage();
    QVERIFY(frame.pixelColor(int((splash.cardRect().left() + 4) * frame.devicePixelRatio()), int((splash.cardRect().top() + 4) * frame.devicePixelRatio())).alpha() > 200);
  }

  // A small screen shrinks the notice and its card becomes wider than 700:400: the strata still reach
  // the right edge (no stripe of paper beside the bedrock).
  void strataReachTheRightEdgeOfAWiderCard() {
    QImage image(760, 400, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);
    QPainter painter(&image);
    KaSplashStrata::paint(painter, QRectF(0, 0, 760, 400), 16, QPixmap(), KaSplashStrata::Frame());
    painter.end();
    QCOMPARE(image.pixelColor(750, 372), KaSplashStrata::bedrock());
  }
};

int main(int argc, char** argv) {
  // Tests exercise the motion regardless of the PC's animation setting.
  qputenv("KA_HGIS_REDUCED_MOTION", "0");
  QApplication app(argc, argv);
  StartupStrataTest test;
  return QTest::qExec(&test, argc, argv);
}
#include "test_startup_strata.moc"
