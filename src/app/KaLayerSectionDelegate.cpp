#include "KaLayerSectionDelegate.h"
#include "KaTheme.h"

#include "core/LayerOps.h"
#include "core/LayerRole.h"

#include <QAbstractItemView>
#include <QApplication>
#include <QFont>
#include <QPainter>
#include <QPen>

#include <qgslayertree.h>
#include <qgslayertreeview.h>
#include <qgsmaplayer.h>

namespace {
constexpr int kTextLeft = 6;
constexpr int kCountRight = 4;  // the count column's gap before the 글자 column
}  // namespace

KaLayerSectionDelegate::KaLayerSectionDelegate(QAbstractItemDelegate* inner, QgsLayerTreeView* view)
    : QStyledItemDelegate(view), m_inner(inner), m_view(view) {}

KaLayerSectionDelegate::Section KaLayerSectionDelegate::sectionOf(QgsLayerTreeNode* node) {
  if (!node || QgsLayerTree::isGroup(node)) return Section::None;
  auto* leaf = qobject_cast<QgsLayerTreeLayer*>(node);
  const QgsMapLayer* layer = leaf ? leaf->layer() : nullptr;
  if (!layer) return Section::Nearby;
  if (!LayerOps::layerKeyOf(layer).isEmpty() || LayerRole::resolve(layer) == LayerRole::Kind::Survey)
    return Section::Survey;
  if (LayerOps::isBasemapLayer(layer)) return Section::Basemap;
  return Section::Nearby;
}

QString KaLayerSectionDelegate::sectionTitle(Section section) {
  switch (section) {
    case Section::Survey: return QStringLiteral("조사 데이터");
    case Section::Nearby: return QStringLiteral("주변·참조");
    case Section::Basemap: return QStringLiteral("배경 지도");
    case Section::None: break;
  }
  return {};
}

bool KaLayerSectionDelegate::enabled() const {
  const QVariant flag = m_view ? m_view->property("kaLayerSections") : QVariant();
  return !flag.isValid() || flag.toBool();
}

bool KaLayerSectionDelegate::hasBand(const QModelIndex& index) const {
  if (!m_view || !index.isValid() || index.parent().isValid() || !enabled()) return false;
  const QModelIndex first = index.siblingAtColumn(0);
  const Section section = sectionOf(m_view->index2node(first));
  if (section == Section::None) return false;
  if (first.row() == 0) return true;
  return sectionOf(m_view->index2node(first.sibling(first.row() - 1, 0))) != section;
}

QStyleOptionViewItem KaLayerSectionDelegate::below(const QStyleOptionViewItem& option,
                                                   const QModelIndex& index) const {
  QStyleOptionViewItem moved(option);
  const int band = bandHeight(index);
  if (band > 0) moved.rect.adjust(0, band, 0, 0);
  return moved;
}

// The band keeps the row's own background (alternate colours stay); it adds a 1 px
// top line and the section title, 10 px bold in the muted ink, 6 px from the left.
// Colours come from the view palette KaTheme::palette() fills (Mid = border,
// PlaceholderText = inkMuted).
void KaLayerSectionDelegate::paintBand(QPainter* painter, const QStyleOptionViewItem& option,
                                       const QModelIndex& index) const {
  if (index.column() != 0) return;
  const auto* view = qobject_cast<const QAbstractItemView*>(option.widget);
  const int width = view && view->viewport() ? view->viewport()->width() : option.rect.right() + 1;
  const QRect band(0, option.rect.top(), width, kBandHeight);
  painter->save();
  painter->setPen(QPen(option.palette.color(QPalette::Mid), 1));
  painter->drawLine(band.topLeft(), band.topRight());
  QFont font = option.font;
  font.setPixelSize(11);  // band caption; the list rows themselves stay 10 px (user decision)
  font.setBold(true);
  painter->setFont(font);
  painter->setPen(option.palette.color(QPalette::PlaceholderText));
  painter->drawText(band.adjusted(kTextLeft, 1, -4, 0), Qt::AlignLeft | Qt::AlignVCenter | Qt::TextSingleLine,
                    sectionTitle(sectionOf(m_view->index2node(index))));
  painter->restore();
}

void KaLayerSectionDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                                   const QModelIndex& index) const {
  if (hasBand(index)) paintBand(painter, option, index);
  const QStyleOptionViewItem moved = below(option, index);
  if (m_inner) m_inner->paint(painter, moved, index);
  else QStyledItemDelegate::paint(painter, moved, index);
  paintPlateCheck(painter, moved, index);
  const QVariant count = index.column() == 0 ? index.data(kCountRole) : QVariant();
  if (!count.isValid()) return;
  // The count is a right-aligned column at the end of the layer column in 10 px mono, muted ink (2차 보드 판 1;
  // QGIS draws the editing pencil at the list's right edge). A name that would reach it pushes it back to 6 px
  // after the name; it never runs past the column.
  QStyleOptionViewItem item(moved);
  initStyleOption(&item, index);
  const QStyle* style = item.widget ? item.widget->style() : QApplication::style();
  const QRect text = style->subElementRect(QStyle::SE_ItemViewItemText, &item, item.widget);
  const int margin = style->pixelMetric(QStyle::PM_FocusFrameHMargin, nullptr, item.widget) + 1;
  QFont mono(QStringLiteral("Consolas"));
  mono.setStyleHint(QFont::Monospace);
  mono.setPixelSize(10);
  const QString digits = QString::number(count.toLongLong());
  const int width = QFontMetrics(mono).horizontalAdvance(digits);
  const int afterName = text.left() + margin + QFontMetrics(item.font).horizontalAdvance(item.text) + 6;
  const int x = qMin(qMax(afterName, text.right() - kCountRight - width), text.right() - width);
  painter->save();
  painter->setFont(mono);
  painter->setPen(option.palette.color(QPalette::PlaceholderText));
  painter->drawText(QRect(x, text.top(), width + 1, text.height()), Qt::AlignLeft | Qt::AlignVCenter, digits);
  painter->restore();
}

// 「조용한 도판」 (2차 보드 판 1): QGIS's layer tree style draws a bare tick; in 새 모양 the check is the app's
// own square (ink with an ivory tick, an empty grey-edged square), drawn over it at the same place. It reads the
// look as it paints, so switching 「새 모양」 while the app runs needs nothing else.
void KaLayerSectionDelegate::paintPlateCheck(QPainter* painter, const QStyleOptionViewItem& option,
                                             const QModelIndex& index) const {
  if (!index.data(Qt::CheckStateRole).isValid() || !KaTheme::displayOptions().paperLook) return;
  QStyleOptionViewItem item(option);
  initStyleOption(&item, index);
  const QStyle* style = item.widget ? item.widget->style() : QApplication::style();
  QStyleOptionViewItem box(item);
  box.rect = style->subElementRect(QStyle::SE_ItemViewItemCheckIndicator, &item, item.widget);
  box.state &= ~(QStyle::State_On | QStyle::State_Off | QStyle::State_NoChange);
  box.state |= item.checkState == Qt::Checked            ? QStyle::State_On
               : item.checkState == Qt::PartiallyChecked ? QStyle::State_NoChange
                                                         : QStyle::State_Off;
  KaTheme::paintCheckIndicator(&box, painter);
}

QSize KaLayerSectionDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const {
  QSize size = m_inner ? m_inner->sizeHint(option, index) : QStyledItemDelegate::sizeHint(option, index);
  if (hasBand(index)) size.rheight() += kBandHeight;
  return size;
}

bool KaLayerSectionDelegate::editorEvent(QEvent* event, QAbstractItemModel* model,
                                         const QStyleOptionViewItem& option, const QModelIndex& index) {
  const QStyleOptionViewItem moved = below(option, index);
  if (m_inner) return m_inner->editorEvent(event, model, moved, index);
  return QStyledItemDelegate::editorEvent(event, model, moved, index);
}

bool KaLayerSectionDelegate::helpEvent(QHelpEvent* event, QAbstractItemView* view,
                                       const QStyleOptionViewItem& option, const QModelIndex& index) {
  const QStyleOptionViewItem moved = below(option, index);
  if (m_inner) return m_inner->helpEvent(event, view, moved, index);
  return QStyledItemDelegate::helpEvent(event, view, moved, index);
}

void KaLayerSectionDelegate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option,
                                                  const QModelIndex& index) const {
  QStyledItemDelegate::updateEditorGeometry(editor, below(option, index), index);
}
