#include "KaTabClose.h"

#include <QApplication>
#include <QIcon>
#include <QPainter>
#include <QPixmap>
#include <QTabBar>
#include <QTabWidget>
#include <QToolButton>

namespace {
const char* kCross = "viewTabClose";

// A 10 px cross drawn in 1 px muted ink (the board's tab mark), also at 2x for high-DPI screens.
QIcon cross(const QColor& ink) {
  QIcon icon;
  for (const int scale : {1, 2}) {
    QPixmap mark(10 * scale, 10 * scale);
    mark.setDevicePixelRatio(scale);
    mark.fill(Qt::transparent);
    QPainter p(&mark);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(QPen(ink, 1.2));
    p.drawLine(QPointF(1, 1), QPointF(9, 9));
    p.drawLine(QPointF(9, 1), QPointF(1, 9));
    p.end();
    icon.addPixmap(mark);
  }
  return icon;
}

void sweep(QTabWidget* tabs) {
  if (!qApp || !qApp->property("kaPaperLook").toBool()) return;
  QTabBar* bar = tabs->tabBar();
  for (int i = 0; i < bar->count(); ++i) {
    QWidget* current = bar->tabButton(i, QTabBar::RightSide);
    if (!current || current->objectName() == QLatin1String(kCross)) continue;  // not closable, or already ours
    auto* button = new QToolButton(bar);
    button->setObjectName(QLatin1String(kCross));
    button->setIcon(cross(bar->palette().color(QPalette::PlaceholderText)));
    button->setIconSize(QSize(10, 10));
    button->setAutoRaise(true);
    button->setFixedSize(16, 16);
    button->setCursor(Qt::PointingHandCursor);
    button->setToolTip(QStringLiteral("탭 닫기"));
    button->setAccessibleName(QStringLiteral("%1 탭 닫기").arg(bar->tabText(i)));
    QObject::connect(button, &QToolButton::clicked, tabs, [tabs, bar, button] {
      for (int at = 0; at < bar->count(); ++at)
        if (bar->tabButton(at, QTabBar::RightSide) == button) emit tabs->tabCloseRequested(at);
    });
    bar->setTabButton(i, QTabBar::RightSide, button);
    current->deleteLater();  // the tab bar only hides a replaced button
  }
}
}  // namespace

namespace KaTabClose {

void follow(QTabWidget* tabs) {
  QObject::connect(tabs, &QTabWidget::currentChanged, tabs, [tabs] { sweep(tabs); });
  sweep(tabs);
}

}  // namespace KaTabClose
