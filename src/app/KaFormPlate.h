#pragma once

class QDialog;
class QFormLayout;

// 「조용한 도판」 frame for a plain form window (2차 보드 판 5 「VWorld 지적도 계정」, S 440): 96 px words in 12 px
// muted, a sentence spanning the form as a muted note, 24 px margins and 12 px between rows. Stock is left alone.
namespace KaFormPlate {
void dress(QDialog* dialog, QFormLayout* form);
}  // namespace KaFormPlate
