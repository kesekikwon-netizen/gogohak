// 「조용한 도판」 도면 인쇄 (2차 보드 판 5, 860 × 640): settings on the left over a dotted ledger (용지 · 배열 ·
// 붙인 크기 · 축척), the preview on the right under its own heading, 「A4 | A3」 as joined words and the skipped
// sheets named under the preview. The stock look keeps its one column.
#include <QtTest>
#include <QApplication>
#include <QComboBox>
#include <QLabel>
#include <QPageLayout>
#include <QPainter>
#include <QPdfWriter>
#include <QPushButton>
#include <QRadioButton>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QTemporaryDir>

#include "app/KaPrintDialog.h"

namespace {
QString makePdf(const QString& path, const QSizeF& mm) {
  QPdfWriter writer(path);
  writer.setResolution(72);
  const QSizeF portrait(qMin(mm.width(), mm.height()), qMax(mm.width(), mm.height()));
  writer.setPageLayout(QPageLayout(QPageSize(portrait, QPageSize::Millimeter),
                                   mm.width() > mm.height() ? QPageLayout::Landscape : QPageLayout::Portrait,
                                   QMarginsF(0, 0, 0, 0), QPageLayout::Millimeter));
  QPainter painter(&writer);
  painter.fillRect(QRectF(0, 0, writer.width(), writer.height()), QColor(200, 190, 170));
  return path;
}

QString plain(const QWidget& dialog, const char* name) {
  auto* label = dialog.findChild<QLabel*>(QString::fromLatin1(name));
  return label ? label->text().remove(QRegularExpression(QStringLiteral("<[^>]*>"))) : QStringLiteral("(없음)");
}
}  // namespace

class TestPrintPlate : public QObject {
  Q_OBJECT
private slots:
  void initTestCase() { QStandardPaths::setTestModeEnabled(true); }
  void cleanup() { qApp->setProperty("kaPaperLook", QVariant()); }

  void paperPrintIsTwoColumnsWithALedger() {
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString source = makePdf(dir.filePath(QStringLiteral("drawing.pdf")), QSizeF(841, 594));
    qApp->setProperty("kaPaperLook", true);
    KaPrintDialog dialog(source, QStringLiteral("시험 도면"), 100.0);
    dialog.setTiled(true);
    dialog.setSheet(KaPrintDialog::SheetA3);
    dialog.setOutput(KaPrintDialog::OutputDrawing);
    QVERIFY2(dialog.plan().ok, qPrintable(dialog.plan().error));
    const TilePlan& plan = dialog.plan();
    const int count = int(plan.sheets.size());
    auto* ledger = dialog.findChild<QWidget*>(QStringLiteral("printLedger"));
    QVERIFY(ledger);
    const QString sheet = plain(dialog, "printLedgerSheet");
    QVERIFY2(sheet.startsWith(QStringLiteral("A3")) && sheet.endsWith(QStringLiteral("%1장").arg(count)),
             qPrintable(sheet));
    QCOMPARE(plain(dialog, "printLedgerGrid"), QStringLiteral("가로 %1 × 세로 %2").arg(plan.cols).arg(plan.rows));
    QCOMPARE(plain(dialog, "printLedgerSize"), QStringLiteral("84.1 × 59.4 cm"));
    QCOMPARE(plain(dialog, "printLedgerScale"), QStringLiteral("1:100"));
    QVERIFY2(plain(dialog, "printPreviewHead").contains(QStringLiteral("%1장 중 %1장").arg(count)),
             qPrintable(plain(dialog, "printPreviewHead")));
    dialog.show();
    QVERIFY(QTest::qWaitForWindowExposed(&dialog));
    auto* preview = dialog.findChild<QWidget*>(QStringLiteral("printPreview"));
    const int ledgerRight = ledger->mapTo(&dialog, QPoint(ledger->width(), 0)).x();
    QVERIFY2(preview->mapTo(&dialog, QPoint(0, 0)).x() >= ledgerRight + 16,
             qPrintable(QStringLiteral("%1 vs %2").arg(preview->mapTo(&dialog, QPoint(0, 0)).x()).arg(ledgerRight)));
    qApp->setProperty("kaPaperLook", QVariant());
    KaPrintDialog stock(source, QStringLiteral("시험 도면"), 100.0);
    QVERIFY(!stock.findChild<QWidget*>(QStringLiteral("printLedger")));
  }

