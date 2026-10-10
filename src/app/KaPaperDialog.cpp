#include "KaPaperDialog.h"

#include "KaTheme.h"
#include "KaWindowSize.h"

#include <QButtonGroup>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPen>
#include <QPixmap>
#include <QPushButton>
#include <QStackedLayout>
#include <QToolButton>

#include <algorithm>

namespace {

QIcon paperIcon(bool landscape) {
  QPixmap pm(48, 48);
  pm.fill(Qt::transparent);
  QPainter p(&pm);
  p.setRenderHint(QPainter::Antialiasing, true);
  p.setPen(QPen(QColor(15, 23, 42), 1.6));
  p.setBrush(QColor(255, 255, 255));
  const qreal corner = KaTheme::depictedPaperRadius();
  if (landscape)
    p.drawRoundedRect(QRectF(5, 14, 38, 22), corner, corner);
  else
    p.drawRoundedRect(QRectF(13, 4, 22, 40), corner, corner);
  return QIcon(pm);
}

QString plain(double mm) { return QString::number(mm, 'g', 6); }  // 297, not 297.0

}  // namespace

KaPaperDialog::KaPaperDialog(QWidget* parent, double fieldWidthMm, double fieldHeightMm) : QDialog(parent) {
  const bool plate = KaTheme::displayOptions().paperLook;
  setWindowTitle(QStringLiteral("용지 설정"));
  setMinimumWidth(KaWindowSize::width(KaWindowSize::Small, 400));
  auto* form = new QFormLayout(this);
  auto makeChoice = [this](const char* name, const QString& text, const QIcon& icon, bool checked) {
    auto* b = new QToolButton(this);
    b->setObjectName(QString::fromLatin1(name));
    b->setText(text);
    if (!icon.isNull()) b->setIcon(icon);
    b->setIconSize(QSize(28, 28));
    b->setToolButtonStyle(icon.isNull() ? Qt::ToolButtonTextOnly : Qt::ToolButtonTextUnderIcon);
    b->setCheckable(true);
    b->setChecked(checked);
    b->setCursor(Qt::PointingHandCursor);
    b->setMinimumSize(88, 56);
    return b;
  };
  auto* paperRow = new QWidget(this);
  auto* paperLay = new QHBoxLayout(paperRow);
  paperLay->setContentsMargins(0, 0, 0, 0);
  auto* btnA4 = makeChoice("paperA4", QStringLiteral("A4"), QIcon(), true);
  auto* btnA3 = makeChoice("paperA3", QStringLiteral("A3"), QIcon(), false);
  auto* btnCustom = makeChoice("paperCustom", QStringLiteral("직접 입력"), QIcon(), false);
  auto* paperGroup = new QButtonGroup(this);
  paperGroup->setExclusive(true);
  paperGroup->addButton(btnA4, 0);
  paperGroup->addButton(btnA3, 1);
  paperGroup->addButton(btnCustom, 2);
  paperLay->addWidget(btnA4);
  paperLay->addWidget(btnA3);
  paperLay->addWidget(btnCustom);
  paperLay->addStretch(1);

  auto* orientRow = new QWidget(this);
  auto* orientLay = new QHBoxLayout(orientRow);
  orientLay->setContentsMargins(0, 0, 0, 0);
  auto* btnPort = makeChoice("paperPortrait", QStringLiteral("세로"), paperIcon(false), true);
  auto* btnLand = makeChoice("paperLandscape", QStringLiteral("가로"), paperIcon(true), false);
  auto* orientGroup = new QButtonGroup(this);
  orientGroup->setExclusive(true);
  orientGroup->addButton(btnPort, 0);
  orientGroup->addButton(btnLand, 1);
  orientLay->addWidget(btnPort);
  orientLay->addWidget(btnLand);
  orientLay->addStretch(1);

  const auto makeSpin = [this](const char* name, double value) {
    auto* spin = new QDoubleSpinBox(this);
    spin->setObjectName(QString::fromLatin1(name));
    spin->setRange(50.0, 2000.0);
    spin->setDecimals(1);
    spin->setSuffix(QStringLiteral(" mm"));
    spin->setValue(value);
    return spin;
  };
  m_width = makeSpin("paperWidth", 210.0);
  m_height = makeSpin("paperHeight", 297.0);
  // 새 모양: the size that follows from the paper is a reading line on a dotted leader; the entry field stands in
  // the same place and shows only under 「직접 입력」.
  struct Reading {
    QStackedLayout* stack = nullptr;
    QLabel* value = nullptr;
  };
  const auto addSize = [&](const QString& label, QDoubleSpinBox* spin, const char* specName) {
    Reading reading;
    if (!plate) {
      form->addRow(label, spin);
      return reading;
    }
    auto* holder = new QWidget(this);
    reading.stack = new QStackedLayout(holder);
    reading.stack->addWidget(spin);
    auto* spec = new QWidget(holder);
    spec->setObjectName(QString::fromLatin1(specName));
    auto* line = new QHBoxLayout(spec);
    line->setContentsMargins(0, 0, 0, 0);
    line->setSpacing(6);
    auto* leader = new QFrame(spec);
    leader->setObjectName(QStringLiteral("kaLeader"));
    leader->setFixedHeight(10);
    reading.value = new QLabel(spec);
    reading.value->setObjectName(QStringLiteral("paperSpecValue"));
    auto* unit = new QLabel(QStringLiteral("mm"), spec);
    unit->setObjectName(QStringLiteral("paperSpecUnit"));
    line->addWidget(leader, 1, Qt::AlignVCenter);
    line->addWidget(reading.value);
    line->addWidget(unit);
    reading.stack->addWidget(spec);
    form->addRow(label, holder);
    return reading;
  };
  auto* tip = new QLabel(this);
  tip->setObjectName(QStringLiteral("paperScaleNote"));
  tip->setWordWrap(true);
  form->addRow(QStringLiteral("용지"), paperRow);
  form->addRow(QStringLiteral("방향"), orientRow);
  const Reading widthReading = addSize(QStringLiteral("가로"), m_width, "paperSpecWidth");
  const Reading heightReading = addSize(QStringLiteral("세로"), m_height, "paperSpecHeight");
  form->addRow(tip);
  auto applyPreset = [=]() {
    const int p = paperGroup->checkedId();
    const bool land = btnLand->isChecked();
    const bool custom = p == 2;
    // The field sheet is larger than A4 on purpose. Say so where it is chosen:
    // printed on one A4 sheet it shrinks and the written 1:N no longer holds on paper.
    tip->setText(p == 0 && land
                     ? QStringLiteral("A4 가로는 좌·우·위 1cm를 더 준 현장 용지(317×220mm)입니다. "
                                      "A4 종이 한 장에 찍으면 약 90%로 줄어 종이 위 축척이 달라집니다. "
                                      "축척대로 찍으려면 인쇄 창에서 A3 또는 여러 장 나눠 찍기를 고르세요.")
                     : QStringLiteral("기본은 A4 세로입니다. A4 가로는 좌·우·위 1cm를 더 줍니다."));
    if (!plate) {
      m_width->setEnabled(custom);
      m_height->setEnabled(custom);
    } else {
      widthReading.stack->setCurrentIndex(custom ? 0 : 1);
      heightReading.stack->setCurrentIndex(custom ? 0 : 1);
    }
    if (!custom) {
      double pw = (p == 1) ? 297.0 : 210.0;
      double ph = (p == 1) ? 420.0 : 297.0;
      if (land) std::swap(pw, ph);
      if (p == 0 && land) {
        pw = fieldWidthMm;
        ph = fieldHeightMm;
      }
      m_width->setValue(pw);
      m_height->setValue(ph);
    }
    if (plate) {
      widthReading.value->setText(plain(m_width->value()));
      heightReading.value->setText(plain(m_height->value()));
    }
  };
  connect(paperGroup, &QButtonGroup::idClicked, this, [applyPreset](int) { applyPreset(); });
  connect(orientGroup, &QButtonGroup::idClicked, this, [applyPreset](int) { applyPreset(); });
  applyPreset();
  auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
  buttons->button(QDialogButtonBox::Ok)->setText(QStringLiteral("용지 만들기"));
  buttons->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("취소"));
  form->addRow(buttons);
  connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

double KaPaperDialog::widthMm() const { return m_width->value(); }
double KaPaperDialog::heightMm() const { return m_height->value(); }

bool KaPaperDialog::prompt(QWidget* parent, double* widthMm, double* heightMm, double fieldWidthMm, double fieldHeightMm) {
  if (!widthMm || !heightMm) return false;
  KaPaperDialog dialog(parent, fieldWidthMm, fieldHeightMm);
  if (dialog.exec() != QDialog::Accepted) return false;
  *widthMm = dialog.widthMm();
  *heightMm = dialog.heightMm();
  return true;
}
