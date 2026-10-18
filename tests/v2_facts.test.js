"use strict";

const test = require("node:test");
const assert = require("node:assert/strict");
const path = require("node:path");

const { extractPages } = require("../scripts/v2/page_extractor.js");
const {
  splitChapters,
  buildFactPrompt,
  parseFactReply,
  verifyFactAgainstPages,
  evaluateGateG1,
} = require("../scripts/v2/fact_extractor.js");

const VIRTUAL_REPORT_PATH = path.join(__dirname, "fixtures", "virtual_report.txt");

test("1. page_extractor.js & fact_extractor.js: 가상 보고서 쪽 단위 추출 및 로마 숫자 장 구분", async () => {
  const pages = await extractPages(VIRTUAL_REPORT_PATH);
  assert.equal(pages.length, 6, "[[PAGE 1]] ~ [[PAGE 6]] 총 6쪽이 추출되어야 함");
  assert.equal(pages[0].page, 1);
  assert.equal(pages[5].page, 6);

  const chapters = splitChapters(pages);
  const chapterNames = chapters.map((c) => c.chapter);
  assert.deepEqual(
    chapterNames,
    ["Ⅰ", "Ⅱ", "Ⅲ", "Ⅳ", "목록"],
    "Ⅰ, Ⅱ, Ⅲ, Ⅳ, 목록 5개 장으로 분리되어야 함"
  );

  const prompt = buildFactPrompt(chapters[2], "virtual_report.txt");
  assert.ok(prompt.includes("[[PAGE 3]]"));
  assert.ok(prompt.includes("[[PAGE 4]]"));
  assert.ok(!prompt.includes("Google"), "프롬프트에 금지 키워드가 없어야 함");
});

test("2. verifyFactAgainstPages: 원문 구절 대조 및 수치 일치 결정적 검증 (verified / unverified 분리)", async () => {
  const pages = await extractPages(VIRTUAL_REPORT_PATH);

  const rawReply = {
    cards: [
      {
        kind: "feature",
        subject: "3호 주거지",
        attribute: "장축 길이",
        value: "6.20", // 원문은 6.2m (6.2 == 6.20 수치 동등성 허용)
        unit: "m",
        quote: "주거부 규모는 장축 6.2m, 단축 4.1m(6.2×4.1m), 잔존깊이 42cm, 면적 25.4㎡이다.",
        page: 4,
      },
      {
        kind: "feature",
        subject: "3호 주거지",
        attribute: "장축 길이(날조)",
        value: "9.8", // quote에 없는 허구 수치 -> 반드시 unverified
        unit: "m",
        quote: "주거부 규모는 장축 6.2m, 단축 4.1m(6.2×4.1m), 잔존깊이 42cm, 면적 25.4㎡이다.",
        page: 4,
      },
      {
        kind: "dating",
        subject: "VRT-01",
        attribute: "측정연대",
        value: "2010±30",
        unit: "BP",
        quote: "이 문장은 원문 보고서 어느 쪽에도 존재하지 않는 지어낸 구절이다 2010±30 BP.",
        page: 5,
      },
    ],
  };

  const parsed = parseFactReply(rawReply, "virtual_report.txt", "Ⅲ", 1);
  assert.equal(parsed.length, 3);

  const verifiedList = parsed.map((c) => verifyFactAgainstPages(c, pages));
  assert.equal(verifiedList[0].confidence, "verified", "원문 구절 일치 및 6.2==6.20 동치 -> verified");
  assert.equal(verifiedList[1].confidence, "unverified", "원문에 없는 수치(9.8) -> 반드시 unverified");
  assert.equal(verifiedList[2].confidence, "unverified", "원문에 없는 quote -> 반드시 unverified");

  // 게이트 G1 판정 (1/3 = 0.333 < 0.6 -> 미통과, 2/3 = 0.667 >= 0.6 -> 통과)
  const g1Fail = evaluateGateG1(verifiedList, []);
  assert.equal(g1Fail.ok, false);

  const g1Pass = evaluateGateG1([verifiedList[0], { ...verifiedList[0], id: "F2" }, verifiedList[1]], []);
  assert.equal(g1Pass.ok, true);
});
