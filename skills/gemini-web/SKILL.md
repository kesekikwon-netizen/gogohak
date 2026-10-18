---
name: gemini-web
description: >-
  Use this skill when the user wants to offload coding tasks, code generation,
  refactoring, or Ph.D.-grade Korean archaeological excavation report ('Ⅳ. 고찰')
  & academic paper ('논문') writing with DBpia AI and WordCraft (.docx) / PdfCraft
  (.pdf) native compilation to the separate Gemini Desktop (gemini.google.com/app,
  '작업' mode) web quota via ARCHAEO STUDIO 5.0.
---

# ARCHAEO STUDIO 5.0 (`gemini-web` × `DBpia AI` × `WordCraft` × `PdfCraft`) Skill

This skill integrates **Google Gemini Desktop (`gemini.google.com/app`, locked to `4 Argon High` + `작업` mode)**, **DBpia AI (`ai.dbpia.co.kr`)**, **WordCraft v0.7.0 (`.docx` native OOXML editor with embedded ARCHAEO STUDIO left panel)**, and **PdfCraft v0.8.0 (on-demand `.pdf` exporter & PDF/OCR parser)** via the local HTTP/MCP bridge (`gemini_mcp_server.js`, port `18787`).

> **마스터 설계도 및 타 PC 이식 블루프린트**:
> - [`1_ARCHAEO_STUDIO_전체개발방법_설계도.md`](../../1_ARCHAEO_STUDIO_전체개발방법_설계도.md)
> - [`2_ARCHAEO_STUDIO_타컴퓨터이식_및_향후확장_설계도.md`](../../2_ARCHAEO_STUDIO_타컴퓨터이식_및_향후확장_설계도.md)
> - [`3_ARCHAEO_STUDIO_데이터수집_및_안정화_개선플랜.md`](../../3_ARCHAEO_STUDIO_데이터수집_및_안정화_개선플랜.md)
> - [`4_매장유산_발굴조사보고서_고찰과_학술논문_양식비교_및_DBpia_AI_전략.md`](../../4_매장유산_발굴조사보고서_고찰과_학술논문_양식비교_및_DBpia_AI_전략.md)
> - [`archaeo-studio-pro` 스킬](../archaeo-studio-pro/SKILL.md)

