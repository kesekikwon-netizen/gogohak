// 세부 디자인 보완 (docs/intent/2026-10-11-design-detail-polish.md) 지도 화면 항목: 「조용한 도판」 기준서와
// 어긋난 곳을 하나씩 고친 것. 새 모양(paperLook)에서 본다. 판·상태줄 시험 파일들이 줄 수 한도(300)에 닿아 따로 둔다.
#include <QtTest>

#include <QAction>
#include <QFrame>
#include <QLabel>
#include <QMainWindow>
#include <QSignalSpy>
#include <QToolBar>
#include <QTabBar>
#include <QToolButton>
#include <QWidget>

#include <qgsapplication.h>

#include "app/KaDrawGuideBand.h"
#include "app/KaFeatureCard.h"
#include "app/KaIcons.h"
#include "app/KaInspectorPanel.h"
#include "app/KaStatusBar.h"
#include "app/KaTheme.h"
#include "core/FeaturePresets.h"

namespace {

void paper(bool on) {
  KaTheme::DisplayOptions options;
  options.paperLook = on;
  KaTheme::setDisplayOptions(qApp, options);  // the app sheet is applied again for the look
}

// How many pixels of the label's own rim are inked (a box edge, dashed or not).
int rimPixels(const QImage& image) {
  int n = 0;
  for (int x = 0; x < image.width(); ++x) n += (image.pixelColor(x, 0).alpha() > 0) + (image.pixelColor(x, image.height() - 1).alpha() > 0);
  for (int y = 0; y < image.height(); ++y) n += (image.pixelColor(0, y).alpha() > 0) + (image.pixelColor(image.width() - 1, y).alpha() > 0);
  return n;
}

}  // namespace

class TestMapPolish : public QObject {
  Q_OBJECT
private slots:
  void init() { paper(true); }
  void cleanup() { paper(false); }

  // 오른쪽 판 (보완 M5): the two tabs are words with the underline, no icon; switching the look back brings the icons back.
  void inspectorTabsAreWordsOnly() {
    KaInspectorPanel panel;
    QTabBar* tabs = panel.tabs();
    QCOMPARE(tabs->count(), 2);
    for (int i = 0; i < 2; ++i) QVERIFY2(tabs->tabIcon(i).isNull(), "an icon sits in front of the tab's words");
    paper(false);
    for (int i = 0; i < 2; ++i) QVERIFY2(!tabs->tabIcon(i).isNull(), "the stock look keeps its icons");
    paper(true);
    for (int i = 0; i < 2; ++i) QVERIFY(tabs->tabIcon(i).isNull());
  }

  // 오른쪽 판 (보완 M5): the empty-state sentence is words only, no dashed box around it.
  void inspectorSentenceHasNoBox() {
    KaInspectorPanel panel;
    panel.setFeatureCard(new KaFeatureCard);
    panel.resize(300, 560);
    panel.show();
    QVERIFY(QTest::qWaitForWindowExposed(&panel));
    auto* sentence = panel.attributeHost()->findChild<QLabel*>(QStringLiteral("inspectorSentence"));  // the 스타일 tab has its own
    QVERIFY(sentence && sentence->isVisible());
    QVERIFY(!sentence->text().isEmpty());
    const int rim = rimPixels(sentence->grab().toImage());
    QVERIFY2(rim == 0, qPrintable(QStringLiteral("%1 inked pixels on the sentence's rim").arg(rim)));
  }

  // 지도 위 되돌리기 · 다시 실행 (보완 M6): one 73 x 36 cover with the two glyphs and a rule between them, not two
  // separate circles; the stock look keeps its two round buttons 8 px apart.
  void undoRedoShareOneCover() {
    QWidget host;
    host.resize(900, 400);
    KaDrawGuideBand band(&host);
    QAction undo(QStringLiteral("되돌리기")), redo(QStringLiteral("다시 실행"));
    band.setActions(&undo, &redo);
    band.setTool(QString(), QStringLiteral("그리기 · 조사구역"), QStringLiteral("클릭으로 점을 찍습니다"));
    host.show();
    QVERIFY(QTest::qWaitForWindowExposed(&host));
    QToolButton* first = band.undoButton();
    QToolButton* second = band.redoButton();
    QWidget* cover = first->parentWidget();
    QVERIFY2(cover && cover != &band && cover == second->parentWidget(), "the two buttons sit in one cover");
    QCOMPARE(cover->size(), QSize(73, 36));
    auto* rule = cover->findChild<QFrame*>(QStringLiteral("drawGuideDivider"));
    QVERIFY(rule && rule->isVisible());
    QCOMPARE(rule->width(), 1);
    QVERIFY2(rule->x() >= first->geometry().right() && rule->geometry().right() <= second->x(), "the rule stands between the glyphs");
    QVERIFY2(cover->grab().toImage().pixelColor(cover->width() / 2, 0).alpha() > 0, "the cover has its own edge");
    paper(false);  // the layout follows on the next turn of the event loop
    QTRY_COMPARE(first->size(), QSize(36, 36));
    QTRY_COMPARE(second->x() - first->geometry().right() - 1, 8);
    QVERIFY(!rule->isVisible());
  }

