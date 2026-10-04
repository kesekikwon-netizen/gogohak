#include "KaLongitudeRuler.h"

#include "KaTheme.h"
#include "KaThemeFonts.h"

#include <QFontMetrics>
#include <QPainter>
#include <QPolygonF>

#include <algorithm>
#include <cmath>

namespace {
constexpr double kWest = 126.0;
constexpr double kEast = 130.0;
constexpr double kBoundary = 128.0;  // 중부원점 | 동부원점
// Baselines and the axis as drawn on 2차 보드 판 5 (512 × 88).
constexpr double kZoneBase = 11;
constexpr double kNameBase = 31;
constexpr double kAxisY = 52.5;
constexpr double kDegreeBase = 74;

struct Place {
  const char* name;
  double longitude;
  Qt::AlignmentFlag anchor;  // which side of the name sits on its longitude
};
// The board's towns, west to east; 강릉 and 울산 lean away from 부산 so the three stay apart.
constexpr Place kPlaces[] = {{"제주", 126.53, Qt::AlignHCenter}, {"광주", 126.85, Qt::AlignHCenter},
                             {"대전", 127.38, Qt::AlignHCenter}, {"춘천", 127.73, Qt::AlignHCenter},
                             {"대구", 128.60, Qt::AlignHCenter}, {"강릉", 128.90, Qt::AlignRight},
                             {"부산", 129.08, Qt::AlignHCenter}, {"울산", 129.31, Qt::AlignLeft}};
}  // namespace

KaLongitudeRuler::KaLongitudeRuler(double centre, QWidget* parent) : QWidget(parent), m_centre(centre) {
  setFixedHeight(88);  // 2차 보드 판 5: 512 × 88
  setMinimumWidth(320);
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  QStringList names;
  for (const Place& place : kPlaces) names << QString::fromUtf8(place.name);
  setToolTip(QStringLiteral("동경 128° 서쪽은 중부원점(5186), 동쪽은 동부원점(5187)\n눈금: ") +
             names.join(QStringLiteral(" · ")));
  QString name = QStringLiteral("경도 눈금자: 동경 128도 서쪽은 중부원점, 동쪽은 동부원점.");
  if (std::isfinite(centre)) name += QStringLiteral(" 지금 지도 중심은 %1도").arg(centre, 0, 'f', 1);
  setAccessibleName(name);
}

double KaLongitudeRuler::xFor(double longitude) const {
  const double clamped = std::clamp(longitude, kWest, kEast);
  return (clamped - kWest) / (kEast - kWest) * width();
}

// 2차 보드 판 5: 「중부원점 | 동부원점」 either side of the 128° line, the towns over the axis, tenth-degree ticks and
// the degrees under it, the map centre as a clay line ending in a triangle on the axis.
void KaLongitudeRuler::paintEvent(QPaintEvent*) {
  QPainter p(this);
  const auto& t = KaTheme::tokens();
  const auto pixel = [this](double longitude) { return std::min(std::round(xFor(longitude)), width() - 1.0) + 0.5; };
  // Writes text on a baseline with its left, middle or right on x.
  const auto write = [&p](const QString& text, double x, double baseline, Qt::AlignmentFlag anchor) {
    const double advance = p.fontMetrics().horizontalAdvance(text);
    const double left = anchor == Qt::AlignRight ? x - advance : anchor == Qt::AlignHCenter ? x - advance / 2 : x;
    p.drawText(QPointF(left, baseline), text);
  };
  QFont font = p.font();
  font.setPixelSize(11);
  p.setFont(font);
  const double boundary = pixel(kBoundary);
  p.setPen(t.inkMuted);
  write(QStringLiteral("중부원점"), boundary - 8.5, kZoneBase, Qt::AlignRight);
  write(QStringLiteral("동부원점"), boundary + 8.5, kZoneBase, Qt::AlignLeft);
  p.setPen(t.ink);
  for (const Place& place : kPlaces)
    write(QString::fromUtf8(place.name), pixel(place.longitude), kNameBase, place.anchor);
  // The axis with a tick every tenth of a degree (half and whole degrees longer), the boundary full height.
  p.setPen(QPen(t.ink, 1));
  p.drawLine(QPointF(0, kAxisY), QPointF(width(), kAxisY));
  for (int tenth = 0; tenth <= int(std::lround((kEast - kWest) * 10)); ++tenth) {
    const double x = pixel(kWest + tenth / 10.0);
    p.drawLine(QPointF(x, kAxisY), QPointF(x, kAxisY + (tenth % 10 == 0 ? 7 : tenth % 5 == 0 ? 5 : 3)));
  }
  p.drawLine(QPointF(boundary, 0), QPointF(boundary, height()));
  QFont mono;
  mono.setFamilies(KaTheme::monoFontStack());
  mono.setStyleHint(QFont::Monospace);
  mono.setPixelSize(10);
  p.setFont(mono);
  p.setPen(t.inkMuted);
  for (int degree = int(kWest); degree <= int(kEast); ++degree) {
    const QString label = QStringLiteral("%1°").arg(degree);
    if (degree == int(kWest)) write(label, 0, kDegreeBase, Qt::AlignLeft);
    else if (degree == int(kEast)) write(label, width(), kDegreeBase, Qt::AlignRight);
    else if (degree == int(kBoundary)) write(label, boundary + 6.5, kDegreeBase, Qt::AlignLeft);
    else write(label, pixel(degree), kDegreeBase, Qt::AlignHCenter);
  }
  if (!std::isfinite(m_centre)) return;
  const double cx = std::floor(xFor(m_centre)) + 0.5;
  p.setPen(QPen(t.accent, 1));
  p.drawLine(QPointF(cx, 16), QPointF(cx, 42));
  p.setRenderHint(QPainter::Antialiasing);
  p.setPen(Qt::NoPen);
  p.setBrush(t.accent);
  p.drawPolygon(QPolygonF({QPointF(cx - 5, 43), QPointF(cx + 5, 43), QPointF(cx, 51)}));
}
