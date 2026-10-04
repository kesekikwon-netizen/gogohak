#include "KaClosePrompt.h"

#include "KaHomeText.h"
#include "KaTheme.h"

#include <QFileInfo>
#include <QPushButton>
#include <QStyle>

namespace KaClosePrompt {

QString withObjectParticle(const QString& name) {
  const QChar last = name.isEmpty() ? QChar() : name.back();
  bool batchim = false;  // the name ends in a closed syllable: 「을」, else 「를」
  if (last.unicode() >= 0xAC00 && last.unicode() <= 0xD7A3) {
    batchim = (last.unicode() - 0xAC00) % 28 != 0;
  } else if (last.isDigit()) {
    batchim = QStringLiteral("013678").contains(last);  // 영 일 삼 육 칠 팔 end closed; 이 사 오 구 open
  } else {
    return name + QStringLiteral("을(를)");
  }
  return name + (batchim ? QStringLiteral("을") : QStringLiteral("를"));
}

void dress(QMessageBox& box, const QString& surveyPath, int unsavedFeatures) {
  box.setWindowTitle(QStringLiteral("조사 닫기"));
  box.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
  box.setDefaultButton(QMessageBox::Save);
  const QFileInfo file(surveyPath);
  if (!KaTheme::displayOptions().paperLook) {
    box.setIcon(QMessageBox::Question);
    box.setText(QStringLiteral("저장하지 않은 작업이 있습니다.\n\n%1\n\n저장할까요?").arg(file.fileName()));
    return;
  }
  box.setProperty("kaPlate", true);  // the question in the serif Heading (QSS)
  QAbstractButton* discard = box.button(QMessageBox::Discard);
  discard->setObjectName(QStringLiteral("closePromptDiscard"));  // danger text (QSS)
  discard->style()->unpolish(discard);  // the box styled its buttons when it made them, before the name
  discard->style()->polish(discard);
  box.setIcon(QMessageBox::NoIcon);
  box.setText(QStringLiteral("%1 저장할까요?").arg(withObjectParticle(file.completeBaseName())));
  box.setInformativeText(
      (unsavedFeatures > 0 ? QStringLiteral("저장하지 않은 작업이 %1건 있습니다.").arg(unsavedFeatures)
                           : QStringLiteral("저장하지 않은 작업이 있습니다.")) +
      QStringLiteral("\n%1 › %2").arg(KaHomeText::folder(surveyPath), file.fileName()));
}

QMessageBox::StandardButton ask(QWidget* parent, const QString& surveyPath, int unsavedFeatures) {
  QMessageBox box(parent);
  dress(box, surveyPath, unsavedFeatures);
  return static_cast<QMessageBox::StandardButton>(box.exec());
}

}  // namespace KaClosePrompt
