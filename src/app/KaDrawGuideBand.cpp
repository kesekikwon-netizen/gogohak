#include "app/KaDrawGuideBand.h"

#include "app/KaIcons.h"
#include "app/KaTheme.h"

#include <QAction>
#include <QEvent>
#include <QFrame>
#include <QHBoxLayout>
#include <QHideEvent>
#include <QLabel>
#include <QShowEvent>
#include <QStyle>
#include <QToolButton>

namespace {
constexpr int kGlyphPx = 20;
constexpr int kButtonIconPx = 20;
}  // namespace

KaDrawGuideBand::KaDrawGuideBand(QWidget* host) : QWidget(host), m_host(host) {
  setObjectName(QStringLiteral("drawGuideBand"));
  setFocusPolicy(Qt::NoFocus);
  auto* row = new QHBoxLayout(this);
  row->setContentsMargins(0, 0, 0, 0);
  row->setSpacing(kButtonGap);

  m_pill = new QFrame(this);
  m_pill->setObjectName(QStringLiteral("drawGuidePill"));
  m_pill->setAttribute(Qt::WA_StyledBackground, true);
  m_pill->setFixedHeight(kHeight);
  auto* pillRow = new QHBoxLayout(m_pill);
  pillRow->setContentsMargins(10, 0, 6, 0);
  pillRow->setSpacing(6);
  m_glyph = new QLabel(m_pill);
  m_glyph->setObjectName(QStringLiteral("drawGuideGlyph"));
  m_glyph->setFixedSize(kGlyphPx, kGlyphPx);
  m_glyph->setAlignment(Qt::AlignCenter);
  m_title = new QLabel(m_pill);
  m_title->setObjectName(QStringLiteral("drawGuideTitle"));
  m_title->setTextFormat(Qt::PlainText);
  m_hint = new QLabel(m_pill);
  m_hint->setObjectName(QStringLiteral("drawGuideHint"));
  m_hint->setTextFormat(Qt::PlainText);
  pillRow->addWidget(m_glyph);
  pillRow->addWidget(m_title);
  pillRow->addWidget(m_hint);
  m_measure = new QLabel(m_pill);
  m_measure->setObjectName(QStringLiteral("drawGuideMeasure"));
  m_measure->setTextFormat(Qt::PlainText);
  m_measure->hide();
  pillRow->addWidget(m_measure);
  row->addWidget(m_pill);

  // 되돌리기 · 다시 실행: two round buttons in the stock look; in 새 모양 one cover with a rule between (보완 M6).
  m_pair = new QFrame(this);
  m_pair->setObjectName(QStringLiteral("drawGuideUndoPair"));
  m_pair->setAttribute(Qt::WA_StyledBackground, true);
  m_pairRow = new QHBoxLayout(m_pair);
  m_pairRow->setContentsMargins(0, 0, 0, 0);
  m_undo = makeRoundButton(QStringLiteral("drawGuideUndo"), QStringLiteral("undo"), QStringLiteral("되돌리기 (Ctrl+Z)"));
  m_redo = makeRoundButton(QStringLiteral("drawGuideRedo"), QStringLiteral("redo"), QStringLiteral("다시 실행 (Ctrl+Y)"));
  m_divider = new QFrame(m_pair);
  m_divider->setObjectName(QStringLiteral("drawGuideDivider"));
  m_divider->setFixedWidth(1);
  m_pairRow->addWidget(m_undo);
  m_pairRow->addWidget(m_divider);
  m_pairRow->addWidget(m_redo);
  row->addWidget(m_pair);
  applyLook();

  setTool(QString(), QString(), QString());
  if (m_host) {
    m_host->installEventFilter(this);
    relayout();
  }
}

QToolButton* KaDrawGuideBand::makeRoundButton(const QString& objectName, const QString& iconId, const QString& tip) {
  auto* button = new QToolButton(m_pair);
  button->setObjectName(objectName);
  button->setFixedSize(kHeight, kHeight);
  button->setIconSize(QSize(kButtonIconPx, kButtonIconPx));
  button->setToolButtonStyle(Qt::ToolButtonIconOnly);
  button->setAutoRaise(false);
  button->setCursor(Qt::PointingHandCursor);
  button->setIcon(KaIcons::icon(iconId));
  button->setToolTip(tip);
  button->setEnabled(false);  // until the window hands over its action
  return button;
}

