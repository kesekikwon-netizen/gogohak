#pragma once

#include <QString>

class QProgressDialog;

// 「조용한 도판」 progress windows (2차 보드 판 5 「PDF 내보내기」「제출 꾸러미 만드는 중」, S 440): the step in ink with
// one muted sentence under it, a 4 px ink bar on a light track without its own figure, 「취소」 as a plain button.
// Stock keeps its own text and bar.
namespace KaProgressPlate {
// Marks the window and shapes its bar in 새 모양; does nothing in stock.
void dress(QProgressDialog* dialog);
// The label text: in 새 모양 the first line stays as it is and the following lines (and paperNote, a sentence the
// board adds) become one muted line under it; in stock the text as it is, without paperNote.
QString label(const QString& text, const QString& paperNote = QString());
// 「제출 꾸러미: <step>」, with the board's 「파일만 만듭니다. 지도에는 올리지 않습니다.」 under it in 새 모양.
QString packageLabel(const QString& step);
}  // namespace KaProgressPlate
