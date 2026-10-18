'use strict';

const fs = require('node:fs');
const path = require('node:path');
const crypto = require('node:crypto');

const { createEvidenceStore } = require('./evidence_store');
const { splitClaims } = require('./verify/claim_splitter');
const { runDeterministic } = require('./verify/deterministic');
const { checkFacts } = require('./verify/fact_checker');
const { checkCitations } = require('./verify/citation_checker');
const { assemble } = require('./assembler');
const { buildCrosswalk } = require('./crosswalk');

function sha256File(filePath) {
  const buf = fs.readFileSync(filePath);
  const hex = crypto.createHash('sha256').update(buf).digest('hex');
  return `sha256:${hex}`;
}

function buildReadmeClaude() {
  return [
    '# Claude 2차 학술 다듬기 인계 지침 (ARCHAEO STUDIO 6.1)',
    '',
    '이 폴더(`handoff_for_claude/`)는 ARCHAEO STUDIO 6.1 파이프라인의 1차 집필 및 교차검증을 통과한 근거 묶음입니다.',
    'Claude는 아래 규칙을 엄격히 준수하여 `manuscript_v1.md`를 다듬고, 같은 폴더에 `manuscript_v2.md`로 저장하십시오.',
    '',
    '## 1. 해도 되는 일 (허용 범위)',
    '- 학술적 문장 다듬기 및 가독성 개선',
    '- 단락 간 논리적 연결 및 전개 순서 조정',
    '- 고고학 전문 용어 통일',
    '- `verify_summary.md`의 심사(X3) 의견 반영 및 논증 보강',
    '',
    '## 2. 절대 해서는 안 되는 일 (금지 규칙 — 반입 시 자동 재검증으로 거부됨)',
    '1. **근거 표시 임의 삭제·추가 금지**: 본문 내 `⟦F12⟧`, `⟦L3⟧`, `⟦F1,L2⟧` 형태의 근거 표시를 임의로 지우거나 `facts.json`/`literature.json`에 없는 번호를 추가하지 마십시오.',
    '2. **미등록 숫자 추가 금지**: `facts.json`에 없는 숫자·치수·연대·유구 번호를 절대 추가하지 마십시오(V1/V2 위반).',
    '3. **미등록 문헌 추가 금지**: `literature.json`에 없는 저자·연도·논문명을 절대 추가하지 마십시오(V4 위반).',
    '4. **`〔확인 필요〕` 표식 임의 제거 금지**: 본문에 부착된 `〔확인 필요: ...〕` 표식을 원문 근거 확인 없이 지우지 마십시오.',
    '5. **근거 카드 파일 수정 금지**: `facts.json`과 `literature.json`은 `manifest.json`의 SHA-256 해시로 잠겨 있습니다. 수정 시 반입 관문(G6)에서 거부됩니다.',
    '',
    '## 3. 산출물 저장 규칙',
    '- 다듬은 원고는 반드시 근거 표시(`⟦F...⟧`, `⟦L...⟧`)를 유지한 상태로 `manuscript_v2.md`에 저장하십시오.',
    '- 반입(Import) 시 ARCHAEO STUDIO가 자동으로 `V1~V8` 결정적 검사를 재실행한 뒤 최종 각주(`1)`, `2)`)와 참고문헌으로 조립합니다.',
  ].join('\n');
}

