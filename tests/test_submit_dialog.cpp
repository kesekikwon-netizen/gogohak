// 검수·제출 dialog: the package button follows the hard block (errors or a
// missing rule file), and 「위치 보기」 / tool buttons report the chosen row.
#include "app/KaSubmitDialog.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QPushButton>
#include <QTemporaryDir>
#include <QTreeWidget>
#include <QtTest>

namespace {
CheckResult result(const QString& id, const QString& severity, bool passed, const QString& action = {}) {
  CheckResult r;
  r.id = id;
  r.severity = severity;
  r.passed = passed;
  r.messageKo = id;
  r.fixKo = QStringLiteral("고치세요");
  r.action = action;
  return r;
}
}  // namespace

class TestSubmitDialog : public QObject {
  Q_OBJECT
private slots:
  // 「조용한 도판」: five paper tiles for the required drawings, present or 「없음」, from the drawing rules.
  void requiredDrawingsStripFollowsTheDrawingRules() {
    const QVector<CheckResult> results = {result(QStringLiteral("REQ_DRAWING_SURVEY_AREA"), QStringLiteral("error"), true),
                                          result(QStringLiteral("REQ_DRAWING_SITE_LOCATION"), QStringLiteral("error"), true),
                                          result(QStringLiteral("REQ_DRAWING_FEATURE_PLAN"), QStringLiteral("error"), true),
                                          result(QStringLiteral("REQ_DRAWING_FEATURE_DETAIL"), QStringLiteral("warn"), false),
                                          result(QStringLiteral("REQ_DRAWING_SECTION"), QStringLiteral("warn"), false)};
    {
      KaSubmitDialog stock;
      stock.setResults(results, true);
      QVERIFY(!stock.findChild<QWidget*>(QStringLiteral("submitDrawingStrip")));
    }
    qApp->setProperty("kaPaperLook", true);
    KaSubmitDialog dialog;
    qApp->setProperty("kaPaperLook", QVariant());
    dialog.setResults(results, true);
    auto* strip = dialog.findChild<QWidget*>(QStringLiteral("submitDrawingStrip"));
    QVERIFY(strip);
    QCOMPARE(strip->property("missing").toStringList(), (QStringList{QStringLiteral("개별유구실측도"), QStringLiteral("층위도")}));
    QCOMPARE(strip->property("present").toStringList(),
             (QStringList{QStringLiteral("조사구역도"), QStringLiteral("유적위치도"), QStringLiteral("유구배치도")}));
    dialog.setResults({}, false);  // no rule file: no drawing state to show
    QVERIFY(strip->isHidden());
  }
  // The rule file's own title and version name the check under the headline (2차 보드 검수·제출).
  void rulesTitleNamesTheCheck() {
    QTemporaryDir dir;
    QFile file(dir.filePath(QStringLiteral("rules.json")));
    QVERIFY(file.open(QIODevice::WriteOnly));
    const QJsonObject rule{{QStringLiteral("id"), QStringLiteral("A")}, {QStringLiteral("severity"), QStringLiteral("error")},
                           {QStringLiteral("message_ko"), QStringLiteral("a")},
                           {QStringLiteral("check_type"), QStringLiteral("layer_nonempty:survey_area")}};
    const QJsonObject root{{QStringLiteral("version"), 1}, {QStringLiteral("title_ko"), QStringLiteral("발굴 위치도면 유의사항 기반 검수")},
                           {QStringLiteral("rules"), QJsonArray{rule}}};
    file.write(QJsonDocument(root).toJson());
    file.close();
    ChecklistEngine engine;
    QVERIFY(engine.loadRules(file.fileName()));
    QCOMPARE(engine.title(), QStringLiteral("발굴 위치도면 유의사항 기반 검수 v1"));
  }

