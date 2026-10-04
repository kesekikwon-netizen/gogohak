#pragma once

class QScrollArea;
class QWidget;

// 「조용한 도판」 도면 (2차 보드 판 4): pieces of the drawing studio's 새 모양 kept out of KaDrawingStudio.cpp.
namespace KaDrawingStudioPlate {
// The right panel for the splitter. In 새 모양 a column: the scrolling panel, then a foot under a hairline holding
// 「PDF 내보내기 | 인쇄」 (the btnPrimary and btnPrint buttons moved out of their card), so they never scroll away.
// In stock the scrolling panel itself, untouched.
QWidget* inspectorColumn(QScrollArea* scroll);
}  // namespace KaDrawingStudioPlate
