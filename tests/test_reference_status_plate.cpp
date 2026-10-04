// 「조용한 도판」 자료 준비 상태 (2차 보드 판 5, 860 × 420): the summary as the serif title over the two-line note, then
// a ruled table with the board's column widths and the sizes set right. Stock keeps the note first and its own
// column sizing. tests/test_reference_status.cpp is near its 300-line limit.
#include <QtTest>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QTreeWidget>
#include <qgsapplication.h>
#include <qgsproject.h>
#include <qgsvectorlayer.h>
#include "app/KaReferenceStatusDialog.h"
#include "core/LayerOps.h"

class ReferenceStatusPlateTest : public QObject {
  Q_OBJECT
private slots:
  void cleanup() {
    qApp->setProperty("kaPaperLook", QVariant());
    QgsProject::instance()->removeAllMapLayers();
  }

  void paperTableReadsLikeTheBoard() {
    auto* project = QgsProject::instance();
    auto* layer = new QgsVectorLayer(QStringLiteral("LineString?crs=EPSG:5187"), QStringLiteral("첫 참조"),
                                     QStringLiteral("memory"));
    LayerOps::markReferenceLayer(layer);
    project->addMapLayer(layer);
    qApp->setProperty("kaPaperLook", true);
    KaReferenceStatusDialog dialog(project);
    dialog.show();
    QVERIFY(QTest::qWaitForWindowExposed(&dialog));
    auto* summary = dialog.findChild<QLabel*>(QStringLiteral("referenceStatusSummary"));
    auto* hint = dialog.findChild<QLabel*>(QStringLiteral("referenceStatusHint"));
    QVERIFY(summary && hint);
    QVERIFY2(summary->y() < hint->y(), "the summary is the title over the note");
    QCOMPARE(hint->text(),
             QStringLiteral("조사에 올린 참조 자료가 현장에서 인터넷 없이 열리는지 확인하는 표입니다. 이 창은 자료를 새로 받거나 "
                            "지우지 않습니다.") +
                 QLatin1Char(0x0A) +
                 QStringLiteral("인터넷이 필요한 배경은 레이어를 오른쪽 클릭해 「오프라인 저장」으로 직접 받아 두세요."));
    auto* table = dialog.findChild<QTreeWidget*>(QStringLiteral("referenceStatusTable"));
    const QList<int> widths{100, 136, 132, 72, 212};
    for (int column = 1; column <= 5; ++column) QCOMPARE(table->header()->sectionSize(column), widths.at(column - 1));
    QVERIFY(table->topLevelItem(0)->textAlignment(4) & Qt::AlignRight);
    // 「다시 확인」·「닫기」 are both plain words: nothing here is the one main action.
    for (QPushButton* button : dialog.findChildren<QPushButton*>()) QVERIFY2(!button->isDefault() && !button->autoDefault(), qPrintable(button->text()));
    qApp->setProperty("kaPaperLook", QVariant());
    KaReferenceStatusDialog stock(project);
    stock.show();
    QVERIFY(QTest::qWaitForWindowExposed(&stock));
    QVERIFY(stock.findChild<QLabel*>(QStringLiteral("referenceStatusSummary"))->y() >
            stock.findChild<QLabel*>(QStringLiteral("referenceStatusHint"))->y());
  }
};

int main(int argc, char** argv) {
  QgsApplication app(argc, argv, true);
  QgsApplication::setPrefixPath(qEnvironmentVariable("QGIS_PREFIX_PATH", "A:/OSGeo4W/apps/qgis-dev"), true);
  QgsApplication::initQgis();
  ReferenceStatusPlateTest test;
  const int result = QTest::qExec(&test, argc, argv);
  QgsApplication::exitQgis();
  return result;
}

#include "test_reference_status_plate.moc"
