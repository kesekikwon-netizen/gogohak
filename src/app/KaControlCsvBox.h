#pragma once

#include <QString>

class QMessageBox;

// 「기준점 미리보기」 after a CSV of control points is read. Stock shows the whole summary under an icon. 새 모양
// (2차 보드 판 5) leads with the finding as the serif heading — 「X·Y가 바뀐 것 같습니다.」 or 「기준점 n개를
// 읽었습니다.」 — with the reason first under it and the rest of the summary after, no icon. The caller adds the
// 「이대로 가져오기」「X·Y 바꿔서 가져오기」「취소」 buttons.
namespace KaControlCsvBox {
void dress(QMessageBox& box, bool swapSuggested, int count, const QString& summary);
}  // namespace KaControlCsvBox
