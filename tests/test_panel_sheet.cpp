// 「조용한 도판」 map screen panels (2차 보드 판 1) as the application sheet draws them: the panels on the page,
// the layer head words, the file buttons' edge, the one-line list head, and kaPlate following a live switch of
// 「새 모양」 (화면 보기). The stock look keeps its boxes.
#include <QtTest>
#include <QApplication>
#include <QFrame>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QPushButton>
#include <QRegularExpression>
#include <QToolButton>
#include <QTreeWidget>
#include "app/KaTheme.h"

namespace {

class TestPanelSheet : public QObject {
  Q_OBJECT
private slots:
  void initTestCase() { KaTheme::apply(qApp); }
  void cleanup() { KaTheme::setDisplayOptions(qApp, KaTheme::DisplayOptions()); }

  // 지도 화면 판 (2차 보드 판 1): the layer and record panels sit on the page — no box, square corners;
  // stock keeps the white 8 px panels.
  void sidePanelsHaveNoBoxInPaper() {
    const QString panel = QStringLiteral("(?m)^QFrame#inspectorPanel \\{ background: ([^;]+); border: 1px solid [^;]+; border-radius: (\\d+)px;");
    const auto both = [&panel]() {
      const auto m = QRegularExpression(panel).match(qApp->styleSheet());
      return QStringList{m.captured(1).toLower(), m.captured(2)};
    };
    QCOMPARE(both(), (QStringList{KaTheme::tokens().surface.name(), QStringLiteral("8")}));
    KaTheme::DisplayOptions paper;
    paper.paperLook = true;
    KaTheme::setDisplayOptions(qApp, paper);
    QCOMPARE(both(), (QStringList{QStringLiteral("transparent"), QStringLiteral("0")}));
  }

  // 레이어 머리 (2차 보드 판 1): 「파일함」 and 「전체 끄기」 are words, not boxed buttons; the open 파일함 is oat
  // with a warm grey edge.
  void layerHeadButtonsAreWordsInPaper() {
    KaTheme::DisplayOptions paper;
    paper.paperLook = true;
    KaTheme::setDisplayOptions(qApp, paper);
    QWidget host;  // the side panel's ground under the see-through card
    host.setAutoFillBackground(true);
    auto* hostRow = new QHBoxLayout(&host);
    auto* card = new QFrame(&host);
    hostRow->addWidget(card);
    card->setObjectName(QStringLiteral("layersCard"));
    auto* row = new QHBoxLayout(card);
    auto* files = new QToolButton(card);
    files->setObjectName(QStringLiteral("sidebarFilesToggle"));
    files->setText(QStringLiteral("파일함"));
    files->setCheckable(true);
    files->setChecked(true);
    files->setFocusPolicy(Qt::NoFocus);  // the resting look; the keyboard ring has its own test
    auto* all = new QToolButton(card);
    all->setObjectName(QStringLiteral("layerCheckAllBtn"));
    all->setText(QStringLiteral("전체 끄기"));
    all->setFocusPolicy(Qt::NoFocus);  // as the window's layerCheckAllBtn
    for (QToolButton* button : {files, all}) {
      button->setProperty("kaPlate", true);
      row->addWidget(button);
    }
    host.show();
    QVERIFY(QTest::qWaitForWindowExposed(&host));
    const QImage image = host.grab().toImage();
    const auto at = [&](QWidget* w, const QPoint& p) {
      return image.pixelColor(w->mapTo(&host, p) * image.devicePixelRatio()).name();
    };
    const KaTheme::Tokens t = KaTheme::tokens();
    QCOMPARE(at(files, QPoint(4, files->height() / 2)), t.tileShown.name());
    QCOMPARE(at(files, QPoint(files->width() / 2, 0)), t.tileShownBorder.name());
    const QString ground = at(all, QPoint(all->width() / 2, -2));
    QCOMPARE(at(all, QPoint(all->width() / 2, 0)), ground);                  // no edge
    QCOMPARE(at(all, QPoint(all->width() / 2, all->height() - 1)), ground);  // no bottom ledge
    QCOMPARE(at(all, QPoint(4, all->height() / 2)), ground);                 // no face
  }

  // 파일함 경로 단추 (내 PC · 바탕화면 …): the boards draw them as edged white buttons in 새 모양 too.
  void fileButtonsKeepTheirEdgeInPaper() {
    KaTheme::DisplayOptions paper;
    paper.paperLook = true;
    KaTheme::setDisplayOptions(qApp, paper);
    QWidget host;
    host.setAutoFillBackground(true);
    auto* hostRow = new QHBoxLayout(&host);
    auto* card = new QFrame(&host);
    card->setObjectName(QStringLiteral("filesCard"));
    hostRow->addWidget(card);
    auto* row = new QHBoxLayout(card);
    auto* mine = new QToolButton(card);
    mine->setText(QStringLiteral("내 PC"));
    row->addWidget(mine);
    host.show();
    QVERIFY(QTest::qWaitForWindowExposed(&host));
    const QImage image = host.grab().toImage();
    const auto at = [&](const QPoint& p) { return image.pixelColor(mine->mapTo(&host, p) * image.devicePixelRatio()).name(); };
    QVERIFY(at(QPoint(mine->width() / 2, 0)) != at(QPoint(mine->width() / 2, -2)));  // an edge, not the ground
    QCOMPARE(at(QPoint(4, mine->height() / 2)), KaTheme::tokens().surface.name());
  }

