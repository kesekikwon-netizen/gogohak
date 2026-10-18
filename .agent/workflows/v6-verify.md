---
description: ARCHAEO STUDIO 6.1 개발 절차 ⑤ 검증 (verify.md 생성)
---

# `/v6-verify` 워크플로 (14.1 ⑤ 검증)

1. `node scripts/v2/tools/check.js` 및 `node --test tests/`를 실행한다.
2. 실행한 명령어, 종료 코드, 통과 테스트 수, 그리고 미실행 항목(있을 경우 `"미실행"`으로 명시)을 `docs/v6_jobs/s<번호>/verify.md`에 기록한다.
3. 로그 원문은 커밋하지 않고 경로와 요약만 기록한다.
