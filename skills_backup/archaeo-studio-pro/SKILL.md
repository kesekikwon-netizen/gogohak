---
name: archaeo-studio-pro
description: Complete architectural blueprint, UI/UX design system specification, implementation guide, and Ph.D.-grade Korean archaeological excavation report ('Ⅳ. 고찰(考察)') & academic paper ('논문') automation methodology for building, deploying, or replicating ARCHAEO STUDIO 6.1 (Harness Supervisor × Evidence Store F/R/D × V1~V8 & X1~X4 Cross-Verification × Claude 2nd-Pass Handoff × Final DBpia AI Review × Nano Banana Pro Visuals × WordCraft/PdfCraft Native) inside Antigravity on any Windows/macOS PC.
---

# 🏛️ ARCHAEO STUDIO 6.1 — Master Blueprint & Complete Replication Skill

This skill contains the complete, zero-omission specification of **ARCHAEO STUDIO 6.1** (building upon 5.0). Any AI agent reading this skill can reconstruct the entire application — from UI design tokens, Nano Banana Pro archaeological SVG/OOXML visual renderers, and desktop icons to the `scripts/v2/` Harness Supervisor, Evidence Store (`F-*`/`R-*`/`D-*`), `V1~V8` deterministic verifiers, `X1~X4` independent cross-verification conversations, Claude 2nd-pass handoff, final DBpia AI literature review, and native Rust `WordCraft`/`PdfCraft` OOXML engines.

---

## 1. System Architecture & Zero-Quota Principle
- **0% Antigravity Agent Quota Usage**: All prompt engineering (`1차 가공`), DBpia literature synthesis, table/chart/diagram generation, and manuscript writing run through Antigravity's embedded **Google Gemini Web (`https://gemini.google.com/app`, `4 Argon High` + `작업` mode)** and **DBpia AI (`https://ai.dbpia.co.kr`)** via Chrome DevTools Protocol (`127.0.0.1:9223` / `DevToolsActivePort`) and local HTTP bridge (`http://127.0.0.1:18787`).
- **WordCraft-Centered Editing (`.docx`) & On-Demand PDF (`PdfCraft`)**:
  - All editing, tables (`<표 1>`, `<표 2>`), native charts (`wc-chart`), phase diagrams (`wc-diagram`), and bottom-of-page footnotes (`1)`, `2)`) happen inside **WordCraft (`.docx`)**.
  - `wordcraft.exe` embeds a native docked left panel (`crates/ui-egui/src/archaeo_studio.rs`) and title-bar `ARCHAEO STUDIO` toggle button, connected to `127.0.0.1:18787` and live-reloaded via TCP control port `7981`.
  - `PdfCraft` (`pdfcraft.exe` / `pdfcraft-cli.exe`) is used in the background for extracting text/ONNX OCR from uploaded `.pdf` excavation reports and is only opened on screen when the user explicitly clicks **[PDF로 내보내기]**.

---

## 2. Complete UI/UX Design System (`tokens.css` & 5-Zone Workstation Layout)

### 2.1 Color Palette & Typography Tokens (`assets/archaeo-studio/tokens.css`)
- **Light Mode (Warm Hanji / Parchment)**:
  - `--as-bg-app: #f6f2ea`, `--as-bg-panel: #efe9dc`, `--as-bg-elev: #fbf8f1`, `--as-bg-subtle: #e7dfd0`, `--as-bg-canvas: #e5dec9`, `--as-bg-page: #fbf9f4`
  - `--as-ink-1: #1f1b16`, `--as-ink-2: #4c443a`, `--as-ink-3: #786e60`, `--as-ink-4: #9b9080`
  - `--as-border-1: #ded6c5`, `--as-border-2: #c9beaa`, `--as-border-focus: #8d4b32`
