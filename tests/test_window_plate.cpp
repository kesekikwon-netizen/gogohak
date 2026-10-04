// 「조용한 도판」 windows (2차 보드 판 5): message boxes start with a serif question or outcome and one plain
// sentence, without an icon disc (설계 2.4 Dialog). Stock keeps its own words. No QGIS.
#include <QtTest>

#include <QApplication>
#include <QDialog>
#include <QFormLayout>
#include <QDir>
#include <QFontDatabase>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QPixmap>
#include <QStandardPaths>
#include <QWidgetAction>

#include "app/KaApiKeyDialog.h"
#include "app/KaClosePrompt.h"
#include "app/KaControlPointDialog.h"
#include "app/KaFormPlate.h"
#include "app/KaMenuCaption.h"
#include "app/KaTheme.h"
#include "app/KaUserError.h"

namespace {
void paper(bool on) {
  KaTheme::DisplayOptions options;
  options.paperLook = on;
  KaTheme::setDisplayOptions(nullptr, options);
}
}  // namespace

class TestWindowPlate : public QObject {
  Q_OBJECT
 private slots:
  void cleanup() { paper(false); }

  // 조사 닫기 (보드 판 5): 「광령리1을 저장할까요?」, how much is unsaved and where the file is, no icon.
  void closePromptAsksByNameInPaper() {
    const QString path = QDir(QStandardPaths::writableLocation(QStandardPaths::DesktopLocation))
                             .filePath(QStringLiteral("광령리1/광령리1.gpkg"));
    paper(true);
    QMessageBox box;
    KaClosePrompt::dress(box, path, 3);
    QCOMPARE(box.text(), QStringLiteral("광령리1을 저장할까요?"));
    QVERIFY(box.informativeText().contains(QStringLiteral("저장하지 않은 작업이 3건 있습니다.")));
    QVERIFY(box.informativeText().contains(QStringLiteral("바탕 화면 › 광령리1 › 광령리1.gpkg")));
    QCOMPARE(box.icon(), QMessageBox::NoIcon);
    QVERIFY(box.property("kaPlate").toBool());
    QCOMPARE(static_cast<QAbstractButton*>(box.defaultButton()), box.button(QMessageBox::Save));
    QMessageBox stock;
    paper(false);
    KaClosePrompt::dress(stock, path, 3);
    QVERIFY(stock.text().contains(QStringLiteral("저장하지 않은 작업이 있습니다.")));
    QCOMPARE(stock.icon(), QMessageBox::Question);
  }

  // The question reads as the 20 px serif Heading, the sentence under it 12 px muted (설계 2.4 Dialog).
  void plateQuestionIsTheSerifHeading() {
    KaTheme::DisplayOptions options;
    options.paperLook = true;
    KaTheme::setDisplayOptions(qApp, options);
    QMessageBox box;
    KaClosePrompt::dress(box, QStringLiteral("C:/조사/광령리1.gpkg"), 2);
    box.ensurePolished();
    auto* question = box.findChild<QLabel*>(QStringLiteral("qt_msgbox_label"));
    auto* sentence = box.findChild<QLabel*>(QStringLiteral("qt_msgbox_informativelabel"));
    QVERIFY(question && sentence);
    question->ensurePolished();
    sentence->ensurePolished();
    const int questionPx = question->font().pixelSize(), sentencePx = sentence->font().pixelSize();
    KaTheme::setDisplayOptions(qApp, KaTheme::DisplayOptions());
    QCOMPARE(questionPx, 20);
    QCOMPARE(sentencePx, 12);
  }

  // 「저장 안 함」 throws work away, so it reads in the danger colour; the window is the small 440 width.
  void closePromptDiscardIsDangerAndTheBoxIsSmall() {
    KaTheme::DisplayOptions options;
    options.paperLook = true;
    KaTheme::setDisplayOptions(qApp, options);
    QMessageBox box;
    KaClosePrompt::dress(box, QStringLiteral("C:/조사/광령리1.gpkg"), 2);
    box.show();
    QVERIFY(QTest::qWaitForWindowExposed(&box));
    const QColor discard = box.button(QMessageBox::Discard)->palette().color(QPalette::ButtonText);
    const int width = box.width();
    KaTheme::setDisplayOptions(qApp, KaTheme::DisplayOptions());
    QCOMPARE(discard, QColor(0xA3, 0x3B, 0x3B));
    QVERIFY2(width >= 440, qPrintable(QString::number(width)));
  }

