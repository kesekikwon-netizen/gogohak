// KaTheme style sheet: token resolution and the embedded-first loader. The
// raw sheet is data/theme/ka-hgis.qss, compiled in as ka-hgis.qss.inc.
#include "KaTheme.h"
#include "KaThemeFonts.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QHash>
#include <QRegularExpression>

namespace KaTheme {
namespace {

QHash<QString, QString> replacementTable() {
  QHash<QString, QString> values;
  const auto& m = buttonMetrics();
  const struct { const char* name; int value; } metrics[] = {
      {"ribbonIconSize", m.ribbonIconSize}, {"ribbonFontSize", m.ribbonFontSize},
      {"ribbonChipWidth", m.ribbonChipWidth}, {"ribbonMinWidth", m.ribbonMinWidth},
      {"buttonPadding", m.buttonPadding},
      {"buttonSpacing", m.buttonSpacing}, {"ribbonChipGap", m.ribbonChipGap},
      {"ribbonGroupPad", m.ribbonGroupPad}, {"scaleButtonHeight", m.scaleButtonHeight},
      {"scaleButtonMinWidth", m.scaleButtonMinWidth}, {"scaleFontSize", m.scaleFontSize},
      {"layoutIconSize", m.layoutIconSize}, {"layoutButtonHeight", m.layoutButtonHeight},
      {"panelMargin", m.panelMargin}, {"uiFontSize", uiFontSize()},
      {"chipHeight", m.chipHeight}, {"chipRadius", m.chipRadius},
  };
  for (const auto& entry : metrics) values.insert(QString::fromLatin1(entry.name), QString::number(entry.value));
  const Tokens& c = tokens();
  // Copies, not references or pointers to members: MSVC /O2 once hit an ICE on
  // initializer lists pointing into a QColor aggregate.
  const struct { const char* name; QColor value; } colors[] = {
      {"accent", c.accent}, {"accentHover", c.accentHover}, {"accentDeep", c.accentDeep},
      {"accentWash", c.accentWash}, {"ink", c.ink}, {"inkMuted", c.inkMuted},
      {"inkDisabled", c.inkDisabled}, {"surface", c.surface}, {"desk", c.desk},
      {"altRow", c.altRow}, {"stripe", c.stripe}, {"hover", c.hover}, {"selected", c.selected}, {"pressed", c.pressed},
      {"disabledSurface", c.disabledSurface}, {"border", c.border}, {"borderStrong", c.borderStrong},
      {"edgeLight", c.edgeLight}, {"edgeDark", c.borderStrong}, {"danger", c.danger}, {"ok", c.ok},
      {"successSurface", c.successSurface}, {"dangerSurface", c.dangerSurface},
      {"warn", c.warn}, {"warnSurface", c.warnSurface}, {"rail", c.rail},
      {"railText", c.railText}, {"railMuted", c.railMuted}, {"progressFill", c.progressFill},
      {"focusRing", c.focusRing}, {"ribbonLabelInk", c.ribbonLabelInk}, {"ribbonActiveInk", c.ribbonActiveInk},
      {"ribbonGroupInk", c.ribbonGroupInk},
      {"primary", c.primary}, {"primaryHover", c.primaryHover}, {"primaryPressed", c.primaryPressed},
      {"primaryText", c.primaryText}, {"primaryFocus", c.primaryFocus}, {"tabInk", c.tabInk}, {"tabMark", c.tabMark}, {"chosenFill", c.chosenFill}, {"plateRule", c.plateRule}, {"selectedBar", c.selectedBar}, {"dialogGround", c.dialogGround}, {"chrome", c.chrome}, {"heroButton", c.heroButton},
      {"heroButtonText", c.heroButtonText}, {"heroButtonHover", c.heroButtonHover},
      {"heroButtonPressed", c.heroButtonPressed}, {"heroGhostHover", c.heroGhostHover},
      {"heroGhostPressed", c.heroGhostPressed}, {"band", c.band},
      // Legacy placeholders, kept so older sheets still resolve.
      {"glossMiddle", c.glossMiddle}, {"glossBottom", c.glossBottom}, {"hoverTop", c.hoverTop},
      {"hoverBottom", c.hoverBottom}, {"pressedTop", c.pressedTop}, {"pressedBottom", c.pressedBottom},
      {"selectedTop", c.selectedTop}, {"selectedBottom", c.selectedBottom},
      {"glossReflection", c.glossReflection}, {"glossShoulder", c.glossShoulder},
      {"accentReflection", c.accentReflection},
  };
  for (const auto& entry : colors)
    values.insert(QString::fromLatin1(entry.name), entry.value.name(QColor::HexRgb));
  // Font stacks (KaThemeFonts): quoted family lists for the append-only
  // font-family overrides at the end of ka-hgis.qss.
  values.insert(QStringLiteral("uiFont"), quotedFamilies(uiFontStack()));
  values.insert(QStringLiteral("monoFont"), quotedFamilies(monoFontStack()));
  // Names and headings (the last block of ka-hgis.qss). The stock values repeat what the sheet's
  // literal rules say; 새 모양 swaps in the serif title face and sets names a step larger.
  const bool paper = displayOptions().paperLook;
  values.insert(QStringLiteral("titleFont"), titleFontFamilies());
  values.insert(QStringLiteral("titleWeight"), QString::number(titleFontWeight()));
  values.insert(QStringLiteral("wordmarkFont"),
                paper ? titleFontFamilies() : quotedFamilies({QStringLiteral("Segoe UI"), QStringLiteral("Malgun Gothic")}));
  values.insert(QStringLiteral("wordmarkWeight"), paper ? QStringLiteral("500") : QStringLiteral("600"));
  values.insert(QStringLiteral("buttonEdge"), QString::number(buttonEdgeWidth()));
  values.insert(QStringLiteral("nameSize"), paper ? QStringLiteral("20") : QStringLiteral("15"));     // serif Heading
  values.insert(QStringLiteral("bigNameSize"), paper ? QStringLiteral("28") : QStringLiteral("20"));  // serif Title
  // Home section heads (최근 조사·작업 순서): the 15 px sans Lead in 새 모양, the title face in stock.
  values.insert(QStringLiteral("sectionFont"), paper ? quotedFamilies(uiFontStack()) : titleFontFamilies());
  values.insert(QStringLiteral("figureFont"), quotedFamilies(paper ? monoFontStack() : uiFontStack()));  // scale chips
  // Drawing tiles that are on: oat with a grey ring (legend, title) or an ink edge (chosen north).
  // Stock resolves to the same accent face the sample tiles already use.
  values.insert(QStringLiteral("sheetOnFace"), (paper ? c.tileShown : c.selected).name());
  values.insert(QStringLiteral("sheetOnInk"), (paper ? c.ink : c.accentDeep).name());
  values.insert(QStringLiteral("sheetOnEdge"), (paper ? c.tileShownBorder : c.accent).name());
  values.insert(QStringLiteral("northOnFace"), (paper ? c.chosenFill : c.selected).name());
  values.insert(QStringLiteral("northOnInk"), (paper ? c.ink : c.accentDeep).name());
  values.insert(QStringLiteral("northOnEdge"), (paper ? c.ink : c.accent).name());
  values.insert(QStringLiteral("sectionWeight"), paper ? QStringLiteral("400") : QString::number(titleFontWeight()));
  // The drawing row's tools (QToolBar#subToolbar): buttons in 새 모양, the flat row in the stock look.
  const QString none = QStringLiteral("transparent");
  values.insert(QStringLiteral("subFace"), c.surface.name());
  values.insert(QStringLiteral("subEdge"), paper ? c.border.name() : none);
  values.insert(QStringLiteral("subEdgeWidth"), paper ? QStringLiteral("1") : QStringLiteral("2"));
  values.insert(QStringLiteral("subRadius"), paper ? QStringLiteral("8") : QStringLiteral("0"));
  values.insert(QStringLiteral("subGap"), paper ? QStringLiteral("1") : QStringLiteral("0"));
  values.insert(QStringLiteral("subPadSide"), paper ? QStringLiteral("6") : QStringLiteral("7"));
  values.insert(QStringLiteral("mapRadius"), paper ? QStringLiteral("0") : QStringLiteral("10"));  // 지도는 그림: square in 새 모양
  // 홈 판 (2차 보드 판 2): no box around the home cards, a smaller wordmark.
  values.insert(QStringLiteral("cardFace"), paper ? QStringLiteral("transparent") : c.surface.name());
  values.insert(QStringLiteral("cardEdge"), paper ? QStringLiteral("transparent") : c.border.name());
  values.insert(QStringLiteral("heroNameSize"), paper ? QStringLiteral("34") : QStringLiteral("40"));
  values.insert(QStringLiteral("figureSize"), paper ? QStringLiteral("16") : QStringLiteral("13"));  // 수치 대장 (spec 20: 16 keeps 12,000,000.00 m² inside the 272 px panel)
  values.insert(QStringLiteral("subCheckedFace"), paper ? c.selected.name() : none);
  values.insert(QStringLiteral("subCheckedEdge"), paper ? c.accent.name() : none);
  values.insert(QStringLiteral("subCheckedEdgeWidth"), paper ? QStringLiteral("1") : QStringLiteral("2"));
  values.insert(QStringLiteral("subUnderline"),
                paper ? QStringLiteral("1px solid ") + c.accent.name() : QStringLiteral("2px solid ") + c.ribbonActiveInk.name());
  return values;
}

const QRegularExpression& tokenPattern() {
  static const QRegularExpression pattern(QStringLiteral("@([A-Za-z][A-Za-z0-9_]*)@"));
  return pattern;
}

}  // namespace

QString resolvedStyleSheet(const QString& sheet) {
  const QHash<QString, QString> values = replacementTable();
  QString resolved;
  resolved.reserve(sheet.size());
  qsizetype last = 0;
  auto it = tokenPattern().globalMatch(sheet);
  while (it.hasNext()) {
    const auto match = it.next();
    const auto value = values.constFind(match.captured(1));
    if (value == values.constEnd()) continue;
    resolved += QStringView(sheet).mid(last, match.capturedStart() - last);
    resolved += *value;
    last = match.capturedEnd();
  }
  resolved += QStringView(sheet).mid(last);
  return resolved;
}

QStringList unresolvedTokens(const QString& resolved) {
  QStringList names;
  auto it = tokenPattern().globalMatch(resolved);
  while (it.hasNext()) {
    const QString name = it.next().captured(1);
    if (!names.contains(name)) names.append(name);
  }
  return names;
}

QString embeddedStyleSheet() {
  return
#include "ka-hgis.qss.inc"
      ;
}

QStringList styleSheetCandidates() {
  const QString appDir = QCoreApplication::applicationDirPath();
  return {
      QDir(appDir).filePath(QStringLiteral("../data/theme/ka-hgis.qss")),
      QDir(appDir).filePath(QStringLiteral("data/theme/ka-hgis.qss")),
      QDir::current().filePath(QStringLiteral("data/theme/ka-hgis.qss")),
  };
}

bool styleSheetFromDiskRequested() {
  return qEnvironmentVariableIntValue("KA_HGIS_QSS_FROM_DISK") == 1;
}

QString loadStyleSheet() {
  if (!styleSheetFromDiskRequested()) {
    qInfo() << "KaTheme QSS using the embedded sheet";
    return embeddedStyleSheet();
  }
  for (const QString& p : styleSheetCandidates()) {
    QFile f(p);
    if (!f.exists())
      continue;
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
      qWarning() << "KaTheme QSS unreadable, skipping:" << p << f.errorString();
      continue;
    }
    const QString sheet = QString::fromUtf8(f.readAll());
    if (sheet.trimmed().isEmpty()) {
      qWarning() << "KaTheme QSS empty:" << p;
      continue;
    }
    qInfo() << "KaTheme QSS loaded from disk (KA_HGIS_QSS_FROM_DISK):" << p;
    return sheet;
  }
  qInfo() << "KaTheme QSS no disk copy found; using the embedded sheet";
  return embeddedStyleSheet();
}

QString completeStyleSheet(const QString& raw) {
  const QString sheet = resolvedStyleSheet(raw);
  const QStringList missing = unresolvedTokens(sheet);
  if (missing.isEmpty())
    return sheet;
  qWarning().noquote() << "KaTheme QSS has unknown tokens" << missing.join(QLatin1Char(','))
                       << "- using the embedded sheet";
  return resolvedStyleSheet(embeddedStyleSheet());
}

QString applicationStyleSheet() { return completeStyleSheet(loadStyleSheet()); }

}  // namespace KaTheme
