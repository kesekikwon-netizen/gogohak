#include "KaHeritageSetupDialogs.h"
#include "core/HeritageIntranetSettings.h"
#include "core/HeritagePledge.h"
#include "core/HeritageRecentDownloads.h"
#include "core/HeritageRegionResolver.h"
#include "core/KoreaRegionCatalog.h"
#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QFrame>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QScreen>
#include <QScrollArea>
#include <QVBoxLayout>

KaHeritageAccountDialog::KaHeritageAccountDialog(QWidget* parent) : QDialog(parent) {
  setObjectName(QStringLiteral("heritageAccountDialog"));
  setWindowTitle(QStringLiteral("국가유산 인트라넷 아이디·비밀번호"));
  auto* layout = new QVBoxLayout(this);
  auto* scroll = new QScrollArea(this);
  scroll->setWidgetResizable(true);
  scroll->setFrameShape(QFrame::NoFrame);
  scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  auto* content = new QWidget(scroll);
  auto* inner = new QVBoxLayout(content);
  auto* description = new QLabel(QStringLiteral(
      "주변유적을 받을 때 사용할 국가유산 공간정보 인트라넷 계정을 이 PC에 저장합니다. "
      "수치지형도 계정과는 별개입니다. 두 항목을 모두 비우고 저장하면 자동로그인 계정을 해제합니다."), content);
  description->setWordWrap(true);
  description->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
  inner->addWidget(description);
  auto* form = new QFormLayout;
  form->setRowWrapPolicy(QFormLayout::WrapLongRows);
  auto* username = new QLineEdit(content);
  username->setObjectName(QStringLiteral("heritageAccountUsername"));
  auto* password = new QLineEdit(content);
  password->setObjectName(QStringLiteral("heritageAccountPassword"));
  password->setEchoMode(QLineEdit::Password);
  const auto saved = HeritageIntranetSettings::credentials();
  username->setText(saved.username);
  password->setText(saved.password);
  form->addRow(QStringLiteral("아이디"), username);
  form->addRow(QStringLiteral("비밀번호"), password);
  inner->addLayout(form);
  auto* error = new QLabel(content);
  error->setObjectName(QStringLiteral("heritageAccountError"));
  error->setWordWrap(true);
  error->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
  error->hide();
  inner->addWidget(error);
  inner->addStretch();
  scroll->setWidget(content);
  layout->addWidget(scroll, 1);
  auto* buttons = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, this);
  buttons->button(QDialogButtonBox::Save)->setText(QStringLiteral("저장"));
  buttons->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("취소"));
  layout->addWidget(buttons);
  connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
  connect(buttons, &QDialogButtonBox::accepted, this, [this, username, password, error, scroll] {
    const HeritageIntranetSettings::Credentials entered{username->text().trimmed(), password->text()};
    QString why;
    if (entered.username.isEmpty() != entered.password.isEmpty())
      why = QStringLiteral("아이디와 비밀번호를 모두 입력하거나 두 항목을 모두 비워 주세요.");
    else if (HeritageIntranetSettings::saveCredentials(entered, &why)) { accept(); return; }
    error->setText(why);
    error->show();
    scroll->ensureWidgetVisible(error);
  });
  QSize initial(480, 300);
  if (auto* currentScreen = screen())
    initial = initial.boundedTo(currentScreen->availableGeometry().size() - QSize(24, 24));
  resize(initial);
}

