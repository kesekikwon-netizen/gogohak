"use strict";

const fs = require("node:fs");
const path = require("node:path");
const defaultConfig = require("./config.default.json");
const { makeFactId } = require("./ids.js");
const { validateFactCard, FACT_KINDS } = require("./schemas.js");

const PROMPT_PATH = path.join(__dirname, "prompts", "fact_extract.md");

const CHAPTER_PATTERNS = [
  {
    chapter: "Ⅰ",
    title: "Ⅰ. 조사개요",
    rx: /(?:^|\n)\s*(?:Ⅰ|I)\.\s*(?:조사\s*개요|머리말)/,
  },
  {
    chapter: "Ⅱ",
    title: "Ⅱ. 주변 환경",
    rx: /(?:^|\n)\s*(?:Ⅱ|II)\.\s*(?:주변\s*환경|자연|지리|고고학적\s*환경)/,
  },
  {
    chapter: "Ⅲ",
    title: "Ⅲ. 조사내용",
    rx: /(?:^|\n)\s*(?:Ⅲ|III)\.\s*(?:조사\s*내용|유구\s*및\s*유물)/,
  },
  {
    chapter: "Ⅳ",
    title: "Ⅳ. 조사결과",
    rx: /(?:^|\n)\s*(?:Ⅳ|IV)\.\s*(?:조사\s*결과|고\s*찰|조사단\s*의견|맺음말)/,
  },
  {
    chapter: "목록",
    title: "출토유물 목록",
    rx: /(?:^|\n)\s*(?:출토유물\s*목록|유물\s*관찰표)/,
  },
];

function detectChapterInText(text = "") {
  const lines = String(text || "").split(/\r?\n/);
  for (const line of lines) {
    if (/[·…]{3,}|\.{4,}/.test(line)) continue;
    for (const def of CHAPTER_PATTERNS) {
      if (def.rx.test(line)) {
        return def.chapter;
      }
    }
  }
  return null;
}

function splitChapters(pages = []) {
  const chaptersMap = new Map();
  let currentChapter = "Ⅰ";

  for (const p of pages) {
    const pageText = String(p.text || "").trim();
    if (!pageText) continue;

    const detected = detectChapterInText(pageText);
    if (detected) {
      currentChapter = detected;
    }

    if (!chaptersMap.has(currentChapter)) {
      chaptersMap.set(currentChapter, {
        chapter: currentChapter,
        pages: [],
      });
    }
    chaptersMap.get(currentChapter).pages.push({
      page: p.page ?? null,
      text: pageText,
    });
  }

  return Array.from(chaptersMap.values()).map((ch) => ({
    chapter: ch.chapter,
    pages: ch.pages,
    combinedText: ch.pages
      .map((p) => (p.page ? `[[PAGE ${p.page}]]\n${p.text}` : p.text))
      .join("\n\n"),
  }));
}

function buildFactPrompt(chapterObj, fileName = "보고서.pdf") {
  const template = fs.readFileSync(PROMPT_PATH, "utf8");
  return template
    .replace(/\{\{FILE_NAME\}\}/g, String(fileName || "보고서.pdf"))
    .replace(/\{\{CHAPTER_NAME\}\}/g, String(chapterObj.chapter || "Ⅲ"))
    .replace(/\{\{CHAPTER_TEXT\}\}/g, String(chapterObj.combinedText || ""));
}

function normalizeWs(str) {
  return String(str || "")
    .replace(/\s+/g, " ")
    .trim();
}

function valueMatchesInQuote(valueStr, quoteStr) {
  const val = String(valueStr || "").trim();
  const quote = String(quoteStr || "");
  if (!val || !quote) return false;
  if (quote.includes(val)) return true;

  const numVal = Number(val.replace(/,/g, ""));
  if (!Number.isNaN(numVal) && /^[-+]?\d[\d,]*(?:\.\d+)?$/.test(val)) {
    const numsInQuote = quote.match(/\d[\d,]*(?:\.\d+)?/g) || [];
    for (const token of numsInQuote) {
      const parsed = Number(token.replace(/,/g, ""));
      if (!Number.isNaN(parsed) && Math.abs(parsed - numVal) < 1e-9) {
        return true;
      }
    }
  }
  return false;
}

/**
 * Deterministic verification of FactCard against source pages (spec 6.1):
 * 1) After whitespace normalization, card.quote must be a substring of the page text.
 * 2) Numeric/string card.value must appear inside card.quote (allowing 6.2 == 6.20).
 */
