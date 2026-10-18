'use strict';

const fs = require('node:fs');
const path = require('node:path');
const { parseMarkers } = require('../markers');

const PROMPT_TEMPLATE = fs.readFileSync(path.join(__dirname, '..', 'prompts', 'check_citation.md'), 'utf8');

function normalizeWs(str) {
  return String(str || '').replace(/\s+/g, ' ').trim();
}

function buildCitationCheckPrompt(claims, literature = [], stepId = 'P5.X2') {
  const litMap = new Map((literature || []).map((l) => [l.id, l]));
  const blocks = [];

  for (const claim of claims || []) {
    const lIds = parseMarkers(claim.sentence).litIds;
    const refCards = lIds.map((id) => litMap.get(id)).filter(Boolean);
    const cardLines = refCards.map((lc) => {
      const authors = Array.isArray(lc.authors) ? lc.authors.join('·') : '';
      const quotesStr =
        Array.isArray(lc.quotes) && lc.quotes.length > 0
          ? lc.quotes.map((q, i) => `    [구절 ${i + 1}] "${q}"`).join('\n')
          : '    (인용 구절 없음 — 서지만 있음)';
      return [
        `  - [${lc.id}] ${authors}(${lc.year}), 「${lc.title}」, 『${lc.journal || ''}』`,
        `    요점(summary): "${lc.summary || ''}"`,
        quotesStr,
      ].join('\n');
    });

    blocks.push(
      [
        `### 주장 ID: ${claim.id}`,
        `- 문장: "${claim.sentence}"`,
        `- 표시된 L 카드: ${lIds.join(', ') || '(없음)'}`,
        ...(cardLines.length > 0 ? cardLines : ['  - (참조된 문헌 카드 없음)']),
      ].join('\n')
    );
  }

  return PROMPT_TEMPLATE
    .replace('{{CITATION_CLAIMS_TEXT}}', blocks.join('\n\n') || '(검증 대상 없음)')
    .replace(/\{\{STEP_ID\}\}/g, stepId);
}

/**
 * 6.2 & 7.3 결정적 재확인:
 * - 참조된 모든 L 카드의 quotes가 비어 있으면('서지만 있음') '구절없음'으로 판정.
 * - X2가 '지지' 또는 '부분지지'라고 한 경우 evidenceQuote가 실제 L 카드의 quotes·summary 안에 있는지 코드로 확인.
 *   없으면 '검증불가'로 격하.
 */
function recheckCitationVerdictItem(item, claim, litMap) {
  const rawVerdict = String(item?.verdict || '').trim();
  const evidenceQuote = String(item?.evidenceQuote || '').trim();
  const normEvidence = normalizeWs(evidenceQuote);

  const lIds = claim ? parseMarkers(claim.sentence).litIds : [];
  const refCards = lIds.map((id) => litMap.get(id)).filter(Boolean);

  // 6.2: quotes가 비면 그 카드는 "서지만 있음"으로 표시하고 X2에서 "구절 없음" 판정을 받는다.
  const anyHasQuotes = refCards.some((lc) => Array.isArray(lc.quotes) && lc.quotes.length > 0);
  if (refCards.length > 0 && !anyHasQuotes) {
    return {
      claimId: item?.claimId || claim?.id || '',
      verdict: '구절없음',
      rawVerdict,
      evidenceQuote: '',
      rechecked: true,
      reason: '6.2 서지만 있음(quotes 빈 배열) → 구절없음 판정',
    };
  }

  if (!['지지', '부분지지', '불지지', '구절없음'].includes(rawVerdict)) {
    return {
      claimId: item?.claimId || claim?.id || '',
      verdict: '검증불가',
      rawVerdict,
      evidenceQuote,
      rechecked: false,
      reason: `알 수 없는 X2 판정값: ${rawVerdict}`,
    };
  }

  if (rawVerdict === '지지' || rawVerdict === '부분지지') {
    if (!normEvidence) {
      return {
        claimId: item?.claimId || claim?.id || '',
        verdict: '검증불가',
        rawVerdict,
        evidenceQuote: '',
        rechecked: false,
        reason: `${rawVerdict} 판정이나 evidenceQuote가 비어 있음`,
      };
    }

    const inQuotesOrSummary = refCards.some((lc) => {
      const inQuotes = (lc.quotes || []).some((q) => normalizeWs(q).includes(normEvidence));
      const inSummary = normalizeWs(lc.summary || '').includes(normEvidence);
      return inQuotes || inSummary;
    });

    if (!inQuotesOrSummary) {
      return {
        claimId: item?.claimId || claim?.id || '',
        verdict: '검증불가',
        rawVerdict,
        evidenceQuote,
        rechecked: false,
        reason: '7.3 재확인 실패: evidenceQuote가 문헌 카드 quotes·summary에 존재하지 않음',
      };
    }
  }

  return {
    claimId: item?.claimId || claim?.id || '',
    verdict: rawVerdict,
    rawVerdict,
    evidenceQuote,
    rechecked: true,
    reason: '',
  };
}

/**
 * 7.2 X2 인용 검증 + 7.3 결정적 재확인 실행
 */
async function checkCitations({
  claims = [],
  literature = [],
  geminiDriver,
  sectionId = 'S1',
  round = 0,
} = {}) {
  const litMap = new Map((literature || []).map((l) => [l.id, l]));
  const targetClaims = (claims || []).filter((c) => {
    const lIds = parseMarkers(c.sentence).litIds;
    return c.kind === 'citation' || lIds.length > 0;
  });

  if (targetClaims.length === 0) {
    return { byClaimId: {}, rawItems: [] };
  }

  const stepId = `P5.${sectionId}.r${round}.X2`;
  const prompt = buildCitationCheckPrompt(targetClaims, literature, stepId);
  const res = await geminiDriver.ask({
    jobStep: stepId,
    prompt,
    expectJson: true,
    newChat: true,
  });

  const rawList = Array.isArray(res.json) ? res.json : Array.isArray(res.json?.items) ? res.json.items : [];
  const claimMap = new Map(targetClaims.map((c) => [c.id, c]));
  const byClaimId = {};
  for (const tc of targetClaims) {
    byClaimId[tc.id] = [];
  }

  for (const item of rawList) {
    const cid = String(item?.claimId || '');
    const claim = claimMap.get(cid);
    if (!claim) continue;
    byClaimId[cid].push(recheckCitationVerdictItem(item, claim, litMap));
  }

  for (const tc of targetClaims) {
    if (!byClaimId[tc.id] || byClaimId[tc.id].length === 0) {
      byClaimId[tc.id] = [recheckCitationVerdictItem({ claimId: tc.id, verdict: '' }, tc, litMap)];
    }
  }

  return {
    byClaimId,
    rawItems: rawList,
  };
}

module.exports = {
  checkCitations,
  buildCitationCheckPrompt,
  recheckCitationVerdictItem,
};