KaHeritageRegionDialog::KaHeritageRegionDialog(const QString& initialSido, const QString& initialCity,
                                             const QString& reason, QWidget* parent) : QDialog(parent) {
  // 새 모양 (2차 보드 판 5, 520): two plain lines, 96 px words beside 240 px lists, the nearby 시·군 under a section
  // word, the pledge as a muted note (KaHeritageSetupDialogs.h).
  m_paper = qApp && qApp->property("kaPaperLook").toBool();
  setObjectName(QStringLiteral("heritageRegionDialog"));
  setProperty("kaPlate", m_paper);
  setWindowTitle(QStringLiteral("주변유적 받을 시·군 선택"));
  m_layout = new QVBoxLayout(this);
  if (m_paper) m_layout->setContentsMargins(24, 24, 24, 16);
  auto* description = new QLabel(reason.isEmpty()
      ? QStringLiteral("조사구역이 속한 시·군을 확인하세요.") + (m_paper ? QString(QLatin1Char(0x0A)) : QStringLiteral(" ")) +
            QStringLiteral("자료는 선택한 시·군 단위로만 받습니다.")
      : reason + QStringLiteral("\n조사구역이 속한 시·군을 아래에서 직접 선택하면 계속 받을 수 있습니다."), this);
  description->setObjectName(QStringLiteral("heritageRegionText"));
  description->setWordWrap(true);
  description->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
  m_layout->addWidget(description);
  auto* form = new QFormLayout;
  m_sido = new QComboBox(this);
  m_sido->setObjectName(QStringLiteral("heritageSido"));
  m_city = new QComboBox(this);
  m_city->setObjectName(QStringLiteral("heritageCity"));
  m_sido->addItem(QStringLiteral("시·도 선택"), QString());
  for (const auto& name : KoreaRegionCatalog::sidoNames()) m_sido->addItem(name, name);
  form->addRow(QStringLiteral("시·도"), m_sido);
  form->addRow(QStringLiteral("시·군·구"), m_city);
  if (m_paper) {
    form->setHorizontalSpacing(0);
    form->setVerticalSpacing(12);
    for (QComboBox* list : {m_sido, m_city}) {
      list->setFixedWidth(240);
      if (auto* word = qobject_cast<QLabel*>(form->labelForField(list))) {
        word->setFixedWidth(96);
        word->setProperty("kaFormWord", true);
      }
    }
    m_layout->addSpacing(8);
  }
  m_layout->addLayout(form);
  // F174: reusing a recent download is the user's choice, never automatic.
  m_reuse = new QCheckBox(this);
  m_reuse->setObjectName(QStringLiteral("heritageReuseRecent"));
  m_reuse->setToolTip(QStringLiteral("같은 시·군에서 최근에 받아 검사·적재까지 마친 자료를 다시 씁니다. "
                                     "끄면 서버에서 새로 받습니다."));
  m_reuse->hide();
  m_layout->addWidget(m_reuse);
  m_reuseWhen = new QLabel(this);  // 새 모양: 「2026-10-02 17:05 받음 · 없는 자료만 서버에서 받음」 under the check
  m_reuseWhen->setObjectName(QStringLiteral("heritageReuseWhen"));
  m_reuseWhen->hide();
  m_layout->addWidget(m_reuseWhen);
  // F169: the pledge is still agreed automatically; say so before anything is fetched.
  auto* pledge = new QLabel(HeritagePledge::preDisclosure(), this);
  pledge->setObjectName(QStringLiteral("heritagePledgeNote"));
  pledge->setWordWrap(true);
  pledge->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
  if (!m_paper) pledge->setStyleSheet(QStringLiteral("color: palette(mid);"));  // 새 모양: muted note in the QSS
  m_layout->addWidget(pledge);
  auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
  auto* ok = buttons->button(QDialogButtonBox::Ok);
  ok->setText(QStringLiteral("이 지역 받기"));
  buttons->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("취소"));
  m_layout->addWidget(buttons);
  auto updateCities = [this] {
    m_city->clear();
    m_city->addItem(QStringLiteral("시·군·구 선택"), QString());
    for (const auto& name : KoreaRegionCatalog::citiesOf(sido())) m_city->addItem(name, name);
  };
  auto updateEnabled = [this, ok] {
    ok->setEnabled(!sido().isEmpty() && !city().isEmpty());
    refreshReuse();
  };
  connect(m_sido, &QComboBox::currentIndexChanged, this, updateCities);
  connect(m_city, &QComboBox::currentIndexChanged, this, updateEnabled);
  connect(buttons, &QDialogButtonBox::accepted, this, [this] {
    if (!sido().isEmpty() && !city().isEmpty()) accept();
  });
  connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
  updateCities();
  const int province = m_sido->findData(KoreaRegionCatalog::canonicalSido(initialSido));
  if (province > 0) m_sido->setCurrentIndex(province);
  const int municipality = m_city->findData(initialCity);
  if (municipality > 0) m_city->setCurrentIndex(municipality);
  updateEnabled();
  resize(520, 300);
}

KaHeritageRegionDialog::KaHeritageRegionDialog(const HeritageRegion& region, const QString& reason,
                                             QWidget* parent)
    : KaHeritageRegionDialog(region.sido, region.city, reason, parent) {
  addNearby(region.nearby, region.nearbyNote);
}

