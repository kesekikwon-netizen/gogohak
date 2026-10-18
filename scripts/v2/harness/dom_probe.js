"use strict";

const fs = require("node:fs");
const path = require("node:path");
const selectors = require("./selectors.json");
const { HarnessError } = require("./errors.js");

function resolveSelectorList(name) {
  const parts = String(name || "").split(".");
  let cur = selectors;
  for (const p of parts) {
    if (!cur || typeof cur !== "object") return [];
    cur = cur[p];
  }
  if (Array.isArray(cur)) return cur;
  if (typeof cur === "string") return [cur];
  return [];
}

function saveDomSnapshot(jobDir, htmlSnippet, label = "drift") {
  if (!jobDir || !htmlSnippet) return null;
  const rawDir = path.join(jobDir, "raw");
  fs.mkdirSync(rawDir, { recursive: true });
  const ts = new Date().toISOString().replace(/[:.]/g, "-");
  const snapPath = path.join(rawDir, `dom_${label}_${ts}.html`);
  fs.writeFileSync(snapPath, String(htmlSnippet).slice(0, 4000), "utf8");
  return snapPath;
}

/**
 * Probes the target DOM for each named selector entry in selectors.json.
 * @param {Function} sendOrEval - Either a CDP `send(method, params)` function or a custom async `(namesMap) => probeResult` evaluator (for unit tests).
 * @param {string[]} names - Array of selector names, e.g. ["gemini.editor", "gemini.sendButton"]
 * @param {object} options - { jobDir, throwOnMissing }
 */
async function probe(sendOrEval, names = [], options = {}) {
  const selectorMap = {};
  for (const name of names) {
    selectorMap[name] = resolveSelectorList(name);
  }

  let evalOutput = null;
  if (typeof sendOrEval === "function" && sendOrEval.isMockEvaluator) {
    evalOutput = await sendOrEval(selectorMap);
  } else if (typeof sendOrEval === "function") {
    const res = await sendOrEval("Runtime.evaluate", {
      expression: `(() => {
        const map = ${JSON.stringify(selectorMap)};
        const details = {};
        const missing = [];
        for (const [name, candidates] of Object.entries(map)) {
          let matchedSelector = null;
          let count = 0;
          let visible = false;
          for (const sel of (candidates || [])) {
            try {
              const nodes = Array.from(document.querySelectorAll(sel));
              if (nodes.length > 0) {
                matchedSelector = sel;
                count = nodes.length;
                visible = nodes.some(el => el.offsetParent !== null || el.getClientRects().length > 0);
                break;
              }
            } catch (e) {}
          }
          details[name] = { exists: count > 0, visible, count, matchedSelector };
          if (count === 0) missing.push(name);
        }
        const domSnippet = missing.length > 0 && document.body
          ? String(document.body.outerHTML || '').slice(0, 4000)
          : '';
        return { ok: missing.length === 0, missing, details, domSnippet };
      })()`,
      returnByValue: true,
    });
    evalOutput = res?.result?.value || {
      ok: false,
      missing: names,
      details: {},
      domSnippet: "",
    };
  } else {
    throw new Error("probe requires a valid CDP send function or evaluator");
  }

  let snapshotPath = null;
  if (!evalOutput.ok && evalOutput.domSnippet && options.jobDir) {
    snapshotPath = saveDomSnapshot(options.jobDir, evalOutput.domSnippet);
  }

  const result = {
    ok: Boolean(evalOutput.ok),
    missing: Array.isArray(evalOutput.missing) ? evalOutput.missing : [],
    details: evalOutput.details || {},
    snapshotPath,
  };

  if (!result.ok && options.throwOnMissing) {
    throw new HarnessError(
      "E_DOM_DRIFT",
      `화면 구조가 바뀌었습니다: ${result.missing.join(", ")}을(를) 찾지 못함`,
      { missing: result.missing, snapshotPath }
    );
  }

  return result;
}

module.exports = {
  selectors,
  resolveSelectorList,
  saveDomSnapshot,
  probe,
};
