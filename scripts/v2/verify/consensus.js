'use strict';

const fs = require('node:fs');
const path = require('node:path');
const { validateVerifyMatrix } = require('../schemas');

const DEFAULT_CONFIG = JSON.parse(fs.readFileSync(path.join(__dirname, '..', 'config.default.json'), 'utf8'));

/**
 * X1 배열과 X2 배열의 판정을 하나로 요약한다.
 * 반환값:
 * - '일치_지지' (모두 일치/지지이거나 X1/X2 대상이 아닌 순수 해석 문장)
 * - '불일치_불지지' (하나라도 불일치 또는 불지지)
 * - '근거없음_구절없음_부분지지' (근거없음, 구절없음, 부분지지 중 하나)
 * - '검증불가' (검증불가 또는 이중검증 불일치)
 */
function summarizeModelVerdict(x1List = [], x2List = [], doubleCheck = null) {
  if (doubleCheck && doubleCheck.matched === false) {
    return { category: '검증불가', detail: `이중 검증 불일치(${doubleCheck.verdict1} vs ${doubleCheck.verdict2})` };
  }

  const allVerdicts = [
    ...(x1List || []).map((x) => x.verdict),
    ...(x2List || []).map((x) => x.verdict),
  ];

  if (allVerdicts.length === 0) {
    return { category: '일치_지지', detail: '해석 문장(결정적 검사 통과)' };
  }

  if (allVerdicts.includes('검증불가')) {
    const item = [...(x1List || []), ...(x2List || [])].find((x) => x.verdict === '검증불가');
    return { category: '검증불가', detail: item?.reason || '검증불가(형식 오류 또는 7.3 재확인 실패)' };
  }

  if (allVerdicts.includes('불일치') || allVerdicts.includes('불지지')) {
    const bad = allVerdicts.find((v) => v === '불일치' || v === '불지지');
    return { category: '불일치_불지지', detail: `X1/X2 판정: ${bad}` };
  }

  if (
    allVerdicts.includes('근거없음') ||
    allVerdicts.includes('구절없음') ||
    allVerdicts.includes('부분지지')
  ) {
    const weak = allVerdicts.find((v) => v === '근거없음' || v === '구절없음' || v === '부분지지');
    return { category: '근거없음_구절없음_부분지지', detail: `X1/X2 판정: ${weak}` };
  }

  return { category: '일치_지지', detail: '일치/지지' };
}

/**
 * 7.5 합의 규칙 표의 단일 문장 판정
 *
 * | 결정적 검사(V) | X1/X2 판정(재확인 후) | 최종 |
 * | block 있음 | 무엇이든 | 재작성 (단, round >= maxRework 이면 needs-human) |
 * | 통과 | 일치/지지 | 통과 |
 * | 통과 | 불일치/불지지 | 재작성 (단, round >= maxRework 이면 needs-human) |
 * | 통과 | 근거없음/구절없음/부분지지 | 재작성 1회(round === 0) → 그래도면(round >= 1) 확인 필요 |
 * | 통과 | 검증불가 | 확인 필요 |
 * | warn만 있음 | 일치/지지 | 통과 + 꼬리표(warnTag: true) |
 */
function decideClaimFinal({
  detStatus = 'pass',
  detReasons = [],
  x1List = [],
  x2List = [],
  doubleCheck = null,
  round = 0,
  maxRework = DEFAULT_CONFIG?.verify?.maxRework ?? 2,
} = {}) {
  const modelSummary = summarizeModelVerdict(x1List, x2List, doubleCheck);
  const combinedReasons = [...(detReasons || [])];
  if (modelSummary.detail && modelSummary.category !== '일치_지지') {
    combinedReasons.push(modelSummary.detail);
  }

  // 1. 결정적 검사 block 있음 -> 무엇이든 재작성 (소진 시 확인 필요)
  if (detStatus === 'block') {
    const exhausted = round >= maxRework;
    return {
      final: exhausted ? 'needs-human' : 'rework',
      warnTag: false,
      reason: combinedReasons.join('; ') || '결정적 검사(V) block 위반',
    };
  }

  const isWarnOnly = detStatus === 'warn';

  // 2. 검증불가 -> 즉시 확인 필요
  if (modelSummary.category === '검증불가') {
    return {
      final: 'needs-human',
      warnTag: isWarnOnly,
      reason: combinedReasons.join('; ') || '검증불가',
    };
  }

  // 3. 불일치/불지지 -> 재작성 (maxRework 소진 시 확인 필요)
  if (modelSummary.category === '불일치_불지지') {
    const exhausted = round >= maxRework;
    return {
      final: exhausted ? 'needs-human' : 'rework',
      warnTag: isWarnOnly,
      reason: combinedReasons.join('; ') || 'X1/X2 불일치·불지지',
    };
  }

  // 4. 근거없음/구절없음/부분지지 -> 재작성 1회(round === 0) 후 그래도면(round >= 1) 확인 필요
  if (modelSummary.category === '근거없음_구절없음_부분지지') {
    const toNeedsHuman = round >= 1 || round >= maxRework;
    return {
      final: toNeedsHuman ? 'needs-human' : 'rework',
      warnTag: isWarnOnly,
      reason: combinedReasons.join('; ') || '근거없음/구절없음/부분지지',
    };
  }

  // 5. 통과(또는 warn만 있음) + 일치/지지 -> 통과 (warn만 있으면 통과 + 꼬리표)
  return {
    final: 'pass',
    warnTag: isWarnOnly,
    reason: isWarnOnly ? combinedReasons.join('; ') || '경고 꼬리표' : '',
  };
}

/**
 * 7.6 검증 매트릭스(VerifyMatrix) 생성
 */
