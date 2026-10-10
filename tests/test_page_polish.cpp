// 세부 디자인 보완 (docs/intent/2026-10-11-design-detail-polish.md) 판 틀: 지도 화면의 판 · 분할선 · 홈 바깥 틀(M1 · H1).
// 새 모양(paperLook)에서 본다. test_map_polish.cpp가 줄 수 한도(300)에 닿아 따로 둔다.
#include <QtTest>

#include <QFrame>
#include <QHBoxLayout>
#include <QSplitter>
#include <QVBoxLayout>

#include <qgsapplication.h>

#include "app/KaMapPagePlate.h"
#include "app/KaTheme.h"

namespace {

void paper(bool on) {
  KaTheme::DisplayOptions options;
  options.paperLook = on;
  KaTheme::setDisplayOptions(qApp, options);  // the app sheet is applied again for the look
}

// The map page as MainWindow::buildUi builds it: centralRoot { mainSplit { left | mapCard | inspector } }.
struct Page {
  QWidget central;
  QSplitter* split = new QSplitter(Qt::Horizontal, &central);
  QFrame* left = new QFrame(split);
  QFrame* mapCard = new QFrame(split);
  QFrame* right = new QFrame(split);
  Page() {
    central.setObjectName(QStringLiteral("centralRoot"));
    central.setAttribute(Qt::WA_StyledBackground, true);
    auto* root = new QHBoxLayout(&central);
    root->setContentsMargins(8, 8, 8, 8);
    root->setSpacing(8);
    split->setObjectName(QStringLiteral("mainSplit"));
    split->setHandleWidth(10);
    split->setChildrenCollapsible(false);
    mapCard->setObjectName(QStringLiteral("mapCard"));
    auto* mapLay = new QVBoxLayout(mapCard);
    mapLay->setContentsMargins(4, 4, 4, 4);
    mapLay->setSpacing(4);
    mapLay->addWidget(new QWidget(mapCard), 1);
    split->addWidget(left);
    split->addWidget(mapCard);
    split->addWidget(right);
    split->setSizes({200, 400, 200});
    root->addWidget(split, 1);
  }
};

}  // namespace

class TestPagePolish : public QObject {
  Q_OBJECT
private slots:
  void init() { paper(true); }
  void cleanup() { paper(false); }

  // 지도 화면 (보완 M1 · H1): the three panels and the map are one sheet: no frame round the page, no gap between the
  // map and the panels, and between them a 7 px splitter with one centred hairline. The stock look keeps its frame.
  void panelsAreOneSheetWithHairlineSplitters() {
    Page page;
    KaMapPagePlate::follow(&page.central, page.split, page.mapCard);
    page.central.resize(800, 300);
    page.central.show();
    QVERIFY(QTest::qWaitForWindowExposed(&page.central));
    QCOMPARE(page.central.layout()->contentsMargins(), QMargins(0, 0, 0, 0));
    QCOMPARE(page.mapCard->layout()->contentsMargins(), QMargins(0, 0, 0, 0));
    QCOMPARE(page.split->handleWidth(), 7);
    const QImage image = page.central.grab().toImage();
    const int y = image.height() / 2;
    const QColor ground = image.pixelColor(2, y);
    QCOMPARE(image.pixelColor(0, y), ground);  // no frame at the page edge
    const QRect handle = page.split->handle(1)->geometry();
    const QPoint inCentral = page.split->mapTo(&page.central, handle.center());
    QVERIFY2(image.pixelColor(inCentral.x(), y) != image.pixelColor(inCentral.x() - 3, y), "one hairline in the middle");
    QCOMPARE(image.pixelColor(inCentral.x() - 3, y), image.pixelColor(inCentral.x() + 3, y));  // and air on both sides
    // the map runs up to the handle: nothing of the page shows between the map and the splitter
    QCOMPARE(page.mapCard->geometry().left(), page.split->handle(1)->geometry().right() + 1);
    paper(false);
    QTRY_COMPARE(page.central.layout()->contentsMargins(), QMargins(8, 8, 8, 8));
    QCOMPARE(page.mapCard->layout()->contentsMargins(), QMargins(4, 4, 4, 4));
    QCOMPARE(page.split->handleWidth(), 10);
  }
};

int main(int argc, char** argv) {
  QgsApplication app(argc, argv, true);
  QgsApplication::setPrefixPath(qEnvironmentVariable("QGIS_PREFIX_PATH", "A:/OSGeo4W/apps/qgis-dev"), true);
  QgsApplication::initQgis();
  KaTheme::apply(&app);
  TestPagePolish test;
  const int result = QTest::qExec(&test, argc, argv);
  QgsApplication::exitQgis();
  return result;
}

#include "test_page_polish.moc"
