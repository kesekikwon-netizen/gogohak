#include "KaElevationFigure.h"
#include "KaThemeFonts.h"

#include <QPainter>
#include <QPainterPath>

#include <optional>

namespace {
const QColor kGuide(0xB0, 0xAE, 0xA5);  // the board's dashed level guides

std::optional<double> number(const QString& text) {
  bool ok = false;
  const double value = text.trimmed().toDouble(&ok);
  return ok ? std::optional<double>(value) : std::nullopt;
}

QString metres(const std::optional<double>& value) {
  return value ? QString::number(*value, 'f', 2) : QStringLiteral("—");
}

std::optional<double> depthOf(const QString& top, const QString& bottom, const QString& depth) {
  if (const auto typed = number(depth)) return typed;
  const auto upper = number(top);
  const auto lower = number(bottom);
  if (upper && lower) return *upper - *lower;
  return std::nullopt;
}
}  // namespace

KaElevationFigure::KaElevationFigure(QWidget* parent) : QWidget(parent) {
  setObjectName(QStringLiteral("recordElevationFigure"));
  setFixedSize(sizeHint());
  setLevels(QString(), QString(), QString());
}

void KaElevationFigure::setLevels(const QString& top, const QString& bottom, const QString& depth) {
  m_top = top;
  m_bottom = bottom;
  m_depth = depth;
  setAccessibleName(QStringLiteral("상면 표고 %1, 바닥 표고 %2, 깊이 %3 미터")
                        .arg(metres(number(top)), metres(number(bottom)), metres(depthOf(top, bottom, depth))));
  update();
}

QString KaElevationFigure::level(const QString& text) {
  return QStringLiteral("EL. ") + metres(number(text));
}

void KaElevationFigure::paintEvent(QPaintEvent*) {
  QPainter p(this);
  p.setRenderHint(QPainter::Antialiasing);
  const QColor ink = palette().color(QPalette::WindowText);
  const QColor muted = palette().color(QPalette::PlaceholderText);
  QPen guide(kGuide, 1);
  guide.setDashPattern({3, 3});
  p.setPen(guide);
  p.drawLine(QPointF(8, 18.5), QPointF(196, 18.5));   // 상면
  p.drawLine(QPointF(76, 58.5), QPointF(196, 58.5));  // 바닥
  QPainterPath ground(QPointF(8, 18.5));  // the ground surface and the pit cut into it
  ground.lineTo(62, 18.5);
  ground.lineTo(76, 58.5);
  ground.lineTo(150, 58.5);
  ground.lineTo(164, 18.5);
  ground.lineTo(196, 18.5);
  p.setPen(QPen(ink, 1.25));
  p.setBrush(Qt::NoBrush);
  p.drawPath(ground);
  p.setPen(QPen(ink, 1));  // the depth bracket
  p.drawLine(QPointF(188.5, 18.5), QPointF(188.5, 58.5));
  p.drawLine(QPointF(185, 18.5), QPointF(192, 18.5));
  p.drawLine(QPointF(185, 58.5), QPointF(192, 58.5));
  QFont words = font();
  words.setPixelSize(10);
  QFont mono = words;
  mono.setFamilies(KaTheme::monoFontStack());
  mono.setStyleHint(QFont::Monospace);
  const auto caption = [&](const QPointF& at, const QString& word, const QString& figure) {
    p.setPen(muted);
    p.setFont(words);
    p.drawText(at, word);
    p.setFont(mono);
    p.drawText(at + QPointF(QFontMetricsF(words).horizontalAdvance(word), 0), figure);
  };
  caption(QPointF(8, 12), QStringLiteral("상면 "), level(m_top));
  caption(QPointF(80, 71), QStringLiteral("바닥 "), level(m_bottom));
  const auto depth = depthOf(m_top, m_bottom, m_depth);
  p.setPen(ink);
  p.setFont(mono);
  p.drawText(QPointF(198, 42), depth ? metres(depth) + QStringLiteral(" m") : QStringLiteral("—"));
}
