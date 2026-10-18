# 1단계 · 테스트 기반 구축 — ② 명세 (Spec)

## 1. 생성 파일
- `package.json`
- `tests/index.js`
- `tests/fixtures/virtual_report.txt`
- `tests/existing_functions.test.js`

## 2. 수정 파일
- `scripts/gemini_mcp_server.js (+1줄, extractStructuredArchaeologyReportFacts export)`

## 3. 절대 수정하지 않는 보호 경로
- `bin/`, `patches/`, `CLAUDE.md`, `.claude/`, `factory/`, `.github/workflows/archaeo-studio-ci.yml`

## 4. 완료 기준 및 확인 방법
| 완료 기준 | 확인 명령 / 테스트 | 결과 |
| :--- | :--- | :--- |
| node --test tests/ 통과 (7개 테스트) | `node scripts/v2/tools/check.js` 및 `node --test tests/` | 통과 (7/7 pass) |
| git diff --stat 기존 파일 변경 1줄 이내 | `node scripts/v2/tools/check.js` 및 `node --test tests/` | 통과 (1 insertion) |
| 외부 npm 패키지 의존성 0개 | `node scripts/v2/tools/check.js` 및 `node --test tests/` | 통과 (dependencies 없음) |
