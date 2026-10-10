#pragma once

class QScrollArea;
class QToolButton;
class QWidget;

// 「조용한 도판」 도면 (2차 보드 판 4): pieces of the drawing studio's 새 모양 kept out of KaDrawingStudio.cpp.
namespace KaDrawingStudioPlate {
// The right panel for the splitter. In 새 모양 a column: the scrolling panel, then a foot under a hairline holding
// 「PDF 내보내기 | 인쇄」 (the btnPrimary and btnPrint buttons moved out of their card), so they never scroll away.
// In stock the scrolling panel itself, untouched.
QWidget* inspectorColumn(QScrollArea* scroll);
// The bottom tool strip, returned as given. In 새 모양 one strip of joined 96 × 52 cells, a hairline between them and
// one-line words (QSS: QWidget#layoutBottomTools[kaPlate]); in stock left as it is.
QWidget* bottomStrip(QWidget* strip);
// 「세트」 on the layer-set button, 「세트 ▾」 in 새 모양 (the board's list mark); follows a switch of the look.
void setWord(QToolButton* button);
// The scale presets: 3 a row with the stock gap; in 새 모양 7 a row with none (the board's joined table).
int chipsPerRow();
int chipGap(int stockGap);
// In 새 모양 marks the scaleChip rows and columns under content (kaRow/kaCol, first or rest) so the QSS shares their
// inner edges, and closes the gaps between the chip rows; the gap above the table stays. Nothing in stock.
void joinScaleTable(QWidget* content);
// In 새 모양 「축척 글자 · 좌표계 · 좌표 격자」 (by their tooltip word; the grid button by its name) become words
// without icons, 32 px high, sharing their row (the board's bottom of the panel). Nothing in stock.
void wordTiles(QWidget* content);
// The right column (the inspector scroll area's panel) on a short drawing tab: its vertical margins and spacing halve
// under 860 px so the last row
// stays above 「PDF 내보내기」 (the user's 3440×1440 at 150 % leaves 753 px; 2026-10-10). Buttons keep their size;
// a taller tab gets the full spacing back. Both looks.
void fitToHeight(QScrollArea* inspector, int studioHeight);
}  // namespace KaDrawingStudioPlate
