// 세부 디자인 보완 (docs/intent/2026-10-11-design-detail-polish.md) 파일함 항목(M8): 바로가기 단추.
#include <QtTest>

#include <QApplication>
#include <QToolButton>

#include <qgsapplication.h>

#include "app/KaFileBrowserPanel.h"
#include "app/KaTheme.h"

namespace {

void paper(bool on) {
  KaTheme::DisplayOptions options;
  options.paperLook = on;
  KaTheme::setDisplayOptions(qApp, options);  // the app sheet is applied again for the look
}

}  // namespace

class TestBrowserPolish : public QObject {
  Q_OBJECT
private slots:
  void init() { paper(true); }
  void cleanup() { paper(false); }

  // 파일함 (보완 M8, 사용자 요청 R15): no 「SD 카드」 button (a memory card is opened with 「폴더 직접 찾기」); five
  // shortcuts, and 「상위」 alone at the right end.
  void fiveShortcutsAndUpAtTheRightEnd() {
    KaFileBrowserPanel panel;
    panel.resize(400, 320);
    panel.show();
    QVERIFY(QTest::qWaitForWindowExposed(&panel));
    auto button = [&panel](const char* name) { return panel.findChild<QToolButton*>(QString::fromLatin1(name)); };
    QVERIFY2(!button("btnBrowseSd"), "the SD card button is gone");
    QVERIFY(button("btnBrowseFolder") && button("btnBrowseUp"));
    QCOMPARE(button("btnBrowseFolder")->text(), QStringLiteral("폴더 직접 찾기"));
    QToolButton* up = button("btnBrowseUp");
    for (const char* name : {"btnBrowsePc", "btnBrowseDesktop", "btnBrowseDocs", "btnBrowseDownloads", "btnBrowseFolder"}) {
      QToolButton* shortcut = button(name);
      QVERIFY2(shortcut && shortcut->isVisible(), name);
      QVERIFY2(shortcut->geometry().right() < up->geometry().left(), name);  // 상위 stands last, on the right
    }
    QVERIFY2(up->geometry().right() >= panel.width() - 24, "상위 is at the right end of its row, not next to 폴더 직접 찾기");
    paper(false);  // the stock look keeps 상위 right behind the last shortcut, as before the SD button went
    QTRY_VERIFY2(up->x() - button("btnBrowseFolder")->geometry().right() < 24, qPrintable(QString::number(up->x())));
  }

  // 파일함 (보완 M8): the shortcuts are quiet 24 px words; the place you are at (상위) alone is oat.
  void shortcutsAreQuietWordsAndUpIsOat() {
    KaFileBrowserPanel panel;
    panel.resize(400, 320);
    panel.show();
    QVERIFY(QTest::qWaitForWindowExposed(&panel));
    if (QWidget* focused = QApplication::focusWidget()) focused->clearFocus();  // the first button would show its focus ring
    for (const char* name : {"btnBrowsePc", "btnBrowseDesktop", "btnBrowseDocs", "btnBrowseDownloads", "btnBrowseFolder"}) {
      auto* shortcut = panel.findChild<QToolButton*>(QString::fromLatin1(name));
      QVERIFY(shortcut);
      QCOMPARE(shortcut->height(), 24);
      QVERIFY2(shortcut->grab().toImage().pixelColor(0, shortcut->height() / 2).alpha() == 0, qPrintable(QStringLiteral("%1 %2").arg(QLatin1String(name), shortcut->grab().toImage().pixelColor(0, shortcut->height() / 2).name(QColor::HexArgb))));
    }
    auto* up = panel.findChild<QToolButton*>(QStringLiteral("btnBrowseUp"));
    QCOMPARE(up->height(), 24);
    const QColor face = up->grab().toImage().pixelColor(4, up->height() / 2);
    QVERIFY2(qAbs(face.red() - 0xE3) < 10 && qAbs(face.blue() - 0xCC) < 10, qPrintable(face.name()));
  }
};

int main(int argc, char** argv) {
  QgsApplication app(argc, argv, true);
  QgsApplication::setPrefixPath(qEnvironmentVariable("QGIS_PREFIX_PATH", "A:/OSGeo4W/apps/qgis-dev"), true);
  QgsApplication::initQgis();
  KaTheme::apply(&app);
  TestBrowserPolish test;
  const int result = QTest::qExec(&test, argc, argv);
  QgsApplication::exitQgis();
  return result;
}

#include "test_browser_polish.moc"
