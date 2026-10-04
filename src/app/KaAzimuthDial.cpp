#include "KaAzimuthDial.h"

#include <QLocale>
#include <QPainter>

#include <cmath>

namespace {
const QColor kRing(0x87, 0x86, 0x7F);  // the board's dial ring
}  // namespace

KaAzimuthDial::KaAzimuthDial(QWidget* parent) : QWidget(parent) {
  setObjectName(QStringLiteral("trenchDial"));
  setFixedSize(sizeHint());
  setAzimuth(0.0);
}

void KaAzimuthDial::setAzimuth(double degrees) {
  m_azimuth = std::fmod(std::fmod(degrees, 360.0) + 360.0, 360.0);
  setAccessibleName(QStringLiteral("방위 %1도").arg(QLocale().toString(m_azimuth, 'f', 0)));
  update();
}

QString KaAzimuthDial::note(double degrees) {
  const double turn = std::fmod(std::fmod(degrees, 360.0) + 360.0, 360.0);
  if (std::abs(turn) < 0.5 || std::abs(turn - 360.0) < 0.5) return QStringLiteral("위가 북쪽입니다");
  return QStringLiteral("북에서 시계 방향 %1°").arg(QLocale().toString(turn, 'f', 0));
}

void KaAzimuthDial::paintEvent(QPaintEvent*) {
  QPainter p(this);
  p.setRenderHint(QPainter::Antialiasing);
  const QColor ink = palette().color(QPalette::WindowText);
  const QPointF centre(20, 20);
  p.setPen(QPen(kRing, 1));
  p.setBrush(Qt::white);
  p.drawEllipse(centre, 15.5, 15.5);
  p.setPen(QPen(ink, 2));
  p.drawLine(QPointF(20, 0.5), QPointF(20, 6));  // north
  p.save();
  p.translate(centre);
  p.rotate(m_azimuth);  // clockwise from north, as the azimuth is
  p.setPen(QPen(ink, 1.5, Qt::SolidLine, Qt::RoundCap));
  p.drawLine(QPointF(0, 0), QPointF(0, -11.5));
  p.restore();
  p.setPen(Qt::NoPen);
  p.setBrush(ink);
  p.drawEllipse(centre, 2, 2);
}
