#pragma once

#include <QDialog>

class QDoubleSpinBox;

// 「용지 설정」: A4 · A3 · 직접 입력, 세로 · 가로, and the size in mm. Moved out of KaDrawingStudio.cpp (보완 L5). In 새 모양 a
// size that follows from the paper is a reading line (dotted leader, the value in mono ink, the unit grey), and only
// 「직접 입력」 shows entry fields; the stock look keeps its disabled fields. The field sheet (A4 가로 + 1 cm on three sides)
// is given by the caller.
class KaPaperDialog : public QDialog {
  Q_OBJECT
public:
  KaPaperDialog(QWidget* parent, double fieldWidthMm, double fieldHeightMm);
  double widthMm() const;
  double heightMm() const;
  // Shows the dialog; true when 「용지 만들기」 was pressed, with the size in mm.
  static bool prompt(QWidget* parent, double* widthMm, double* heightMm, double fieldWidthMm, double fieldHeightMm);

private:
  QDoubleSpinBox* m_width = nullptr;
  QDoubleSpinBox* m_height = nullptr;
};
