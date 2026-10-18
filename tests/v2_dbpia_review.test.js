'use strict';

const test = require('node:test');
const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const os = require('node:os');

const {
  selectReviewClaims,
  buildReviewQuery,
  runDbpiaFinalReview,
  evaluateGateG7,
  handleDbpiaReviewAction,
} = require('../scripts/v2/verify/dbpia_review');
const { createFakeDrivers } = require('../scripts/v2/harness/fake_drivers');

const sampleFacts = [
  {
    id: 'F1',
    kind: 'site',
    subject: '가상유적',
    attribute: '유적명',
    value: '가상유적',
    unit: '',
    quote: '이번 조사는 가상유적에 대한 발굴조사이다.',
    source: { file: '가상보고서.pdf', page: 1, chapter: 'Ⅰ' },
    confidence: 'verified',
  },
  {
    id: 'F2',
    kind: 'feature',
    subject: '3호 주거지',
    attribute: '평면형태',
    value: '凸자형',
    unit: '',
    quote: '3호 주거지는 장축 6.2m의 평면 凸자형 주거지이다.',
    source: { file: '가상보고서.pdf', page: 4, chapter: 'Ⅲ' },
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

test('7-1단계 selectReviewClaims 및 buildReviewQuery: 최대 5개 선별 및 유적 고유명 제거 확인', () => {
  const outline = {
    sections: [
      { id: 'S1', heading: '1. 평면 구조' },
      { id: 'S2', heading: '2. 출토유물과 편년' },
    ],
  };
  const manuscriptMd = [
    '## 1. 평면 구조',
    '가상유적 3호 주거지의 평면 凸자형 구조는 중부지역 원삼국시대 취락의 전형적인 양식이다 ⟦F2⟧ ⟦L1⟧.',
    '## 2. 출토유물과 편년',
    '가상유적에서 출토된 경질무문토기와 타날문 단경호는 기원후 1세기 전엽으로 편년된다 ⟦L1⟧.',
  ].join('\n');

  const claims = selectReviewClaims(outline, manuscriptMd, sampleFacts);
  assert.ok(claims.length >= 2 && claims.length <= 5, '핵심 주장 2~5개 선별');

  const query = buildReviewQuery(claims[0], sampleFacts);
  assert.ok(!query.includes('가상유적'), '질의문에서 당해 유적 고유명(가상유적)이 제거되어야 함');
  assert.ok(!query.includes('3호'), '질의문에서 개별 유구 번호(3호)가 제거되어야 함');
  assert.ok(query.includes('凸자형') || query.includes('원삼국시대'), '시대·유구 형태 학술어는 유지되어야 함');
});

test('7-1단계 runDbpiaFinalReview & evaluateGateG7: 상충 검출 시 G7 중단, 신규 문헌(L_new) 식별, 원고 자동 수정 금지', async () => {
  const tmpDir = fs.mkdtempSync(path.join(os.tmpdir(), 'archaeo-v2-x4-'));
  const originalMs = [
    '# Ⅳ. 고찰',
    '',
    '## 1. 출토유물과 편년',
    '',
    '가상유적의 凸자형 주거지와 타날문 단경호 공반 단계는 기원전 3세기로 편년된다 ⟦L1⟧.',
  ].join('\n');
  const msPath = path.join(tmpDir, 'manuscript.md');
  fs.writeFileSync(msPath, originalMs, 'utf8');

  const { dbpiaDriver, geminiDriver } = createFakeDrivers({
    dbpiaReplies: [
      {
        query: '凸자형 주거지 타날문 단경호 편년',
        answerText: '중부지역 凸자형 주거지와 타날문 단경호 공반 단계는 기원후 2~3세기(원삼국시대 중후기)로 편년하는 것이 학계 통설이다.',
        refs: [
          // 기존 L1과 동일한 문헌
          {
            nodeId: 'NODE00000001',
            author: '홍길동',
            year: '2015',
            title: '중부지역 원삼국시대 주거지 편년 연구',
            journal: '가상고고학보',
            pages: '1~30',
            abstract: '凸자형 주거지는 원삼국시대 중기 단계로 편년된다.',
          },
          // 기존 literature.json에 없던 새 문헌
          {
            nodeId: 'NODE99999999',
            author: '최신학',
            year: '2023',
            title: '중부권 원삼국시대 타날문토기 수용 단계 재검토',
            journal: '한국고고학가상학보',
            pages: '55~88',
            abstract: '타날문 단경호와 凸자형 주거지의 공반은 기원후 2세기 중엽 이후에 해당한다.',
          },
        ],
      },
    ],
    defaultGeminiHandler: async (prompt) => {
      const allMarkers = [...prompt.matchAll(/<<END:([^>]+)>>/g)];
      const stepId = allMarkers.length > 0 ? allMarkers[allMarkers.length - 1][1] : 'STEP';
      return (
        '```json\n' +
        JSON.stringify({
          claimId: 'REV.r0.K1',
          sectionId: 'S1',
          verdict: '상충',
          keyPoints: '학계 통설은 기원후 2~3세기로 보나 원고는 기원전 3세기로 기술함',
          conflictDetail: '연대 비정이 학계 통설(기원후 2~3세기)과 400년 이상 차이남',
          suggestion: '최신학(2023) 등을 참조하여 기원후 2~3세기로 수정 필요',
        }) +
        `\n\`\`\`\n<<END:${stepId}>>`
      );
    },
  });

  const reviewRes = await runDbpiaFinalReview({
    jobDir: tmpDir,
    outline: { sections: [{ id: 'S1', heading: '1. 출토유물과 편년' }] },
    manuscriptMd: originalMs,
    facts: sampleFacts,
    literature: sampleLit,
    dbpiaDriver,
    geminiDriver,
  });

  assert.equal(reviewRes.summary.conflict, 1, '상충 1건이 집계되어야 함');
  assert.equal(reviewRes.newLiterature.length, 1, '기존 L1을 제외한 신규 문헌 1건(NODE99999999)만 L_new1로 추출되어야 함');
  assert.equal(reviewRes.newLiterature[0].id, 'L_new1');
  assert.equal(reviewRes.newLiterature[0].nodeId, 'NODE99999999');

  const g7 = evaluateGateG7(reviewRes);
  assert.equal(g7.ok, false, '상충이 1건 이상이면 게이트 G7은 멈춰야 함');

  // 7.7.4 자동 고쳐쓰기 금지 검증: runDbpiaFinalReview 실행 직후 manuscript.md는 바이트 단위로 동일해야 함
  assert.equal(fs.readFileSync(msPath, 'utf8'), originalMs, '사용자 선택 전 원고 자동 수정 금지');

  // 사용자가 'tag'([확인 필요 표식만 붙이고 내보내기]) 선택 시 표식 부착 확인
  handleDbpiaReviewAction(tmpDir, 'tag');
  const taggedMs = fs.readFileSync(msPath, 'utf8');
  assert.ok(taggedMs.includes('〔확인 필요: DBpia 문헌 검수 상충'), 'tag 선택 시 상충 문장에 확인 필요 표식 부착');
});
