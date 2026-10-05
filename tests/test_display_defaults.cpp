// 화면 보기 기본값 (사용자 요청 2026-10-05): a first start shows 새 모양 with 고대비 화면 on and 큰 글씨 off, as the
// user's own 화면 보기 menu was set. A choice the user made of 고대비 is kept either way.
#include <QtTest>

#include <QApplication>
#include <QSettings>
#include <QTemporaryDir>

#include "app/KaTheme.h"
#include "app/KaThemeOptions.h"

class TestDisplayDefaults : public QObject {
  Q_OBJECT
private slots:
  void initTestCase() {
    QVERIFY(m_settings.isValid());
    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, m_settings.path());
    QCoreApplication::setOrganizationName(QStringLiteral("ka-hgis-display-tests"));
    QCoreApplication::setApplicationName(QStringLiteral("display-defaults"));
    KaTheme::apply(qApp);
  }

  void firstStartIsPaperWithHighContrast() {
    QSettings().clear();
    KaTheme::applySavedDisplayOptions(qApp);
    QVERIFY(KaTheme::displayOptions().paperLook);
    QVERIFY2(KaTheme::displayOptions().highContrast, "고대비 is on until the user turns it off");
    QVERIFY(!KaTheme::displayOptions().largeText);
  }

  void anExplicitHighContrastOffIsKept() {
    QSettings().clear();
    KaTheme::DisplayOptions chosen;
    chosen.paperLook = true;
    chosen.highContrast = false;
    KaTheme::saveDisplayOptions(chosen);
    KaTheme::applySavedDisplayOptions(qApp);
    QVERIFY(KaTheme::displayOptions().paperLook);
    QVERIFY2(!KaTheme::displayOptions().highContrast, "the user's own choice survives the next start");
  }

private:
  QTemporaryDir m_settings;
};

QTEST_MAIN(TestDisplayDefaults)
#include "test_display_defaults.moc"
