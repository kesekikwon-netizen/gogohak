#include "KaTilePreview.h"

#include <QApplication>
#include <QLineF>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>

#include <algorithm>

namespace {
QSizeF portraitMm(const QPageSize& size) {
  const QSizeF s = size.size(QPageSize::Millimeter);
  return s.width() <= s.height() ? s : s.transposed();
}

// 「조용한 도판」 colours of the board's preview (KaThemePaper.cpp: rail, ink, muted; the hatch is 「선 진함」).
const QColor kDesk(0xF0, 0xEE, 0xE6);
const QColor kInk(0x14, 0x14, 0x13);
const QColor kMuted(0x5E, 0x5D, 0x59);
const QColor kHatch(0xB0, 0xAE, 0xA5);
const QColor kEdge(0xDE, 0xDC, 0xD1);

// Text on a baseline with a halo, so the number reads over the drawing.
void haloText(QPainter& p, const QPointF& at, const QString& text, const QFont& font, const QColor& ink,
              const QColor& halo) {
  QPainterPath path;
  path.addText(at, font, text);
  p.strokePath(path, QPen(halo, 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
  p.fillPath(path, ink);
}
}  // namespace

KaTilePreview::KaTilePreview(QWidget* parent)
    : QWidget(parent), m_plate(qApp && qApp->property("kaPaperLook").toBool()) {
  setObjectName(QStringLiteral("printPreview"));
  setMinimumSize(360, 300);
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void KaTilePreview::setContent(const QImage& thumb, const TilePlan& plan, const QSet<int>& skipped, bool tiled,
                               const QSizeF& drawingMm, const QPageSize& sheet, const TileFit& fit) {
  m_thumb = thumb;
  m_plan = plan;
  m_skipped = skipped;
  m_tiled = tiled && plan.ok;
  m_drawingMm = drawingMm;
  m_sheet = sheet;
  m_fit = fit;
  setCursor(m_tiled && plan.sheets.size() > 1 ? Qt::PointingHandCursor : Qt::ArrowCursor);
  update();
}

int KaTilePreview::sheetAt(const QPointF& pos) const {
  if (!m_tiled) return -1;
  double k = 1.0;
  QPointF origin;
  layoutTiles(&k, &origin);
  int best = -1;
  double bestDistance = 1e18;
  for (int i = 0; i < m_plan.sheets.size(); ++i) {
    const QRectF r = mapped(m_plan.sheets.at(i).areaMm, k, origin);
    if (!r.contains(pos)) continue;
    const double distance = QLineF(r.center(), pos).length();
    if (distance < bestDistance) {
      bestDistance = distance;
      best = i;
    }
  }
  return best;
}

void KaTilePreview::paintEvent(QPaintEvent*) {
  QPainter p(this);
  p.setRenderHint(QPainter::Antialiasing);
  p.setRenderHint(QPainter::SmoothPixmapTransform);
  p.fillRect(rect(), m_plate ? kDesk : QColor(244, 244, 245));
  if (m_plate) {  // the board's hairline round the preview
    p.setPen(QPen(kEdge, 1));
    p.drawRect(QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5));
  }
  if (m_thumb.isNull() || m_drawingMm.isEmpty()) {
    if (m_plate) p.setPen(kMuted);
    p.drawText(rect(), Qt::AlignCenter, QStringLiteral("미리보기를 만들지 못했습니다."));
    return;
  }
  if (m_tiled) paintTiles(p);
  else paintSheet(p);
}

void KaTilePreview::mousePressEvent(QMouseEvent* event) {
  if (!m_tiled || m_plan.sheets.size() < 2 || event->button() != Qt::LeftButton) return;
  const int index = sheetAt(event->position());
  if (index >= 0 && onToggle) onToggle(index);
}

QRectF KaTilePreview::box() const { return QRectF(rect()).adjusted(10, 10, -10, -10); }

void KaTilePreview::layoutTiles(double* k, QPointF* origin) const {
  QRectF extent(QPointF(0, 0), m_plan.outputMm);
  for (const TileSheet& tile : m_plan.sheets) extent = extent.united(tile.areaMm);
  const QRectF b = box();
  *k = std::min(b.width() / extent.width(), b.height() / extent.height());
  *origin = QPointF(b.center().x() - (extent.x() + extent.width() / 2.0) * *k,
                    b.center().y() - (extent.y() + extent.height() / 2.0) * *k);
}

QRectF KaTilePreview::mapped(const QRectF& mm, double k, const QPointF& origin) {
  return QRectF(origin.x() + mm.x() * k, origin.y() + mm.y() * k, mm.width() * k, mm.height() * k);
}

void KaTilePreview::paintTiles(QPainter& p) const {
  double k = 1.0;
  QPointF origin;
  layoutTiles(&k, &origin);
  if (m_plate) {
    paintPlateTiles(p, k, origin);
    return;
  }
  const QRectF drawing = mapped(QRectF(QPointF(0, 0), m_plan.outputMm), k, origin);
  p.fillRect(drawing, Qt::white);
  p.drawImage(drawing, m_thumb);
  QFont font = p.font();
  font.setBold(true);
  font.setPixelSize(13);
  p.setFont(font);
  for (int i = 0; i < m_plan.sheets.size(); ++i) {
    const QRectF r = mapped(m_plan.sheets.at(i).areaMm, k, origin);
    const bool skipped = m_skipped.contains(i);
    if (skipped) {
      p.setPen(Qt::NoPen);
      p.setBrush(QColor(110, 110, 110, 150));
      p.drawRect(r);
    }
    p.setPen(QPen(skipped ? QColor(110, 110, 110) : QColor(37, 99, 235), 1.5));
    p.setBrush(skipped ? QBrush(Qt::NoBrush) : QBrush(QColor(37, 99, 235, 14)));
    p.drawRect(r);
    const QString label = skipped ? QStringLiteral("%1 뺌").arg(i + 1) : QString::number(i + 1);
    const QSizeF size = QFontMetricsF(font).size(Qt::TextSingleLine, label) + QSizeF(10, 4);
    const QRectF chip(r.center().x() - size.width() / 2.0, r.center().y() - size.height() / 2.0,
                      size.width(), size.height());
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(255, 255, 255, 225));
    p.drawRoundedRect(chip, 4, 4);
    p.setPen(skipped ? QColor(90, 90, 90) : QColor(30, 64, 175));
    p.drawText(chip, Qt::AlignCenter, label);
  }
}

// 2차 보드 판 5: white sheets on the desk with the drawing over them, a skipped sheet washed back to the desk,
// the overlaps hatched, ink sheet edges (a skipped sheet dashed) and mono numbers in each top-left corner.
void KaTilePreview::paintPlateTiles(QPainter& p, double k, const QPointF& origin) const {
  QList<QRectF> sheets;
  for (const TileSheet& tile : m_plan.sheets) sheets.append(mapped(tile.areaMm, k, origin));
  for (const QRectF& r : sheets) p.fillRect(r, Qt::white);
  p.drawImage(mapped(QRectF(QPointF(0, 0), m_plan.outputMm), k, origin), m_thumb);
  QColor wash = kDesk;
  wash.setAlphaF(0.8);
  for (int i : m_skipped)
    if (i >= 0 && i < sheets.size()) p.fillRect(sheets.at(i), wash);
  QPixmap hatch(4, 4);  // 45° lines every 4 px
  hatch.fill(Qt::white);
  {
    QPainter h(&hatch);
    h.setPen(kHatch);
    for (int d = 0; d < 4; ++d) h.drawPoint(3 - d, d);
  }
  for (int i = 0; i < sheets.size(); ++i)
    for (int j = i + 1; j < sheets.size(); ++j) {
      const QRectF overlap = sheets.at(i) & sheets.at(j);
      if (overlap.width() > 0 && overlap.height() > 0) p.fillRect(overlap, QBrush(hatch));
    }
  p.setBrush(Qt::NoBrush);
  p.setRenderHint(QPainter::Antialiasing, false);
  p.setPen(QPen(kHatch, 1, Qt::DashLine));
  for (int i : m_skipped)
    if (i >= 0 && i < sheets.size()) p.drawRect(sheets.at(i).adjusted(0, 0, -1, -1));
  p.setPen(QPen(kInk, 1));
  for (int i = 0; i < sheets.size(); ++i)
    if (!m_skipped.contains(i)) p.drawRect(sheets.at(i).adjusted(0, 0, -1, -1));
  p.setRenderHint(QPainter::Antialiasing);
  QFont mono(QStringLiteral("Consolas"));
  mono.setStyleHint(QFont::Monospace);
  mono.setPixelSize(11);
  QFont words = p.font();
  words.setPixelSize(11);
  for (int i = 0; i < sheets.size(); ++i) {
    const bool skipped = m_skipped.contains(i);
    const QPointF at = sheets.at(i).topLeft() + QPointF(12, 20);
    const QString number = QString::number(i + 1);
    haloText(p, at, number, mono, skipped ? kMuted : kInk, Qt::white);
    if (skipped)
      haloText(p, at + QPointF(QFontMetricsF(mono).horizontalAdvance(number) + 4, 0),
               QStringLiteral("· 이번에는 뺌"), words, kMuted, kDesk);
  }
}

void KaTilePreview::paintSheet(QPainter& p) const {
  QSizeF paper = portraitMm(m_sheet);
  if (m_fit.landscape) paper.transpose();
  const QRectF b = box();
  const double k = std::min(b.width() / paper.width(), b.height() / paper.height());
  const QRectF page(b.center().x() - paper.width() * k / 2.0, b.center().y() - paper.height() * k / 2.0,
                    paper.width() * k, paper.height() * k);
  p.fillRect(page, Qt::white);
  p.setPen(QPen(m_plate ? kInk : QColor(120, 120, 120), 1));
  p.drawRect(page);
  const QSizeF drawn = m_drawingMm * m_fit.ratio * k;
  p.drawImage(QRectF(page.center().x() - drawn.width() / 2.0, page.center().y() - drawn.height() / 2.0,
                     drawn.width(), drawn.height()),
              m_thumb);
}
