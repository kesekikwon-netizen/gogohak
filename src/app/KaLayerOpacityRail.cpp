#include "KaLayerOpacityRail.h"

#include <QEvent>
#include <QGridLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QSettings>
#include <QShowEvent>
#include <QSlider>
#include <QStyle>

namespace {
QString placementSettingsKey(const QString& placementKey) {
  return QStringLiteral("ui/opacityRail/%1").arg(placementKey);
}
}  // namespace

KaLayerOpacityRail::KaLayerOpacityRail(QWidget* host, const QString& placementKey)
    : QFrame(host), m_host(host), m_placementKey(placementKey) {
  setObjectName(QStringLiteral("layerOpacityRail"));
  setAttribute(Qt::WA_StyledBackground, true);
  setFocusPolicy(Qt::NoFocus);
  // Two horizontal rows, so the names read across instead of one letter per line.
  auto* lay = new QGridLayout(this);
  lay->setContentsMargins(12, 8, 12, 8);
  lay->setHorizontalSpacing(10);
  lay->setVerticalSpacing(6);

  // 맨 윗줄은 지금 조절하는 레이어 이름이다. 무엇을 바꾸는지 모른 채 밀지 않게 한다.
  m_target = new QLabel(this);
  m_target->setObjectName(QStringLiteral("layerOpacityTarget"));
  m_target->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  m_target->setTextFormat(Qt::PlainText);

  m_title = new QLabel(QStringLiteral("투명도"), this);
  m_title->setObjectName(QStringLiteral("layerOpacityTitle"));
  m_title->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

  m_slider = new QSlider(Qt::Horizontal, this);
  m_slider->setObjectName(QStringLiteral("layerOpacitySlider"));
  m_slider->setRange(0, 100);
  m_slider->setValue(100);
  m_slider->setEnabled(false);
  m_slider->setInvertedAppearance(false);
  m_slider->setTickPosition(QSlider::NoTicks);
  m_slider->setToolTip(QStringLiteral("선택한 레이어의 투명도. 오른쪽으로 밀면 진해집니다. 지도에서만 흐려지고 자료는 그대로입니다."));
  m_slider->setFocusPolicy(Qt::ClickFocus);

  m_value = new QLabel(QStringLiteral("-"), this);
  m_value->setObjectName(QStringLiteral("layerOpacityValue"));
  m_value->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  m_value->setMinimumWidth(36);

  // 밝기: 오래된 항공사진은 원판이 어두워 그대로는 지번·경계가 잘 안 보인다.
  m_brightTitle = new QLabel(QStringLiteral("밝기"), this);
  m_brightTitle->setObjectName(QStringLiteral("layerOpacityTitle"));
  m_brightTitle->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

  m_bright = new QSlider(Qt::Horizontal, this);
  m_bright->setObjectName(QStringLiteral("layerBrightnessSlider"));
  m_bright->setRange(-255, 255);
  m_bright->setValue(0);
  m_bright->setEnabled(false);
  m_bright->setTickPosition(QSlider::NoTicks);
  m_bright->setToolTip(QStringLiteral(
      "고른 그림의 밝기. 가운데가 원본입니다. 원본 파일은 바뀌지 않습니다."));
  m_bright->setFocusPolicy(Qt::ClickFocus);

  m_brightValue = new QLabel(QStringLiteral("-"), this);
  m_brightValue->setObjectName(QStringLiteral("layerOpacityValue"));
  m_brightValue->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  m_brightValue->setMinimumWidth(36);

  connect(m_bright, &QSlider::valueChanged, this, [this](int value) {
    if (m_brightValue && m_bright && m_bright->isEnabled())
      m_brightValue->setText(QString::number(value));
    emit brightnessChanged(value);
  });

  lay->addWidget(m_target, 0, 0, 1, 3);
  lay->addWidget(m_title, 1, 0);
  lay->addWidget(m_slider, 1, 1);
  lay->addWidget(m_value, 1, 2);
  lay->addWidget(m_brightTitle, 2, 0);
  lay->addWidget(m_bright, 2, 1);
  lay->addWidget(m_brightValue, 2, 2);
  lay->setColumnStretch(1, 1);

  connect(m_slider, &QSlider::valueChanged, this, [this](int value) {
    if (m_value && m_slider && m_slider->isEnabled())
      m_value->setText(QStringLiteral("%1%").arg(value));
    emit percentChanged(value);
  });

  // 2026-10-10 현장 요청: 「화면 맞춤」한 도면이나 지도를 가리면 빈 곳을 잡고 끌어 옮긴다.
  // 놓은 자리는 지도·도면 따로 기억한다. 슬라이더는 값만 바꾼다.
  setCursor(Qt::OpenHandCursor);
  m_slider->setCursor(Qt::ArrowCursor);
  m_bright->setCursor(Qt::ArrowCursor);
  setToolTip(QStringLiteral("빈 곳을 잡고 끌면 옮길 수 있습니다. 놓은 자리는 다음에도 그대로입니다."));
  if (!m_placementKey.isEmpty()) {
    const QVariant saved = QSettings().value(placementSettingsKey(m_placementKey));
    m_placed = saved.isValid();
    m_placedPos = saved.toPoint();
  }

  setTarget(QString(), false);
  if (m_host) {
    m_host->installEventFilter(this);
    reposition();
    raise();
  }
}

