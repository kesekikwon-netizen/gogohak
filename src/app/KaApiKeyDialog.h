#pragma once

#include <QDialog>
#include <QString>

class QLineEdit;

// 「API 키 입력」 (더보기): the VWorld key and the 역사지리정보DB key. 새 모양 (2차 보드 판 5, 440 wide): each name
// over its box with one plain sentence under it. Stock keeps the side-by-side form and its own words. The caller
// loads and saves the keys; this window only shows and edits them.
class KaApiKeyDialog final : public QDialog {
public:
  KaApiKeyDialog(const QString& vworldKey, const QString& historyKey, QWidget* parent = nullptr);

  QString vworldKey() const;   // trimmed
  QString historyKey() const;  // trimmed

private:
  QLineEdit* m_vworld = nullptr;
  QLineEdit* m_history = nullptr;
};
