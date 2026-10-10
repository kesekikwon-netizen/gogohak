#pragma once

class QFrame;
class QSplitter;
class QWidget;

// 지도 화면의 판 틀 (보완 M1 · H1): in 새 모양 the map page is one sheet. Its layouts lose their margins (page 8 → 0, map card
// 4 → 0, spacing to 0) and the splitter handle is 7 px (stock 10); the QSS draws the one hairline in the handle and no frame
// round the page. Follows a switch of the look (KaTheme sends StyleChange to every widget). Stock values come back.
namespace KaMapPagePlate {
void follow(QWidget* page, QSplitter* split, QFrame* mapCard);
}  // namespace KaMapPagePlate
