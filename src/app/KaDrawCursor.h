#pragma once
#include <QCursor>
#include <qgsapplication.h>

// 그리기 커서: QGIS's own crosshair, whose centre is open, so the line under the point being placed stays
// visible (user 2026-10-04 「조사구역 그리기나 그리기를 할때 + 가 커서 선을 가린다」). The system
// Qt::CrossCursor drew its thick bars right over that point.
namespace KaDrawCursor {
inline QCursor cursor() { return QgsApplication::getThemeCursor(QgsApplication::Cursor::CrossHair); }
}  // namespace KaDrawCursor
