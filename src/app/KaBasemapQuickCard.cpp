#include "KaBasemapQuickCard.h"

#include "KaIcons.h"
#include "KaTheme.h"

#include <QEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QSignalBlocker>
#include <QStyle>
#include <QToolButton>
#include <QVBoxLayout>

namespace {
struct Button {
  const char* id;
  const char* objectName;
  const char* text;
  const char* icon;
};
constexpr Button kButtons[] = {
    {"satellite", "basemapQuickSatellite", "위성", "satellite"},
    {"terrain", "basemapQuickTerrain", "지형", "contour"},
    {"old_map", "basemapQuickOldMap", "옛 지도", "old_map"},
};
}  // namespace

KaBasemapQuickCard::KaBasemapQuickCard(QWidget* parent) : QFrame(parent) {
  setObjectName(QStringLiteral("basemapQuickCard"));
  m_column = new QVBoxLayout(this);
  m_column->setSpacing(8);
  m_head = new QHBoxLayout;
  m_head->setSpacing(8);
  m_glyph = new QLabel(this);
  m_glyph->setPixmap(KaIcons::glyphPixmap(QStringLiteral("layer"), KaTheme::tokens().ink, 16, devicePixelRatioF()));
  m_glyph->setFixedSize(16, 16);
  m_title = new QLabel(QStringLiteral("배경 지도"), this);
  m_title->setObjectName(QStringLiteral("basemapQuickTitle"));
  QFont font = m_title->font();
  font.setPixelSize(14);
  font.setBold(true);
  m_title->setFont(font);
  m_rule = new QFrame(this);
  m_rule->setObjectName(QStringLiteral("basemapQuickRule"));
  m_rule->setFixedHeight(1);
  m_head->addWidget(m_glyph, 0, Qt::AlignVCenter);
  m_head->addWidget(m_title, 1);
  m_head->addWidget(m_rule, 1, Qt::AlignVCenter);
  m_column->addLayout(m_head);
  // One short sentence: at 12 px in the 272 px pane a longer note wrapped to a third line that
  // the footer's size hint (measured at a wider width) did not reserve, so it was cut off.
  m_note = new QLabel(QStringLiteral("조사를 열면 위성과 지적이 올라옵니다."), this);
  m_note->setObjectName(QStringLiteral("basemapQuickNote"));
  m_note->setWordWrap(true);
  m_note->setToolTip(QStringLiteral("아래 단추는 리본의 「배경 지도」와 같은 단추입니다."));
  m_column->addWidget(m_note);
  m_row = new QHBoxLayout;
  for (const Button& spec : kButtons) {
    const QString id = QString::fromLatin1(spec.id);
    auto* button = new QToolButton(this);
    button->setObjectName(QString::fromLatin1(spec.objectName));
    button->setIconSize(QSize(16, 16));
    button->setCheckable(true);
    button->setFocusPolicy(Qt::StrongFocus);
    connect(button, &QToolButton::clicked, this, [this, id, button](bool nowChecked) {
      {
        const QSignalBlocker quiet(button);
        button->setChecked(!nowChecked);  // the window answers with setChecked()
      }
      emit basemapRequested(id);
    });
    m_row->addWidget(button, 1);
    m_buttons.insert(id, button);
  }
  m_stockPolicy = m_buttons.value(QStringLiteral("satellite"))->sizePolicy();
  m_column->addLayout(m_row);
  applyLook();
}

// 「조용한 도판」 (2차 보드 판 1): no box, icon or note — a small caption with a rule and three plain buttons that
// share the row. Called again when 「새 모양」 is switched while the app runs.
void KaBasemapQuickCard::applyLook() {
  const bool plate = KaTheme::displayOptions().paperLook;
  m_plate = plate;
  m_column->setContentsMargins(plate ? 0 : 12, 12, plate ? 0 : 12, plate ? 0 : 12);
  m_row->setSpacing(plate ? 4 : 8);
  m_glyph->setVisible(!plate);
  m_rule->setVisible(plate);
  m_head->setStretchFactor(m_title, plate ? 0 : 1);
  m_note->setVisible(!plate);
  m_title->setToolTip(plate ? m_note->text() : QString());
  for (const Button& spec : kButtons) {
    QToolButton* button = m_buttons.value(QString::fromLatin1(spec.id));
    const QString name = QString::fromUtf8(spec.text);
    // 옛 지도 opens the old-map list, so it says so in 새 모양; readers still hear the plain name.
    const bool list = plate && qstrcmp(spec.id, "old_map") == 0;
    button->setText(list ? name + QStringLiteral(" ▾") : name);
    button->setAccessibleName(plate ? name : QString());
    button->setIcon(plate ? QIcon() : KaIcons::icon(QString::fromLatin1(spec.icon)));
    button->setToolButtonStyle(plate ? Qt::ToolButtonTextOnly : Qt::ToolButtonTextBesideIcon);
    button->setMinimumSize(plate ? QSize(0, 32) : QSize(72, 34));
    button->setSizePolicy(plate ? QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed) : m_stockPolicy);
  }
  for (QWidget* plated : std::initializer_list<QWidget*>{this, m_title}) {
    plated->setProperty("kaPlate", plate);
    plated->style()->unpolish(plated);
    plated->style()->polish(plated);
  }
}

void KaBasemapQuickCard::changeEvent(QEvent* event) {
  QFrame::changeEvent(event);
  if (event->type() == QEvent::StyleChange && m_plate != KaTheme::displayOptions().paperLook) applyLook();
}

QStringList KaBasemapQuickCard::ids() {
  QStringList list;
  for (const Button& spec : kButtons) list << QString::fromLatin1(spec.id);
  return list;
}

QToolButton* KaBasemapQuickCard::button(const QString& id) const { return m_buttons.value(id, nullptr); }

bool KaBasemapQuickCard::isChecked(const QString& id) const {
  const QToolButton* button = m_buttons.value(id, nullptr);
  return button && button->isChecked();
}

QString KaBasemapQuickCard::sentence() const { return m_note->text(); }

void KaBasemapQuickCard::setChecked(const QString& id, bool on) {
  QToolButton* button = m_buttons.value(id, nullptr);
  if (!button || button->isChecked() == on) return;
  const QSignalBlocker quiet(button);
  button->setChecked(on);
}
