#include "KaSymbolSwatch.h"
#include "KaTheme.h"

#include <QPainter>

#include <qgslegendsymbolitem.h>
#include <qgsrenderer.h>
#include <qgssymbol.h>
#include <qgssymbollayerutils.h>
#include <qgsvectorlayer.h>

namespace KaSymbolSwatch {

QPixmap framed(QgsSymbol* symbol, const QSize& size) {
  if (!symbol) return {};
  QPixmap patch = QgsSymbolLayerUtils::symbolPreviewPixmap(symbol, size, 0);
  if (patch.isNull()) return {};
  QPainter painter(&patch);
  painter.setPen(QPen(KaTheme::tokens().ink, 1));
  painter.drawRect(0, 0, size.width() - 1, size.height() - 1);
  return patch;
}

QPixmap ofLayer(QgsVectorLayer* layer) {
  QgsFeatureRenderer* renderer = layer ? layer->renderer() : nullptr;
  const QgsLegendSymbolList symbols = renderer ? renderer->legendSymbolItems() : QgsLegendSymbolList();
  return symbols.isEmpty() ? QPixmap() : framed(symbols.first().symbol());
}

}  // namespace KaSymbolSwatch
