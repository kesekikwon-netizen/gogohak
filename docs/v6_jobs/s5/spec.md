# 5단계 · 문헌 카드·질의 생성·목차 계획 (P2 · P3 · G2 · G3) — ② 명세 (Spec)

## 1. 생성 파일
- `scripts/v2/literature_cards.js`
- `scripts/v2/query_builder.js`
- `scripts/v2/prompts/outline.md`
- `scripts/v2/outline_planner.js`
- `tests/fixtures/virtual_dbpia.json`
- `tests/v2_lit_outline.test.js`

## 2. 수정 파일
- 없음

## 3. 절대 수정하지 않는 보호 경로
- `bin/`, `patches/`, `CLAUDE.md`, `.claude/`, `factory/`, `.github/workflows/archaeo-studio-ci.yml`

## 4. 완료 기준 및 확인 방법
| 완료 기준 | 확인 명령 / 테스트 | 결과 |
| :--- | :--- | :--- |
| 문헌 카드 변환·중복 병합·verified 및 quoteStatus 판정 | `node scripts/v2/tools/check.js` 및 `node --test tests/` | 통과 (tests/v2_lit_outline.test.js) |
| 질의 생성기 유적명 하드코딩 0건 및 사실 카드 기반 동적 생성 | `node scripts/v2/tools/check.js` 및 `node --test tests/` | 통과 (tests/v2_lit_outline.test.js) |
| 없는 카드·근거 없는 주장·빈 절 자동 제거 및 게이트 G2/G3 판정 | `node scripts/v2/tools/check.js` 및 `node --test tests/` | 통과 (tests/v2_lit_outline.test.js) |
