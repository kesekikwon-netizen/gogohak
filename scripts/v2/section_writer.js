'use strict';

const fs = require('node:fs');
const path = require('node:path');

const REPORT_PROMPT = fs.readFileSync(path.join(__dirname, 'prompts', 'section_report.md'), 'utf8');
const PAPER_PROMPT = fs.readFileSync(path.join(__dirname, 'prompts', 'section_paper.md'), 'utf8');
const DEFAULT_CONFIG = JSON.parse(fs.readFileSync(path.join(__dirname, 'config.default.json'), 'utf8'));

/**
 * 5.2 & 9절: 절 집필 프롬프트 조립
 * - 이전 회차 재작성(fixes)인 경우, 앞 원고와 지적 사항만 넣는다(5.4 대화 분리 원칙).
 * - 프롬프트 길이 한도(maxPromptChars) 준수: 우선순위가 낮은 문헌 카드부터 요약화하되 사실 카드의 quote는 보존한다.
 */
function buildSectionPrompt(
  section,
  cards = { facts: [], literature: [] },
  docType = 'report',
  prevSummary = '',
  fixes = null,
  stepId = 'P4.S1.r0',
  maxPromptChars = DEFAULT_CONFIG?.gemini?.maxPromptChars || 12000
) {
  const logs = [];
  const template = docType === 'paper' ? PAPER_PROMPT : REPORT_PROMPT;
  const secId = section?.id || 'S1';
  const heading = section?.heading || '1. 고찰';
  const targetChars = section?.targetChars || 1500;

  const neededIds = new Set();
  for (const c of section?.claims || []) {
    for (const id of c.evidence || []) neededIds.add(id);
  }
  for (const v of section?.visuals || []) {
    for (const id of v.evidence || []) neededIds.add(id);
  }

  const allFacts = Array.isArray(cards.facts) ? cards.facts : [];
  const allLit = Array.isArray(cards.literature) ? cards.literature : [];

  // 절에 배정된 카드를 최우선 배치하고 나머지 verified 카드를 보조로 배치
  const sortedFacts = [...allFacts].sort((a, b) => {
    const aNeed = neededIds.has(a.id) ? 1 : 0;
    const bNeed = neededIds.has(b.id) ? 1 : 0;
    return bNeed - aNeed;
  });

  const sortedLit = allLit
    .filter((l) => l && l.verified !== false)
    .sort((a, b) => {
      const aNeed = neededIds.has(a.id) ? 2 : 0;
      const bNeed = neededIds.has(b.id) ? 2 : 0;
      return bNeed - aNeed;
    });

  const claimsText =
    (section?.claims || [])
      .map((c) => `- (${c.id}) ${c.text} [근거 카드: ${(c.evidence || []).join(', ')}]`)
      .join('\n') || '(주장 목록 없음)';

  const visualsText =
    (section?.visuals || [])
      .map((v) => `- [${v.type}] ${v.purpose} [근거 카드: ${(v.evidence || []).join(', ')}]`)
      .join('\n') || '(시각 자료 없음)';

  const renderFacts = (factList) =>
    factList
      .map(
        (f) =>
          `- [${f.id}] (${f.kind}, ${f.confidence || 'verified'}) ${f.subject} / ${f.attribute}: ${f.value}${f.unit || ''} | 원문(p.${f.source?.page ?? '미상'}): "${f.quote}"`
      )
      .join('\n') || '(사실 카드 없음)';

  const renderLit = (litList, compact = false) =>
    litList
      .map((l) => {
        const authors = Array.isArray(l.authors) ? l.authors.join('·') : '';
        if (compact && !neededIds.has(l.id)) {
          return `- [${l.id}] ${authors}(${l.year}), 「${l.title}」 | 요점: ${(l.summary || '').slice(0, 80)}`;
        }
        const quotes = Array.isArray(l.quotes) && l.quotes.length > 0 ? ` | 인용 구절: "${l.quotes.join(' / ').slice(0, 220)}"` : '';
        return `- [${l.id}] ${authors}(${l.year}), 「${l.title}」, 『${l.journal || ''}』 | 요점: ${(l.summary || '').slice(0, 140)}${quotes}`;
      })
      .join('\n') || '(문헌 카드 없음)';

  let reworkBlock = '';
  if (fixes && (fixes.previousDraft || (Array.isArray(fixes.issues) && fixes.issues.length > 0))) {
    const issueLines = (fixes.issues || []).map((iss, idx) => `${idx + 1}. ${iss}`).join('\n');
    reworkBlock = [
      '## 재작성 지시 (이전 초안의 검증 지적 사항 수정)',
      '아래 이전 초안에서 지적된 사항을 모두 바로잡아 절 전체를 다시 작성하십시오.',
      '### 지적 사항',
      issueLines || '- 근거 표시 및 수치·인용 일치성 보완',
      '### 이전 초안',
      String(fixes.previousDraft || '').slice(0, 3500),
    ].join('\n');
  }

  const assemblePrompt = (factSubset, litSubset, compactLit) =>
    template
      .replace(/\{\{SECTION_ID\}\}/g, secId)
      .replace(/\{\{SECTION_HEADING\}\}/g, heading)
      .replace(/\{\{TARGET_CHARS\}\}/g, String(targetChars))
      .replace(/\{\{PREV_SUMMARY\}\}/g, String(prevSummary || '첫 절이므로 앞 절 요약 없음'))
      .replace('{{SECTION_CLAIMS}}', claimsText)
      .replace('{{SECTION_VISUALS}}', visualsText)
      .replace('{{FACT_CARDS_TEXT}}', renderFacts(factSubset))
      .replace('{{LIT_CARDS_TEXT}}', renderLit(litSubset, compactLit))
      .replace('{{REWORK_INSTRUCTIONS}}', reworkBlock)
      .replace(/\{\{STEP_ID\}\}/g, stepId);

  let prompt = assemblePrompt(sortedFacts, sortedLit, false);
  if (prompt.length > maxPromptChars) {
    logs.push(`절 프롬프트(${prompt.length}자) 한도 초과로 비필수 문헌 카드 축약 적용`);
    prompt = assemblePrompt(sortedFacts, sortedLit, true);
  }
  if (prompt.length > maxPromptChars) {
    const essentialFacts = sortedFacts.filter((f) => neededIds.has(f.id));
    const essentialLit = sortedLit.filter((l) => neededIds.has(l.id));
    logs.push(`절 프롬프트(${prompt.length}자) 한도 초과로 절 배정 필수 카드만 유지`);
    prompt = assemblePrompt(
      essentialFacts.length > 0 ? essentialFacts : sortedFacts.slice(0, 15),
      essentialLit.length > 0 ? essentialLit : sortedLit.slice(0, 6),
      true
    );
  }

  return { prompt, logs };
}

