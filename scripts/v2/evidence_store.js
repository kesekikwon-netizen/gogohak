"use strict";

const fs = require("node:fs");
const path = require("node:path");

function writeJsonAtomic(filePath, data) {
  fs.mkdirSync(path.dirname(filePath), { recursive: true });
  const tmpPath = `${filePath}.${process.pid}.${Date.now()}.tmp`;
  fs.writeFileSync(tmpPath, JSON.stringify(data, null, 2), "utf8");
  fs.renameSync(tmpPath, filePath);
  return filePath;
}

function readJsonIfExists(filePath, fallback = null) {
  if (!fs.existsSync(filePath)) return fallback;
  return JSON.parse(fs.readFileSync(filePath, "utf8"));
}

function savePages(jobDir, pages = []) {
  const pagesDir = path.join(jobDir, "pages");
  fs.mkdirSync(pagesDir, { recursive: true });
  for (let i = 0; i < pages.length; i++) {
    const p = pages[i];
    const num = p.page ? Number(p.page) : i + 1;
    const fileName = `p${String(num).padStart(4, "0")}.txt`;
    fs.writeFileSync(path.join(pagesDir, fileName), String(p.text || ""), "utf8");
  }
  writeJsonAtomic(path.join(pagesDir, "index.json"), pages);
  return pagesDir;
}

function loadPages(jobDir) {
  const indexPath = path.join(jobDir, "pages", "index.json");
  if (fs.existsSync(indexPath)) {
    return readJsonIfExists(indexPath, []);
  }
  const pagesDir = path.join(jobDir, "pages");
  if (!fs.existsSync(pagesDir)) return [];
  const files = fs
    .readdirSync(pagesDir)
    .filter((f) => /^p\d{4}\.txt$/.test(f))
    .sort();
  return files.map((f) => {
    const m = /^p(\d{4})\.txt$/.exec(f);
    return {
      page: m ? Number(m[1]) : null,
      text: fs.readFileSync(path.join(pagesDir, f), "utf8"),
    };
  });
}

function saveFacts(jobDir, facts = []) {
  return writeJsonAtomic(path.join(jobDir, "facts.json"), facts);
}

function loadFacts(jobDir) {
  return readJsonIfExists(path.join(jobDir, "facts.json"), []);
}

function saveLiterature(jobDir, cards = []) {
  return writeJsonAtomic(path.join(jobDir, "literature.json"), cards);
}

function loadLiterature(jobDir) {
  return readJsonIfExists(path.join(jobDir, "literature.json"), []);
}

function saveOutline(jobDir, outline) {
  return writeJsonAtomic(path.join(jobDir, "outline.json"), outline);
}

function loadOutline(jobDir) {
  return readJsonIfExists(path.join(jobDir, "outline.json"), null);
}

function saveSectionDraft(jobDir, sectionId, round, markdown) {
  const secPath = path.join(jobDir, "sections", `${sectionId}.r${round}.md`);
  fs.mkdirSync(path.dirname(secPath), { recursive: true });
  fs.writeFileSync(secPath, String(markdown || ""), "utf8");
  return secPath;
}

function loadSectionDraft(jobDir, sectionId, round) {
  const secPath = path.join(jobDir, "sections", `${sectionId}.r${round}.md`);
  if (!fs.existsSync(secPath)) return null;
  return fs.readFileSync(secPath, "utf8");
}

function saveVerifyMatrix(jobDir, sectionId, round, matrix) {
  const vPath = path.join(jobDir, "verify", `${sectionId}.r${round}.json`);
  return writeJsonAtomic(vPath, matrix);
}

function loadVerifyMatrix(jobDir, sectionId, round) {
  const vPath = path.join(jobDir, "verify", `${sectionId}.r${round}.json`);
  return readJsonIfExists(vPath, null);
}

function loadLatestVerifyMatrix(jobDir, sectionId) {
  const verifyDir = path.join(jobDir, "verify");
  if (!fs.existsSync(verifyDir)) return null;
  const prefix = `${sectionId}.r`;
  const files = fs
    .readdirSync(verifyDir)
    .filter((f) => f.startsWith(prefix) && f.endsWith(".json"));
  if (files.length === 0) return null;
  let maxRound = -1;
  let targetFile = files[0];
  for (const f of files) {
    const m = /\.r(\d+)\.json$/.exec(f);
    const r = m ? Number(m[1]) : 0;
    if (r >= maxRound) {
      maxRound = r;
      targetFile = f;
    }
  }
  return readJsonIfExists(path.join(verifyDir, targetFile), null);
}

function buildCardLookup(facts = [], literature = []) {
  const map = new Map();
  for (const f of facts) {
    if (f && f.id) map.set(f.id, f);
  }
  for (const l of literature) {
    if (l && l.id) map.set(l.id, l);
  }
  return map;
}

function createEvidenceStore(jobDir) {
  return {
    jobDir,
    savePages: (pages) => savePages(jobDir, pages),
    loadPages: () => loadPages(jobDir),
    saveFacts: (facts) => saveFacts(jobDir, facts),
    loadFacts: () => loadFacts(jobDir),
    saveLiterature: (cards) => saveLiterature(jobDir, cards),
    loadLiterature: () => loadLiterature(jobDir),
    saveOutline: (outline) => saveOutline(jobDir, outline),
    loadOutline: () => loadOutline(jobDir),
    saveSectionDraft: (sectionId, round, md) => saveSectionDraft(jobDir, sectionId, round, md),
    loadSectionDraft: (sectionId, round) => loadSectionDraft(jobDir, sectionId, round),
    saveVerifyMatrix: (sectionId, round, matrix) => saveVerifyMatrix(jobDir, sectionId, round, matrix),
    loadVerifyMatrix: (sectionId, round) => loadVerifyMatrix(jobDir, sectionId, round),
    loadLatestVerifyMatrix: (sectionId) => loadLatestVerifyMatrix(jobDir, sectionId),
    buildCardLookup: (facts, literature) => buildCardLookup(facts, literature),
  };
}

module.exports = {
  writeJsonAtomic,
  readJsonIfExists,
  savePages,
  loadPages,
  saveFacts,
  loadFacts,
  saveLiterature,
  loadLiterature,
  saveOutline,
  loadOutline,
  saveSectionDraft,
  loadSectionDraft,
  saveVerifyMatrix,
  loadVerifyMatrix,
  loadLatestVerifyMatrix,
  buildCardLookup,
  createEvidenceStore,
};