- **Dark Mode (Excavation Stratigraphy Dark)**:
  - `--as-bg-app: #161411`, `--as-bg-panel: #1d1a16`, `--as-bg-elev: #24201b`, `--as-bg-subtle: #2b2620`, `--as-bg-canvas: #12100d`, `--as-bg-page: #1f1c17`
  - `--as-ink-1: #ede6d8`, `--as-ink-2: #bfb6a5`, `--as-ink-3: #8c8374`
- **Semantic Accents (Strictly 3 Functional Accents)**:
  - **Terracotta Clay (`--as-accent: #a2492a` light / `#c96846` dark)**: Used ONLY on the single primary execution button matching the active document mode (`고찰 실행` or `논문 실행`).
  - **Bronze Brass (`--as-brass: #8a6a1f` light / `#cfa54a` dark)**: Used for `[1차 가공]` badge and `[디비피아 로그인]` status.
  - **Sage Green (`--as-ok: #2f6b4a` light / `#5aa47b` dark)**: Used for active collection dot and progress bar fill.
- **Typography**:
  - `--font-sans: "Pretendard Variable", "Pretendard", "Malgun Gothic", "맑은 고딕", sans-serif`
  - `--font-serif: "Noto Serif KR", "Source Han Serif KR", "Batang", "바탕", serif`
  - `--font-mono: "JetBrains Mono", "Consolas", monospace`

### 2.2 5-Zone Workstation Layout (`scripts/standalone_studio_app.js`)
1. **Window Titlebar (`.as-winbar`, `34px`)**: Document title (`가상유적 Ⅳ 고찰.docx - ARCHAEO STUDIO`), `WordCraft 통합` badge, `WordCraft 데스크톱 열기 (.docx)` button, Light/Dark theme switch.
2. **WordCraft Ribbon (`.as-ribbon`, `62px`)**:
   - `파일`: `새 문서`, `열기`, `저장 (.docx)`
   - `홈 · 서식`: `글자` (Serif ↔ Sans-Serif toggle)
   - `삽입`: `표`, `도면`, `각주`
   - `스튜디오`: `스튜디오` (Toggle left 380px panel)
   - `내보내기`: `PDF 변환` (Open `ExportDialog`)
3. **Left 6-Step Studio Panel (`.as-screen__studio`, `380px` width)**:
   - **Step ① 문서 종류**: 2-segment switch (`발굴조사보고서 고찰` vs `학술논문`).
   - **Step ② 요청**: Focus topic textarea + `[1차 가공]` brass tag + collapsible `▸ 가공된 요청 보기` preview box.
   - **Step ③ 내 보고서·조사 자료**: Excavation data textarea + `[파일 불러오기]` (`.docx`, `.pdf`, `.txt`, `.md`, `.csv` upload with automatic `PdfCraft`/`WordCraft` text & ONNX OCR extraction + removable file chips).
   - **Step ④ 연대분석·보유 자료 (`선택`)**: AMS/OSL dating textarea + `비워 두면 빼고 작성` badge.
   - **Step ⑤ 실행**: Primary run buttons (`▶ 고찰 실행`, `▶ 논문 실행`), `[일시 멈춤]`, `[그만두기]`, `[디비피아 로그인]` + `● 로그인됨` pill, `[PDF로 내보내기]`, and 6.1 Claude 2nd-pass handoff controls (`[Claude 2차 다듬기용 내보내기]`, `[Claude 2차 원고 가져오기]`).
   - **Step ⑥ 진행 기록**: Live progress bar (`0%~100%`) + timestamped step logs + `G1~G7` gate badges.
4. **Right A4 Document Canvas (`.as-screen__canvas`)**:
   - Top collection strip (`.as-collect`, `32px`): Live DBpia paper count, progress bar, `WordCraft 창 열기`.
   - Scrollable A4 Page (`.as-page`, `794px × 1123px`, `contenteditable="true"`): Live rendering of headings, tables, Nano Banana Pro SVG charts/diagrams, inline citations `1)`, `2)`, and bottom footnote separator (`.as-page__notes`).
5. **Bottom Status Bar (`.as-status`, `26px`)**: Context hint, page counter (`쪽 1 / 1`), character counter (`글자 0`), zoom selector (`90% / 100% / 110%`).

