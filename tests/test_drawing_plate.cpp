// 「조용한 도판」 도면 (2차 보드 판 4): 「PDF 내보내기 | 인쇄」 sit at the foot of the right panel, under a hairline and
// outside its scrolling, so they never scroll away; stock keeps them in the first card's row. No QGIS.
#include <QtTest>

#include <QApplication>
#include <QFrame>
#include <QHBoxLayout>
#include <QListWidget>
#include <QScrollArea>
#include <QSpacerItem>
#include <QToolButton>
#include <QVBoxLayout>

#include "app/KaDrawingStudioPlate.h"
#include "app/KaFileRowDelegate.h"

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

  // 아래 도구 줄 (보드 판 4): one strip of joined 96 × 52 cells with a hairline between them and one-line words;
  // stock keeps its spaced buttons.
  void bottomToolsAreOneStripOfCellsInPaper() {
    const auto strip = [] {
      auto* tools = new QWidget;
      tools->setObjectName(QStringLiteral("layoutBottomTools"));
      auto* line = new QHBoxLayout(tools);
      line->setContentsMargins(14, 6, 14, 6);
      line->setSpacing(22);
      for (const QString& word : {QStringLiteral("좌표점"), QStringLiteral("항목\n이동"), QStringLiteral("도면 목록")}) {
        auto* cell = new QToolButton(tools);
        cell->setText(word);
        line->addWidget(cell);
      }
      return tools;
    };
    qApp->setProperty("kaPaperLook", true);
    std::unique_ptr<QWidget> tools(strip());
    QCOMPARE(KaDrawingStudioPlate::bottomStrip(tools.get()), tools.get());
    QCOMPARE(tools->layout()->spacing(), 0);
    QCOMPARE(tools->layout()->contentsMargins(), QMargins());
    const auto cells = tools->findChildren<QToolButton*>();
    QCOMPARE(cells.size(), 3);
    for (QToolButton* cell : cells) {
      QCOMPARE(cell->minimumSize(), QSize(96, 52));
      QVERIFY2(!cell->text().contains(QLatin1Char(0x0A)), qPrintable(cell->text()));
    }
    QCOMPARE(cells.at(0)->property("kaCell").toString(), QStringLiteral("first"));
    QCOMPARE(cells.at(1)->property("kaCell").toString(), QStringLiteral("rest"));
    qApp->setProperty("kaPaperLook", QVariant());
    std::unique_ptr<QWidget> stock(strip());
    KaDrawingStudioPlate::bottomStrip(stock.get());
    QCOMPARE(stock->layout()->spacing(), 22);
  }

  // 「세트 ▾」 (보드 판 4): the layer-set word shows its list mark in 새 모양 and follows a switch of the look.
  void setWordShowsItsListMarkAndFollowsTheLook() {
    QToolButton word;
    qApp->setProperty("kaPaperLook", true);
    KaDrawingStudioPlate::setWord(&word);
    QCOMPARE(word.text(), QStringLiteral("세트 ▾"));
    qApp->setProperty("kaPaperLook", false);
    QEvent restyle(QEvent::StyleChange);
    QApplication::sendEvent(&word, &restyle);
    QCOMPARE(word.text(), QStringLiteral("세트"));
  }

  // 파일함 목록 (보드 판 4): a folder or file mark, the name, and a file's extension set apart at the right in mono;
  // rows 24 px. Stock keeps the plain 「[폴더] 이름」 rows.
  void fileRowsShowNameAndExtensionApartInPaper() {
    using Parts = QPair<QString, QString>;
    QCOMPARE(KaFileRowDelegate::parts(QStringLiteral("[폴더] 광령리1"), true), Parts(QStringLiteral("광령리1"), QString()));
    QCOMPARE(KaFileRowDelegate::parts(QStringLiteral("광령리_측량_0923.dxf"), false),
             Parts(QStringLiteral("광령리_측량_0923"), QStringLiteral(".dxf")));
    QCOMPARE(KaFileRowDelegate::parts(QStringLiteral("C:\\"), true), Parts(QStringLiteral("C:\\"), QString()));
    QCOMPARE(KaFileRowDelegate::parts(QStringLiteral("README"), false), Parts(QStringLiteral("README"), QString()));
    QListWidget list;
    auto* delegate = new KaFileRowDelegate(&list);
    list.setItemDelegate(delegate);
    list.addItem(QStringLiteral("광령리_측량_0923.dxf"));
    QStyleOptionViewItem option;
    option.initFrom(&list);
    const QModelIndex row = list.model()->index(0, 0);
    const int stockHeight = delegate->sizeHint(option, row).height();
    qApp->setProperty("kaPaperLook", true);
    QCOMPARE(delegate->sizeHint(option, row).height(), 24);
    qApp->setProperty("kaPaperLook", QVariant());
    QCOMPARE(delegate->sizeHint(option, row).height(), stockHeight);
  }

  // 축척 단추 (보드 판 4): seven a row in one joined table — no gap between the rows, inner edges shared; stock keeps
  // three a row with gaps.
  void scaleChipsAreOneJoinedTableInPaper() {
    QCOMPARE(KaDrawingStudioPlate::chipsPerRow(), 3);
    QCOMPARE(KaDrawingStudioPlate::chipGap(6), 6);
    qApp->setProperty("kaPaperLook", true);
    QCOMPARE(KaDrawingStudioPlate::chipsPerRow(), 7);
    QCOMPARE(KaDrawingStudioPlate::chipGap(6), 0);
    QWidget bar;  // as KaDrawingStudio builds it: a gap before each row of chips
    auto* column = new QVBoxLayout(&bar);
    for (int r = 0; r < 2; ++r) {
      column->addSpacerItem(new QSpacerItem(0, 10, QSizePolicy::Minimum, QSizePolicy::MinimumExpanding));
      auto* row = new QHBoxLayout;
      for (int c = 0; c < 7; ++c) {
        auto* chip = new QToolButton(&bar);
        chip->setObjectName(QStringLiteral("scaleChip"));
        row->addWidget(chip);
      }
      column->addLayout(row);
    }
    KaDrawingStudioPlate::joinScaleTable(&bar);
    const auto chips = bar.findChildren<QToolButton*>(QStringLiteral("scaleChip"));
    QCOMPARE(chips.at(0)->property("kaRow").toString(), QStringLiteral("first"));
    QCOMPARE(chips.at(0)->property("kaCol").toString(), QStringLiteral("first"));
    QCOMPARE(chips.at(8)->property("kaRow").toString(), QStringLiteral("rest"));
    QCOMPARE(chips.at(8)->property("kaCol").toString(), QStringLiteral("rest"));
    QCOMPARE(column->itemAt(0)->spacerItem()->sizeHint().height(), 10);  // the gap above the table stays
    QCOMPARE(column->itemAt(2)->spacerItem()->sizeHint().height(), 0);   // none between its rows
  }

  // 「축척 글자 · 좌표계 · 좌표 격자」 (보드 판 4): words without icons sharing the row, 32 px high.
  void sheetMarksAreWordsSharingTheRowInPaper() {
    const auto build = [](QWidget& content) {
      auto* row = new QHBoxLayout(&content);
      for (const QString& word : {QStringLiteral("축척 글자"), QStringLiteral("좌표계"), QStringLiteral("좌표 격자")}) {
        auto* tile = new QToolButton(&content);
        tile->setText(word);
        tile->setToolTip(word);
        if (word == QStringLiteral("좌표 격자")) {  // its tooltip changes with the grid; it is known by name
          tile->setObjectName(QStringLiteral("btnGridSettings"));
          tile->setToolTip(QStringLiteral("좌표 격자를 켭니다"));
        }
        tile->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        tile->setFixedHeight(64);
        row->addWidget(tile);
      }
      row->addStretch(1);
      return row;
    };
    qApp->setProperty("kaPaperLook", true);
    QWidget content;
    QHBoxLayout* row = build(content);
    KaDrawingStudioPlate::wordTiles(&content);
    for (QToolButton* tile : content.findChildren<QToolButton*>()) {
      QCOMPARE(tile->toolButtonStyle(), Qt::ToolButtonTextOnly);
      QCOMPARE(tile->maximumHeight(), 32);
      QCOMPARE(row->stretch(row->indexOf(tile)), 1);
    }
    QCOMPARE(row->itemAt(3)->spacerItem()->sizeHint().width(), 0);
    qApp->setProperty("kaPaperLook", QVariant());
    QWidget stock;
    build(stock);
    KaDrawingStudioPlate::wordTiles(&stock);
    QCOMPARE(stock.findChildren<QToolButton*>().first()->toolButtonStyle(), Qt::ToolButtonTextUnderIcon);
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
