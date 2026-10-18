'use strict';

const test = require('node:test');
const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');

const { toLiteratureCards, mergeLiterature, evaluateGateG2 } = require('../scripts/v2/literature_cards');
const { buildQueriesFromFacts } = require('../scripts/v2/query_builder');
const { buildOutlinePrompt, parseOutline, evaluateGateG3 } = require('../scripts/v2/outline_planner');

const VIRTUAL_DBPIA = JSON.parse(
  fs.readFileSync(path.join(__dirname, 'fixtures', 'virtual_dbpia.json'), 'utf8')
);

test('1. 문헌 카드 변환·중복 합치기·verified 및 quoteStatus 판정', () => {
  const rawCards = toLiteratureCards(VIRTUAL_DBPIA, '원삼국시대 수혈주거지 편년');
  assert.equal(rawCards.length, 5);

  const merged = mergeLiterature(rawCards);
  // NODE09000001 두 건이 1건으로 병합되어 총 4건이어야 함
  assert.equal(merged.length, 4);

  const c1 = merged.find((c) => c.nodeId === 'NODE09000001');
  assert.ok(c1);
  assert.equal(c1.verified, true);
  assert.equal(c1.quotes.length, 3);
  assert.equal(c1.quoteStatus, 'has_quotes');

  const c3 = merged.find((c) => c.nodeId === 'NODE09000003');
  assert.ok(c3);
  assert.equal(c3.verified, true);
  assert.equal(c3.quotes.length, 0);
  assert.equal(c3.quoteStatus, 'metadata_only');

  const c4 = merged.find((c) => c.nodeId === 'NODE09000004');
  assert.ok(c4);
  assert.equal(c4.verified, false, '저자미상·연도 누락 카드는 verified:false여야 함');

  const g2Pass = evaluateGateG2(merged, 'paper');
  assert.equal(g2Pass.ok, true);
  assert.equal(g2Pass.verifiedCount, 3);

  const g2FailPaper = evaluateGateG2(merged.slice(0, 2), 'paper');
  assert.equal(g2FailPaper.ok, false);
  assert.equal(g2FailPaper.canContinue, false, '논문 모드는 G2 미달 시 반드시 멈춤');

  const g2FailReport = evaluateGateG2(merged.slice(0, 2), 'report');
  assert.equal(g2FailReport.ok, false);
  assert.equal(g2FailReport.canContinue, true, '고찰 모드는 G2 미달 시 경고 후 진행 가능');
});

test('2. 질의 생성기(query_builder.js)에 유적명 하드코딩이 없으며 입력 사실에서만 생성됨', () => {
  const sourceCode = fs.readFileSync(path.join(__dirname, '..', 'scripts', 'v2', 'query_builder.js'), 'utf8');
  assert.ok(!/병산동|기안동|삼양일동|옥천|경주|강릉/.test(sourceCode), 'query_builder.js에 특정 유적·지역명 하드코딩 금지');

  const factsA = [
    { id: 'F1', kind: 'feature', subject: '1호 주거지', attribute: '평면 형태', value: '방형', unit: '', quote: '방형 주거지' },
    { id: 'F2', kind: 'artifact', subject: '출토유물', attribute: '기종', value: '심발형토기', unit: '', quote: '심발형토기 출토' },
    { id: 'F3', kind: 'chronology', subject: '유적 편년', attribute: '시대', value: '원삼국시대', unit: '', quote: '원삼국시대로 편년' },
  ];
  const queriesA = buildQueriesFromFacts(factsA, 'report', '가상지역 방형 주거지');
  assert.ok(queriesA.length >= 2 && queriesA.length <= 8);
  assert.ok(queriesA.some((q) => q.includes('주거지') || q.includes('심발형토기')));
  assert.ok(!queriesA.some((q) => /병산동|기안동|삼양일동/.test(q)));
});

test('3. 목차 정리(없는 카드·근거 없는 주장·빈 절 제거) 및 게이트 G3 판정', () => {
  const validIds = new Set(['F1', 'F2', 'L1', 'L2']);
  const rawReply = [
    '```json',
    JSON.stringify({
      docType: 'report',
      title: 'Ⅳ. 고찰',
      approved: false,
      omitted: [],
      sections: [
        {
          id: 'S1',
          heading: '1. 머리말',
          claims: [
            { id: 'S1.C1', text: '유효한 주장입니다', evidence: ['F1', 'F999'] },
            { id: 'S1.C2', text: '존재하지 않는 카드만 가진 주장', evidence: ['F888'] },
            { id: 'S1.C3', text: '근거 배열이 빈 주장', evidence: [] },
          ],
          visuals: [
            { type: 'table', purpose: '유효 표', evidence: ['F1', 'F2'] },
            { type: 'chart', purpose: '없는 카드 차트', evidence: ['F999'] },
          ],
          targetChars: 1200,
        },
        {
          id: 'S2',
          heading: '2. 근거 없는 빈 절',
          claims: [
            { id: 'S2.C1', text: '없는 카드 주장', evidence: ['L999'] },
          ],
          visuals: [],
          targetChars: 1200,
        },
      ],
    }),
    '```',
    '<<END:P3.OUTLINE>>',
  ].join('\n');

  const { outline, logs } = parseOutline(rawReply, validIds, 'report');
  assert.equal(outline.sections.length, 1, '유효 주장이 없는 S2 절은 제거되어야 함');
  assert.equal(outline.sections[0].claims.length, 1, '유효 카드가 남은 S1.C1만 유지되어야 함');
  assert.deepEqual(outline.sections[0].claims[0].evidence, ['F1'], '존재하지 않는 F999는 제거되어야 함');
  assert.equal(outline.sections[0].visuals.length, 1, '유효 카드가 없는 시각자료는 제거되어야 함');
  assert.ok(outline.omitted.some((o) => o.heading.includes('근거 없는 빈 절')), '제거된 절은 omitted에 기록되어야 함');
  assert.ok(logs.length >= 3);

  const g3Wait = evaluateGateG3(outline, validIds, false);
  assert.equal(g3Wait.ok, false);
  assert.equal(g3Wait.needsApproval, true);

  const g3Auto = evaluateGateG3(outline, validIds, true);
  assert.equal(g3Auto.ok, true);
  assert.equal(g3Auto.needsApproval, false);

  const { prompt } = buildOutlinePrompt({
    docType: 'report',
    userFocus: '수혈주거지 구조',
    facts: [{ id: 'F1', kind: 'feature', subject: '1호 주거지', attribute: '장축', value: '6.2', unit: 'm', quote: '장축 6.2m' }],
    literature: [{ id: 'L1', authors: ['김고고'], year: '2019', title: '논문', journal: '학보', quotes: ['구절'], verified: true }],
    stepId: 'P3.OUTLINE',
  });
  assert.ok(!/@Google|Google Workspace|Gmail|Google Drive/i.test(prompt));
  assert.ok(prompt.includes('<<END:P3.OUTLINE>>'));
});
