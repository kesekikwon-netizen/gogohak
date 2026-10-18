'use strict';

const { parseMarkers, stripFactMarkers } = require('./markers');

/**
 * 9절 crosswalk.js:
 * 원고 문장 / 판정 / 근거 카드 / 원문 구절 / 쪽수(+ 선택적 최종 DBpia 검수 열)를
 * 구조화된 배열과 마크다운 대조표로 생성한다.
 * - 라이브러리 카드(librarySiteKey가 있는 카드)는 "라이브러리: <유적키>/F12"로 표시한다(11장 9단계 호환).
 */
function buildCrosswalk(md = '', facts = [], lit = [], matrices = [], dbpiaReview = null) {
  const factMap = new Map((facts || []).map((f) => [f.id, f]));
  const litMap = new Map((lit || []).map((l) => [l.id, l]));

  const reviewByClaim = new Map();
  if (dbpiaReview && Array.isArray(dbpiaReview.items)) {
    for (const item of dbpiaReview.items) {
      if (item && item.claimId) {
        reviewByClaim.set(item.claimId, item);
      }
    }
  }

  const rows = [];
  for (const matrix of matrices || []) {
    const secId = matrix?.sectionId || 'S1';
    for (const r of matrix?.rows || []) {
      const parsed = parseMarkers(r.sentence || '');
      const cardDetails = [];

      for (const fId of parsed.factIds) {
        const fc = factMap.get(fId);
        if (fc) {
          const displayId = fc.librarySiteKey ? `라이브러리: ${fc.librarySiteKey}/${fc.id}` : fc.id;
          const pageLabel =
            fc.source?.page != null && fc.source?.page !== ''
              ? `p.${fc.source.page}`
              : '쪽수 미상';
          cardDetails.push({
            id: fc.id,
            displayId,
            kind: 'fact',
            quote: fc.quote || '',
            page: pageLabel,
            file: fc.source?.file || '',
          });
        } else {
          cardDetails.push({
            id: fId,
            displayId: fId,
            kind: 'fact',
            quote: '(존재하지 않는 사실 카드)',
            page: '쪽수 미상',
            file: '',
          });
        }
      }

      for (const lId of parsed.litIds) {
        const lc = litMap.get(lId);
        if (lc) {
          const displayId = lc.librarySiteKey ? `라이브러리: ${lc.librarySiteKey}/${lc.id}` : lc.id;
          const authors = Array.isArray(lc.authors) ? lc.authors.join('·') : '';
          const quoteOrBib =
            Array.isArray(lc.quotes) && lc.quotes.length > 0
              ? lc.quotes[0]
              : `${authors}(${lc.year}), 「${lc.title}」`;
          cardDetails.push({
            id: lc.id,
            displayId,
            kind: 'literature',
            quote: quoteOrBib,
            page: lc.pages ? `pp.${lc.pages}` : '쪽수 미상',
            file: lc.journal || '',
          });
        } else {
          cardDetails.push({
            id: lId,
            displayId: lId,
            kind: 'literature',
            quote: '(존재하지 않는 문헌 카드)',
            page: '쪽수 미상',
            file: '',
          });
        }
      }

      const x4 = reviewByClaim.get(r.claimId) || null;
      rows.push({
        sectionId: secId,
        claimId: r.claimId,
        sentence: stripFactMarkers(r.sentence || ''),
        rawSentence: r.sentence || '',
        final: r.final || 'pass',
        warnTag: Boolean(r.warnTag),
        reason: r.reason || '',
        cards: cardDetails,
        dbpiaVerdict: x4 ? x4.verdict : '',
      });
    }
  }

  const mdLines = [
    '# 근거 대조표 (Evidence Crosswalk)',
    '',
    '| 주장 ID | 원고 문장 | 검증 판정 | 근거 카드 | 원문 구절 / 인용 근거 | 출처 쪽수 | 최종 문헌 검수(X4) |',
    '| :--- | :--- | :---: | :---: | :--- | :---: | :---: |',
  ];

  for (const row of rows) {
    const cleanSent = row.sentence.replace(/\|/g, '/').replace(/\r?\n/g, ' ').slice(0, 120);
    const verdictBadge =
      row.final === 'pass'
        ? row.warnTag
          ? '통과(경고)'
          : '통과'
        : row.final === 'rework'
          ? '재작성'
          : '확인 필요';
    const cardLabels = row.cards.map((c) => c.displayId).join(', ') || '-';
    const quoteLabels =
      row.cards
        .map((c) => c.quote.replace(/\|/g, '/').replace(/\r?\n/g, ' ').slice(0, 90))
        .join(' / ') || '-';
    const pageLabels = row.cards.map((c) => c.page).join(', ') || '쪽수 미상';
    const x4Label = row.dbpiaVerdict || '-';

    mdLines.push(
      `| ${row.claimId} | ${cleanSent} | ${verdictBadge} | ${cardLabels} | ${quoteLabels} | ${pageLabels} | ${x4Label} |`
    );
  }

  const needsHumanRows = rows.filter((r) => r.final === 'needs-human');
  const needsHumanLines = [
    '# 확인 필요 문장 목록 (Needs Human Review)',
    '',
    needsHumanRows.length === 0
      ? '- 확인 필요 문장이 없습니다.'
      : needsHumanRows
          .map((r) => {
            const pages = r.cards.map((c) => `${c.displayId}(${c.page})`).join(', ') || '쪽수 미상';
            return `- **[${r.claimId}]** ${r.sentence}\n  - 사유: ${r.reason || '근거 대조 필요'}\n  - 관련 카드 및 쪽수: ${pages}`;
          })
          .join('\n'),
  ];

  return {
    rows,
    markdown: mdLines.join('\n'),
    needsHumanMarkdown: needsHumanLines.join('\n'),
  };
}

module.exports = {
  buildCrosswalk,
};
