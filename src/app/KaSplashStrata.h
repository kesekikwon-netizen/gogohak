#pragma once

#include <QColor>
#include <QRectF>
#include <QString>

class QPainter;
class QPixmap;

// 새 모양 startup notice (2차 보드 「시작 화면 5초」): a paper card, the name and status at the top left, a
// section drawn in ink above a flat bedrock band that carries the two notice lines. From the first frame
// the card shows the name, the notices and the dashed level string; once the app is ready four
// boundaries are inked bottom up while the levelling staff grows, then the pit, ground line, peg and
// Ⅰ–Ⅴ, stratum Ⅲ fills with clay, two hairlines cross where the 기준점 lands and fade. The level string
// is the progress line: it reaches the staff (「EL. 154.00 m」) at 「준비 완료」. With reduced motion the
// finished picture is shown at once.
namespace KaSplashStrata {

// Timeline, in seconds since the app became ready (2차 보드 시작 화면). Each value runs 0..1.
constexpr double kTimelineSeconds = 4.8;      // the hairlines have faded, the 기준점 stays
double boundaryReveal(int index, double seconds);  // 0 = lowest boundary .. 3, inked left to right
double staffReveal(double seconds);           // the levelling staff grows from the bedrock up
double pitReveal(double seconds);             // pit, ground line, peg and Ⅰ–Ⅴ
double clayReveal(double seconds);            // stratum Ⅲ fills with clay, left to right
double markerReveal(double seconds);          // the 기준점 lands
double crossVisibility(double seconds);       // the two hairlines slide in, then fade

QColor paper();      // the card
QColor bedrock();    // the lowest stratum, under the notice lines
QColor noticeInk();  // the copyright line's ink on the bedrock

struct Frame {
  double seconds = 0.0;   // since readiness; 0 while the app is still starting
  bool still = false;     // reduced motion: the finished picture
  double progress = 0.0;  // 0..1 of the reading interval
  QString status;
  QString version;
};

// Paints the whole card into card (the shadow around it is the caller's).
void paint(QPainter& painter, const QRectF& card, double radius, const QPixmap& icon, const Frame& frame);

}  // namespace KaSplashStrata
