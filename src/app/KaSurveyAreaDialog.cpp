#include "KaSurveyAreaDialog.h"
#include "KaStylePalette.h"
#include "KaTheme.h"
#include "core/LayerOps.h"

#include <QDoubleSpinBox>
#include <QFileInfo>
#include <QFormLayout>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QStyle>
#include <QVBoxLayout>
#include <cmath>
#include <qgsproviderregistry.h>
#include <qgsvectorlayer.h>

namespace {

double relativeLuminance(const QColor& color) {
  auto channel = [](double value) {
    return value <= 0.03928 ? value / 12.92 : std::pow((value + 0.055) / 1.055, 2.4);
  };
  return 0.2126 * channel(color.redF()) + 0.7152 * channel(color.greenF()) + 0.0722 * channel(color.blueF());
}

double contrastRatio(const QColor& a, const QColor& b) {
  const double la = relativeLuminance(a) + 0.05;
  const double lb = relativeLuminance(b) + 0.05;
  return la > lb ? la / lb : lb / la;
}

}  // namespace

QColor KaSurveyAreaDialog::readableTextOn(const QColor& fill) {
  for (const QColor& text : {QColor(Qt::white), KaTheme::tokens().ink})
    if (contrastRatio(fill, text) >= 4.5) return text;
  return QColor(Qt::black);
}

QgsVectorLayer* KaSurveyAreaDialog::layerToContinue(QgsProject* project, const QString& surveyPath, QgsMapLayer* current) {
  // Only this survey's own area tables: an outside file named 조사구역 opened for tracing would be
  // written on 저장, which must never touch the user's own data.
  const QString survey = QFileInfo(surveyPath).absoluteFilePath();
  QList<QgsVectorLayer*> own;
  for (QgsVectorLayer* layer : LayerOps::surveyAreaLayers(project)) {
    const QString key = LayerOps::layerKeyOf(layer);
    const QString path = QgsProviderRegistry::instance()
                             ->decodeUri(layer->providerType(), layer->source())
                             .value(QStringLiteral("path"))
                             .toString();
    if ((key == QLatin1String("survey_area") || key.startsWith(QLatin1String("survey_area_"))) && !surveyPath.isEmpty() &&
        QFileInfo(path).absoluteFilePath().compare(survey, Qt::CaseInsensitive) == 0)
      own << layer;
  }
  auto* picked = qobject_cast<QgsVectorLayer*>(current);
  return own.contains(picked) ? picked : own.value(0);  // the area picked in the list first
}

KaSurveyAreaDialog::KaSurveyAreaDialog(QWidget* parent) : QDialog(parent) {
  setWindowTitle(QStringLiteral("조사구역 그리기 설정"));
  setModal(true);

  m_paletteColors = KaStylePalette::colors();  // 첫 조사구역은 주황
  m_colorNames = KaStylePalette::names();
  setupUi();
  selectColorIndex(1);
}

