#include "KaMapPagePlate.h"

#include "KaTheme.h"

#include <QEvent>
#include <QFrame>
#include <QLayout>
#include <QObject>
#include <QPointer>
#include <QSplitter>
#include <QWidget>

namespace {

// Sets the page's layout values for the look in use, now and whenever the look changes.
class Follower final : public QObject {
public:
  Follower(QWidget* page, QSplitter* split, QFrame* card) : QObject(page), m_page(page), m_split(split), m_card(card) {
    page->installEventFilter(this);
    apply();
  }

  bool eventFilter(QObject* watched, QEvent* event) override {
    if (watched == m_page && event->type() == QEvent::StyleChange && m_plate != KaTheme::displayOptions().paperLook) apply();
    return QObject::eventFilter(watched, event);
  }

private:
  void apply() {
    m_plate = KaTheme::displayOptions().paperLook;
    const int pad = m_plate ? 0 : 8;
    if (m_page && m_page->layout()) {
      m_page->layout()->setContentsMargins(pad, pad, pad, pad);
      m_page->layout()->setSpacing(pad);
    }
    const int inner = m_plate ? 0 : 4;
    if (m_card && m_card->layout()) {
      m_card->layout()->setContentsMargins(inner, inner, inner, inner);
      m_card->layout()->setSpacing(inner);
    }
  }

  QPointer<QWidget> m_page;
  QPointer<QSplitter> m_split;
  QPointer<QFrame> m_card;
  bool m_plate = false;
};

// One splitter's handle width for the look in use: 7 px in 새 모양 (3 + the hairline + 3), the stock width otherwise.
class HandleFollower final : public QObject {
public:
  HandleFollower(QSplitter* split, int stockWidth) : QObject(split), m_split(split), m_stock(stockWidth) {
    setObjectName(QStringLiteral("kaHandleFollower"));
    split->installEventFilter(this);
    apply();
  }
  void apply(int stockWidth = -1) {
    if (stockWidth > 0) m_stock = stockWidth;
    m_plate = KaTheme::displayOptions().paperLook;
    if (m_split) m_split->setHandleWidth(m_plate ? 7 : m_stock);
  }
  bool eventFilter(QObject* watched, QEvent* event) override {
    if (watched == m_split && event->type() == QEvent::StyleChange && m_plate != KaTheme::displayOptions().paperLook) apply();
    return QObject::eventFilter(watched, event);
  }

private:
  QPointer<QSplitter> m_split;
  int m_stock;
  bool m_plate = false;
};

}  // namespace

namespace KaMapPagePlate {

void followHandle(QSplitter* split, int stockWidth) {
  if (!split) return;
  if (auto* existing = split->findChild<QObject*>(QStringLiteral("kaHandleFollower"), Qt::FindDirectChildrenOnly))
    static_cast<HandleFollower*>(existing)->apply(stockWidth);
  else
    new HandleFollower(split, stockWidth);
}

void applyHandle(QSplitter* split) {
  if (!split) return;
  if (auto* existing = split->findChild<QObject*>(QStringLiteral("kaHandleFollower"), Qt::FindDirectChildrenOnly))
    static_cast<HandleFollower*>(existing)->apply();
}

void follow(QWidget* page, QSplitter* split, QFrame* mapCard) {
  if (page) new Follower(page, split, mapCard);
  followHandle(split, 10);
}

}  // namespace KaMapPagePlate