void KaLayerOpacityRail::setTarget(const QString& layerName, bool adjustable) {
  // 조절할 그림이 없으면 「-」 막대 두 줄을 띄워 두지 않고 안내 한 줄로 접는다.
  m_adjustable = adjustable;
  if (m_target) {
    const QString text = adjustable && !layerName.isEmpty()
                             ? layerName
                             : QStringLiteral("투명도·밝기: 조절할 레이어를 고르세요");
    m_target->setText(fontMetrics().elidedText(text, Qt::ElideMiddle, 236));
    m_target->setToolTip(adjustable ? layerName : QString());
    m_target->setProperty("idle", !adjustable);
    m_target->style()->unpolish(m_target);
    m_target->style()->polish(m_target);
  }
  for (QWidget* w : {static_cast<QWidget*>(m_title), static_cast<QWidget*>(m_slider),
                     static_cast<QWidget*>(m_value), static_cast<QWidget*>(m_brightTitle),
                     static_cast<QWidget*>(m_bright), static_cast<QWidget*>(m_brightValue)}) {
    if (w) w->setVisible(adjustable);
  }
  reposition();
}

void KaLayerOpacityRail::setPercent(int percent, bool enabled) {
  const int val = qBound(0, percent, 100);
  if (m_slider) {
    m_slider->blockSignals(true);
    m_slider->setEnabled(enabled);
    m_slider->setValue(enabled ? val : 100);
    m_slider->blockSignals(false);
  }
  if (m_value) {
    m_value->setEnabled(enabled);
    m_value->setText(enabled ? QStringLiteral("%1%").arg(val) : QStringLiteral("-"));
  }
  if (m_title)
    m_title->setEnabled(enabled);
}

int KaLayerOpacityRail::percent() const {
  return m_slider ? m_slider->value() : 100;
}

void KaLayerOpacityRail::setBrightness(int value, bool enabled) {
  const int val = qBound(-255, value, 255);
  if (m_bright) {
    m_bright->blockSignals(true);
    m_bright->setEnabled(enabled);
    m_bright->setValue(enabled ? val : 0);
    m_bright->blockSignals(false);
  }
  if (m_brightValue) {
    // 벡터(지적도·유적 경계)에는 밝기가 없다. 「-」는 값을 못 읽은 것처럼 보였다.
    m_brightValue->setEnabled(enabled);
    m_brightValue->setText(enabled ? QString::number(val) : QStringLiteral("그림만"));
    m_brightValue->setToolTip(enabled ? QString()
                                      : QStringLiteral("밝기는 위성·항공사진 같은 그림 레이어에만 있습니다."));
  }
  if (m_brightTitle)
    m_brightTitle->setEnabled(enabled);
}

int KaLayerOpacityRail::brightness() const {
  return m_bright ? m_bright->value() : 0;
}

bool KaLayerOpacityRail::eventFilter(QObject* watched, QEvent* event) {
  if (watched == m_host && event &&
      (event->type() == QEvent::Resize || event->type() == QEvent::Show ||
       event->type() == QEvent::LayoutRequest)) {
    reposition();
  }
  return QFrame::eventFilter(watched, event);
}

void KaLayerOpacityRail::showEvent(QShowEvent* event) {
  QFrame::showEvent(event);
  reposition();
}

void KaLayerOpacityRail::setTopInset(int px) {
  const int inset = qMax(0, px);
  if (inset == m_topInset)
    return;
  m_topInset = inset;
  reposition();
}

void KaLayerOpacityRail::reposition() {
  if (!m_host)
    return;
  if (auto* lay = layout())
    lay->activate();
  resize(260, m_adjustable ? qMax(88, sizeHint().height()) : qMax(34, sizeHint().height()));
  move(m_placed ? keptInside(m_placedPos) : QPoint(10, 10 + m_topInset));
  raise();
}

QPoint KaLayerOpacityRail::keptInside(const QPoint& wanted) const {
  // 창이 줄거나 그리기 안내 띠가 떠도 카드 전체가 보이게 당겨 온다.
  const int top = m_topInset > 0 ? m_topInset + 10 : 0;
  return {qBound(0, wanted.x(), qMax(0, m_host->width() - width())),
          qBound(top, wanted.y(), qMax(top, m_host->height() - height()))};
}

void KaLayerOpacityRail::mousePressEvent(QMouseEvent* event) {
  if (event->button() != Qt::LeftButton || !m_host) {
    QFrame::mousePressEvent(event);
    return;
  }
  m_dragging = true;
  m_grab = event->position().toPoint();
  setCursor(Qt::ClosedHandCursor);
  event->accept();
}

void KaLayerOpacityRail::mouseMoveEvent(QMouseEvent* event) {
  if (!m_dragging || !(event->buttons() & Qt::LeftButton)) {
    QFrame::mouseMoveEvent(event);
    return;
  }
  m_placed = true;
  m_placedPos = mapToParent(event->position().toPoint()) - m_grab;
  reposition();
  m_placedPos = pos();  // remember the place actually shown
  event->accept();
}

void KaLayerOpacityRail::mouseReleaseEvent(QMouseEvent* event) {
  if (!m_dragging || event->button() != Qt::LeftButton) {
    QFrame::mouseReleaseEvent(event);
    return;
  }
  m_dragging = false;
  setCursor(Qt::OpenHandCursor);
  if (m_placed && !m_placementKey.isEmpty())
    QSettings().setValue(placementSettingsKey(m_placementKey), m_placedPos);
  event->accept();
}
