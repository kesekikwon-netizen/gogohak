#include "KaParcelCopyKeys.h"

#include "KaFeatureSelectTool.h"
#include "core/LayerOps.h"

#include <qgsmapcanvas.h>
#include <qgsproject.h>
#include <qgsvectorlayer.h>

#include <QEvent>
#include <QKeyEvent>
#include <QKeySequence>

KaParcelCopyKeys::KaParcelCopyKeys(QgsMapCanvas* canvas) : QObject(canvas), m_canvas(canvas) {
  if (m_canvas) m_canvas->installEventFilter(this);
}

bool KaParcelCopyKeys::eventFilter(QObject* watched, QEvent* event) {
  // 키는 도형선택 도구가 켜진 지도에서만 받는다. 글 입력칸의 Ctrl+C · Ctrl+V 는 입력칸이 가진다.
  if (event->type() == QEvent::KeyPress && qobject_cast<KaFeatureSelectTool*>(m_canvas->mapTool())) {
    auto* key = static_cast<QKeyEvent*>(event);
    // 키를 누르고 있어 오는 반복 입력으로는 다시 붙이지 않는다.
    if (key->isAutoRepeat() && (key->matches(QKeySequence::Copy) || key->matches(QKeySequence::Paste))) return true;
    if (key->matches(QKeySequence::Copy)) {
      copyPicked();
      return true;
    }
    if (key->matches(QKeySequence::Paste)) {
      pasteCopied();
      return true;
    }
  }
  return QObject::eventFilter(watched, event);
}

int KaParcelCopyKeys::copyPicked() {
  QList<QPair<QgsVectorLayer*, QgsFeatureId>> picked;
  for (const KaFeatureSelectTool::SelectedItem& item : KaFeatureSelectTool::allSelectedFeatures(m_canvas))
    picked.append({item.layer.data(), item.fid});
  const QList<ParcelClipboard::Parcel> copied = ParcelClipboard::copyFeatures(picked);
  if (copied.isEmpty()) {
    // 고른 것이 지적 필지가 아니면 이전에 담아 둔 것은 그대로 둔다.
    emit statusMessage(QStringLiteral("지적 필지를 도형선택으로 고른 뒤 Ctrl+C 하세요."));
    return 0;
  }
  m_copied = copied;
  emit statusMessage(QStringLiteral("지적 필지 %1개를 복사했습니다. Ctrl+V 하면 같은 자리에 붙습니다.")
                         .arg(static_cast<qlonglong>(copied.size())));
  return static_cast<int>(copied.size());
}

int KaParcelCopyKeys::pasteCopied() {
  QString error;
  const int added = ParcelClipboard::paste(QgsProject::instance(), m_copied, &error);
  if (added == 0) {
    emit statusMessage(error);
    return 0;
  }
  LayerOps::refreshCanvasIfIdle(m_canvas);
  emit statusMessage(QStringLiteral("필지 %1개를 같은 자리에 붙였습니다. 「지적도 · 복사한 필지」 줄에 있습니다.")
                         .arg(static_cast<qlonglong>(added)));
  return added;
}
