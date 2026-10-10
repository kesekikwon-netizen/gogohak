#pragma once

#include <QRectF>

// 범례 상자를 용지 안에 둔다 (보완 L4). 범례 상자가 용지 밖으로 걸치면 가장 짧게 안쪽으로 민다. 이미 들어 있으면 그대로,
// 용지보다 크면 왼쪽 위 모서리를 용지 왼쪽 위에 맞춘다(크기는 바꾸지 않는다). 용지를 모르면(빈 사각형) 건드리지 않는다.
namespace LegendPlacement {
QRectF insidePage(const QRectF& page, const QRectF& box);
}  // namespace LegendPlacement
