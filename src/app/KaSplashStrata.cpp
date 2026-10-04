#include "KaSplashStrata.h"

#include "KaSplashCredits.h"
#include "KaSplashPalette.h"

#include <QFontMetricsF>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>

#include <algorithm>
#include <array>
#include <initializer_list>
#include <iterator>

namespace KaSplashStrata {
namespace {

// Everything is laid out on the notice's 700 x 400 card (2차 보드 「시작 화면 5초」) and scaled with it.
constexpr double kHeight = 400.0;
constexpr double kStaffX = 632.0;  // the levelling staff; the section ends at it
constexpr double kStringY = 204.5;  // the level string, which is also the progress line

const QColor kPaper(0xFA, 0xF9, 0xF5);
const QColor kInk(0x14, 0x14, 0x13);
const QColor kInk2(0x3D, 0x3D, 0x3A);
const QColor kMuted(0x5E, 0x5D, 0x59);
const QColor kHair(0x87, 0x86, 0x7F);
const QColor kRing(0xB0, 0xAE, 0xA5);
const QColor kClay(0xD9, 0x77, 0x57);
const QColor kBedrock(0x26, 0x26, 0x24);

double ease(double x) {  // OutCubic
  x = std::clamp(x, 0.0, 1.0);
  return 1.0 - (1.0 - x) * (1.0 - x) * (1.0 - x);
}

// A boundary: a start point and two cubics, as drawn on the board.
QPainterPath line(std::initializer_list<double> p) {
  const auto v = std::data(p);
  QPainterPath path(QPointF(v[0], v[1]));
  path.cubicTo(v[2], v[3], v[4], v[5], v[6], v[7]);
  path.cubicTo(v[8], v[9], v[10], v[11], v[12], v[13]);
  return path;
}
// Lowest first: Ⅳ|Ⅴ, Ⅲ|Ⅳ, Ⅱ|Ⅲ, Ⅰ|Ⅱ. The ground line (topsoil) comes with the pit.
const std::array<QPainterPath, 4>& boundaries() {
  static const std::array<QPainterPath, 4> paths = {
      line({0, 324, 100, 318, 220, 328, 330, 320, 440, 312, 520, 312, 632, 316}),
      line({0, 300, 120, 292, 210, 306, 310, 297, 410, 288, 480, 286, 632, 291}),
      line({0, 277, 110, 270, 190, 283, 290, 274, 390, 265, 490, 262, 632, 267}),
      line({0, 258, 100, 252, 200, 262, 300, 254, 400, 246, 480, 243, 632, 246})};
  return paths;
}
QPainterPath groundLine() { return line({0, 243, 90, 237, 170, 247, 262, 238, 354, 229, 450, 227, 632, 229}); }
QPainterPath pitOutline() {
  QPainterPath pit(QPointF(384, 248.3));
  pit.cubicTo(386, 276, 388, 293, 396, 302);
  pit.cubicTo(402, 310, 414, 313, 428, 313);
  pit.cubicTo(442, 313, 452, 309, 456, 300);
  pit.cubicTo(458, 294, 459, 289, 460, 284);
  pit.lineTo(468, 282);
  pit.cubicTo(471, 272, 474, 260, 476, 245.3);
  return pit;
}

QFont serifFont(double pixels) {
  QFont font(QStringLiteral("Noto Serif KR"));
  font.setFamilies({QStringLiteral("Noto Serif KR"), QStringLiteral("Batang"), QStringLiteral("Malgun Gothic")});
  font.setPixelSize(int(pixels + 0.5));
  font.setWeight(QFont::Medium);
  return font;
}
QFont monoFont(double pixels) {
  QFont font;
  font.setFamilies({QStringLiteral("Consolas"), QStringLiteral("Cascadia Mono")});
  font.setStyleHint(QFont::Monospace);
  font.setPixelSize(int(pixels + 0.5));
  return font;
}

// A notice line is never cut off: the text shrinks to the width instead.
void drawFitted(QPainter& painter, QFont font, const QString& text, const QPointF& baseline, double width) {
  while (font.pixelSize() > 8 && QFontMetricsF(font).horizontalAdvance(text) > width)
    font.setPixelSize(font.pixelSize() - 1);
  painter.setFont(font);
  painter.drawText(baseline, text);
}

// Strokes a path from its left end up to share 0..1 of the section's width.
void strokeLeftPart(QPainter& painter, const QPainterPath& path, const QPen& pen, double share) {
  if (share <= 0.0) return;
  painter.save();
  painter.setClipRect(QRectF(-4, 0, (kStaffX + 8) * share, kHeight), Qt::IntersectClip);
  painter.strokePath(path, pen);
  painter.restore();
}

void paintSection(QPainter& painter, double s, double progress) {
  // Ⅲ is the one colour on the card: it fills from the left once the section stands.
  if (const double clay = clayReveal(s); clay > 0.0) {
    QPainterPath fill = boundaries()[2];
    fill.lineTo(632, 291);
    fill.cubicTo(480, 286, 410, 288, 310, 297);
    fill.cubicTo(210, 306, 120, 292, 0, 300);
    fill.closeSubpath();
    painter.save();
    painter.setClipRect(QRectF(0, 240, kStaffX * clay, 80), Qt::IntersectClip);
    painter.fillPath(fill, kClay);
    painter.restore();
  }
  for (int i = 0; i < 3; ++i) strokeLeftPart(painter, boundaries()[size_t(i)], QPen(kInk, 1), boundaryReveal(i, s));
  const double pit = pitReveal(s);
  if (pit > 0.0) {
    painter.save();
    painter.setOpacity(pit);
    // Small stones in Ⅴ.
    painter.setPen(QPen(kHair, 1));
    for (const QPointF& stone : {QPointF(58, 330), QPointF(74.5, 328.2), QPointF(94.7, 328.8), QPointF(168.5, 331.6),
                                 QPointF(282, 331), QPointF(299.5, 328.6), QPointF(317.6, 327.3), QPointF(338.6, 326.6),
                                 QPointF(511.8, 320.2), QPointF(530.5, 329.9), QPointF(552.7, 320.4), QPointF(570.9, 326.7)})
      painter.drawEllipse(stone, 3.0, 2.0);
    const QPainterPath cut = pitOutline();
    QPainterPath body = cut;
    body.closeSubpath();
    painter.fillPath(body, kPaper);
    painter.setClipPath(body, Qt::IntersectClip);
    painter.setPen(QPen(kMuted, 1));
    QPainterPath lens(QPointF(378, 264));
    lens.cubicTo(404, 279, 440, 277, 482, 259);
    lens.moveTo(386, 288);
    lens.cubicTo(410, 300, 438, 298, 470, 281);
    painter.drawPath(lens);
    painter.restore();
    painter.save();
    painter.setOpacity(pit);
    painter.strokePath(cut, QPen(kInk, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter.restore();
  }
  strokeLeftPart(painter, boundaries()[3], QPen(kInk, 1), boundaryReveal(3, s));
  strokeLeftPart(painter, groundLine(), QPen(kInk, 2, Qt::SolidLine, Qt::RoundCap), pit);
  if (pit > 0.0) {
    painter.save();
    painter.setOpacity(pit);
    painter.setPen(QPen(kInk, 1.5));
    painter.drawLine(QPointF(32.5, 200.5), QPointF(32.5, 247.5));  // the peg the string is tied to
    painter.setFont(serifFont(11));
    painter.setPen(kInk);
    const std::array<std::pair<const char*, double>, 5> numerals = {
        {{"Ⅰ", 241.2}, {"Ⅱ", 259.9}, {"Ⅲ", 282.2}, {"Ⅳ", 306.7}, {"Ⅴ", 333.6}}};
    for (const auto& [numeral, baseline] : numerals)
      painter.drawText(QRectF(598, baseline - 12, 20, 14), Qt::AlignHCenter | Qt::AlignBottom, QString::fromUtf8(numeral));
    painter.restore();
  }
  // The levelling staff grows from the bedrock; a long tick every metre (60 px), mid ticks between.
  if (const double staff = staffReveal(s); staff > 0.0) {
    const double top = 344.0 - (344.0 - 196.0) * staff;
    painter.setPen(QPen(kInk, 1));
    painter.drawLine(QPointF(kStaffX + 0.5, top), QPointF(kStaffX + 0.5, 344));
    for (int k = 0; k < 24; ++k) {
      const double y = 204.5 + 6.0 * k;
      if (y < top) continue;
      const double length = k % 10 == 0 ? 8 : k % 5 == 0 ? 5 : 3;
      painter.drawLine(QPointF(kStaffX, y), QPointF(kStaffX + length, y));
    }
    painter.setFont(monoFont(10));
    painter.setPen(kMuted);
    for (const auto& [label, y] : {std::pair{"154", 204.5}, std::pair{"153", 264.5}, std::pair{"152", 324.5}})
      if (y >= top) painter.drawText(QPointF(645, y + 3.5), QString::fromLatin1(label));
  }
  // The level string: dashed until it is pulled from the peg to the staff; it is the progress line.
  QPen dashed(kRing, 1);
  dashed.setDashPattern({8, 6});
  painter.setPen(dashed);
  painter.drawLine(QPointF(32, kStringY), QPointF(kStaffX, kStringY));
  const double pulled = 32 + (kStaffX - 32) * std::clamp(progress, 0.0, 1.0);
  painter.setPen(QPen(kInk, 1));
  painter.drawLine(QPointF(32, kStringY), QPointF(pulled, kStringY));
  if (progress >= 0.999) {
    painter.setFont(monoFont(10));
    painter.setPen(kMuted);
    painter.drawText(QRectF(424, 186, 200, 14), Qt::AlignRight | Qt::AlignVCenter, QStringLiteral("EL. 154.00 m"));
  }
  // Two hairlines slide in from the top and the left, cross where the 기준점 lands, then fade.
  if (const double cross = crossVisibility(s); cross > 0.0) {
    const double in = ease((s - 3.2) / 0.4);
    painter.save();
    painter.setOpacity(cross);
    painter.setPen(QPen(kHair, 1));
    painter.drawLine(QPointF(236.5, kStringY), QPointF(236.5, kStringY + 47.6 * in));
    painter.drawLine(QPointF(0, 234.6), QPointF(266 * in, 234.6));
    painter.restore();
  }
  if (const double landed = markerReveal(s); landed > 0.0) {
    painter.save();
    painter.setOpacity(landed);
    painter.translate(0, -18 * (1.0 - landed));
    QPainterPath mark(QPointF(236, 225.6));
    mark.lineTo(244, 238.6);
    mark.lineTo(228, 238.6);
    mark.closeSubpath();
    painter.fillPath(mark, kPaper);
    painter.strokePath(mark, QPen(kInk, 1.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter.setPen(Qt::NoPen);
    painter.setBrush(kInk);
    painter.drawEllipse(QPointF(236, 234.6), 1.5, 1.5);
    painter.restore();
    painter.save();
    painter.setOpacity(ease((s - 3.6) / 0.4));
    painter.setPen(kMuted);
    painter.setFont(KaSplashPalette::uiFont(10));
    painter.drawText(QPointF(249, 233.1), QStringLiteral("기준점"));
    painter.restore();
  }
}

}  // namespace

double boundaryReveal(int index, double seconds) { return ease((seconds - 0.4 - 0.35 * index) / 0.35); }
double staffReveal(double seconds) { return ease((seconds - 0.4) / 1.4); }
double pitReveal(double seconds) { return ease((seconds - 1.9) / 0.5); }
double clayReveal(double seconds) { return ease((seconds - 2.6) / 0.6); }
double markerReveal(double seconds) { return ease((seconds - 3.2) / 0.4); }
double crossVisibility(double seconds) {
  if (seconds < 3.2) return 0.0;
  return 1.0 - ease((seconds - 3.6) / 1.2);
}

QColor paper() { return kPaper; }
QColor bedrock() { return kBedrock; }
QColor noticeInk() { return KaSplashPalette::withAlpha(kPaper, 0.96); }

void paint(QPainter& painter, const QRectF& card, double radius, const QPixmap& icon, const Frame& frame) {
  painter.save();
  painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform | QPainter::TextAntialiasing);
  QPainterPath outline;
  outline.addRoundedRect(card, radius, radius);
  painter.setClipPath(outline);
  painter.fillPath(outline, kPaper);
  painter.translate(card.topLeft());
  const double unit = card.height() / kHeight;
  painter.scale(unit, unit);
  const double width = card.width() / unit;  // a shrunk notice can be wider than 700:400

  paintSection(painter, frame.still ? 10.0 : frame.seconds, frame.still ? 1.0 : frame.progress);

  if (!icon.isNull())
    painter.drawPixmap(QRectF(32, 32, 44, 44), icon, QRectF(icon.rect()));
  painter.setPen(kInk);
  painter.setFont(serifFont(60));
  painter.drawText(QPointF(29, 140), KaSplashCredits::productName());
  painter.setPen(kMuted);
  painter.setFont(KaSplashPalette::uiFont(14));
  painter.drawText(QPointF(32, 167), KaSplashCredits::productSubtitle(frame.version));
  painter.setPen(kInk2);
  painter.setFont(KaSplashPalette::uiFont(13));
  painter.drawText(QPointF(32, 192), frame.status);

  // The bedrock band carries the two notice lines, worded exactly as on the stock card.
  painter.fillRect(QRectF(0, 344, width, 56), kBedrock);
  painter.setPen(noticeInk());
  drawFitted(painter, KaSplashPalette::uiFont(12), KaSplashCredits::copyrightLine(), QPointF(32, 367), width - 64);
  painter.setPen(KaSplashPalette::withAlpha(kPaper, 0.86));
  drawFitted(painter, KaSplashPalette::lightFont(11), KaSplashCredits::dataLine(), QPointF(32, 385), width - 64);
  painter.restore();
  // A hairline edge so the paper card reads against a light desktop.
  painter.save();
  painter.setRenderHint(QPainter::Antialiasing);
  painter.setPen(QPen(QColor(31, 30, 29, 40), 1.0));
  painter.setBrush(Qt::NoBrush);
  painter.drawRoundedRect(card.adjusted(0.5, 0.5, -0.5, -0.5), radius, radius);
  painter.restore();
}

}  // namespace KaSplashStrata
