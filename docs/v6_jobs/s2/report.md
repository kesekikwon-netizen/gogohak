# 2단계 · 하네스 계층 (4장·5장) — ⑦ 단계 보고서 (12장 · 14.6 양식)

## 14.6 사람 검토 대상 변경 여부
- scripts/v2/config.default.json 임계값·시간 초과 기본값 정의 (14.6 검토 대상)

## 12장 보고 항목
- **브랜치**: `v6/s2-harness`
- **커밋 SHA**: `91c27fa`
- **만든 파일**: scripts/v2/config.default.json, scripts/v2/harness/errors.js, scripts/v2/harness/events.js, scripts/v2/harness/checkpoint.js, scripts/v2/harness/supervisor.js, scripts/v2/harness/selectors.json, scripts/v2/harness/dom_probe.js, scripts/v2/harness/gemini_driver.js, scripts/v2/harness/dbpia_driver.js, scripts/v2/harness/craft_driver.js, scripts/v2/harness/selftest.js, scripts/v2/harness/fake_drivers.js, tests/harness.test.js
- **바꾼 파일**: scripts/gemini_mcp_server.js (--selftest CLI 플래그 추가)
- **검증 명령 및 종료 코드**: `node scripts/v2/tools/check.js` (`0`), `node --test tests/` (`0`)

### 완료 기준 대조표
| 완료 기준 | 판정 |
| :--- | :--- |
| 감독기 시간 초과 → 재시도 → 성공 / 재시도 소진 → needs-human | 통과 (tests/harness.test.js) |
| 체크포인트 중단 후 재개 시 완료 단위 건너뛰기 및 깨진 파일 재실행 | 통과 (tests/harness.test.js) |
| Gemini 끝 표식(<<END>>) 절단 검출 및 JSON 파싱 재요청 | 통과 (tests/harness.test.js) |
| 동시 실행 잠금(.lock) 및 죽은 PID 잠금 회수 | 통과 (tests/harness.test.js) |
