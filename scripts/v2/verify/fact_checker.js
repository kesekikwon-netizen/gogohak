'use strict';

const fs = require('node:fs');
const path = require('node:path');
const { parseMarkers } = require('../markers');

const PROMPT_TEMPLATE = fs.readFileSync(path.join(__dirname, '..', 'prompts', 'check_fact.md'), 'utf8');
const DEFAULT_CONFIG = JSON.parse(fs.readFileSync(path.join(__dirname, '..', 'config.default.json'), 'utf8'));

function normalizeWs(str) {
  return String(str || '').replace(/\s+/g, ' ').trim();
}

/**
 * 원문 쪽(pages)에서 quote 주변 앞뒤 600자 발췌문을 만든다.
 */
function buildExcerptAroundQuote(card, pages = []) {
  const quote = String(card?.quote || '').trim();
  const pageNum = card?.source?.page;
  const pageObj =
    pageNum != null
      ? (pages || []).find((p) => Number(p.page) === Number(pageNum))
      : (pages || [])[0];
  const fullText = String(pageObj?.text || '');
  if (!fullText) return quote;

  const idx = fullText.indexOf(quote);
  if (idx === -1) {
    return fullText.slice(0, 1200);
  }
  const start = Math.max(0, idx - 600);
  const end = Math.min(fullText.length, idx + quote.length + 600);
  return fullText.slice(start, end);
}

/**
 * X1 프롬프트 생성 (reverseExcerpts=true 이면 이중 검증용으로 원문 발췌 순서를 역순 배치)
 */
function buildFactCheckPrompt(claims, facts = [], pages = [], stepId = 'P5.X1', reverseExcerpts = false) {
  const factMap = new Map((facts || []).map((f) => [f.id, f]));
  const blocks = [];

  for (const claim of claims || []) {
    const fIds = parseMarkers(claim.sentence).factIds;
    const refCards = fIds.map((id) => factMap.get(id)).filter(Boolean);
    const orderedCards = reverseExcerpts ? [...refCards].reverse() : refCards;

    const excerptLines = orderedCards.map((fc) => {
      const excerpt = buildExcerptAroundQuote(fc, pages);
      return `  - [${fc.id}] (쪽: ${fc.source?.page ?? '미상'}, 속성: ${fc.subject} ${fc.attribute}=${fc.value}${fc.unit || ''})\n    원문 발췌: "${excerpt}"`;
    });

    blocks.push(
      [
        `### 주장 ID: ${claim.id}`,
        `- 문장: "${claim.sentence}"`,
        `- 표시된 F 카드: ${fIds.join(', ') || '(없음)'}`,
        ...(excerptLines.length > 0 ? excerptLines : ['  - (참조된 원문 발췌 없음)']),
      ].join('\n')
    );
  }

  return PROMPT_TEMPLATE
    .replace('{{CLAIMS_WITH_EXCERPTS}}', blocks.join('\n\n') || '(검증 대상 없음)')
    .replace(/\{\{STEP_ID\}\}/g, stepId);
}

/**
 * 7.3 결정적 재확인:
 * X1이 '일치'라고 판정한 경우 evidenceQuote가 실제 원문(pages 또는 참조 F 카드 quote)의 부분 문자열인지 코드로 확인한다.
 * 아니면 그 판정은 무효('검증불가')가 된다.
 */
function recheckFactVerdictItem(item, claim, factMap, pages = []) {
  const rawVerdict = String(item?.verdict || '').trim();
  const evidenceQuote = String(item?.evidenceQuote || '').trim();
  const normEvidence = normalizeWs(evidenceQuote);

  if (!['일치', '불일치', '근거없음'].includes(rawVerdict)) {
    return {
      claimId: item?.claimId || claim?.id || '',
      atom: String(item?.atom || claim?.sentence || ''),
      verdict: '검증불가',
      rawVerdict,
      evidenceQuote,
      page: item?.page ?? null,
      rechecked: false,
      reason: `알 수 없는 X1 판정값: ${rawVerdict}`,
    };
  }

  if (rawVerdict === '일치') {
    if (!normEvidence) {
      return {
        claimId: item?.claimId || claim?.id || '',
        atom: String(item?.atom || claim?.sentence || ''),
        verdict: '검증불가',
        rawVerdict,
        evidenceQuote: '',
        page: item?.page ?? null,
        rechecked: false,
        reason: '일치 판정이나 evidenceQuote가 비어 있음',
      };
    }

    const fIds = claim ? parseMarkers(claim.sentence).factIds : [];
    const refCards = fIds.map((id) => factMap.get(id)).filter(Boolean);

    const inCardQuotes = refCards.some((fc) => normalizeWs(fc.quote).includes(normEvidence));
    const inPages = (pages || []).some((p) => normalizeWs(p.text).includes(normEvidence));

    if (!inCardQuotes && !inPages) {
      return {
        claimId: item?.claimId || claim?.id || '',
        atom: String(item?.atom || claim?.sentence || ''),
        verdict: '검증불가',
        rawVerdict,
        evidenceQuote,
        page: item?.page ?? null,
        rechecked: false,
        reason: '7.3 재확인 실패: evidenceQuote가 원문에 존재하지 않음',
      };
    }
  }

  return {
    claimId: item?.claimId || claim?.id || '',
    atom: String(item?.atom || claim?.sentence || ''),
    verdict: rawVerdict,
    rawVerdict,
    evidenceQuote,
    page: item?.page ?? null,
    rechecked: true,
    reason: '',
  };
}