---

## 3. Critical CDP Bridge Rules (`scripts/gemini_mcp_server.js`)

### 3.1 CRITICAL FIX #1: Multi-Line Prompt Injection into Gemini `.ql-editor` (TrustedHTML-Safe + Newline-Safe + Quill Blot-Tree Fast)
- **Never** pass a multi-line string containing raw newlines into `execCommand` `insertText` on Gemini Web's Quill editor (`.ql-editor`) — Quill intercepts the first newline as an `Enter` key press and immediately submits **only line 1**, discarding the rest of the prompt!
- **Never** assign directly to `innerHTML` on `gemini.google.com/app` — it enforces CSP `TrustedHTML`.
- **Never** create 2,000+ individual `<p>` nodes (one per PDF hard line-wrap), which freezes Quill's Parchment Blot tree and causes `CDP command timeout: Runtime.evaluate`. Instead, use `injectTextIntoGeminiEditor()` which groups consecutive lines into ~50–80 paragraph blocks (~650 chars each, keeping headings, JSON specs, and code fences on separate blocks) and inserts once via `document.createDocumentFragment()`, with `1200ms` polling wrapped in `try/catch`!

### 3.2 CRITICAL FIX #2: Structured PDF Excavation Report Parser (`extractStructuredArchaeologyReportFacts`) & Pure DBpia Queries
1. **Zero-Truncation Section-Aware PDF Parsing (`extractStructuredArchaeologyReportFacts` & `scripts/v2/page_extractor.js`)**: When a 50~100+ page excavation report PDF is uploaded, never blindly slice `.slice(0, 30000)`. Instead, segment the report by page (`P1` `page_extractor.js`) and section:
   - `Ⅰ. 조사 개요` (preserved)
   - `Ⅱ. 주변 환경` (compressed to key surrounding archaeological sites only, ~1,500 chars)
   - `Ⅲ. 조사 내용` (100% preserved — all feature dimensions, plan shapes like `凸자형`/`방형`, internal facilities like `아궁이`/`부뚜막`/`노지`, stratigraphy, and feature overlaps like `3호 ↔ 3-1호 주거지`)
   - `Ⅳ. 조사결과 및 조사단 의견` (100% preserved)
   - `출토유물 목록` (grouped by feature count + representative artifact specifications)
2. **Accurate Keyword Extraction from Facts (`scripts/v2/query_builder.js`)**: Ranks period keywords (`철기시대`, `원삼국`, `삼국시대`, etc.), feature types (`凸자형 주거지`, `아궁이`, `부뚜막`, `노지`), and artifact types (`경질무문토기`, `타날문토기`, `심발형토기`, `시루`, `외반구연호`) directly from extracted fact records (`F-*`).
3. **Never Include Workspace/Drive/Mail Extension Trigger Keywords in Prompts**: Even negative mentions trigger external extension connection popups.

### 3.3 DBpia AI Vetter JSON API Full Synthesis (`요점`) & Multi-Hover Quotes (`extractLatestDbpiaAnswer` & `enrichDbpiaReferencesMetadata`)
After submitting a query on `https://ai.dbpia.co.kr/ko/search`, extract the conversation ID from `/search/{convId}` and fetch `https://agent.dbpia.co.kr/vetter_conversations/{convId}` with `{ credentials: 'include' }` inside the DBpia webview:
- **Full Synthesis (`요점`)**: Always prioritize `messages[last].content` (3,000~4,500 chars of structured academic synthesis) over DOM `[class*="useMarkdownComponents_"]`.
- **All Hover Quotes (`hovers`)**: Join all hover entries (`NODE..._0`, `NODE..._1`, `NODE..._2`, etc.) for each `node_id` so every cited passage from the paper (up to 950 chars per paper) is captured.
- **Bibliography Enrichment**: `messages[last].references` provides `node_id`, `node_title`, `node_autr`, `node_pbsh_yy`, `node_plct_nm`, and `node_iprd_nm`, enriched in parallel via `articleDetail?nodeId=NODE...` (`citation_volume`, `citation_issue`, `citation_firstpage`, `citation_lastpage`) and OpenAlex batch API.

