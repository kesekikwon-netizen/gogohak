// 세부 디자인 보완 (docs/intent/2026-10-11-design-detail-polish.md) 홈 항목: 「조용한 도판」 기준서와 어긋난 곳을
// 하나씩 고친 것. 새 모양(paperLook)에서 격리된 설정으로 본다. test_home_plate.cpp는 줄 수 한도(300)에 닿아 따로 둔다.
#include <QtTest>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFrame>
#include <QLabel>
#include <QPushButton>
#include <QRect>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QSettings>
#include <QStandardPaths>
#include <QTemporaryDir>

#include <qgsapplication.h>

#include "app/KaChip.h"
#include "app/KaHomeConnectionCard.h"
#include "app/KaHomeText.h"
#include "app/KaStartPage.h"
#include "app/KaTheme.h"
#include "core/RecentSurveys.h"
#include "core/SurveyFacts.h"

namespace {

QString touch(const QString& path) {
  QDir().mkpath(QFileInfo(path).absolutePath());
  QFile file(path);
  if (!file.open(QIODevice::WriteOnly)) return {};
  file.write("fixture");
  return QFileInfo(path).absoluteFilePath();
}

}  // namespace

class TestHomePolish : public QObject {
  Q_OBJECT
  QTemporaryDir m_dir;

private slots:
  void init() {
    KaTheme::DisplayOptions paper;
    paper.paperLook = true;
    KaTheme::setDisplayOptions(qApp, paper);
  }
  void cleanup() {
    RecentSurveys::userSettings().clear();
    KaTheme::setDisplayOptions(qApp, KaTheme::DisplayOptions());
  }

  // 제출 준비 (보완 H5): one plain line under 「도면 · 제출」, no box and no fill; the counts read in the mono face.
  void submitReadyIsAPlainLine() {
    const QString path = touch(m_dir.filePath(QStringLiteral("k/광령리1.gpkg")));
    {
      QSettings st = RecentSurveys::userSettings();
      RecentSurveys::remember(st, path, QStringLiteral("광령리1"));
      SurveyFacts::rememberCounts(st, path, 1, 6, QStringLiteral("EPSG:5186"));
      SurveyFacts::rememberCheck(st, path, 5, 3);
    }
    KaStartPage page;
    page.resize(1600, 900);
    page.show();
    QVERIFY(QTest::qWaitForWindowExposed(&page));
    auto* ready = page.findChild<QFrame*>(QStringLiteral("startSubmitReady"));
    QVERIFY(ready);
    QTRY_VERIFY(ready->isVisible());
    const QImage image = ready->grab().toImage();
    // the top row and the two ends of it are empty paper: an edge or a fill would show there
    QVERIFY2(image.pixelColor(0, 0).alpha() == 0, "an edge or a fill is drawn at the top left");
    QVERIFY2(image.pixelColor(image.width() - 1, 0).alpha() == 0, "an edge or a fill is drawn at the top right");
    QVERIFY2(image.pixelColor(image.width() / 2, image.height() - 1).alpha() == 0, "an edge or a fill is drawn at the bottom");
    auto* errors = page.findChild<KaChip*>(QStringLiteral("startSubmitErrors"));
    QVERIFY(errors);
    QCOMPARE(errors->text(), QStringLiteral("오류 5"));
    // the box's padding is gone too: the line starts at the card's text edge
    auto* label = ready->findChild<QLabel*>();
    QVERIFY(label);
    QVERIFY2(label->x() <= 1, qPrintable(QString::number(label->x())));
  }

  void chipDigitsCanReadInMono() {
    const auto runs = KaChip::figureRuns(QStringLiteral("오류 12건 3"));
    QCOMPARE(runs.size(), 4);
    QCOMPARE(runs[0], qMakePair(QStringLiteral("오류 "), false));
    QCOMPARE(runs[1], qMakePair(QStringLiteral("12"), true));
    QCOMPARE(runs[2], qMakePair(QStringLiteral("건 "), false));
    QCOMPARE(runs[3], qMakePair(QStringLiteral("3"), true));
    QVERIFY(KaChip::figureRuns(QString()).isEmpty());
    QCOMPARE(KaChip::figureRuns(QStringLiteral("다음")).size(), 1);
  }

