'use strict';

const fs = require('node:fs');
const path = require('node:path');
const { makeLitId } = require('./ids');
const { validateLiteratureCard } = require('./schemas');

const DEFAULT_CONFIG_PATH = path.join(__dirname, 'config.default.json');
const DEFAULT_CONFIG = JSON.parse(fs.readFileSync(DEFAULT_CONFIG_PATH, 'utf8'));

function cleanAuthorList(rawAuthors) {
  if (Array.isArray(rawAuthors)) {
    return rawAuthors
      .map((a) => String(a || '').trim())
      .filter((a) => a.length > 0 && !/저자\s*미상|미상|unknown/i.test(a));
  }
  if (typeof rawAuthors === 'string') {
    return rawAuthors
      .split(/[,;·/]+/)
      .map((a) => a.trim())
      .filter((a) => a.length > 0 && !/저자\s*미상|미상|unknown/i.test(a));
  }
  return [];
}

function cleanYear(rawYear) {
  const m = String(rawYear || '').match(/\b(19\d{2}|20\d{2})\b/);
  return m ? m[1] : '';
}

function normalizeQuotes(rawQuotes) {
  if (!Array.isArray(rawQuotes)) {
    if (typeof rawQuotes === 'string' && rawQuotes.trim()) {
      return [rawQuotes.trim()];
    }
    return [];
  }
  const seen = new Set();
  const out = [];
  for (const q of rawQuotes) {
    const text = String(q || '').replace(/\s+/g, ' ').trim();
    if (text && !seen.has(text)) {
      seen.add(text);
      out.push(text);
    }
  }
  return out;
}

/**
 * DBpia AI 응답(extractLatestDbpiaAnswer / enrichDbpiaReferencesMetadata 결과)을
 * 6.2 문헌 카드(LiteratureCard) 배열로 변환한다.
 */
function toLiteratureCards(dbpiaResult, query = '', startSeq = 1) {
  if (!dbpiaResult || typeof dbpiaResult !== 'object') return [];
  const rawRefs = Array.isArray(dbpiaResult.references)
    ? dbpiaResult.references
    : Array.isArray(dbpiaResult.refs)
      ? dbpiaResult.refs
      : Array.isArray(dbpiaResult.papers)
        ? dbpiaResult.papers
        : [];
  const answerSummary = String(dbpiaResult.answerText || dbpiaResult.summary || dbpiaResult.answer || '').trim();

  const cards = [];
  let seq = startSeq;
  for (const ref of rawRefs) {
    if (!ref || typeof ref !== 'object') continue;
    const title = String(ref.title || ref.paperTitle || '').trim();
    if (!title) continue;

    const authors = cleanAuthorList(ref.authors || ref.author || '');
    const year = cleanYear(ref.year || ref.pubYear || ref.date || '');
    const nodeId = String(ref.nodeId || ref.node_id || ref.id || '').trim();
    const journal = String(ref.journal || ref.journalTitle || ref.publisher || '').trim();
    const volume = String(ref.volume || ref.vol || '').trim();
    const issue = String(ref.issue || ref.no || '').trim();
    const pages = String(ref.pages || ref.pageRange || ref.page || '').trim();
    const quotes = normalizeQuotes(
      ref.quotes ||
        ref.hoverQuotes ||
        ref.snippets ||
        (ref.quote ? [ref.quote] : ref.abstract ? [ref.abstract] : [])
    );
    const summary = String(ref.summary || ref.abstract || answerSummary || '').slice(0, 600).trim();

    const verified = authors.length > 0 && /^\d{4}$/.test(year) && title.length > 0;
    const card = {
      id: makeLitId(seq++),
      nodeId,
      authors,
      year,
      title,
      journal,
      volume,
      issue,
      pages,
      quotes,
      summary,
      query: String(query || ref.query || '').trim(),
      verified,
      quoteStatus: quotes.length > 0 ? 'has_quotes' : 'metadata_only',
    };
    const v = validateLiteratureCard(card);
    if (!v.ok) {
      card.verified = false;
    }
    cards.push(card);
  }
  return cards;
}

