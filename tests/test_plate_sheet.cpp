// 「조용한 도판」: the recent-survey table is a three-rule table (ink rules above and below, a hairline under
// the head) and a chosen row is oat with no accent bar. The stock look keeps its accent bar and no rules.
#include <QtTest>
#include <QApplication>
#include <QHBoxLayout>
#include <QPushButton>
#include <QSignalSpy>
#include <QTabBar>
#include <QTabWidget>
#include <QToolButton>
#include <QRegularExpression>
#include "app/KaTabClose.h"
#include "app/KaTheme.h"
#include "app/KaWindowSize.h"

namespace {

QString captured(const QString& pattern) {
  return QRegularExpression(pattern).match(qApp->styleSheet()).captured(1).toLower();
}

class TestPlateSheet : public QObject {
  Q_OBJECT
private slots:
  void initTestCase() { KaTheme::apply(qApp); }
  void cleanup() { KaTheme::setDisplayOptions(qApp, KaTheme::DisplayOptions()); }

  void stockKeepsTheAccentBarAndNoRule() {
    const QString bar = QStringLiteral("QTableWidget#recentSurveyList::item:selected \{ border-left: 2px solid (#[0-9a-fA-F]{6});");
    QCOMPARE(captured(bar), KaTheme::tokens().accent.name());
    const QString rule = QStringLiteral("QTableWidget#recentSurveyList \{ border-top: 1px solid (#[0-9a-fA-F]{6});");
    QCOMPARE(captured(rule), KaTheme::tokens().surface.name());  // invisible on the white table ground
  }

  // 수치 대장: the inspector's 면적 and 둘레 read as 16 px figures between ink rules (13 px, no rule in stock).
  void measureFigureIsLedgerInPaper() {
    const QString size = QStringLiteral("QFrame#kaMeasureField QLabel#kaMeasureValue \\{ color: #[0-9a-fA-F]{6}; font-size: (\\d+)px;");
    QCOMPARE(captured(size), QStringLiteral("13"));
    KaTheme::DisplayOptions paper;
    paper.paperLook = true;
    KaTheme::setDisplayOptions(qApp, paper);
    QCOMPARE(captured(size), QStringLiteral("16"));
    const QString rule = QStringLiteral("QFrame#kaMeasureField \\{ border-top: 1px solid (#[0-9a-fA-F]{6});");
    QCOMPARE(captured(rule), KaTheme::tokens().ink.name());
  }

  // 지도는 그림: the map is a square figure in 새 모양 (r0), a rounded card (r10) in stock.
  void mapFigureIsSquareInPaper() {
    const QString radius = QStringLiteral("QFrame#mapCard \\{ background: #[0-9a-fA-F]{6}; border-radius: (\\d+)px;");
    QCOMPARE(captured(radius), QStringLiteral("10"));
    KaTheme::DisplayOptions paper;
    paper.paperLook = true;
    KaTheme::setDisplayOptions(qApp, paper);
    QCOMPARE(captured(radius), QStringLiteral("0"));
  }

  // 판은 각지다: the docked cards (레이어·파일함·기록) lose their rounded corners in 새 모양.
  void dockedCardsAreSquareInPaper() {
    const QString radius = QStringLiteral("QFrame#terrainFilesCard \\{(?:[^}]*?)border-radius: (\\d+)px;");
    QCOMPARE(captured(radius), QStringLiteral("10"));
    KaTheme::DisplayOptions paper;
    paper.paperLook = true;
    KaTheme::setDisplayOptions(qApp, paper);
    QCOMPARE(captured(radius), QStringLiteral("0"));
  }

  // 창 바탕: a window's body is the one ivory sheet in 새 모양 (inputs stay white); stock keeps the desk colour.
  void dialogBodyIsIvoryInPaper() {
    const QString ground = QStringLiteral("(?m)^QDialog \\{ background: (#[0-9a-fA-F]{6}); \\}");
    QCOMPARE(captured(ground), KaTheme::tokens().desk.name());
    KaTheme::DisplayOptions paper;
    paper.paperLook = true;
    KaTheme::setDisplayOptions(qApp, paper);
    QCOMPARE(captured(ground), KaTheme::tokens().chrome.name());
  }

