#include "KaProgressPlate.h"

#include <QAbstractButton>
#include <QApplication>
#include <QLabel>
#include <QProgressBar>
#include <QProgressDialog>
#include <QPushButton>
#include <QStringList>

namespace {
bool paperLook() { return qApp && qApp->property("kaPaperLook").toBool(); }
}  // namespace

namespace KaProgressPlate {

void dress(QProgressDialog* dialog) {
  if (!dialog || !paperLook()) return;
  dialog->setProperty("kaPlate", true);  // the 4 px bar and the muted line (QSS)
  dialog->setMinimumWidth(440);
  if (auto* bar = dialog->findChild<QProgressBar*>()) bar->setTextVisible(false);
  if (auto* text = dialog->findChild<QLabel*>()) text->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  if (auto* cancel = dialog->findChild<QPushButton*>()) cancel->setAutoDefault(false);  // a plain 「취소」
}

QString label(const QString& text, const QString& paperNote) {
  if (!paperLook()) return text;
  QStringList lines = text.split(QLatin1Char(0x0A));
  const QString step = lines.takeFirst();
  if (!paperNote.isEmpty()) lines << paperNote;
  if (lines.isEmpty()) return step;
  const QString muted = QApplication::palette().color(QPalette::PlaceholderText).name();  // KaTheme: inkMuted
  return step.toHtmlEscaped() + QStringLiteral("<br><span style=\"color:%1; font-size:12px\">").arg(muted) +
         lines.join(QLatin1Char(' ')).toHtmlEscaped() + QStringLiteral("</span>");
}

QString packageLabel(const QString& step) {
  return label(QStringLiteral("제출 꾸러미: %1").arg(step), QStringLiteral("파일만 만듭니다. 지도에는 올리지 않습니다."));
}

}  // namespace KaProgressPlate
