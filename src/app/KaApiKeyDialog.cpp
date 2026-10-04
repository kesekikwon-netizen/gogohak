#include "KaApiKeyDialog.h"

#include "KaTheme.h"
#include "KaWindowSize.h"

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

KaApiKeyDialog::KaApiKeyDialog(const QString& vworldKey, const QString& historyKey, QWidget* parent)
    : QDialog(parent) {
  const bool paper = KaTheme::displayOptions().paperLook;
  setObjectName(QStringLiteral("apiKeyDialog"));
  setProperty("kaPlate", paper);
  setWindowTitle(QStringLiteral("API 키 입력"));
  setMinimumWidth(KaWindowSize::width(KaWindowSize::Small, 460));
  m_vworld = new QLineEdit(this);
  m_vworld->setObjectName(QStringLiteral("apiKeyVworld"));
  m_vworld->setText(vworldKey);
  m_vworld->setPlaceholderText(QStringLiteral("vworld.kr 인증키"));
  m_history = new QLineEdit(this);
  m_history->setObjectName(QStringLiteral("apiKeyHistory"));
  m_history->setText(historyKey);
  m_history->setPlaceholderText(QStringLiteral("hgis.history.go.kr 인증키"));
  auto* vworldHint = new QLabel(paper ? QStringLiteral("위성 · 지적 · 주소 찾기에 함께 씁니다.")
                                      : QStringLiteral("위성·지적·검색 공통 (SSOT: VWorld/ApiKey)"),
                                this);
  vworldHint->setObjectName(QStringLiteral("apiKeyVworldHint"));
  vworldHint->setWordWrap(true);
  auto* historyHint = new QLabel(QStringLiteral("1919 조선지형도 전용. VWorld 키로는 인증되지 않습니다."), this);
  historyHint->setObjectName(QStringLiteral("apiKeyHistoryHint"));
  historyHint->setWordWrap(true);
  auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
  buttons->button(QDialogButtonBox::Ok)->setText(QStringLiteral("저장"));
  buttons->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("취소"));
  connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

  if (!paper) {
    historyHint->setText(historyHint->text() + QStringLiteral("\nhttps://hgis.history.go.kr/api/intro.do"));
    historyHint->setOpenExternalLinks(true);
    auto* form = new QFormLayout(this);
    form->setSpacing(12);
    form->setContentsMargins(20, 20, 20, 16);
    form->addRow(QStringLiteral("VWorld API 키"), m_vworld);
    form->addRow(vworldHint);
    form->addRow(QStringLiteral("역사지리정보DB API 키"), m_history);
    form->addRow(historyHint);
    form->addRow(buttons);
    return;
  }
  // 2차 보드 판 5: name, box and sentence 6 px apart, the two keys 16 apart, the buttons 24 below.
  auto* column = new QVBoxLayout(this);
  column->setContentsMargins(24, 24, 24, 16);
  column->setSpacing(16);
  const auto addKey = [this, column](const QString& name, const char* nameId, QLineEdit* box, QLabel* hint) {
    auto* group = new QVBoxLayout;
    group->setSpacing(6);
    auto* word = new QLabel(name, this);
    word->setObjectName(QString::fromLatin1(nameId));
    word->setBuddy(box);
    group->addWidget(word);
    group->addWidget(box);
    group->addWidget(hint);
    column->addLayout(group);
  };
  addKey(QStringLiteral("VWorld API 키"), "apiKeyVworldName", m_vworld, vworldHint);
  addKey(QStringLiteral("역사지리정보DB API 키"), "apiKeyHistoryName", m_history, historyHint);
  column->addSpacing(8);
  column->addWidget(buttons);
}

QString KaApiKeyDialog::vworldKey() const { return m_vworld->text().trimmed(); }

QString KaApiKeyDialog::historyKey() const { return m_history->text().trimmed(); }
