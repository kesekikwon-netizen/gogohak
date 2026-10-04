#include "KaLongitudeRuler.h"

#include "KaTheme.h"

#include <QFontMetrics>
#include <QPainter>
#include <QPolygonF>

#include <algorithm>
#include <cmath>

namespace {
constexpr double kWest = 124.0;
constexpr double kEast = 132.0;
constexpr double kBoundary = 128.0;  // 중부원점 | 동부원점
constexpr int kSide = 16;            // room for the end labels
constexpr int kAxisY = 30;

struct Place {
  const char* name;
  double longitude;
};
// Towns the 새 조사 region hint names, west to east, either side of 128°.
constexpr Place kPlaces[] = {{"제주", 126.53}, {"광주", 126.85}, {"서울", 126.98}, {"대전", 127.38},
                             {"춘천", 127.73}, {"원주", 127.95}, {"대구", 128.60}, {"강릉", 128.90},
                             {"부산", 129.08}, {"삼척", 129.17}, {"울산", 129.31}};
}  // namespace

KaLongitudeRuler::KaLongitudeRuler(double centre, QWidget* parent) : QWidget(parent), m_centre(centre) {
  setFixedHeight(72);
  setMinimumWidth(320);
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  QStringList names;
  for (const Place& place : kPlaces) names << QString::fromUtf8(place.name);
  setToolTip(QStringLiteral("동경 128° 서쪽은 중부원점(5186), 동쪽은 동부원점(5187)\n눈금: ") +
             names.join(QStringLiteral(" · ")));
}

double KaLongitudeRuler::xFor(double longitude) const {
  const double clamped = std::clamp(longitude, kWest, kEast);
  return kSide + (clamped - kWest) / (kEast - kWest) * (width() - 2 * kSide);
}

void KaLongitudeRuler::paintEvent(QPaintEvent*) {
  QPainter p(this);
  const auto& t = KaTheme::tokens();
  QFont font = p.font();
  font.setPixelSize(11);
  p.setFont(font);
  p.setPen(t.inkMuted);
  p.drawText(QRectF(xFor(kWest), 0, xFor(kBoundary) - xFor(kWest) - 6, 13), Qt::AlignLeft | Qt::AlignVCenter,
             QStringLiteral("중부원점 5186"));
  p.drawText(QRectF(xFor(kBoundary) + 6, 0, xFor(kEast) - xFor(kBoundary) - 6, 13), Qt::AlignRight | Qt::AlignVCenter,
             QStringLiteral("동부원점 5187"));
  // Axis, one tick per degree with a mono label under it, the 128° boundary as a full ink line.
  p.setPen(QPen(t.ink, 1));
  p.drawLine(QPointF(xFor(kWest), kAxisY + 0.5), QPointF(xFor(kEast), kAxisY + 0.5));
  QFont mono(QStringLiteral("Consolas"));
  mono.setStyleHint(QFont::Monospace);
  mono.setPixelSize(10);
  for (int degree = int(kWest); degree <= int(kEast); ++degree) {
    const double x = std::round(xFor(degree)) + 0.5;
    p.setPen(QPen(t.ink, 1));
    p.drawLine(QPointF(x, kAxisY), QPointF(x, kAxisY + 6));
    p.setFont(mono);
    p.setPen(t.inkMuted);
    p.drawText(QRectF(x - 20, kAxisY + 7, 40, 12), Qt::AlignHCenter | Qt::AlignTop, QStringLiteral("%1°").arg(degree));
  }
  const double boundary = std::round(xFor(kBoundary)) + 0.5;
  p.setPen(QPen(t.ink, 1));
  p.drawLine(QPointF(boundary, 14), QPointF(boundary, kAxisY + 6));  // stops above its 「128°」 label
  // Place ticks above the axis; names in two rows under the degrees, a name that would touch its
  // neighbour is left to the tooltip.
  font.setPixelSize(10);
  p.setFont(font);
  const QFontMetrics metrics(font);
  double rowEnd[2] = {-1e9, -1e9};
  for (const Place& place : kPlaces) {
    const double x = std::round(xFor(place.longitude)) + 0.5;
    p.setPen(QPen(t.borderStrong, 1));
    p.drawLine(QPointF(x, kAxisY - 5), QPointF(x, kAxisY));
    const QString name = QString::fromUtf8(place.name);
    const double half = metrics.horizontalAdvance(name) / 2.0;
    for (int row = 0; row < 2; ++row) {
      if (x - half < rowEnd[row] + 4) continue;
      p.setPen(t.inkMuted);
      p.drawText(QRectF(x - half, 50 + row * 11, 2 * half, 11), Qt::AlignCenter, name);
      rowEnd[row] = x + half;
      break;
    }
  }
  if (!std::isfinite(m_centre)) return;
  // The map centre: a clay triangle on the axis with its longitude above.
  const double cx = xFor(m_centre);
  p.setRenderHint(QPainter::Antialiasing);
  p.setPen(Qt::NoPen);
  p.setBrush(t.accent);
  p.drawPolygon(QPolygonF({QPointF(cx - 5, kAxisY - 9), QPointF(cx + 5, kAxisY - 9), QPointF(cx, kAxisY - 1)}));
  font.setPixelSize(11);
  p.setFont(font);
  p.setPen(t.ink);
  const QString label = QStringLiteral("지금 지도 중심 %1°").arg(m_centre, 0, 'f', 1);
  const double w = QFontMetrics(font).horizontalAdvance(label) + 2;
  const double left = std::clamp(cx - w / 2, 0.0, width() - w);
  p.fillRect(QRectF(left - 2, 0, w + 4, 13), palette().window());
  p.drawText(QRectF(left, 0, w, 13), Qt::AlignCenter, label);
}
