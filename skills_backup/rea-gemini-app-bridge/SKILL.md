---
name: rea-gemini-app-bridge
description: Universal blueprint and workflow for reverse-engineering any target website or desktop app using REA (morluto/rea) and building a zero-lag Standalone Desktop Application or Native Craft App (WordCraft/PdfCraft) that attaches to Antigravity's embedded Google Gemini ('작업' mode / 4 Argon High) web quota to autonomously design apps, collect & synthesize site data, or generate reports.
---

# 🔬 REA × Gemini '작업' Mode × Craft Native App Integration Blueprint

## 1. Workflow Summary
1. **Inspect Target Website or Native Binary with REA**:
   - For web targets (e.g., Gemini Web, DBpia AI): inspect DOM selectors, event listeners, and network payloads via CDP (`127.0.0.1:9223`).
   - For native/Rust/C++ desktop targets (e.g., `WordCraft`, `PdfCraft`): inspect CLI commands (`--help-json`), UI crate structure (`crates/ui-egui`), and font database (`crates/fonts`) to embed custom studio panels and patch CJK/Korean system fonts (`Malgun Gothic`).
2. **Bridge via Local HTTP/MCP Server (`gemini_mcp_server.js`, Port `18787`)**:
   - Route all LLM generation through Antigravity's embedded Gemini Web (`https://gemini.google.com/app`, '작업' mode) so Antigravity agent quota consumption remains **0%**.
3. **Structured OOXML Document Generation (`wordcraft-cli`)**:
   - Set Korean theme fonts: `design.themeFonts={"heading":"Malgun Gothic","body":"Malgun Gothic"}`
   - Convert Markdown tables (`|...|`) into native OOXML tables (`insert.table` + `table.setCell`)
   - Convert `wc-chart` JSON blocks into native OOXML charts (`insert.chart`)
   - Convert `wc-diagram` JSON blocks into native OOXML SmartArt diagrams (`insert.smartArt`)
   - Convert inline Korean footnotes (`1)`, `2)`, `3)`) into bottom-of-page OOXML footnotes (`select.text` + `references.footnote`)
   - Export to PDF (`pdfcraft.exe` / `file.exportPdf`) only on user request.
