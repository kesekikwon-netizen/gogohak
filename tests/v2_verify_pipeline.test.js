'use strict';

const test = require('node:test');
const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const os = require('node:os');

const { splitClaims } = require('../scripts/v2/verify/claim_splitter');
const { runDeterministic } = require('../scripts/v2/verify/deterministic');
const { checkFacts } = require('../scripts/v2/verify/fact_checker');
const { checkCitations } = require('../scripts/v2/verify/citation_checker');
const { decideClaimFinal, buildVerifyMatrix, attachNeedsHumanTags } = require('../scripts/v2/verify/consensus');
const { planVisuals } = require('../scripts/v2/visuals_from_facts');
const { assemble } = require('../scripts/v2/assembler');
const { buildCrosswalk } = require('../scripts/v2/crosswalk');
const { writeAndVerifySection, runPipelineV2 } = require('../scripts/v2/index');
const { createEvidenceStore } = require('../scripts/v2/evidence_store');
const { createFakeDrivers } = require('../scripts/v2/harness/fake_drivers');

const sampleFacts = [
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
  {
    id: 'F3',
    kind: 'feature',
    subject: '2호 주거지',
    attribute: '장축',
    value: '5.4',
    unit: 'm',
    quote: '2호 주거지는 장축 5.4m, 단축 4.1m의 장방형 주거지이다.',
    source: { file: '가상보고서.pdf', page: 3, chapter: 'Ⅲ' },
    confidence: 'verified',
  },
  {
    id: 'F4',
    kind: 'feature',
    subject: '1호 주거지',
    attribute: '장축',
    value: '4.9',
    unit: 'm',
    quote: '1호 주거지는 장축 4.9m이다.',
    source: { file: '가상보고서.pdf', page: 2, chapter: 'Ⅲ' },
    confidence: 'verified',
  },
  {
    id: 'F5',
    kind: 'dating',
    subject: '3호 주거지 목탄',
    attribute: '탄소연대',
    value: '1820±30',
    unit: 'BP',
    quote: '3호 주거지 목탄 시료의 탄소연대는 1820±30 BP(AD 130~250)로 측정되었다.',
    source: { file: '가상보고서.pdf', page: 5, chapter: 'Ⅳ' },
    confidence: 'verified',
  },
  {
    id: 'F6',
    kind: 'dating',
    subject: '2호 주거지 목탄',
    attribute: '탄소연대',
    value: '1890±35',
    unit: 'BP',
    quote: '2호 주거지 목탄 시료의 탄소연대는 1890±35 BP로 측정되었다.',
    source: { file: '가상보고서.pdf', page: 5, chapter: 'Ⅳ' },
    confidence: 'verified',
  },
];