---

## 4. ARCHAEO STUDIO 6.1 (`scripts/v2/`) — Harness Supervisor, Evidence Store (`F/R/D`), `V1~V8` + `X1~X4` Cross-Verification, Claude 2nd-Pass Handoff & Final DBpia AI Review

### 4.1 Harness Layer (`scripts/v2/harness/`)
- **`preflight.js`**: Pre-run diagnostic verifying CDP connection, Gemini tab (`4 Argon High` + `'작업'` mode lock), DBpia AI login state, `pdfcraft-cli.exe` / `wordcraft-cli.exe` binaries, and workspace write permissions.
- **`dom_Contract.js`**: Versioned DOM selector and API contract for Gemini (`inputEditor`, `sendButton`, `streamingIndicator`, `responseTurn`, `newChatButton`, `modelSelector`, `modeWorkButton`) and DBpia (`searchTextarea`, `vetterApiPattern`, `deepResearchApiPattern`).
- **`supervisor.js`**: Wraps all external calls (`gemini_ask`, `dbpia_ask`, `pdfcraft_extract`, `wordcraft_compile`) with timeouts, exponential backoff retries, `ensureGeminiEffortLock` (set-then-read verification of `4 Argon High` + `'작업'` mode), and `<<<END>>>` end-marker truncation detection + continuation recovery.
- **`checkpoint.js` & `run_logger.js`**: Atomic per-stage (`P1`~`P9`) state persistence (`checkpoint.json`) for crash recovery and structured JSONL run logging (`logs/run.jsonl`).

### 4.2 Evidence Store (`scripts/v2/evidence_store.js`) & 9-Stage Pipeline (`P1~P9`, `scripts/v2/index.js`)
1. **`P1` Page-Preserving Extraction (`page_extractor.js`) & Fact Extraction (`fact_extractor.js`, Gate `G1`)**:
   - Registers every archaeological feature, artifact, metric, overlap, and stratigraphy record as `F-0001` (with mandatory `source_page` and `verbatim_quote`) and dating samples as `D-01`.
2. **`P2` Deterministic DBpia Query Builder (`query_builder.js`) & Literature Cards (`literature_cards.js`, Gate `G2`)**:
   - Converts verified DBpia papers into `R-001` cards (`authors`, `year`, `title`, `journal`, `volume`, `issue`, `pages`, `verbatim_quotes`, `claims`, `completeness`).
3. **`P3` Section Outline Planner (`outline_planner.js`, Gate `G3`)**:
   - Assigns specific `fact_ids`, `ref_ids`, and visual specs (`required_tables`, `required_charts`, `required_diagrams`) to each section (`S1`~`S5`).
4. **`P4` Section-by-Section Writing (`section_writer.js`) & Fact-Derived Visuals (`visuals_from_facts.js`, Gate `G4`)**:
   - Calls Gemini once per section using only that section's assigned `F-*`/`R-*`/`D-*` cards.
   - Every quantitative metric in prose must carry `[F-xxxx]` or `[D-xx]`, and every scholarly claim must carry `[R-xxx]` (`markers.js`).
   - All tables, `wc-chart` blocks, and `wc-diagram` blocks are generated deterministically from `F-*` and `R-*` records (`visuals_from_facts.js`).
