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

### 3.1 CRITICAL FIX #1: Multi-Line Prompt Injection into Gemini `.ql-editor` (TrustedHTML-Safe + Newline-Safe + Quill Blot-Tree Fast)
- **Never** pass a multi-line string containing `\n` into `document.execCommand('insertText', false, fullPrompt)` on Gemini Web's Quill editor (`.ql-editor`) — Quill intercepts the first `\n` as an `Enter` key press and immediately submits **only line 1**, discarding the rest of the prompt!
- **Never** assign `editor.innerHTML = ...` on `gemini.google.com/app` — it enforces CSP `TrustedHTML`.
- **Never** create 2,000+ individual `<p>` nodes (one per PDF hard line-wrap), which freezes Quill's Parchment Blot tree and causes `CDP command timeout: Runtime.evaluate`. Instead, group consecutive lines into ~50–80 paragraph blocks (~650 chars each, keeping headings, JSON specs, and code fences on separate blocks) and insert once via `document.createDocumentFragment()`, with `1200ms` polling wrapped in `try/catch`!

### 3.2 CRITICAL FIX #2: Structured PDF Excavation Report Parser (`extractStructuredArchaeologyReportFacts`) & Pure DBpia Queries
1. **Zero-Truncation Section-Aware PDF Parsing (`extractStructuredArchaeologyReportFacts`)**: When a 50~100+ page excavation report PDF is uploaded, never blindly slice `.slice(0, 30000)` (which wastes 10,000 chars on textbook `Ⅱ. 자연·지리적 환경` and cuts off the `출토유물 목록` at the end of the report). Instead, segment the report into:
   - `Ⅰ. 조사 개요` (preserved)
   - `Ⅱ. 주변 환경` (compressed to key surrounding archaeological sites only, ~1,500 chars)
   - `Ⅲ. 조사 내용` (100% preserved — all feature dimensions, plan shapes like `凸자형`/`방형`, internal facilities like `아궁이`/`부뚜막`/`노지`, stratigraphy, and feature overlaps like `3호 ↔ 3-1호 주거지`)
   - `Ⅳ. 조사결과 및 조사단 의견` (100% preserved)
   - `출토유물 목록` (grouped by feature count + representative artifact specifications)
2. **Accurate Keyword Extraction from `Ⅲ. 조사내용`**: `buildMultiFacetedDbpiaQueries()` ranks period keywords (`철기시대`, `원삼국`, `삼국시대`, etc.), feature types (`凸자형 주거지`, `아궁이`, `부뚜막`, `노지`), and artifact types (`경질무문토기`, `타날문토기`, `심발형토기`, `시루`, `외반구연호`) specifically from `Ⅲ. 조사내용` and `Ⅳ. 조사결과` rather than the regional background history in `Ⅱ. 주변 환경`.
3. **Never Include `@Google Workspace`, `@Gmail`, `@Google Drive`, or `Google` Keywords in Prompts**: Even negative mentions trigger the Google Drive connection popup.

### 3.3 DBpia AI Vetter JSON API Full Synthesis (`요점`) & Multi-Hover Quotes (`extractLatestDbpiaAnswer` & `enrichDbpiaReferencesMetadata`)
After submitting a query on `https://ai.dbpia.co.kr/ko/search`, extract the conversation ID from `/search/{convId}` and fetch `https://agent.dbpia.co.kr/vetter_conversations/{convId}` with `{ credentials: 'include' }` inside the DBpia webview:
- **Full Synthesis (`요점`)**: Always prioritize `messages[last].content` (3,000~4,500 chars of structured academic synthesis) over DOM `[class*="useMarkdownComponents_"]` (which matches individual `<p>`/`<li>` elements and would otherwise truncate to the last `<li>` of ~160 chars).
- **All Hover Quotes (`hovers`)**: Join all hover entries (`NODE..._0`, `NODE..._1`, `NODE..._2`, etc.) for each `node_id` so every cited passage from the paper (up to 950 chars per paper) is captured.
- **Bibliography Enrichment**: `messages[last].references` provides `node_id`, `node_title`, `node_autr`, `node_pbsh_yy`, `node_plct_nm`, and `node_iprd_nm`, enriched in parallel via `articleDetail?nodeId=NODE...` (`citation_volume`, `citation_issue`, `citation_firstpage`, `citation_lastpage`) and OpenAlex batch API.
- **Studio Multi-Page A4 Pagination & Live DBpia Board (`standalone_studio_app.js`)**: Renders `#dbpia-live-board` during `PHASE_2_DBPIA_LOOP` so the user sees live DBpia queries, `요점` summaries, and verified paper cards in real time, and paginates the manuscript into discrete A4 pages (`- 1 -`, `- 2 -`, `- 3 -`...) with per-page bottom footnotes (`.as-page__notes`).

---

## 4. `/goal`-Driven 8-Point Completion Criteria, Legal `[별표 2]` vs Academic Paper (`사료 세포`) Paradigm & Multi-Visual Enricher
Instead of stopping after a single 1-shot Q&A, `runArchaeologyDiscussionPipeline()` establishes an explicit **8-Point Completion & Cross-Verification Rubric (`buildGoalEvaluationPlan`)** upfront based on:
- **Excavation Report (`docType === "report"`)**: 「매장유산 보호 및 조사에 관한 법률」 시행규칙 제9조 [별표 2] & 국립문화유산연구원 보고서 평가 지표 (공간 구속적 Site-bound · 귀납적 4대 고찰 영역: 일러두기 기술 표준[도북/자북·해발고도·유물 일련번호 1:1 매칭·잔존치 `( )`], 학술자문회의 의견 반영, 층위·파괴/절치 중복 관계 분기[分期], 유구·유물 정량 비교 및 정비·복원 제언).
- **Academic Paper (`docType === "paper"`)**: 『한국고고학보』/『역사학보』 동료 심사(Peer Review) 규격 (문제 구속적 Problem-bound · 연역적 5단계: ① 서론 문제 제기 및 비판적 연구사 검토 ➔ ② 본론 '사료 세포[표제어·출처·수집정보·가공정보]' 재분류 및 핵심 개념어·논지 도출 ➔ ③ 결론 신규 사료 추가 금지·문제 재확인·근거 요약, 참고문헌 한국어 가나다순 → 중·일·서양어 정렬).