  // 「조용한 도판」 검수·제출 (2차 보드 판 3): the serif headline says what blocks, the counts sit top right and the
  // rule title under the headline; a blocking row is red and 「위치 보기」 is a word button. Stock keeps its intro.
  void paperHeadSaysWhatBlocks() {
    {
      KaSubmitDialog stock;
      QVERIFY(!stock.findChild<QLabel*>(QStringLiteral("submitHeadline")));
    }
    qApp->setProperty("kaPaperLook", true);
    KaSubmitDialog dialog;
    qApp->setProperty("kaPaperLook", QVariant());
    dialog.setRulesTitle(QStringLiteral("발굴 위치도면 유의사항 기반 검수 v1"));
    CheckResult blocking = result(QStringLiteral("SPEC"), QStringLiteral("error"), false, QStringLiteral("edit_attributes"));
    CheckTarget target;
    target.layerId = QStringLiteral("f");
    target.layerName = QStringLiteral("유구 (면)");
    target.label = QStringLiteral("3호 수혈");
    blocking.targets = {target};
    dialog.setResults({blocking, result(QStringLiteral("W1"), QStringLiteral("warn"), false),
                       result(QStringLiteral("W2"), QStringLiteral("warn"), false), result(QStringLiteral("P"), QStringLiteral("error"), true)},
                      true);
    const auto text = [&dialog](const char* name) {
      auto* label = dialog.findChild<QLabel*>(QString::fromLatin1(name));
      return label ? label->text() : QStringLiteral("(없음)");
    };
    QCOMPARE(text("submitHeadline"), QStringLiteral("오류 1건을 고쳐야 제출할 수 있습니다"));
    QVERIFY(text("submitSubline").startsWith(QStringLiteral("발굴 위치도면 유의사항 기반 검수 v1")));
    QCOMPARE(text("submitCountError"), QStringLiteral("1"));  // mono figures; 「건」 is its own faint label
    QCOMPARE(text("submitCountWarn"), QStringLiteral("2"));
    QCOMPARE(text("submitCountPass"), QStringLiteral("1"));
    QVERIFY(!dialog.findChild<QLabel*>(QStringLiteral("submitCountError"))->property("zero").toBool());
    QCOMPARE(text("submitPackageLine"), QStringLiteral("EPSG:5179 SHP · 조사도면.pdf · MANIFEST.sha256"));
    QCOMPARE(dialog.size(), QSize(820, 580));  // the built size stays (spec: 검수·제출 820 × 580)
    QVERIFY(dialog.findChild<QTreeWidget*>(QStringLiteral("submitReviewTree"))->property("kaPlate").toBool());
    QVERIFY(dialog.findChild<QLabel*>(QStringLiteral("submitIntro"))->isHidden());
    auto* tree = dialog.findChild<QTreeWidget*>(QStringLiteral("submitReviewTree"));
    QCOMPARE(tree->topLevelItem(0)->child(0)->background(0).color(), QColor(0xF3, 0xDE, 0xDA));
    QVERIFY(dialog.findChild<QPushButton*>(QStringLiteral("btnGoTo_SPEC"))->isFlat());
    dialog.show();
    QVERIFY(QTest::qWaitForWindowExposed(&dialog));
    auto* fix = dialog.findChild<QPushButton*>(QStringLiteral("btnAction_SPEC"));
    QVERIFY2(fix->width() <= fix->sizeHint().width() + 4, qPrintable(QString::number(fix->width())));  // compact, at the right
    QVERIFY(text("submitSubline").endsWith(QStringLiteral("「위치 보기」나 옆 단추로 고칠 곳을 여세요.")));
    dialog.setResults({result(QStringLiteral("W1"), QStringLiteral("warn"), false)}, true);
    QCOMPARE(text("submitHeadline"), QStringLiteral("오류가 없습니다"));
    QVERIFY(text("submitSubline").endsWith(QStringLiteral("주의 항목은 확인한 뒤 진행할 수 있습니다.")));
    QVERIFY(dialog.findChild<QLabel*>(QStringLiteral("submitCountError"))->property("zero").toBool());
    dialog.setResults({result(QStringLiteral("P"), QStringLiteral("error"), true)}, true);
    QCOMPARE(text("submitHeadline"), QStringLiteral("오류와 주의 항목이 없습니다"));
    QCOMPARE(text("submitSubline"), QStringLiteral("발굴 위치도면 유의사항 기반 검수 v1"));  // nothing left to do
    dialog.setResults({}, false);
    QCOMPARE(text("submitHeadline"), QStringLiteral("검수 규칙 파일을 찾지 못했습니다"));
    QVERIFY(!text("submitSubline").contains(QStringLiteral("진행")));  // never 「go ahead」 while blocked
  }