function buildVerifyMatrix({
  sectionId = 'S1',
  round = 0,
  detResult,
  x1Result = { byClaimId: {}, doubleChecks: {} },
  x2Result = { byClaimId: {} },
  criticResult = { verdict: 'accept', issues: [] },
  maxRework = DEFAULT_CONFIG?.verify?.maxRework ?? 2,
} = {}) {
  const rows = [];
  let passCount = 0;
  let reworkCount = 0;
  let needsHumanCount = 0;

  for (const cr of detResult?.claimResults || []) {
    const x1List = x1Result?.byClaimId?.[cr.claimId] || [];
    const x2List = x2Result?.byClaimId?.[cr.claimId] || [];
    const dbl = x1Result?.doubleChecks?.[cr.claimId] || null;

    const decision = decideClaimFinal({
      detStatus: cr.status,
      detReasons: cr.reasons,
      x1List,
      x2List,
      doubleCheck: dbl,
      round,
      maxRework,
    });

    if (decision.final === 'pass') passCount += 1;
    else if (decision.final === 'rework') reworkCount += 1;
    else needsHumanCount += 1;

    rows.push({
      claimId: cr.claimId,
      sentence: cr.sentence,
      deterministic: cr.deterministic,
      x1: x1List.map((x) => ({
        atom: x.atom,
        verdict: x.verdict,
        evidenceQuote: x.evidenceQuote || '',
        page: x.page ?? null,
        rechecked: Boolean(x.rechecked),
      })),
      x2: x2List.map((x) => ({
        verdict: x.verdict,
        evidenceQuote: x.evidenceQuote || '',
        rechecked: Boolean(x.rechecked),
      })),
      double: dbl,
      final: decision.final,
      warnTag: Boolean(decision.warnTag),
      reason: decision.reason || '',
    });
  }

  // 표·차트 V8 block 위반이 문장 밖(표/차트 블록)에서 발생한 경우 반영
  const v8Issues = detResult?.v8Issues || [];
  if (v8Issues.length > 0 && rows.length === 0) {
    const exhausted = round >= maxRework;
    const fState = exhausted ? 'needs-human' : 'rework';
    if (fState === 'rework') reworkCount += 1;
    else needsHumanCount += 1;
    rows.push({
      claimId: `${sectionId}.r${round}.V8`,
      sentence: '[시각자료 표·차트 근거 검사]',
      deterministic: { V8: 'block' },
      x1: [],
      x2: [],
      double: null,
      final: fState,
      warnTag: false,
      reason: `V8(block): ${v8Issues.join('; ')}`,
    });
  }

  const matrix = {
    sectionId,
    round,
    rows,
    critic: {
      verdict: criticResult?.verdict === 'revise' ? 'revise' : 'accept',
      issues: Array.isArray(criticResult?.issues) ? criticResult.issues : [],
    },
    summary: {
      pass: passCount,
      rework: reworkCount,
      needsHuman: needsHumanCount,
    },
  };

  validateVerifyMatrix(matrix);
  return matrix;
}

/**
 * 7.5 재작성 소진 후 남은 확인 필요(needs-human) 또는 미해결 rework 문장을 지우지 않고
 * 문장 끝에 `〔확인 필요: 사유〕`를 부착한다.
 */
function attachNeedsHumanTags(markdown, verifyMatrix) {
  let result = String(markdown || '');
  if (!verifyMatrix || !Array.isArray(verifyMatrix.rows)) return result;

  for (const row of verifyMatrix.rows) {
    if (row.final === 'needs-human' || row.final === 'rework') {
      const target = String(row.sentence || '').trim();
      if (!target || target.startsWith('[시각자료')) continue;
      const reasonText = String(row.reason || '근거 대조 필요').replace(/[〔〕\r\n]/g, ' ').trim();
      const tag = `〔확인 필요: ${reasonText}〕`;
      if (!result.includes(tag) && result.includes(target)) {
        result = result.replace(target, `${target} ${tag}`);
      }
    }
  }
  return result;
}

/**
 * 4.7 게이트 G5 판정:
 * - 확인 필요(needs-human) 문장 비율 <= G5_maxNeedsHumanRatio (기본 0.2)
 * - 확인 필요로 넘긴 것을 제외하고 미해결 rework/block 판정 0건
 */
function evaluateGateG5(matrices = [], config = DEFAULT_CONFIG) {
  const maxRatio = config?.gates?.G5_maxNeedsHumanRatio ?? 0.2;
  let totalRows = 0;
  let needsHumanRows = 0;
  let unresolvedReworkRows = 0;

  for (const m of matrices || []) {
    for (const r of m?.rows || []) {
      totalRows += 1;
      if (r.final === 'needs-human') needsHumanRows += 1;
      else if (r.final === 'rework') unresolvedReworkRows += 1;
    }
  }

  const ratio = totalRows > 0 ? Number((needsHumanRows / totalRows).toFixed(4)) : 0;
  const ok = totalRows > 0 && ratio <= maxRatio && unresolvedReworkRows === 0;

  return {
    gate: 'G5',
    ok,
    totalRows,
    needsHumanRows,
    unresolvedReworkRows,
    needsHumanRatio: ratio,
    maxAllowedRatio: maxRatio,
    reason: ok
      ? `교차검증 통과 (전체 ${totalRows}문장 중 확인 필요 ${needsHumanRows}건, 비율 ${(ratio * 100).toFixed(1)}%)`
      : `교차검증 미달 (확인 필요 ${needsHumanRows}/${totalRows}건 [${(ratio * 100).toFixed(1)}%], 미해결 재작성 ${unresolvedReworkRows}건)`,
  };
}

module.exports = {
  summarizeModelVerdict,
  decideClaimFinal,
  buildVerifyMatrix,
  attachNeedsHumanTags,
  evaluateGateG5,
};