  // Type scale: the chosen object's name is the 20 px serif Heading, the home survey name the 28 px Title,
  // and the home section heads 「최근 조사」·「작업 순서」 are 15 px sans (not serif).
  void namesFollowTheTypeScaleInPaper() {
    KaTheme::DisplayOptions paper;
    paper.paperLook = true;
    KaTheme::setDisplayOptions(qApp, paper);
    const auto size = [](const char* name) {
      return captured(QStringLiteral("QLabel#%1 \\{ font-family: [^;]+; font-weight: [^;]+; font-size: (\\d+)px;").arg(QLatin1String(name)));
    };
    QCOMPARE(size("kaFeatureCardName"), QStringLiteral("20"));
    QCOMPARE(size("startContinueName"), QStringLiteral("28"));
    QVERIFY(captured(QStringLiteral("QLabel#startRecentTitle \\{ font-family: ([^;]+);")).contains(QStringLiteral("malgun gothic")));
  }

  // 고른 단추·입력칸: a chosen push button (5186 | 5187 in 새 조사) and a focused text field are edged in ink
  // in 새 모양, not clay; stock keeps its accent edge and deep accent text.
  void chosenButtonAndFocusedFieldAreInkInPaper() {
    const QString chosen = QStringLiteral("(?m)^QPushButton:enabled:checked \\{\\s*background: #[0-9a-fA-F]{6};\\s*color: (#[0-9a-fA-F]{6});\\s*border-color: (#[0-9a-fA-F]{6});");
    const QString edge = QStringLiteral("(?m)^QPushButton:enabled:checked \\{\\s*background: #[0-9a-fA-F]{6};\\s*color: #[0-9a-fA-F]{6};\\s*border-color: (#[0-9a-fA-F]{6});");
    const QString focus = QStringLiteral("(?m)^QLineEdit:focus \\{\\s*border-color: (#[0-9a-fA-F]{6});");
    QCOMPARE(captured(chosen), KaTheme::tokens().accentDeep.name());
    QCOMPARE(captured(edge), KaTheme::tokens().accent.name());
    QCOMPARE(captured(focus), KaTheme::tokens().accent.name());
    KaTheme::DisplayOptions paper;
    paper.paperLook = true;
    KaTheme::setDisplayOptions(qApp, paper);
    const QString ink = KaTheme::tokens().ink.name();
    QCOMPARE(captured(chosen), ink);
    QCOMPARE(captured(edge), ink);
    QCOMPARE(captured(focus), ink);
  }

  // 고른 칸: a chosen tool button (A4 | A3, 세로 | 가로 in 용지 설정) reads ink on oat with an ink underline in
  // 새 모양; stock keeps its deep accent text and accent underline.
  void chosenToolButtonIsInkInPaper() {
    const QString rule = QStringLiteral("(?m)^QToolButton:enabled:checked \\{\\s*background: #[0-9a-fA-F]{6};\\s*color: (#[0-9a-fA-F]{6});\\s*border: 2px solid transparent;\\s*border-bottom: 2px solid (#[0-9a-fA-F]{6});");
    const auto both = [&rule]() {
      const auto m = QRegularExpression(rule).match(qApp->styleSheet());
      return QStringList{m.captured(1).toLower(), m.captured(2).toLower()};
    };
    QCOMPARE(both(), (QStringList{KaTheme::tokens().accentDeep.name(), KaTheme::tokens().accent.name()}));
    KaTheme::DisplayOptions paper;
    paper.paperLook = true;
    KaTheme::setDisplayOptions(qApp, paper);
    const QString ink = KaTheme::tokens().ink.name();
    QCOMPARE(both(), (QStringList{ink, ink}));
  }

  // 축척 단추: the drawing's scale presets (20 … 25000) are mono figures in 새 모양; stock keeps the UI face.
  void scaleChipsAreMonoInPaper() {
    const QString family = QStringLiteral("(?m)^QToolButton#scaleChip \\{ font-family: ([^;]+);");
    QVERIFY2(captured(family).contains(QStringLiteral("malgun gothic")), qPrintable(captured(family)));
    KaTheme::DisplayOptions paper;
    paper.paperLook = true;
    KaTheme::setDisplayOptions(qApp, paper);
    QVERIFY2(captured(family).startsWith(QStringLiteral("\"consolas\"")), qPrintable(captured(family)));
  }

  // 홈 판 (2차 보드 판 2): the home cards lose their box in 새 모양 and the wordmark is 34 px; stock keeps
  // the white rounded cards and the 40 px name.
  void homeCardsHaveNoBoxInPaper() {
    const QString face = QStringLiteral("(?m)^QFrame#startRecentCard, QFrame#startGuideCard, QFrame#startGuideCard2, QFrame#startConnectionCard \\{\\s*background: ([^;]+);");
    const QString name = QStringLiteral("(?m)^QLabel#startHeroName \\{ font-size: (\\d+)px;");
    QCOMPARE(captured(face), KaTheme::tokens().surface.name());
    QCOMPARE(captured(name), QStringLiteral("40"));
    KaTheme::DisplayOptions paper;
    paper.paperLook = true;
    KaTheme::setDisplayOptions(qApp, paper);
    QCOMPARE(captured(face), QStringLiteral("transparent"));
    QCOMPARE(captured(name), QStringLiteral("34"));
  }

