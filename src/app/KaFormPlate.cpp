#include "KaFormPlate.h"

#include <QApplication>
#include <QDialog>
#include <QFormLayout>
#include <QLabel>

namespace KaFormPlate {

void dress(QDialog* dialog, QFormLayout* form) {
  if (!qApp || !qApp->property("kaPaperLook").toBool()) return;
  dialog->setProperty("kaPlate", true);  // the words and notes in 12 px muted (QSS)
  dialog->setMinimumWidth(440);
  form->setContentsMargins(24, 24, 24, 16);
  form->setHorizontalSpacing(0);
  form->setVerticalSpacing(12);
  for (int row = 0; row < form->rowCount(); ++row) {
    if (QLayoutItem* item = form->itemAt(row, QFormLayout::LabelRole))
      if (auto* word = qobject_cast<QLabel*>(item->widget())) {
        word->setFixedWidth(96);
        word->setProperty("kaFormWord", true);
      }
    if (QLayoutItem* item = form->itemAt(row, QFormLayout::SpanningRole))
      if (auto* note = qobject_cast<QLabel*>(item->widget())) {
        note->setWordWrap(true);
        note->setProperty("kaFormNote", true);
      }
  }
}

}  // namespace KaFormPlate