All 8 criteria are verified and deterministically enriched (`repairAndEnrichManuscriptWithVerifiedRefs` + `scripts/archaeology_visuals_enricher.js`):
1. **`C1_DBPIA_SUFFICIENCY`**: Multi-faceted DBpia AI research (`vetter_conversations` full `요점` + `hovers` verbatim quotes + `articleDetail` KCI metadata).
2. **`C2_SECTION_STRUCTURE`**: Complete mandatory sections (`머리말 ➔ 유구·자료 분석 ➔ 출토유물·편년 단계 설정 ➔ 주변 비교유적·고고학적 의의 ➔ 맺음말 ➔ 국·영문 초록 ➔ 【각주】 ➔ 【참고문헌】`).
3. **`C3_ARGUMENT_DEPTH`**: Target character length (`6,500~7,500+` chars) and formal Korean archaeological argumentation endings (`~로 파악된다`, `~로 편년된다`, `~로 판단된다`).
4. **`C4_QUANTITATIVE_TABLES` (4+ Comparative Tables)**:
   - `<표 1>` 조사구역 확인 유구 제원·내부시설·중복관계·출토유물 속성 일람표
   - `<표 2>` 주요 출토유물 기종별·형식별 제작기법 속성 및 선행연구 상대편년 비교표
   - `<표 3>` 주변 병행유적(DBpia 수집 문헌 기반 동일 문화권 핵심 유적군) 유구·유물 정밀 대비표
   - `<표 4>` DBpia AI 수집 핵심 선행연구(실제 저자·연도·논문명) 학술 쟁점별 논거 및 본 유적 적용 종합 대비표
5. **`C5_CHART_AND_DIAGRAM` (3+ `wc-chart` Comparison Charts & 3+ `wc-diagram` Schematic Diagrams)**:
   - `<그림 1>` (`wc-chart` #1): 본 유적 및 주변 비교유적군 시대별 확인 유구 비교 차트
   - `<그림 2>` (`wc-chart` #2): 조사구역 주요 주거지 유구별 평면 규모(장축·단축 m 및 잔존 면적 ㎡) 정량 비교 차트
   - `<그림 3>` (`wc-chart` #3): DBpia AI 수집 선행연구 주제 영역별 인용 논문 수 및 교차검증 비교 지표 분포 차트
   - `<모식도 1>` (`wc-diagram` #1): 조사구역 퇴적 층위(Ⅰ~Ⅴ층) 및 지반 보강·점토 다짐·벽구·화덕 축조 공정 모식도
   - `<모식도 2>` (`wc-diagram` #2): 유구 중복 관계 및 출토유물 조합에 기반한 단계별(Ⅰ~Ⅲ단계) 취락·물질문화 변천 모식도
   - `<모식도 3>` (`wc-diagram` #3): DBpia 수집 문헌 교차검증 기반 지역 취락망 공간 네트워크 및 문화 교류 모식도
   - Both rendered as interactive SVG multi-series charts & visual process diagrams on the Studio A4 canvas (`renderManuscriptOntoA4Page`) AND compiled as native OOXML `<c:chart>` (`insert.chart`) and `<dgm:relIds>` (`insert.smartArt`) inside WordCraft `.docx`.
6. **`C6_VERIFIED_FOOTNOTES`**: Numbered Korean footnotes (`1)`~`N)`) with **0 occurrences of `'저자미상'` or `'연도미상'`** and **0 modulo duplication** (preserves all valid footnotes without wrapping around).
7. **`C7_BIBLIOGRAPHY_CROSSCHECK`**: Deterministic 1:1 synchronization between inline `1)`~`N)`, `### 【각주】`, and `### 【참고문헌】` (sorted in Korean alphabetical order `가나다순`, using actual uploaded report titles instead of hardcoded placeholder reports).
8. **`C8_DATA_FIDELITY_COMPLETENESS`**: Strict non-fabrication (`없는 자료는 없는 대로` — never invent fake AMS BP dates or fake phase-by-phase pottery counts; all charts and tables use only verified excavation report and DBpia AI numbers).

---

## 5. WordCraft Live Control (`control-key.local-7981`), Korean Font (`Malgun Gothic`) & Desktop Shortcut
- **WordCraft Live TCP Control (`sendCommandToRunningWordCraft`)**: `wordcraft.exe --control 7981` writes its authentication key to `%APPDATA%\WordCraft\control-key.local-7981` (checked alongside `control-key.7981`) and streams newline-delimited JSON over `127.0.0.1:7981`. Always buffer `sock.on("data")` chunks until `\n` before checking `"ok":true`.
- **Template-Literal Script Escaping (`standalone_studio_app.js`)**: Because `getStandaloneStudioHtml()` returns a template literal, any newline inside inline JS strings must be double-escaped (`\\n`) and verified with `new vm.Script(scriptBody)` in CI.
- `.docx` theme fonts: `design.themeFonts={"heading":"Malgun Gothic","body":"Malgun Gothic"}`.
- Desktop Shortcut (`ARCHAEO STUDIO.lnk`): Points to `wscript.exe "$PluginDir\scripts\launch_archaeo_studio.vbs"` with multi-resolution icon `assets\archaeo-studio\archaeo_studio.ico`.

