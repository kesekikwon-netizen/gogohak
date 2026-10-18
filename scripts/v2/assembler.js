'use strict';

const { markersToFootnotes } = require('./markers');

/**
 * 9절 assembler.js:
 * - 목차(outline)와 절 원고 배열(sections)을 하나로 합친다.
 * - 1차 인계용 원고(rawMarkedMd)에는 근거 표시 `⟦F12,L3⟧`를 그대로 보존한다(8.1 manuscript_v1.md 용).
 * - 최종 출판용 원고(manuscriptMd)에는 F 표시를 제거하고 L 표시를 등장 순 각주 `1)`, `2)`로 치환하며,
 *   본문에 실제로 등장한 문헌만 1:1로 가나다순 정렬하여 `## 참고문헌` 절과 `### 각주` 절을 덧붙인다.
 */
function assemble(outline = {}, sections = [], facts = [], lit = [], options = {}) {
  const title = String(outline?.title || (outline?.docType === 'paper' ? '학술논문' : 'Ⅳ. 고찰')).trim();

  const sectionBodies = (sections || []).map((sec) => {
    if (typeof sec === 'string') return sec.trim();
    return String(sec?.markdown || sec?.text || '').trim();
  }).filter(Boolean);

  const rawMarkedMd = [`# ${title}`, '', ...sectionBodies].join('\n\n').trim();

  const formatCitationFn = options.formatCitationFn || (() => {
    try {
      const server = require('../gemini_mcp_server');
      if (typeof server.formatVerifiedCitationLine === 'function') {
        return (card) => server.formatVerifiedCitationLine({
          authors: Array.isArray(card.authors) ? card.authors.join('·') : card.authors,
          year: card.year,
          title: card.title,
          journal: card.journal,
          volume: card.volume,
          issue: card.issue,
          pages: card.pages,
        });
      }
    } catch (_) {}
    return null;
  })();

  const converted = markersToFootnotes(rawMarkedMd, lit, formatCitationFn);

  // 표·차트 코드블록 내부에 남아 있던 [F1] / ⟦F1⟧ 표시 정리(최종 출판 원고용)
  let cleanBody = converted.text;

  const extraBlocks = [];
  if (Array.isArray(outline?.omitted) && outline.omitted.length > 0 && options.includeOmittedNote !== false) {
    const omittedNote = outline.omitted
      .map((o) => `- **${o.heading}**: ${o.reason}`)
      .join('\n');
    extraBlocks.push(`> [!NOTE] 자료 한계로 생략된 분석 항목\n${omittedNote.split('\n').map((l) => `> ${l}`).join('\n')}`);
  }

  if (converted.footnotes.length > 0) {
    const fnLines = converted.footnotes.map((fn) => `${fn.num}) ${fn.text}`);
    extraBlocks.push(`## 각주\n\n${fnLines.join('\n')}`);
  }

  if (converted.bibliography.length > 0) {
    const bibLines = converted.bibliography.map((b) => `- ${b.text}`);
    extraBlocks.push(`## 참고문헌\n\n${bibLines.join('\n')}`);
  }

  const manuscriptMd = [cleanBody, ...extraBlocks].filter(Boolean).join('\n\n').trim();

  return {
    title,
    rawMarkedMd,
    manuscriptMd,
    footnotes: converted.footnotes,
    bibliography: converted.bibliography,
  };
}

module.exports = {
  assemble,
};
