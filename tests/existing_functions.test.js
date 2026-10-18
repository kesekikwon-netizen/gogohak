"use strict";

const test = require("node:test");
const assert = require("node:assert/strict");
const fs = require("node:fs");
const path = require("node:path");

const {
  prepareMarkdownAndFootnotesForWordCraft,
  evaluateArchaeologyManuscriptQuality,
  extractStructuredArchaeologyReportFacts,
} = require("../scripts/gemini_mcp_server.js");

const {
  repairCollapsedMarkdownTables,
  normalizeWcDiagramSpec,
  parseFeatureDimensionsFromMarkdown,
  replaceInlineFootnoteSafely,
} = require("../scripts/archaeology_visuals_enricher.js");

const VIRTUAL_REPORT_PATH = path.join(__dirname, "fixtures", "virtual_report.txt");

test("1. prepareMarkdownAndFootnotesForWordCraft: 괄호 연도 보존, 표 안 토큰 비파괴, 각주·참고문헌 1:1 동기화", () => {
  const sampleMd = [
    "### Ⅳ. 고찰",
    "",
    "가상시 가상동 유적의 1호 주거지는 홍길동(2010)이 제시한 중부지역 원삼국시대 주거지 유형과 일치한다 1).",
    "또한 김철수(2018)의 부뚜막 형식 분류에 따르면 터널식 구조에 해당한다 2).",
    "",
    "| 유구명 | 평면형태 | 관련 선행연구 |",
    "|---|---|---|",
    "| 1호 주거지 | 凸자형 | 홍길동(2010) 1) 및 김철수(2018) 2) |",
    "",
    "### 【각주】",
    "1) 홍길동, 2010, 「중부지역 원삼국시대 주거지 구조 연구」, 『가상고고학보』 12, 15~48쪽.",
    "2) 김철수, 2018, 「凸자형 주거지의 부뚜막 축조 기법 검토」, 『한국상고문화학보』 34, 101~132쪽.",
    "",
    "### 【참고문헌】",
    "- 홍길동, 2010, 「중부지역 원삼국시대 주거지 구조 연구」, 『가상고고학보』 12.",
  ].join("\n");

  const result = prepareMarkdownAndFootnotesForWordCraft(sampleMd);

  assert.equal(typeof result.wcMarkdown, "string");
  assert.ok(Array.isArray(result.footnotes), "footnotes 배열이 반환되어야 함");
  assert.equal(result.footnotes.length, 2, "2개의 각주가 생성되어야 함");
  assert.equal(result.footnotes[0].token, "[[WC_FN_1]]");
  assert.equal(result.footnotes[1].token, "[[WC_FN_2]]");

  // 본문 내 괄호 연도 '홍길동(2010)'과 '김철수(2018)'가 잘리지 않고 그대로 보존되는지 확인
  assert.ok(result.wcMarkdown.includes("홍길동(2010)"), "괄호 연도 홍길동(2010)가 보존되어야 함");
  assert.ok(result.wcMarkdown.includes("김철수(2018)"), "괄호 연도 김철수(2018)가 보존되어야 함");

  // 마크다운 표 안의 행이 각주 토큰 치환으로 깨지지 않고 유지되는지 확인
  const tableRow = result.wcMarkdown
    .split(/\r?\n/)
    .find((line) => line.includes("| 1호 주거지 |"));
  assert.ok(tableRow, "1호 주거지 표 행이 유지되어야 함");
  assert.ok(!tableRow.includes("[[WC_FN_"), "표 행 내에는 [[WC_FN_]] 토큰이 잘못 삽입되지 않아야 함");

  // 참고문헌에 누락되었던 2번 각주 문헌이 1:1 교차검증으로 자동 보충되었는지 확인
  assert.ok(
    result.wcMarkdown.includes("凸자형 주거지의 부뚜막 축조 기법 검토"),
    "참고문헌에 누락된 각주 문헌이 1:1 동기화로 추가되어야 함"
  );
});

test("2. repairCollapsedMarkdownTables: 한 줄로 병합된 마크다운 표 복원", () => {
  const collapsed = [
    "| 유구명 | 길이(m) | 너비(m) | 1호 주거지 | 5.8 | 4.2 | 3호 주거지 | 6.2 | 4.1 |",
    "|---|---|---|",
  ].join("\n");
  const repaired = repairCollapsedMarkdownTables(collapsed);
  const lines = repaired.trim().split(/\r?\n/);

  assert.equal(lines.length, 4, "헤더, 구분선, 데이터 2행 총 4줄로 분리되어야 함");
  assert.ok(lines[0].includes("| 유구명 | 길이(m) | 너비(m) |"), "첫 줄은 헤더여야 함");
  assert.ok(lines[1].includes("| --- | --- | --- |"), "둘째 줄은 구분선이어야 함");
  assert.ok(lines[2].includes("| 1호 주거지 | 5.8 | 4.2 |"), "셋째 줄은 1호 주거지 행이어야 함");
  assert.ok(lines[3].includes("| 3호 주거지 | 6.2 | 4.1 |"), "넷째 줄은 3호 주거지 행이어야 함");
});

test("3. normalizeWcDiagramSpec: WordCraft 호환 스키마로 모식도 정규화", () => {
  const rawSpec = {
    title: "가상유적 층위 및 주거지 중복 관계 모식도",
    nodes: [
      { label: "2호 수혈 (선행)", description: "타원형 저장공 굴광" },
      { label: "1호 주거지 (후행)", description: "凸자형 주거지 중복 축조" },
      { label: "3호 주거지 (확장)", description: "구들·부뚜막 설치" },
    ],
  };

  const normalized = normalizeWcDiagramSpec(rawSpec);
  assert.ok(normalized, "정규화 객체가 반환되어야 함");
  assert.equal(normalized.layout, "process");
  assert.equal(normalized.colors, "accent1");
  assert.equal(normalized.title, "가상유적 층위 및 주거지 중복 관계 모식도");
  assert.ok(Array.isArray(normalized.items) && normalized.items.length >= 3);
  assert.ok(normalized.items.some((it) => it.level === 0));
});

