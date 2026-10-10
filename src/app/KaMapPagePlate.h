#pragma once

class QFrame;
class QSplitter;
class QWidget;

// 지도 화면의 판 틀 (보완 M1 · H1): in 새 모양 the map page is one sheet. Its layouts lose their margins (page 8 → 0, map card
// 4 → 0, spacing to 0) and the splitter handle is 7 px (stock 10); the QSS draws the one hairline in the handle and no frame
// round the page. Follows a switch of the look (KaTheme sends StyleChange to every widget). Stock values come back.
namespace KaMapPagePlate {
void follow(QWidget* page, QSplitter* split, QFrame* mapCard);
// 분할선 폭: 새 모양 7 px(3 + 가운데 1 px 선 + 3), 아니면 stockWidth. 켜고 끄면 따라 바뀐다. 이미 쓴 폭이 있으면(저장된 상태를 되살린 뒤 등)
// applyHandle로 다시 맞춘다. 판 틀(follow)의 분할선도 같은 길로 간다.
void followHandle(QSplitter* split, int stockWidth);
void applyHandle(QSplitter* split);
}  // namespace KaMapPagePlate
