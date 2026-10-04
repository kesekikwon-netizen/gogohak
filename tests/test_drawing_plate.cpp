// 「조용한 도판」 도면 (2차 보드 판 4): 「PDF 내보내기 | 인쇄」 sit at the foot of the right panel, under a hairline and
// outside its scrolling, so they never scroll away; stock keeps them in the first card's row. No QGIS.
#include <QtTest>

#include <QApplication>
#include <QFrame>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QToolButton>
#include <QVBoxLayout>

#include "app/KaDrawingStudioPlate.h"

namespace {
// The panel as KaDrawingStudio builds it: a scrolling side with a card whose row holds the two output buttons.
QScrollArea* panel(QToolButton** pdf, QToolButton** print) {
  auto* scroll = new QScrollArea;
  scroll->setObjectName(QStringLiteral("drawingInspectorScroll"));
  auto* side = new QWidget;
  auto* sideLay = new QVBoxLayout(side);
  auto* card = new QFrame(side);
  auto* row = new QHBoxLayout(card);
  *pdf = new QToolButton(card);
  (*pdf)->setObjectName(QStringLiteral("btnPrimary"));
  (*pdf)->setText(QStringLiteral("PDF 내보내기"));
  *print = new QToolButton(card);
  (*print)->setObjectName(QStringLiteral("btnPrint"));
  (*print)->setText(QStringLiteral("인쇄"));
  row->addWidget(*pdf);
  row->addWidget(*print);
  sideLay->addWidget(card);
  sideLay->addStretch(1);
  scroll->setWidget(side);
  scroll->setWidgetResizable(true);
  return scroll;
}
}  // namespace

class TestDrawingPlate : public QObject {
  Q_OBJECT
private slots:
  void cleanup() { qApp->setProperty("kaPaperLook", QVariant()); }

  void outputButtonsSitAtThePanelFootInPaper() {
    QToolButton* pdf = nullptr;
    QToolButton* print = nullptr;
    QScrollArea* scroll = panel(&pdf, &print);
    qApp->setProperty("kaPaperLook", true);
    std::unique_ptr<QWidget> column(KaDrawingStudioPlate::inspectorColumn(scroll));
    QVERIFY(column.get() != scroll);
    QVERIFY(scroll->parentWidget() == column.get());
    auto* foot = column->findChild<QWidget*>(QStringLiteral("studioOutputFoot"));
    QVERIFY(foot);
    QVERIFY(pdf->parentWidget() == foot && print->parentWidget() == foot);
    column->resize(320, 600);
    column->show();
    QVERIFY(QTest::qWaitForWindowExposed(column.get()));
    QVERIFY(foot->y() >= scroll->geometry().bottom());
    QVERIFY(pdf->x() < print->x());
  }

  void stockKeepsTheButtonsInTheirCard() {
    QToolButton* pdf = nullptr;
    QToolButton* print = nullptr;
    std::unique_ptr<QScrollArea> scroll(panel(&pdf, &print));
    QWidget* parentBefore = pdf->parentWidget();
    QCOMPARE(KaDrawingStudioPlate::inspectorColumn(scroll.get()), static_cast<QWidget*>(scroll.get()));
    QCOMPARE(pdf->parentWidget(), parentBefore);
  }
};

QTEST_MAIN(TestDrawingPlate)
#include "test_drawing_plate.moc"
