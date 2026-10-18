'use strict';

const test = require('node:test');
const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const os = require('node:os');

const {
  saveJobToSiteLibrary,
  loadSiteLibrary,
  querySiteLibrary,
} = require('../scripts/v2/site_library');
const { buildCrosswalk } = require('../scripts/v2/crosswalk');

const sampleFacts = [
  {
    id: 'F1',
    category: 'feature',
    subject: '1호 주거지',
    attribute: '평면 및 규모',
    value: '장축 5.4m, 단축 4.2m',
    unit: 'm',
    quote: '1호 주거지는 평면 말각장방형(凸자형)이며, 규모는 장축 5.4m, 단축 4.2m, 잔존 깊이 0.35m이다.',
    source: { file: 'virtual_report.txt', page: 3, chapter: 'Ⅲ. 조사내용' },
    confidence: 'verified',
  },
  {
    id: 'F2',
    category: 'feature',
    subject: '2호 주거지',
    attribute: '미검증 카드',
    value: '장축 99.9m',
    unit: 'm',
    quote: '원문에 없는 구절',
    source: { file: 'virtual_report.txt', page: null, chapter: 'Ⅲ. 조사내용' },
    confidence: 'unverified',
  },
];

const sampleLit = [
  {
    id: 'L1',
    authors: ['홍길동'],
    year: 2015,
    title: '중부지역 원삼국시대 주거지 편년 연구',
    journal: '가상고고학보',
    volume: '34',
    pages: '1~30',
    nodeId: 'NODE00000001',
    quotes: ['凸자형 주거지는 원삼국시대 중기 단계로 편년된다.'],
    quoteStatus: 'abstract',
    verified: true,
    sourceQuery: '중부지역 원삼국시대 주거지',
  },
];

test('9단계 saveJobToSiteLibrary: verified 카드만 저장하고 동일 유적 2회 적재 시 중복 방지', () => {
  const tmpLibDir = fs.mkdtempSync(path.join(os.tmpdir(), 'archaeo-v2-lib-'));

  const first = saveJobToSiteLibrary({
    siteKey: '가상유적 A지구',
    facts: sampleFacts,
    literature: sampleLit,
    baseDir: tmpLibDir,
  });

  assert.equal(first.factsCount, 1, 'verified 사실 카드 1건만 적재되어야 함');
  assert.equal(first.literatureCount, 1, 'verified 문헌 카드 1건만 적재되어야 함');

  const second = saveJobToSiteLibrary({
    siteKey: '가상유적 A지구',
    facts: sampleFacts,
    literature: sampleLit,
    baseDir: tmpLibDir,
  });

  assert.equal(second.factsCount, 1, '같은 유적을 두 번 적재해도 사실 카드가 중복되지 않아야 함');
  assert.equal(second.literatureCount, 1, '같은 유적을 두 번 적재해도 문헌 카드가 중복되지 않아야 함');
});

test('9단계 loadSiteLibrary & buildCrosswalk: 불러온 카드의 sourceTag가 대조표에 표시됨', () => {
  const tmpLibDir = fs.mkdtempSync(path.join(os.tmpdir(), 'archaeo-v2-lib-cw-'));

  saveJobToSiteLibrary({
    siteKey: '가상유적_A지구',
    facts: sampleFacts,
    literature: sampleLit,
    baseDir: tmpLibDir,
  });

  const queried = querySiteLibrary({
    baseDir: tmpLibDir,
    excludeSiteKey: '가상유적_B지구',
    category: 'feature',
  });

  assert.equal(queried.facts.length, 1);
  assert.equal(queried.facts[0].sourceTag, '라이브러리: 가상유적_A지구/F1');

  const libLoaded = loadSiteLibrary('가상유적_A지구', tmpLibDir);
  assert.equal(libLoaded.facts[0].sourceTag, '라이브러리: 가상유적_A지구/F1');
  assert.equal(libLoaded.literature[0].sourceTag, '라이브러리: 가상유적_A지구/L1');

  const rawMd = '비교 유적인 가상유적 A지구의 1호 주거지는 장축 5.4m, 단축 4.2m이다 ⟦F1⟧ ⟦L1⟧.';
  const matrices = [
    {
      sectionId: 'S1',
      round: 0,
      rows: [
        {
          claimId: 'S1.r0.K1',
          sentence: rawMd,
          final: 'pass',
          warnTag: false,
          reason: '통과',
        },
      ],
    },
  ];

  const cw = buildCrosswalk(rawMd, libLoaded.facts, libLoaded.literature, matrices);
  assert.ok(
    cw.markdown.includes('라이브러리: 가상유적_A지구/F1'),
    '대조표 마크다운에 라이브러리 사실 카드 sourceTag가 표시되어야 함'
  );
  assert.ok(
    cw.markdown.includes('라이브러리: 가상유적_A지구/L1'),
    '대조표 마크다운에 라이브러리 문헌 카드 sourceTag가 표시되어야 함'
  );
});