const sampleLit = [
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

test('6단계 V1~V8 결정적 검사: 통과 사례와 각 위반 사례 검출', () => {
  const validMd = [
    '## 1. 유구 구조 분석',
    '',
    '가상유적의 3호 주거지는 장축 6.2m, 단축 4.8m의 평면 凸자형 주거지이다 ⟦F1⟧ ⟦F2⟧.',
    '홍길동(2015)에 따르면 중부지역 원삼국시대 凸자형 주거지는 중기 단계로 편년된다 ⟦L1⟧.',
  ].join('\n');

  const section = {
    id: 'S1',
    heading: '1. 유구 구조 분석',
    claims: [{ id: 'S1.C1', text: '3호 주거지 규모와 편년', evidence: ['F1', 'F2', 'L1'] }],
    targetChars: 80,
  };

  const validClaims = splitClaims(validMd, 'S1', 0);
  const passRes = runDeterministic({
    markdown: validMd,
    claims: validClaims,
    facts: sampleFacts,
    literature: sampleLit,
    section,
  });
  assert.equal(passRes.ok, true, '유효한 원고는 V1~V8 통과해야 함');
  assert.equal(passRes.hasBlock, false);

  // V1(측정 수치 문장에 F 표식 없음), V2(9.9m 카드 값과 불일치), V3(존재하지 않는 카드 F99), V4(김철수 2020 저자 불일치), V5(unverified 카드), V6(목차 근거 카드 미사용), V7(Google 키워드), V8(표 내 근거 없는 수치)
  const factsWithUnverified = [
    ...sampleFacts,
    {
      id: 'F7',
      kind: 'feature',
      subject: '4호 주거지',
      attribute: '구조',
      value: '방형',
      unit: '',
      quote: '4호 주거지는 방형이다.',
      source: { file: '가상보고서.pdf', page: 6, chapter: 'Ⅲ' },
      confidence: 'unverified',
    },
  ];
  const badMd = [
    '## 1. 유구 구조 분석',
    '',
    '5호 주거지는 장축 7.1m로 조사되었다.',
    '3호 주거지는 장축 9.9m로 확인되었다 ⟦F1⟧.',
    '존재하지 않는 카드를 인용한 문장이다 ⟦F99⟧.',
    '김철수(2020)는 이를 후기 양식으로 보았다 ⟦L1⟧.',
    '4호 주거지는 방형으로 확인되었다 ⟦F7⟧.',
    '또한 @Google Workspace 검색을 참조하였다 ⟦F1⟧.',
    '',
    '| 구분 | 수치 |',
    '| --- | --- |',
    '| 무근거 항목 | 12.5m |',
  ].join('\n');

  const badClaims = splitClaims(badMd, 'S1', 0);
  const failRes = runDeterministic({
    markdown: badMd,
    claims: badClaims,
    facts: factsWithUnverified,
    literature: sampleLit,
    section: {
      ...section,
      claims: [{ id: 'S1.C1', text: '미사용 근거 확인', evidence: ['F1', 'F2', 'F3', 'F4', 'F5', 'F6'] }],
    },
  });

  assert.equal(failRes.ok, false, '위반 원고는 실패 판정이 나와야 함');
  assert.equal(failRes.hasBlock, true);
  assert.ok(failRes.claimResults.some((c) => c.deterministic.V1 === 'block'), 'V1 수치 문장 F 누락 검출');
  assert.ok(failRes.claimResults.some((c) => c.deterministic.V2 === 'block'), 'V2 수치 불일치 검출');
  assert.ok(failRes.claimResults.some((c) => c.deterministic.V3 === 'block'), 'V3 존재하지 않는 카드 검출');
  assert.ok(failRes.claimResults.some((c) => c.deterministic.V4 === 'block'), 'V4 저자 불일치 검출');
  assert.ok(failRes.claimResults.some((c) => c.deterministic.V5 === 'warn'), 'V5 unverified 카드 경고 검출');
  assert.equal(failRes.v6SectionStatus, 'warn', 'V6 목차 근거 카드 절반 이상 미사용 경고 검출');
  assert.ok(failRes.claimResults.some((c) => c.deterministic.V7 === 'block'), 'V7 금지 어구 검출');
  assert.equal(failRes.v8SectionStatus, 'block', 'V8 표 내 무근거 수치 검출');
});

test('6단계 7.3 검증인의 인용 구절 재검증: 카드에 없는 구절을 지어내면 검증불가로 강제 전환', async () => {
  const md = '가상유적의 3호 주거지는 장축 6.2m이다 ⟦F1⟧.\n\n중부지역 凸자형 주거지는 중기 단계로 편년된다 ⟦L1⟧.';
  const claims = splitClaims(md, 'S1', 0);

  const { geminiDriver } = createFakeDrivers({
    geminiReplies: [
      // X1 응답: evidenceQuote를 원문에 없는 허구 문장으로 반환
      JSON.stringify([
        {
          claimId: claims[0].id,
          verdict: '일치',
          evidenceQuote: '이 구절은 사실 카드 어디에도 없는 지어낸 문장입니다.',
          page: 4,
        },
      ]) + '\n<<END:P5.S1.r0.X1>>',
      // X2 응답: evidenceQuote를 문헌 카드에 없는 허구 문장으로 반환
      JSON.stringify([
        {
          claimId: claims[1].id,
          verdict: '지지',
          evidenceQuote: '문헌 카드에 전혀 존재하지 않는 허구 초록 문장입니다.',
        },
      ]) + '\n<<END:P5.S1.r0.X2>>',
    ],
  });

  const x1 = await checkFacts({
    claims,
    facts: sampleFacts,
    pages: [{ page: 4, text: sampleFacts[0].quote }],
    geminiDriver,
    sectionId: 'S1',
    round: 0,
  });
  assert.equal(x1.byClaimId[claims[0].id][0].verdict, '검증불가');
  assert.equal(x1.byClaimId[claims[0].id][0].rechecked, false);

  const x2 = await checkCitations({
    claims,
    literature: sampleLit,
    geminiDriver,
    sectionId: 'S1',
    round: 0,
  });
  assert.equal(x2.byClaimId[claims[1].id][0].verdict, '검증불가');
  assert.equal(x2.byClaimId[claims[1].id][0].rechecked, false);
});

test('6단계 7.5 합의 규칙 표의 모든 행 판정 검증', () => {
  // 1) 결정적 검사 block 있음 -> X1/X2 무엇이든 rework (소진 시 needs-human)
  assert.equal(
    decideClaimFinal({
      detStatus: 'block',
      x1List: [{ verdict: '일치' }],
      x2List: [{ verdict: '지지' }],
      round: 0,
      maxRework: 2,
    }).final,
    'rework'
  );
  // 2) 결정적 통과 + 일치/지지 -> pass
  assert.equal(
    decideClaimFinal({
      detStatus: 'pass',
      x1List: [{ verdict: '일치' }],
      x2List: [{ verdict: '지지' }],
      round: 0,
      maxRework: 2,
    }).final,
    'pass'
  );
  // 3) 결정적 통과 + 불일치/불지지 -> rework (maxRework 소진 시 needs-human)
  assert.equal(
    decideClaimFinal({
      detStatus: 'pass',
      x1List: [{ verdict: '불일치' }],
      x2List: [],
      round: 0,
      maxRework: 2,
    }).final,
    'rework'
  );
  assert.equal(
    decideClaimFinal({
      detStatus: 'pass',
      x1List: [],
      x2List: [{ verdict: '불지지' }],
      round: 2,
      maxRework: 2,
    }).final,
    'needs-human'
  );
  // 4) 결정적 통과 + 근거없음/구절없음/부분지지 -> round 0에서는 rework, round >= 1에서는 needs-human
  assert.equal(
    decideClaimFinal({
      detStatus: 'pass',
      x1List: [{ verdict: '근거없음' }],
      x2List: [],
      round: 0,
      maxRework: 2,
    }).final,
    'rework'
  );
  assert.equal(
    decideClaimFinal({
      detStatus: 'pass',
      x1List: [],
      x2List: [{ verdict: '부분지지' }],
      round: 1,
      maxRework: 2,
    }).final,
    'needs-human'
  );
  // 5) 결정적 통과 + 검증불가 -> 즉시 needs-human (자동 통과 금지)
  assert.equal(
    decideClaimFinal({
      detStatus: 'pass',
      x1List: [{ verdict: '검증불가' }],
      x2List: [],
      round: 0,
      maxRework: 2,
    }).final,
    'needs-human'
  );
  // 6) warn만 있음 + 일치/지지 -> pass + warnTag: true
  const warnDec = decideClaimFinal({
    detStatus: 'warn',
    detReasons: ['V5(warn): 미확인 사실 카드 사용'],
    x1List: [{ verdict: '일치' }],
    x2List: [],
    round: 0,
    maxRework: 2,
  });
  assert.equal(warnDec.final, 'pass');
  assert.equal(warnDec.warnTag, true);
});

test('6단계 재작성 2회 소진 시 〔확인 필요〕 표식 부착 검증', async () => {
  const tmpDir = fs.mkdtempSync(path.join(os.tmpdir(), 'archaeo-v2-rework-'));
  const store = createEvidenceStore(tmpDir);
  const section = {
    id: 'S1',
    heading: '1. 유구 구조',
    claims: [{ id: 'S1.C1', text: '3호 주거지 규모', evidence: ['F1'] }],
    targetChars: 80,
  };

  // r0, r1, r2 모두 카드에 없는 수치(9.9m -> V2 block 실패)를 반환하도록 설정
  const { geminiDriver } = createFakeDrivers({
    defaultGeminiHandler: async (prompt) => {
      const m = prompt.match(/<<END:([^>]+)>>/);
      const marker = m ? `<<END:${m[1]}>>` : '<<END:STEP>>';
      if (prompt.includes('P4.S1')) {
        return `## 1. 유구 구조\n\n가상유적 3호 주거지는 장축 9.9m로 조사되었다 ⟦F1⟧.\n\n[절 요약]\n- 3호 주거지 검토\n${marker}`;
      }
      if (prompt.includes('.X1') || prompt.includes('.X2')) {
        return `\`\`\`json\n[]\n\`\`\`\n${marker}`;
      }
      return `\`\`\`json\n{"scores":{"evidence":3,"coverage":3,"logic":3,"tone":3},"issues":[],"verdict":"accept"}\n\`\`\`\n${marker}`;
    },
  });

  const res = await writeAndVerifySection({
    jobDir: tmpDir,
    store,
    section,
    facts: sampleFacts,
    literature: sampleLit,
    pages: [{ page: 4, text: sampleFacts[0].quote }],
    docType: 'report',
    prevSummary: '',
    geminiDriver,
  });

  assert.equal(res.roundsUsed, 3, '초안(r0) + 재작성 2회(r1, r2) 총 3회 실행');
  assert.ok(res.markdown.includes('〔확인 필요:'), '2회 재작성 후에도 남은 실패 문장에 〔확인 필요〕 표식이 붙어야 함');
});

test('6단계 Nano Banana Pro 시각자료 생성 및 runPipelineV2 전체 E2E 완주', async () => {
  const tmpDir = fs.mkdtempSync(path.join(os.tmpdir(), 'archaeo-v2-e2e-'));

  // 조건부 시각자료 단독 확인
  const vis = planVisuals(sampleFacts, sampleLit, null);
  assert.ok(vis.specs.length >= 2, '측정치 3건 이상 + 절대연대 2건 이상이므로 표·차트·연대분포도 등이 생성되어야 함');
  const svgSpec = vis.specs.find((s) => Boolean(s.svg));
  assert.ok(svgSpec && svgSpec.svg.includes('NANO BANANA ARCHAEOLOGY PRO'), 'Nano Banana Pro 고고학 테마 SVG 헤더 포함');

  // E2E 파이프라인 가짜 드라이버 핸들러
  const { geminiDriver, dbpiaDriver, craftDriver, calls } = createFakeDrivers({
    defaultGeminiHandler: async (prompt) => {
      const allMarkers = [...prompt.matchAll(/<<END:([^>]+)>>/g)];
      const stepId = allMarkers.length > 0 ? allMarkers[allMarkers.length - 1][1] : 'STEP';
      const marker = `<<END:${stepId}>>`;

      if (stepId.startsWith('P1.CH')) {
        return (
          '```json\n' +
          JSON.stringify([
            {
              id: 'F1',
              kind: 'feature',
              subject: '3호 주거지',
              attribute: '장축',
              value: '6.2',
              unit: 'm',
              quote: '3호 주거지는 장축 6.2m, 단축 4.8m의 평면 凸자형 주거지이다.',
              source: { file: '가상보고서.pdf', page: 1, chapter: 'Ⅲ' },
            },
            {
              id: 'F2',
              kind: 'feature',
              subject: '3호 주거지',
              attribute: '단축',
              value: '4.8',
              unit: 'm',
              quote: '3호 주거지는 장축 6.2m, 단축 4.8m의 평면 凸자형 주거지이다.',
              source: { file: '가상보고서.pdf', page: 1, chapter: 'Ⅲ' },
            },
          ]) +
          '\n```\n' +
          marker
        );
      }

      if (stepId === 'P3.OUTLINE') {
        return (
          '```json\n' +
          JSON.stringify({
            docType: 'report',
            title: 'Ⅳ. 고찰 — 가상유적 주거지 검토',
            approved: true,
            omitted: [],
            sections: [
              {
                id: 'S1',
                heading: '1. 주거지 구조와 편년',
                claims: [
                  {
                    id: 'S1.C1',
                    text: '3호 주거지 평면 규모와 원삼국시대 편년 검토',
                    evidence: ['F1', 'F2', 'L1'],
                  },
                ],
                visuals: [],
                targetChars: 80,
              },
            ],
          }) +
          '\n```\n' +
          marker
        );
      }

      if (stepId.startsWith('P4.S')) {
        return (
          '## 1. 주거지 구조와 편년\n\n' +
          '가상유적의 3호 주거지는 장축 6.2m, 단축 4.8m의 평면 凸자형 주거지이다 ⟦F1⟧ ⟦F2⟧.\n\n' +
          '중부지역 원삼국시대 凸자형 주거지는 원삼국시대 중기 단계로 편년된다 ⟦L1⟧.\n\n' +
          '[절 요약]\n- 3호 주거지 구조와 편년을 검토하였다.\n' +
          marker
        );
      }

      if (stepId.includes('.X1')) {
        return (
          '```json\n' +
          JSON.stringify([
            {
              claimId: 'S1.r0.C1',
              verdict: '일치',
              evidenceQuote: '3호 주거지는 장축 6.2m, 단축 4.8m의 평면 凸자형 주거지이다.',
              page: 1,
            },
          ]) +
          '\n```\n' +
          marker
        );
      }

      if (stepId.includes('.X2')) {
        return (
          '```json\n' +
          JSON.stringify([
            {
              claimId: 'S1.r0.C2',
              verdict: '지지',
              evidenceQuote: '3호 주거지와 같은 凸자형 주거지는 원삼국시대 중기 단계로 편년된다.',
            },
          ]) +
          '\n```\n' +
          marker
        );
      }

      if (stepId.includes('.X3')) {
        return (
          '```json\n' +
          JSON.stringify({
            scores: { evidence: 5, coverage: 5, logic: 5, tone: 5 },
            issues: [],
            verdict: 'accept',
          }) +
          '\n```\n' +
          marker
        );
      }

      return `\`\`\`json\n{}\n\`\`\`\n${marker}`;
    },
  });

  const result = await runPipelineV2({
    jobId: 'test_job_v2_e2e',
    baseOutputDir: tmpDir,
    docType: 'report',
    userFocus: '가상유적 원삼국시대 주거지',
    reportText: '[[PAGE 1]]\nⅢ. 조사내용\n3호 주거지는 장축 6.2m, 단축 4.8m의 평면 凸자형 주거지이다.',
    autoApproveOutline: true,
    enableDbpiaReview: false,
    drivers: { geminiDriver, dbpiaDriver, craftDriver },
  });

  assert.equal(result.ok, true, `파이프라인 완료 기대 (실제: ${JSON.stringify(result.error || result.gate)})`);
  assert.equal(result.phase, 'V2_DONE');
  assert.ok(!result.manuscriptMd.includes('⟦'), '최종 manuscript.md에는 내부 근거 표식이 남지 않아야 함');
  assert.ok(result.manuscriptMd.includes('【각주】'), '최종 원고에 각주 장 포함');
  assert.ok(result.manuscriptMd.includes('【참고문헌】'), '최종 원고에 참고문헌 장 포함');
  assert.ok(fs.existsSync(path.join(result.jobDir, 'manuscript.md')));
  assert.ok(fs.existsSync(path.join(result.jobDir, 'manuscript_v1.md')));
  assert.ok(fs.existsSync(path.join(result.jobDir, 'crosswalk.md')));
  assert.ok(fs.existsSync(path.join(result.jobDir, 'needs_human.md')));
  assert.ok(calls.resetChatCount >= 4, '작성 대화와 검증 대화(X1, X2, X3)는 각각 새 대화로 분리되어야 함');
});