  // 연결 상태 (보완 H6): 「설정」 is a quiet 24 px word button (no edge, no fill at rest) and a 1 x 32 rule stands
  // between the cells of the strip.
  void connectionSetupIsQuietWithRulesBetween() {
    QWidget host;
    auto* column = new QVBoxLayout(&host);
    KaHomeConnectionCard card;
    card.placeIn(column, true);
    KaHomeStatus::Inputs inputs;
    for (auto source : {AccountStatus::Source::VworldKey, AccountStatus::Source::TopographicAccount,
                        AccountStatus::Source::HeritageAccount}) {
      AccountStatus::Entry entry;
      entry.source = source;
      entry.ready = true;
      inputs.accounts << entry;
    }
    inputs.crsDatabase = true;
    card.setInputs(inputs);
    host.resize(1400, 120);
    host.show();
    QVERIFY(QTest::qWaitForWindowExposed(&host));
    const auto rules = card.findChildren<QFrame*>(QStringLiteral("startConnRule"));
    QCOMPARE(rules.size(), 2);  // between three cells
    for (QFrame* rule : rules) {
      QTRY_VERIFY(rule->isVisible());
      QCOMPARE(rule->width(), 1);
      QCOMPARE(rule->height(), 32);
    }
    auto* internet = card.findChild<QPushButton*>(QStringLiteral("startEnvInternet"));  // the strip's one bordered button: 24 px
    QVERIFY(internet);
    QCOMPARE(internet->height(), 24);
    const auto setups = card.findChildren<QPushButton*>(QStringLiteral("startConnectionSetup"));
    QCOMPARE(setups.size(), 3);
    for (QPushButton* setup : setups) {
      QCOMPARE(setup->height(), 24);
      const QImage face = setup->grab().toImage();
      QVERIFY2(face.pixelColor(0, face.height() / 2).alpha() == 0, "an edge or a fill is drawn at the left");
      QVERIFY2(face.pixelColor(face.width() / 2, 0).alpha() == 0, "an edge or a fill is drawn at the top");
    }
  }

  // 「이어서 작업」 표제란 (보완 H2): one frame of three rows (caption · name beside the button · cells). The caption row
  // runs over the button column with 「저장됨」 at its right end, the button sits in the second row, and nothing hangs
  // under the cells (the empty recovery-note row used to leave a band, so the bottom rules were 13 px apart).
  void titleBlockIsThreeRowsInOneFrame() {
    const QString path = touch(m_dir.filePath(QStringLiteral("m/광령리1.gpkg")));
    {
      QSettings st = RecentSurveys::userSettings();
      RecentSurveys::remember(st, path, QStringLiteral("광령리1"));
      SurveyFacts::rememberCounts(st, path, 1, 6, QStringLiteral("EPSG:5186"));
      SurveyFacts::rememberCheck(st, path, 1, 3);
    }
    KaStartPage page;
    page.resize(1600, 900);
    page.show();
    QVERIFY(QTest::qWaitForWindowExposed(&page));
    auto* block = page.findChild<QFrame*>(QStringLiteral("startTitleBlock"));
    QVERIFY(block);
    auto part = [&](const char* name) {
      auto* widget = block->findChild<QWidget*>(QString::fromLatin1(name));
      return widget ? QRect(widget->mapTo(block, QPoint()), widget->size()) : QRect();
    };
    const QRect tag = part("startTitleTag"), state = part("startTitleState"), name = part("startTitleName");
    const QRect resume = part("startContinueBtn"), folder = part("startCellFolder"), check = part("startCellCheck");
    QVERIFY(!tag.isNull() && !state.isNull() && !name.isNull() && !resume.isNull() && !folder.isNull() && !check.isNull());
    QVERIFY2(tag.bottom() < name.top() && state.bottom() < name.top(), "the caption row is above the name");
    QVERIFY2(state.right() >= block->width() - 40, qPrintable(QStringLiteral("state right %1 of %2").arg(state.right()).arg(block->width())));
    QVERIFY2(resume.top() > tag.bottom() && resume.bottom() < folder.top(), "the button sits in the second row");
    QVERIFY2(resume.right() <= block->width() - 24, "the button is inset 24 from the frame");
    const int below = block->height() - 1 - check.bottom();
    QVERIFY2(below <= 14, qPrintable(QStringLiteral("%1 px under the cells").arg(below)));
  }