function verifyFactAgainstPages(card, pages = []) {
  const normQuote = normalizeWs(card?.quote || "");
  if (!normQuote) {
    return {
      ...card,
      confidence: "unverified",
      note: "인용 구절(quote)이 비어 있음",
    };
  }

  const targetPageNum =
    card?.source && Number.isInteger(card.source.page) ? card.source.page : null;
  let candidatePages = pages;
  if (targetPageNum !== null) {
    const exact = pages.filter((p) => p.page === targetPageNum);
    if (exact.length > 0) {
      candidatePages = exact;
    }
  }

  const quoteFoundInPage = candidatePages.some((p) =>
    normalizeWs(p.text || "").includes(normQuote)
  );
  const valueInQuote = valueMatchesInQuote(card?.value, card?.quote);

  const verified = Boolean(quoteFoundInPage && valueInQuote);
  const notes = [];
  if (!quoteFoundInPage) notes.push("원문 해당 쪽에서 quote 일치 구절 미발견");
  if (!valueInQuote) notes.push("quote 내 value 불일치");

  return {
    ...card,
    confidence: verified ? "verified" : "unverified",
    note: verified ? card.note || "" : notes.join("; "),
  };
}

function parseFactReply(replyOrJson, defaultFile = "보고서.pdf", defaultChapter = "Ⅲ", startIndex = 1) {
  let obj = replyOrJson;
  if (typeof replyOrJson === "string") {
    try {
      obj = JSON.parse(replyOrJson);
    } catch {
      obj = null;
    }
  }
  const rawList = Array.isArray(obj)
    ? obj
    : Array.isArray(obj?.cards)
      ? obj.cards
      : Array.isArray(obj?.facts)
        ? obj.facts
        : [];

  const cards = [];
  let seq = startIndex;

  for (const raw of rawList) {
    if (!raw || typeof raw !== "object") continue;
    const kind = FACT_KINDS.has(raw.kind) ? raw.kind : "other";
    const pageVal =
      raw.page !== undefined && raw.page !== null && Number.isInteger(Number(raw.page)) && Number(raw.page) > 0
        ? Number(raw.page)
        : raw.source && Number.isInteger(Number(raw.source.page)) && Number(raw.source.page) > 0
          ? Number(raw.source.page)
          : null;

    const card = {
      id: raw.id && /^F\d+$/.test(raw.id) ? raw.id : makeFactId(seq++),
      kind,
      subject: String(raw.subject || "").trim(),
      attribute: String(raw.attribute || "").trim(),
      value: String(raw.value ?? "").trim(),
      unit: String(raw.unit ?? "").trim(),
      quote: String(raw.quote || "").trim(),
      source: {
        file: String(raw.source?.file || defaultFile),
        page: pageVal,
        chapter: String(raw.source?.chapter || raw.chapter || defaultChapter),
      },
      confidence: raw.confidence === "verified" ? "verified" : "unverified",
      note: String(raw.note || ""),
    };

    const valid = validateFactCard(card);
    if (valid.ok) {
      cards.push(card);
    }
  }

  return cards;
}

function evaluateGateG1(facts = [], failedChapters = [], config = defaultConfig) {
  const minRatio = Number(config?.gates?.G1_minVerifiedRatio ?? 0.6);
  const total = facts.length;
  const verifiedCount = facts.filter((f) => f.confidence === "verified").length;
  const verifiedRatio = total > 0 ? verifiedCount / total : 0;
  const ok = total >= 1 && verifiedRatio >= minRatio;

  return {
    gate: "G1",
    ok,
    total,
    verifiedCount,
    verifiedRatio: Number(verifiedRatio.toFixed(4)),
    minRequiredRatio: minRatio,
    failedChapters,
    detail: ok
      ? `사실 카드 ${total}개 중 ${verifiedCount}개 원문 검증 통과 (${Math.round(verifiedRatio * 100)}%)`
      : `사실 카드 검증 기준 미달: 총 ${total}개, 검증 비율 ${Math.round(verifiedRatio * 100)}% (최소 ${Math.round(minRatio * 100)}% 필요)`,
  };
}

module.exports = {
  splitChapters,
  buildFactPrompt,
  parseFactReply,
  verifyFactAgainstPages,
  evaluateGateG1,
  normalizeWs,
  valueMatchesInQuote,
};
