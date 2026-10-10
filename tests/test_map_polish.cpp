// 세부 디자인 보완 (docs/intent/2026-10-11-design-detail-polish.md) 지도 화면 항목: 「조용한 도판」 기준서와
// 어긋난 곳을 하나씩 고친 것. 새 모양(paperLook)에서 본다. 판·상태줄 시험 파일들이 줄 수 한도(300)에 닿아 따로 둔다.
#include <QtTest>

#include <QAction>
#include <QFrame>
#include <QLabel>
#include <QTabBar>
#include <QToolButton>
#include <QWidget>

#include <qgsapplication.h>

#include "app/KaDrawGuideBand.h"
#include "app/KaFeatureCard.h"
#include "app/KaInspectorPanel.h"
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
