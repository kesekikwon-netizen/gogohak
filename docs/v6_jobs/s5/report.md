# 5단계 · 문헌 카드·질의 생성·목차 계획 (P2 · P3 · G2 · G3) — ⑦ 단계 보고서 (12장 · 14.6 양식)

## 14.6 사람 검토 대상 변경 여부
- G2·G3 게이트 판정 로직 (14.6 검토 대상)

## 12장 보고 항목
- **브랜치**: `v6/s5-lit-outline`
- **커밋 SHA**: `690b8bd`
- **만든 파일**: scripts/v2/literature_cards.js, scripts/v2/query_builder.js, scripts/v2/prompts/outline.md, scripts/v2/outline_planner.js, tests/fixtures/virtual_dbpia.json, tests/v2_lit_outline.test.js
- **바꾼 파일**: 없음
- **검증 명령 및 종료 코드**: `node scripts/v2/tools/check.js` (`0`), `node --test tests/` (`0`)

### 완료 기준 대조표
| 완료 기준 | 판정 |
| :--- | :--- |
| 문헌 카드 변환·중복 병합·verified 및 quoteStatus 판정 | 통과 (tests/v2_lit_outline.test.js) |
| 질의 생성기 유적명 하드코딩 0건 및 사실 카드 기반 동적 생성 | 통과 (tests/v2_lit_outline.test.js) |
| 없는 카드·근거 없는 주장·빈 절 자동 제거 및 게이트 G2/G3 판정 | 통과 (tests/v2_lit_outline.test.js) |
