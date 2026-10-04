#pragma once
#include <QString>
#include <QVector>
#include <qgspointxy.h>

// 「조용한 도판」 figures for the drawing guide band: 「변 4.20 m · 면적 32.9 ㎡」 for a polygon,
// 「변 6.00 m · 길이 11.00 m」 for a line. points = the sketch so far plus the cursor, in metres
// (a projected work CRS). Fewer than two points: empty.
namespace KaSketchMeasure {
QString text(const QVector<QgsPointXY>& points, bool polygon);
}