  // 최근 조사 표 (보완 H4): the mouse over a row washes the whole row (every cell), not the one cell it is over.
  void recentRowWashesWholeOnHover() {
    for (const char* folder : {"n", "o"}) {
      const QString path = touch(m_dir.filePath(QStringLiteral("%1/광령리.gpkg").arg(QLatin1String(folder))));
      QSettings st = RecentSurveys::userSettings();
      RecentSurveys::remember(st, path, QStringLiteral("광령리%1").arg(QLatin1String(folder)));
    }
    KaStartPage page;
    page.resize(1600, 900);
    page.show();
    QVERIFY(QTest::qWaitForWindowExposed(&page));
    auto* table = page.findChild<QTableWidget*>(QStringLiteral("recentSurveyList"));
    QVERIFY(table && table->rowCount() >= 2);
    const int last = table->columnCount() - 1;
    auto edgeOf = [&](int row, int column) {  // a pixel in a cell's left padding, clear of its text
      const QRect cell = table->visualRect(table->model()->index(row, column));
      return table->viewport()->grab().toImage().pixelColor(cell.left() + 3, cell.center().y());
    };
    QCOMPARE(edgeOf(0, last), edgeOf(1, last));  // nothing under the mouse: the rows look alike
    const QRect first = table->visualRect(table->model()->index(0, 0));
    QTest::mouseMove(table->viewport(), first.center());
    QTRY_VERIFY2(edgeOf(0, last) != edgeOf(1, last), "the cells away from the mouse stay plain");
    QCOMPARE(edgeOf(0, last), edgeOf(0, 0));  // one wash along the whole row
  }

  // 폴더 칸과 최근 조사 줄의 경로 (보완 H3, 설계 판): the cell ends with the survey's name, the table's path line with only
  // the last folder; a short one stays whole.
  void folderCellAndRowPathFollowTheBoard() {
    const QString path = touch(m_dir.filePath(QStringLiteral("p/q/r/폴더/기록.gpkg")));
    {
      QSettings st = RecentSurveys::userSettings();
      RecentSurveys::remember(st, path, QStringLiteral("현장기록"));
    }
    KaStartPage page;
    page.resize(1600, 900);
    page.reload();
    auto* block = page.findChild<QFrame*>(QStringLiteral("startTitleBlock"));
    QVERIFY(block);
    QCOMPARE(block->findChild<QLabel*>(QStringLiteral("startCellFolder"))->text(), QStringLiteral("… › 폴더 › 현장기록"));
    // the survey's own name closes the cell: 「… › 폴더 › 조사명」, and a folder named like the survey is not repeated
    QCOMPARE(KaHomeText::folderTail(QStringLiteral("C: › Temp › survey-verify"), QStringLiteral("verify1-저장")),
             QStringLiteral("… › survey-verify › verify1-저장"));
    QCOMPARE(KaHomeText::folderTail(QStringLiteral("바탕 화면 › 광령리1"), QStringLiteral("광령리1")), QStringLiteral("바탕 화면 › 광령리1"));
    QCOMPARE(KaHomeText::folderTail(QStringLiteral("문서"), QStringLiteral("광령리1")), QStringLiteral("문서 › 광령리1"));
    QCOMPARE(KaHomeText::folderEnd(QStringLiteral("C: › Users › 권을 › ka-hgis-survey-verify")), QStringLiteral("… › ka-hgis-survey-verify"));
    QCOMPARE(KaHomeText::folderEnd(QStringLiteral("바탕 화면 › andong")), QStringLiteral("바탕 화면 › andong"));
    QCOMPARE(KaHomeText::folderEnd(QStringLiteral("문서")), QStringLiteral("문서"));
  }
};

int main(int argc, char** argv) {
  if (qEnvironmentVariableIsEmpty("KA_HGIS_LOG_DIR")) qputenv("KA_HGIS_LOG_DIR", QDir::tempPath().toUtf8() + "/ka-hgis-test-logs");
  qputenv("KA_HGIS_HOME_PROBE", "0");
  QgsApplication app(argc, argv, true);
  QgsApplication::setPrefixPath(qEnvironmentVariable("QGIS_PREFIX_PATH", "A:/OSGeo4W/apps/qgis-dev"), true);
  QgsApplication::initQgis();
  QStandardPaths::setTestModeEnabled(true);
  QTemporaryDir settings;  // never the user's ini
  QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, settings.path());
  KaTheme::apply(&app);
  TestHomePolish test;
  const int result = QTest::qExec(&test, argc, argv);
  QgsApplication::exitQgis();
  return result;
}

#include "test_home_polish.moc"
