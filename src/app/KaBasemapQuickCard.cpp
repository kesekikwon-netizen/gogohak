#include "KaBasemapQuickCard.h"

#include "KaIcons.h"
#include "KaTheme.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QSignalBlocker>
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
  // 「조용한 도판」 (2차 보드 판 1): no box, icon or note — a small caption with a rule and three plain buttons.
  const bool plate = KaTheme::displayOptions().paperLook;
  setProperty("kaPlate", plate);
  auto* column = new QVBoxLayout(this);
  column->setContentsMargins(plate ? 0 : 12, 12, plate ? 0 : 12, plate ? 0 : 12);
  column->setSpacing(8);
  auto* head = new QHBoxLayout;
  head->setSpacing(8);
  auto* title = new QLabel(QStringLiteral("배경 지도"), this);
  title->setObjectName(QStringLiteral("basemapQuickTitle"));
  title->setProperty("kaPlate", plate);
  if (plate) {
    auto* rule = new QFrame(this);
    rule->setObjectName(QStringLiteral("basemapQuickRule"));
    rule->setFixedHeight(1);
    head->addWidget(title, 0);
    head->addWidget(rule, 1, Qt::AlignVCenter);
  } else {
    auto* glyph = new QLabel(this);
    glyph->setPixmap(KaIcons::glyphPixmap(QStringLiteral("layer"), KaTheme::tokens().ink, 16, devicePixelRatioF()));
    glyph->setFixedSize(16, 16);
    QFont font = title->font();
    font.setPixelSize(14);
    font.setBold(true);
    title->setFont(font);
    head->addWidget(glyph, 0, Qt::AlignVCenter);
    head->addWidget(title, 1);
  }
  column->addLayout(head);
  // One short sentence: at 12 px in the 272 px pane a longer note wrapped to a third line that
  // the footer's size hint (measured at a wider width) did not reserve, so it was cut off.
  m_note = new QLabel(QStringLiteral("조사를 열면 위성과 지적이 올라옵니다."), this);
  m_note->setObjectName(QStringLiteral("basemapQuickNote"));
  m_note->setWordWrap(true);
  m_note->setToolTip(QStringLiteral("아래 단추는 리본의 「배경 지도」와 같은 단추입니다."));
  column->addWidget(m_note);
  if (plate) {
    m_note->hide();
    title->setToolTip(m_note->text());
  }
  auto* row = new QHBoxLayout;
  row->setSpacing(plate ? 4 : 8);
  for (const Button& spec : kButtons) {
    const QString id = QString::fromLatin1(spec.id);
    auto* button = new QToolButton(this);
    button->setObjectName(QString::fromLatin1(spec.objectName));
    button->setText(QString::fromUtf8(spec.text));
    if (plate) {
      // 옛 지도 opens the old-map list, so it says so; readers still hear the plain name.
      button->setAccessibleName(button->text());
      if (id == QLatin1String("old_map")) button->setText(button->text() + QStringLiteral(" ▾"));
      button->setToolButtonStyle(Qt::ToolButtonTextOnly);
      button->setMinimumSize(0, 32);
      button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);  // the three share the row
    } else {
      button->setIcon(KaIcons::icon(QString::fromLatin1(spec.icon)));
      button->setIconSize(QSize(16, 16));
      button->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
      button->setMinimumSize(72, 34);
    }
    button->setCheckable(true);
    button->setFocusPolicy(Qt::StrongFocus);
    connect(button, &QToolButton::clicked, this, [this, id, button](bool nowChecked) {
      {
        const QSignalBlocker quiet(button);
        button->setChecked(!nowChecked);  // the window answers with setChecked()
      }
      emit basemapRequested(id);
    });
    row->addWidget(button, 1);
    m_buttons.insert(id, button);
  }
  column->addLayout(row);
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
