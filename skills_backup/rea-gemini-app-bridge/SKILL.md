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

## 2. Critical Gemini `.ql-editor` Multi-Line Injection & Pure DBpia Rules
1. **Never Pass `\n` into `document.execCommand('insertText')`, Never Assign `editor.innerHTML`, and Never Create 2,000+ `<p>` Nodes on `.ql-editor`**:
   - Gemini Web's Quill editor intercepts the first `\n` inside `insertText` as an `Enter` key press and immediately submits only line 1, `gemini.google.com` enforces CSP `TrustedHTML` which throws on `editor.innerHTML = ...`, and creating 2,000+ `<p>` nodes (one per PDF line wrap) freezes Quill's Parchment Blot tree (`CDP command timeout: Runtime.evaluate`).
   - **Mandatory TrustedHTML-Safe + Blot-Tree-Fast `DocumentFragment` Injection Pattern**:
     ```javascript
     const rawText = String(fullPrompt || '');
     while (editor.firstChild) editor.removeChild(editor.firstChild);
     const rawLines = rawText.split(/\r?\n/);
     const groupedBlocks = [];
     let buf = [];
     let bufLen = 0;
     for (const rawLine of rawLines) {
       const line = rawLine.trim();
       if (!line) {
         if (buf.length > 0) { groupedBlocks.push(buf.join(' ')); buf = []; bufLen = 0; }
         continue;
       }
       const isTableLine = line.startsWith('|');
       const prevWasTable = buf.length > 0 && buf[0].startsWith('|');
       if (isTableLine && prevWasTable) {
         buf.push(line);
         bufLen += line.length + 1;
         continue;
       }
       const isCodeOrFence = line.startsWith('```') || (line.startsWith('{') && line.endsWith('}'));
       const isBlockStart = isCodeOrFence || isTableLine || /^(?:#{1,6}\s|\d+[.)]|[-•*]|【|▶|■|◈|[ⅠⅡⅢⅣⅤⅥⅦⅧ1-9][.\s]|[가나다라마바사아][.\s]|\(\d+\)|\[관점|\[제공된)/.test(line);
       if (isBlockStart || bufLen + line.length > 650) {
         if (buf.length > 0) { groupedBlocks.push(buf.join(prevWasTable ? '\n' : ' ')); buf = []; bufLen = 0; }
       }
       if (isCodeOrFence) groupedBlocks.push(line);
       else { buf.push(line); bufLen += line.length + 1; }
     }
     if (buf.length > 0) groupedBlocks.push(buf.join(buf[0].startsWith('|') ? '\n' : ' '));
     const frag = document.createDocumentFragment();
     for (const blk of groupedBlocks) {
       const p = document.createElement('p');
       p.textContent = blk;
       frag.appendChild(p);
     }
     editor.appendChild(frag);
     ```
2. **Prevent Unwanted Google Workspace (Gmail / Drive / Docs) Searches & Enforce 100% Universal Site/Topic Agnosticism**:
   - Never ask Gemini to generate DBpia search queries before DBpia runs; build DBpia queries deterministically in JS (`buildMultiFacetedDbpiaQueries`) from the active report/topic and call `askDbpiaAi(q)` directly.
   - Never include `@Google Workspace`, `@Gmail`, `@Google Drive`, or `Google` keywords inside the prompt (even in negative instructions), as Gemini's router parses `@...` as an extension trigger.
   - Never hardcode site-specific branches (`isByeongsan`, etc.) or canned tables/charts/diagrams; all visual and textual synthesis must work universally for any report or academic paper.

## 3. Structured OOXML Document Generation (`wordcraft-cli`)
- Set Korean theme fonts first: `design.themeFonts={"heading":"Malgun Gothic","body":"Malgun Gothic"}`.
- Convert `wc-chart` JSON blocks (`[[WC_CHART_n]]`) via `select.text={"text":"[[WC_CHART_n]]","story":"body"}` + `insert.chart` (with `spec.title = ""` when body `**<그림 n> ...**` caption is emitted, preventing SVG title clipping/duplication).
- Normalize `wc-diagram` JSON blocks (supporting `items`, `nodes`/`edges`, `steps`, `stages`, and ````코드 스니펫` / ````json` fences) into `items: [{ text, level }]` and compile via `select.text={"text":"[[WC_DIAGRAM_n]]","story":"body"}` + `insert.smartArt`.
- Convert inline Korean footnotes (`1)`, `2)`, `3)`) using `(?<![0-9(])` (never matching parenthesized years like `(2010)` and never matching inside Markdown table rows `| ... |`) via `select.text={"text":"[[WC_FN_n]]","story":"body"}` + `references.footnote={"text":"...","mark":"1)"}`.
- Repair collapsed single-line Markdown tables (`repairCollapsedMarkdownTables`) before compilation.
- Export to PDF (`pdfcraft.exe` / `file.save={"path":"...pdf"}`) strictly on demand when requested by the user.
