#pragma once

#include "TrenchGridGenerator.h"

#include <QByteArray>
#include <QString>
#include <vector>

// ㄱ·ㄴ자 시굴트렌치 (사용자 요청 2026-10-19): one trench bent at a right angle into two arms of
// the same outer length (15 m + 15 m, 2 m wide). Each L sits in its own arm × arm square of a
// rotated grid, outer corner at the square's low corner: at 0° one arm runs east and the other
// north (ㄴ); the azimuth turns the whole grid. Only whole Ls inside the survey area are kept.
// TrenchGridGenerator calls these when Spec::armLength (or the ratio's armLength) is not 0.
namespace TrenchLShape {

// 7-point closed ring, arms within the 20 m trench limit and longer than the width, matching
// its width / arm / length (= 2 × arm − width) attributes.
bool validCell(const TrenchGridGenerator::Cell& cell);

// rows × cols Ls from the spec origin (map click), squares spaced by balkWidth.
std::vector<TrenchGridGenerator::Cell> build(const TrenchGridGenerator::Spec& spec);

// Whole Ls of the spec inside the area, the grid anchored like the straight fill.
std::vector<TrenchGridGenerator::Cell> buildInArea(const TrenchGridGenerator::Spec& spec, const QByteArray& areaWkb,
                                                   QString* reasonOut = nullptr);

// Searches the gap for the fewest Ls covering targetPct, then shortens every arm equally
// about its corner so the total is exactly the target. Ls stay inside and apart.
TrenchGridGenerator::RatioFill buildForTargetRatio(const QByteArray& areaWkb, double targetPct, double width,
                                                   double azimuthDeg, double armLength);

}  // namespace TrenchLShape