5. **`P5~P6` Deterministic Verification (`V1~V8`, `verify/deterministic.js`) × Independent Cross-Verification (`X1~X3`, `verify/cross_verifier.js`) × Consensus (`X4`, `verify/consensus.js`, Gate `G5`)**:
   - `V1`: Zero unregistered `[F-*]`/`[R-*]`/`[D-*]` markers.
   - `V2`: Zero unmarked quantitative measurements in prose.
   - `V3`: Zero fabricated BP/cal BC/cal AD/AMS/OSL dates when `D-*` is empty.
   - `V4`: 1:1 bijective mapping between inline `[R-*]`, footnotes, and bibliography (`completeness: "full"`).
   - `V5`: 100% match between table/chart/diagram numbers and `F-*` facts.
   - `V6`: Zero forbidden keywords or placeholders (`저자미상`, `연도미상`, `00쪽`).
   - `V7`: Section length and formal Korean academic sentence endings (`~로 판단된다`, `~로 편년된다`, `~로 파악된다`).
   - `V8`: Direct verification of `verbatim_quote` against `pages.json`.
   - `X1~X3`: Opens 3 separate Gemini verification chats (`X1` Fact vs Prose, `X2` Reference Quote vs Claim, `X3` Peer-Review Rubric) and merges with `V1~V8` via `X4` Consensus (`passed`, `needs_repair`, `contested`).
6. **`P7` Claude 2nd-Pass Handoff (`handoff.js`, Gate `G6`)**:
   - Exports `handoff/` bundle (`01_draft_marked.md`, `02_facts.json`, `03_references.json`, `04_verification_report.json`, `05_claude_instructions.md`) and re-verifies imported `06_claude_revised_marked.md` through `V1~V8`.
7. **`P7.5` Final DBpia AI Literature Review (`verify/dbpia_review.js`, Gate `G7`)**:
   - Queries DBpia AI with core manuscript claims to detect `contradicted` claims or `missing_must_cite` papers (`dbpia_final_review.json`).
8. **`P8~P9` Final Assembly (`assembler.js`), Crosswalk (`crosswalk.js`), Regional Comparative Site Library (`site_library.js`), and WordCraft (`.docx`) Compilation**:
   - Strips `[F-xxxx]` markers, converts `[R-xxx]` into sequential Korean footnotes `1)`, `2)` and sorted `【참고문헌】`, generates `crosswalk.md`, and compiles `.docx`.

---

## 5. Nano Banana Pro Archaeological Visual Renderer (`scripts/archaeology_visuals_enricher.js`)
- `renderPublicationGradeChartSvg` (`wc-chart`) and `renderPublicationGradeDiagramSvg` (`wc-diagram`) implement the **Nano Banana Pro Archaeological Design System**:
  - Multi-stop SVG `<defs>` gradients (`nbTerracottaGrad`, `nbBronzeGrad`, `nbSageGrad`, `nbSlateGrad`, `nbCanvasGrad`), fine grid background pattern (`nbGridPattern`), and soft card/node elevation drop shadows (`nbCardShadow`, `nbNodeShadow`).
  - Architectural header banner (`NANO BANANA PRO · ARCHAEOLOGICAL DESIGN SYSTEM`), high-contrast data value pills, orthogonal/bezier directional connectors, and hierarchical phase cards (`Ⅰ단계`, `Ⅱ단계`, `Ⅲ단계` with sub-attribute nodes).

---

## 6. WordCraft (`.docx`) Export Integrity, Portable Setup & Chapter 14 SDLC Gate (`scripts/v2/tools/check.js`)
1. **100% Universal Site & Topic Agnosticism — Zero Hardcoded Site Branches**:
   - Never write site-specific branches (`isByeongsan`, etc.) or hardcoded site tables/charts/diagrams.
2. **Parenthesized-Year-Safe & Table-Safe Footnote Tokenization (`prepareMarkdownAndFootnotesForWordCraft`)**:
   - Never include digits (`0-9`) or `(` in the character class preceding `${num})` (`(?<![0-9(])`), and never substitute `[[WC_FN_n]]` tokens inside Markdown table rows (`| ... |`).
3. **Mechanical SDLC Gate (`node scripts/v2/tools/check.js`, `.githooks/pre-commit`, `.github/workflows/v6-check.yml`)**:
   - Enforces syntax checks, 33 unit tests (`node --test tests/`), CI inline checks, protected path checks (`bin/`, `patches/`, `factory/`), forbidden pattern checks, and `skills/` === `skills_backup/` synchronization.