## Key Architectural Invariants
1. **CSP `TrustedHTML`-Safe + Blot-Tree-Fast `.ql-editor` `DocumentFragment` Injection**: Never call `document.execCommand('insertText', false, multiLineText)` on Gemini Web's `.ql-editor` (Quill treats the first `\n` as `Enter` immediate submit), never assign `editor.innerHTML = ...` (blocked by CSP `TrustedHTML`), and never create 2,000+ `<p>` nodes (which freezes Quill's Parchment Blot tree and causes `CDP command timeout: Runtime.evaluate`). Always group lines into ~50–80 paragraph blocks (~650 chars each) inserted via `document.createDocumentFragment()`, and poll every `1200ms` inside `try/catch`.
2. **Structured Excavation Report Parser (`extractStructuredArchaeologyReportFacts`) & Full DBpia Vetter Synthesis (`요점`)**: Parses 50~100+ page excavation report PDFs by preserving `Ⅰ. 조사 개요`, compressing textbook `Ⅱ. 주변 환경`, preserving 100% of `Ⅲ. 조사내용 (전 유구·중복관계·학술자문회의·출토유물)` and `Ⅳ. 조사결과`, and summarizing `출토유물 목록` by feature. Queries DBpia AI using keywords ranked from `Ⅲ. 조사내용` and captures the full `messages[last].content` synthesis (`요점`, up to 4,500 chars) + all `hovers` (`NODE..._0`, `_1`, `_2`) + real authors/journals/pages (`enrichDbpiaReferencesMetadata`).
3. **`/goal`-Driven 8-Point Completion Rubric, Multi-Visual Enricher (`archaeology_visuals_enricher.js`), & Autonomous Cross-Verification Loop**: Establishes 8 explicit completion criteria (`buildGoalEvaluationPlan`) based on 「매장유산 보호 및 조사에 관한 법률」 [별표 2] 『발굴조사보고서 Ⅳ. 고찰』 (Site-bound 귀납적 4대 영역) or 『한국고고학보』/『역사학보』 『학술논문』 (Problem-bound 연역적 5단계 & '사료 세포' 재분류). Enriches every manuscript with **4+ comparative tables (`<표 1>`~`<표 4>`), 3+ comparison charts (`wc-chart` `<그림 1>`~`<그림 3>`), and 3+ schematic diagrams (`wc-diagram` `<모식도 1>`~`<모식도 3>`)** grounded strictly in report & DBpia AI data, preserves valid footnotes without modulo duplication, and sorts `### 【참고문헌】` in Korean alphabetical order (`가나다순`).
4. **Multi-Page A4 SVG Chart/Diagram Rendering & WordCraft Live Reload (`control-key.local-7981`)**: Renders interactive SVG bar/column charts and multi-stage schematic diagrams on the Studio A4 canvas (`renderManuscriptOntoA4Page`), compiles native OOXML charts (`insert.chart`), SmartArt diagrams (`insert.smartArt`), and footnotes (`references.footnote`), and live-reloads the running `wordcraft.exe` window via `%APPDATA%\WordCraft\control-key.local-7981` over TCP port `7981`.
5. **100% Universal Site/Topic Agnosticism, WordCraft (`.docx`) Export Integrity & Zero Legacy UI**:
   - **Zero Site-Specific Hardcoding**: Never include site-specific conditional branches (such as `isByeongsan`) or hardcoded sample data in `archaeology_visuals_enricher.js` or `gemini_mcp_server.js`. All tables, charts (`wc-chart`), and diagrams (`wc-diagram`) must be 100% dynamically derived from the currently uploaded/input report facts, the Gemini manuscript, and live DBpia AI references, working identically for any site, period, or academic topic.
   - **Direct WordCraft (`.docx`) Pipeline**: Never route through or display the legacy `ARCHAEO STUDIO PRO 4.0 (Google Docs)` overlay; always compile directly to WordCraft (`.docx`) and keep Gemini Web locked to `'작업'` mode (`gem-segmented-button[data-value="40"]`).
   - **Safe Footnote Tokenization**: In `prepareMarkdownAndFootnotesForWordCraft`, never match digits or `(` before `${num})` (`(?<![0-9(])`) so parenthesized years like `(2010)` are never corrupted into `[[WC_FN_10]]`, and never substitute footnote tokens inside Markdown table rows (`| ... |`).
   - **Schema & Table Normalization**: Deterministically normalize `wc-diagram` specs with `nodes`/`edges`/`steps` (and `코드 스니펫` / `json` fences) into `items: [{text, level}]`, repair collapsed single-line Markdown tables (`repairCollapsedMarkdownTables`), and pass `title: ""` to `insert.chart` to prevent SVG title clipping and duplication with `<그림 n>` captions.

## Available MCP Tools
1. **`gemini_web_ask`**: Sends a prompt (and optional file paths) to the embedded `gemini.google.com/app` session in **`작업`** mode.
2. **`gemini_web_status`**: Checks connection status, active Gemini model/mode, DBpia AI login status, and pipeline status.
3. **`gemini_web_toggle_panel`**: Shows or hides the embedded studio panel or launches the standalone `ARCHAEO STUDIO` window.
4. **`gemini_web_new_chat`**: Resets the conversation in the embedded Gemini web session and locks `4 Argon High + 작업 모드`.
5. **`dbpia_open_login`**: Opens the embedded DBpia AI login window (`ai.dbpia.co.kr`).
6. **`dbpia_ai_ask`**: Sends a research question directly to DBpia AI and extracts full answer + Vetter JSON API bibliography.
7. **`archaeology_auto_discussion`**: Runs the autonomous `/goal` 8-criteria Korean Archaeological Excavation Report 『Ⅳ. 고찰』 / 『논문』 pipeline and opens the compiled `.docx` in WordCraft.
