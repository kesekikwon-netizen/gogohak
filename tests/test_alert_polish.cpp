// 세부 디자인 보완 (docs/intent/2026-10-11-design-detail-polish.md) 창 항목(D3): 실패 알림은 한 틀(KaUserError)이다.
// 「무엇이 안 됐나 · 왜 · 어떻게」가 한 모양으로 나오도록, 실패를 알리는 기본 경고창(QMessageBox::warning / critical)을
// 묶음마다 KaUserError::warn으로 옮겼다. 원본 소스 글을 읽어 남은 기본 경고창이 없음을 본다.
// 예외: QGIS 없는 빌드의 「QGIS 빌드 필요」 자리(실제 앱에서는 안 나온다)와 MainWindow::notify의 마지막 대화상자
// 분기(메시지 띠가 없을 때만).
#include <QtTest>

#include <QFile>

namespace {

// How many failure boxes in src/app/<file> still use the plain Qt dialog.
int plainAlerts(const QString& file, QStringList* where = nullptr) {
  QFile source(QStringLiteral("src/app/") + file);
  if (!source.open(QIODevice::ReadOnly | QIODevice::Text)) return -1;
  const QString text = QString::fromUtf8(source.readAll());
  int count = 0;
  for (const QString& call : {QStringLiteral("QMessageBox::warning("), QStringLiteral("QMessageBox::critical(")}) {
    for (int at = text.indexOf(call); at >= 0; at = text.indexOf(call, at + call.size())) {
      const QString window = text.mid(at, 260);
      if (window.contains(QStringLiteral("QGIS 빌드 필요")) || window.contains(QStringLiteral("this, title, body"))) continue;
      ++count;
      if (where) *where << QStringLiteral("%1:%2").arg(file).arg(text.left(at).count(QLatin1Char('\n')) + 1);
    }
  }
  return count;
}

void expectNone(const QStringList& files) {
  QStringList where;
  for (const QString& file : files) {
    const int found = plainAlerts(file, &where);
    QVERIFY2(found >= 0, qPrintable(file + QStringLiteral(": 파일을 읽지 못했습니다")));
  }
  QVERIFY2(where.isEmpty(), qPrintable(QStringLiteral("기본 경고창이 남아 있습니다: ") + where.join(QStringLiteral(", "))));
}

}  // namespace

class TestAlertPolish : public QObject {
  Q_OBJECT
private slots:
  // 첫째 묶음 — 저장·내보내기 (MainWindowSession · MainWindowExport).
  void saveAndExportAlertsUseTheOneFrame() {
    expectNone({QStringLiteral("MainWindowSession.cpp"), QStringLiteral("MainWindowExport.cpp")});
  }

  // 둘째 묶음 — 지도·정합·지적 (MainWindow · MainWindowAlign · MainWindowCadastral · MainWindowOverlay).
  void mapAlignAndCadastralAlertsUseTheOneFrame() {
    expectNone({QStringLiteral("MainWindow.cpp"), QStringLiteral("MainWindowAlign.cpp"), QStringLiteral("MainWindowCadastral.cpp"),
                QStringLiteral("MainWindowOverlay.cpp")});
  }
};

QTEST_APPLESS_MAIN(TestAlertPolish)
#include "test_alert_polish.moc"
