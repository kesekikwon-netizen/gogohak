'use strict';

const { parseMarkers, stripFactMarkers } = require('../markers');
const { splitClaims } = require('./claim_splitter');

const FORBIDDEN_V7_RE =
  /저자\s*미상|연도\s*미상|@Google\s*Workspace|@Gmail|@Google\s*Drive|Google\s*Workspace/i;

const UNIT_NUM_GLOBAL_RE =
  /(\d+(?:\.\d+)?)\s*(m|cm|mm|km|㎡|m2|m²|점|기|동|편|개|구|세기|년|BP|cal\s*BP|%)/gi;

function toMetersIfLength(num, unit) {
  const u = String(unit || '').toLowerCase();
  if (u === 'm') return num;
  if (u === 'cm') return num / 100;
  if (u === 'mm') return num / 1000;
  return null;
}

function numbersMatchWithUnitConversion(numA, unitA, numB, unitB) {
  if (!Number.isFinite(numA) || !Number.isFinite(numB)) return false;
  if (Math.abs(numA - numB) < 1e-6) return true;
  const mA = toMetersIfLength(numA, unitA);
  const mB = toMetersIfLength(numB, unitB);
  if (mA !== null && mB !== null && Math.abs(mA - mB) < 1e-4) {
    return true;
  }
  return false;
}

function cardMatchesNumberAndUnit(card, numVal, unitStr) {
  if (!card) return false;
  const cardNum = Number(String(card.value || '').replace(/,/g, '').match(/-?\d+(?:\.\d+)?/)?.[0]);
  const cardUnit = String(card.unit || '').trim();
  if (Number.isFinite(cardNum) && numbersMatchWithUnitConversion(numVal, unitStr, cardNum, cardUnit)) {
    return true;
  }
  // quote 안에 동일 수치(또는 m<->cm 환산 수치)가 있는지 확인
  const quote = String(card.quote || '');
  for (const m of quote.matchAll(/(\d+(?:\.\d+)?)\s*(m|cm|mm|km|㎡|m2|m²|점|기|동|편|개|구|세기|년|BP|cal\s*BP|%)?/gi)) {
    const qNum = Number(m[1]);
    const qUnit = m[2] || cardUnit;
    if (numbersMatchWithUnitConversion(numVal, unitStr, qNum, qUnit)) {
      return true;
    }
  }
  return false;
}

/**
 * 마크다운 내 표(Table)와 차트/모식도 코드블록(wc-chart, wc-diagram)의 값 칸에
 * 근거 카드(⟦F...⟧ / ⟦L...⟧ 또는 JSON evidence 배열)가 연결되어 있는지 검사(V8).
 */
function checkTablesAndChartsEvidence(markdown, cardMap) {
  const issues = [];
  const raw = String(markdown || '');

  // 1. wc-chart / wc-diagram 코드블록 검사
  const blockMatches = [...raw.matchAll(/```(wc-chart|wc-diagram)\s*([\s\S]*?)```/gi)];
  for (const bm of blockMatches) {
    const kind = bm[1];
    const body = bm[2].trim();
    const markersInBody = parseMarkers(body).allIds;
    let jsonEvidence = [];
    try {
      const parsed = JSON.parse(body);
      if (Array.isArray(parsed.evidence)) jsonEvidence = parsed.evidence.map(String);
      else if (Array.isArray(parsed.sourceCards)) jsonEvidence = parsed.sourceCards.map(String);
    } catch (_) {}
    const allEv = [...new Set([...markersInBody, ...jsonEvidence])];
    if (allEv.length === 0) {
      issues.push(`${kind} 시각자료 블록에 근거 카드(evidence 또는 ⟦F...⟧)가 연결되지 않음`);
    } else if (cardMap) {
      const missing = allEv.filter((id) => !cardMap.has(id));
      if (missing.length > 0) {
        issues.push(`${kind} 시각자료 블록에 존재하지 않는 카드 ID 포함: ${missing.join(', ')}`);
      }
    }
  }

  // 2. 마크다운 표 데이터 행 검사
  const withoutCode = raw.replace(/```[\s\S]*?```/g, '\n');
  const lines = withoutCode.split(/\r?\n/);
  let tableLines = [];
  const flushTable = () => {
    if (tableLines.length >= 3) {
      const dataRows = tableLines.slice(2);
      for (const row of dataRows) {
        if (/^\|\s*[-:|\s]+\|\s*$/.test(row)) continue;
        // 숫자가 들어간 데이터 행인데 근거 카드 표시가 하나도 없으면 V8 실패
        const hasValueNumber = /\d+(?:\.\d+)?/.test(row.replace(/^\|\s*\d+\s*\|/, '|'));
        const rowMarkers = parseMarkers(row).allIds;
        if (hasValueNumber && rowMarkers.length === 0) {
          issues.push(`표 데이터 행에 근거 카드 표시(⟦F...⟧/⟦L...⟧) 누락: "${row.slice(0, 50)}"`);
        }
      }
    }
    tableLines = [];
  };

  for (const line of lines) {
    const trimmed = line.trim();
    if (/^\|.*\|$/.test(trimmed)) {
      tableLines.push(trimmed);
    } else {
      flushTable();
    }
  }
  flushTable();

  return {
    ok: issues.length === 0,
    issues,
  };
}