  // 「A4 | A3」: two joined words instead of the list; they follow setSheet and set the sheet.
  void sheetIsTwoJoinedWordsInPaper() {
    QTemporaryDir dir;
    const QString source = makePdf(dir.filePath(QStringLiteral("drawing.pdf")), QSizeF(841, 594));
    qApp->setProperty("kaPaperLook", true);
    KaPrintDialog dialog(source, QStringLiteral("시험 도면"));
    dialog.setTiled(true);
    auto* a4 = dialog.findChild<QPushButton*>(QStringLiteral("printSheetA4"));
    auto* a3 = dialog.findChild<QPushButton*>(QStringLiteral("printSheetA3"));
    QVERIFY(a4 && a3);
    QVERIFY(!dialog.findChild<QComboBox*>(QStringLiteral("printSheet"))->isVisibleTo(&dialog));
    dialog.setSheet(KaPrintDialog::SheetA3);
    QVERIFY(a3->isChecked() && !a4->isChecked());
    a4->click();
    QVERIFY(a4->isChecked() && !a3->isChecked());
    QVERIFY2(dialog.summary().startsWith(QStringLiteral("A4")), qPrintable(dialog.summary()));
  }

  // A sheet taken out in the preview is counted under it with 「모두 다시 넣기」 beside.
  void skippedSheetIsNamedUnderThePreview() {
    QTemporaryDir dir;
    const QString source = makePdf(dir.filePath(QStringLiteral("drawing.pdf")), QSizeF(841, 594));
    qApp->setProperty("kaPaperLook", true);
    KaPrintDialog dialog(source, QStringLiteral("시험 도면"));
    dialog.setTiled(true);
    dialog.setSheet(KaPrintDialog::SheetA3);
    dialog.setOverview(true);
    const int count = int(dialog.plan().sheets.size());
    QVERIFY(count > 1);
    auto* note = dialog.findChild<QLabel*>(QStringLiteral("printSkipNote"));
    QVERIFY(note);
    QVERIFY(!note->isVisibleTo(&dialog));
    dialog.toggleSheet(0);
    QVERIFY(note->isVisibleTo(&dialog));
    QCOMPARE(note->text(), QStringLiteral("이번에는 %1장만 찍습니다 (미리보기에서 뺀 장 1장)").arg(count - 1) +
                               QLatin1Char(0x0A) + QStringLiteral("안내 장은 다시 찍지 않습니다"));
    QVERIFY(plain(dialog, "printPreviewHead").contains(QStringLiteral("%1장 중 %2장").arg(count).arg(count - 1)));
    QVERIFY(dialog.findChild<QPushButton*>(QStringLiteral("printPickAll"))->isVisibleTo(&dialog));
  }

  // The settings column is the board's 380 px, but never so narrow that a 「방법」 line is cut off.
  void methodLinesAreNeverCut() {
    QTemporaryDir dir;
    const QString source = makePdf(dir.filePath(QStringLiteral("drawing.pdf")), QSizeF(841, 594));
    const QFont before = qApp->font();
    QFont wide = before;
    wide.setPixelSize(15);
    qApp->setFont(wide);
    qApp->setProperty("kaPaperLook", true);
    KaPrintDialog dialog(source, QStringLiteral("시험 도면"));
    dialog.show();
    QVERIFY(QTest::qWaitForWindowExposed(&dialog));
    qApp->setFont(before);
    for (const char* name : {"printModeTiles", "printModeFit"}) {
      auto* method = dialog.findChild<QRadioButton*>(QString::fromLatin1(name));
      QVERIFY2(method->width() >= method->sizeHint().width(),
               qPrintable(QStringLiteral("%1: %2 < %3").arg(QString::fromLatin1(name)).arg(method->width()).arg(method->sizeHint().width())));
    }
  }

  // A warning sits on the paper look's warn wash (#F6ECC8), not the stock amber.
  void warningUsesThePaperWash() {
    QTemporaryDir dir;
    const QString source = makePdf(dir.filePath(QStringLiteral("drawing.pdf")), QSizeF(841, 594));
    qApp->setProperty("kaPaperLook", true);
    KaPrintDialog dialog(source, QStringLiteral("시험 도면"));
    dialog.setTiled(false);
    dialog.setSheet(KaPrintDialog::SheetA3);
    QVERIFY(!dialog.warning().isEmpty());
    auto* warning = dialog.findChild<QLabel*>(QStringLiteral("printWarning"));
    warning->resize(300, 40);
    QCOMPARE(warning->grab().toImage().pixelColor(8, 20), QColor(0xF6, 0xEC, 0xC8));
  }

  // The preview lies on the oat desk of the board (#F0EEE6), not the stock grey.
  void previewIsOnTheDeskInPaper() {
    QTemporaryDir dir;
    const QString source = makePdf(dir.filePath(QStringLiteral("drawing.pdf")), QSizeF(841, 594));
    qApp->setProperty("kaPaperLook", true);
    KaPrintDialog dialog(source, QStringLiteral("시험 도면"));
    dialog.setTiled(true);
    auto* preview = dialog.findChild<QWidget*>(QStringLiteral("printPreview"));
    preview->resize(406, 370);
    QCOMPARE(preview->grab().toImage().pixelColor(2, 2), QColor(0xF0, 0xEE, 0xE6));
  }
};

QTEST_MAIN(TestPrintPlate)
#include "test_print_plate.moc"
