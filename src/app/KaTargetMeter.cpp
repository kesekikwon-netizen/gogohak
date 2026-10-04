#include "KaTargetMeter.h"

#include <QLocale>
#include <QPainter>

#include <algorithm>
#include <cmath>

namespace {
const QColor kTrack(0xE8, 0xE6, 0xDC);  // the board's light track
QString figure(double percent) { return QLocale().toString(percent, 'f', percent == int(percent) ? 0 : 1); }
}  // namespace

KaTargetMeter::KaTargetMeter(QWidget* parent) : QWidget(parent) {
  setObjectName(QStringLiteral("trenchMeter"));
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  setFixedHeight(sizeHint().height());
}

void KaTargetMeter::setValue(double percent) {
  m_value = std::max(0.0, percent);
  setAccessibleName(QStringLiteral("시굴 비율 %1%, 목표 %2%").arg(figure(m_value), figure(m_target)));
  update();
}

void KaTargetMeter::setTarget(double percent) {
  m_target = std::max(0.1, percent);
  setValue(m_value);
}

void KaTargetMeter::paintEvent(QPaintEvent*) {
  QPainter p(this);
  p.setRenderHint(QPainter::Antialiasing);
  const QColor ink = palette().color(QPalette::WindowText);
  const QColor muted = palette().color(QPalette::PlaceholderText);
  const double top = m_target * 2.0;  // the track runs to twice the 목표
  const double w = width();
  p.setPen(Qt::NoPen);
  p.setBrush(kTrack);
  p.drawRoundedRect(QRectF(0, 0, w, 4), 2, 2);
  p.setBrush(ink);
  p.drawRoundedRect(QRectF(0, 0, w * std::min(1.0, m_value / top), 4), 2, 2);
  const double mark = std::round(w / 2.0) + 0.5;  // 목표 sits at the middle
  p.setRenderHint(QPainter::Antialiasing, false);
  p.setPen(QPen(ink, 1));
  p.drawLine(QPointF(mark, 0), QPointF(mark, 11));
  QFont mono(QStringLiteral("Consolas"));
  mono.setStyleHint(QFont::Monospace);
  mono.setPixelSize(10);
  QFont words = font();
  words.setPixelSize(11);
  const QRectF band(0, 10, w, 16);
  p.setPen(muted);
  p.setFont(mono);
  p.drawText(band, Qt::AlignLeft | Qt::AlignTop, QStringLiteral("0"));
  p.drawText(band, Qt::AlignRight | Qt::AlignTop, figure(top) + QLatin1Char('%'));
  p.setFont(words);
  p.drawText(QRectF(mark + 6, 10, w / 2.0, 16), Qt::AlignLeft | Qt::AlignTop,
             QStringLiteral("목표 %1%").arg(figure(m_target)));
}
