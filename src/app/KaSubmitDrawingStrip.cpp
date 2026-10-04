#include "KaSubmitDrawingStrip.h"

#include <QPainter>

#include <algorithm>

namespace {
struct RequiredDrawing { const char* rule; const char* name; };
constexpr RequiredDrawing kRequiredDrawings[] = {{"REQ_DRAWING_SURVEY_AREA", "조사구역도"},
                                                 {"REQ_DRAWING_SITE_LOCATION", "유적위치도"},
                                                 {"REQ_DRAWING_FEATURE_PLAN", "유구배치도"},
                                                 {"REQ_DRAWING_FEATURE_DETAIL", "개별유구실측도"},
                                                 {"REQ_DRAWING_SECTION", "층위도"}};
}  // namespace

KaSubmitDrawingStrip::KaSubmitDrawingStrip(QWidget* parent) : QWidget(parent) {
  setObjectName(QStringLiteral("submitDrawingStrip"));
  setFixedHeight(108);
}

void KaSubmitDrawingStrip::setResults(const QVector<CheckResult>& results) {
  QStringList present, missing;
  for (const RequiredDrawing& drawing : kRequiredDrawings) {
    const auto found = std::find_if(results.cbegin(), results.cend(),
                                    [&drawing](const CheckResult& r) { return r.id == QLatin1String(drawing.rule); });
    if (found != results.cend()) (found->passed ? present : missing) << QString::fromUtf8(drawing.name);
  }
  setProperty("present", present);
  setProperty("missing", missing);
  setVisible(!present.isEmpty() || !missing.isEmpty());  // no rule file: no drawing state to show
  update();
}

void KaSubmitDrawingStrip::paintEvent(QPaintEvent*) {
  const QColor desk(0xF0, 0xEE, 0xE6), ink(0x14, 0x14, 0x13), edge(0xC2, 0xC0, 0xB6), muted(0x5E, 0x5D, 0x59),
      caution(0x7A, 0x5A, 0x00);
  QPainter p(this);
  p.fillRect(rect(), desk);
  const QStringList present = property("present").toStringList();
  const QStringList missing = property("missing").toStringList();
  constexpr int kTileW = 88, kTileH = 62;
  const int count = int(std::size(kRequiredDrawings));
  const int gap = qMax(8, (width() - count * kTileW) / (count + 1));
  QFont font = p.font();
  for (int i = 0; i < count; ++i) {
    const QString name = QString::fromUtf8(kRequiredDrawings[i].name);
    const QRect tile(gap + i * (kTileW + gap), 10, kTileW, kTileH);
    const bool has = present.contains(name);
    if (has) p.fillRect(tile, Qt::white);
    p.setPen(has ? QPen(ink, 1) : QPen(edge, 1, Qt::DashLine));
    p.setBrush(Qt::NoBrush);
    p.drawRect(tile.adjusted(0, 0, -1, -1));
    font.setPixelSize(12);
    p.setFont(font);
    p.setPen(ink);
    p.drawText(QRect(tile.left() - 10, tile.bottom() + 4, kTileW + 20, 16), Qt::AlignCenter, name);
    if (!missing.contains(name)) continue;
    // 「없음」 with a caution triangle under the name.
    font.setPixelSize(11);
    p.setFont(font);
    const QRect status(tile.left(), tile.bottom() + 20, kTileW, 14);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(Qt::NoPen);
    p.setBrush(caution);
    const double cx = status.center().x() - 16, top = status.top() + 2;
    p.drawPolygon(QPolygonF({QPointF(cx, top), QPointF(cx + 5, top + 9), QPointF(cx - 5, top + 9)}));
    p.setRenderHint(QPainter::Antialiasing, false);
    p.setPen(muted);
    p.drawText(status.adjusted(10, 0, 0, 0), Qt::AlignCenter, QStringLiteral("없음"));
  }
}
