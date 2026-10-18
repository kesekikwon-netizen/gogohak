# 9단계 · 유적 라이브러리 (P9) — ② 명세 (Spec)

## 1. 생성 파일
- `scripts/v2/site_library.js`
- `tests/v2_site_library.test.js`

## 2. 수정 파일
- `scripts/v2/crosswalk.js (sourceTag 표시 보강)`

## 3. 절대 수정하지 않는 보호 경로
- `bin/`, `patches/`, `CLAUDE.md`, `.claude/`, `factory/`, `.github/workflows/archaeo-studio-ci.yml`

## 4. 완료 기준 및 확인 방법
| 완료 기준 | 확인 명령 / 테스트 | 결과 |
| :--- | :--- | :--- |
| 동일 유적 2회 적재 시 verified 카드 중복 방지 | `node scripts/v2/tools/check.js` 및 `node --test tests/` | 통과 (tests/v2_site_library.test.js) |
| 불러온 카드의 sourceTag("라이브러리: <유적키>/<카드ID>") 대조표 표기 | `node scripts/v2/tools/check.js` 및 `node --test tests/` | 통과 (tests/v2_site_library.test.js) |
