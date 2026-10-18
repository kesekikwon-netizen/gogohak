---
name: archaeo-studio-pro
description: Complete architectural blueprint, implementation guide, and Ph.D.-grade Korean archaeological excavation report ('Ⅳ. 고찰(考察)') & academic paper ('논문') automation methodology for building, deploying, or replicating ARCHAEO STUDIO 5.0 (WordCraft Native × Gemini Desktop Web '작업' Mode × DBpia AI × PdfCraft On-Demand Export) inside Antigravity on any Windows/macOS PC.
---

# 🏛️ ARCHAEO STUDIO 5.0 — Master Blueprint & Skill

## 1. Architecture Overview
- **Zero Antigravity Agent Token Consumption**: All prompt engineering (`1차 가공`), multi-round archaeological synthesis, table construction, chart/diagram generation, and bibliography cross-verification run through Antigravity's embedded **Google Gemini Web (`https://gemini.google.com/app`, '작업' mode / 4 Argon High)** and **DBpia AI** via CDP (`127.0.0.1:9223`) and local bridge (`http://127.0.0.1:18787`).
- **WordCraft-Centered Editing & On-Demand PDF (`PdfCraft`)**:
  - Primary workspace is **WordCraft (`.docx`)** (`wordcraft.exe` + `wordcraft-cli.exe`).
  - `wordcraft.exe` includes a native docked left panel (`crates/ui-egui/src/archaeo_studio.rs`) and title-bar `ARCHAEO STUDIO` toggle button, so the user can configure Steps 01–06 on the left while editing the live `.docx` document on the right.
  - Full-screen Web Studio (`http://127.0.0.1:18787/studio`) implements the exact uploaded `ARCHAEO STUDIO` design (`tokens.css`, `bundle.css`, `StudioRibbon`, `StudioStep` 01–06, `CollectStrip`, live A4 `DocPage` with bottom footnotes, `DbpiaWindow`, `ExportDialog`).
  - `PdfCraft` (`pdfcraft.exe`) is invoked **only on demand** when the user clicks `[PDF로 내보내기]`.

## 2. Core Korean Archaeological Manuscript Quality Engine (`gemini_mcp_server.js`)
1. **1st-Stage Prompt Engineering (`POST /first-stage-prompt`)**:
   - Converts raw excavation overview, feature overlap/stratigraphy, artifact attributes, and dating notes into a structured Korean archaeological research directive before drafting.
2. **Strict Non-Fabrication of Missing Data (`없는 자료는 없는 대로`)**:
   - If AMS/radiocarbon or natural-science analysis data is not provided, never fabricate BP numbers or percentages; explicitly state the absence of absolute dating data and rely on stratigraphy (`층위·유구 중복관계`) and typological relative chronology (`상대편년`).
3. **Stratigraphic & Feature-Overlap Priority (`유구 중복관계·층위 선후관계 최우선`)**:
   - Establishes chronological phases (`1기 → 2기 → 3기`) primarily from physical feature-cutting/superposition relationships before typological seriation.
4. **Mandatory Academic Tables (`<표 1>`, `<표 2>`)**:
   - `<표 1> 주요 유구·출토유물 속성 일람표`
   - `<표 2> 주변 병행유적 비교 대비표`
5. **Gemini-Direct Native WordCraft Charts (`wc-chart`) & Schematic Diagrams (`wc-diagram`)**:
   - Gemini emits `wc-chart` and `wc-diagram` JSON blocks directly in the manuscript.
   - `exportManuscriptToWordCraftAndPdfCraft()` converts `[[WC_CHART_n]]` into native OOXML `insert.chart` and `[[WC_DIAGRAM_n]]` into native OOXML `insert.smartArt` inside `.docx`.
6. **True Korean Archaeological Footnotes (`1)`, `2)`, `3)`) & 1:1 Bibliography Verification**:
   - Inline citations `1)`, `2)`, `3)` (including multiple footnotes on the same paragraph line) are converted into bottom-of-page OOXML footnotes via `select.text` + `references.footnote` (`mark: "1)"`) while preserving numbered subheadings (`1) 토기류`, `2) 철기류`).
   - Enforces `『보고서/학술지/단행본』`, `「학술논문」`, `00~00쪽` formatting and verifies 1:1 correspondence between inline footnotes and `【참고문헌】`.

## 3. Korean Font Fix (`□□□` Prevention)
- `.docx` documents always begin with `design.themeFonts={"heading":"Malgun Gothic","body":"Malgun Gothic"}`.
- `WordCraft` (`crates/ui-egui/src/theme.rs`, `crates/fonts/src/fontdb.rs`) and `PdfCraft` (`crates/ui-egui/src/system_fonts.rs`, `theme.rs`) load `C:\Windows\Fonts\malgun.ttf` (`Malgun Gothic`) as the primary Korean fallback font.

## 4. Cross-PC Migration (1-Click Setup)
- Run `새컴퓨터_자동설정.bat` from the cloned repository (`https://github.com/kesekikwon-netizen/gogohak.git`) on any new Windows PC with Antigravity + Node.js installed.