test("4. parseFeatureDimensionsFromMarkdown: 마크다운 표 및 산문에서 유구 치수 추출", () => {
  const fixtureText = fs.readFileSync(VIRTUAL_REPORT_PATH, "utf8");
  const dims = parseFeatureDimensionsFromMarkdown(fixtureText);

  assert.ok(Array.isArray(dims), "치수 배열이 반환되어야 함");
  assert.ok(dims.length >= 3, "최소 3기 이상의 유구 치수가 추출되어야 함");

  const h3 = dims.find((d) => d.name && d.name.replace(/\s+/g, "").includes("3호주거지"));
  assert.ok(h3, "3호 주거지가 추출되어야 함");
  assert.equal(Number(h3.lengthM), 6.2);
  assert.equal(Number(h3.widthM), 4.1);
});

test("5. replaceInlineFootnoteSafely: 표 내부 건너뛰기 및 일반 문단 각주 토큰 치환", () => {
  const body = [
    "| 1호 주거지 | 5.8m | 홍길동(2010) 1) |",
    "가상유적 1호 주거지는 홍길동(2010)의 분류와 일치한다. 1)",
  ].join("\n");

  const res = replaceInlineFootnoteSafely(body, 1, "[[WC_FN_1]]", "¹");
  assert.equal(res.replaced, true, "일반 문단의 각주 번호가 치환되어야 함");
  const lines = res.text.split(/\r?\n/);
  assert.ok(!lines[0].includes("[[WC_FN_1]]"), "마크다운 표 행 내부는 건드리지 않아야 함");
  assert.ok(lines[1].includes("[[WC_FN_1]]"), "일반 문단 끝에 [[WC_FN_1]] 토큰이 삽입되어야 함");
  assert.ok(lines[1].includes("홍길동(2010)"), "괄호 연도 (2010)는 보존되어야 함");
});

test("6. evaluateArchaeologyManuscriptQuality: 품질 지표 산출 및 미충족 항목(deficiencies) 판정", () => {
  const shortDraft = "### Ⅳ. 고찰\n\n가상유적의 1호 주거지는 장축 5.8m이다.";
  const evalShort = evaluateArchaeologyManuscriptQuality(shortDraft, { docType: "report" });
  assert.equal(evalShort.passed, false, "짧고 시각자료·각주가 없는 초안은 passed=false여야 함");
  assert.ok(
    Array.isArray(evalShort.deficiencies) && evalShort.deficiencies.length > 0,
    "deficiencies 배열에 미충족 사유가 담겨야 함"
  );
});

test("7. extractStructuredArchaeologyReportFacts: 가상 보고서 장 구분 및 키워드 추출 (단문 & 15,000자 초과 분기)", () => {
  const fixtureText = fs.readFileSync(VIRTUAL_REPORT_PATH, "utf8");

  // 7-1. 기본 픽스처 추출
  const shortResult = extractStructuredArchaeologyReportFacts(fixtureText);
  assert.ok(shortResult.siteName.includes("가상시 가상동"), `siteName 확인: ${shortResult.siteName}`);
  assert.equal(shortResult.regionName, "가상");
  assert.ok(
    shortResult.primaryPeriods.includes("원삼국") ||
      shortResult.primaryPeriods.includes("무문토기") ||
      shortResult.primaryPeriods.includes("청동기시대"),
    `핵심 시대 키워드가 추출되어야 함: ${JSON.stringify(shortResult.primaryPeriods)}`
  );
  assert.ok(
    shortResult.featureKeywords.includes("凸자형 주거지") ||
      shortResult.featureKeywords.includes("주거지"),
    "유구 키워드가 추출되어야 함"
  );
  assert.ok(
    shortResult.artifactKeywords.includes("경질무문토기") ||
      shortResult.artifactKeywords.includes("심발형토기"),
    "유물 키워드가 추출되어야 함"
  );

  // 7-2. 15,000자 초과 시 장별 스마트 발췌 블록([1. 발굴조사 개요], [3. Ⅲ. 조사내용], [4. Ⅳ. 조사결과], [5. 출토유물 목록]) 생성 확인
  const paddedReport =
    "발굴조사 보고서 표지 및 일러두기 해설 문단. ".repeat(60) +
    "\n" +
    fixtureText.replace(
      "Ⅲ. 조사내용",
      "주변 지형 및 고고학 배경 상세 보충 설명. ".repeat(50) + "\n\nⅢ. 조사내용"
    ) +
    "\n\n" +
    "3호 주거지 세부 토층 및 유물 출토 상태 추가 기술: 장축 6.2m, 단축 4.1m, 깊이 42cm. ".repeat(450);
  assert.ok(paddedReport.length > 15000, "패딩 후 길이가 15000자를 초과해야 함");

  const longResult = extractStructuredArchaeologyReportFacts(paddedReport);
  assert.ok(
    longResult.structuredText.includes("[1. 발굴조사 개요 및 표제 정보 (원문 추출)]"),
    "15,000자 초과 시 1번 섹션 헤더가 포함되어야 함"
  );
  assert.ok(
    longResult.structuredText.includes("[3. Ⅲ. 조사내용 전 유구·유물 상세 기술 (원문 100% 보존)]"),
    "15,000자 초과 시 3번 섹션 헤더가 포함되어야 함"
  );
});
