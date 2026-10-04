// 「도형 속성 — 유구 (면)」 (조사카드) in 「조용한 도판」 (2차 보드 판 5, M 560): the record fields come in leader
// sections 식별 / 층위 / 표고 / 조사 / 사진 · 비고, two to a row, 표고 boxes in mono with 「m」, and a section
// picture beside them (상면 · 바닥 · 깊이). The same editors stay in the window, so saving reads them as before.
#include <QtTest>

#include <QApplication>
#include <QDir>
#include <QFontDatabase>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QScreen>
#include <QScrollArea>

#include <memory>

#include "app/KaElevationFigure.h"
#include "app/KaRecordPlate.h"
#include "app/KaTheme.h"

namespace {
void paper(bool on) {
  KaTheme::DisplayOptions options;
  options.paperLook = on;
  KaTheme::setDisplayOptions(qApp, options);  // the app sheet, as the window gets it
}

struct Field {
  const char* name;
  const char* word;
  const char* value;
};

// The window as MainWindow::editFeatureAttributes builds it for a 유구 면 (SurveySchema feature_poly order).
std::unique_ptr<QDialog> recordWindow(QFormLayout** formOut = nullptr, bool elevations = true) {
  auto dlg = std::make_unique<QDialog>();
  auto* form = new QFormLayout(dlg.get());
  auto* tip = new QLabel(QStringLiteral("그린 도형의 속성입니다. 종류·시대 등을 입력한 뒤 저장하세요."), dlg.get());
  tip->setWordWrap(true);
  form->addRow(tip);
  const Field fields[] = {{"kind", "유구종류", "주거지"},      {"period", "시대", "청동기"},
                          {"feature_no", "유구번호", "1호"},   {"note", "비고", "바닥에서 숯이 나옴"},
                          {"layer_no", "층위·문맥 번호", "Ⅲ-2"}, {"depth_m", "깊이(m)", ""},
                          {"top_el", "상면 표고(m)", "152.84"}, {"bottom_el", "바닥 표고(m)", "152.42"},
                          {"relation", "선후관계", "1호 수혈보다 먼저"}, {"status", "조사 상태", "조사 중"},
                          {"photo", "사진", "전경.jpg"},       {"surveyor", "조사자", "권영인"},
                          {"surv_date", "조사일", "2026-10-03"}, {"section_id", "단면번호", ""}};
  for (const Field& field : fields) {
    if (!elevations && QByteArray(field.name).endsWith("_el")) continue;  // a 유구 선 has no 표고
    auto* edit = new QLineEdit(QString::fromUtf8(field.value), dlg.get());
    edit->setObjectName(QStringLiteral("edit_") + QLatin1String(field.name));
    edit->setProperty("kaRecordField", QLatin1String(field.name));
    form->addRow(QString::fromUtf8(field.word), edit);
  }
  auto* buttons = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, dlg.get());
  buttons->button(QDialogButtonBox::Save)->setText(QStringLiteral("저장"));
  form->addRow(buttons);
  if (formOut) *formOut = form;
  return dlg;
}

QLineEdit* edit(QDialog* dlg, const char* field) {
  return dlg->findChild<QLineEdit*>(QStringLiteral("edit_") + QLatin1String(field));
}

QStringList sectionWords(QDialog* dlg) {
  QList<QLabel*> heads;
  for (QLabel* label : dlg->findChildren<QLabel*>(QStringLiteral("recordSection"))) heads << label;
  std::sort(heads.begin(), heads.end(), [](QLabel* a, QLabel* b) { return a->y() < b->y(); });
  QStringList words;
  for (QLabel* label : heads) words << label->text();
  return words;
}
}  // namespace

class TestRecordPlate : public QObject {
  Q_OBJECT
private slots:
  void cleanup() { paper(false); }