void KaSurveyAreaDialog::setupUi() {
  // 새 모양 (2차 보드 판 5, 440): plain swatches in two rows with the chosen one ringed and named under its word,
  // the widths joined in one row and 「직접」 mm below; stock keeps its one row of named swatches.
  m_paper = KaTheme::displayOptions().paperLook;
  setObjectName(QStringLiteral("surveyAreaDialog"));
  setProperty("kaPlate", m_paper);
  if (m_paper) setMinimumWidth(440);
  auto* mainLayout = new QVBoxLayout(this);
  mainLayout->setSpacing(m_paper ? 24 : 12);
  mainLayout->setContentsMargins(m_paper ? 24 : 16, m_paper ? 24 : 16, m_paper ? 24 : 16, 16);
  auto* form = new QFormLayout();
  form->setHorizontalSpacing(m_paper ? 0 : 10);
  form->setVerticalSpacing(m_paper ? 12 : 10);

  m_nameEdit = new QLineEdit(QStringLiteral("조사구역"), this);
  form->addRow(QStringLiteral("이름"), m_nameEdit);

  auto* colorRow = new QHBoxLayout();
  colorRow->setSpacing(4);
  auto* colorGrid = new QGridLayout();  // 새 모양: four a row; the 4 px ring room on each side makes the 8 px gap
  colorGrid->setSpacing(0);
  for (int i = 0; i < m_paletteColors.size(); ++i) {
    auto* btn = new QPushButton(m_paper ? QString() : m_colorNames[i], this);
    btn->setObjectName(QStringLiteral("surveyAreaColor"));
    btn->setAutoDefault(false);  // Enter and the main-button paint stay with 「그리기 시작」
    btn->setFixedSize(m_paper ? QSize(52, 32) : QSize(52, 28));
    btn->setCursor(Qt::PointingHandCursor);
    btn->setAccessibleName(m_colorNames[i]);
    if (m_paper) btn->setToolTip(m_colorNames[i]);
    connect(btn, &QPushButton::clicked, this, [this, i]() { selectColorIndex(i); });
    m_colorButtons.append(btn);
    if (m_paper) colorGrid->addWidget(btn, i / 4, i % 4);
    else colorRow->addWidget(btn);
  }
  if (m_paper) {
    colorGrid->setColumnStretch(4, 1);
    auto* word = new QWidget(this);  // 「외곽선 색」 with the chosen colour's name under it
    auto* words = new QVBoxLayout(word);
    words->setContentsMargins(0, 4, 0, 0);
    words->setSpacing(14);
    auto* caption = new QLabel(QStringLiteral("외곽선 색"), word);
    caption->setProperty("kaFormWord", true);
    m_colorName = new QLabel(word);
    m_colorName->setObjectName(QStringLiteral("surveyAreaColorName"));
    words->addWidget(caption);
    words->addWidget(m_colorName);
    words->addStretch(1);
    word->setFixedWidth(96);
    form->addRow(word, colorGrid);
    delete colorRow;
  } else {
    delete colorGrid;
    form->addRow(QStringLiteral("외곽선 색"), colorRow);
  }

  auto* widthRow = new QHBoxLayout();
  widthRow->setSpacing(4);
  const QList<QPair<QString, double>> presets = {{QStringLiteral("보통 1.0"), 1.0}, {QStringLiteral("기본 1.5"), 1.5},
                                                 {QStringLiteral("굵게 2.0"), 2.0}, {QStringLiteral("강조 3.0"), 3.0}};
  if (m_paper) widthRow->setSpacing(0);  // joined like the board's group
  for (const auto& p : presets) {
    auto* btn = new QPushButton(p.first, this);
    btn->setFixedHeight(m_paper ? 32 : 28);
    btn->setAutoDefault(false);
    if (m_paper) {  // the QSS joins them: square inner edges, round outer ends
      btn->setObjectName(QStringLiteral("surveyAreaWidth"));
      btn->setProperty("kaJoin", m_widthPresetButtons.isEmpty()            ? QStringLiteral("first")
                                 : p.second == presets.last().second ? QStringLiteral("last")
                                                                     : QStringLiteral("middle"));
    } else {
      btn->setStyleSheet(QStringLiteral("QPushButton { min-height: 0px; padding: 2px 8px; }"));
    }
    btn->setCheckable(true);
    btn->setChecked(p.second == 1.5);
    connect(btn, &QPushButton::clicked, this, [this, w = p.second]() { selectPresetWidth(w); });
    m_widthPresetButtons.append(btn);
    widthRow->addWidget(btn);
  }
  m_widthSpin = new QDoubleSpinBox(this);
  m_widthSpin->setRange(0.5, 10.0);
  m_widthSpin->setSingleStep(0.2);
  m_widthSpin->setDecimals(1);
  m_widthSpin->setValue(1.5);
  m_widthSpin->setSuffix(QStringLiteral(" mm"));
  m_widthSpin->setMinimumWidth(96);  // 「1.5 mm」 whole, with the app's spin padding
  connect(m_widthSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double) {
    for (auto* b : m_widthPresetButtons) b->setChecked(false);
    markWidthEdges();
  });
  if (m_paper) {
    m_widthSpin->setFixedWidth(96);
    form->addRow(QStringLiteral("선 굵기"), widthRow);
    form->addRow(QStringLiteral("직접"), m_widthSpin);
    markWidthEdges();
    for (int row = 0; row < form->rowCount(); ++row)  // the board's 96 px words
      if (QLayoutItem* item = form->itemAt(row, QFormLayout::LabelRole))
        if (auto* label = qobject_cast<QLabel*>(item->widget())) {
          label->setFixedWidth(96);
          label->setProperty("kaFormWord", true);
        }
  } else {
    widthRow->addWidget(m_widthSpin);
    widthRow->addStretch();
    form->addRow(QStringLiteral("선 굵기"), widthRow);
  }
  mainLayout->addLayout(form);

  auto* btnLayout = new QHBoxLayout();
  btnLayout->addStretch();
  // The main button first, then 「취소」, like the other windows' button rows.
  auto* btnStart = new QPushButton(QStringLiteral("그리기 시작 (Enter)"), this);
  btnStart->setObjectName(QStringLiteral("surveyAreaStart"));
  btnStart->setDefault(true);  // the theme's main-button rule colours it
  btnStart->setMinimumSize(130, 36);
  btnStart->setMaximumHeight(36);
  connect(btnStart, &QPushButton::clicked, this, &QDialog::accept);
  btnLayout->addWidget(btnStart);
  auto* btnCancel = new QPushButton(QStringLiteral("취소"), this);
  btnCancel->setFixedSize(80, 36);
  connect(btnCancel, &QPushButton::clicked, this, &QDialog::reject);
  btnLayout->addWidget(btnCancel);
  mainLayout->addLayout(btnLayout);
}