/**
 * 7.1 결정적 검사 V1~V8 실행
 * @param {object} params
 * @param {string} params.markdown 절 또는 전체 원고 마크다운
 * @param {Array} params.claims splitClaims 결과(없으면 내부에서 생성)
 * @param {Array} params.facts FactCard 배열
 * @param {Array} params.literature LiteratureCard 배열
 * @param {object} params.section Outline의 해당 section 객체(V6 목차 이행 검사용)
 */
function runDeterministic({
  markdown = '',
  claims = null,
  facts = [],
  literature = [],
  section = null,
  sectionId = 'S1',
  round = 0,
} = {}) {
  const factMap = new Map((facts || []).map((f) => [f.id, f]));
  const litMap = new Map((literature || []).map((l) => [l.id, l]));
  const allCardMap = new Map([...factMap.entries(), ...litMap.entries()]);

  const claimList = Array.isArray(claims) ? claims : splitClaims(markdown, sectionId, round);

  // V6(절 단위): 절 주장의 근거 카드 중 절반 이상이 원고에 없는지 확인
  const outlineEvidenceIds = new Set();
  for (const c of section?.claims || []) {
    for (const id of c.evidence || []) outlineEvidenceIds.add(id);
  }
  const usedInManuscriptIds = new Set(parseMarkers(markdown).allIds);
  let v6SectionStatus = 'pass';
  let v6Reason = '';
  if (outlineEvidenceIds.size > 0) {
    let missingCount = 0;
    for (const id of outlineEvidenceIds) {
      if (!usedInManuscriptIds.has(id)) missingCount += 1;
    }
    if (missingCount >= Math.ceil(outlineEvidenceIds.size / 2)) {
      v6SectionStatus = 'warn';
      v6Reason = `목차 근거 카드 ${outlineEvidenceIds.size}개 중 ${missingCount}개 미사용`;
    }
  }

  // V8(절 단위 표·차트 칸 근거)
  const v8Check = checkTablesAndChartsEvidence(markdown, allCardMap);
  const v8SectionStatus = v8Check.ok ? 'pass' : 'block';

  const claimResults = [];
  let hasBlock = !v8Check.ok;
  let hasWarn = v6SectionStatus === 'warn';

  for (const claim of claimList) {
    const sentence = String(claim.sentence || '');
    const parsed = parseMarkers(sentence);
    const fIds = parsed.factIds;
    const lIds = parsed.litIds;
    const allIds = parsed.allIds;

    const checks = {
      V1: 'pass',
      V2: 'pass',
      V3: 'pass',
      V4: 'pass',
      V5: 'pass',
      V6: v6SectionStatus,
      V7: 'pass',
      V8: v8SectionStatus,
    };
    const reasons = [];
    if (v6SectionStatus === 'warn') reasons.push(`V6(warn): ${v6Reason}`);
    if (!v8Check.ok) reasons.push(`V8(block): ${v8Check.issues[0]}`);

    // V3: 카드 실재 검사
    const missingCards = allIds.filter((id) => !allCardMap.has(id));
    if (missingCards.length > 0) {
      checks.V3 = 'block';
      reasons.push(`V3(block): 존재하지 않는 카드 번호(${missingCards.join(', ')})`);
    }

    // 문장 내 측정·수량 수치 추출 (괄호 안 인용 연도 제외)
    const strippedCitation = stripFactMarkers(sentence).replace(
      /\(\s*(?:[가-힣A-Za-z·\s]+,?\s*)?(?:19|20)\d{2}[a-z]?\s*\)/g,
      ''
    );
    const unitMatches = [...strippedCitation.matchAll(UNIT_NUM_GLOBAL_RE)];

    // V1: 측정·수량·연대 숫자가 든 문장에 F 표시 없음
    if (unitMatches.length > 0 && fIds.length === 0) {
      checks.V1 = 'block';
      reasons.push(`V1(block): 수치(${unitMatches.map((m) => m[0]).join(', ')})가 포함된 문장에 F 근거 표시 없음`);
    }

    // V2: 문장 숫자가 가리키는 F 카드 value와 일치 여부 (m<->cm 환산 허용)
    if (unitMatches.length > 0 && fIds.length > 0) {
      const refFacts = fIds.map((id) => factMap.get(id)).filter(Boolean);
      if (refFacts.length > 0) {
        for (const m of unitMatches) {
          const numVal = Number(m[1]);
          const unitStr = m[2] || '';
          const matched = refFacts.some((fc) => cardMatchesNumberAndUnit(fc, numVal, unitStr));
          if (!matched) {
            checks.V2 = 'block';
            reasons.push(`V2(block): 문장 수치 "${m[0]}"가 참조된 F 카드(${fIds.join(',')})의 값과 불일치`);
          }
        }
      }
    }

    // V4: L 표시 문장의 저자명이 카드 저자와 다름 / verified:false 카드 사용
    if (lIds.length > 0) {
      const refLits = lIds.map((id) => litMap.get(id)).filter(Boolean);
      for (const lc of refLits) {
        if (lc.verified !== true) {
          checks.V4 = 'block';
          reasons.push(`V4(block): 미검증(verified:false) 문헌 카드(${lc.id}) 인용`);
        }
      }

      const validAuthors = new Set();
      for (const lc of refLits) {
        for (const a of lc.authors || []) {
          validAuthors.add(String(a).trim());
        }
      }

      // 문장 안에 "홍길동(2019)" 또는 "홍길동·이유물(2021)" 형태의 저자명이 적혀 있을 때 대조
      const authorMentions = [
        ...sentence.matchAll(/([가-힣]{2,6})(?:·[가-힣]{2,6})*\s*(?:외\s*)?\(\s*(?:19|20)\d{2}[a-z]?\s*\)/g),
      ];
      for (const am of authorMentions) {
        const mentionedLead = am[1].trim();
        if (validAuthors.size > 0 && !validAuthors.has(mentionedLead)) {
          checks.V4 = 'block';
          reasons.push(
            `V4(block): 본문 인용 저자("${mentionedLead}")가 문헌 카드(${lIds.join(',')}) 저자(${Array.from(validAuthors).join('·')})와 불일치`
          );
        }
      }
    }

    // V5: unverified F 카드 사용 (warn)
    if (fIds.length > 0) {
      const unverifiedF = fIds.filter((id) => {
        const fc = factMap.get(id);
        return fc && fc.confidence !== 'verified';
      });
      if (unverifiedF.length > 0) {
        checks.V5 = 'warn';
        reasons.push(`V5(warn): 미확인(unverified) 사실 카드(${unverifiedF.join(', ')}) 사용`);
      }
    }

    // V7: 금지 표현 검사 (block)
    if (FORBIDDEN_V7_RE.test(sentence)) {
      checks.V7 = 'block';
      reasons.push('V7(block): 금지 표현(저자미상/연도미상/Google Workspace 키워드) 포함');
    }

    const rowHasBlock = Object.values(checks).includes('block');
    const rowHasWarn = Object.values(checks).includes('warn');
    if (rowHasBlock) hasBlock = true;
    if (rowHasWarn) hasWarn = true;

    claimResults.push({
      claimId: claim.id,
      sentence,
      kind: claim.kind,
      markers: allIds,
      deterministic: checks,
      status: rowHasBlock ? 'block' : rowHasWarn ? 'warn' : 'pass',
      reasons,
    });
  }

  // 문장이 하나도 없거나 표/코드블록에만 V7 위반이 있는 경우 추가 확인
  if (FORBIDDEN_V7_RE.test(markdown)) {
    hasBlock = true;
  }

  return {
    ok: !hasBlock,
    hasBlock,
    hasWarn,
    v6SectionStatus,
    v6Reason,
    v8SectionStatus,
    v8Issues: v8Check.issues,
    claimResults,
  };
}

module.exports = {
  runDeterministic,
  checkTablesAndChartsEvidence,
  numbersMatchWithUnitConversion,
};