/**
 * 7.2 X1 사실 검증 + 7.3 결정적 재확인 + 7.4 이중 검증(연대·편년) 실행
 */
async function checkFacts({
  claims = [],
  facts = [],
  pages = [],
  geminiDriver,
  sectionId = 'S1',
  round = 0,
  config = DEFAULT_CONFIG,
} = {}) {
  const factMap = new Map((facts || []).map((f) => [f.id, f]));
  const targetClaims = (claims || []).filter((c) => {
    const fIds = parseMarkers(c.sentence).factIds;
    return c.kind === 'fact' || fIds.length > 0;
  });

  if (targetClaims.length === 0) {
    return { byClaimId: {}, rawItems: [], doubleChecks: {} };
  }

  const stepId = `P5.${sectionId}.r${round}.X1`;
  const prompt = buildFactCheckPrompt(targetClaims, facts, pages, stepId, false);
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
    const checked = recheckFactVerdictItem(item, claim, factMap, pages);
    byClaimId[cid].push(checked);
  }

  // 응답에 누락된 claimId는 검증불가 처리
  for (const tc of targetClaims) {
    if (!byClaimId[tc.id] || byClaimId[tc.id].length === 0) {
      byClaimId[tc.id] = [
        {
          claimId: tc.id,
          atom: tc.sentence,
          verdict: '검증불가',
          rawVerdict: '',
          evidenceQuote: '',
          page: null,
          rechecked: false,
          reason: 'X1 응답에서 해당 claimId 누락',
        },
      ];
    }
  }

  // 7.4 이중 검증: doubleCheckKinds(기본 dating, chronology)에 해당하는 주장은 원문 발췌 순서를 바꿔 2차 실행
  const doubleCheckKinds = new Set(config?.verify?.doubleCheckKinds || ['dating', 'chronology']);
  const doubleTargets = targetClaims.filter((c) => {
    const fIds = parseMarkers(c.sentence).factIds;
    return fIds.some((id) => {
      const fc = factMap.get(id);
      return fc && doubleCheckKinds.has(fc.kind);
    });
  });

  const doubleChecks = {};
  if (doubleTargets.length > 0) {
    const stepId2 = `P5.${sectionId}.r${round}.X1.double`;
    const prompt2 = buildFactCheckPrompt(doubleTargets, facts, pages, stepId2, true);
    const res2 = await geminiDriver.ask({
      jobStep: stepId2,
      prompt: prompt2,
      expectJson: true,
      newChat: true,
    });
    const rawList2 = Array.isArray(res2.json) ? res2.json : Array.isArray(res2.json?.items) ? res2.json.items : [];
    const secondByClaim = {};
    for (const dt of doubleTargets) secondByClaim[dt.id] = [];
    for (const item of rawList2) {
      const cid = String(item?.claimId || '');
      const claim = claimMap.get(cid);
      if (!claim || !secondByClaim[cid]) continue;
      secondByClaim[cid].push(recheckFactVerdictItem(item, claim, factMap, pages));
    }

    for (const dt of doubleTargets) {
      const v1List = byClaimId[dt.id] || [];
      const v2List = secondByClaim[dt.id] || [];
      const summarize = (arr) => {
        if (arr.length === 0) return '검증불가';
        if (arr.some((x) => x.verdict === '검증불가')) return '검증불가';
        if (arr.some((x) => x.verdict === '불일치')) return '불일치';
        if (arr.some((x) => x.verdict === '근거없음')) return '근거없음';
        return '일치';
      };
      const sum1 = summarize(v1List);
      const sum2 = summarize(v2List);
      const matched = sum1 === sum2;
      doubleChecks[dt.id] = {
        verdict1: sum1,
        verdict2: sum2,
        matched,
      };
      if (!matched) {
        byClaimId[dt.id] = v1List.map((entry) => ({
          ...entry,
          verdict: '검증불가',
          rechecked: false,
          reason: `7.4 이중 검증 불일치(1차: ${sum1}, 2차: ${sum2})`,
        }));
      }
    }
  }

  return {
    byClaimId,
    rawItems: rawList,
    doubleChecks,
  };
}

module.exports = {
  checkFacts,
  buildFactCheckPrompt,
  recheckFactVerdictItem,
};
