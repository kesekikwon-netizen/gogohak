#pragma once

class QDialog;
class QFormLayout;

// 「조용한 도판」 「도형 속성」 (조사카드, 2차 보드 판 5, M 560): rebuilds the record form into the board's leader
// sections 식별 / 층위 / 표고 / 조사 / 사진 · 비고, two fields to a row (선후관계 · 사진 · 비고 across), 표고 boxes in
// mono with 「m」 and the section picture beside them. Each field box carries its field name in the
// "kaRecordField" property; the boxes themselves stay, so the caller reads them as before. Stock is left alone.
namespace KaRecordPlate {
void arrange(QDialog* dialog, QFormLayout* form);
}  // namespace KaRecordPlate
