'use strict';

const fs = require('node:fs');
const path = require('node:path');
const { makeSectionId } = require('./ids');
const { validateOutline } = require('./schemas');

const PROMPT_PATH = path.join(__dirname, 'prompts', 'outline.md');
const PROMPT_TEMPLATE = fs.readFileSync(PROMPT_PATH, 'utf8');
const DEFAULT_CONFIG = JSON.parse(fs.readFileSync(path.join(__dirname, 'config.default.json'), 'utf8'));

function parseJsonBlock(text) {
  const raw = String(text || '');
  try {
    const server = require('../gemini_mcp_server');
    if (typeof server.parseJsonFromGeminiReply === 'function') {
      const parsed = server.parseJsonFromGeminiReply(raw);
      if (parsed && typeof parsed === 'object') return parsed;
    }
  } catch (_) {}

  const blocks = [...raw.matchAll(/```(?:json)?\s*([\s\S]*?)```/gi)];
  const candidate = blocks.length > 0 ? blocks[blocks.length - 1][1].trim() : raw.trim();
  return JSON.parse(candidate);
}

/**
 * 5.2 프롬프트 길이 제한(maxPromptChars)을 지키며 목차 생성 프롬프트를 조립한다.
 * 한도를 넘으면 우선순위가 낮은 문헌 카드(quotes 없는 카드 -> 요약만 남김)부터 줄이고,
 * 사실 카드의 quote는 줄이지 않는다.
 */
function buildOutlinePrompt({
  docType = 'report',
  userFocus = '',
  facts = [],
  literature = [],
  stepId = 'P3.OUTLINE',
  maxPromptChars = DEFAULT_CONFIG?.gemini?.maxPromptChars || 12000,
} = {}) {
  const logs = [];
  const factLines = (facts || []).map(
    (f) => `- [${f.id}] (${f.kind}, ${f.confidence || 'verified'}) ${f.subject} / ${f.attribute}: ${f.value}${f.unit || ''} | 원문: "${f.quote}"`
  );
  const factText = factLines.length > 0 ? factLines.join('\n') : '(사실 카드 없음)';

  const sortedLit = [...(literature || [])].sort((a, b) => {
    const scoreA = (a.verified ? 2 : 0) + (Array.isArray(a.quotes) && a.quotes.length > 0 ? 1 : 0);
    const scoreB = (b.verified ? 2 : 0) + (Array.isArray(b.quotes) && b.quotes.length > 0 ? 1 : 0);
    return scoreB - scoreA;
  });

  const renderLit = (litItems, compactMode = false) => {
    if (!litItems || litItems.length === 0) return '(문헌 카드 없음)';
    return litItems
      .map((l) => {
        const authors = Array.isArray(l.authors) ? l.authors.join('·') : '';
        if (compactMode) {
          return `- [${l.id}] ${authors}(${l.year}), 「${l.title}」 (verified:${Boolean(l.verified)})`;
        }
        const quoteStr = Array.isArray(l.quotes) && l.quotes.length > 0 ? ` | 인용: "${l.quotes[0].slice(0, 140)}"` : '';
        return `- [${l.id}] ${authors}(${l.year}), 「${l.title}」, 『${l.journal || ''}』 | 요점: ${(l.summary || '').slice(0, 120)}${quoteStr}`;
      })
      .join('\n');
  };

  let litText = renderLit(sortedLit, false);
  let prompt = PROMPT_TEMPLATE
    .replace(/\{\{DOC_TYPE\}\}/g, docType === 'paper' ? 'paper' : 'report')
    .replace(/\{\{USER_FOCUS\}\}/g, String(userFocus || '유구·유물 종합 고찰'))
    .replace('{{FACT_CARDS_TEXT}}', factText)
    .replace('{{LIT_CARDS_TEXT}}', litText)
    .replace(/\{\{STEP_ID\}\}/g, stepId);

  if (prompt.length > maxPromptChars && sortedLit.length > 0) {
    logs.push(`프롬프트 길이(${prompt.length}자)가 한도(${maxPromptChars}자)를 초과하여 문헌 카드를 요약 모드로 축약함`);
    litText = renderLit(sortedLit, true);
    prompt = PROMPT_TEMPLATE
      .replace(/\{\{DOC_TYPE\}\}/g, docType === 'paper' ? 'paper' : 'report')
      .replace(/\{\{USER_FOCUS\}\}/g, String(userFocus || '유구·유물 종합 고찰'))
      .replace('{{FACT_CARDS_TEXT}}', factText)
      .replace('{{LIT_CARDS_TEXT}}', litText)
      .replace(/\{\{STEP_ID\}\}/g, stepId);
  }

  return { prompt, logs };
}

/**
 * 6.3 목차 정리:
 * - 존재하지 않는 카드 번호 제거
 * - 근거 없는 주장(evidence 빈 배열) 제거
 * - 주장 없는 절(claims 빈 배열) 제거 및 omitted로 이동
 */