/**
 * Gemini 절 집필 응답에서 끝 표식을 제거하고 절 제목이 포함되도록 정리한다.
 */
function parseSectionReply(text, section = null) {
  let clean = String(text || '')
    .replace(/<<END:[^\n>]+>>/g, '')
    .trim();

  // 코드블록으로 감싸서 응답한 경우 벗겨냄(단, 내부 wc-chart/wc-diagram 코드블록은 유지)
  if (/^```(?:markdown|md)?\s*\n[\s\S]*\n```$/.test(clean) && !/^```wc-/.test(clean)) {
    clean = clean.replace(/^```(?:markdown|md)?\s*\n/, '').replace(/\n```$/, '').trim();
  }

  let explicitSummary = '';
  const summaryMatch = clean.match(/\n*\s*\[절\s*요약\]\s*\n([\s\S]*)$/i);
  if (summaryMatch) {
    explicitSummary = summaryMatch[1].replace(/\s+/g, ' ').trim();
    clean = clean.slice(0, summaryMatch.index).trim();
  }

  if (section?.heading && !/^#{1,4}\s+/m.test(clean)) {
    clean = `## ${section.heading}\n\n${clean}`;
  }

  const plainSummary =
    explicitSummary ||
    clean
      .replace(/^#{1,4}\s+.*$/gm, '')
      .replace(/```[\s\S]*?```/g, '')
      .replace(/\s+/g, ' ')
      .trim()
      .slice(0, 250);

  return {
    markdown: clean,
    summary: plainSummary,
  };
}

module.exports = {
  buildSectionPrompt,
  parseSectionReply,
};
