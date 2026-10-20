#pragma once

#include "TrenchGridGenerator.h"

#include <QByteArray>

// 「좌표격자에 맞추기」 (사용자 요청 2026-10-20): ㄱ·ㄴ자 trenches laid on the map's coordinate grid
// (KaCanvasGridOverlay) instead of a grid of their own. The coordinate grid's turn is the trench
// azimuth, every outer corner sits on a grid node, and the trenches repeat by whole grid cells (rows
// may shift by whole cells too). Trenches never come closer than kMinBalk, and the plan is the
// fewest whole Ls reaching the target. A grid too coarse to reach it gives no plan and says why.
namespace TrenchGridSnap {

struct Grid {
  double originX = 0.0;  // a grid node (map metres)
  double originY = 0.0;
  double rotationDeg = 0.0;  // clockwise, the sense of trench azimuths (CanvasGridMath::Frame)
  double step = 0.0;         // metres between grid lines
};

constexpr double kMinBalk = 1.0;  // metres of ground left between two trenches

TrenchGridGenerator::RatioFill buildForTargetRatio(const QByteArray& areaWkb, double targetPct, double width,
                                                   double armLength, int armBend, const Grid& grid);

}  // namespace TrenchGridSnap