  void fieldsComeInTheBoardSectionsTwoToARow() {
    paper(true);
    QFormLayout* form = nullptr;
    auto dlg = recordWindow(&form);
    KaRecordPlate::arrange(dlg.get(), form);
    dlg->show();
    QVERIFY(QTest::qWaitForWindowExposed(dlg.get()));
    QCOMPARE(dlg->width(), 560);
    QCOMPARE(dlg->property("kaPlate").toBool(), true);
    QCOMPARE(sectionWords(dlg.get()),
             QStringList({QStringLiteral("식별"), QStringLiteral("층위"), QStringLiteral("표고"), QStringLiteral("조사"),
                          QStringLiteral("사진 · 비고")}));
    QLineEdit* kind = edit(dlg.get(), "kind");
    QLineEdit* period = edit(dlg.get(), "period");
    QCOMPARE(kind->mapTo(dlg.get(), QPoint()).y(), period->mapTo(dlg.get(), QPoint()).y());  // side by side
    QVERIFY(period->mapTo(dlg.get(), QPoint()).x() > kind->mapTo(dlg.get(), QPoint()).x() + 200);
    QVERIFY(edit(dlg.get(), "relation")->width() > 360);  // 선후관계 runs the full width
    QVERIFY(edit(dlg.get(), "note")->width() > 360);
    // A field the board does not name stays, under 식별.
    QLineEdit* other = edit(dlg.get(), "section_id");
    QVERIFY(other->isVisible());
    QVERIFY(other->mapTo(dlg.get(), QPoint()).y() < edit(dlg.get(), "layer_no")->mapTo(dlg.get(), QPoint()).y());
    // Every word is shown once, beside its box; none is left loose in the window.
    int words = 0;
    for (QLabel* label : dlg->findChildren<QLabel*>())
      if (label->text() == QStringLiteral("유구종류")) {
        ++words;
        QVERIFY(label->isVisible());
      }
    QCOMPARE(words, 1);
    const QString out = qEnvironmentVariable("KA_HGIS_QA_OUTPUT_DIR");  // a picture for a look
    if (!out.isEmpty()) QVERIFY(dlg->grab().save(QDir(out).filePath(QStringLiteral("record-card-paper.png"))));
    auto* buttons = dlg->findChild<QDialogButtonBox*>();
    QCOMPARE(buttons->button(QDialogButtonBox::Cancel)->text(), QStringLiteral("취소"));
    QVERIFY(buttons->mapTo(dlg.get(), QPoint()).y() > edit(dlg.get(), "note")->mapTo(dlg.get(), QPoint()).y());
  }

  void elevationsAreMonoNumbersWithAMetreMark() {
    paper(true);
    QFormLayout* form = nullptr;
    auto dlg = recordWindow(&form);
    KaRecordPlate::arrange(dlg.get(), form);
    for (const char* field : {"top_el", "bottom_el", "depth_m"}) {
      QLineEdit* box = edit(dlg.get(), field);
      QVERIFY2(box->alignment() & Qt::AlignRight, field);
      QCOMPARE(box->font().styleHint(), QFont::Monospace);
      auto* unit = box->findChild<QLabel*>(QStringLiteral("recordUnit"));
      QVERIFY2(unit && unit->text() == QStringLiteral("m"), field);
    }
  }

  // The picture reads the boxes as they are typed; 깊이 left empty is 상면 − 바닥.
  void sectionPictureFollowsTheElevationBoxes() {
    paper(true);
    QFormLayout* form = nullptr;
    auto dlg = recordWindow(&form);
    KaRecordPlate::arrange(dlg.get(), form);
    auto* figure = dlg->findChild<KaElevationFigure*>();
    QVERIFY(figure);
    QCOMPARE(figure->size(), QSize(244, 76));
    QCOMPARE(figure->accessibleName(), QStringLiteral("상면 표고 152.84, 바닥 표고 152.42, 깊이 0.42 미터"));
    dlg->show();
    QVERIFY(QTest::qWaitForWindowExposed(dlg.get()));
    QLineEdit* top = edit(dlg.get(), "top_el");
    QVERIFY(figure->mapTo(dlg.get(), QPoint()).x() > top->mapTo(dlg.get(), QPoint()).x() + 140);  // beside the boxes
    QCOMPARE(top->width(), edit(dlg.get(), "depth_m")->width());  // 상면 · 바닥 · 깊이 boxes line up
    top->setText(QStringLiteral("153.00"));
    QCOMPARE(figure->accessibleName(), QStringLiteral("상면 표고 153.00, 바닥 표고 152.42, 깊이 0.58 미터"));
    edit(dlg.get(), "depth_m")->setText(QStringLiteral("0.6"));
    QCOMPARE(figure->accessibleName(), QStringLiteral("상면 표고 153.00, 바닥 표고 152.42, 깊이 0.60 미터"));
  }

  void sectionPictureSaysWhatIsNotKnownYet() {
    KaElevationFigure figure;
    figure.setLevels(QString(), QStringLiteral("152.42"), QString());
    QCOMPARE(figure.accessibleName(), QStringLiteral("상면 표고 —, 바닥 표고 152.42, 깊이 — 미터"));
    QCOMPARE(KaElevationFigure::level(QStringLiteral("152.4")), QStringLiteral("EL. 152.40"));
    QCOMPARE(KaElevationFigure::level(QStringLiteral("숫자 아님")), QStringLiteral("EL. —"));
    figure.resize(figure.sizeHint());
    const QImage image = figure.grab().toImage();
    int ink = 0;
    for (int x = 0; x < image.width(); ++x)
      if (image.pixelColor(x, 58).lightness() < 100) ++ink;  // the pit floor line
    QVERIFY2(ink >= 60, qPrintable(QString::number(ink)));
  }

