---
name: archaeo-studio-pro
description: Complete architectural blueprint, UI/UX design system specification, implementation guide, and Ph.D.-grade Korean archaeological excavation report ('Ⅳ. 고찰(考察)') & academic paper ('논문') automation methodology for building, deploying, or replicating ARCHAEO STUDIO 5.0 (WordCraft Native × Gemini Desktop Web '작업' Mode × DBpia AI × PdfCraft On-Demand Export) inside Antigravity on any Windows/macOS PC.
---

# 🏛️ ARCHAEO STUDIO 5.0 — Master Blueprint & Complete Replication Skill

This skill contains the complete, zero-omission specification of **ARCHAEO STUDIO 5.0**. Any AI agent reading this skill can reconstruct the entire application — from UI design tokens and desktop icons to the CDP WebSocket bridge, DBpia Vetter API harvester, Anti-Gmail Workspace guard, and native Rust `WordCraft`/`PdfCraft` OOXML engines — without missing a single detail.

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
1. **Window Titlebar (`.as-winbar`, `34px`)**: Document title (`기안동 유적 Ⅳ 고찰.docx - ARCHAEO STUDIO`), `WordCraft 통합` badge, `WordCraft 데스크톱 열기 (.docx)` button, Light/Dark theme switch.
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
   - **Step ⑤ 실행**: Primary run buttons (`▶ 고찰 실행`, `▶ 논문 실행`), `[일시 멈춤]`, `[그만두기]`, `[디비피아 로그인]` + `● 로그인됨` pill, `[PDF로 내보내기]`.
   - **Step ⑥ 진행 기록**: Live progress bar (`0%~100%`) + timestamped step logs.
4. **Right A4 Document Canvas (`.as-screen__canvas`)**:
   - Top collection strip (`.as-collect`, `32px`): Live DBpia paper count, progress bar, `WordCraft 창 열기`.
   - Scrollable A4 Page (`.as-page`, `794px × 1123px`, `contenteditable="true"`): Live rendering of headings, tables, SVG charts/diagrams, inline citations `1)`, `2)`, and bottom footnote separator (`.as-page__notes`).
5. **Bottom Status Bar (`.as-status`, `26px`)**: Context hint, page counter (`쪽 1 / 1`), character counter (`글자 0`), zoom selector (`90% / 100% / 110%`).

---

## 3. Critical CDP Bridge Rules (`scripts/gemini_mcp_server.js`)

### 3.1 CRITICAL FIX #1: Multi-Line Prompt Injection into Gemini `.ql-editor` (Preventing First-Newline Auto-Submit)
- **Never** pass a multi-line string containing `\n` into `document.execCommand('insertText', false, fullPrompt)` on Gemini Web's Quill editor (`.ql-editor`). Quill intercepts the first `\n` as an `Enter` key press and immediately submits **only line 1**, discarding the rest of the prompt and causing Gemini to search Gmail/Google Drive for the missing text!
- **Mandatory Implementation (`_askGeminiWebImpl` & `injectTextIntoGeminiEditor`)**:
  ```javascript
  const rawText = String(fullPrompt || '');
  const escHtml = s => String(s).replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;');
  editor.innerHTML = rawText
    .split(/\r?\n/)
    .map(line => '<p>' + (escHtml(line) || '<br>') + '</p>')
    .join('');

  const sel = window.getSelection();
  const range = document.createRange();
  range.selectNodeContents(editor);
  range.collapse(false);
  sel.removeAllRanges();
  sel.addRange(range);
  document.execCommand('insertText', false, ' ');
  editor.dispatchEvent(new InputEvent('input', { bubbles: true, inputType: 'insertText', data: ' ' }));
  editor.dispatchEvent(new Event('change', { bubbles: true }));
  ```

