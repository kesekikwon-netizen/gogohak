# 2단계 · 하네스 계층 (4장·5장) — ② 명세 (Spec)

## 1. 생성 파일
- `scripts/v2/config.default.json`
- `scripts/v2/harness/errors.js`
- `scripts/v2/harness/events.js`
- `scripts/v2/harness/checkpoint.js`
- `scripts/v2/harness/supervisor.js`
- `scripts/v2/harness/selectors.json`
- `scripts/v2/harness/dom_probe.js`
- `scripts/v2/harness/gemini_driver.js`
- `scripts/v2/harness/dbpia_driver.js`
- `scripts/v2/harness/craft_driver.js`
- `scripts/v2/harness/selftest.js`
- `scripts/v2/harness/fake_drivers.js`
- `tests/harness.test.js`

## 2. 수정 파일
- `scripts/gemini_mcp_server.js (--selftest CLI 플래그 추가)`

## 3. 절대 수정하지 않는 보호 경로
- `bin/`, `patches/`, `CLAUDE.md`, `.claude/`, `factory/`, `.github/workflows/archaeo-studio-ci.yml`

## 4. 완료 기준 및 확인 방법
| 완료 기준 | 확인 명령 / 테스트 | 결과 |
| :--- | :--- | :--- |
| 감독기 시간 초과 → 재시도 → 성공 / 재시도 소진 → needs-human | `node scripts/v2/tools/check.js` 및 `node --test tests/` | 통과 (tests/harness.test.js) |
| 체크포인트 중단 후 재개 시 완료 단위 건너뛰기 및 깨진 파일 재실행 | `node scripts/v2/tools/check.js` 및 `node --test tests/` | 통과 (tests/harness.test.js) |
| Gemini 끝 표식(<<END>>) 절단 검출 및 JSON 파싱 재요청 | `node scripts/v2/tools/check.js` 및 `node --test tests/` | 통과 (tests/harness.test.js) |
| 동시 실행 잠금(.lock) 및 죽은 PID 잠금 회수 | `node scripts/v2/tools/check.js` 및 `node --test tests/` | 통과 (tests/harness.test.js) |