  // Tab walks the boxes as they now stand on screen, then 저장 and 취소.
  void tabFollowsTheScreenOrder() {
    paper(true);
    QFormLayout* form = nullptr;
    auto dlg = recordWindow(&form);
    KaRecordPlate::arrange(dlg.get(), form);
    QStringList walked;
    QWidget* at = edit(dlg.get(), "kind");
    for (int step = 0; step < 200 && walked.size() < 16; ++step, at = at->nextInFocusChain()) {
      if (at->objectName().startsWith(QStringLiteral("edit_"))) walked << at->objectName().mid(5);
      if (auto* button = qobject_cast<QPushButton*>(at)) walked << button->text();
      if (walked.size() > 1 && walked.first() == walked.last()) break;
    }
    QCOMPARE(walked.mid(0, 16),
             QStringList({"kind", "period", "feature_no", "section_id", "layer_no", "relation", "top_el", "bottom_el",
                          "depth_m", "status", "surv_date", "surveyor", "photo", "note", "저장", "취소"}));
  }

  // On a short screen the sections scroll and 저장 · 취소 stay in the window.
  void saveStaysOnAShortScreen() {
    paper(true);
    QFormLayout* form = nullptr;
    auto dlg = recordWindow(&form);
    KaRecordPlate::arrange(dlg.get(), form);
    dlg->show();
    QVERIFY(QTest::qWaitForWindowExposed(dlg.get()));
    QVERIFY(dlg->findChild<QScrollArea*>());
    QVERIFY2(dlg->height() <= dlg->screen()->availableGeometry().height(), qPrintable(QString::number(dlg->height())));
    auto* buttons = dlg->findChild<QDialogButtonBox*>();
    QVERIFY(buttons->mapTo(dlg.get(), QPoint(0, buttons->height())).y() <= dlg->height());
  }

  // A long word from the survey's own file is shortened with the whole of it on hover; 표고 Z joins 표고.
  void longWordsAndPointHeights() {
    paper(true);
    QFormLayout* form = nullptr;
    auto dlg = recordWindow(&form, false);
    auto* z = new QLineEdit(QStringLiteral("12.5"), dlg.get());
    z->setObjectName(QStringLiteral("edit_z"));
    z->setProperty("kaRecordField", QStringLiteral("z"));
    form->insertRow(1, QStringLiteral("excavation_record_date"), z);
    KaRecordPlate::arrange(dlg.get(), form);
    QVERIFY(!dlg->findChild<KaElevationFigure*>());  // no 상면 · 바닥: no picture
    QCOMPARE(sectionWords(dlg.get()).value(2), QStringLiteral("표고"));
    QVERIFY(z->findChild<QLabel*>(QStringLiteral("recordUnit")));
    dlg->show();
    QVERIFY(QTest::qWaitForWindowExposed(dlg.get()));
    QVERIFY(z->mapTo(dlg.get(), QPoint()).y() > edit(dlg.get(), "relation")->mapTo(dlg.get(), QPoint()).y());
    for (QLabel* label : dlg->findChildren<QLabel*>())
      if (label->toolTip() == QStringLiteral("excavation_record_date"))
        QVERIFY(label->text() != label->toolTip());  // shortened to its 96 px
  }

  void stockKeepsTheFormAsItWas() {
    QFormLayout* form = nullptr;
    auto dlg = recordWindow(&form);
    KaRecordPlate::arrange(dlg.get(), form);
    QCOMPARE(dlg->layout(), static_cast<QLayout*>(form));
    QVERIFY(!dlg->findChild<KaElevationFigure*>());
    QVERIFY(dlg->findChildren<QLabel*>(QStringLiteral("recordSection")).isEmpty());
  }
};

int main(int argc, char** argv) {
  QApplication app(argc, argv);
#ifdef Q_OS_WIN
  if (QGuiApplication::platformName() == QLatin1String("offscreen")) {  // Korean for the QA pictures
    const QDir windows(qEnvironmentVariable("WINDIR", QStringLiteral("C:/Windows")));
    for (const QString& file : {QStringLiteral("malgun.ttf"), QStringLiteral("malgunbd.ttf")})
      QFontDatabase::addApplicationFont(windows.filePath(QStringLiteral("Fonts/") + file));
  }
#endif
  KaTheme::apply(&app);
  TestRecordPlate test;
  return QTest::qExec(&test, argc, argv);
}
#include "test_record_plate.moc"
