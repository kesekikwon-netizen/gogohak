# 7단계 · Claude 2차 다듬기 인계 및 반입 재검증 (P7 · G6) — ② 명세 (Spec)

## 1. 생성 파일
- `scripts/v2/handoff.js`
- `tests/v2_handoff.test.js`

## 2. 수정 파일
- 없음

## 3. 절대 수정하지 않는 보호 경로
- `bin/`, `patches/`, `CLAUDE.md`, `.claude/`, `factory/`, `.github/workflows/archaeo-studio-ci.yml`

## 4. 완료 기준 및 확인 방법
| 완료 기준 | 확인 명령 / 테스트 | 결과 |
| :--- | :--- | :--- |
| 인계 묶음 9종 파일 생성 및 manifest.json SHA-256 해시 기록 | `node scripts/v2/tools/check.js` 및 `node --test tests/` | 통과 (tests/v2_handoff.test.js) |
| 미등록 수치(99.9m)·미등록 문헌(김가짜 2024)·해시 변조 시 G6 즉시 거부 | `node scripts/v2/tools/check.js` 및 `node --test tests/` | 통과 (tests/v2_handoff.test.js) |
| 유효 2차 원고 반입 시 변경 문장만 X1/X2 재검증 후 최종 manuscript.md 생성 | `node scripts/v2/tools/check.js` 및 `node --test tests/` | 통과 (tests/v2_handoff.test.js) |
