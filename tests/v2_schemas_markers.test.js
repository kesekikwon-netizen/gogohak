"use strict";

const test = require("node:test");
const assert = require("node:assert/strict");

const {
  validateFactCard,
  validateLiteratureCard,
  validateOutline,
  validateClaim,
  validateVerifyMatrix,
} = require("../scripts/v2/schemas.js");

const {
  parseMarkers,
  stripFactMarkers,
  stripAllMarkers,
  markersToFootnotes,
} = require("../scripts/v2/markers.js");

test("1. schemas.js: 6장 모든 형식(FactCard, LiteratureCard, Outline, Claim, VerifyMatrix)의 올바른 예·틀린 예 검증", () => {
  // 6.1 FactCard
  const validFact = {
    id: "F12",
    kind: "feature",
    subject: "3호 주거지",
    attribute: "장축 길이",
    value: "6.2",
    unit: "m",
    quote: "3호 주거지는 장축 6.2m, 단축 4.1m로",
    source: { file: "가상보고서.pdf", page: 57, chapter: "Ⅲ" },
    confidence: "verified",
    note: "",
  };
  assert.equal(validateFactCard(validFact).ok, true);

  const invalidFact = { ...validFact, id: "X12", kind: "unknown", confidence: "maybe" };
  const badFactRes = validateFactCard(invalidFact);
  assert.equal(badFactRes.ok, false);
  assert.ok(badFactRes.errors.length >= 3);

  // 6.2 LiteratureCard
  const validLit = {
    id: "L3",
    nodeId: "NODE01234567",
    authors: ["홍길동"],
    year: "2015",
    title: "중부지역 원삼국시대 주거지 연구",
    journal: "가상고고학보",
    volume: "10",
    issue: "1",
    pages: "1~30",
    quotes: ["凸자형 주거지는 원삼국시대에 등장한다."],
    summary: "중부지역 주거지 구조 변천 검토",
    query: "중부지역 凸자형 주거지 편년",
    verified: true,
  };
  assert.equal(validateLiteratureCard(validLit).ok, true);
  assert.equal(validateLiteratureCard({ ...validLit, id: "F3", verified: "yes" }).ok, false);

  // 6.3 Outline
  const validOutline = {
    docType: "report",
    title: "Ⅳ. 고찰",
    approved: false,
    omitted: [{ heading: "연대 분석", reason: "연대 자료 없음" }],
    sections: [
      {
        id: "S2",
        heading: "1. 주거지의 구조와 축조 방식",
        claims: [{ id: "S2.C1", text: "凸자형 평면이 주류를 이룬다", evidence: ["F12", "L3"] }],
        visuals: [{ type: "table", purpose: "유구 제원 일람", evidence: ["F12"] }],
        targetChars: 1200,
      },
    ],
  };
  assert.equal(validateOutline(validOutline).ok, true);
  assert.equal(
    validateOutline({ ...validOutline, sections: [{ ...validOutline.sections[0], claims: [] }] }).ok,
    false
  );

  // 6.5 Claim
  const validClaim = {
    id: "S2.r0.K4",
    sentence: "3호 주거지는 장축 6.2m이다⟦F12⟧.",
    atoms: ["3호 주거지 장축 6.2m"],
    markers: ["F12"],
    kind: "fact",
  };
  assert.equal(validateClaim(validClaim).ok, true);
  assert.equal(validateClaim({ ...validClaim, kind: "invalid" }).ok, false);

  // 6.6 / 7.6 VerifyMatrix
  const validMatrix = {
    sectionId: "S2",
    round: 1,
    rows: [
      {
        claimId: "S2.r1.K4",
        sentence: "3호 주거지는 장축 6.2m이다.",
        deterministic: { V1: "pass", V2: "pass", V3: "pass" },
        x1: [{ atom: "3호 주거지 장축 6.2m", verdict: "일치", evidenceQuote: "장축 6.2m", page: 57, rechecked: true }],
        x2: [],
        double: null,
        final: "pass",
        reason: "",
      },
    ],
    critic: { verdict: "accept", issues: [] },
    summary: { pass: 1, rework: 0, needsHuman: 0 },
  };
  assert.equal(validateVerifyMatrix(validMatrix).ok, true);
  assert.equal(
    validateVerifyMatrix({
      ...validMatrix,
      rows: [{ ...validMatrix.rows[0], final: "unknown" }],
    }).ok,
    false
  );
});

test("2. markers.js: ⟦⟧·[[ ]] 파싱, F 제거 후 문장부호 정상, L → 각주 번호 등장 순 1부터 및 같은 L 재등장 시 동일 번호", () => {
  const sample =
    "3호 주거지는 장축 6.2m이다⟦F12, L3⟧. 또한 1호 주거지는 장축 5.8m로 확인된다[[F1, L1]]. 이는 중부지역 전형 양상이다⟦L3⟧.";

  const parsed = parseMarkers(sample);
  assert.deepEqual(parsed.factIds, ["F12", "F1"]);
  assert.deepEqual(parsed.litIds, ["L3", "L1"]);

  const fStripped = stripFactMarkers(sample);
  assert.ok(!fStripped.includes("F12") && !fStripped.includes("F1"));
  assert.ok(fStripped.includes("⟦L3⟧."));
  assert.ok(fStripped.includes("⟦L1⟧."));

  const allStripped = stripAllMarkers(sample);
  assert.equal(
    allStripped,
    "3호 주거지는 장축 6.2m이다. 또한 1호 주거지는 장축 5.8m로 확인된다. 이는 중부지역 전형 양상이다."
  );

  const litCards = [
    {
      id: "L1",
      authors: ["김철수"],
      year: "2018",
      title: "부뚜막 축조 기법 연구",
      journal: "한국상고문화학보",
      volume: "34",
      pages: "101~132",
      verified: true,
    },
    {
      id: "L3",
      authors: ["홍길동"],
      year: "2015",
      title: "중부지역 원삼국시대 주거지 편년",
      journal: "가상고고학보",
      volume: "10",
      pages: "1~30",
      verified: true,
    },
  ];

  const fnConverted = markersToFootnotes(sample, litCards);
  // L3가 먼저 등장했으므로 1), L1이 두 번째로 등장했으므로 2), 마지막 문장의 L3 재등장은 1)이어야 함
  assert.equal(fnConverted.footnotes.length, 2, "고유 문헌 카드 2개만 각주 목록에 생성되어야 함");
  assert.equal(fnConverted.footnotes[0].num, 1);
  assert.equal(fnConverted.footnotes[0].litId, "L3");
  assert.equal(fnConverted.footnotes[1].num, 2);
  assert.equal(fnConverted.footnotes[1].litId, "L1");

  assert.ok(
    fnConverted.markdown.includes("장축 6.2m이다 1)."),
    `첫 등장 L3 -> 1) 확인: ${fnConverted.markdown}`
  );
  assert.ok(
    fnConverted.markdown.includes("확인된다 2)."),
    `둘째 등장 L1 -> 2) 확인: ${fnConverted.markdown}`
  );
  assert.ok(
    fnConverted.markdown.includes("전형 양상이다 1)."),
    `재등장 L3 -> 동일 번호 1) 확인: ${fnConverted.markdown}`
  );

  // 참고문헌은 가나다순(김철수 -> 홍길동) 정렬 확인
  assert.equal(fnConverted.bibliography.length, 2);
  assert.ok(fnConverted.bibliography[0].text.startsWith("김철수"));
  assert.ok(fnConverted.bibliography[1].text.startsWith("홍길동"));
});