  // 「새 모양」 can be switched while the app runs: a widget that follows the look gets kaPlate on and off.
  void plateWordsFollowALiveSwitch() {
    QToolButton words;
    KaTheme::followPaperLook(&words);
    words.show();
    QVERIFY(QTest::qWaitForWindowExposed(&words));
    QVERIFY(!words.property("kaPlate").toBool());
    KaTheme::DisplayOptions paper;
    paper.paperLook = true;
    KaTheme::setDisplayOptions(qApp, paper);
    QCoreApplication::processEvents();
    QVERIFY(words.property("kaPlate").toBool());
    KaTheme::setDisplayOptions(qApp, KaTheme::DisplayOptions());
    QCoreApplication::processEvents();
    QVERIFY(!words.property("kaPlate").toBool());
  }

  // The open 「파일함」 still shows the focus ring when the keyboard is on it.
  void chosenLayerHeadWordShowsFocus() {
    KaTheme::DisplayOptions paper;
    paper.paperLook = true;
    KaTheme::setDisplayOptions(qApp, paper);
    QFrame card;
    card.setObjectName(QStringLiteral("layersCard"));
    auto* row = new QHBoxLayout(&card);
    auto* files = new QToolButton(&card);
    files->setText(QStringLiteral("파일함"));
    files->setCheckable(true);
    files->setChecked(true);
    files->setFocusPolicy(Qt::StrongFocus);
    files->setProperty("kaPlate", true);
    row->addWidget(files);
    card.show();
    card.activateWindow();
    QVERIFY(QTest::qWaitForWindowActive(&card));
    files->setFocus(Qt::TabFocusReason);
    QTRY_VERIFY(files->hasFocus());
    const QImage image = card.grab().toImage();
    const QPoint top = files->mapTo(&card, QPoint(files->width() / 2, 0)) * image.devicePixelRatio();
    QCOMPARE(image.pixelColor(top).name(), KaTheme::tokens().focusRing.name());
  }

  // 레이어 목록 머리 (2차 보드 판 1): 「도면 · 레이어 | 글자」 is one line under the heads, not a box per cell.
  void layerListHeadIsOneLineInPaper() {
    KaTheme::DisplayOptions paper;
    paper.paperLook = true;
    KaTheme::setDisplayOptions(qApp, paper);
    QWidget host;
    host.setAutoFillBackground(true);
    auto* hostRow = new QHBoxLayout(&host);
    auto* tree = new QTreeWidget(&host);
    tree->setProperty("kaLayerInformation", true);
    tree->setHeaderLabels({QStringLiteral("도면 · 레이어"), QStringLiteral("글자")});
    hostRow->addWidget(tree);
    host.resize(300, 160);
    host.show();
    QVERIFY(QTest::qWaitForWindowExposed(&host));
    QHeaderView* head = tree->header();
    const QImage image = host.grab().toImage();
    const auto at = [&](const QPoint& p) { return image.pixelColor(head->mapTo(&host, p) * image.devicePixelRatio()).name(); };
    const QString ground = at(QPoint(head->sectionSize(0) - 6, head->height() / 2));  // right of the words
    QCOMPARE(at(QPoint(head->sectionSize(0) / 2, 0)), ground);  // no top edge
    QCOMPARE(at(QPoint(0, head->height() / 2)), ground);        // no side edge
    QCOMPARE(at(QPoint(head->sectionSize(0) / 2, head->height() - 1)), KaTheme::tokens().border.name());  // the one line
  }

  // 검수·제출 (2차 보드 판 3): in a result row 「위치 보기」 is a word (no box) beside the 24 px fix button.
  void submitRowGoToIsAWord() {
    KaTheme::DisplayOptions paper;
    paper.paperLook = true;
    KaTheme::setDisplayOptions(qApp, paper);
    QTreeWidget tree;
    tree.setObjectName(QStringLiteral("submitReviewTree"));
    tree.setProperty("kaPlate", true);
    tree.setColumnCount(2);
    auto* row = new QTreeWidgetItem(&tree, {QStringLiteral("종류·시대가 빈 유구가 있습니다.")});
    auto* go = new QPushButton(QStringLiteral("위치 보기"));
    go->setFlat(true);
    tree.setItemWidget(row, 1, go);
    tree.resize(400, 120);
    tree.show();
    QVERIFY(QTest::qWaitForWindowExposed(&tree));
    const QImage image = tree.viewport()->grab().toImage();
    const QPoint left = go->mapTo(tree.viewport(), QPoint(0, go->height() / 2)) * image.devicePixelRatio();
    const QPoint beside = go->mapTo(tree.viewport(), QPoint(-3, go->height() / 2)) * image.devicePixelRatio();
    QCOMPARE(image.pixelColor(left).name(), image.pixelColor(beside).name());  // no edge on the word button
    QVERIFY2(go->height() <= 24, qPrintable(QString::number(go->height())));
  }
};

}  // namespace

QTEST_MAIN(TestPanelSheet)
#include "test_panel_sheet.moc"
