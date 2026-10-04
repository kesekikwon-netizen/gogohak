// 「조용한 도판」 home (2차 보드 판 2): no hero band, the 「이어서 작업」 title block with the survey's record,
// and 「새 조사」 quiet while there is a survey to continue. Isolated settings; probe off.
#include <QtTest>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFrame>
#include <QLabel>
#include <QPushButton>
#include <QSettings>
#include <QStandardPaths>
#include <QTemporaryDir>

#include <qgsapplication.h>

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

QString text(const QWidget& root, const char* name) {
  auto* label = root.findChild<QLabel*>(QString::fromLatin1(name));
  return label ? label->text() : QStringLiteral("(없음)");
}

}  // namespace

class TestHomePlate : public QObject {
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

  void titleBlockShowsTheSurveyRecord() {
    const QString path = touch(m_dir.filePath(QStringLiteral("광령리1/광령리1.gpkg")));
    {
      QSettings st = RecentSurveys::userSettings();
      RecentSurveys::remember(st, path, QStringLiteral("광령리1"));
      SurveyFacts::rememberCounts(st, path, 1, 6, QStringLiteral("EPSG:5186"));
      SurveyFacts::rememberCheck(st, path, 1, 3);
    }
    KaStartPage page;
    page.resize(1600, 900);
    page.reload();
    auto* block = page.findChild<QFrame*>(QStringLiteral("startTitleBlock"));
    QVERIFY(block);
    QCOMPARE(text(*block, "startTitleName"), QStringLiteral("광령리1"));
    QCOMPARE(text(*block, "startCellCrs"), QStringLiteral("5186 중부원점"));
    QCOMPARE(text(*block, "startCellAreas"), QStringLiteral("1 곳"));
    QCOMPARE(text(*block, "startCellFeatures"), QStringLiteral("6 기"));
    QCOMPARE(text(*block, "startCellCheck"), QStringLiteral("오류 1 · 주의 3"));
    QVERIFY(text(*block, "startCellFolder").contains(QStringLiteral("광령리1")));
  }

  void unknownRecordShowsDashesNotGuesses() {
    const QString path = touch(m_dir.filePath(QStringLiteral("b/새조사.gpkg")));
    {
      QSettings st = RecentSurveys::userSettings();
      RecentSurveys::remember(st, path, QStringLiteral("새조사"));
    }
    KaStartPage page;
    page.reload();
    auto* block = page.findChild<QFrame*>(QStringLiteral("startTitleBlock"));
    QVERIFY(block);
    QCOMPARE(text(*block, "startCellCrs"), QStringLiteral("—"));
    QCOMPARE(text(*block, "startCellAreas"), QStringLiteral("—"));
    QCOMPARE(text(*block, "startCellCheck"), QStringLiteral("검수 전"));
  }

  void newSurveyIsQuietWhileThereIsASurveyToContinue() {
    {
      KaStartPage empty;
      empty.reload();
      auto* start = empty.findChild<QPushButton*>(QStringLiteral("startNewBtn"));
      QVERIFY(start);
      QVERIFY(!start->property("kaQuiet").toBool());  // nothing to continue: 새 조사 is the main button
    }
    const QString path = touch(m_dir.filePath(QStringLiteral("c/이어서.gpkg")));
    {
      QSettings st = RecentSurveys::userSettings();
      RecentSurveys::remember(st, path, QStringLiteral("이어서"));
    }
    KaStartPage page;
    page.reload();
    QVERIFY(page.findChild<QPushButton*>(QStringLiteral("startNewBtn"))->property("kaQuiet").toBool());
  }

  void heroDrawsNoBand() {
    KaStartPage page;
    page.resize(1600, 900);
    page.show();
    QVERIFY(QTest::qWaitForWindowExposed(&page));
    auto* hero = page.findChild<QWidget*>(QStringLiteral("startHero"));
    QVERIFY(hero);
    const QImage image = page.grab().toImage();
    const qreal dpr = image.devicePixelRatio();
    const QPoint inHero = hero->mapTo(&page, QPoint(hero->width() / 2, 6));
    const QPoint outside = hero->mapTo(&page, QPoint(hero->width() / 2, hero->height() + 8));  // the gap under it
    QCOMPARE(image.pixelColor(int(inHero.x() * dpr), int(inHero.y() * dpr)),
             image.pixelColor(int(outside.x() * dpr), int(outside.y() * dpr)));
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
  TestHomePlate test;
  const int result = QTest::qExec(&test, argc, argv);
  QgsApplication::exitQgis();
  return result;
}

#include "test_home_plate.moc"