function buildVerifySummaryMarkdown(jobDir, outline) {
  const verifyDir = path.join(jobDir, 'verify');
  const lines = ['# 1차 교차검증(P5) 및 심사(X3) 요약', ''];
  if (!fs.existsSync(verifyDir)) {
    lines.push('- 검증 매트릭스 기록 없음');
    return lines.join('\n');
  }
  const files = fs.readdirSync(verifyDir).filter((f) => f.endsWith('.json')).sort();
  if (files.length === 0) {
    lines.push('- 검증 매트릭스 기록 없음');
    return lines.join('\n');
  }
  for (const f of files) {
    try {
      const m = JSON.parse(fs.readFileSync(path.join(verifyDir, f), 'utf8'));
      lines.push(`## 절 ${m.sectionId} (회차 r${m.round})`);
      lines.push(
        `- 요약: 통과 ${m.summary?.pass ?? 0}건 / 재작성 ${m.summary?.rework ?? 0}건 / 확인 필요 ${m.summary?.needsHuman ?? 0}건`
      );
      if (m.critic) {
        const s = m.critic.scores || {};
        lines.push(
          `- 심사(X3) 판정: ${m.critic.verdict || 'accept'} (근거:${s.evidence ?? '-'}, 포괄:${s.coverage ?? '-'}, 논리:${s.logic ?? '-'}, 문체:${s.tone ?? '-'})`
        );
        for (const iss of m.critic.issues || []) {
          lines.push(`  - [${iss.type}] "${iss.sentence}" → ${iss.comment} (제안: ${iss.suggestion})`);
        }
      }
      lines.push('');
    } catch (_) {}
  }
  return lines.join('\n');
}

/**
 * 8.1 & 8.2 Claude 인계 묶음 내보내기 (P7 exportHandoff)
 */
function exportHandoff(jobDir) {
  if (!jobDir || !fs.existsSync(jobDir)) {
    throw new Error(`작업 폴더가 존재하지 않습니다: ${jobDir}`);
  }
  const store = createEvidenceStore(jobDir);
  const facts = store.loadFacts() || [];
  const literature = store.loadLiterature() || [];
  const outline = store.loadOutline() || { docType: 'report', title: 'Ⅳ. 고찰', sections: [] };

  const handoffDir = path.join(jobDir, 'handoff_for_claude');
  fs.mkdirSync(handoffDir, { recursive: true });

  const v1Src = path.join(jobDir, 'manuscript_v1.md');
  const manuscriptV1 = fs.existsSync(v1Src)
    ? fs.readFileSync(v1Src, 'utf8')
    : fs.existsSync(path.join(jobDir, 'manuscript.md'))
      ? fs.readFileSync(path.join(jobDir, 'manuscript.md'), 'utf8')
      : '# 원고 없음\n';

  const crosswalkSrc = path.join(jobDir, 'crosswalk.md');
  const crosswalkMd = fs.existsSync(crosswalkSrc)
    ? fs.readFileSync(crosswalkSrc, 'utf8')
    : '# 근거 대조표\n';

  const needsHumanSrc = path.join(jobDir, 'needs_human.md');
  const needsHumanMd = fs.existsSync(needsHumanSrc)
    ? fs.readFileSync(needsHumanSrc, 'utf8')
    : '# 확인 필요 항목\n';

  const readmePath = path.join(handoffDir, 'README_CLAUDE.md');
  const v1Path = path.join(handoffDir, 'manuscript_v1.md');
  const factsPath = path.join(handoffDir, 'facts.json');
  const litPath = path.join(handoffDir, 'literature.json');
  const outlinePath = path.join(handoffDir, 'outline.json');
  const verifySummaryPath = path.join(handoffDir, 'verify_summary.md');
  const cwPath = path.join(handoffDir, 'crosswalk.md');
  const nhPath = path.join(handoffDir, 'needs_human.md');
  const manifestPath = path.join(handoffDir, 'manifest.json');

  fs.writeFileSync(readmePath, buildReadmeClaude(), 'utf8');
  fs.writeFileSync(v1Path, manuscriptV1, 'utf8');
  fs.writeFileSync(factsPath, JSON.stringify(facts, null, 2), 'utf8');
  fs.writeFileSync(litPath, JSON.stringify(literature, null, 2), 'utf8');
  fs.writeFileSync(outlinePath, JSON.stringify(outline, null, 2), 'utf8');
  fs.writeFileSync(verifySummaryPath, buildVerifySummaryMarkdown(jobDir, outline), 'utf8');
  fs.writeFileSync(cwPath, crosswalkMd, 'utf8');
  fs.writeFileSync(nhPath, needsHumanMd, 'utf8');

  // 작업 폴더 루트의 facts.json / literature.json / manuscript_v1.md 도 동일 바이트로 동기화하여 해시 기준 일치 보장
  fs.writeFileSync(path.join(jobDir, 'facts.json'), fs.readFileSync(factsPath));
  fs.writeFileSync(path.join(jobDir, 'literature.json'), fs.readFileSync(litPath));
  fs.writeFileSync(path.join(jobDir, 'manuscript_v1.md'), fs.readFileSync(v1Path));

  const manifest = {
    jobId: path.basename(jobDir),
    createdAt: new Date().toISOString(),
    docType: outline.docType || 'report',
    files: {
      'facts.json': sha256File(factsPath),
      'literature.json': sha256File(litPath),
      'manuscript_v1.md': sha256File(v1Path),
    },
  };
  fs.writeFileSync(manifestPath, JSON.stringify(manifest, null, 2), 'utf8');

  return {
    ok: true,
    handoffDir,
    manifest,
    files: [
      'README_CLAUDE.md',
      'manuscript_v1.md',
      'facts.json',
      'literature.json',
      'outline.json',
      'verify_summary.md',
      'crosswalk.md',
      'needs_human.md',
      'manifest.json',
    ],
  };
}

