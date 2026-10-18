'use strict';

const { makeClaimId } = require('../ids');
const { parseMarkers } = require('../markers');

const MEASUREMENT_NUMBER_RE =
  /\d+(?:\.\d+)?\s*(?:m|cm|mm|km|㎡|m2|m²|점|기|동|호|편|개|구|세기|년|BP|cal\s*BP|BC|AD|%)/i;

const AUTHOR_YEAR_CITATION_RE = /[가-힣A-Za-z·,\s]{2,15}\(\s*(?:19|20)\d{2}[a-z]?\s*\)/;

/**
 * 마크다운 본문에서 제목(#), 표(|...|), 코드블록(```...```)을 제외한 산문 문장을 분리하여
 * 6.5 주장 단위(Claim) 배열로 반환한다.
 */
function splitClaims(markdown, sectionId = 'S1', round = 0) {
  const raw = String(markdown || '');
  // 코드블록 제거
  const withoutCodeBlocks = raw.replace(/```[\s\S]*?```/g, '\n');
  const lines = withoutCodeBlocks.split(/\r?\n/);

  const proseLines = [];
  for (const line of lines) {
    const trimmed = line.trim();
    if (!trimmed) continue;
    if (/^#{1,6}\s+/.test(trimmed)) continue;
    if (/^\|.*\|$/.test(trimmed)) continue;
    if (/^[-*+]\s*$/.test(trimmed)) continue;
    if (/^>\s*\[!/.test(trimmed)) continue;
    if (/^\*{1,2}<\s*(?:표|그림|모식도|차트)[^>]*>/.test(trimmed)) continue;
    if (/^>\s*근거\s*카드\s*:/.test(trimmed)) continue;
    proseLines.push(trimmed);
  }

  const sentences = [];
  for (const paragraph of proseLines) {
    // 마침표(.), 물음표(?), 느낌표(!) 뒤 공백 또는 줄 끝 기준으로 문장 분리 (단, 소수점 6.2m 나 약어 p. 57 제외)
    const parts = paragraph
      .split(/(?<=[가-힣\])⟧»"']\s*[.?!])\s+|(?<=(?:다|요|음|됨|함)\.)\s+/g)
      .map((s) => s.trim())
      .filter(Boolean);
    for (const p of parts) {
      sentences.push(p);
    }
  }

  const claims = [];
  let seq = 1;
  for (const sentence of sentences) {
    const parsed = parseMarkers(sentence);
    const markers = parsed.allIds;
    const hasF = parsed.factIds.length > 0;
    const hasL = parsed.litIds.length > 0;

    // 괄호 연도 인용(예: (2019))을 제외한 측정·수량·유구번호·연대 숫자가 있는지 확인
    const strippedCitationYears = sentence.replace(/\(\s*(?:[가-힣A-Za-z·\s]+,?\s*)?(?:19|20)\d{2}[a-z]?\s*\)/g, '');
    const hasMeasNum = MEASUREMENT_NUMBER_RE.test(strippedCitationYears);
    const hasAuthorYear = AUTHOR_YEAR_CITATION_RE.test(sentence);

    let kind = 'interpretation';
    if (hasL || hasAuthorYear) {
      kind = 'citation';
    } else if (hasF || hasMeasNum) {
      kind = 'fact';
    }

    claims.push({
      id: makeClaimId(sectionId, round, seq++),
      sentence,
      atoms: [],
      markers,
      kind,
    });
  }

  return claims;
}

module.exports = {
  splitClaims,
  MEASUREMENT_NUMBER_RE,
  AUTHOR_YEAR_CITATION_RE,
};
