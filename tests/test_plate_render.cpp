// 「조용한 도판」 pieces drawn with the app's own style sheet in 새 모양 (code review, 2차 보드 판 4): the tab ✕ is drawn,
// 「PDF 내보내기」 keeps its filled clay face in the panel foot, and a chosen file row keeps its name readable. These
// are checked on pixels, because the app-wide rules (padding, selectors) decide what is actually drawn.
#include <QtTest>

#include <QApplication>
#include <QFrame>
#include <QHBoxLayout>
#include <QListWidget>
#include <QScrollArea>
#include <QTabBar>
#include <QTabWidget>
#include <QToolButton>
#include <QVBoxLayout>

#include <algorithm>
#include <cstdlib>
#include <memory>

#include "app/KaDrawingStudioPlate.h"
#include "app/KaFileRowDelegate.h"
#include "app/KaTabClose.h"
#include "app/KaTheme.h"

namespace {
void paper(bool on) {
  KaTheme::DisplayOptions options;
  options.paperLook = on;
  KaTheme::setDisplayOptions(qApp, options);  // the app sheet is applied again for the look
}

// The inspector card with its two output buttons, turned into the paper column (scroll + foot).
QWidget* footColumn(QToolButton** pdf, QToolButton** print) {
  auto* scroll = new QScrollArea;
  auto* side = new QWidget;
  auto* sideLay = new QVBoxLayout(side);
  auto* card = new QFrame(side);
  card->setObjectName(QStringLiteral("itemInspector"));
  auto* row = new QHBoxLayout(card);
  *pdf = new QToolButton(card);
  (*pdf)->setObjectName(QStringLiteral("btnPrimary"));
  (*pdf)->setText(QStringLiteral("PDF 내보내기"));
  *print = new QToolButton(card);
  (*print)->setObjectName(QStringLiteral("btnPrint"));
  (*print)->setText(QStringLiteral("인쇄"));
  for (QToolButton* button : {*pdf, *print}) {  // as the studio builds them
    button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    row->addWidget(button, 1);
  }
  sideLay->addWidget(card);
  scroll->setWidget(side);
  return KaDrawingStudioPlate::inspectorColumn(scroll);
}

int darkPixels(const QImage& image, const QRect& area, int below = 100) {
  int n = 0;
  for (int y = area.top(); y <= area.bottom(); ++y)
    for (int x = area.left(); x <= area.right(); ++x)
      if (image.rect().contains(x, y) && image.pixelColor(x, y).lightness() < below) ++n;
  return n;
}
}  // namespace

class TestPlateRender : public QObject {
  Q_OBJECT
private slots:
  void cleanup() { paper(false); }

  void tabCrossIsDrawn() {
    paper(true);
    QTabWidget tabs;
    tabs.setObjectName(QStringLiteral("viewTabs"));
    tabs.setDocumentMode(true);
    tabs.setTabsClosable(true);
    tabs.addTab(new QWidget, QStringLiteral("홈"));
    tabs.tabBar()->setTabButton(0, QTabBar::RightSide, nullptr);
    tabs.addTab(new QWidget, QStringLiteral("도면"));
    KaTabClose::follow(&tabs);
    tabs.resize(400, 120);
    tabs.show();
    QVERIFY(QTest::qWaitForWindowExposed(&tabs));
    QWidget* cross = tabs.tabBar()->tabButton(1, QTabBar::RightSide);
    QVERIFY(cross && cross->objectName() == QStringLiteral("viewTabClose"));
    const QImage image = cross->grab().toImage();
    QVERIFY2(darkPixels(image, image.rect(), 140) >= 6, qPrintable(QString::number(darkPixels(image, image.rect(), 140))));
  }

  // A 200 % screen gets a cross drawn for it, in the muted ink of the look in use (here the stronger one).
  void tabCrossFollowsTheInkAndTheScreen() {
    KaTheme::DisplayOptions options;
    options.paperLook = true;
    options.highContrast = true;
    KaTheme::setDisplayOptions(qApp, options);
    QTabWidget tabs;
    tabs.setTabsClosable(true);
    tabs.addTab(new QWidget, QStringLiteral("도면"));
    KaTabClose::follow(&tabs);
    auto* cross = qobject_cast<QToolButton*>(tabs.tabBar()->tabButton(0, QTabBar::RightSide));
    QVERIFY(cross);
    const QImage sharp = cross->icon().pixmap(QSize(10, 10), 2.0).toImage();
    QVERIFY2(sharp.width() >= 20, qPrintable(QString::number(sharp.width())));
    QColor strongest(Qt::transparent);
    for (int y = 0; y < sharp.height(); ++y)
      for (int x = 0; x < sharp.width(); ++x)
        if (sharp.pixelColor(x, y).alpha() > strongest.alpha()) strongest = sharp.pixelColor(x, y);
    const QColor muted = qApp->palette().color(QPalette::PlaceholderText);
    QVERIFY2(std::abs(strongest.red() - muted.red()) <= 4 && std::abs(strongest.blue() - muted.blue()) <= 4,
             qPrintable(strongest.name() + QLatin1Char(' ') + muted.name()));
  }

  // 아래 줄 (보드 판 4): 「PDF 내보내기」 takes the room, 「인쇄」 keeps its own width (at least 88).
  void printKeepsItsOwnWidthInTheFoot() {
    paper(true);
    QToolButton* pdf = nullptr;
    QToolButton* print = nullptr;
    std::unique_ptr<QWidget> column(footColumn(&pdf, &print));
    column->resize(420, 400);
    column->show();
    QVERIFY(QTest::qWaitForWindowExposed(column.get()));
    QCOMPARE(print->width(), std::max(88, print->sizeHint().width()));
    QVERIFY(pdf->width() > print->width());
  }

  void pdfKeepsItsFilledFaceInThePanelFoot() {
    paper(true);
    QToolButton* pdf = nullptr;
    QToolButton* print = nullptr;
    std::unique_ptr<QWidget> column(footColumn(&pdf, &print));
    column->resize(320, 400);
    column->show();
    QVERIFY(QTest::qWaitForWindowExposed(column.get()));
    const QColor face = pdf->grab().toImage().pixelColor(6, pdf->height() / 2);
    QVERIFY2(face.red() > face.blue() + 60, qPrintable(face.name()));  // clay, not white
  }

  void chosenFileRowNameStaysReadable() {
    paper(true);
    QListWidget list;
    list.setObjectName(QStringLiteral("fileBrowser"));
    list.setItemDelegate(new KaFileRowDelegate(&list));
    auto* item = new QListWidgetItem(QStringLiteral("MMMMMMMMMMMMMMMM.dxf"), &list);
    item->setData(Qt::UserRole + 1, false);
    list.resize(300, 80);
    list.show();
    QVERIFY(QTest::qWaitForWindowExposed(&list));
    item->setSelected(true);
    const QRect row = list.visualItemRect(item);
    const QImage image = list.viewport()->grab().toImage();
    QVERIFY2(darkPixels(image, QRect(row.left() + 24, row.top(), 120, row.height())) > 20, "the chosen name reads");
  }
};

int main(int argc, char** argv) {
  QApplication app(argc, argv);
  KaTheme::apply(&app);
  TestPlateRender test;
  return QTest::qExec(&test, argc, argv);
}

#include "test_plate_render.moc"
