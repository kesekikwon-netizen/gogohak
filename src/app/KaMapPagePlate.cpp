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
    if (m_split) m_split->setHandleWidth(m_plate ? 7 : 10);
  }

  QPointer<QWidget> m_page;
  QPointer<QSplitter> m_split;
  QPointer<QFrame> m_card;
  bool m_plate = false;
};

}  // namespace

namespace KaMapPagePlate {

void follow(QWidget* page, QSplitter* split, QFrame* mapCard) {
  if (page) new Follower(page, split, mapCard);
}

}  // namespace KaMapPagePlate
