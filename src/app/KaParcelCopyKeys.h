#pragma once

#include <QList>
#include <QObject>

#include "core/ParcelClipboard.h"

class QEvent;
class QgsMapCanvas;

// Ctrl+C · Ctrl+V on the map while 도형선택 is the map tool. The canvas filters the keys, so a text
// field keeps its own Ctrl+C and Ctrl+V. The copy and paste work itself is in core/ParcelClipboard.
class KaParcelCopyKeys : public QObject {
  Q_OBJECT
 public:
  explicit KaParcelCopyKeys(QgsMapCanvas* canvas);

  // Copies the cadastral parcels picked now and returns how many. An empty pick keeps the old copy.
  int copyPicked();
  // Pastes what was copied at its own coordinates and returns how many features were added.
  int pasteCopied();

 signals:
  void statusMessage(const QString& text);

 protected:
  bool eventFilter(QObject* watched, QEvent* event) override;

 private:
  QgsMapCanvas* m_canvas = nullptr;
  QList<ParcelClipboard::Parcel> m_copied;
};
