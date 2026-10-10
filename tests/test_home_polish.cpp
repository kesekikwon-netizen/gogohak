// 세부 디자인 보완 (docs/intent/2026-10-11-design-detail-polish.md) 홈 항목: 「조용한 도판」 기준서와 어긋난 곳을
// 하나씩 고친 것. 새 모양(paperLook)에서 격리된 설정으로 본다. test_home_plate.cpp는 줄 수 한도(300)에 닿아 따로 둔다.
#include <QtTest>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFrame>
#include <QLabel>
#include <QSettings>
#include <QStandardPaths>
#include <QTemporaryDir>

#include <qgsapplication.h>

#include "app/KaChip.h"
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
