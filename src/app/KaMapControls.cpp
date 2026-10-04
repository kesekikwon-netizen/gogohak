#include "KaMapControls.h"

#include "KaTheme.h"

#include <QEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include <QToolButton>
#include <QVBoxLayout>

#include <qgscoordinatereferencesystem.h>
#include <qgsmapcanvas.h>
#include <qgsmapsettings.h>

#include <cmath>

namespace {

enum class Glyph { Plus, Minus, Fit };

QIcon glyph(Glyph kind) {
  QIcon icon;
  for (qreal dpr : {1.0, 2.0}) {
    QPixmap pixmap(QSize(20, 20) * dpr);
    pixmap.setDevicePixelRatio(dpr);
    pixmap.fill(Qt::transparent);
    QPainter p(&pixmap);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(QPen(KaTheme::tokens().ink, 2.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    if (kind != Glyph::Fit) p.drawLine(QPointF(4, 10), QPointF(16, 10));
    if (kind == Glyph::Plus) p.drawLine(QPointF(10, 4), QPointF(10, 16));
    if (kind == Glyph::Fit) {
      QPainterPath corners;
      for (const QPointF& c : {QPointF(3, 3), QPointF(17, 3), QPointF(17, 17), QPointF(3, 17)}) {
        const double sx = c.x() < 10 ? 1 : -1, sy = c.y() < 10 ? 1 : -1;
        corners.moveTo(c + QPointF(0, 5 * sy));
        corners.lineTo(c);
        corners.lineTo(c + QPointF(5 * sx, 0));
      }
      p.drawPath(corners);
    }
    p.end();
    icon.addPixmap(pixmap);
  }
  return icon;
}

QToolButton* button(QWidget* parent, const char* name, Glyph kind, const QString& label,
                    const QString& tip) {
  auto* b = new QToolButton(parent);
  b->setObjectName(QString::fromLatin1(name));
  b->setIcon(glyph(kind));
  b->setIconSize(QSize(20, 20));
  // Icon-only: screen readers read this name, not the tooltip.
  b->setAccessibleName(label);
  b->setToolTip(tip);
  b->setFixedSize(36, 36);
  b->setFocusPolicy(Qt::NoFocus);
  return b;
}

QString lengthText(double metres) {
  if (metres >= 1000.0) return QStringLiteral("%1 km").arg(metres / 1000.0, 0, 'g', 3);
  return QStringLiteral("%1 m").arg(metres, 0, 'g', 3);
}

}  // namespace

KaMapControls::KaMapControls(QgsMapCanvas* canvas, QWidget* host)
    : QFrame(host), m_canvas(canvas), m_host(host) {
  setObjectName(QStringLiteral("mapControls"));
  setAccessibleName(QStringLiteral("지도 확대·축소"));
  setAttribute(Qt::WA_StyledBackground, true);
  auto* column = new QVBoxLayout(this);
  column->setContentsMargins(0, 0, 0, 0);
  column->setSpacing(0);
  auto* zoomIn = button(this, "mapZoomIn", Glyph::Plus, QStringLiteral("지도 확대"),
                        QStringLiteral("확대 (마우스 휠을 앞으로)"));
  auto* zoomOut = button(this, "mapZoomOut", Glyph::Minus, QStringLiteral("지도 축소"),
                         QStringLiteral("축소 (마우스 휠을 뒤로)"));
  auto* fit = button(this, "mapZoomFit", Glyph::Fit, QStringLiteral("조사구역 맞춤"),
                     QStringLiteral("조사구역이 화면에 꽉 차게 봅니다"));
  column->addWidget(zoomIn);
  column->addWidget(zoomOut);
  column->addWidget(fit);
  connect(zoomIn, &QToolButton::clicked, this, [this]() { if (m_canvas) m_canvas->zoomIn(); });
  connect(zoomOut, &QToolButton::clicked, this, [this]() { if (m_canvas) m_canvas->zoomOut(); });
  connect(fit, &QToolButton::clicked, this, [this]() {
    if (m_fit) m_fit();
    else if (m_canvas) m_canvas->zoomToFullExtent();
  });
  adjustSize();
  if (m_host) m_host->installEventFilter(this);
  reposition();
  raise();
}

bool KaMapControls::eventFilter(QObject* watched, QEvent* event) {
  if (watched == m_host && event && (event->type() == QEvent::Resize || event->type() == QEvent::Show))
    reposition();
  return QFrame::eventFilter(watched, event);
}

void KaMapControls::reposition() {
  if (!m_host) return;
  move(m_host->width() - width() - 12, 12);
  raise();
}

KaMapScaleBar::KaMapScaleBar(QgsMapCanvas* canvas, QWidget* host)
    : QWidget(host), m_canvas(canvas), m_host(host) {
  setObjectName(QStringLiteral("mapScaleBar"));
  setAccessibleName(QStringLiteral("축척 막대"));
  setAttribute(Qt::WA_TransparentForMouseEvents, true);
  setFixedHeight(34);
  if (m_canvas) {
    connect(m_canvas, &QgsMapCanvas::scaleChanged, this, &KaMapScaleBar::refresh);
    connect(m_canvas, &QgsMapCanvas::extentsChanged, this, &KaMapScaleBar::refresh);
    connect(m_canvas, &QgsMapCanvas::destinationCrsChanged, this, &KaMapScaleBar::refresh);
    connect(m_canvas, &QgsMapCanvas::rotationChanged, this, &KaMapScaleBar::refresh);
  }
  if (m_host) m_host->installEventFilter(this);
  refresh();
}

bool KaMapScaleBar::eventFilter(QObject* watched, QEvent* event) {
  if (watched == m_host && event && (event->type() == QEvent::Resize || event->type() == QEvent::Show))
    refresh();
  return QWidget::eventFilter(watched, event);
}

void KaMapScaleBar::changeEvent(QEvent* event) {
  QWidget::changeEvent(event);
  if (event->type() == QEvent::StyleChange) refresh();  // 새 모양 switched on or off: the 도북 arrow follows
}

void KaMapScaleBar::refresh() {
  if (!m_canvas || m_canvas->width() <= 0) return;
  const QgsCoordinateReferenceSystem crs = m_canvas->mapSettings().destinationCrs();
  const double extent = m_canvas->extent().width();
  if (!crs.isValid() || crs.isGeographic() || !(extent > 0.0)) {
    hide();
    return;
  }
  // Logical pixels, so the bar is right on high-DPI screens too.
  const double perPixel = extent / m_canvas->width();
  const double wanted = perPixel * 120.0;
  const double decade = std::pow(10.0, std::floor(std::log10(wanted)));
  double round = decade;
  for (double step : {2.0, 5.0, 10.0})
    if (step * decade <= wanted) round = step * decade;
  m_metres = round;
  m_pixels = round / perPixel;
  // 「조용한 도판」: a 도북 arrow (grid north, turned with the map) comes before the bar.
  m_north = KaTheme::displayOptions().paperLook;
  setAccessibleDescription(QStringLiteral("막대 길이 %1").arg(lengthText(m_metres)) +
                           (m_north ? QStringLiteral(" · 도북 화살표") : QString()));
  setFixedWidth(int(std::ceil(m_pixels)) + (m_north ? kPlateBarLeft + 30 : 56));
  reposition();
  show();
  update();
}

QStringList KaMapScaleBar::ticks(double metres) {
  const double end = metres >= 1000.0 ? metres / 1000.0 : metres;
  return {QStringLiteral("0"), QString::number(end / 2.0, 'g', 3), QString::number(end, 'g', 3)};
}

QString KaMapScaleBar::unit(double metres) {
  return metres >= 1000.0 ? QStringLiteral("km") : QStringLiteral("m");
}

void KaMapScaleBar::reposition() {
  if (!m_host) return;
  move(12, m_host->height() - height() - 12);
  raise();
}

void KaMapScaleBar::paintEvent(QPaintEvent*) {
  if (m_pixels <= 0.0) return;
  QPainter p(this);
  p.setRenderHint(QPainter::Antialiasing);
  if (m_north) {
    paintPlate(p);
    return;
  }
  const auto& tokens = KaTheme::tokens();
  p.setPen(Qt::NoPen);
  p.setBrush(QColor(255, 255, 255, 225));
  p.drawRoundedRect(QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5), 7, 7);
  const QRectF bar(10, 8, m_pixels, 6);
  // Two dark and two light quarters, as on a printed survey scale.
  for (int i = 0; i < 4; ++i) {
    const QRectF part(bar.left() + bar.width() * i / 4.0, bar.top(), bar.width() / 4.0, bar.height());
    p.fillRect(part, i % 2 ? QColor(Qt::white) : tokens.ink);
  }
  p.setPen(QPen(tokens.ink, 1));
  p.setBrush(Qt::NoBrush);
  p.drawRect(bar);
  QFont font = p.font();
  font.setPixelSize(11);
  p.setFont(font);
  p.drawText(QPointF(bar.left() - 2, bar.bottom() + 14), QStringLiteral("0"));
  const QString end = lengthText(m_metres);
  p.drawText(QPointF(bar.right() - 4, bar.bottom() + 14), end);
}

// 「조용한 도판」 (2차 보드 판 1): no box. 「▲ 도북」 first (a 12 x 20 arrow, left half ink, turned with the map),
// then the bar with 0 · middle · end over it in 11 px mono and the unit after the end. Every mark has a 3 px
// white halo so it reads on any map.
void KaMapScaleBar::paintPlate(QPainter& p) const {
  const auto& tokens = KaTheme::tokens();
  const qreal h = height();
  const QPen halo(Qt::white, 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
  const QPolygonF arrow({QPointF(0, -9.5), QPointF(5.5, 9.5), QPointF(0, 5.5), QPointF(-5.5, 9.5)});
  p.save();
  p.translate(kArrowMiddle, h - 11.5);
  p.rotate(m_canvas ? m_canvas->rotation() : 0.0);
  p.setPen(halo);
  p.setBrush(Qt::white);
  p.drawPolygon(arrow);
  p.setPen(Qt::NoPen);
  p.setBrush(tokens.ink);
  p.drawPolygon(QPolygonF({QPointF(0, -9.5), QPointF(-5.5, 9.5), QPointF(0, 5.5)}));
  p.setPen(QPen(tokens.ink, 1));
  p.setBrush(Qt::NoBrush);
  p.drawPolygon(arrow);
  p.restore();
  const auto haloText = [&p, &halo](const QPointF& at, const QString& text, const QColor& ink, const QFont& font) {
    QPainterPath path;
    path.addText(at, font, text);
    p.strokePath(path, halo);
    p.fillPath(path, ink);
  };
  QFont font = p.font();
  font.setPixelSize(11);
  haloText(QPointF(kArrowMiddle + 12, h - 2.5), QStringLiteral("도북"), tokens.ink, font);
  const QRectF bar(kPlateBarLeft + 0.5, h - 8.5, m_pixels, 6);
  p.setPen(halo);
  p.setBrush(Qt::white);
  p.drawRect(bar);
  // Two dark and two light quarters, as on a printed survey scale.
  for (int i = 0; i < 4; i += 2)
    p.fillRect(QRectF(bar.left() + bar.width() * i / 4.0, bar.top(), bar.width() / 4.0, bar.height()), tokens.ink);
  p.setPen(QPen(tokens.ink, 1));
  p.setBrush(Qt::NoBrush);
  p.drawRect(bar);
  QFont mono(QStringLiteral("Consolas"));
  mono.setStyleHint(QFont::Monospace);
  mono.setPixelSize(11);
  const QFontMetricsF fm(mono);
  const QStringList figures = ticks(m_metres);
  for (int i = 0; i < figures.size(); ++i)
    haloText(QPointF(bar.left() + bar.width() * i / 2.0 - fm.horizontalAdvance(figures.at(i)) / 2.0, bar.top() - 4.5),
             figures.at(i), tokens.ink, mono);
  // The unit a space after the end figure, however many digits it has.
  haloText(QPointF(bar.right() + fm.horizontalAdvance(figures.last()) / 2.0 + 4, bar.top() - 4.5), unit(m_metres),
           tokens.inkMuted, mono);
}

KaMapCornerBar::KaMapCornerBar(QWidget* host) : QFrame(host), m_host(host) {
  setObjectName(QStringLiteral("mapCornerBar"));
  setAttribute(Qt::WA_StyledBackground, true);
  if (m_host) m_host->installEventFilter(this);
}

bool KaMapCornerBar::eventFilter(QObject* watched, QEvent* event) {
  if (watched == m_host && event && (event->type() == QEvent::Resize || event->type() == QEvent::Show))
    reposition();
  return QFrame::eventFilter(watched, event);
}

bool KaMapCornerBar::event(QEvent* event) {
  const bool handled = QFrame::event(event);
  // Children showing or hiding (grid details) change the wanted width.
  if (event && (event->type() == QEvent::LayoutRequest || event->type() == QEvent::Show))
    reposition();
  return handled;
}

void KaMapCornerBar::reposition() {
  if (!m_host) return;
  if (auto* lay = layout()) lay->activate();
  resize(sizeHint());
  move(m_host->width() - width() - 12, m_host->height() - height() - 12);
  raise();
}