void KaSurveyAreaDialog::selectColorIndex(int index) {
  if (index < 0 || index >= m_paletteColors.size()) return;
  m_strokeColor = m_paletteColors[index];
  m_fillColor = QColor(m_strokeColor.red(), m_strokeColor.green(), m_strokeColor.blue(), 50);
  const KaTheme::Tokens& theme = KaTheme::tokens();
  if (m_paper) {  // plain swatches; the chosen one ringed in ink with a paper gap (the 2 px padding)
    for (int i = 0; i < m_colorButtons.size(); ++i)
      m_colorButtons[i]->setStyleSheet(
          QStringLiteral("QPushButton { background-color: %1; background-clip: content; border: 2px solid %2; "
                         "border-radius: 4px; padding: 2px; min-height: 24px; max-height: 24px; }")
              .arg(m_paletteColors[i].name(), i == index ? theme.ink.name() : QStringLiteral("transparent")));
    m_colorName->setText(m_colorNames[index]);
    return;
  }
  for (int i = 0; i < m_colorButtons.size(); ++i) {
    const QColor& fill = m_paletteColors[i];
    const bool selected = i == index;
    m_colorButtons[i]->setStyleSheet(
        QStringLiteral("QPushButton { background-color: %1; color: %2; font-weight: %3; font-size: 12px; "
                       "border: %4 solid %5; border-radius: 6px; min-height: 0px; padding: 0px; }")
            .arg(fill.name(), readableTextOn(fill).name(), selected ? QStringLiteral("bold") : QStringLiteral("normal"),
                 selected ? QStringLiteral("3px") : QStringLiteral("1px"),
                 (selected ? theme.ink : theme.border).name()));
    m_colorButtons[i]->setText(selected ? QStringLiteral("✔") + m_colorNames[i] : m_colorNames[i]);
  }
}

void KaSurveyAreaDialog::selectPresetWidth(double width) {
  m_widthSpin->blockSignals(true);
  m_widthSpin->setValue(width);
  m_widthSpin->blockSignals(false);
  for (auto* b : m_widthPresetButtons) b->setChecked(false);
  if (auto* senderBtn = qobject_cast<QPushButton*>(sender())) senderBtn->setChecked(true);
  markWidthEdges();
}

void KaSurveyAreaDialog::markWidthEdges() {
  if (!m_paper) return;
  for (int i = 0; i < m_widthPresetButtons.size(); ++i) {
    QPushButton* button = m_widthPresetButtons.at(i);
    const bool after = i > 0 && m_widthPresetButtons.at(i - 1)->isChecked();
    if (button->property("kaAfterChecked").toBool() == after) continue;
    button->setProperty("kaAfterChecked", after);  // its left edge gives way to the chosen one's ink edge (QSS)
    button->style()->unpolish(button);
    button->style()->polish(button);
  }
}

QString KaSurveyAreaDialog::layerName() const {
  const QString typed = m_nameEdit ? m_nameEdit->text().trimmed() : QString();
  return typed.isEmpty() ? QStringLiteral("조사구역") : typed;
}

double KaSurveyAreaDialog::strokeWidthMm() const {
  return m_widthSpin ? m_widthSpin->value() : 1.5;
}
