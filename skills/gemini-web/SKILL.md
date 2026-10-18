---
name: gemini-web
description: >-
  Use this skill when the user wants to offload coding tasks, code generation,
  refactoring, or Ph.D.-grade Korean archaeological excavation report ('Ⅳ. 고찰')
  writing with DBpia AI and Google Docs live sync to the separate Gemini Desktop
  (gemini.google.com/app, '작업' mode) web quota via ARCHAEO STUDIO PRO 4.0.
---

# ARCHAEO STUDIO PRO 4.0 (`gemini-web` × `DBpia AI` × `Google Docs`) Skill

This skill integrates **Google Gemini Desktop (`gemini.google.com/app`, locked to `4 Argon High` + `작업` mode)**, **DBpia AI (`ai.dbpia.co.kr`, `persist:dbpia`)**, **OxCal/NRICH Publication Figure Studio (`<도면 1~4>`)**, and **Google Docs Live Editor (`persist:gemini-desktop`)** directly inside the Antigravity PC App (`Antigravity.exe`) via a Zinc-Obsidian enterprise workstation and MCP server (`gemini_mcp_server.js`).

> **다른 PC 이식 및 제작 방법론(Portable Blueprint)**: 전체 아키텍처 설계, 디버깅 불변 규칙(`Google Docs 중간 내보내기 멈춤 방지`, `model-response 단일 셀렉터`, `일시멈춤/즉시중단 하드 제어`), 박사급 고찰 집필 방법론 및 타 PC 자동 설치 스크립트는 [`archaeo-studio-pro` 스킬](file:///C:/Users/kyi25/.gemini/config/skills/archaeo-studio-pro/SKILL.md) 및 [`setup_portable.ps1`](file:///C:/Users/kyi25/.gemini/config/skills/archaeo-studio-pro/scripts/setup_portable.ps1)에 영구 보존되어 있습니다.

## Available MCP Tools

1. **`gemini_web_ask`**
   - Sends a prompt (and optional file paths for context) to the embedded `gemini.google.com/app` session in **`작업` (Work/Task)** mode and returns the markdown response.
2. **`gemini_web_status`**
   - Checks connection status, active Gemini model/mode (`4 Argon High + 작업 모드`), DBpia AI login status, and pipeline status (`running`, `paused`, `phase`, `latestGoogleDocUrl`).
3. **`gemini_web_toggle_panel`**
   - Shows or hides the **ARCHAEO STUDIO PRO 4.0** workstation panel (`Ctrl+Shift+G`) directly inside the Antigravity PC App window (`view_mode`: `"split"`, `"gemini"`, `"dbpia"`, `"figures"`, or `"gdocs"`).
4. **`gemini_web_new_chat`**
   - Resets the conversation in the embedded Gemini web session and locks `4 Argon High + 작업 모드`.
5. **`dbpia_open_login`**
   - Opens the embedded DBpia AI login view so the user can log into their institutional/paid DBpia account (`persist:dbpia`).
6. **`dbpia_ai_ask`**
   - Sends a research question directly to the embedded DBpia AI Agent (`ai.dbpia.co.kr`) and returns the full-text-grounded answer + cited KCI/thesis bibliography.
7. **`archaeology_auto_discussion`**
   - Runs the Ph.D.-grade Korean Archaeological Excavation Report 『Ⅳ. 고찰(考察)』 pipeline:
     - **`phd_consolidate` (Recommended Default)**: Synthesizes all survey data and DBpia references in the active thread into a disciplined, non-repetitive **Ph.D. Chief Researcher Master Manuscript (`Ⅰ. 머리말`, `Ⅱ. 유구 구조·단계 변천 [<표 1>]`, `Ⅲ. 출토유물·절대연대 교차검증 [<표 2, 3>]`, `Ⅳ. 주변 유적 비교·역사적 성격·참고문헌 [<표 4>]`)**, renders 4 publication-grade SVG figures (`<도면 1~4>`), and exports to **one unified Google Docs document** at completion without mid-pipeline stalls.
     - **`phd_full`**: Runs Phase 1–2 autonomous Gemini ↔ DBpia AI verification loop, then executes `phd_consolidate`.
     - **`phd_4chap`**: Writes Sections Ⅰ–Ⅳ sequentially with precision tables, then exports once to Google Docs at the end.
     - **`figures_only`**: Immediately extracts quantitative data and renders the 4 OxCal/NRICH publication figures (`<도면 1~4>`).