  void errorsBlockThePackage() {
    KaSubmitDialog dialog;
    auto* package = dialog.findChild<QPushButton*>(QStringLiteral("btnSubmitPackage"));
    QVERIFY(package);
    dialog.setResults({result(QStringLiteral("A"), QStringLiteral("error"), false),
                       result(QStringLiteral("B"), QStringLiteral("warn"), false)},
                      true);
    QVERIFY(!package->isEnabled());
    QCOMPARE(dialog.errorCount(), 1);
    QVERIFY(dialog.findChild<QLabel*>(QStringLiteral("submitBlockReason"))->text().contains(QStringLiteral("1건")));
    package->click();  // disabled: nothing happens
    QCOMPARE(dialog.choice(), KaSubmitDialog::Choice::None);

    dialog.setResults({result(QStringLiteral("A"), QStringLiteral("error"), true),
                       result(QStringLiteral("B"), QStringLiteral("warn"), false)},
                      true);
    QVERIFY2(package->isEnabled(), "warnings alone must not block");
  }

  void missingRulesBlockLikeAnError() {
    KaSubmitDialog dialog;
    auto* package = dialog.findChild<QPushButton*>(QStringLiteral("btnSubmitPackage"));
    dialog.setResults({}, false, QStringLiteral("C:/nowhere/drawing_checklist.v1.json"));
    QVERIFY(!package->isEnabled());
    QVERIFY(!dialog.packageAllowed());
    QVERIFY(dialog.findChild<QLabel*>(QStringLiteral("submitBlockReason"))->text().contains(QStringLiteral("규칙 파일")));
    dialog.setResults({}, true);  // loaded but nothing evaluated is not "clean" either
    QVERIFY(!dialog.packageAllowed());
  }

  void goToAndToolButtonsReportTheRow() {
    KaSubmitDialog dialog;
    CheckResult withTargets = result(QStringLiteral("GCP_DATUM_META"), QStringLiteral("error"), false,
                                     QStringLiteral("edit_attributes"));
    CheckTarget target;
    target.layerId = QStringLiteral("cp");
    target.layerName = QStringLiteral("기준점");
    target.featureId = 3;
    target.label = QStringLiteral("G2");
    withTargets.targets = {target};
    dialog.setResults({withTargets, result(QStringLiteral("REQ_DRAWING_SITE_LOCATION"), QStringLiteral("error"), false,
                                           QStringLiteral("open_layout"))},
                      true);
    auto* go = dialog.findChild<QPushButton*>(QStringLiteral("btnGoTo_GCP_DATUM_META"));
    QVERIFY(go);
    QVERIFY(!dialog.findChild<QPushButton*>(QStringLiteral("btnGoTo_REQ_DRAWING_SITE_LOCATION")));
    go->click();
    QCOMPARE(dialog.choice(), KaSubmitDialog::Choice::GoToTargets);
    QCOMPARE(dialog.chosenResult().targets.size(), 1);
    QCOMPARE(dialog.chosenResult().targets.first().featureId, qint64(3));

    auto* tool = dialog.findChild<QPushButton*>(QStringLiteral("btnAction_REQ_DRAWING_SITE_LOCATION"));
    QVERIFY(tool);
    QCOMPARE(tool->text(), KaSubmitDialog::actionLabel(QStringLiteral("open_layout")));
    tool->click();
    QCOMPARE(dialog.choice(), KaSubmitDialog::Choice::RunAction);
    QCOMPARE(dialog.chosenResult().action, QStringLiteral("open_layout"));
  }

  void convertNeedsASelectedLayer() {
    KaSubmitDialog dialog;
    auto* convert = dialog.findChild<QPushButton*>(QStringLiteral("btnConvertLayer5179"));
    QVERIFY(convert && !convert->isEnabled());
    dialog.setConvertLayerName(QStringLiteral("유구면"));
    QVERIFY(convert->isEnabled());
    QVERIFY(convert->toolTip().contains(QStringLiteral("유구면")));
    convert->click();
    QCOMPARE(dialog.choice(), KaSubmitDialog::Choice::ConvertLayer);
  }

  void everyRuleActionHasALabel() {
    for (const char* action : {"open_layout", "open_section", "draw_survey_area", "draw_feature", "add_control_point",
                               "edit_attributes", "vertex_edit", "select_tool", "set_work_crs"})
      QVERIFY2(!KaSubmitDialog::actionLabel(QString::fromLatin1(action)).isEmpty(), action);
  }
};

QTEST_MAIN(TestSubmitDialog)
#include "test_submit_dialog.moc"
