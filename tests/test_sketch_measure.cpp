// 「조용한 도판」 guide band while drawing: the last edge and the area (polygon) or the length (line),
// in mono figures after the hint. Display only; the sketch itself is never changed.
#include <QtTest>
#include <QLabel>
#include "app/KaDrawCursor.h"
#include "app/KaDrawGuideBand.h"
#include "app/KaSketchMeasure.h"
#include "app/KaTheme.h"

class TestSketchMeasure : public QObject {
  Q_OBJECT
private slots:
  void polygonShowsLastEdgeAndArea() {
    const QVector<QgsPointXY> points{{0, 0}, {10, 0}, {10, 5}};  // the cursor is the last point
    QCOMPARE(KaSketchMeasure::text(points, true), QStringLiteral("변 5.00 m · 면적 25.0 ㎡"));
  }
  void lineShowsLastEdgeAndLength() {
    const QVector<QgsPointXY> points{{0, 0}, {3, 4}, {3, 10}};
    QCOMPARE(KaSketchMeasure::text(points, false), QStringLiteral("변 6.00 m · 길이 11.00 m"));
  }
  void twoPolygonPointsShowOnlyTheEdge() {
    QCOMPARE(KaSketchMeasure::text({{0, 0}, {4.2, 0}}, true), QStringLiteral("변 4.20 m"));
  }
  // The band shows the figures in mono after the hint, only in 새 모양 and only while there are any.
  void guideBandShowsTheFiguresOnlyInPaper() {
    QWidget host;
    host.resize(900, 600);
    KaDrawGuideBand band(&host);
    band.setTool(QStringLiteral("draw_poly"), QStringLiteral("그리기 · 유구 면"), QStringLiteral("점을 찍습니다"));
    const QString figures = QStringLiteral("변 4.20 m · 면적 32.9 ㎡");
    band.setMeasure(figures);
    auto* label = band.findChild<QLabel*>(QStringLiteral("drawGuideMeasure"));
    QVERIFY(label && label->isHidden());  // stock look: no figures
    KaTheme::DisplayOptions paper;
    paper.paperLook = true;
    KaTheme::setDisplayOptions(nullptr, paper);
    band.setMeasure(figures);
    const bool shown = !label->isHidden();
    const QString text = label->text();
    band.setMeasure(QString());
    const bool emptyHides = label->isHidden();
    KaTheme::setDisplayOptions(nullptr, KaTheme::DisplayOptions());
    QVERIFY(shown);
    QCOMPARE(text, figures);
    QVERIFY(emptyHides);
  }
  // The drawing cursor leaves the point being placed open, so the line under it stays visible
  // (user 2026-10-04 「+ 가 커서 선을 가린다」): a picture cursor whose hot spot is transparent.
  void drawCursorHasAnOpenCentre() {
    const QCursor cursor = KaDrawCursor::cursor();
    const QImage image = cursor.pixmap().toImage();
    QVERIFY2(!image.isNull(), "the system cross covers the point; a picture cursor with an open centre is wanted");
    const QPoint hot = cursor.hotSpot();
    for (const QPoint& offset : {QPoint(0, 0), QPoint(1, 0), QPoint(0, 1), QPoint(-1, 0), QPoint(0, -1)})
      QCOMPARE(image.pixelColor(hot + offset).alpha(), 0);
  }
  void onePointSaysNothing() {
    QVERIFY(KaSketchMeasure::text({{1, 1}}, true).isEmpty());
    QVERIFY(KaSketchMeasure::text({}, false).isEmpty());
  }
};

QTEST_MAIN(TestSketchMeasure)
#include "test_sketch_measure.moc"
