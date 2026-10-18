'use strict';

const fs = require('node:fs');
const path = require('node:path');
const os = require('node:os');

function getDefaultLibraryDir() {
  return path.join(os.homedir(), 'archaeology_reports', 'library');
}

/**
 * 11장 9단계: 유적 키 정규화 (파일시스템 안전 식별자)
 */
function normalizeSiteKey(rawKey = '') {
  const cleaned = String(rawKey || '')
    .trim()
    .replace(/[\\/:*?"<>|]/g, '_')
    .replace(/\s+/g, '_');
  return cleaned || 'unknown_site';
}

function makeFactDedupeKey(fact) {
  return [
    String(fact?.category || '').trim(),
    String(fact?.subject || '').trim(),
    String(fact?.attribute || '').trim(),
    String(fact?.value || '').trim(),
    String(fact?.quote || '').replace(/\s+/g, ' ').trim(),
  ].join('||');
}

function makeLitDedupeKey(lit) {
  if (lit?.nodeId) {
    return `NODE:${String(lit.nodeId).trim()}`;
  }
  const normTitle = String(lit?.title || '')
    .replace(/\s+/g, '')
    .toLowerCase();
  const normYear = String(lit?.year || '').trim();
  return `TY:${normTitle}:${normYear}`;
}

/**
 * 11장 9단계 (P9):
 * 작업이 끝난 뒤 `verified` 사실 카드와 `verified` 문헌 카드를
 * `<출력폴더>/library/<정규화된_유적키>/` 아래에 중복 없이 적재한다.
 */
function saveJobToSiteLibrary({
  siteKey,
  facts = [],
  literature = [],
  baseDir = getDefaultLibraryDir(),
} = {}) {
  const normKey = normalizeSiteKey(siteKey);
  const siteDir = path.join(baseDir, normKey);
  fs.mkdirSync(siteDir, { recursive: true });

  const factsPath = path.join(siteDir, 'facts.json');
  const litPath = path.join(siteDir, 'literature.json');

  const existingFacts = fs.existsSync(factsPath)
    ? JSON.parse(fs.readFileSync(factsPath, 'utf8'))
    : [];
  const existingLit = fs.existsSync(litPath)
    ? JSON.parse(fs.readFileSync(litPath, 'utf8'))
    : [];

  const factSeen = new Set(existingFacts.map(makeFactDedupeKey));
  const mergedFacts = [...existingFacts];
  for (const f of facts || []) {
    if (!f || f.confidence !== 'verified') continue;
    const key = makeFactDedupeKey(f);
    if (!factSeen.has(key)) {
      factSeen.add(key);
      mergedFacts.push({
        ...f,
        id: `F${mergedFacts.length + 1}`,
        librarySiteKey: normKey,
      });
    }
  }

  const litSeen = new Set(existingLit.map(makeLitDedupeKey));
  const mergedLit = [...existingLit];
  for (const l of literature || []) {
    if (!l || !l.verified) continue;
    const key = makeLitDedupeKey(l);
    if (!litSeen.has(key)) {
      litSeen.add(key);
      mergedLit.push({
        ...l,
        id: `L${mergedLit.length + 1}`,
        librarySiteKey: normKey,
      });
    }
  }

  fs.writeFileSync(factsPath, JSON.stringify(mergedFacts, null, 2), 'utf8');
  fs.writeFileSync(litPath, JSON.stringify(mergedLit, null, 2), 'utf8');

  return {
    siteKey: normKey,
    siteDir,
    factsCount: mergedFacts.length,
    literatureCount: mergedLit.length,
  };
}

function loadSiteLibrary(siteKey, baseDir = getDefaultLibraryDir()) {
  const normKey = normalizeSiteKey(siteKey);
  const siteDir = path.join(baseDir, normKey);
  const factsPath = path.join(siteDir, 'facts.json');
  const litPath = path.join(siteDir, 'literature.json');

  const rawFacts = fs.existsSync(factsPath)
    ? JSON.parse(fs.readFileSync(factsPath, 'utf8'))
    : [];
  const rawLit = fs.existsSync(litPath)
    ? JSON.parse(fs.readFileSync(litPath, 'utf8'))
    : [];

  const facts = rawFacts.map((f) => ({
    ...f,
    librarySiteKey: normKey,
    sourceTag: `라이브러리: ${normKey}/${f.id}`,
  }));

  const literature = rawLit.map((l) => ({
    ...l,
    librarySiteKey: normKey,
    sourceTag: `라이브러리: ${normKey}/${l.id}`,
  }));

  return {
    siteKey: normKey,
    facts,
    literature,
  };
}

/**
 * 다른 작업에서 같은 시대·유구 유형 카드를 불러올 때 사용한다.
 * 불러온 카드에는 `sourceTag: "라이브러리: <유적키>/<카드ID>"`를 부여한다.
 */
function querySiteLibrary({
  baseDir = getDefaultLibraryDir(),
  excludeSiteKey = '',
  category = '',
  keyword = '',
} = {}) {
  if (!fs.existsSync(baseDir)) {
    return { facts: [], literature: [] };
  }
  const exKey = excludeSiteKey ? normalizeSiteKey(excludeSiteKey) : '';
  const entries = fs.readdirSync(baseDir, { withFileTypes: true });
  const matchedFacts = [];
  const matchedLit = [];

  for (const ent of entries) {
    if (!ent.isDirectory()) continue;
    const siteKey = ent.name;
    if (exKey && siteKey === exKey) continue;
    const loaded = loadSiteLibrary(siteKey, baseDir);

    for (const f of loaded.facts) {
      if (category && f.category !== category) continue;
      if (keyword) {
        const hay = `${f.subject} ${f.attribute} ${f.value} ${f.quote}`.toLowerCase();
        if (!hay.includes(String(keyword).toLowerCase())) continue;
      }
      matchedFacts.push(f);
    }

    for (const l of loaded.literature) {
      if (keyword) {
        const hay = `${l.title} ${(l.quotes || []).join(' ')}`.toLowerCase();
        if (!hay.includes(String(keyword).toLowerCase())) continue;
      }
      matchedLit.push(l);
    }
  }

  return {
    facts: matchedFacts,
    literature: matchedLit,
  };
}

module.exports = {
  normalizeSiteKey,
  saveJobToSiteLibrary,
  loadSiteLibrary,
  querySiteLibrary,
};
