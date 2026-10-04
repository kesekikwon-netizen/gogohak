// 「조용한 도판」: in 새 모양 a status chip is a mark and words (no pill, no edge); stock keeps its pill.
#include <QtTest>
#include <QApplication>
#include <QFontMetrics>
#include <QImage>
#include <QPainter>
#include "app/KaChip.h"
#include "app/KaTheme.h"

namespace {

using Tone = KaChip::Tone;

QImage renderChip(const QString& text, Tone tone) {
  const bool hasGlyph = !KaChip::defaultGlyph(tone).isEmpty() || tone == Tone::Accent;  // Warn has a glyph too
  const QSize size = KaChip::sizeForText(QFontMetrics(KaChip::chipFont(qApp->font())), text, hasGlyph) + QSize(0, 8);
  QImage image(size, QImage::Format_ARGB32);
  image.fill(Qt::transparent);
  QPainter painter(&image);
  painter.setFont(qApp->font());
  KaChip::paintChip(painter, QRect(QPoint(0, 0), size), text, tone, QString());
  return image;
}

class TestChipPlate : public QObject {
  Q_OBJECT
private slots:
  void cleanup() { KaTheme::setDisplayOptions(qApp, KaTheme::DisplayOptions()); }

  void stockChipIsAPill() {
    const QImage chip = renderChip(QStringLiteral("저장됨"), Tone::Ok);
    QVERIFY(chip.pixelColor(chip.width() - 3, chip.height() / 2).alpha() > 0);  // the pill's own ground
  }

  void paperChipHasNoPillAndAccentIsAClayDot() {
    KaTheme::DisplayOptions paper;
    paper.paperLook = true;
    KaTheme::setDisplayOptions(qApp, paper);
    const QImage saved = renderChip(QStringLiteral("저장됨"), Tone::Ok);
    QCOMPARE(saved.pixelColor(saved.width() - 2, saved.height() / 2).alpha(), 0);
    const QImage needs = renderChip(QStringLiteral("설정 필요"), Tone::Accent);
    const QColor dot = needs.pixelColor(3, needs.height() / 2);
    QVERIFY2(dot.alpha() == 255 && qAbs(dot.red() - 0xD9) < 8 && qAbs(dot.green() - 0x77) < 8 && qAbs(dot.blue() - 0x57) < 8,
             qPrintable(dot.name()));  // a 6 px clay dot where the pill used to start
  }

  // 「저장 안 됨」 and 「설정 필요」 need the user's hand: a clay dot with ink words, even on a caution-toned chip.
  void unsavedAndUnsetChipsAreClayDots() {
    KaTheme::DisplayOptions paper;
    paper.paperLook = true;
    KaTheme::setDisplayOptions(qApp, paper);
    for (const QString& words : {QStringLiteral("저장 안 됨 3건"), QStringLiteral("설정 필요")}) {
      const QImage chip = renderChip(words, Tone::Warn);
      const QColor dot = chip.pixelColor(3, chip.height() / 2);
      QVERIFY2(dot.alpha() == 255 && qAbs(dot.red() - 0xD9) < 8 && qAbs(dot.green() - 0x77) < 8, qPrintable(words + dot.name()));
    }
  }

  void paperSizeFollowsTheMark() {
    KaTheme::DisplayOptions paper;
    paper.paperLook = true;
    KaTheme::setDisplayOptions(qApp, paper);
    const QFontMetrics fm(KaChip::chipFont(qApp->font()));
    const int words = fm.horizontalAdvance(QStringLiteral("저장됨"));
    QCOMPARE(KaChip::sizeForText(fm, QStringLiteral("저장됨"), false).width(), words);  // no padding without a pill
  }
};

}  // namespace

QTEST_MAIN(TestChipPlate)
#include "test_chip_plate.moc"
