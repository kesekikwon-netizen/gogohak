'use strict';

const test = require('node:test');
const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const os = require('node:os');

const { createEvidenceStore } = require('../scripts/v2/evidence_store');
const { exportHandoff, importHandoff } = require('../scripts/v2/handoff');
const { createFakeDrivers } = require('../scripts/v2/harness/fake_drivers');

function setupSampleJobDir() {
  const jobDir = fs.mkdtempSync(path.join(os.tmpdir(), 'archaeo-v2-handoff-'));
  const store = createEvidenceStore(jobDir);

  const facts = [
    {
      id: 'F1',
      kind: 'feature',
      subject: '3호 주거지',
      attribute: '장축',
      value: '6.2',
      unit: 'm',
      quote: '3호 주거지는 장축 6.2m, 단축 4.8m의 평면 凸자형 주거지이다.',
      source: { file: '가상보고서.pdf', page: 4, chapter: 'Ⅲ' },
      confidence: 'verified',
    },
    {
      id: 'F2',
      kind: 'feature',
      subject: '3호 주거지',
      attribute: '단축',
      value: '4.8',
      unit: 'm',
      quote: '3호 주거지는 장축 6.2m, 단축 4.8m의 평면 凸자형 주거지이다.',
      source: { file: '가상보고서.pdf', page: 4, chapter: 'Ⅲ' },
      confidence: 'verified',
    },
  ];

  const literature = [
    {
      id: 'L1',
      nodeId: 'NODE00000001',
      authors: ['홍길동'],
      year: '2015',
      title: '중부지역 원삼국시대 주거지 편년 연구',
      journal: '가상고고학보',
      volume: '12',
      issue: '1',
      pages: '1~30',
      quotes: ['3호 주거지와 같은 凸자형 주거지는 원삼국시대 중기 단계로 편년된다.'],
      summary: '중부지역 원삼국시대 凸자형 주거지의 구조와 편년을 분석하였다.',
      query: '원삼국시대 凸자형 주거지 편년',
      verified: true,
      quoteStatus: 'has_quotes',
    },
  ];

  const outline = {
    docType: 'report',
    title: 'Ⅳ. 고찰 — 가상유적 주거지 검토',
    approved: true,
    omitted: [],
    sections: [
      {
        id: 'S1',
        heading: '1. 주거지 구조와 편년',
        claims: [{ id: 'S1.C1', text: '3호 주거지 규모와 편년', evidence: ['F1', 'F2', 'L1'] }],
        visuals: [],
        targetChars: 100,
      },
    ],
  };

  store.savePages([{ page: 4, text: facts[0].quote }]);
  store.saveFacts(facts);
  store.saveLiterature(literature);
  store.saveOutline(outline);

  const v1Md = [
    '# Ⅳ. 고찰 — 가상유적 주거지 검토',
    '',
    '## 1. 주거지 구조와 편년',
    '',
    '가상유적의 3호 주거지는 장축 6.2m이다 ⟦F1⟧.',
    '중부지역 원삼국시대 凸자형 주거지는 중기 단계로 편년된다 ⟦L1⟧.',
  ].join('\n');

  fs.writeFileSync(path.join(jobDir, 'manuscript_v1.md'), v1Md, 'utf8');
  fs.writeFileSync(path.join(jobDir, 'crosswalk.md'), '# 근거 대조표\n', 'utf8');
  fs.writeFileSync(path.join(jobDir, 'needs_human.md'), '# 확인 필요 항목\n', 'utf8');

  return { jobDir, facts, literature, outline };
}

test('7단계 exportHandoff: 인계 묶음 9종 파일 생성 및 manifest.json SHA-256 해시 기록 검증', () => {
  const { jobDir } = setupSampleJobDir();
  const exp = exportHandoff(jobDir);

  assert.equal(exp.ok, true);
  for (const fileName of exp.files) {
    assert.ok(
      fs.existsSync(path.join(exp.handoffDir, fileName)),
      `handoff_for_claude/${fileName} 파일이 생성되어야 함`
    );
  }
  assert.match(exp.manifest.files['facts.json'], /^sha256:[0-9a-f]{64}$/);
  assert.match(exp.manifest.files['literature.json'], /^sha256:[0-9a-f]{64}$/);
  assert.match(exp.manifest.files['manuscript_v1.md'], /^sha256:[0-9a-f]{64}$/);
});