function sanitizeOutlineObject(rawObj, validCardIds = new Set(), docTypeFallback = 'report') {
  const idSet = validCardIds instanceof Set ? validCardIds : new Set(validCardIds || []);
  const logs = [];
  const docType = rawObj?.docType === 'paper' || rawObj?.docType === 'report' ? rawObj.docType : docTypeFallback;
  const title = String(rawObj?.title || (docType === 'paper' ? '학술논문' : 'Ⅳ. 고찰')).trim();
  const omitted = Array.isArray(rawObj?.omitted)
    ? rawObj.omitted
        .filter((o) => o && o.heading)
        .map((o) => ({ heading: String(o.heading).trim(), reason: String(o.reason || '자료 부족').trim() }))
    : [];

  const rawSections = Array.isArray(rawObj?.sections) ? rawObj.sections : [];
  const cleanSections = [];

  for (let i = 0; i < rawSections.length; i++) {
    const sec = rawSections[i];
    if (!sec || typeof sec !== 'object') continue;
    const secId = /^S\d+$/.test(String(sec.id || '')) ? String(sec.id) : makeSectionId(cleanSections.length + 1);
    const heading = String(sec.heading || `${cleanSections.length + 1}. 절`).trim();

    const rawClaims = Array.isArray(sec.claims) ? sec.claims : [];
    const cleanClaims = [];
    for (let j = 0; j < rawClaims.length; j++) {
      const c = rawClaims[j];
      if (!c || typeof c !== 'object') continue;
      const text = String(c.text || '').trim();
      if (!text) continue;
      const rawEv = Array.isArray(c.evidence) ? c.evidence.map((e) => String(e).trim()) : [];
      const validEv = [];
      for (const evId of rawEv) {
        if (idSet.size === 0 || idSet.has(evId)) {
          if (!validEv.includes(evId)) validEv.push(evId);
        } else {
          logs.push(`[목차 정리] ${secId} 주장 "${text.slice(0, 25)}"에서 존재하지 않는 카드 ID 제거: ${evId}`);
        }
      }
      if (validEv.length === 0) {
        logs.push(`[목차 정리] ${secId} 주장 "${text.slice(0, 25)}" 제거: 유효한 근거 카드 없음`);
        continue;
      }
      cleanClaims.push({
        id: /^S\d+\.C\d+$/.test(String(c.id || '')) ? String(c.id) : `${secId}.C${cleanClaims.length + 1}`,
        text,
        evidence: validEv,
      });
    }

    if (cleanClaims.length === 0) {
      logs.push(`[목차 정리] 절 "${heading}" 제거: 유효한 주장이 없음`);
      omitted.push({ heading, reason: '유효한 근거 주장이 없어 제외' });
      continue;
    }

    const rawVisuals = Array.isArray(sec.visuals) ? sec.visuals : [];
    const cleanVisuals = [];
    for (const v of rawVisuals) {
      if (!v || typeof v !== 'object') continue;
      const vType = ['table', 'chart', 'diagram'].includes(v.type) ? v.type : 'table';
      const vEv = (Array.isArray(v.evidence) ? v.evidence : [])
        .map((e) => String(e).trim())
        .filter((e) => idSet.size === 0 || idSet.has(e));
      if (vEv.length === 0) {
        logs.push(`[목차 정리] ${secId} 시각자료("${v.purpose || vType}") 제거: 유효한 근거 카드 없음`);
        continue;
      }
      cleanVisuals.push({
        type: vType,
        purpose: String(v.purpose || `${heading} 시각자료`).trim(),
        evidence: vEv,
      });
    }

    cleanSections.push({
      id: makeSectionId(cleanSections.length + 1),
      heading,
      claims: cleanClaims.map((cl, idx) => ({
        ...cl,
        id: `${makeSectionId(cleanSections.length + 1)}.C${idx + 1}`,
      })),
      visuals: cleanVisuals,
      targetChars: Number.isInteger(sec.targetChars) && sec.targetChars > 0 ? sec.targetChars : 1200,
    });
  }

  return {
    outline: {
      docType,
      title,
      approved: Boolean(rawObj?.approved),
      omitted,
      sections: cleanSections,
    },
    logs,
  };
}

function parseOutline(textOrObj, validCardIds = new Set(), docTypeFallback = 'report') {
  const rawObj = typeof textOrObj === 'string' ? parseJsonBlock(textOrObj) : textOrObj;
  return sanitizeOutlineObject(rawObj, validCardIds, docTypeFallback);
}

/**
 * 4.7 게이트 G3 판정:
 * - 목차 형식 검사(validateOutline) 통과
 * - 모든 주장에 존재하는 카드 번호만 포함
 * - 사용자 승인(outline.approved === true) 또는 자동 승인(autoApprove === true)
 */
function evaluateGateG3(outline, validCardIds = new Set(), autoApprove = false) {
  const idSet = validCardIds instanceof Set ? validCardIds : new Set(validCardIds || []);
  const validation = validateOutline(outline, idSet);
  if (!validation.ok) {
    return {
      gate: 'G3',
      ok: false,
      needsApproval: false,
      errors: validation.errors,
      reason: `목차 형식 검사 실패: ${validation.errors.join('; ')}`,
    };
  }

  const approved = Boolean(outline.approved || autoApprove);
  return {
    gate: 'G3',
    ok: approved,
    needsApproval: !approved,
    errors: [],
    reason: approved
      ? `목차 검증 및 승인 완료 (${outline.sections.length}개 절)`
      : `목차 형식 검증 통과 (${outline.sections.length}개 절) — 사용자 승인 대기 중`,
  };
}

module.exports = {
  buildOutlinePrompt,
  parseOutline,
  sanitizeOutlineObject,
  evaluateGateG3,
};
