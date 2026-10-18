"use strict";

const fs = require("node:fs");
const path = require("node:path");
const { execFileSync } = require("node:child_process");
const defaultConfig = require("../config.default.json");
const { HarnessError } = require("./errors.js");

const PLUGIN_ROOT = path.resolve(__dirname, "..", "..", "..");
const DEFAULT_PDFCRAFT_CLI = path.join(
  PLUGIN_ROOT,
  "bin",
  "pdfcraft",
  "pdfcraft-0.8.0-windows-x64-portable",
  "pdfcraft-cli.exe"
);
const DEFAULT_WORDCRAFT_CLI = path.join(
  PLUGIN_ROOT,
  "bin",
  "wordcraft",
  "wordcraft-0.7.0-windows-x64-portable",
  "wordcraft-cli.exe"
);

function createCraftDriver(deps = {}) {
  const cfg = deps.config || defaultConfig;
  const pdfCliPath = deps.pdfCliPath || DEFAULT_PDFCRAFT_CLI;
  const wordCliPath = deps.wordCliPath || DEFAULT_WORDCRAFT_CLI;
  const runExecFile =
    deps.execFileSync ||
    ((exe, args, opts) => execFileSync(exe, args, opts));

  function checkExecutables() {
    const pdfExists = fs.existsSync(pdfCliPath);
    const wordExists = fs.existsSync(wordCliPath);
    return {
      pdfExists,
      wordExists,
      pdfCliPath,
      wordCliPath,
    };
  }

  function pdfInfo(filePath) {
    if (!fs.existsSync(pdfCliPath)) {
      throw new HarnessError("E_CRAFT_EXE", `pdfcraft-cli 없음: ${pdfCliPath}`);
    }
    try {
      const stdout = runExecFile(pdfCliPath, ["info", filePath], {
        encoding: "utf8",
        timeout: cfg.timeouts.craftExtract,
        windowsHide: true,
      });
      const text = String(stdout || "");
      const m = /(?:Pages|페이지|쪽수)\s*[:=]\s*(\d+)/i.exec(text) || /\b(\d+)\s*pages?\b/i.exec(text);
      return {
        raw: text.trim(),
        pageCount: m ? Number(m[1]) : null,
      };
    } catch (err) {
      throw new HarnessError("E_CRAFT_FAIL", `pdfcraft-cli info 실패: ${err.message}`);
    }
  }

  function pdfPageText(filePath, pageNumber = null) {
    if (!fs.existsSync(pdfCliPath)) {
      throw new HarnessError("E_CRAFT_EXE", `pdfcraft-cli 없음: ${pdfCliPath}`);
    }
    const args =
      typeof pageNumber === "number" && pageNumber > 0
        ? ["text", filePath, "--page", String(pageNumber)]
        : ["text", filePath];
    try {
      const stdout = runExecFile(pdfCliPath, args, {
        encoding: "utf8",
        timeout: cfg.timeouts.craftExtract,
        maxBuffer: 32 * 1024 * 1024,
        windowsHide: true,
      });
      return String(stdout || "");
    } catch (err) {
      throw new HarnessError(
        "E_CRAFT_FAIL",
        `pdfcraft-cli text 실패${pageNumber ? ` (page ${pageNumber})` : ""}: ${err.message}`
      );
    }
  }

  async function exportDocx(options = {}) {
    if (typeof deps.exportManuscriptToWordCraftAndPdfCraft === "function") {
      try {
        const res = await deps.exportManuscriptToWordCraftAndPdfCraft(options);
        if (!res || res.ok === false) {
          throw new Error(res?.message || "WordCraft/PdfCraft 내보내기 실패");
        }
        return res;
      } catch (err) {
        if (err instanceof HarnessError) throw err;
        throw new HarnessError("E_CRAFT_FAIL", err.message);
      }
    }

    if (!fs.existsSync(wordCliPath)) {
      throw new HarnessError("E_CRAFT_EXE", `wordcraft-cli 없음: ${wordCliPath}`);
    }
    return { ok: true, docxPath: options.docxPath || null };
  }

  return {
    checkExecutables,
    pdfInfo,
    pdfPageText,
    exportDocx,
    pdfCliPath,
    wordCliPath,
  };
}

module.exports = {
  createCraftDriver,
  DEFAULT_PDFCRAFT_CLI,
  DEFAULT_WORDCRAFT_CLI,
};
