// 세부 디자인 보완 (docs/intent/2026-10-11-design-detail-polish.md) 도면 화면 항목: 오른쪽 판 · 왼쪽 판 머리줄 · 빈 상태.
// 새 모양(paperLook)에서 본다. test_map_polish.cpp가 줄 수 한도(300)에 닿아 따로 둔다.
#include <QtTest>

#include <QFrame>
#include <QHBoxLayout>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QScrollArea>
#include <QToolButton>
#include <QWidget>
#include <QVBoxLayout>

#include <memory>

#include <qgsapplication.h>

#include "app/KaDrawingStudioPlate.h"
#include "app/KaPaperDialog.h"
#include "app/KaTheme.h"

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

class TestDrawingPolish : public QObject {
  Q_OBJECT
private slots:
  void init() { paper(true); }
  void cleanup() { paper(false); }

  // 도면 오른쪽 판 (보완 L1): the three cards are no boxes; each caption runs on into a hairline to the edge.
  void studioCardsAreCaptionsWithALeader() {
    auto* scroll = new QScrollArea;
    auto* side = new QWidget;
    auto* sideLay = new QVBoxLayout(side);
    QToolButton *pdf = nullptr, *print = nullptr;
    for (const char* name : {"도면 항목", "방위 (도북)", "도면 정보"}) {
      auto* card = new QFrame(side);
      card->setObjectName(QStringLiteral("itemInspector"));
      auto* lay = new QVBoxLayout(card);
      auto* caption = new QLabel(QString::fromUtf8(name), card);
      caption->setObjectName(QStringLiteral("cardCaption"));
      lay->addWidget(caption);
      lay->addWidget(new QToolButton(card));
      if (!pdf) {  // the output buttons sit in the first card, as the studio builds them
        pdf = new QToolButton(card);
        pdf->setObjectName(QStringLiteral("btnPrimary"));
        print = new QToolButton(card);
        print->setObjectName(QStringLiteral("btnPrint"));
        lay->addWidget(pdf);
        lay->addWidget(print);
      }
      sideLay->addWidget(card);
    }
    scroll->setWidget(side);
    std::unique_ptr<QWidget> column(KaDrawingStudioPlate::inspectorColumn(scroll));
    column->resize(320, 520);
    column->show();
    QVERIFY(QTest::qWaitForWindowExposed(column.get()));
    const auto cards = side->findChildren<QFrame*>(QStringLiteral("itemInspector"));
    QCOMPARE(cards.size(), 3);
    for (QFrame* card : cards) {
      QVERIFY(card->property("kaPlate").toBool());
      auto* caption = card->findChild<QLabel*>(QStringLiteral("cardCaption"));
      auto* rule = card->findChild<QFrame*>(QStringLiteral("studioCaptionRule"));
      QVERIFY(caption && rule && rule->isVisible());
      QCOMPARE(rule->height(), 1);
      QVERIFY2(rule->x() > caption->geometry().right() && rule->y() >= caption->y() && rule->y() <= caption->geometry().bottom(),
               "the hairline continues the caption's row");
      QVERIFY2(rule->geometry().right() >= card->width() - 4, "and runs to the card's edge");
      const QImage face = card->grab().toImage();
      QVERIFY2(face.pixelColor(0, 0).alpha() == 0 && face.pixelColor(face.width() - 1, face.height() - 1).alpha() == 0 &&
                   face.pixelColor(face.width() - 1, face.height() / 2).alpha() == 0,
               "the card draws no box");
    }
  }

  // 도면 왼쪽 판 머리줄 (보완 L3): 파일함 · 전체 끄기 · 세트 are the map screen's quiet words (kaPlate under layersCard),
  // the 파일함 that is on in oat; the empty-state sentence has no dashed box.
  void studioLeftHeaderIsQuietWordsAndTheEmptySentenceHasNoBox() {
    QFrame card;
    card.setObjectName(QStringLiteral("layersCard"));
    auto* row = new QHBoxLayout(&card);
    auto* files = new QToolButton(&card);
    files->setText(QStringLiteral("파일함"));
    files->setCheckable(true);
    files->setChecked(true);
    auto* all = new QToolButton(&card);
    all->setText(QStringLiteral("전체 끄기"));
    for (QToolButton* button : {files, all}) row->addWidget(KaDrawingStudioPlate::quiet(button));
    auto* empty = new QLabel(QStringLiteral("이 도면에는 레이어가 없습니다."), &card);
    empty->setObjectName(QStringLiteral("emptyState"));
    row->addWidget(empty);
    card.resize(520, 120);
    card.show();
    QVERIFY(QTest::qWaitForWindowExposed(&card));
    QVERIFY(files->property("kaPlate").toBool() && all->property("kaPlate").toBool());
    const QColor oat = files->grab().toImage().pixelColor(3, files->height() / 2);
    QVERIFY2(qAbs(oat.red() - 0xE3) < 8 && qAbs(oat.blue() - 0xCC) < 8, qPrintable(QStringLiteral("파일함 on %1").arg(oat.name())));
    QVERIFY2(all->grab().toImage().pixelColor(3, all->height() / 2).alpha() == 0, "전체 끄기 is a word, not a box");
    const int rim = rimPixels(empty->grab().toImage());
    QVERIFY2(rim == 0, qPrintable(QStringLiteral("%1 inked pixels on the empty sentence's rim").arg(rim)));
    paper(false);  // the stock look keeps its boxes
    QTRY_VERIFY(!files->property("kaPlate").toBool());
    QVERIFY(rimPixels(empty->grab().toImage()) > 0);
  }

