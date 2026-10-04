#pragma once

#include <QPixmap>

class QgsSymbol;
class QgsVectorLayer;

// 「조용한 도판」 견본: a map symbol as a 20 × 14 swatch in a 1 px ink frame, like the layer list's 유구 표지.
namespace KaSymbolSwatch {
// Empty for no symbol.
QPixmap framed(QgsSymbol* symbol);
// The layer's first legend symbol (the first kind of a 종류·시대 look); empty without a renderer.
QPixmap ofLayer(QgsVectorLayer* layer);
}  // namespace KaSymbolSwatch
