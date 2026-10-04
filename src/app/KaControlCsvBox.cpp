#include "KaControlCsvBox.h"

#include <QApplication>
#include <QMessageBox>
#include <QStringList>

namespace KaControlCsvBox {

void dress(QMessageBox& box, bool swapSuggested, int count, const QString& summary) {
  box.setWindowTitle(QStringLiteral("기준점 미리보기"));
  if (!qApp || !qApp->property("kaPaperLook").toBool()) {
    box.setIcon(swapSuggested ? QMessageBox::Warning : QMessageBox::Information);
    box.setText(summary);
    return;
  }
  const QString finding = QStringLiteral("X·Y가 바뀐 것 같습니다.");
  QStringList lines = summary.split(QLatin1Char(0x0A));
  QString reason;
  for (int i = lines.size() - 1; i >= 0; --i)  // the summary ends with 「X·Y가 바뀐 것 같습니다. <reason>」
    if (lines.at(i).startsWith(finding)) reason = lines.takeAt(i).mid(finding.size()).trimmed();
  if (!reason.isEmpty()) lines.prepend(reason);
  box.setProperty("kaPlate", true);  // serif heading, 12 px sentences (QSS)
  box.setIcon(QMessageBox::NoIcon);
  box.setText(swapSuggested ? finding : QStringLiteral("기준점 %1개를 가져옵니다.").arg(count));
  box.setInformativeText(lines.join(QLatin1Char(0x0A)));
}

}  // namespace KaControlCsvBox
