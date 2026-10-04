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
}  // namespace KaDrawingStudioPlate
