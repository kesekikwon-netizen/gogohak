#include "KaReferenceStatusDialog.h"
#include "core/ReferenceInventory.h"

#include <QApplication>
#include <QDialogButtonBox>
#include <QFrame>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QTreeWidget>
#include <QPainter>
#include <QPixmap>
#include <QVBoxLayout>
#include <qgsproject.h>

namespace {
// 2차 보드 판 5: the warn triangle before 「인터넷 필요」 (warn ink #7A5A00 of the paper look).
QIcon warnMark() {
  QPixmap mark(12, 12);
  mark.fill(Qt::transparent);
  QPainter p(&mark);
  p.setRenderHint(QPainter::Antialiasing);
  p.setPen(QPen(QColor(0x7A, 0x5A, 0x00), 1.2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
  p.drawPolygon(QPolygonF({QPointF(6, 1.5), QPointF(11, 10.5), QPointF(1, 10.5)}));
  p.drawLine(QPointF(6, 4.8), QPointF(6, 7.2));
  p.drawPoint(QPointF(6, 8.9));
  return QIcon(mark);
}
}  // namespace

KaReferenceStatusDialog::KaReferenceStatusDialog(QgsProject* project, QWidget* parent)
    : QDialog(parent), m_project(project), m_paper(qApp && qApp->property("kaPaperLook").toBool()) {
  setObjectName(QStringLiteral("referenceStatusDialog"));
  setProperty("kaPlate", m_paper);
  setWindowTitle(QStringLiteral("자료 준비 상태"));
  setModal(false);
  resize(860, 420);
  auto* layout = new QVBoxLayout(this);
  const QString what = QStringLiteral("조사에 올린 참조 자료가 현장에서 인터넷 없이 열리는지 확인하는 표입니다. "
                                      "이 창은 자료를 새로 받거나 지우지 않습니다.");
  const QString how = QStringLiteral("인터넷이 필요한 배경은 레이어를 오른쪽 클릭해 「오프라인 저장」으로 직접 받아 두세요.");
  auto* hint = new QLabel(what + (m_paper ? QString(QLatin1Char(0x0A)) : QStringLiteral(" ")) + how, this);
  hint->setObjectName(QStringLiteral("referenceStatusHint"));
  hint->setWordWrap(true);
  m_summary = new QLabel(this);
  m_summary->setObjectName(QStringLiteral("referenceStatusSummary"));
  m_summary->setWordWrap(true);
  // 새 모양 (2차 보드 판 5): the counts are the serif title and the note follows; stock keeps the note first.
  layout->addWidget(m_paper ? m_summary : hint);
  layout->addWidget(m_paper ? hint : m_summary);
  m_table = new QTreeWidget(this);
  m_table->setObjectName(QStringLiteral("referenceStatusTable"));
  m_table->setRootIsDecorated(false);
  m_table->setSelectionMode(QAbstractItemView::SingleSelection);
  m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_table->setHeaderLabels({QStringLiteral("자료"), QStringLiteral("종류"), QStringLiteral("범위"),
                            QStringLiteral("받은 날짜"), QStringLiteral("크기"), QStringLiteral("현장(오프라인)")});
  m_table->header()->setSectionResizeMode(0, QHeaderView::Stretch);
  const int boardWidths[] = {0, 100, 136, 132, 72, 212};  // 2차 보드 판 5 columns after 「자료」
  for (int column = 1; column < m_table->columnCount(); ++column) {
    m_table->header()->setSectionResizeMode(column, m_paper ? QHeaderView::Fixed : QHeaderView::ResizeToContents);
    if (m_paper) m_table->header()->resizeSection(column, boardWidths[column]);
  }
  if (m_paper) {
    m_table->header()->setStretchLastSection(false);
    m_table->header()->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_table->setUniformRowHeights(true);
    layout->setContentsMargins(24, 24, 24, 16);
    layout->setSpacing(4);
    layout->addSpacing(8);
  }
  layout->addWidget(m_table, 1);
  if (m_paper) {  // the hairline over the buttons
    auto* foot = new QFrame(this);
    foot->setObjectName(QStringLiteral("referenceStatusFoot"));
    foot->setFixedHeight(16);
    layout->addSpacing(8);
    layout->addWidget(foot);
  }
  auto* buttons = new QDialogButtonBox(QDialogButtonBox::Close, this);
  auto* again = buttons->addButton(QStringLiteral("다시 확인"), QDialogButtonBox::ActionRole);
  again->setObjectName(QStringLiteral("referenceStatusRefresh"));
  connect(again, &QPushButton::clicked, this, &KaReferenceStatusDialog::refresh);
  connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::close);
  if (m_paper)  // 「다시 확인」·「닫기」 are plain words: neither is the window's one main action
    for (QAbstractButton* button : buttons->buttons())
      if (auto* push = qobject_cast<QPushButton*>(button)) push->setAutoDefault(false);
  layout->addWidget(buttons);
  refresh();
}

void KaReferenceStatusDialog::refresh() {
  m_table->clear();
  const auto items = ReferenceInventory::collect(m_project);
  m_summary->setText(ReferenceInventory::summary(items));
  for (const auto& item : items) {
    QString offline = ReferenceInventory::availabilityLabel(item.availability);
    if (!item.offlineNote.isEmpty()) offline += QStringLiteral(" · ") + item.offlineNote;
    auto* row = new QTreeWidgetItem(m_table, {item.name, item.kind, item.extentText, item.receivedText,
        ReferenceInventory::sizeLabel(item.bytes), offline});
    row->setData(0, Qt::UserRole, item.layerId);
    row->setData(5, Qt::UserRole, static_cast<int>(item.availability));
    row->setToolTip(0, item.location);
    row->setToolTip(5, item.location);
    if (!m_paper) continue;
    row->setTextAlignment(4, Qt::AlignRight | Qt::AlignVCenter);
    QFont figures = row->font(3);
    figures.setFamilies({QStringLiteral("Consolas"), QStringLiteral("JetBrains Mono")});
    for (int column : {3, 4}) row->setFont(column, figures);
    if (item.availability != ReferenceInventory::Availability::Offline) row->setIcon(5, warnMark());
  }
}
