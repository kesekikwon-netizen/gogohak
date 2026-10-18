# 9단계 · 유적 라이브러리 (P9) — ⑦ 단계 보고서 (12장 · 14.6 양식)

## 14.6 사람 검토 대상 변경 여부
- 없음

## 12장 보고 항목
- **브랜치**: `v6/s9-library`
- **커밋 SHA**: `b65afb7`
- **만든 파일**: scripts/v2/site_library.js, tests/v2_site_library.test.js
- **바꾼 파일**: scripts/v2/crosswalk.js (sourceTag 표시 보강)
- **검증 명령 및 종료 코드**: `node scripts/v2/tools/check.js` (`0`), `node --test tests/` (`0`)

### 완료 기준 대조표
| 완료 기준 | 판정 |
| :--- | :--- |
| 동일 유적 2회 적재 시 verified 카드 중복 방지 | 통과 (tests/v2_site_library.test.js) |
| 불러온 카드의 sourceTag("라이브러리: <유적키>/<카드ID>") 대조표 표기 | 통과 (tests/v2_site_library.test.js) |