  // 풍선 도움말과 메뉴 (설계 2.4): the tip is ink with ivory words, r4; a menu row is 32 high with the light 「desk」
  // wash under the pointer. Stock keeps the white tip and the oat row.
  void tooltipIsInkAndMenuRowsAreLightInPaper() {
    const QString tip = QStringLiteral("(?m)^QToolTip \\{\\s*color: ([^;]+);\\s*background: ([^;]+);");
    const QString hover = QStringLiteral("QMenu::item:selected \\{ background: (#[0-9a-fA-F]{6});");
    const auto both = [](const QString& pattern) {
      const auto m = QRegularExpression(pattern).match(qApp->styleSheet());
      return QStringList{m.captured(1).trimmed().toLower(), m.captured(2).trimmed().toLower()};
    };
    QCOMPARE(both(tip), (QStringList{KaTheme::tokens().ink.name(), KaTheme::tokens().surface.name()}));
    QCOMPARE(captured(hover), KaTheme::tokens().selected.name());
    KaTheme::DisplayOptions paper;
    paper.paperLook = true;
    KaTheme::setDisplayOptions(qApp, paper);
    QCOMPARE(both(tip), (QStringList{QStringLiteral("#faf9f5"), KaTheme::tokens().ink.name()}));
    QCOMPARE(captured(hover), KaTheme::tokens().desk.name());
  }

  // 설계 2.4: tooltip text is the 12 px Caption in 새 모양; stock keeps the UI size.
  void tooltipTextIsTheCaptionSizeInPaper() {
    const QRegularExpression size(QStringLiteral("(?m)^QToolTip \\{[^}]*font-size: (\\d+)px;"));
    const QString stock = size.match(qApp->styleSheet()).captured(1);
    KaTheme::DisplayOptions paper;
    paper.paperLook = true;
    KaTheme::setDisplayOptions(qApp, paper);
    const QString plate = size.match(qApp->styleSheet()).captured(1);
    KaTheme::setDisplayOptions(qApp, KaTheme::DisplayOptions());
    QCOMPARE(stock, QStringLiteral("13"));
    QCOMPARE(plate, QStringLiteral("12"));
  }

  // 「A4 | A3」 joined: the chosen A3 keeps a whole ink outline, its left edge included (보드).
  void joinedSheetWordsKeepTheChosenOutline() {
    KaTheme::DisplayOptions paper;
    paper.paperLook = true;
    KaTheme::setDisplayOptions(qApp, paper);
    QWidget row;
    auto* line = new QHBoxLayout(&row);
    line->setSpacing(0);
    auto* a4 = new QPushButton(QStringLiteral("A4"), &row);
    auto* a3 = new QPushButton(QStringLiteral("A3"), &row);
    a4->setObjectName(QStringLiteral("printSheetA4"));
    a3->setObjectName(QStringLiteral("printSheetA3"));
    for (QPushButton* word : {a4, a3}) {
      word->setCheckable(true);
      line->addWidget(word);
    }
    a3->setChecked(true);
    row.show();
    QVERIFY(QTest::qWaitForWindowExposed(&row));
    const QColor edge = row.grab().toImage().pixelColor(a3->x(), a3->y() + a3->height() / 2);
    KaTheme::setDisplayOptions(qApp, KaTheme::DisplayOptions());
    QVERIFY2(edge.lightness() < 60, qPrintable(edge.name()));
  }

