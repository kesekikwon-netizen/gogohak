// 세부 디자인 보완 (docs/intent/2026-10-11-design-detail-polish.md) 튼튼함(R1): 낮은 창에서도 오른쪽 판이 잘리지 않는다.
#include <QtTest>

#include <QLabel>
#include <QScrollArea>
#include <QScrollBar>
#include <QTabBar>
#include <QVBoxLayout>

#include <memory>

#include <qgsapplication.h>
#include <qgsfeature.h>
#include <qgsgeometry.h>
#include <qgsrectangle.h>
#include <qgsvectordataprovider.h>
#include <qgsvectorlayer.h>

#include "app/KaFeatureCard.h"
#include "app/KaInspectorPanel.h"
#include "app/KaInspectorStyle.h"
#include "app/KaTheme.h"
#include "core/FeaturePresets.h"
#include "core/LayerOps.h"

namespace {

std::unique_ptr<QgsVectorLayer> houseLayer() {
  auto layer = std::make_unique<QgsVectorLayer>(
      QStringLiteral("Polygon?crs=EPSG:5187&field=kind:string&field=period:string&field=feature_no:string"),
      QStringLiteral("유구"), QStringLiteral("memory"));
  LayerOps::markSurveyLayer(layer.get(), QStringLiteral("feature_poly"));
  QgsFeature f(layer->fields());
  f.setGeometry(QgsGeometry::fromRect(QgsRectangle(200000, 450000, 200010, 450005)));
  layer->dataProvider()->addFeature(f);
  return layer;
}

}  // namespace

class TestInspectorPolish : public QObject {
  Q_OBJECT
private slots:
  // 오른쪽 판 (보완 R1): on a low window the 스타일 tab scrolls inside the panel (it used to be cut off below) and the
  // 「배경 지도」 foot stays pinned under it; a tall window needs no scrolling.
  void styleTabScrollsAndTheFootStaysPinned() {
    auto layer = houseLayer();
    for (const int height : {360, 420, 460, 900}) {
      KaInspectorPanel panel;
      panel.setFeatureCard(new KaFeatureCard);
      panel.styleEditor()->setLayer(layer.get());
      auto* foot = new QLabel(QStringLiteral("배경 지도"));
      panel.footerLayout()->addWidget(foot);
      panel.resize(300, height);
      panel.show();
      QVERIFY(QTest::qWaitForWindowExposed(&panel));
      panel.tabs()->setCurrentIndex(1);
      QTest::qWait(30);
      auto* scroll = panel.findChild<QScrollArea*>(QStringLiteral("inspectorStyleScroll"));
      QVERIFY2(scroll, qPrintable(QStringLiteral("height %1: the 스타일 tab has no scroll area").arg(height)));
      QVERIFY(scroll->isVisible() && scroll->widget() == panel.styleEditor());
      const int needed = panel.styleEditor()->minimumSizeHint().height();
      QVERIFY2(scroll->widget()->height() >= needed, qPrintable(QStringLiteral("height %1: the editor is squeezed to %2 of %3").arg(height).arg(scroll->widget()->height()).arg(needed)));
      const bool cutOff = scroll->widget()->height() > scroll->viewport()->height();
      QCOMPARE(scroll->verticalScrollBar()->maximum() > 0, cutOff);  // what does not fit is reached by scrolling
      if (height == 900) QVERIFY2(!cutOff, "a tall window shows the whole editor");
      const QRect footBox(foot->mapTo(&panel, QPoint()), foot->size());
      QVERIFY2(panel.rect().contains(footBox), qPrintable(QStringLiteral("height %1: the foot leaves the panel").arg(height)));
      QVERIFY2(scroll->mapTo(&panel, QPoint()).y() + scroll->height() <= footBox.top(), "the scroll area ends above the foot");
    }
  }
};

int main(int argc, char** argv) {
  QgsApplication app(argc, argv, true);
  QgsApplication::setPrefixPath(qEnvironmentVariable("QGIS_PREFIX_PATH", "A:/OSGeo4W/apps/qgis-dev"), true);
  QgsApplication::initQgis();
  FeaturePresets::instance().ensureLoaded();
  KaTheme::apply(&app);
  TestInspectorPolish test;
  const int result = QTest::qExec(&test, argc, argv);
  QgsApplication::exitQgis();
  return result;
}

#include "test_inspector_polish.moc"
