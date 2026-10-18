# 10단계 · 문서·스킬 동기화 및 14장 개발 절차(관문·훅·CI·워크플로) 반영 — ② 명세 (Spec)

## 1. 생성 파일
- `scripts/v2/tools/check.js`
- `.githooks/pre-commit`
- `.github/workflows/v6-check.yml`
- `.agent/workflows/v6-intent.md`
- `.agent/workflows/v6-spec.md`
- `.agent/workflows/v6-build.md`
- `.agent/workflows/v6-verify.md`
- `.agent/workflows/v6-review.md`
- `.agent/workflows/v6-report.md`
- `ARCHAEO_STUDIO_6.1_Antigravity_구현명세서_하네스_교차검증.md`

## 2. 수정 파일
- `README.md`
- `1_ARCHAEO_STUDIO_전체개발방법_설계도.md`
- `skills/archaeo-studio-pro/SKILL.md`
- `skills_backup/archaeo-studio-pro/SKILL.md`

## 3. 절대 수정하지 않는 보호 경로
- `bin/`, `patches/`, `CLAUDE.md`, `.claude/`, `factory/`, `.github/workflows/archaeo-studio-ci.yml`

## 4. 완료 기준 및 확인 방법
| 완료 기준 | 확인 명령 / 테스트 | 결과 |
| :--- | :--- | :--- |
| node scripts/v2/tools/check.js 종료 코드 0 통과 | `node scripts/v2/tools/check.js` 및 `node --test tests/` | 통과 (Exit code 0) |
| skills/archaeo-studio-pro/SKILL.md 와 skills_backup/archaeo-studio-pro/SKILL.md 완전 일치 | `node scripts/v2/tools/check.js` 및 `node --test tests/` | 통과 |
| 33개 전체 단위·통합 테스트 통과 | `node scripts/v2/tools/check.js` 및 `node --test tests/` | 통과 (33/33 pass) |
