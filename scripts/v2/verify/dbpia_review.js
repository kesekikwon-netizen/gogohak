'use strict';

const fs = require('node:fs');
const path = require('node:path');

const { splitClaims } = require('./claim_splitter');
const { parseMarkers, stripAllMarkers } = require('../markers');
const { toLiteratureCards, mergeLiterature } = require('../literature_cards');
const { createEvidenceStore } = require('../evidence_store');

const PROMPT_PATH = path.join(__dirname, '..', 'prompts', 'dbpia_review_query.md');
const PROMPT_TEMPLATE = fs.readFileSync(PROMPT_PATH, 'utf8');
const DEFAULT_CONFIG = JSON.parse(fs.readFileSync(path.join(__dirname, '..', 'config.default.json'), 'utf8'));

const VALID_VERDICTS = new Set(['일치', '이견있음', '상충', '미검증']);

/**
 * 7.7.1 검수 대상 주장 선별 (최대 maxQueries = 5개)
 * - 각 절에서 대표 해석/인용 주장 1개씩 우선 선별
 * - 남은 자리는 L 문헌 인용이나 편년·계통·비교 어휘가 들어간 핵심 문장으로 채움
 */
function selectReviewClaims(outline = {}, manuscriptMd = '', facts = [], config = DEFAULT_CONFIG) {
  const maxQueries = config?.dbpiaReview?.maxQueries ?? 5;
  const rawClaims = splitClaims(manuscriptMd, 'REV', 0);

  // 해석·인용 문장 또는 편년/계통/비교 어휘가 든 문장 필터
  const candidates = rawClaims.filter((c) => {
    const s = String(c.sentence || '');
    const parsed = parseMarkers(s);
    if (parsed.litIds.length > 0) return true;
    if (/(편년|계통|단계|비교|양식|문화권|전통|변천|성격|세기|시대)/.test(s)) return true;
    return c.kind === 'citation' || c.kind === 'interpretation';
  });

  const pool = candidates.length > 0 ? candidates : rawClaims;
  const selected = [];
  const usedSentences = new Set();

  // 1) outline.sections가 있으면 절별로 1개씩 배정 시도
  const sections = Array.isArray(outline?.sections) ? outline.sections : [];
  for (let i = 0; i < sections.length && selected.length < maxQueries; i++) {
    const sec = sections[i];
    const match = pool.find((c) => !usedSentences.has(c.sentence));
    if (match) {
      usedSentences.add(match.sentence);
      selected.push({
        claimId: match.id,
        sectionId: sec.id || `S${i + 1}`,
        heading: sec.heading || '',
        sentence: match.sentence,
      });
    }
  }

  // 2) 남은 슬롯 채우기
  for (const c of pool) {
    if (selected.length >= maxQueries) break;
    if (usedSentences.has(c.sentence)) continue;
    usedSentences.add(c.sentence);
    selected.push({
      claimId: c.id,
      sectionId: sections[0]?.id || 'S1',
      heading: sections[0]?.heading || '',
      sentence: c.sentence,
    });
  }

  return selected.slice(0, maxQueries);
}

/**
 * 7.7.2 학술 질의문 변환 (유적 고유명 제거 규칙)
 * - 당해 유적 고유명(kind === 'site' 카드의 subject/value 또는 '~유적')을 제거하고,
 *   시대·지역권·유구 형태·유물 형식·편년 주제어만 남겨 학계 일반 논의를 질의한다.
 */