void KaDrawGuideBand::applyLook() {
  m_plate = KaTheme::displayOptions().paperLook;
  m_pair->setProperty("kaPlate", m_plate);
  m_pairRow->setSpacing(m_plate ? 0 : kButtonGap);
  m_divider->setVisible(m_plate);
  // The cover is 73 x 36 with its 1 px edge (QSS): two 35 x 34 halves and the 1 px rule inside.
  const QSize half = m_plate ? QSize(kHeight - 1, kHeight - 2) : QSize(kHeight, kHeight);
  m_undo->setFixedSize(half);
  m_redo->setFixedSize(half);
  m_pair->style()->unpolish(m_pair);
  m_pair->style()->polish(m_pair);
  if (m_host) relayout();
}

void KaDrawGuideBand::changeEvent(QEvent* event) {
  QWidget::changeEvent(event);
  if (event->type() == QEvent::StyleChange && m_pair && m_plate != KaTheme::displayOptions().paperLook) applyLook();
}

void KaDrawGuideBand::bindAction(QToolButton* button, QAction* action, const QString& iconId) {
  if (!button) return;
  if (!action) {
    button->setEnabled(false);
    return;
  }
  button->setDefaultAction(action);
  button->setToolButtonStyle(Qt::ToolButtonIconOnly);
  // setDefaultAction copies the action's icon on every change; an action without one
  // would leave the round button blank, so the outline glyph is put back after each.
  auto keepGlyph = [button, iconId]() {
    if (button->icon().isNull()) button->setIcon(KaIcons::icon(iconId));
  };
  keepGlyph();
  QObject::connect(action, &QAction::changed, button, keepGlyph);
}

void KaDrawGuideBand::setMeasure(const QString& text) {
  const bool show = !text.isEmpty() && KaTheme::displayOptions().paperLook;
  if (text == m_measure->text() && show == !m_measure->isHidden()) return;  // called on every mouse move
  m_measure->setText(text);
  m_measure->setVisible(show);
  relayout();
}

void KaDrawGuideBand::setActions(QAction* undo, QAction* redo) {
  bindAction(m_undo, undo, QStringLiteral("undo"));
  bindAction(m_redo, redo, QStringLiteral("redo"));
}

void KaDrawGuideBand::setTool(const QString& iconId, const QString& title, const QString& hint) {
  m_iconId = iconId;
  m_titleText = title.trimmed();
  m_hintText = hint.trimmed();
  const bool has = hasTool();
  m_title->setText(m_titleText);
  m_hint->setText(m_hintText.isEmpty() ? QString() : QStringLiteral("— %1").arg(m_hintText));
  m_hint->setVisible(!m_hintText.isEmpty());
  QPixmap glyph;
  if (has && !m_iconId.isEmpty() && KaIcons::hasIcon(m_iconId))
    glyph = KaIcons::glyphPixmap(m_iconId, KaTheme::tokens().tabInk, kGlyphPx, devicePixelRatioF());
  m_glyph->setPixmap(glyph);
  m_glyph->setVisible(!glyph.isNull());
  m_pill->setVisible(has);
  relayout();
  emitInsetIfChanged();
}

int KaDrawGuideBand::topInset() const { return hasTool() && !isHidden() ? kInsetWithTool : 0; }

void KaDrawGuideBand::emitInsetIfChanged() {
  const int inset = topInset();
  if (inset == m_lastInset) return;
  m_lastInset = inset;
  emit topInsetChanged(inset);
}

void KaDrawGuideBand::relayout() {
  if (auto* lay = layout()) lay->activate();
  resize(sizeHint());
  int x = hasTool() ? kMargin : kFoldedLeft;
  if (m_host && m_host->width() > 0) x = qMax(kMargin, qMin(x, m_host->width() - width() - kMargin));
  move(x, kMargin);
  raise();
}

bool KaDrawGuideBand::eventFilter(QObject* watched, QEvent* event) {
  if (watched == m_host && event &&
      (event->type() == QEvent::Resize || event->type() == QEvent::Show || event->type() == QEvent::LayoutRequest)) {
    relayout();
  }
  return QWidget::eventFilter(watched, event);
}

void KaDrawGuideBand::showEvent(QShowEvent* event) {
  QWidget::showEvent(event);
  relayout();
  emitInsetIfChanged();
}

void KaDrawGuideBand::hideEvent(QHideEvent* event) {
  QWidget::hideEvent(event);
  emitInsetIfChanged();
}
