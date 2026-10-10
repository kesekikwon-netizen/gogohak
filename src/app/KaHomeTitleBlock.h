#pragma once

#include "core/RecentSurveys.h"

#include <QFrame>

class QHBoxLayout;
class QLabel;
class QPushButton;
class QVBoxLayout;
namespace SurveyFacts {
struct Facts;
}

// 「조용한 도판」 홈 (2차 보드 판 2): the survey to continue as a drawing's title block — an ink frame, the
// survey name large in the title face, a row of cells (폴더 · 마지막 열림 · 작업 좌표계 · 조사구역 · 유구 ·
// 검수) and 「이어서 열기 →」 beside the name. Three rows in one frame: caption · name + button · cells. Only facts already recorded are shown; an unknown fact
// reads 「—」 (검수: 「검수 전」). Read-only: nothing here opens the survey.
class KaHomeTitleBlock final : public QFrame {
  Q_OBJECT
public:
  explicit KaHomeTitleBlock(QWidget* parent = nullptr);
  void setSurvey(const RecentSurveys::Item& item, const SurveyFacts::Facts& facts, bool unsaved);
  QPushButton* resumeButton() const { return m_resume; }
  // A line under the cells (the recovery note); the block keeps it and shows its row only while the note shows.
  void addNote(QWidget* note);

  // "5186 중부원점" for a work CRS auth id, "—" when unknown.
  static QString crsText(const QString& authId);

protected:
  bool eventFilter(QObject* watched, QEvent* event) override;

private:
  QLabel* cell(const char* caption, const char* name, QLayout* row);
  void showFolder();

  QLabel* m_state = nullptr;
  QLabel* m_name = nullptr;
  QLabel* m_folder = nullptr;
  QLabel* m_when = nullptr;
  QLabel* m_crs = nullptr;
  QLabel* m_areas = nullptr;
  QLabel* m_features = nullptr;
  QLabel* m_check = nullptr;
  QPushButton* m_resume = nullptr;
  QVBoxLayout* m_outer = nullptr;
  QWidget* m_noteBox = nullptr;
  QHBoxLayout* m_noteRow = nullptr;
  QString m_folderText;
};