  // 「도면 ✕」 (보드 판 4): a closable view tab shows a clear muted ✕ that closes it; tabs that cannot close keep none.
  void closableTabShowsAClearCrossInPaper() {
    qApp->setProperty("kaPaperLook", true);
    QTabWidget tabs;
    tabs.setTabsClosable(true);
    tabs.addTab(new QWidget, QStringLiteral("홈"));
    tabs.tabBar()->setTabButton(0, QTabBar::RightSide, nullptr);
    KaTabClose::follow(&tabs);
    tabs.addTab(new QWidget, QStringLiteral("도면"));
    tabs.setCurrentIndex(1);
    auto* cross = qobject_cast<QToolButton*>(tabs.tabBar()->tabButton(1, QTabBar::RightSide));
    QVERIFY(cross && cross->objectName() == QStringLiteral("viewTabClose"));
    QVERIFY(!tabs.tabBar()->tabButton(0, QTabBar::RightSide));
    QSignalSpy closing(&tabs, &QTabWidget::tabCloseRequested);
    cross->click();
    QCOMPARE(closing.count(), 1);
    QCOMPARE(closing.first().first().toInt(), 1);
    qApp->setProperty("kaPaperLook", QVariant());
    QTabWidget stock;
    stock.setTabsClosable(true);
    KaTabClose::follow(&stock);
    stock.addTab(new QWidget, QStringLiteral("도면"));
    stock.setCurrentIndex(0);
    QVERIFY(stock.tabBar()->tabButton(0, QTabBar::RightSide)->objectName() != QStringLiteral("viewTabClose"));
  }

  // 창 너비: 새 모양 windows come in three widths (S 440 · M 560 · L 860); stock keeps each window's own width.
  void windowWidthsAreTheThreeSizesInPaper() {
    QCOMPARE(KaWindowSize::width(KaWindowSize::Medium, 460), 460);
    KaTheme::DisplayOptions paper;
    paper.paperLook = true;
    KaTheme::setDisplayOptions(qApp, paper);
    QCOMPARE(KaWindowSize::width(KaWindowSize::Small, 380), 440);
    QCOMPARE(KaWindowSize::width(KaWindowSize::Medium, 460), 560);
    QCOMPARE(KaWindowSize::width(KaWindowSize::Large, 820), 860);
  }

  // 도면: 범례·표제란이 켜지면 귀리색에 회색 고리, 고른 방위 칸은 귀리색에 먹 테두리,
  // 용지 그림(세로·가로)은 각진다. 예전 모양은 강조색 테두리와 둥근 모서리 그대로.
  void sheetTilesAreOatInPaper() {
    const QString sheet = QStringLiteral("QToolButton#legendTile:enabled:checked,\\s*QToolButton#btnTitleBlock:enabled:checked \\{\\s*background: (#[0-9a-fA-F]{6});\\s*color: (#[0-9a-fA-F]{6});\\s*border: 1px solid (#[0-9a-fA-F]{6});");
    const QString north = QStringLiteral("QToolButton#northSwatch:enabled:checked \\{\\s*background: (#[0-9a-fA-F]{6});\\s*color: (#[0-9a-fA-F]{6});\\s*border: 1px solid (#[0-9a-fA-F]{6});");
    const auto three = [](const QString& pattern) {
      const auto m = QRegularExpression(pattern).match(qApp->styleSheet());
      return QStringList{m.captured(1).toLower(), m.captured(2).toLower(), m.captured(3).toLower()};
    };
    const KaTheme::Tokens stock = KaTheme::tokens();
    const QStringList accent = {stock.selected.name(), stock.accentDeep.name(), stock.accent.name()};
    QCOMPARE(three(sheet), accent);
    QCOMPARE(three(north), accent);
    QCOMPARE(KaTheme::depictedPaperRadius(), 3);
    KaTheme::DisplayOptions paper;
    paper.paperLook = true;
    KaTheme::setDisplayOptions(qApp, paper);
    const KaTheme::Tokens t = KaTheme::tokens();
    QCOMPARE(three(sheet), (QStringList{t.tileShown.name(), t.ink.name(), t.tileShownBorder.name()}));
    QCOMPARE(three(north), (QStringList{t.chosenFill.name(), t.ink.name(), t.ink.name()}));
    QCOMPARE(KaTheme::depictedPaperRadius(), 0);
  }

  void paperTableHasInkRulesAndOatRow() {
    KaTheme::DisplayOptions paper;
    paper.paperLook = true;
    KaTheme::setDisplayOptions(qApp, paper);
    const KaTheme::Tokens t = KaTheme::tokens();
    const QString bar = QStringLiteral("QTableWidget#recentSurveyList::item:selected \{ border-left: 2px solid (#[0-9a-fA-F]{6});");
    const QString top = QStringLiteral("QTableWidget#recentSurveyList \{ border-top: 1px solid (#[0-9a-fA-F]{6}); border-bottom: 1px solid (#[0-9a-fA-F]{6});");
    QCOMPARE(captured(bar), t.selected.name());  // the bar takes the row's own colour: no accent bar
    QCOMPARE(captured(top), t.ink.name());
  }
};

}  // namespace

QTEST_MAIN(TestPlateSheet)
#include "test_plate_sheet.moc"
