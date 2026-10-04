// 「조용한 도판」 받는 순서 (2차 보드 판 5 「수치지형도 내려받기」): six steps on one line — a done step is a ✓ in an ink
// circle, the current one an ink square, later ones a grey number — with their names under them. In 새 모양 it takes
// the place of the stock 「도엽 확인 → 로그인 → …」 line. No QGIS.
#include <QtTest>

#include <QApplication>
#include <QLabel>
#include <QVBoxLayout>

#include "app/KaStageRail.h"

namespace {
const QStringList kStages{QStringLiteral("도엽 확인"), QStringLiteral("로그인"), QStringLiteral("신청"),
                          QStringLiteral("파일 준비"), QStringLiteral("다운로드"), QStringLiteral("지도에 올리기")};

// Dark (ink) pixels inside the step circle of column `step`.
int inkInCircle(const QImage& image, int step, int steps) {
  const int column = image.width() / steps;
  const int cx = column * step + column / 2;
  int ink = 0;
  for (int y = 1; y < 18; ++y)
    for (int x = cx - 8; x <= cx + 8; ++x)
      if (image.pixelColor(x, y).lightness() < 70) ++ink;
  return ink;
}
}  // namespace

class TestStageRail : public QObject {
  Q_OBJECT
private slots:
  void cleanup() { qApp->setProperty("kaPaperLook", QVariant()); }

  void currentIsKeptWithinTheSteps() {
    KaStageRail rail(kStages);
    rail.setCurrent(2);
    QCOMPARE(rail.current(), 2);
    rail.setCurrent(-3);
    QCOMPARE(rail.current(), 0);
    rail.setCurrent(99);
    QCOMPARE(rail.current(), 6);  // every step done
  }

  void doneAndCurrentStepsAreInkLaterStepsGrey() {
    KaStageRail rail(kStages);
    rail.resize(600, rail.sizeHint().height());
    rail.setCurrent(2);
    const QImage image = rail.grab().toImage();
    QVERIFY2(inkInCircle(image, 0, 6) > 10, "a done step shows its ✓ in ink");
    QVERIFY2(inkInCircle(image, 2, 6) > 30, "the current step shows its ink square");
    QCOMPARE(inkInCircle(image, 4, 6), 0);  // a later step is grey only
  }

  void paperPutsTheRailInPlaceOfTheArrowLine() {
    QWidget box;
    auto* column = new QVBoxLayout(&box);
    auto* trail = new QLabel(kStages.join(QStringLiteral(" → ")), &box);
    column->addWidget(trail);
    QVERIFY(!KaStageRail::replace(trail));  // stock keeps its line
    QVERIFY(!trail->isHidden());
    qApp->setProperty("kaPaperLook", true);
    KaStageRail* rail = KaStageRail::replace(trail);
    QVERIFY(rail);
    QVERIFY(trail->isHidden());
    QCOMPARE(column->indexOf(rail), column->indexOf(trail) + 1);
    QCOMPARE(rail->stages(), kStages);
  }
};

QTEST_MAIN(TestStageRail)
#include "test_stage_rail.moc"
