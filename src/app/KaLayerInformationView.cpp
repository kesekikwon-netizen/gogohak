// The layer list view: column split, the five-row minimum and configureView (moved out
// of KaLayerInformation.cpp unchanged), plus the section-band aware row height and
// branch drawing that KaLayerSectionDelegate needs.
#include "KaLayerInformation.h"
#include "KaLayerSectionDelegate.h"
#include "KaTheme.h"

#include <QFont>
#include <QHeaderView>
#include <QScrollBar>
#include <QSplitter>
#include <QStyle>
#include <QToolButton>
#include <qgslayertree.h>
#include <qgslayertreeview.h>

namespace {
int labelColumnWidth(const QgsLayerTreeView* view) {
  return qMax(88, view->fontMetrics().horizontalAdvance(QStringLiteral("이름·면적")) +
      view->style()->pixelMetric(QStyle::PM_IndicatorWidth, nullptr, view) + 18);
}
}  // namespace

// The 10 px list row without a section band. Row 0 carries the first band, so its
// hint is taken back by the band height; an empty list falls back to the font.
int KaLayerInformationView::baseRowHeightOf(const QgsLayerTreeView* tree) {
  const int fallback = tree->fontMetrics().height() + 10;
  int hinted = tree->model() && tree->model()->rowCount() > 0 ? tree->sizeHintForRow(0) : 0;
  if (hinted > 0) {
    if (const auto* sections = qobject_cast<const KaLayerSectionDelegate*>(tree->itemDelegate()))
      hinted -= sections->bandHeight(tree->model()->index(0, 0));
  }
  return qMax(22, hinted > 0 ? hinted : fallback);
}

int KaLayerInformationView::minimumListHeight() const {
  const int row = baseRowHeight();
  const int head = header() ? qMax(header()->sizeHint().height(), fontMetrics().height() + 6) : row;
  return head + row * kMinVisibleRows + frameWidth() * 2;
}

void KaLayerInformationView::protectSidebarList(QSplitter* split, QgsLayerTreeView* tree,
                                                QToolButton* filesToggle, QWidget* filesPane,
                                                KaLayerInformationPanel* panel) {
  if (!split || !tree) return;
  const int row = baseRowHeightOf(tree);
  const int head = tree->header() ? qMax(tree->header()->sizeHint().height(),
                                         tree->fontMetrics().height() + 6)
                                  : row;
  const int need = head + row * kMinVisibleRows + tree->frameWidth() * 2;
  tree->setMinimumHeight(need);
  if (split->count() > 1) split->setCollapsible(1, true);
  if (filesPane) filesPane->setMinimumHeight(0);
  // A sidebar not shown yet (the 지도 tab is not the start page) has a stale size; its
  // Show event judges it again.
  if (!split->isVisible()) return;
  const int chrome = 48;
  const int handle = split->handleWidth();
  // With no layers the list hides behind the empty-state text; its stale size is no squeeze.
  const int treeH = tree->isHidden() ? 0 : tree->viewport() ? tree->viewport()->height() : tree->height();
  const bool filesOpen = filesToggle && filesToggle->isChecked();
  const bool tight = split->height() < need + chrome ||
                     (filesOpen && split->height() < need + chrome + 160) ||
                     (treeH > 0 && treeH < row * kMinVisibleRows);
  if (tight && panel) panel->collapseDetails();
  if (tight && filesToggle) filesToggle->setChecked(false);
  if (tight && filesPane) filesPane->hide();
  if (tight && split->count() >= 2) {
    const int layers = qMax(need + chrome, split->height() - handle);
    split->setSizes({layers, 0});
  }
  // A short column (the user's 3440×1440 at 150 % leaves ~700 px, a 1080p screen ~830): the list keeps ¾ and an open
  // 파일함 at most ¼ (2026-10-10). Taller columns keep the 3:2 share. Once per column height: a tab switch keeps
  // a 파일함 the user dragged larger.
  if (!tight && filesOpen && split->count() >= 2 && split->height() < 900 &&
      split->property("kaCappedHeight").toInt() != split->height()) {
    split->setProperty("kaCappedHeight", split->height());
    const QList<int> sizes = split->sizes();
    const int total = sizes.at(0) + sizes.at(1);
    if (sizes.at(1) * 4 > total) split->setSizes({total - total / 4, total / 4});
  }
}

