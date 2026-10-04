#include "KaSketchMeasure.h"

#include <cmath>

namespace KaSketchMeasure {

QString text(const QVector<QgsPointXY>& points, bool polygon) {
  if (points.size() < 2) return {};
  const QgsPointXY& a = points.at(points.size() - 2);
  const QgsPointXY& b = points.last();
  QString out = QStringLiteral("변 %1 m").arg(std::hypot(b.x() - a.x(), b.y() - a.y()), 0, 'f', 2);
  if (polygon) {
    if (points.size() < 3) return out;
    double twice = 0.0;  // shoelace over the closed ring
    for (int i = 0; i < points.size(); ++i) {
      const QgsPointXY& p = points.at(i);
      const QgsPointXY& q = points.at((i + 1) % points.size());
      twice += p.x() * q.y() - q.x() * p.y();
    }
    return out + QStringLiteral(" · 면적 %1 ㎡").arg(std::abs(twice) / 2.0, 0, 'f', 1);
  }
  double length = 0.0;
  for (int i = 1; i < points.size(); ++i)
    length += std::hypot(points.at(i).x() - points.at(i - 1).x(), points.at(i).y() - points.at(i - 1).y());
  return out + QStringLiteral(" · 길이 %1 m").arg(length, 0, 'f', 2);
}

}  // namespace KaSketchMeasure