  // 오류 알림 (보드 판 5): what failed as the serif heading, why and how as plain sentences under it, no icon;
  // stock keeps the three bold-headed parts.
  void errorAlertSaysWhatThenWhyAndHowInPaper() {
    KaUserError::Spec spec;
    spec.title = QStringLiteral("지적도 받기");
    spec.what = QStringLiteral("지적도를 받지 못했습니다.");
    spec.why = QStringLiteral("아이디나 비밀번호가 맞지 않습니다.");
    spec.how = QStringLiteral("아이디·비밀번호를 다시 입력한 뒤 받기를 누르세요.");
    spec.actionLabel = QStringLiteral("아이디·비밀번호 다시 입력");
    paper(true);
    QMessageBox box;
    KaUserError::dress(box, QMessageBox::Warning, spec);
    QCOMPARE(box.text(), spec.what);
    QCOMPARE(box.informativeText(), spec.why + QLatin1Char(0x0A) + spec.how);
    QCOMPARE(box.icon(), QMessageBox::NoIcon);
    QVERIFY(box.property("kaPlate").toBool());
    QCOMPARE(box.windowTitle(), spec.title);
    paper(false);
    QMessageBox stock;
    KaUserError::dress(stock, QMessageBox::Warning, spec);
    QVERIFY(stock.text().contains(QStringLiteral("무엇이 안 됐나")));
    QCOMPARE(stock.icon(), QMessageBox::Warning);
  }

  // API 키 입력 (보드 판 5, 440): each name over its box and one plain sentence under it; the keys come back trimmed.
  void apiKeysAreStackedWithPlainSentencesInPaper() {
    paper(true);
    KaApiKeyDialog dialog(QStringLiteral(" vworld-test "), QStringLiteral("history-test"));
    dialog.show();
    QVERIFY(QTest::qWaitForWindowExposed(&dialog));
    QCOMPARE(dialog.minimumWidth(), 440);
    const auto text = [&dialog](const char* name) {
      auto* label = dialog.findChild<QLabel*>(QString::fromLatin1(name));
      return label ? label->text() : QStringLiteral("(없음)");
    };
    QCOMPARE(text("apiKeyVworldHint"), QStringLiteral("위성 · 지적 · 주소 찾기에 함께 씁니다."));
    QCOMPARE(text("apiKeyHistoryHint"), QStringLiteral("1919 조선지형도 전용. VWorld 키로는 인증되지 않습니다."));
    auto* name = dialog.findChild<QLabel*>(QStringLiteral("apiKeyVworldName"));
    auto* box = dialog.findChild<QLineEdit*>(QStringLiteral("apiKeyVworld"));
    QVERIFY(name && box);
    QVERIFY2(name->geometry().bottom() < box->geometry().top() && name->x() == box->x(),
             qPrintable(QStringLiteral("name %1,%2 box %3,%4").arg(name->x()).arg(name->geometry().bottom()).arg(box->x()).arg(box->y())));
    QCOMPARE(dialog.vworldKey(), QStringLiteral("vworld-test"));
    QCOMPARE(dialog.historyKey(), QStringLiteral("history-test"));
    paper(false);
    KaApiKeyDialog stock{QString(), QString()};
    QVERIFY(!stock.findChild<QLabel*>(QStringLiteral("apiKeyVworldHint"))->text().contains(QStringLiteral("주소 찾기")));
  }

  // 더보기 (보드 판 5): small group captions over the items and no item icons in 새 모양; stock keeps its icons and
  // shows no captions. The menu follows a switch of the look each time it opens.
  void menuGroupsHaveCaptionsInPaper() {
    QMenu menu;
    QPixmap dot(8, 8);
    dot.fill(Qt::black);
    QAction* first = menu.addAction(QIcon(dot), QStringLiteral("벡터·도면 불러오기"));
    QAction* caption = KaMenuCaption::insert(&menu, QStringLiteral("불러오기·배경"), first);
    QCOMPARE(menu.actions().first(), caption);
    auto* word = qobject_cast<QLabel*>(static_cast<QWidgetAction*>(caption)->defaultWidget());
    QVERIFY(word);
    QCOMPARE(word->text(), QStringLiteral("불러오기·배경"));
    paper(true);
    KaMenuCaption::follow(&menu);
    QVERIFY(caption->isVisible() && !first->isIconVisibleInMenu());
    paper(false);
    KaMenuCaption::follow(&menu);
    QVERIFY(!caption->isVisible() && first->isIconVisibleInMenu());
    QMenu grouped;  // install(): the captions go in hidden in stock and come back when the menu opens in 새 모양
    QAction* view = grouped.addAction(QStringLiteral("지도 넓게 보기"));
    KaMenuCaption::install(&grouped, {{QStringLiteral("화면"), view}});
    QVERIFY(!grouped.actions().first()->isVisible());
    paper(true);
    emit grouped.aboutToShow();
    QVERIFY(grouped.actions().first()->isVisible());
  }

