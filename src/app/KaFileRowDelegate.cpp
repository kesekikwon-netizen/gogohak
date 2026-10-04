#include "KaFileRowDelegate.h"

#include <QApplication>
#include <QPainter>
#include <QPainterPath>

namespace {
bool paperLook() { return qApp && qApp->property("kaPaperLook").toBool(); }

// A 12 px folder (tab and body) or file (folded corner) outline in muted ink.
void drawMark(QPainter* p, const QRectF& box, bool folder, const QColor& ink) {
  p->save();
  p->setRenderHint(QPainter::Antialiasing);
  p->setPen(QPen(ink, 1.1));
  p->setBrush(Qt::NoBrush);
  const QRectF r = box.adjusted(0.5, 0.5, -0.5, -0.5);
  QPainterPath path;
  if (folder) {
    path.moveTo(r.left(), r.top() + 2);
    path.lineTo(r.left() + r.width() * 0.4, r.top() + 2);
    path.lineTo(r.left() + r.width() * 0.5, r.top() + 3.5);
    path.lineTo(r.right(), r.top() + 3.5);
    path.lineTo(r.right(), r.bottom() - 1);
    path.lineTo(r.left(), r.bottom() - 1);
    path.closeSubpath();
  } else {
    const double fold = 3.0;
    path.moveTo(r.left() + 1.5, r.top());
    path.lineTo(r.right() - 1.5 - fold, r.top());
    path.lineTo(r.right() - 1.5, r.top() + fold);
    path.lineTo(r.right() - 1.5, r.bottom());
    path.lineTo(r.left() + 1.5, r.bottom());
    path.closeSubpath();
  }
  p->drawPath(path);
  p->restore();
}
}  // namespace

QPair<QString, QString> KaFileRowDelegate::parts(const QString& text, bool folder) {
  const QString prefix = QStringLiteral("[폴더] ");
  if (folder) return {text.startsWith(prefix) ? text.mid(prefix.size()) : text, QString()};
  const int dot = text.lastIndexOf(QLatin1Char('.'));
  if (dot <= 0) return {text, QString()};
  return {text.left(dot), text.mid(dot)};
}

void KaFileRowDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const {
  if (!paperLook()) {
    QStyledItemDelegate::paint(painter, option, index);
    return;
  }
  QStyleOptionViewItem plain = option;
  initStyleOption(&plain, index);
  const QString text = plain.text;
  plain.text.clear();  // the frame, hover and selection as the style draws them, without its text
  const QStyle* style = plain.widget ? plain.widget->style() : QApplication::style();
  style->drawControl(QStyle::CE_ItemViewItem, &plain, painter, plain.widget);
  const bool folder = index.data(Qt::UserRole + 1).toBool();
  const auto [name, extension] = parts(text, folder);
  const QColor muted = option.palette.color(QPalette::PlaceholderText);
  const QColor ink = option.palette.color(QPalette::Text);  // the sheet's selection is a light wash, not Highlight
  const QRect row = option.rect.adjusted(4, 0, -6, 0);
  drawMark(painter, QRectF(row.left(), row.center().y() - 6, 12, 12), folder, muted);
  int right = row.right();
  painter->save();
  if (!extension.isEmpty()) {
    QFont mono(QStringLiteral("Consolas"));
    mono.setStyleHint(QFont::Monospace);
    mono.setPixelSize(11);
    painter->setFont(mono);
    painter->setPen(muted);
    const int width = QFontMetrics(mono).horizontalAdvance(extension);
    painter->drawText(QRect(right - width, row.top(), width, row.height()), Qt::AlignRight | Qt::AlignVCenter, extension);
    right -= width + 8;
  }
  painter->setFont(option.font);
  painter->setPen(ink);
  const QRect nameBox(row.left() + 20, row.top(), std::max(0, right - row.left() - 20), row.height());
  painter->drawText(nameBox, Qt::AlignLeft | Qt::AlignVCenter,
                    option.fontMetrics.elidedText(name, Qt::ElideRight, nameBox.width()));
  painter->restore();
}

QSize KaFileRowDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const {
  QSize size = QStyledItemDelegate::sizeHint(option, index);
  if (paperLook()) size.setHeight(24);
  return size;
}