test('7단계 importHandoff: 미등록 수치(99.9m) 또는 미등록 문헌(김가짜 2024) 포함 시 G6 거부', async () => {
  const { jobDir } = setupSampleJobDir();
  const exp = exportHandoff(jobDir);

  // 미등록 수치(99.9m)와 미등록 저자(김가짜(2024))를 넣은 manuscript_v2.md
  const badV2 = [
    '# Ⅳ. 고찰 — 가상유적 주거지 검토',
    '',
    '## 1. 주거지 구조와 편년',
    '',
    '가상유적의 3호 주거지는 장축 99.9m로 크게 확장되었다 ⟦F1⟧.',
    '김가짜(2024)는 이를 후기 양식으로 편년하였다 ⟦L1⟧.',
  ].join('\n');
  const v2Path = path.join(exp.handoffDir, 'manuscript_v2.md');
  fs.writeFileSync(v2Path, badV2, 'utf8');

  const imp = await importHandoff(jobDir, v2Path);
  assert.equal(imp.ok, false, '미등록 수치·저자가 추가된 2차 원고는 반입이 거부되어야 함');
  assert.equal(imp.gate.ok, false);
  assert.ok(imp.gate.blockReasons.some((r) => r.includes('V2(block)')), 'V2 수치 불일치 차단');
  assert.ok(imp.gate.blockReasons.some((r) => r.includes('V4(block)')), 'V4 저자 불일치 차단');
});

test('7단계 importHandoff: facts.json 해시 변조 시 G6 해시 불일치 거부', async () => {
  const { jobDir } = setupSampleJobDir();
  const exp = exportHandoff(jobDir);

  const validV2 = [
    '# Ⅳ. 고찰 — 가상유적 주거지 검토',
    '',
    '## 1. 주거지 구조와 편년',
    '',
    '가상유적의 3호 주거지는 장축 6.2m이다 ⟦F1⟧.',
  ].join('\n');
  const v2Path = path.join(exp.handoffDir, 'manuscript_v2.md');
  fs.writeFileSync(v2Path, validV2, 'utf8');

  // handoff_for_claude/facts.json 임의 변조
  fs.appendFileSync(path.join(exp.handoffDir, 'facts.json'), '\n// tampered\n', 'utf8');

  const imp = await importHandoff(jobDir, v2Path);
  assert.equal(imp.ok, false, 'facts.json이 변조되면 해시 불일치로 거부되어야 함');
  assert.equal(imp.cardHashMismatch, true);
  assert.equal(imp.gate.cardHashMismatch, true);
});

test('7단계 importHandoff: 유효한 2차 원고 반입 시 변경 문장 X1/X2 재검증 및 최종 manuscript.md 생성', async () => {
  const { jobDir } = setupSampleJobDir();
  const exp = exportHandoff(jobDir);

  // 기존 F2(단축 4.8m)를 새로 인용하여 다듬은 적법한 2차 원고
  const goodV2 = [
    '# Ⅳ. 고찰 — 가상유적 주거지 검토',
    '',
    '## 1. 주거지 구조와 편년',
    '',
    '가상유적의 3호 주거지는 장축 6.2m, 단축 4.8m의 평면 凸자형 구조를 갖춘다 ⟦F1⟧ ⟦F2⟧.',
    '홍길동(2015)에 의하면 중부지역 원삼국시대 凸자형 주거지는 중기 단계로 편년된다 ⟦L1⟧.',
  ].join('\n');
  const v2Path = path.join(exp.handoffDir, 'manuscript_v2.md');
  fs.writeFileSync(v2Path, goodV2, 'utf8');

  const { geminiDriver } = createFakeDrivers({
    defaultGeminiHandler: async (prompt) => {
      const allMarkers = [...prompt.matchAll(/<<END:([^>]+)>>/g)];
      const stepId = allMarkers.length > 0 ? allMarkers[allMarkers.length - 1][1] : 'STEP';
      if (stepId.includes('.X1')) {
        return (
          '```json\n' +
          JSON.stringify([
            {
              claimId: 'V2.r0.K1',
              verdict: '일치',
              evidenceQuote: '3호 주거지는 장축 6.2m, 단축 4.8m의 평면 凸자형 주거지이다.',
              page: 4,
            },
          ]) +
          `\n\`\`\`\n<<END:${stepId}>>`
        );
      }
      return (
        '```json\n' +
        JSON.stringify([
          {
            claimId: 'V2.r0.K2',
            verdict: '지지',
            evidenceQuote: '3호 주거지와 같은 凸자형 주거지는 원삼국시대 중기 단계로 편년된다.',
          },
        ]) +
        `\n\`\`\`\n<<END:${stepId}>>`
      );
    },
  });

  const imp = await importHandoff(jobDir, v2Path, { geminiDriver });
  assert.equal(imp.ok, true);
  assert.equal(imp.gate.ok, true);
  assert.ok(imp.triggeredReverifyClaims.length >= 1, '변경된 문장에 대해 선택적 X1/X2 재검증이 실행되어야 함');
  assert.ok(!imp.manuscriptMd.includes('⟦'), '최종 manuscript.md에는 내부 표식이 제거되어야 함');
  assert.ok(imp.manuscriptMd.includes('1)'), '문헌 인용은 각주 번호로 치환되어야 함');
  assert.ok(imp.manuscriptMd.includes('【각주】'));
  assert.ok(imp.manuscriptMd.includes('【참고문헌】'));
});
