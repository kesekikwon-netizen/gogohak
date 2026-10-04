// 「조용한 도판」: the recent-survey table is a three-rule table (ink rules above and below, a hairline under
// the head) and a chosen row is oat with no accent bar. The stock look keeps its accent bar and no rules.
#include <QtTest>
#include <QApplication>
#include <QRegularExpression>
#include "app/KaTheme.h"

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
