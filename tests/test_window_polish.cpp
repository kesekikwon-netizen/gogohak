// 세부 디자인 보완 (docs/intent/2026-10-11-design-detail-polish.md) 창 항목(D1): 창의 단추와 입력칸 높이.
// 새 모양(paperLook)에서 본다. 높이는 테두리·안쪽 여백까지 합친 위젯 전체 높이다.
#include <QtTest>

#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QTreeWidget>
#include <QVBoxLayout>

#include <qgsapplication.h>

#include "app/KaSubmitDialog.h"
#include "app/KaTheme.h"

namespace {

void paper(bool on) {
  KaTheme::DisplayOptions options;
  options.paperLook = on;
  KaTheme::setDisplayOptions(qApp, options);  // the app sheet is applied again for the look
}

}  // namespace

class TestWindowPolish : public QObject {
  Q_OBJECT
private slots:
  void init() { paper(true); }
  void cleanup() { paper(false); }

  // 창의 단추 · 입력칸 (보완 D1): a window's buttons and fields are 32 px high (the board's default control), not about 50 and 40.
  // The home's own buttons (40) keep their own rule.
  void windowControlsAre32High() {
    QDialog window;
    auto* column = new QVBoxLayout(&window);
    auto* line = new QLineEdit(&window);
    auto* combo = new QComboBox(&window);
    combo->addItems({QStringLiteral("5186 중부원점"), QStringLiteral("5187 동부원점")});
    auto* spin = new QSpinBox(&window);
    auto* real = new QDoubleSpinBox(&window);
    auto* push = new QPushButton(QStringLiteral("취소"), &window);
    auto* box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &window);
    auto* home = new QPushButton(QStringLiteral("이어서 열기"), &window);
    home->setObjectName(QStringLiteral("startContinueBtn"));
    for (QWidget* widget : {static_cast<QWidget*>(line), static_cast<QWidget*>(combo), static_cast<QWidget*>(spin),
                            static_cast<QWidget*>(real), static_cast<QWidget*>(push), static_cast<QWidget*>(box),
                            static_cast<QWidget*>(home)})
      column->addWidget(widget);
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    const auto heights = [&] {
      return QStringLiteral("line %1 combo %2 spin %3 real %4 push %5 ok %6 home %7")
          .arg(line->height()).arg(combo->height()).arg(spin->height()).arg(real->height()).arg(push->height())
          .arg(box->button(QDialogButtonBox::Ok)->height()).arg(home->height());
    };
    for (QWidget* widget : {static_cast<QWidget*>(line), static_cast<QWidget*>(combo), static_cast<QWidget*>(spin),
                            static_cast<QWidget*>(real), static_cast<QWidget*>(push), static_cast<QWidget*>(box->button(QDialogButtonBox::Ok)),
                            static_cast<QWidget*>(box->button(QDialogButtonBox::Cancel))})
      QVERIFY2(widget->height() == 32, qPrintable(heights()));
    QVERIFY2(home->height() >= 40, qPrintable(heights()));  // the home's button is not changed by D1
    paper(false);  // the stock look keeps its taller controls
    QTRY_VERIFY2(push->height() > 40 && line->height() > 36, qPrintable(heights()));
  }

  // 검수·제출 (보완 D2): the item list is a three-rule table: an ink rule above and below, no box and no side edge.
  void reviewListIsAThreeRuleTable() {
    KaSubmitDialog dialog;
    CheckResult blocking;
    blocking.id = QStringLiteral("SPEC");
    blocking.severity = QStringLiteral("error");
    blocking.passed = false;
    blocking.messageKo = QStringLiteral("종류·시대가 빈 유구가 있습니다.");
    blocking.fixKo = QStringLiteral("종류와 시대를 넣으세요.");
    dialog.setResults({blocking}, true);
    dialog.show();
    QVERIFY(QTest::qWaitForWindowExposed(&dialog));
    auto* tree = dialog.findChild<QTreeWidget*>(QStringLiteral("submitReviewTree"));
    QVERIFY(tree);
    const QImage face = tree->grab().toImage();
    QVERIFY2(face.pixelColor(face.width() / 2, 0).lightness() < 90, "an ink rule above the table");
    QVERIFY2(face.pixelColor(face.width() / 2, face.height() - 1).lightness() < 90, "and one below");
    const QColor side = face.pixelColor(0, face.height() / 2);
    QVERIFY2(side.alpha() == 0 || side.lightness() > 200, qPrintable(QStringLiteral("a box edge at the side: %1").arg(side.name())));
  }
};

int main(int argc, char** argv) {
  QgsApplication app(argc, argv, true);
  QgsApplication::setPrefixPath(qEnvironmentVariable("QGIS_PREFIX_PATH", "A:/OSGeo4W/apps/qgis-dev"), true);
  QgsApplication::initQgis();
  KaTheme::apply(&app);
  TestWindowPolish test;
  const int result = QTest::qExec(&test, argc, argv);
  QgsApplication::exitQgis();
  return result;
}

#include "test_window_polish.moc"
