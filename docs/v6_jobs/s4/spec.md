# 4단계 · 보고서 쪽 추출과 사실 카드 (P1 · G1) — ② 명세 (Spec)

## 1. 생성 파일
- `scripts/v2/page_extractor.js`
- `scripts/v2/prompts/fact_extract.md`
- `scripts/v2/fact_extractor.js`
- `tests/v2_facts.test.js`

## 2. 수정 파일
- 없음

## 3. 절대 수정하지 않는 보호 경로
- `bin/`, `patches/`, `CLAUDE.md`, `.claude/`, `factory/`, `.github/workflows/archaeo-studio-ci.yml`

## 4. 완료 기준 및 확인 방법
| 완료 기준 | 확인 명령 / 테스트 | 결과 |
| :--- | :--- | :--- |
| 가상 보고서 쪽 추출 및 로마 숫자 장 구분(Ⅰ~Ⅳ) | `node scripts/v2/tools/check.js` 및 `node --test tests/` | 통과 (tests/v2_facts.test.js) |
| 원문 quote 대조 및 수치 일치 결정적 검증으로 verified/unverified 분리 및 G1 판정 | `node scripts/v2/tools/check.js` 및 `node --test tests/` | 통과 (tests/v2_facts.test.js) |
