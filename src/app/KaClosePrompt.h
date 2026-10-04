#pragma once

#include <QMessageBox>
#include <QString>

class QWidget;

// 「조사 닫기」: asked when the window closes with unsaved work. 새 모양 (2차 보드 판 5, 설계 2.4 Dialog): the serif
// question 「광령리1을 저장할까요?」, how much is unsaved and where the file is, no icon disc. Stock keeps the plain
// question. The answer is Save, Discard or Cancel; the window acts on it.
namespace KaClosePrompt {
void dress(QMessageBox& box, const QString& surveyPath, int unsavedFeatures);
QMessageBox::StandardButton ask(QWidget* parent, const QString& surveyPath, int unsavedFeatures);
// 「광령리1을」 · 「테스트를」: the object particle after a name (「을(를)」 when the name ends in Latin letters).
QString withObjectParticle(const QString& name);
}  // namespace KaClosePrompt
