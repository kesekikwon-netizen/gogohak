// P5 shell-chrome, splitter side: KaShellFocus with a three-pane splitter (left panel |
// map | inspector), the F10 right-panel fold and KaLayerOpacityRail::setTopInset. No QGIS.
// tests/test_shell.cpp is at its 300-line limit, so these live here.
#include <QtTest>

#include <QApplication>
#include <QMouseEvent>
#include <QSettings>
#include <QSignalSpy>
#include <QSplitter>
#include <QTemporaryDir>

#include "app/KaLayerOpacityRail.h"
#include "app/KaShellFocus.h"

namespace {
struct ThreePane {
  QSplitter split{Qt::Horizontal};
  QWidget* left = new QWidget(&split);
  QWidget* map = new QWidget(&split);
  QWidget* right = new QWidget(&split);
  ThreePane() {
    split.addWidget(left);
    split.addWidget(map);
    split.addWidget(right);
    split.resize(1366, 500);
  }
  int tolerance() const { return split.handleWidth() + 2; }
};

bool within(int a, int b, int tolerance) { return qAbs(a - b) <= tolerance; }
}  // namespace

class TestShellFocus : public QObject {
  Q_OBJECT
 private slots:
  void initTestCase() {
    // The rail remembers its place in QSettings: keep it in a throwaway ini, never the app's settings.
    QVERIFY(m_settings.isValid());
    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, m_settings.path());
    QCoreApplication::setOrganizationName(QStringLiteral("ka-shell-focus-tests"));
  }

  void focus_threeWidgetSplitKeepsRightSize() {
    ThreePane pane;
    QCOMPARE(KaShellFocus::sizesWithLeft(&pane.split, 260), (QList<int>{260, 1366 - 260 - 272, 272}));
    pane.split.show();
    QVERIFY(QTest::qWaitForWindowExposed(&pane.split));
    pane.split.setSizes({300, 794, 272});
    const int right0 = pane.split.sizes().at(2);
    const QList<int> sizes = KaShellFocus::sizesWithLeft(&pane.split, 260);
    QCOMPARE(sizes.size(), 3);
    QCOMPARE(sizes.at(0), 260);
    QCOMPARE(sizes.at(2), right0);
    pane.split.setSizes(sizes);
    QVERIFY(within(pane.split.sizes().at(0), 260, pane.tolerance()));
    QVERIFY(within(pane.split.sizes().at(2), right0, pane.tolerance()));
    KaShellFocus focus(&pane.split);
    focus.applyDefaultWidthOnce();
    QVERIFY(within(pane.split.sizes().at(0), KaShellFocus::defaultLeftPanelWidth(pane.split.width()), pane.tolerance()));
    QVERIFY2(within(pane.split.sizes().at(2), right0, pane.tolerance()), "the ratio default leaves the inspector alone");
    pane.right->hide();
    QCOMPARE(KaShellFocus::sizesWithLeft(&pane.split, 260).at(2), 0);
    QSplitter two(Qt::Horizontal);
    two.addWidget(new QWidget(&two));
    two.addWidget(new QWidget(&two));
    two.resize(1000, 300);
    QCOMPARE(KaShellFocus::sizesWithLeft(&two, 240), (QList<int>{240, 760}));
  }

  void focus_rightPanelFoldsAndComesBack() {
    ThreePane pane;
    pane.split.show();
    QVERIFY(QTest::qWaitForWindowExposed(&pane.split));
    pane.split.setSizes({300, 794, 272});
    const int left0 = pane.split.sizes().at(0);
    const int right0 = pane.split.sizes().at(2);
    KaShellFocus focus(&pane.split);
    QSignalSpy spy(&focus, &KaShellFocus::changed);
    QCOMPARE(KaShellFocus::rightPanelKey(), QStringLiteral("F10"));
    focus.toggleRightPanel();
    QVERIFY(focus.rightPanelCollapsed() && !pane.right->isVisibleTo(&pane.split));
    QVERIFY(spy.takeFirst().at(0).toString().contains(QStringLiteral("F10")));
    QVERIFY2(within(pane.split.sizes().at(0), left0, pane.tolerance()), "the map takes the folded width, not the left panel");
    focus.toggleRightPanel();
    QVERIFY(!focus.rightPanelCollapsed() && pane.right->isVisibleTo(&pane.split));
    QVERIFY(within(pane.split.sizes().at(2), right0, pane.tolerance()));
    QVERIFY(within(pane.split.sizes().at(0), left0, pane.tolerance()));
    // Folding both, then unfolding in the other order, loses neither width.
    focus.setLeftPanelCollapsed(true);
    QVERIFY2(within(pane.split.sizes().at(2), right0, pane.tolerance()), "left fold does not widen the inspector");
    focus.setRightPanelCollapsed(true);
    focus.setRightPanelCollapsed(false);
    focus.setLeftPanelCollapsed(false);
    QVERIFY(within(pane.split.sizes().at(0), left0, pane.tolerance()));
    QVERIFY(within(pane.split.sizes().at(2), right0, pane.tolerance()));
    focus.setRightPanelCollapsed(true);
    focus.restoreAll();
    QVERIFY(!focus.rightPanelCollapsed() && pane.right->isVisibleTo(&pane.split));
  }

  void focus_compactSplitCapsLeftForTheMap() {
    // 22 % rules until the cap: 260 under 1500 px (a 1366 window minus the page gutters), 348 above.
    QCOMPARE(KaShellFocus::defaultLeftPanelWidth(1093), 240);
    QCOMPARE(KaShellFocus::defaultLeftPanelWidth(1350), KaShellFocus::kCompactLeftWidth);
    QCOMPARE(KaShellFocus::defaultLeftPanelWidth(1499), KaShellFocus::kCompactLeftWidth);
    QCOMPARE(KaShellFocus::defaultLeftPanelWidth(1500), 330);
    ThreePane pane;
    pane.split.resize(1350, 500);
    pane.right->setMinimumWidth(260);  // KaInspectorPanel::kMinimumWidth
    pane.split.show();
    QVERIFY(QTest::qWaitForWindowExposed(&pane.split));
    pane.split.setSizes({348, 730, KaShellFocus::kDefaultRightWidth});  // the window's first sizes
    KaShellFocus focus(&pane.split);
    focus.applyDefaultWidthOnce();
    const QList<int> sizes = pane.split.sizes();
    QVERIFY(within(sizes.at(0), KaShellFocus::kCompactLeftWidth, pane.tolerance()));
    QVERIFY(within(sizes.at(2), KaShellFocus::kDefaultRightWidth, pane.tolerance()));
    // Map pane >= 768: the 760 px canvas plus the map card's 4 + 4 margins (design §2.0).
    QVERIFY2(sizes.at(1) >= 768, qPrintable(QString::number(sizes.at(1))));
  }

  void rail_topInsetMovesDown() {
    QWidget host;
    host.resize(800, 600);
    host.show();
    QVERIFY(QTest::qWaitForWindowExposed(&host));
    KaLayerOpacityRail rail(&host);
    QCOMPARE(rail.pos(), QPoint(10, 10));
    rail.setTopInset(56);
    QCOMPARE(rail.topInset(), 56);
    QCOMPARE(rail.pos(), QPoint(10, 66));
    host.resize(700, 500);  // the host's resize keeps the inset
    QCOMPARE(rail.pos(), QPoint(10, 66));
    rail.setTarget(QStringLiteral("위성"), true);
    QCOMPARE(rail.pos(), QPoint(10, 66));
    rail.setTopInset(-5);
    QCOMPARE(rail.topInset(), 0);
    QCOMPARE(rail.pos(), QPoint(10, 10));
  }

  // 2026-10-10 현장 요청: 투명도·밝기 상자가 도면·지도를 가린다. 빈 곳을 잡고 끌어 옮기고, 놓은 자리는
  // 지도·도면 따로 다음에도 그대로다. 창이 줄면 상자 전체가 보이는 곳으로 당겨 온다.
  void rail_dragMovesAndRemembersPlace() {
    QSettings().remove(QStringLiteral("ui/opacityRail"));
    QWidget host;
    host.resize(800, 600);
    host.show();
    QVERIFY(QTest::qWaitForWindowExposed(&host));
    {
      KaLayerOpacityRail rail(&host, QStringLiteral("map"));
      QCOMPARE(rail.pos(), QPoint(10, 10));  // nothing saved yet: the old corner
      QTest::mousePress(&rail, Qt::LeftButton, {}, QPoint(4, 4));  // the margin, not a slider
      QMouseEvent move(QEvent::MouseMove, QPointF(304, 204), rail.mapToGlobal(QPointF(304, 204)), Qt::NoButton,
                       Qt::LeftButton, Qt::NoModifier);
      QApplication::sendEvent(&rail, &move);
      QTest::mouseRelease(&rail, Qt::LeftButton, {}, QPoint(4, 4));
      QCOMPARE(rail.pos(), QPoint(310, 210));
    }
    KaLayerOpacityRail again(&host, QStringLiteral("map"));
    QCOMPARE(again.pos(), QPoint(310, 210));
    KaLayerOpacityRail drawing(&host, QStringLiteral("drawing"));
    QCOMPARE(drawing.pos(), QPoint(10, 10));
    host.resize(400, 300);
    QVERIFY2(QRect(QPoint(), host.size()).contains(again.geometry()), "창이 줄어도 상자는 보이는 곳에 있다");
  }

 private:
  QTemporaryDir m_settings;
};

QTEST_MAIN(TestShellFocus)
#include "test_shell_focus.moc"
