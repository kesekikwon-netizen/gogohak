// 「조용한 도판」 windows (2차 보드 판 5): message boxes start with a serif question or outcome and one plain
// sentence, without an icon disc (설계 2.4 Dialog). Stock keeps its own words. No QGIS.
#include <QtTest>

#include <QApplication>
#include <QDir>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QStandardPaths>

#include "app/KaClosePrompt.h"
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
  KaTheme::apply(&app);
  TestWindowPlate test;
  return QTest::qExec(&test, argc, argv);
}

#include "test_window_plate.moc"