// A 파일함 shut to 0 px (protectSidebarList, or MainWindow/leftSplit saved that way) came
// back as a 0 px strip: it opens at the 3:2 share the sidebar starts with. The splitter
// still keeps the layers card at its minimum, so a short window gives the 파일함 less.
void KaLayerInformationView::openSidebarFiles(QSplitter* split, QWidget* filesPane) {
  if (!split || !filesPane || split->count() < 2) return;
  filesPane->show();
  const QList<int> sizes = split->sizes();
  const int total = sizes.at(0) + sizes.at(1);
  const int files = total * 2 / 5;
  if (sizes.at(1) < files / 2) split->setSizes({total - files, files});
}

void KaLayerInformationView::resizeEvent(QResizeEvent* event) {
  const int layerWidth = m_columnsSized ? header()->sectionSize(0) : 0;
  QgsLayerTreeView::resizeEvent(event);
  // QGIS's one-column view makes EVERY header section at least viewport-wide.
  // Restore the user's split after that native resize hook, keeping both visible.
  header()->setMinimumSectionSize(24);
  const int maximumLayerWidth = qMax(24, viewport()->width() - labelColumnWidth(this));
  header()->setMaximumSectionSize(maximumLayerWidth);
  if (header()->count() == 2) {
    const int desiredWidth = m_columnsSized ? layerWidth : viewport()->width() - labelColumnWidth(this);
    header()->resizeSection(0, qBound(24, desiredWidth, maximumLayerWidth));
    m_columnsSized = true;
  }
  horizontalScrollBar()->setValue(0);
}

// The expand arrow of a band row sits beside the row text, below the band.
void KaLayerInformationView::drawBranches(QPainter* painter, const QRect& rect, const QModelIndex& index) const {
  int band = 0;
  if (const auto* sections = qobject_cast<const KaLayerSectionDelegate*>(itemDelegate()))
    band = sections->bandHeight(index);
  QgsLayerTreeView::drawBranches(painter, band > 0 ? rect.adjusted(0, band, 0, 0) : rect, index);
}

void KaLayerInformationModel::applyListFont(QgsLayerTreeView* view) {
  QFont compactFont = view->font();
  // 10 px rows (the list previously used 13 px; 2026-10-03). 큰 글자 grows them in the same ratio (2026-10-10).
  compactFont.setPixelSize(KaTheme::scaledUiPx(10));
  view->setFont(compactFont);
  if (auto* model = view->layerTreeModel()) {
    model->setLayerTreeNodeFont(QgsLayerTreeNode::NodeLayer, compactFont);
    compactFont.setBold(true);
    model->setLayerTreeNodeFont(QgsLayerTreeNode::NodeGroup, compactFont);
  }
}

void KaLayerInformationModel::configureView(QgsLayerTreeView* view) {
  if (auto* model = qobject_cast<KaLayerInformationModel*>(view->layerTreeModel())) view->installEventFilter(model);
  view->setProperty("kaLayerInformation", true);
  applyListFont(view);
  view->setAlternatingRowColors(true);
  view->setHeaderHidden(false);
  view->header()->setStretchLastSection(false);
  view->header()->setMinimumSectionSize(24);
  view->header()->setSectionResizeMode(0, QHeaderView::Interactive);
  view->header()->setSectionResizeMode(1, QHeaderView::Stretch);
  view->header()->resizeSection(0, qMax(24, view->viewport()->width() - labelColumnWidth(view)));
  view->header()->setToolTip(QStringLiteral("도면 · 레이어와 글자 사이 경계선을 끌어 너비를 조절하세요."));
  view->setMinimumWidth(300);
  // Keep the header and at least four compact rows usable in a short sidebar.
  view->setMinimumHeight(view->header()->sizeHint().height() + 4 * 22 + 4);
  view->setIndentation(12);
  if (KaTheme::displayOptions().paperLook) view->setIconSize(QSize(20, 14));  // 유구 표지 20 × 14
  view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  // Section bands make the first row of each section 16 px taller than the rest.
  view->setUniformRowHeights(false);
  if (!qobject_cast<KaLayerSectionDelegate*>(view->itemDelegate()))
    view->setItemDelegate(new KaLayerSectionDelegate(view->itemDelegate(), view));
}