  // 상태줄 (보완 M7): 「→ 제출 5179」 is quiet words (no edge, no fill, the pointing hand, underlined under the mouse) that
  // still open 검수·제출 on a click, and the bar is 32 px high.
  void submitCrsIsQuietWordsAndTheBarIs32() {
    QMainWindow window;
    auto* bar = new KaStatusBar(&window);
    window.setStatusBar(bar);
    window.resize(1200, 300);
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    auto* chip = bar->findChild<QLabel*>(QStringLiteral("uploadCrsChip"));
    QVERIFY(chip && chip->isVisible());
    QCOMPARE(chip->text(), QStringLiteral("→ 제출 5179"));
    const int rim = rimPixels(chip->grab().toImage());
    QVERIFY2(rim == 0, qPrintable(QStringLiteral("%1 inked pixels on the chip's rim").arg(rim)));
    QCOMPARE(chip->cursor().shape(), Qt::PointingHandCursor);
    QVERIFY(!chip->font().underline());
    QTest::mouseMove(chip, QPoint(chip->width() / 2, chip->height() / 2));
    QTRY_VERIFY2(chip->font().underline(), "the words are underlined under the mouse");
    QTest::mouseMove(&window, QPoint(5, 5));
    QTRY_VERIFY(!chip->font().underline());
    QSignalSpy clicked(bar, &KaStatusBar::submitClicked);
    QTest::mouseClick(chip, Qt::LeftButton);
    QCOMPARE(clicked.count(), 1);
    QTRY_COMPARE(bar->height(), 32);
  }

  // 그리기 줄 (보완 M3): the tool in hand is a slate fill with ivory words, the same "in hand" look as the ribbon's 그리기 tile;
  // the other tools stay plain white.
  void chosenDrawToolIsInkFilled() {
    QMainWindow window;
    window.setAttribute(Qt::WA_DontShowOnScreen);
    auto* sub = new QToolBar(&window);
    sub->setObjectName(QStringLiteral("subToolbar"));
    sub->setIconSize(QSize(20, 20));
    sub->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    sub->setMovable(false);
    QAction* chosen = nullptr;
    for (const char* id : {"select", "draw_area", "draw_poly"}) {
      QAction* action = sub->addAction(KaIcons::icon(QString::fromLatin1(id)), QString::fromLatin1(id));
      action->setCheckable(true);
      if (QLatin1String(id) == QLatin1String("draw_area")) chosen = action;
    }
    chosen->setChecked(true);
    window.addToolBar(sub);
    window.setCentralWidget(new QWidget(&window));
    window.resize(900, 200);
    window.show();
    QCoreApplication::processEvents();
    int chosenButtons = 0;
    for (QToolButton* button : sub->findChildren<QToolButton*>()) {
      if (!button->defaultAction()) continue;
      const QColor face = button->grab().toImage().pixelColor(3, button->height() / 2);
      if (button->defaultAction() == chosen) {
        ++chosenButtons;
        QVERIFY2(face.lightness() < 70, qPrintable(QStringLiteral("chosen face %1").arg(face.name())));
      } else {
        QVERIFY2(face.lightness() > 200, qPrintable(QStringLiteral("plain face %1").arg(face.name())));
      }
    }
    QCOMPARE(chosenButtons, 1);
  }
};

int main(int argc, char** argv) {
  QgsApplication app(argc, argv, true);
  QgsApplication::setPrefixPath(qEnvironmentVariable("QGIS_PREFIX_PATH", "A:/OSGeo4W/apps/qgis-dev"), true);
  QgsApplication::initQgis();
  FeaturePresets::instance().ensureLoaded();
  KaTheme::apply(&app);
  TestMapPolish test;
  const int result = QTest::qExec(&test, argc, argv);
  QgsApplication::exitQgis();
  return result;
}

#include "test_map_polish.moc"
