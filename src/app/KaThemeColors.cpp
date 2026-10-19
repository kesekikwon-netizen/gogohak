#include "KaThemeColors.h"

namespace KaTheme {
namespace {

// One theme's chrome colours. Light washes carry ink text; accent and tileOn carry white text.
// Each pair is checked in test_theme_options.cpp (colorTheme_inksStayReadable).
struct ThemeColors {
  QColor ink, inkMuted, inkMutedHigh, inkDisabled;
  QColor desk, altRow, stripe, hover, selected, pressed, disabledSurface, chrome;
  QColor border, borderStrong, borderHigh, borderStrongHigh;
  QColor accent, accentHover, accentDeep, accentWash;
  QColor rail, railMuted;
  QColor tile, tileHover, tileOn, glyph, tileShown, tileShownBorder;
};

// 하늘 눈밭: snow-white cards on a pale sky desk, a sky-blue band and a deep sky accent.
ThemeColors skySnow() {
  ThemeColors c;
  c.ink = QColor(0x12, 0x25, 0x36);
  c.inkMuted = QColor(0x46, 0x5A, 0x6C);
  c.inkMutedHigh = QColor(0x33, 0x47, 0x5A);
  c.inkDisabled = QColor(0x56, 0x63, 0x6F);
  c.desk = QColor(0xEA, 0xF4, 0xFC);
  c.altRow = QColor(0xF5, 0xFA, 0xFE);
  c.stripe = QColor(0xED, 0xF5, 0xFC);
  c.hover = QColor(0xE3, 0xF0, 0xFB);
  c.selected = QColor(0xCD, 0xE5, 0xF7);
  c.pressed = QColor(0xB9, 0xDA, 0xF3);
  c.disabledSurface = QColor(0xEB, 0xF0, 0xF4);
  c.chrome = QColor(0xF7, 0xFB, 0xFE);
  c.border = QColor(0xC6, 0xDA, 0xEA);
  c.borderStrong = QColor(0x9A, 0xB7, 0xCE);
  c.borderHigh = QColor(0x6F, 0x8D, 0xA6);
  c.borderStrongHigh = QColor(0x5E, 0x7C, 0x95);
  c.accent = QColor(0x1C, 0x6B, 0xA6);
  c.accentHover = QColor(0x17, 0x5B, 0x8E);
  c.accentDeep = QColor(0x12, 0x4A, 0x75);
  c.accentWash = QColor(0xDC, 0xEE, 0xFA);
  c.rail = QColor(0xCD, 0xE7, 0xF8);
  c.railMuted = QColor(0x33, 0x49, 0x5D);
  c.tile = QColor(0xE0, 0xEE, 0xF8);
  c.tileHover = QColor(0xD0, 0xE5, 0xF4);
  c.tileOn = c.accent;
  c.glyph = QColor(0x22, 0x42, 0x5B);
  c.tileShown = c.selected;
  c.tileShownBorder = QColor(0x86, 0xAB, 0xC8);
  return c;
}

// 번개 노랑: a yellow band and washes with dark ink (white never sits on yellow), red for the filled button.
ThemeColors boltYellow() {
  ThemeColors c;
  c.ink = QColor(0x1F, 0x1A, 0x0A);
  c.inkMuted = QColor(0x54, 0x47, 0x0E);
  c.inkMutedHigh = QColor(0x40, 0x36, 0x0A);
  c.inkDisabled = QColor(0x5E, 0x54, 0x34);
  c.desk = QColor(0xFF, 0xF7, 0xD1);
  c.altRow = QColor(0xFF, 0xFB, 0xE8);
  c.stripe = QColor(0xFF, 0xF5, 0xC7);
  c.hover = QColor(0xFF, 0xF1, 0xB8);
  c.selected = QColor(0xFF, 0xE4, 0x7A);
  c.pressed = QColor(0xFF, 0xD8, 0x4A);
  c.disabledSurface = QColor(0xF3, 0xEE, 0xDA);
  c.chrome = QColor(0xFF, 0xFB, 0xE8);
  c.border = QColor(0xEA, 0xD6, 0x88);
  c.borderStrong = QColor(0xC7, 0xAA, 0x36);
  c.borderHigh = QColor(0x93, 0x80, 0x1F);
  c.borderStrongHigh = QColor(0x7D, 0x6C, 0x14);
  c.accent = QColor(0xC2, 0x26, 0x1F);
  c.accentHover = QColor(0xA6, 0x1F, 0x1A);
  c.accentDeep = QColor(0x8A, 0x19, 0x15);
  c.accentWash = QColor(0xFB, 0xE2, 0xDE);
  c.rail = QColor(0xFF, 0xD8, 0x4A);
  c.railMuted = QColor(0x40, 0x36, 0x0A);
  c.tile = QColor(0xFF, 0xF1, 0xB8);
  c.tileHover = QColor(0xFF, 0xE4, 0x7A);
  c.tileOn = c.accent;
  c.glyph = QColor(0x3A, 0x30, 0x0C);
  c.tileShown = c.selected;
  c.tileShownBorder = QColor(0xB3, 0x9A, 0x2E);
  return c;
}

// 벚꽃 파스텔: a blossom-pink desk and washes, a lavender band and chosen rows, a deep rose button.
ThemeColors blossomPastel() {
  ThemeColors c;
  c.ink = QColor(0x2B, 0x1A, 0x2B);
  c.inkMuted = QColor(0x66, 0x46, 0x62);
  c.inkMutedHigh = QColor(0x4F, 0x34, 0x50);
  c.inkDisabled = QColor(0x68, 0x5A, 0x6A);
  c.desk = QColor(0xFC, 0xEE, 0xF5);
  c.altRow = QColor(0xFE, 0xF6, 0xFA);
  c.stripe = QColor(0xFB, 0xEF, 0xF5);
  c.hover = QColor(0xF8, 0xE5, 0xF0);
  c.selected = QColor(0xEB, 0xDA, 0xF6);
  c.pressed = QColor(0xDD, 0xC8, 0xEF);
  c.disabledSurface = QColor(0xF3, 0xEC, 0xF1);
  c.chrome = QColor(0xFE, 0xF6, 0xFA);
  c.border = QColor(0xEB, 0xD2, 0xE1);
  c.borderStrong = QColor(0xC8, 0xA4, 0xBF);
  c.borderHigh = QColor(0x9B, 0x75, 0x93);
  c.borderStrongHigh = QColor(0x86, 0x62, 0x80);
  c.accent = QColor(0xA1, 0x37, 0x6F);
  c.accentHover = QColor(0x8A, 0x2F, 0x5F);
  c.accentDeep = QColor(0x72, 0x26, 0x4F);
  c.accentWash = QColor(0xF6, 0xDC, 0xEA);
  c.rail = QColor(0xE8, 0xDC, 0xF7);
  c.railMuted = QColor(0x53, 0x3F, 0x63);
  c.tile = QColor(0xF6, 0xE5, 0xF0);
  c.tileHover = QColor(0xEE, 0xD9, 0xEA);
  c.tileOn = QColor(0x6A, 0x4A, 0x99);
  c.glyph = QColor(0x4A, 0x2F, 0x50);
  c.tileShown = c.selected;
  c.tileShownBorder = QColor(0xA9, 0x91, 0xC4);
  return c;
}

}  // namespace

QList<ColorTheme> colorThemes() {
  return {ColorTheme::Default, ColorTheme::SkySnow, ColorTheme::BoltYellow, ColorTheme::BlossomPastel};
}

QString colorThemeKey(ColorTheme theme) {
  switch (theme) {
    case ColorTheme::SkySnow: return QStringLiteral("sky");
    case ColorTheme::BoltYellow: return QStringLiteral("bolt");
    case ColorTheme::BlossomPastel: return QStringLiteral("blossom");
    case ColorTheme::Default: break;
  }
  return QStringLiteral("default");
}

ColorTheme colorThemeFromKey(const QString& key) {
  for (const ColorTheme theme : colorThemes())
    if (colorThemeKey(theme) == key) return theme;
  return ColorTheme::Default;
}

QString colorThemeLabel(ColorTheme theme) {
  switch (theme) {
    case ColorTheme::SkySnow: return QStringLiteral("하늘 눈밭");
    case ColorTheme::BoltYellow: return QStringLiteral("번개 노랑");
    case ColorTheme::BlossomPastel: return QStringLiteral("벚꽃 파스텔");
    case ColorTheme::Default: break;
  }
  return QStringLiteral("기본");
}

// Named assignments like strataTokens(). Danger, ok and warn keep the base look's pairs; the map
// canvas (canvasNeutral) stays white because the map is a picture, not chrome.
void applyColorTheme(Tokens& t, ColorTheme theme, bool highContrast) {
  if (theme == ColorTheme::Default) return;
  const ThemeColors c = theme == ColorTheme::SkySnow ? skySnow() : theme == ColorTheme::BoltYellow ? boltYellow() : blossomPastel();
  const QColor white(0xFF, 0xFF, 0xFF);
  // The base look's shape choices: 새 모양 marks a chosen row without a bar and rules tables in ink.
  const bool rowBar = t.selectedBar != t.selected;
  const bool inkRules = t.plateRule != t.surface;
  t.accent = c.accent;
  t.accentHover = c.accentHover;
  t.accentDeep = c.accentDeep;
  t.accentWash = c.accentWash;
  t.ink = c.ink;
  t.inkMuted = highContrast ? c.inkMutedHigh : c.inkMuted;
  t.inkDisabled = c.inkDisabled;
  t.surface = white;
  t.desk = c.desk;
  t.altRow = c.altRow;
  t.stripe = c.stripe;
  t.hover = c.hover;
  t.selected = c.selected;
  t.pressed = c.pressed;
  t.disabledSurface = c.disabledSurface;
  t.border = highContrast ? c.borderHigh : c.border;
  t.borderStrong = highContrast ? c.borderStrongHigh : c.borderStrong;
  t.rail = c.rail;  // a light band: the home hero reads in ink
  t.railText = c.ink;
  t.railMuted = c.railMuted;
  t.progressFill = c.ink;
  t.focusRing = c.ink;
  t.ribbonLabelInk = c.ink;
  t.ribbonActiveInk = c.accentDeep;
  t.ribbonGroupInk = t.inkMuted;
  t.primary = c.accent;
  t.primaryHover = c.accentHover;
  t.primaryPressed = c.accentDeep;
  t.primaryText = white;
  t.primaryFocus = c.ink;
  t.tabInk = c.accentDeep;
  t.tabMark = c.accent;
  t.chosenFill = c.selected;
  t.plateRule = inkRules ? c.ink : white;
  t.selectedBar = rowBar ? c.accent : c.selected;
  t.dialogGround = c.altRow;
  t.chrome = c.chrome;
  t.heroButton = c.accent;
  t.heroButtonText = white;
  t.heroButtonHover = c.accentHover;
  t.heroButtonPressed = c.accentDeep;
  t.heroGhostHover = c.tileHover;
  t.heroGhostPressed = c.pressed;
  t.band = c.rail;
  t.tile = c.tile;
  t.tileHover = c.tileHover;
  t.tileOn = c.tileOn;
  t.tileOnBorder = c.tileOn;
  t.tileDisabled = c.desk;
  t.tileStrong = c.accent;
  t.glyph = c.glyph;
  t.glyphOn = white;
  t.tileShown = c.tileShown;
  t.tileShownBorder = c.tileShownBorder;
  t.glyphShown = c.ink;
}

}  // namespace KaTheme
