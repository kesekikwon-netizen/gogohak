#include "LegendPlacement.h"

#include <algorithm>

namespace LegendPlacement {

namespace {
// The start of a span of `length` that must lie inside [low, low + room]; a span longer than the room starts at `low`.
double pushed(double start, double length, double low, double room) {
  if (length >= room) return low;
  return std::clamp(start, low, low + room - length);
}
}  // namespace

QRectF insidePage(const QRectF& page, const QRectF& box) {
  if (!page.isValid() || page.isEmpty() || !box.isValid()) return box;
  return QRectF(pushed(box.left(), box.width(), page.left(), page.width()),
                pushed(box.top(), box.height(), page.top(), page.height()), box.width(), box.height());
}

}  // namespace LegendPlacement