function buildReviewQuery(claim, facts = []) {
  const rawSentence = stripAllMarkers(String(claim?.sentence || claim || ''));
  const siteNames = new Set();

  for (const f of facts || []) {
    if (f && f.kind === 'site') {
      if (f.subject) siteNames.add(String(f.subject).trim());
      if (f.value && /유적|지구/.test(String(f.value))) siteNames.add(String(f.value).trim());
    }
  }

  let cleaned = rawSentence;
  for (const sName of siteNames) {
    if (sName && sName.length >= 2) {
      const escaped = sName.replace(/[.*+?^${}()|[\]\\]/g, '\\$&');
      cleaned = cleaned.replace(new RegExp(escaped, 'g'), ' ');
    }
  }

  // 일반적인 'OO유적', '가상유적', '본 유적', '조사유적', 'N호 주거지' 등 개별 유구 번호 표현 제거
  cleaned = cleaned
    .replace(/[가-힣A-Za-z0-9]{1,10}\s*유적(?:에서|의|은|는|이|가)?/g, ' ')
    .replace(/\b(?:본|당해|조사)\s*유적\b/g, ' ')
    .replace(/\d+\s*호\s*/g, '')
    .replace(/\(\s*(?:[가-힣A-Za-z·\s]+,?\s*)?(?:19|20)\d{2}[a-z]?\s*\)/g, ' ')
    .replace(/[.,;:"'「」『』()[\]]/g, ' ')
    .replace(/\s+/g, ' ')
    .trim();

  // 핵심 학술 키워드 추출
  const stopWords = new Set(['따라서', '그러나', '한편', '또한', '이러한', '통해', '바탕으로', '확인되었다', '판단된다', '사료된다', '보인다']);
  const tokens = cleaned
    .split(' ')
    .map((t) => t.replace(/(?:은|는|이|가|을|를|의|에|에서|로|으로|와|과|도|만)$/, ''))
    .filter((t) => t.length >= 2 && !stopWords.has(t));

  const coreKeywords = Array.from(new Set(tokens)).slice(0, 7).join(' ');
  return `${coreKeywords || '고고학 유구 구조 및 출토유물'}의 편년·계통·학계 연구 동향은?`;
}

/**
 * 기존 문헌 카드 목록과 DBpia AI 반환 참고문헌을 비교하여 빠진 새 문헌(newLiterature)을 식별한다.
 */
function findMissingLiterature(dbpiaRefs = [], existingLit = [], query = '') {
  const existingKeys = new Set();
  for (const l of existingLit || []) {
    if (l.nodeId) existingKeys.add(`node:${String(l.nodeId).trim()}`);
    if (l.title) {
      existingKeys.add(`title:${String(l.title).replace(/\s+/g, '').toLowerCase()}:${String(l.year || '').trim()}`);
    }
  }

  const candidateCards = toLiteratureCards({ refs: dbpiaRefs }, query, 1);
  const missing = [];
  let seq = 1;
  for (const c of candidateCards) {
    if (!c.verified) continue;
    const nodeKey = c.nodeId ? `node:${c.nodeId}` : '';
    const titleKey = `title:${String(c.title || '').replace(/\s+/g, '').toLowerCase()}:${String(c.year || '').trim()}`;
    if ((nodeKey && existingKeys.has(nodeKey)) || existingKeys.has(titleKey)) {
      continue;
    }
    missing.push({
      ...c,
      id: `L_new${seq++}`,
    });
  }
  return missing;
}

/**
 * 7.7 최종 DBpia AI 문헌 검수 실행 (P7.5 / X4)
 * - 자동 고쳐쓰기 금지(7.7.4): 이 함수는 원고(manuscript.md / manuscript_v1.md)를 절대 직접 수정하지 않는다.
 */
async function runDbpiaFinalReview({
  jobDir,
  outline = {},
  manuscriptMd = '',
  facts = [],
  literature = [],
  dbpiaDriver,
  geminiDriver,
  config = DEFAULT_CONFIG,
} = {}) {
  const selectedClaims = selectReviewClaims(outline, manuscriptMd, facts, config);
  const items = [];
  const allNewLit = [];

  const existingLitText =
    (literature || [])
      .filter((l) => l && l.verified)
      .map((l) => `- [${l.id}] ${(l.authors || []).join('·')}(${l.year}), 「${l.title}」, 『${l.journal || ''}』`)
      .join('\n') || '(기존 수집 문헌 없음)';

  for (let i = 0; i < selectedClaims.length; i++) {
    const item = selectedClaims[i];
    const query = buildReviewQuery(item, facts);
    const stepId = `P75.X4.Q${i + 1}`;

    let dbRes = { answerText: '', refs: [] };
    try {
      dbRes = await dbpiaDriver.query(query, { stepId });
    } catch (_) {
      items.push({
        claimId: item.claimId,
        sectionId: item.sectionId,
        sentence: item.sentence,
        query,
        verdict: '미검증',
        keyPoints: 'DBpia AI 응답 실패로 미검증 처리',
        conflictDetail: '',
        suggestion: '',
        newLiterature: [],
      });
      continue;
    }

    const newLitForClaim = findMissingLiterature(dbRes.refs || dbRes.references || [], literature, query);
    allNewLit.push(...newLitForClaim);

    const refsText =
      (dbRes.refs || dbRes.references || [])
        .map(
          (r, idx) =>
            `${idx + 1}. ${r.author || (r.authors || []).join('·') || '저자'}(${r.year || ''}), 「${r.title || ''}」, 『${r.journal || ''}』 | 초록: ${r.abstract || (r.quotes || []).join(' ') || ''}`
        )
        .join('\n') || '(참고문헌 없음)';

    const prompt = PROMPT_TEMPLATE
      .replace(/\{\{SECTION_ID\}\}/g, item.sectionId)
      .replace(/\{\{CLAIM_ID\}\}/g, item.claimId)
      .replace('{{CLAIM_SENTENCE}}', item.sentence)
      .replace('{{DBPIA_QUERY}}', query)
      .replace('{{EXISTING_LIT_TEXT}}', existingLitText)
      .replace('{{DBPIA_ANSWER_TEXT}}', String(dbRes.answerText || '').slice(0, 1200))
      .replace('{{DBPIA_REFS_TEXT}}', refsText.slice(0, 1500))
      .replace(/\{\{STEP_ID\}\}/g, stepId);

    let verdict = '미검증';
    let keyPoints = String(dbRes.answerText || '').slice(0, 200);
    let conflictDetail = '';
    let suggestion = '';

    if (geminiDriver && typeof geminiDriver.ask === 'function') {
      try {
        const gRes = await geminiDriver.ask({
          jobDir,
          jobStep: stepId,
          prompt,
          expectJson: true,
          newChat: true,
        });
        const parsed = gRes.json || {};
        if (VALID_VERDICTS.has(parsed.verdict)) {
          verdict = parsed.verdict;
        }
        if (parsed.keyPoints) keyPoints = String(parsed.keyPoints).trim();
        if (parsed.conflictDetail) conflictDetail = String(parsed.conflictDetail).trim();
        if (parsed.suggestion) suggestion = String(parsed.suggestion).trim();
      } catch (_) {}
    }

    items.push({
      claimId: item.claimId,
      sectionId: item.sectionId,
      sentence: item.sentence,
      query,
      verdict,
      keyPoints,
      conflictDetail,
      suggestion,
      newLiterature: newLitForClaim,
    });
  }

  // 중복 제거된 보충 문헌 후보 목록
  const dedupNewLit = mergeLiterature(allNewLit).map((c, idx) => ({
    ...c,
    id: `L_new${idx + 1}`,
  }));

  const summary = {
    total: items.length,
    concur: items.filter((x) => x.verdict === '일치').length,
    debate: items.filter((x) => x.verdict === '이견있음').length,
    conflict: items.filter((x) => x.verdict === '상충').length,
    unverified: items.filter((x) => x.verdict === '미검증').length,
    newLiteratureCount: dedupNewLit.length,
  };

  const reviewResult = {
    checkedAt: new Date().toISOString(),
    summary,
    items,
    newLiterature: dedupNewLit,
  };

  if (jobDir) {
    const verifyDir = path.join(jobDir, 'verify');
    fs.mkdirSync(verifyDir, { recursive: true });
    fs.writeFileSync(
      path.join(verifyDir, 'dbpia_final_review.json'),
      JSON.stringify(reviewResult, null, 2),
      'utf8'
    );
    fs.writeFileSync(
      path.join(verifyDir, 'dbpia_final_review.md'),
      formatDbpiaReviewMarkdown(reviewResult),
      'utf8'
    );
  }

  return reviewResult;
}

function formatDbpiaReviewMarkdown(reviewResult) {
  const s = reviewResult?.summary || {};
  const lines = [
    '# 최종 DBpia AI 문헌 검수 보고서 (P7.5 / X4)',
    '',
    `- 검수 시각: ${reviewResult?.checkedAt || ''}`,
    `- 검수 요약: 총 ${s.total ?? 0}건 (일치 ${s.concur ?? 0} / 이견있음 ${s.debate ?? 0} / 상충 ${s.conflict ?? 0} / 미검증 ${s.unverified ?? 0})`,
    `- 새로 발견된 보충 문헌 후보: ${s.newLiteratureCount ?? 0}건`,
    '',
    '## 1. 주장별 검수 결과',
    '',
    '| 주장 ID | 절 | 판정 | DBpia 질의 | 핵심 논점 및 상충 사유 |',
    '| :---: | :---: | :---: | :--- | :--- |',
  ];

  for (const it of reviewResult?.items || []) {
    const detail = [it.keyPoints, it.conflictDetail, it.suggestion].filter(Boolean).join(' / ').replace(/\|/g, '/');
    lines.push(`| ${it.claimId} | ${it.sectionId} | **${it.verdict}** | ${it.query} | ${detail} |`);
  }

  lines.push('', '## 2. 보충 문헌 후보 (기존 수집 문헌에 없던 논문)', '');
  if (!reviewResult?.newLiterature || reviewResult.newLiterature.length === 0) {
    lines.push('- 추가로 발견된 누락 핵심 문헌 없음');
  } else {
    for (const nl of reviewResult.newLiterature) {
      lines.push(
        `- **[${nl.id}]** ${(nl.authors || []).join('·')}(${nl.year}), 「${nl.title}」, 『${nl.journal || ''}』 (nodeId: ${nl.nodeId || '미상'})`
      );
    }
  }

  return lines.join('\n') + '\n';
}

/**
 * 4.7 게이트 G7 판정:
 * - 상충 === 0 -> ok: true
 * - 상충 >= 1 -> ok: false (사용자 선택 필요, 자동 재작성 금지)
 */
function evaluateGateG7(reviewResult) {
  const conflictCount = reviewResult?.summary?.conflict ?? 0;
  const debateCount = reviewResult?.summary?.debate ?? 0;
  const newLitCount = reviewResult?.summary?.newLiteratureCount ?? 0;
  const ok = conflictCount === 0;

  return {
    gate: 'G7',
    ok,
    conflictCount,
    debateCount,
    newLiteratureCount: newLitCount,
    canContinue: true,
    reason: ok
      ? `최종 DBpia AI 문헌 검수 통과 (상충 0건, 이견 ${debateCount}건, 보충 문헌 후보 ${newLitCount}건)`
      : `최종 DBpia AI 문헌 검수에서 학계 문헌과 상충하는 주장 ${conflictCount}건 발견 — 사용자 확인 필요`,
  };
}

/**
 * 7.7.4 사용자 선택 처리 (handleDbpiaReviewAction)
 * - action === 'rewrite': 보충 문헌을 literature.json에 병합하고 상충 절의 완료 체크포인트를 해제하여 재작성 준비
 * - action === 'tag': 원고는 재작성하지 않고 상충 문장에 〔확인 필요: DBpia 문헌 검수 상충 — ...〕 표식만 부착
 * - action === 'ignore': 원고 변경 없이 통과 처리
 */
function handleDbpiaReviewAction(jobDir, action = 'tag') {
  const reviewPath = path.join(jobDir, 'verify', 'dbpia_final_review.json');
  if (!fs.existsSync(reviewPath)) {
    throw new Error('dbpia_final_review.json 파일이 없습니다.');
  }
  const review = JSON.parse(fs.readFileSync(reviewPath, 'utf8'));
  const store = createEvidenceStore(jobDir);

  if (action === 'rewrite') {
    const existingLit = store.loadLiterature() || [];
    const merged = mergeLiterature([...existingLit, ...(review.newLiterature || [])]);
    store.saveLiterature(merged);
    const conflictSections = Array.from(
      new Set((review.items || []).filter((i) => i.verdict === '상충').map((i) => i.sectionId))
    );
    for (const secId of conflictSections) {
      const finalSecPath = path.join(jobDir, 'sections', `${secId}.final.md`);
      if (fs.existsSync(finalSecPath)) {
        fs.unlinkSync(finalSecPath);
      }
    }
    return { ok: true, action: 'rewrite', addedLiterature: merged.length - existingLit.length, conflictSections };
  }

  if (action === 'tag') {
    const msPath = path.join(jobDir, 'manuscript.md');
    if (fs.existsSync(msPath)) {
      let md = fs.readFileSync(msPath, 'utf8');
      for (const item of review.items || []) {
        if (item.verdict === '상충') {
          const rawSent = String(item.sentence || '').trim();
          const cleanSent = stripAllMarkers(rawSent).trim();
          const corePrefix = cleanSent.replace(/[.!?]+$/, '').trim();
          const tag = ` 〔확인 필요: DBpia 문헌 검수 상충 — ${item.conflictDetail || item.keyPoints || '선행연구 재확인 필요'}〕`;
          if (!md.includes(tag)) {
            if (rawSent && md.includes(rawSent)) {
              md = md.replace(rawSent, `${rawSent}${tag}`);
            } else if (cleanSent && md.includes(cleanSent)) {
              md = md.replace(cleanSent, `${cleanSent}${tag}`);
            } else if (corePrefix && md.includes(corePrefix)) {
              md = md.replace(corePrefix, `${corePrefix}${tag}`);
            }
          }
        }
      }
      fs.writeFileSync(msPath, md, 'utf8');
    }
    return { ok: true, action: 'tag' };
  }

  return { ok: true, action: 'ignore' };
}

module.exports = {
  selectReviewClaims,
  buildReviewQuery,
  findMissingLiterature,
  runDbpiaFinalReview,
  formatDbpiaReviewMarkdown,
  evaluateGateG7,
  handleDbpiaReviewAction,
};
