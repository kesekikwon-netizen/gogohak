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
> - [`1_ARCHAEO_STUDIO_전체개발방법_설계도.md`](file:///C:/Users/kyi25/.gemini/config/plugins/gemini-desktop/1_ARCHAEO_STUDIO_전체개발방법_설계도.md)
> - [`2_ARCHAEO_STUDIO_타컴퓨터이식_및_향후확장_설계도.md`](file:///C:/Users/kyi25/.gemini/config/plugins/gemini-desktop/2_ARCHAEO_STUDIO_타컴퓨터이식_및_향후확장_설계도.md)
> - [`3_ARCHAEO_STUDIO_데이터수집_및_안정화_개선플랜.md`](file:///C:/Users/kyi25/.gemini/config/plugins/gemini-desktop/3_ARCHAEO_STUDIO_데이터수집_및_안정화_개선플랜.md)
> - [`archaeo-studio-pro` 스킬](file:///C:/Users/kyi25\.gemini\config\skills\archaeo-studio-pro\SKILL.md)

## Key Architectural Invariants
1. **CSP `TrustedHTML`-Safe + Blot-Tree-Fast `.ql-editor` `DocumentFragment` Injection**: Never call `document.execCommand('insertText', false, multiLineText)` on Gemini Web's `.ql-editor` (Quill treats the first `\n` as `Enter` immediate submit), never assign `editor.innerHTML = ...` (blocked by CSP `TrustedHTML`), and never create 2,000+ `<p>` nodes (which freezes Quill's Parchment Blot tree and causes `CDP command timeout: Runtime.evaluate`). Always group lines into ~50–80 paragraph blocks (~650 chars each) inserted via `document.createDocumentFragment()`, and poll every `1200ms` inside `try/catch`.
2. **Structured Excavation Report Parser (`extractStructuredArchaeologyReportFacts`) & Full DBpia Vetter Synthesis (`요점`)**: Parses 50~100+ page excavation report PDFs by preserving `Ⅰ. 조사 개요`, compressing textbook `Ⅱ. 주변 환경`, preserving 100% of `Ⅲ. 조사내용 (전 유구·중복관계·출토유물)` and `Ⅳ. 조사결과`, and summarizing `출토유물 목록` by feature. Queries DBpia AI using keywords ranked from `Ⅲ. 조사내용` and captures the full `messages[last].content` synthesis (`요점`, up to 4,500 chars) + all `hovers` (`NODE..._0`, `_1`, `_2`) + real authors/journals/pages (`enrichDbpiaReferencesMetadata`).
3. **`/goal`-Driven 8-Point Completion Rubric & Autonomous Cross-Verification Loop**: Establishes 8 explicit completion criteria (`buildGoalPlanAndRubric`) based on NRICH 『문화재발굴조사보고서 Ⅳ. 고찰』 or Master's/Ph.D. Thesis (『한국고고학보』) standards, deterministically repairs and maps all `1)`~`N)` inline citations to `### 【각주】` and `### 【참고문헌】` with zero `'저자미상'`, and loops until all 8 criteria pass (`8/8 충족 · 100점`).
4. **Multi-Page A4 Pagination (`- 1 -`, `- 2 -`, `- 3 -`...), Live DBpia Board, & WordCraft-First (`.docx`)**: Displays `#dbpia-live-board` during `PHASE_2_DBPIA_LOOP`, paginates the manuscript into discrete A4 pages with per-page bottom footnotes (`.as-page__notes`), and compiles native OOXML `.docx` in WordCraft.

## Available MCP Tools
1. **`gemini_web_ask`**: Sends a prompt (and optional file paths) to the embedded `gemini.google.com/app` session in **`작업`** mode.
2. **`gemini_web_status`**: Checks connection status, active Gemini model/mode, DBpia AI login status, and pipeline status.
3. **`gemini_web_toggle_panel`**: Shows or hides the embedded studio panel or launches the standalone `ARCHAEO STUDIO` window.
4. **`gemini_web_new_chat`**: Resets the conversation in the embedded Gemini web session and locks `4 Argon High + 작업 모드`.
5. **`dbpia_open_login`**: Opens the embedded DBpia AI login window (`ai.dbpia.co.kr`).
6. **`dbpia_ai_ask`**: Sends a research question directly to DBpia AI and extracts full answer + Vetter JSON API bibliography.
7. **`archaeology_auto_discussion`**: Runs the autonomous `/goal` 8-criteria Korean Archaeological Excavation Report 『Ⅳ. 고찰』 / 『논문』 pipeline and opens the compiled `.docx` in WordCraft.
