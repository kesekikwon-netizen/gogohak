# 4단계 · 보고서 쪽 추출과 사실 카드 (P1 · G1) — ⑦ 단계 보고서 (12장 · 14.6 양식)

## 14.6 사람 검토 대상 변경 여부
- G1 게이트 판정 로직 (14.6 검토 대상)

## 12장 보고 항목
- **브랜치**: `v6/s4-facts`
- **커밋 SHA**: `f9b5d76`
- **만든 파일**: scripts/v2/page_extractor.js, scripts/v2/prompts/fact_extract.md, scripts/v2/fact_extractor.js, tests/v2_facts.test.js
- **바꾼 파일**: 없음
- **검증 명령 및 종료 코드**: `node scripts/v2/tools/check.js` (`0`), `node --test tests/` (`0`)

### 완료 기준 대조표
| 완료 기준 | 판정 |
| :--- | :--- |
| 가상 보고서 쪽 추출 및 로마 숫자 장 구분(Ⅰ~Ⅳ) | 통과 (tests/v2_facts.test.js) |
| 원문 quote 대조 및 수치 일치 결정적 검증으로 verified/unverified 분리 및 G1 판정 | 통과 (tests/v2_facts.test.js) |