void KaHeritageRegionDialog::addNearby(const QList<HeritageCity>& nearby, const QString& note) {
  if (nearby.isEmpty() && note.isEmpty()) return;
  const QString heading = QStringLiteral("조사구역 주변 5km에 걸친 다른 시·군");
  auto* group = new QGroupBox(m_paper ? QString() : heading, this);
  group->setObjectName(QStringLiteral("heritageNearbyGroup"));
  auto* inner = new QVBoxLayout(group);
  if (m_paper) {  // the section word with a hairline, the boxes, then the hint (2차 보드 판 5)
    inner->setContentsMargins(0, 8, 0, 0);
    inner->setSpacing(2);
    auto* head = new QHBoxLayout;
    head->setSpacing(8);
    auto* word = new QLabel(heading, group);
    word->setObjectName(QStringLiteral("heritageNearbyHead"));
    auto* rule = new QFrame(group);
    rule->setObjectName(QStringLiteral("heritageNearbyRule"));
    rule->setFixedHeight(1);
    head->addWidget(word);
    head->addWidget(rule, 1, Qt::AlignVCenter);
    inner->addLayout(head);
    inner->addSpacing(6);
  }
  if (!nearby.isEmpty() && !m_paper) {
    auto* hint = new QLabel(QStringLiteral("체크한 시·군도 하나씩 차례로 받습니다. 시·군마다 따로 요청하며 "
                                           "전국 자료는 받지 않습니다."), group);
    hint->setWordWrap(true);
    hint->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    inner->addWidget(hint);
  }
  for (const HeritageCity& city : nearby) {
    if (!city.ok()) continue;
    auto* box = new QCheckBox(city.display(), group);
    box->setObjectName(QStringLiteral("heritageNearby"));
    box->setProperty("sido", city.sido);
    box->setProperty("city", city.city);
    box->setChecked(false);  // default: only the resolved 시/군
    connect(box, &QCheckBox::toggled, this, [this] { refreshReuse(); });
    inner->addWidget(box);
    m_nearby.append(box);
  }
  if (!nearby.isEmpty() && m_paper) {
    auto* hint = new QLabel(QStringLiteral("체크한 시·군도 하나씩 차례로 받습니다."), group);
    hint->setObjectName(QStringLiteral("heritageNearbyHint"));
    inner->addWidget(hint);
  }
  if (!note.isEmpty()) {
    auto* noteLabel = new QLabel(note, group);
    noteLabel->setObjectName(QStringLiteral("heritageNearbyNote"));
    noteLabel->setWordWrap(true);
    noteLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    inner->addWidget(noteLabel);
  }
  m_layout->insertWidget(m_layout->indexOf(m_reuse), group);
  refreshReuse();
  resize(width(), qMax(height(), sizeHint().height()));
}

void KaHeritageRegionDialog::refreshReuse() {
  if (!m_reuse) return;
  QList<HeritageCity> cities{{sido(), city()}};
  for (QCheckBox* box : std::as_const(m_nearby))
    if (box->isChecked())
      cities.append({box->property("sido").toString(), box->property("city").toString()});
  QDateTime newest;
  for (const HeritageCity& c : std::as_const(cities)) {
    if (!c.ok()) continue;
    const QDateTime when = HeritageRecentDownloads::latest(HeritageRecentDownloads::defaultRoot(c));
    if (when.isValid() && (!newest.isValid() || when > newest)) newest = when;
  }
  if (!newest.isValid()) {
    m_reuse->setChecked(false);
    m_reuse->hide();
    m_reuseWhen->hide();
    return;
  }
  const QString when = newest.toString(QStringLiteral("yyyy-MM-dd HH:mm"));
  m_reuse->setText(m_paper ? QStringLiteral("최근 받은 자료 다시 쓰기")
                           : QStringLiteral("최근 받은 자료 다시 쓰기 (%1 받음 · 없는 자료만 서버에서 받음)").arg(when));
  m_reuseWhen->setText(QStringLiteral("%1 받음 · 없는 자료만 서버에서 받음").arg(when));
  m_reuse->show();
  m_reuseWhen->setVisible(m_paper);
}

HeritageFetchPlan KaHeritageRegionDialog::plan() const {
  HeritageFetchPlan plan;
  QList<HeritageCity> picked;
  for (QCheckBox* box : m_nearby)
    if (box->isChecked())
      picked.append({box->property("sido").toString(), box->property("city").toString()});
  plan.followUps = HeritageFetchPlanning::normalizedFollowUps({sido(), city()}, picked);
  plan.reuseRecent = m_reuse && !m_reuse->isHidden() && m_reuse->isChecked();
  return plan;
}

QString KaHeritageRegionDialog::sido() const { return m_sido->currentData().toString(); }
QString KaHeritageRegionDialog::city() const { return m_city->currentData().toString(); }
