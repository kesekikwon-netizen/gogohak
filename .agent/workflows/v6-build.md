---
description: ARCHAEO STUDIO 6.1 개발 절차 ④ 구현 (테스트 우선 작성 → 최소 구현 → 로컬 관문 check.js 통과)
---

# `/v6-build` 워크플로 (14.1 ④ 구현)

1. `docs/v6_jobs/s<번호>/spec.md`와 `plan.md`를 읽는다.
2. 해당 단계 브랜치(`v6/s<번호>-<이름>`)에서 테스트(`tests/*.test.js`)를 먼저 작성하거나 갱신한다.
3. 명세에 정의된 최소 구현만 수행한다 (특정 유적 하드코딩 금지, 사용자 절대경로 금지, Google Workspace 키워드 금지, `innerHTML =` 금지).
4. `node scripts/v2/tools/check.js`를 실행하여 로컬 관문을 통과한 뒤 커밋한다.
