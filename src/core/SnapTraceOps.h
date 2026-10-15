#pragma once

#include <QList>
#include <QVector>
#include <qgspointxy.h>

class QgsMapCanvas;
class QgsVertexMarker;

// 선 따라 자석 (SnapTraceOps):
// 1) 마우스가 가까이 갔을 때(radiusPx 안) 주변 자석점만 미리 표시(전체 표시로 어지러움 방지).
// 2) 같은 선·경계를 따라 그릴 때 두 클릭 사이에 빠진 꼭짓점들을 최단 경로로 자동 채움.
namespace SnapTraceOps {

QVector<QgsPointXY> nearbyVertices(QgsMapCanvas* canvas, const QgsPointXY& cursorMapPt,
                                   const QgsPointXY* activeSnapPt = nullptr, double radiusPx = 48.0, int maxCount = 12);

void syncPreviewMarkers(QgsMapCanvas* canvas, QList<QgsVertexMarker*>& markers, const QVector<QgsPointXY>& points);

void clearPreviewMarkers(QList<QgsVertexMarker*>& markers);

QVector<QgsPointXY> traceBetween(QgsMapCanvas* canvas, const QgsPointXY& fromMapPt, const QgsPointXY& toMapPt,
                                 int maxIntermediate = 20);

}  // namespace SnapTraceOps
