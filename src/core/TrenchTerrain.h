#pragma once

#include "TrenchGridGenerator.h"

#include <QByteArray>
#include <QString>
#include <vector>

class QgsProject;

// 시굴격자 사면 방향 (사용자 요청 2026-10-05): 시굴 trenches run across the contours from the high side to the low
// side; on flat ground, or with no terrain data, north-south.
namespace TrenchTerrain {
// 표고 samples along the 수치지형도 contours — line layers with an "elevation" field — in and around the survey
// area, in the area's CRS (the project's when empty). Read only.
std::vector<TrenchGridGenerator::ElevSample> contourSamples(const QgsProject* project, const QByteArray& areaWkb,
                                                             const QString& areaCrs);
// The uphill direction from those contours (used when there is no DEM).
TrenchGridGenerator::SlopeAspect contourAspect(const QgsProject* project, const QByteArray& areaWkb,
                                               const QString& areaCrs);
// Tr-1 is the cell furthest along the azimuth (uphill, or north at 0°); the numbers walk back down it, and run
// across it from left to right within a row.
void numberFromHighSide(std::vector<TrenchGridGenerator::Cell>& cells, double azimuthDeg, const QString& prefix);
}  // namespace TrenchTerrain
