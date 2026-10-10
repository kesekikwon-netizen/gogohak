// 범례 상자를 용지 안에 둔다 (보완 L4): a legend box that sticks out of the paper is pushed in by the shortest way; a
// box that already fits is left alone; a box bigger than the paper is lined up with its top left corner.
#include <QtTest>

#include "core/LegendPlacement.h"

class TestLegendPlacement : public QObject {
  Q_OBJECT
private slots:
  void aBoxInsideStaysWhereItIs() {
    const QRectF page(0, 0, 297, 210);
    QCOMPARE(LegendPlacement::insidePage(page, QRectF(20, 30, 60, 50)), QRectF(20, 30, 60, 50));
    QCOMPARE(LegendPlacement::insidePage(page, QRectF(237, 160, 60, 50)), QRectF(237, 160, 60, 50));  // touching the corner
  }

  void aBoxStickingOutIsPushedIn() {
    const QRectF page(0, 0, 297, 210);
    QCOMPARE(LegendPlacement::insidePage(page, QRectF(260, 30, 60, 50)), QRectF(237, 30, 60, 50));   // right
    QCOMPARE(LegendPlacement::insidePage(page, QRectF(20, 190, 60, 50)), QRectF(20, 160, 60, 50));   // bottom
    QCOMPARE(LegendPlacement::insidePage(page, QRectF(-15, -8, 60, 50)), QRectF(0, 0, 60, 50));      // top left
    QCOMPARE(LegendPlacement::insidePage(page, QRectF(290, 205, 60, 50)), QRectF(237, 160, 60, 50)); // the centre was inside: still pushed
  }

  void aBoxBiggerThanThePaperLinesUpWithItsTopLeft() {
    const QRectF page(0, 0, 100, 80);
    const QRectF placed = LegendPlacement::insidePage(page, QRectF(30, 20, 150, 120));
    QCOMPARE(placed.topLeft(), QPointF(0, 0));
    QCOMPARE(placed.size(), QSizeF(150, 120));  // never resized here
  }

  void aPageThatDoesNotStartAtZeroAndAnEmptyPage() {
    QCOMPARE(LegendPlacement::insidePage(QRectF(10, 10, 100, 80), QRectF(100, 70, 40, 30)), QRectF(70, 60, 40, 30));
    QCOMPARE(LegendPlacement::insidePage(QRectF(), QRectF(5, 5, 20, 20)), QRectF(5, 5, 20, 20));  // no paper known: untouched
  }
};

QTEST_APPLESS_MAIN(TestLegendPlacement)
#include "test_legend_placement.moc"