/**
 * 같은 nodeId(또는 nodeId가 없을 때 동일 제목+연도)를 가진 문헌 카드를 병합하고,
 * quotes 배열을 합치며 ID를 L1부터 재부여한다.
 */
function mergeLiterature(cards = []) {
  const byKey = new Map();
  for (const raw of cards || []) {
    if (!raw || typeof raw !== 'object') continue;
    const key = raw.nodeId
      ? `node:${String(raw.nodeId).trim()}`
      : `title:${String(raw.title || '').replace(/\s+/g, '').toLowerCase()}:${String(raw.year || '').trim()}`;
    if (!byKey.has(key)) {
      byKey.set(key, {
        ...raw,
        authors: cleanAuthorList(raw.authors),
        year: cleanYear(raw.year),
        quotes: normalizeQuotes(raw.quotes),
      });
    } else {
      const existing = byKey.get(key);
      if (existing.authors.length === 0) existing.authors = cleanAuthorList(raw.authors);
      if (!existing.year) existing.year = cleanYear(raw.year);
      if (!existing.journal && raw.journal) existing.journal = raw.journal;
      if (!existing.volume && raw.volume) existing.volume = raw.volume;
      if (!existing.issue && raw.issue) existing.issue = raw.issue;
      if (!existing.pages && raw.pages) existing.pages = raw.pages;
      if (!existing.summary && raw.summary) existing.summary = raw.summary;
      existing.quotes = normalizeQuotes([...(existing.quotes || []), ...(raw.quotes || [])]);
      existing.verified = existing.authors.length > 0 && /^\d{4}$/.test(existing.year) && Boolean(existing.title);
      existing.quoteStatus = existing.quotes.length > 0 ? 'has_quotes' : 'metadata_only';
    }
  }

  const merged = Array.from(byKey.values());
  return merged.map((c, idx) => {
    const verified = c.authors.length > 0 && /^\d{4}$/.test(c.year) && Boolean(c.title);
    return {
      ...c,
      id: /^L\d+$/.test(String(c.id || '')) ? c.id : makeLitId(idx + 1),
      verified,
      quoteStatus: Array.isArray(c.quotes) && c.quotes.length > 0 ? 'has_quotes' : 'metadata_only',
    };
  });
}

/**
 * 4.7 게이트 G2 판정:
 * verified 문헌 카드 수 >= G2_minVerifiedLiterature.
 * 논문(paper) 모드는 미달 시 반드시 멈추며(canContinue: false),
 * 고찰(report) 모드는 경고 후 계속 가능(canContinue: true).
 */
function evaluateGateG2(cards = [], docType = 'report', config = DEFAULT_CONFIG) {
  const minVerified = config?.gates?.G2_minVerifiedLiterature ?? 3;
  const total = Array.isArray(cards) ? cards.length : 0;
  const verifiedCount = (cards || []).filter((c) => c && c.verified === true).length;
  const withQuotesCount = (cards || []).filter((c) => c && Array.isArray(c.quotes) && c.quotes.length > 0).length;
  const ok = verifiedCount >= minVerified;
  const isPaper = docType === 'paper';

  return {
    gate: 'G2',
    ok,
    docType,
    total,
    verifiedCount,
    withQuotesCount,
    minRequired: minVerified,
    canContinue: ok ? true : !isPaper,
    reason: ok
      ? `검증된 문헌 카드 ${verifiedCount}건 확보 (최소 ${minVerified}건 충족)`
      : isPaper
        ? `검증된 문헌 카드가 ${verifiedCount}건으로 논문 모드 최소 기준(${minVerified}건) 미달 — 진행 중단`
        : `검증된 문헌 카드가 ${verifiedCount}건으로 기준(${minVerified}건) 미달 — 경고 후 진행 가능`,
  };
}

module.exports = {
  toLiteratureCards,
  mergeLiterature,
  evaluateGateG2,
};
