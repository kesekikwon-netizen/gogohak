#pragma once

#include <QDialog>
#include <QString>

class QCheckBox;
class QLineEdit;

// 「GPS 기준점」 typed in by hand: point ID, X·Y (Z optional), the survey basis and the receiver. The caller checks
// and writes the values; this window only collects them as typed. 새 모양 (2차 보드 판 5, 440): the axis note in
// three lines, then 「위치」「측량 기준」「수신」 sections with the basis and receiver values side by side; stock keeps
// one long form.
class KaControlPointDialog final : public QDialog {
public:
  // axisHint: LayerOps::controlPointAxisHint(); accuracyWord / fixWord: the field labels of accuracy_m and fix_type.
  KaControlPointDialog(const QString& axisHint, const QString& accuracyWord, const QString& fixWord,
                       QWidget* parent = nullptr);

  QString pointId() const;
  QString xText() const;
  QString yText() const;
  QString zText() const;
  bool swapAxes() const;
  QString datum() const;
  QString ellipsoid() const;
  QString projection() const;
  QString origin() const;
  QString accuracy() const;
  QString pdop() const;
  QString fix() const;

private:
  QLineEdit* m_id = nullptr;
  QLineEdit* m_x = nullptr;
  QLineEdit* m_y = nullptr;
  QLineEdit* m_z = nullptr;
  QCheckBox* m_swap = nullptr;
  QLineEdit* m_datum = nullptr;
  QLineEdit* m_ellipsoid = nullptr;
  QLineEdit* m_projection = nullptr;
  QLineEdit* m_origin = nullptr;
  QLineEdit* m_accuracy = nullptr;
  QLineEdit* m_pdop = nullptr;
  QLineEdit* m_fix = nullptr;
};
