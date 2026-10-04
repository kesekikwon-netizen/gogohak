#include "KaControlPointDialog.h"

#include "KaFormPlate.h"

#include <QApplication>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>

namespace {
QLineEdit* box(QWidget* parent, const char* name, const QString& text = QString()) {
  auto* edit = new QLineEdit(text, parent);
  edit->setObjectName(QString::fromLatin1(name));
  return edit;
}

// 「위치」 ─────: an 11 px section word with a hairline running to the right edge.
QHBoxLayout* sectionHead(QWidget* parent, const QString& word) {
  auto* head = new QHBoxLayout;
  head->setSpacing(8);
  auto* label = new QLabel(word, parent);
  label->setObjectName(QStringLiteral("controlPointSection"));
  auto* rule = new QFrame(parent);
  rule->setObjectName(QStringLiteral("controlPointRule"));
  rule->setFixedHeight(1);
  head->addWidget(label);
  head->addWidget(rule, 1, Qt::AlignVCenter);
  return head;
}

// Values side by side, each under its 11 px caption.
QHBoxLayout* captionedRow(QWidget* parent, const QList<QPair<QString, QLineEdit*>>& cells) {
  auto* row = new QHBoxLayout;
  row->setSpacing(8);
  for (const auto& cell : cells) {
    auto* column = new QVBoxLayout;
    column->setSpacing(4);
    auto* caption = new QLabel(cell.first, parent);
    caption->setObjectName(QStringLiteral("controlPointCaption"));
    caption->setBuddy(cell.second);
    column->addWidget(caption);
    column->addWidget(cell.second);
    row->addLayout(column, 1);
  }
  return row;
}
}  // namespace

KaControlPointDialog::KaControlPointDialog(const QString& axisHint, const QString& accuracyWord, const QString& fixWord,
                                           QWidget* parent)
    : QDialog(parent) {
  const bool paper = qApp && qApp->property("kaPaperLook").toBool();
  setObjectName(QStringLiteral("controlPointDialog"));
  setWindowTitle(QStringLiteral("GPS 기준점"));
  m_id = box(this, "controlPointId");
  m_x = box(this, "controlPointX");
  m_y = box(this, "controlPointY");
  m_z = box(this, "controlPointZ");
  m_z->setPlaceholderText(QStringLiteral("비워 두면 표고 없이 넣습니다"));
  m_datum = box(this, "controlPointDatum", QStringLiteral("세계측지계"));
  m_ellipsoid = box(this, "controlPointEllipsoid", QStringLiteral("GRS80"));
  m_projection = box(this, "controlPointProjection", QStringLiteral("TM/UTM-K"));
  m_origin = box(this, "controlPointOrigin");
  m_accuracy = box(this, "controlPointAccuracy", QStringLiteral("1.0"));
  m_pdop = box(this, "controlPointPdop", QStringLiteral("1.5"));
  m_fix = box(this, "controlPointFix", QStringLiteral("RTK"));
  m_swap = new QCheckBox(QStringLiteral("X·Y 교환 (측량 X=북, Y=동)"), this);
  QString note = axisHint;
  if (paper) note.replace(QStringLiteral(". "), QStringLiteral(".") + QLatin1Char(0x0A));  // one sentence a line
  auto* hint = new QLabel(note, this);
  hint->setObjectName(QStringLiteral("controlPointHint"));
  hint->setWordWrap(true);
  auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
  connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

  if (!paper) {
    auto* form = new QFormLayout(this);
    form->addRow(hint);
    form->addRow(QStringLiteral("점ID"), m_id);
    form->addRow(QStringLiteral("X (동쪽)"), m_x);
    form->addRow(QStringLiteral("Y (북쪽)"), m_y);
    form->addRow(m_swap);
    form->addRow(QStringLiteral("Z (표고 m, 선택)"), m_z);
    form->addRow(QStringLiteral("측지기준계"), m_datum);
    form->addRow(QStringLiteral("타원체"), m_ellipsoid);
    form->addRow(QStringLiteral("투영"), m_projection);
    form->addRow(QStringLiteral("원점"), m_origin);
    form->addRow(accuracyWord, m_accuracy);
    form->addRow(QStringLiteral("PDOP"), m_pdop);
    form->addRow(fixWord, m_fix);
    form->addRow(buttons);
    return;
  }
  // 2차 보드 판 5: note, 「위치」 form, 「측량 기준」 four across, 「수신」 three across, the buttons 24 below.
  auto* column = new QVBoxLayout(this);
  column->setSpacing(0);
  column->addWidget(hint);
  column->addSpacing(16);
  column->addLayout(sectionHead(this, QStringLiteral("위치")));
  column->addSpacing(8);
  auto* place = new QWidget(this);
  auto* form = new QFormLayout(place);
  form->addRow(QStringLiteral("점ID"), m_id);
  form->addRow(QStringLiteral("X (동쪽)"), m_x);
  form->addRow(QStringLiteral("Y (북쪽)"), m_y);
  form->addRow(m_swap);
  form->addRow(QStringLiteral("Z (표고 m, 선택)"), m_z);
  for (QLineEdit* figure : {m_x, m_y, m_z}) figure->setFixedWidth(176);
  setTabOrder(m_y, m_swap);  // the check stands between Y and Z here
  setTabOrder(m_swap, m_z);
  KaFormPlate::dress(this, form);  // 96 px words, 12 px between rows, the 440 width
  form->setContentsMargins(0, 0, 0, 0);
  layout()->setContentsMargins(24, 24, 24, 16);
  column->addWidget(place);
  column->addSpacing(20);
  column->addLayout(sectionHead(this, QStringLiteral("측량 기준")));
  column->addSpacing(8);
  column->addLayout(captionedRow(this, {{QStringLiteral("측지기준계"), m_datum}, {QStringLiteral("타원체"), m_ellipsoid},
                                        {QStringLiteral("투영"), m_projection}, {QStringLiteral("원점"), m_origin}}));
  column->addSpacing(16);
  column->addLayout(sectionHead(this, QStringLiteral("수신")));
  column->addSpacing(8);
  column->addLayout(captionedRow(this, {{accuracyWord, m_accuracy}, {QStringLiteral("PDOP"), m_pdop}, {fixWord, m_fix}}));
  column->addSpacing(24);
  column->addWidget(buttons);
}

QString KaControlPointDialog::pointId() const { return m_id->text(); }
QString KaControlPointDialog::xText() const { return m_x->text(); }
QString KaControlPointDialog::yText() const { return m_y->text(); }
QString KaControlPointDialog::zText() const { return m_z->text(); }
bool KaControlPointDialog::swapAxes() const { return m_swap->isChecked(); }
QString KaControlPointDialog::datum() const { return m_datum->text(); }
QString KaControlPointDialog::ellipsoid() const { return m_ellipsoid->text(); }
QString KaControlPointDialog::projection() const { return m_projection->text(); }
QString KaControlPointDialog::origin() const { return m_origin->text(); }
QString KaControlPointDialog::accuracy() const { return m_accuracy->text(); }
QString KaControlPointDialog::pdop() const { return m_pdop->text(); }
QString KaControlPointDialog::fix() const { return m_fix->text(); }
