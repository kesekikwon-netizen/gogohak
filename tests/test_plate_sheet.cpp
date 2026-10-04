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