  // 용지 설정 창 (보완 L5): a size that follows from the paper is a reading line (mono ink, dotted leader), entry fields
  // only under 「직접 입력」; the stock look keeps its disabled fields.
  void lockedPaperSizeIsAReadingLine() {
    KaPaperDialog dialog(nullptr, 317.0, 220.0);
    dialog.show();
    QVERIFY(QTest::qWaitForWindowExposed(&dialog));
    auto button = [&dialog](const char* name) { return dialog.findChild<QToolButton*>(QString::fromLatin1(name)); };
    auto spin = [&dialog](const char* name) { return dialog.findChild<QDoubleSpinBox*>(QString::fromLatin1(name)); };
    auto value = [&dialog](const char* name) {
      auto* row = dialog.findChild<QWidget*>(QString::fromLatin1(name));
      return row ? row->findChild<QLabel*>(QStringLiteral("paperSpecValue")) : nullptr;
    };
    QVERIFY(button("paperA4") && button("paperA3") && button("paperCustom") && button("paperLandscape"));
    QVERIFY(spin("paperWidth") && spin("paperHeight") && value("paperSpecWidth") && value("paperSpecHeight"));
    QVERIFY2(!spin("paperWidth")->isVisible() && value("paperSpecWidth")->isVisible(), "A4: the size is read, not typed");
    QCOMPARE(value("paperSpecWidth")->text(), QStringLiteral("210"));
    QCOMPARE(value("paperSpecHeight")->text(), QStringLiteral("297"));
    int ink = 0;  // the reading is in ink, not the disabled grey
    const QImage reading = value("paperSpecWidth")->grab().toImage();
    for (int y = 0; y < reading.height(); ++y)
      for (int x = 0; x < reading.width(); ++x) ink += reading.pixelColor(x, y).lightness() < 100 && reading.pixelColor(x, y).alpha() > 200;
    QVERIFY2(ink >= 8, qPrintable(QString::number(ink)));
    QCOMPARE(dialog.widthMm(), 210.0);
    if (const QString out = qEnvironmentVariable("KA_HGIS_QA_OUTPUT_DIR"); !out.isEmpty()) dialog.grab().save(out + QStringLiteral("/paper-dialog.png"));
    button("paperLandscape")->click();  // A4 가로 is the field sheet
    QCOMPARE(value("paperSpecWidth")->text(), QStringLiteral("317"));
    QCOMPARE(value("paperSpecHeight")->text(), QStringLiteral("220"));
    button("paperA3")->click();
    QCOMPARE(dialog.widthMm(), 420.0);
    QCOMPARE(dialog.heightMm(), 297.0);
    button("paperCustom")->click();
    QVERIFY2(spin("paperWidth")->isVisible() && spin("paperWidth")->isEnabled() && !value("paperSpecWidth")->isVisible(),
             "직접 입력: the fields are typed into");
    spin("paperWidth")->setValue(123.4);
    QCOMPARE(dialog.widthMm(), 123.4);
  }

  void stockPaperDialogKeepsItsDisabledFields() {
    paper(false);
    KaPaperDialog dialog(nullptr, 317.0, 220.0);
    dialog.show();
    QVERIFY(QTest::qWaitForWindowExposed(&dialog));
    auto* width = dialog.findChild<QDoubleSpinBox*>(QStringLiteral("paperWidth"));
    QVERIFY(width && width->isVisible() && !width->isEnabled());
    QCOMPARE(dialog.widthMm(), 210.0);
    QCOMPARE(dialog.heightMm(), 297.0);
  }
};

int main(int argc, char** argv) {
  QgsApplication app(argc, argv, true);
  QgsApplication::setPrefixPath(qEnvironmentVariable("QGIS_PREFIX_PATH", "A:/OSGeo4W/apps/qgis-dev"), true);
  QgsApplication::initQgis();
  KaTheme::apply(&app);
  TestDrawingPolish test;
  const int result = QTest::qExec(&test, argc, argv);
  QgsApplication::exitQgis();
  return result;
}

#include "test_drawing_polish.moc"
