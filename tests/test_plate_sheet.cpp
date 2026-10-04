// 「조용한 도판」: the recent-survey table is a three-rule table (ink rules above and below, a hairline under
// the head) and a chosen row is oat with no accent bar. The stock look keeps its accent bar and no rules.
#include <QtTest>
#include <QApplication>
#include <QRegularExpression>
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
