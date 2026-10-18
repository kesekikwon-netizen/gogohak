'use strict';

const fs = require('node:fs');
const path = require('node:path');
const { stripFactMarkers } = require('../markers');

const REPORT_CRITIC_PROMPT = fs.readFileSync(path.join(__dirname, '..', 'prompts', 'critic_report.md'), 'utf8');
const PAPER_CRITIC_PROMPT = fs.readFileSync(path.join(__dirname, '..', 'prompts', 'critic_paper.md'), 'utf8');

function buildCriticPrompt(markdown, section = null, docType = 'report', stepId = 'P5.X3') {
  const template = docType === 'paper' ? PAPER_CRITIC_PROMPT : REPORT_CRITIC_PROMPT;
  const cleanManuscript = stripFactMarkers(String(markdown || '')).replace(/⟦[^⟧]*⟧|\[\[(?:[FL]\d+(?:\s*,\s*[FL]\d+)*)\]\]/g, '');
  const claimsText =
    (section?.claims || [])
      .map((c) => `- (${c.id}) ${c.text}`)
      .join('\n') || '(절 핵심 주장 목록 없음)';

  return template
    .replace('{{SECTION_CLAIMS}}', claimsText)
    .replace('{{SECTION_MANUSCRIPT}}', cleanManuscript)
    .replace(/\{\{STEP_ID\}\}/g, stepId);
}

/**
 * 7.2 X3 심사위원(발굴조사보고서 평가위원 / 한국고고학보 심사위원) 독립 검증 실행
 */
async function critique({
  markdown = '',
  section = null,
  docType = 'report',
  geminiDriver,
  sectionId = 'S1',
  round = 0,
} = {}) {
  const stepId = `P5.${sectionId}.r${round}.X3`;
  const prompt = buildCriticPrompt(markdown, section, docType, stepId);
  const res = await geminiDriver.ask({
    jobStep: stepId,
    prompt,
    expectJson: true,
    newChat: true,
  });

  const obj = res.json && typeof res.json === 'object' ? res.json : {};
  const verdict = obj.verdict === 'revise' ? 'revise' : 'accept';
  const issues = Array.isArray(obj.issues)
    ? obj.issues
        .filter((i) => i && typeof i === 'object')
        .map((i) => ({
          type: String(i.type || '논증·문체'),
          sentence: String(i.sentence || ''),
          comment: String(i.comment || ''),
          suggestion: String(i.suggestion || ''),
        }))
    : [];

  return {
    verdict,
    issues,
  };
}

module.exports = {
  critique,
  buildCriticPrompt,
};
