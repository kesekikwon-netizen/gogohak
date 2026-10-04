// 「조용한 도판」 기록 카드 (2차 보드 판 3 B1): the shape's own swatch before the name, the record in one line under
// it, and one 「자동 계산」 head over the 면적·둘레 ledger. The stock look keeps the layer kind at the right.
#include "KaFeatureCard.h"
#include "KaIcons.h"
#include "KaSymbolSwatch.h"
#include "KaTheme.h"

#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QRegularExpression>

#include <qgsexpressioncontextutils.h>
#include <qgsrendercontext.h>
#include <qgsrenderer.h>
#include <qgssymbol.h>
#include <qgsvectorlayer.h>

namespace {

// The symbol this feature is drawn with on the map, 20 × 14 in a 1 px ink frame like the layer list's 유구 표지.
QPixmap featureSwatch(QgsVectorLayer* layer, const QgsFeature& feature) {
  QgsFeatureRenderer* renderer = layer ? layer->renderer() : nullptr;
  if (!renderer) return {};
  QgsRenderContext context;
  context.expressionContext() << QgsExpressionContextUtils::layerScope(layer);
  context.expressionContext().setFeature(feature);
  std::unique_ptr<QgsFeatureRenderer> copy(renderer->clone());  // rendering state stays off the layer's renderer
  copy->startRender(context, layer->fields());
  const QgsSymbolList symbols = copy->originalSymbolsForFeature(feature, context);
  const QPixmap patch = KaSymbolSwatch::framed(symbols.value(0));
  copy->stopRender(context);
  return patch;
}

// 「1616-1 과」, 「1620과」, 「산12-3 임」 → {지번, 지목}: the 지목 is the Hangul at the end.
std::pair<QString, QString> splitJibun(const QString& jibun) {
  const QRegularExpressionMatch m = QRegularExpression(QStringLiteral("^(.*?)\\s*(\\p{Hangul}+)$")).match(jibun.trimmed());
  if (!m.hasMatch() || m.captured(1).isEmpty()) return {jibun.trimmed(), QString()};
  return {m.captured(1), m.captured(2)};
}

}  // namespace

void KaFeatureCard::addLockedRows() {
  for (const auto& [label, slot, name] : {std::tuple{"지번", &m_lockedJibun, "kaLockedJibun"},
                                          std::tuple{"지목", &m_lockedJimok, "kaLockedJimok"}}) {
    auto* row = new QWidget(m_body);
    auto* line = new QHBoxLayout(row);
    line->setContentsMargins(0, 0, 0, 0);
    *slot = new QLabel(row);
    (*slot)->setObjectName(QString::fromLatin1(name));
    auto* lock = new QLabel(row);
    lock->setPixmap(KaIcons::glyphPixmap(QStringLiteral("lock"), KaTheme::tokens().inkMuted, 12, row->devicePixelRatioF()));
    lock->setToolTip(QStringLiteral("참조 자료라 고치지 않습니다"));
    line->addWidget(*slot, 1);
    line->addWidget(lock, 0, Qt::AlignVCenter);
    m_form->addRow(QString::fromUtf8(label), row);
  }
}

QString KaFeatureCard::fillLockedRows(const QgsFeature& feature) {
  if (!m_lockedJibun || !m_lockedJimok) return {};
  const auto [jibun, jimok] = splitJibun(feature.attribute(QStringLiteral("JIBUN")).toString());
  m_lockedJibun->setText(jibun);
  m_lockedJimok->setText(jimok.isEmpty() ? QStringLiteral("—") : jimok);
  return jibun;
}

void KaFeatureCard::showPlateHeader(const QgsFeature& feature, QStringList record) {
  const bool plate = KaTheme::displayOptions().paperLook;
  m_layerKind->setVisible(!plate);
  record.removeAll(QString());
  m_sub->setText(record.join(QStringLiteral(" · ")));
  m_sub->setVisible(plate);
  const QPixmap swatch = plate ? featureSwatch(m_layer, feature) : QPixmap();
  m_swatch->setPixmap(swatch);
  m_swatch->setVisible(!swatch.isNull());
}

QFrame* KaFeatureCard::measureHead(QWidget* parent) {
  auto* head = new QFrame(parent);
  head->setObjectName(QStringLiteral("kaMeasureHead"));
  auto* line = new QHBoxLayout(head);
  line->setContentsMargins(0, 0, 0, 4);
  line->setSpacing(4);
  auto* lock = new QLabel(head);
  lock->setPixmap(KaIcons::glyphPixmap(QStringLiteral("lock"), KaTheme::tokens().inkMuted, 12, head->devicePixelRatioF()));
  lock->setFixedSize(12, 12);
  auto* words = new QLabel(QStringLiteral("자동 계산"), head);
  words->setObjectName(QStringLiteral("kaMeasureAuto"));
  line->addStretch(1);
  line->addWidget(lock, 0, Qt::AlignVCenter);
  line->addWidget(words);
  head->setToolTip(QStringLiteral("도형에서 바로 잰 값이라 고칠 수 없습니다"));
  return head;
}
