#pragma once

#include <QPair>
#include <QString>
#include <QStyledItemDelegate>

// 파일함 rows (2차 보드 판 4 왼쪽 파일함): in 새 모양 a small folder or file mark, the name, and a file's extension set
// apart at the right in 11 px mono, rows 24 px. Stock paints the row as the list always did. The rows keep their own
// data (Qt::UserRole path, Qt::UserRole + 1 folder flag); the look is decided at paint time, so a switch follows.
class KaFileRowDelegate final : public QStyledItemDelegate {
public:
  using QStyledItemDelegate::QStyledItemDelegate;

  // The row text split into the name and the extension (with its dot): 「[폴더] 광령리1」 → 광령리1, 「a.dxf」 → a, .dxf.
  static QPair<QString, QString> parts(const QString& text, bool folder);

  void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
  QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;
};
