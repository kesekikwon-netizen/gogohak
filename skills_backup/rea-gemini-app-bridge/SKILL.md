---
name: rea-gemini-app-bridge
description: Universal blueprint and workflow for reverse-engineering any target website or desktop app using REA (morluto/rea) and building a zero-lag Standalone Desktop Application or Native Craft App (WordCraft/PdfCraft) that attaches to Antigravity's embedded Google Gemini ('작업' mode / 4 Argon High) web quota to autonomously design apps, collect & synthesize site data, or generate reports.
---

# 🔬 REA × Gemini '작업' Mode × Craft Native App Integration Blueprint

## 1. End-to-End Reverse Engineering & Bridge Architecture
1. **Inspect Target Website or Native Binary with REA**:
   - **Web Targets (Google Gemini Web, DBpia AI)**:
     - Discover CDP debugger targets from `http://127.0.0.1:9223/json` (or `DevToolsActivePort`).
     - Lock Gemini Web (`https://gemini.google.com/app`) to `4 Argon High` + `'작업'` mode (`gem-segmented-button[data-value="40"]`).
     - Extract DBpia AI (`https://ai.dbpia.co.kr`) answers and full KCI bibliography via both React DOM selectors (`[class*="ReferenceSources_referenceItem__"]`) and the REA-discovered internal Vetter JSON API (`GET https://agent.dbpia.co.kr/vetter_conversations/{convId}` with `credentials: 'include'`).
   - **Native Rust/C++ Desktop Targets (`WordCraft`, `PdfCraft`)**:
     - Inspect CLI command schemas (`wordcraft-cli.exe --help-json`), engine dispatch (`crates/engine/src/dispatch.rs`), UI crates (`crates/ui-egui`), and font database (`crates/fonts/src/fontdb.rs`).
     - Embed custom docked left studio panels (`crates/ui-egui/src/archaeo_studio.rs`) and patch Korean system fonts (`C:\Windows\Fonts\malgun.ttf`).

## 2. Critical Gemini `.ql-editor` Multi-Line Injection & Anti-Workspace Rules
1. **Never Pass `\n` into `document.execCommand('insertText')` on `.ql-editor`**:
   - Gemini Web's Quill editor intercepts the first `\n` inside `insertText` as an `Enter` key press and immediately submits only line 1, truncating the body and causing Gemini's Google Workspace extension to search Gmail / Google Drive for the missing text.
   - **Mandatory Multi-Line Injection Pattern**:
     ```javascript
     const rawText = String(fullPrompt || '');
     const escHtml = s => String(s).replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;');
     editor.innerHTML = rawText.split(/\r?\n/).map(line => '<p>' + (escHtml(line) || '<br>') + '</p>').join('');
     const sel = window.getSelection();
     const range = document.createRange();
     range.selectNodeContents(editor);
     range.collapse(false);
     sel.removeAllRanges();
     sel.addRange(range);
     document.execCommand('insertText', false, ' ');
     editor.dispatchEvent(new InputEvent('input', { bubbles: true, inputType: 'insertText', data: ' ' }));
     ```
2. **Prevent Unwanted Google Workspace (Gmail / Drive / Docs) Searches**:
   - Never ask Gemini to generate DBpia search queries before DBpia runs; build DBpia queries deterministically in JS (`buildDeterministicDbpiaQueries`) and call `askDbpiaAi(q)` directly.
   - Prepend an explicit Anti-Workspace guard (`@Google Workspace, @Gmail, @Google Drive, @Google Docs 호출 절대 금지`) at the top of the Master Prompt and start with `newChat: true`.

## 3. Structured OOXML Document Generation (`wordcraft-cli`)
- Set Korean theme fonts first: `design.themeFonts={"heading":"Malgun Gothic","body":"Malgun Gothic"}`.
- Convert `wc-chart` JSON blocks (`[[WC_CHART_n]]`) via `select.text={"text":"[[WC_CHART_n]]","story":"body"}` + `insert.chart`.
- Convert `wc-diagram` JSON blocks (`[[WC_DIAGRAM_n]]`) via `select.text={"text":"[[WC_DIAGRAM_n]]","story":"body"}` + `insert.smartArt`.
- Convert inline Korean footnotes (`1)`, `2)`, `3)`) via `select.text={"text":"[[WC_FN_n]]","story":"body"}` + `references.footnote={"text":"...","mark":"1)"}`.
- Export to PDF (`pdfcraft.exe` / `file.save={"path":"...pdf"}`) strictly on demand when requested by the user.
