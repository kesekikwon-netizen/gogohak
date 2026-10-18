# 7-1단계 · 최종 DBpia AI 문헌 검수 (P7.5 · X4 · G7) — ② 명세 (Spec)

## 1. 생성 파일
- `scripts/v2/prompts/dbpia_review_query.md`
- `scripts/v2/verify/dbpia_review.js`
- `tests/v2_dbpia_review.test.js`

## 2. 수정 파일
- 없음

## 3. 절대 수정하지 않는 보호 경로
- `bin/`, `patches/`, `CLAUDE.md`, `.claude/`, `factory/`, `.github/workflows/archaeo-studio-ci.yml`

## 4. 완료 기준 및 확인 방법
| 완료 기준 | 확인 명령 / 테스트 | 결과 |
| :--- | :--- | :--- |
| 핵심 주장 최대 5건 선별 및 당해 유적 고유명·유구 번호 제거 질의 생성 | `node scripts/v2/tools/check.js` 및 `node --test tests/` | 통과 (tests/v2_dbpia_review.test.js) |
| 학계 통설과 상충 시 G7 중단, 신규 문헌(L_new1) 추출, 사용자 선택 전 원고 자동 수정 금지 | `node scripts/v2/tools/check.js` 및 `node --test tests/` | 통과 (tests/v2_dbpia_review.test.js) |
