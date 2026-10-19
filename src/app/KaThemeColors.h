#pragma once
#include "KaTheme.h"

#include <QList>
#include <QString>

// 화면 테마 (2026-10-19, 「테마 ▾」): 하늘 눈밭, 번개 노랑 and 벚꽃 파스텔 rewrite the chrome tokens of the
// stock or 새 모양 look. Map symbols, the map canvas and every saved or exported file keep their colours:
// those live in src/core (LayoutService, LayerStyleDefaults) and never read KaTheme.
namespace KaTheme {

// Default first, in menu order.
QList<ColorTheme> colorThemes();
// The ui/colorTheme setting value ("default", "sky", "bolt", "blossom").
QString colorThemeKey(ColorTheme theme);
// Unknown or empty keys are Default.
ColorTheme colorThemeFromKey(const QString& key);
QString colorThemeLabel(ColorTheme theme);
// Rewrites the chrome colours of t for theme (Default leaves t alone). The caller refreshes the legacy aliases.
void applyColorTheme(Tokens& t, ColorTheme theme, bool highContrast);

}  // namespace KaTheme
