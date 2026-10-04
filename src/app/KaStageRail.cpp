#include "KaStageRail.h"

#include <QApplication>
#include <QBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPainterPath>

#include <algorithm>

namespace {
constexpr int kCircle = 18;  // the board's step circle
constexpr int kNameGap = 6;
const QColor kLater(0xB0, 0xAE, 0xA5);    // a later step's ring
const QColor kLaterFace(0xFA, 0xF9, 0xF5);
const QColor kLaterLine(0xC2, 0xC0, 0xB6);

// The box layout that holds widget, searched from layout down (the trail may sit in a nested row or column).
QBoxLayout* holderOf(QLayout* layout, QWidget* widget) {
  if (!layout) return nullptr;
  if (auto* box = qobject_cast<QBoxLayout*>(layout); box && box->indexOf(widget) >= 0) return box;
  for (int i = 0; i < layout->count(); ++i)
    if (QBoxLayout* found = holderOf(layout->itemAt(i)->layout(), widget)) return found;
  return nullptr;
}
}  // namespace

KaStageRail::KaStageRail(const QStringList& stages, QWidget* parent) : QWidget(parent), m_stages(stages) {
  setObjectName(QStringLiteral("stageRail"));
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  setFixedHeight(sizeHint().height());
  setAccessibleName(QStringLiteral("받는 순서: ") + m_stages.join(QStringLiteral(", ")));
}

void KaStageRail::setCurrent(int index) {
  const int clamped = std::clamp(index, 0, int(m_stages.size()));
  if (clamped == m_current) return;
  m_current = clamped;
  update();
}

QSize KaStageRail::sizeHint() const {
  return {int(m_stages.size()) * 96, kCircle + kNameGap + fontMetrics().height()};
}

void KaStageRail::paintEvent(QPaintEvent*) {
  QPainter p(this);
  p.setRenderHint(QPainter::Antialiasing);
  const QColor ink = palette().color(QPalette::WindowText);
  const QColor muted = palette().color(QPalette::PlaceholderText);
  const int steps = int(m_stages.size());
  if (steps == 0) return;
  const double column = double(width()) / steps;
  const double mid = kCircle / 2.0;
  for (int i = 0; i < steps; ++i) {
    const double cx = column * i + column / 2.0;
    const bool done = i < m_current;
    const bool now = i == m_current;
    // the line to the neighbours: ink up to the current step, grey after it
    if (i > 0) {
      p.setPen(QPen(i <= m_current ? ink : kLaterLine, 1));
      p.drawLine(QPointF(column * i, mid), QPointF(cx - mid, mid));
    }
    if (i < steps - 1) {
      p.setPen(QPen(i < m_current ? ink : kLaterLine, 1));
      p.drawLine(QPointF(cx + mid, mid), QPointF(column * (i + 1), mid));
    }
    const QRectF ring(cx - mid + 0.5, 0.5, kCircle - 1, kCircle - 1);
    p.setPen(QPen(done || now ? ink : kLater, 1));
    p.setBrush(done || now ? QColor(Qt::white) : kLaterFace);
    p.drawEllipse(ring);
    if (done) {  // ✓
      QPainterPath tick;
      tick.moveTo(cx - 3.5, mid + 0.2);
      tick.lineTo(cx - 1.0, mid + 2.7);
      tick.lineTo(cx + 3.8, mid - 2.6);
      p.setPen(QPen(ink, 1.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
      p.setBrush(Qt::NoBrush);
      p.drawPath(tick);
    } else if (now) {  // the ink square
      p.setPen(Qt::NoPen);
      p.setBrush(ink);
      p.drawRect(QRectF(cx - 4, mid - 4, 8, 8));
    } else {  // a grey number
      QFont mono(QStringLiteral("Consolas"));
      mono.setStyleHint(QFont::Monospace);
      mono.setPixelSize(10);
      p.setFont(mono);
      p.setPen(muted);
      p.drawText(ring, Qt::AlignCenter, QString::number(i + 1));
    }
    QFont words = font();
    words.setPixelSize(12);
    p.setFont(words);
    p.setPen(done || now ? ink : muted);
    p.drawText(QRectF(column * i, kCircle + kNameGap, column, height() - kCircle - kNameGap),
               Qt::AlignHCenter | Qt::AlignTop, m_stages.at(i));
  }
}

KaStageRail* KaStageRail::replace(QLabel* trail) {
  if (!trail || !qApp || !qApp->property("kaPaperLook").toBool()) return nullptr;
  QBoxLayout* box = holderOf(trail->parentWidget() ? trail->parentWidget()->layout() : nullptr, trail);
  if (!box) return nullptr;
  QStringList stages = trail->text().split(QStringLiteral("→"));
  for (QString& stage : stages) stage = stage.trimmed();
  auto* rail = new KaStageRail(stages, trail->parentWidget());
  box->insertWidget(box->indexOf(trail) + 1, rail);
  trail->hide();
  return rail;
}