/**
 * 8.3 & 4.7 게이트 G6: 2차 원고 반입 및 재검증 (importHandoff)
 * 1) manifest.json의 SHA-256 해시와 현재 facts.json·literature.json 대조
 * 2) manuscript_v2.md에 대해 결정적 검사 V1~V8 전체 재실행
 * 3) manuscript_v1.md 대비 새로 인용된 카드나 숫자가 추가된 문장은 X1/X2 재실행
 * 4) 통과 시 각주·참고문헌 재조립 후 manuscript.md 갱신
 */
async function importHandoff(jobDir, manuscriptV2Path = null, options = {}) {
  const handoffDir = path.join(jobDir, 'handoff_for_claude');
  const targetV2Path =
    manuscriptV2Path || path.join(handoffDir, 'manuscript_v2.md');

  if (!fs.existsSync(targetV2Path)) {
    throw new Error(`2차 원고 파일을 찾을 수 없습니다: ${targetV2Path}`);
  }

  const manifestPath = path.join(handoffDir, 'manifest.json');
  if (!fs.existsSync(manifestPath)) {
    throw new Error(`manifest.json 파일이 없습니다: ${manifestPath}`);
  }

  const manifest = JSON.parse(fs.readFileSync(manifestPath, 'utf8'));
  const handoffFactsPath = path.join(handoffDir, 'facts.json');
  const handoffLitPath = path.join(handoffDir, 'literature.json');
  const jobFactsPath = path.join(jobDir, 'facts.json');
  const jobLitPath = path.join(jobDir, 'literature.json');

  const actualFactsHash = fs.existsSync(handoffFactsPath)
    ? sha256File(handoffFactsPath)
    : sha256File(jobFactsPath);
  const actualLitHash = fs.existsSync(handoffLitPath)
    ? sha256File(handoffLitPath)
    : sha256File(jobLitPath);

  const factsHashMatch = manifest.files?.['facts.json'] === actualFactsHash;
  const litHashMatch = manifest.files?.['literature.json'] === actualLitHash;
  const cardHashMatch = Boolean(factsHashMatch && litHashMatch);

  if (!cardHashMatch) {
    const gate = {
      gate: 'G6',
      ok: false,
      cardHashMismatch: true,
      reason: `인계 묶음 해시 불일치 (facts.json 일치:${factsHashMatch}, literature.json 일치:${litHashMatch})`,
    };
    return {
      ok: false,
      gate,
      cardHashMismatch: true,
    };
  }

  const store = createEvidenceStore(jobDir);
  const facts = store.loadFacts() || [];
  const literature = store.loadLiterature() || [];
  const pages = store.loadPages() || [];
  const outline = store.loadOutline() || { docType: 'report', title: 'Ⅳ. 고찰', sections: [] };

  const v2Raw = fs.readFileSync(targetV2Path, 'utf8');
  const v1Path = path.join(handoffDir, 'manuscript_v1.md');
  const v1Raw = fs.existsSync(v1Path) ? fs.readFileSync(v1Path, 'utf8') : '';

  const v1Claims = splitClaims(v1Raw, 'V1', 0);
  const v1Sentences = new Set(v1Claims.map((c) => c.sentence.trim()));

  const v2Claims = splitClaims(v2Raw, 'V2', 0);
  const detResult = runDeterministic({
    markdown: v2Raw,
    claims: v2Claims,
    facts,
    literature,
    section: { id: 'V2', heading: outline.title, claims: [] },
  });

  // v1에 없던 새 문장 중 카드 표식이나 측정 수치가 있는 문장 추출 -> 선택적 X1/X2 재검증 대상
  const modifiedClaims = v2Claims.filter(
    (c) => !v1Sentences.has(c.sentence.trim()) && (c.markers.length > 0 || c.kind !== 'interpretation')
  );

  let x1Reverify = null;
  let x2Reverify = null;
  if (modifiedClaims.length > 0 && options.geminiDriver) {
    x1Reverify = await checkFacts({
      claims: modifiedClaims,
      facts,
      pages,
      geminiDriver: options.geminiDriver,
      sectionId: 'V2',
      round: 0,
    });
    x2Reverify = await checkCitations({
      claims: modifiedClaims,
      literature,
      geminiDriver: options.geminiDriver,
      sectionId: 'V2',
      round: 0,
    });
  }

  const g6Ok = Boolean(detResult.ok && !detResult.hasBlock && cardHashMatch);
  const blockReasons = detResult.claimResults
    .filter((cr) => cr.status === 'block')
    .flatMap((cr) => cr.reasons);
  if (detResult.v8Issues?.length > 0) {
    blockReasons.push(...detResult.v8Issues);
  }

  const gate = {
    gate: 'G6',
    ok: g6Ok,
    cardHashMismatch: false,
    deterministicOk: detResult.ok,
    blockReasons,
    reverifiedClaimCount: modifiedClaims.length,
    reason: g6Ok
      ? `2차 원고 반입 관문(G6) 통과 (결정적 검사 통과, 변경 문장 ${modifiedClaims.length}건 점검)`
      : `2차 원고 반입 관문(G6) 거부: ${blockReasons.join('; ')}`,
  };

  if (!g6Ok) {
    return {
      ok: false,
      gate,
      detResult,
      triggeredReverifyClaims: modifiedClaims.map((c) => c.id),
      x1Reverify,
      x2Reverify,
    };
  }

  // 통과 시 최종 원고(manuscript.md) 및 대조표(crosswalk.md) 재조립
  // v2Raw가 이미 최상위 제목(# ...)을 포함하므로 본문 절로 감싸 조립
  const bodyWithoutTopTitle = v2Raw.replace(/^#\s+[^\r\n]+\r?\n+/, '').trim();
  const assembled = assemble(outline, [{ markdown: bodyWithoutTopTitle }], facts, literature);

  fs.writeFileSync(path.join(jobDir, 'manuscript_v2.md'), v2Raw, 'utf8');
  fs.writeFileSync(path.join(jobDir, 'manuscript.md'), assembled.manuscriptMd, 'utf8');

  const cw = buildCrosswalk(v2Raw, facts, literature, []);
  fs.writeFileSync(path.join(jobDir, 'crosswalk.md'), cw.markdown, 'utf8');
  fs.writeFileSync(path.join(jobDir, 'needs_human.md'), cw.needsHumanMarkdown, 'utf8');

  return {
    ok: true,
    gate,
    detResult,
    triggeredReverifyClaims: modifiedClaims.map((c) => c.id),
    x1Reverify,
    x2Reverify,
    manuscriptMd: assembled.manuscriptMd,
    footnotes: assembled.footnotes,
    bibliography: assembled.bibliography,
  };
}

module.exports = {
  exportHandoff,
  importHandoff,
  sha256File,
  buildReadmeClaude,
};