### 3.2 CRITICAL FIX #2: 100% DBpia-Only Literature Search & Anti-Google Workspace Guard
1. **No Gemini Call in Step 2**: Step 2 must **never** call `askGeminiWeb` to generate DBpia queries. Instead, `buildDeterministicDbpiaQueries({ userFocus, reportText, docType })` extracts Korean site names, periods (`청동기`, `초기철기`, `원삼국`, `삼국시대`, etc.), and feature types (`주거지`, `목곽묘`, `석곽묘`, `구들`, `토기`) directly in JS and calls `askDbpiaAi(q)` on `https://ai.dbpia.co.kr`.
2. **Explicit Anti-Google Workspace Guard in `buildAdaptiveMasterPrompt()`**:
   Always prepend at the top of the Master Prompt:
   ```text
   ### [시스템 절대 규칙: 외부 Google Workspace(Gmail·Google Drive·Google Docs) 검색 및 호출 엄격 금지]
   - 절대 금지: @Google Workspace, @Gmail, @Google Drive, @Google Docs 등 개인 계정 확장 도구를 절대 호출하거나 검색하지 마십시오.
   - 즉시 집필 원칙: 이메일(Gmail)이나 구글 드라이브(Google Drive)에서 문서를 찾지 마십시오. 필요한 모든 조사자료와 DBpia 학술논문 서지 데이터는 바로 이 메시지 아래에 직접 포함되어 있으므로, 질문 없이 즉시 완성된 본문 원고 전체를 처음부터 끝까지 작성하십시오.
   ```
3. **Start Step 4 with `newChat: true`**: Prevents any prior conversation state from interfering with the manuscript generation.

### 3.3 DBpia AI Vetter JSON API Enrichment (`extractLatestDbpiaAnswer`)
After submitting a query on `https://ai.dbpia.co.kr/ko/search`, extract the conversation ID from `/search/{convId}` and fetch `https://agent.dbpia.co.kr/vetter_conversations/{convId}` with `{ credentials: 'include' }` inside the DBpia webview to retrieve complete JSON metadata (`authors`, `pub_year`, `title`, `journal_title`, `pages`, `kci_listed`, `node_id`, `abstract`).

---

## 4. 6-Pillar Korean Archaeological Quality & OOXML Compilation Pipeline
1. **1st-Stage Prompt Engineering (`buildFirstStageEngineeredPrompt`)**: Structures the request into NRICH (`발굴조사보고서 Ⅳ. 고찰`) or KAS (`『한국고고학보』 학술논문`) sections.
2. **Strict Non-Fabrication (`없는 자료는 없는 대로`)**: Never invent fake AMS BP dates or feature numbers if `datingText` or `reportText` is empty.
3. **Stratigraphy & Feature-Overlap Priority**: Prioritize physical superposition (`유구 중복·파괴 관계`) and soil stratigraphy (`토층 층위`).
4. **2 Mandatory Markdown Tables**: `<표 1. 주요 유구·출토유물 속성 일람표>` & `<표 2. 주변 병행유적 비교 대비표>`.
5. **Gemini-Generated `wc-chart` & `wc-diagram` Blocks**: Tokenized as `[[WC_CHART_n]]` and `[[WC_DIAGRAM_n]]` and compiled via `wordcraft-cli` `select.text` + `insert.chart` / `insert.smartArt`.
6. **True Korean Archaeological Footnotes (`1)`, `2)`) & 1:1 Bibliography Check**:
   - Distinguishes numbered subheadings (`1) 주거지 형태`) from inline citations (`...판단된다.1)`).
   - Replaces inline citations with `[[WC_FN_n]]` and runs `select.text={"text":"[[WC_FN_n]]","story":"body"}` + `references.footnote={"text":"...","mark":"1)"}`.
   - Automatically verifies 1:1 correspondence between `【각주】` and `【참고문헌】`, appending any missing cited reference to `【참고문헌】`.

---

## 5. Korean Font (`Malgun Gothic`) & Desktop Shortcut Setup
- `.docx` theme fonts: `design.themeFonts={"heading":"Malgun Gothic","body":"Malgun Gothic"}`.
- `WordCraft` & `PdfCraft` Rust UI: Load `C:\Windows\Fonts\malgun.ttf` (`SYSTEM_KOREAN_FALLBACK`) across all font families.
- Desktop Shortcut (`ARCHAEO STUDIO.lnk`): Points to `wscript.exe "$PluginDir\scripts\launch_archaeo_studio.vbs"` with multi-resolution icon `assets\archaeo-studio\archaeo_studio.ico`.