  // VWorld 지적도 계정 (보드 판 5, 440): a plain form window gets the board's frame — 96 px words, the note under the
  // boxes as a muted sentence, 24 px margins; stock is left as it is.
  void accountFormGetsTheBoardFrameInPaper() {
    const auto build = [](QDialog& dialog) {
      auto* form = new QFormLayout(&dialog);
      form->addRow(QStringLiteral("아이디"), new QLineEdit(&dialog));
      form->addRow(QStringLiteral("비밀번호"), new QLineEdit(&dialog));
      form->addRow(new QLabel(QStringLiteral("이 PC의 개인 설정에 저장하여 다음 다운로드에 사용합니다."), &dialog));
      return form;
    };
    paper(true);
    QDialog dialog;
    QFormLayout* form = build(dialog);
    KaFormPlate::dress(&dialog, form);
    QCOMPARE(dialog.minimumWidth(), 440);
    auto* word = qobject_cast<QLabel*>(form->itemAt(0, QFormLayout::LabelRole)->widget());
    QVERIFY(word && word->minimumWidth() == 96 && word->property("kaFormWord").toBool());
    auto* note = qobject_cast<QLabel*>(form->itemAt(2, QFormLayout::SpanningRole)->widget());
    QVERIFY(note && note->property("kaFormNote").toBool() && note->wordWrap());
    paper(false);
    QDialog stock;
    QFormLayout* plain = build(stock);
    KaFormPlate::dress(&stock, plain);
    QCOMPARE(stock.minimumWidth(), 0);
    QVERIFY(!qobject_cast<QLabel*>(plain->itemAt(0, QFormLayout::LabelRole)->widget())->property("kaFormWord").isValid());
  }

  // GPS 기준점 (보드 판 5, 440): the axis note in three lines, then 「위치」「측량 기준」「수신」 sections, the basis
  // and receiver values side by side under small captions; the values read back as typed.
  void controlPointWindowHasTheBoardSectionsInPaper() {
    const QString hint = QStringLiteral("이 프로그램은 X를 동쪽, Y를 북쪽으로 읽습니다. 한국 측량 성과표는 흔히 X=북쪽, Y=동쪽입니다. "
                                        "그 값이면 X·Y 교환을 고르세요.");
    paper(true);
    KaControlPointDialog dialog(hint, QStringLiteral("정확도(m)"), QStringLiteral("수신상태"));
    dialog.show();
    QVERIFY(QTest::qWaitForWindowExposed(&dialog));
    QCOMPARE(dialog.minimumWidth(), 440);
    QCOMPARE(dialog.findChild<QLabel*>(QStringLiteral("controlPointHint"))->text().count(QLatin1Char(0x0A)), 2);
    QStringList sections;
    for (QLabel* label : dialog.findChildren<QLabel*>(QStringLiteral("controlPointSection"))) sections << label->text();
    QCOMPARE(sections, (QStringList{QStringLiteral("위치"), QStringLiteral("측량 기준"), QStringLiteral("수신")}));
    const auto box = [&dialog](const char* name) { return dialog.findChild<QLineEdit*>(QString::fromLatin1(name)); };
    QCOMPARE(box("controlPointX")->width(), 176);
    for (const char* name : {"controlPointEllipsoid", "controlPointProjection", "controlPointOrigin"})
      QCOMPARE(box(name)->y(), box("controlPointDatum")->y());  // side by side
    QCOMPARE(box("controlPointFix")->y(), box("controlPointAccuracy")->y());
    box("controlPointId")->setText(QStringLiteral(" GCP4 "));
    box("controlPointX")->setText(QStringLiteral("147958.65"));
    QCOMPARE(dialog.pointId(), QStringLiteral(" GCP4 "));
    QCOMPARE(dialog.xText(), QStringLiteral("147958.65"));
    QCOMPARE(dialog.datum(), QStringLiteral("세계측지계"));
    QCOMPARE(dialog.fix(), QStringLiteral("RTK"));
    const QString out = qEnvironmentVariable("KA_HGIS_QA_OUTPUT_DIR");  // a picture for a look
    if (!out.isEmpty()) QVERIFY(dialog.grab().save(QDir(out).filePath(QStringLiteral("control-point-paper.png"))));
    paper(false);
    KaControlPointDialog stock(hint, QStringLiteral("정확도(m)"), QStringLiteral("수신상태"));
    QVERIFY(stock.findChildren<QLabel*>(QStringLiteral("controlPointSection")).isEmpty());
    QVERIFY(!stock.findChild<QLabel*>(QStringLiteral("controlPointHint"))->text().contains(QLatin1Char(0x0A)));
  }

  // 「광령리1을」 · 「테스트를」 · 「광령리2를」: the object particle follows how the name ends.
  void objectParticleFollowsTheName() {
    QCOMPARE(KaClosePrompt::withObjectParticle(QStringLiteral("광령리1")), QStringLiteral("광령리1을"));
    QCOMPARE(KaClosePrompt::withObjectParticle(QStringLiteral("광령리2")), QStringLiteral("광령리2를"));
    QCOMPARE(KaClosePrompt::withObjectParticle(QStringLiteral("테스트")), QStringLiteral("테스트를"));
    QCOMPARE(KaClosePrompt::withObjectParticle(QStringLiteral("병산동 시굴")), QStringLiteral("병산동 시굴을"));
    QCOMPARE(KaClosePrompt::withObjectParticle(QStringLiteral("site-A")), QStringLiteral("site-A을(를)"));
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
  TestWindowPlate test;
  return QTest::qExec(&test, argc, argv);
}

#include "test_window_plate.moc"
