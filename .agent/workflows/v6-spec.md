---
description: ARCHAEO STUDIO 6.1 개발 절차 ② 명세 및 ③ 계획 작성 (spec.md, plan.md 생성)
---

# `/v6-spec` 워크플로 (14.1 ② 명세 · ③ 계획)

1. `docs/v6_jobs/s<번호>/intent.md`를 읽는다.
2. 완료 기준마다 확인 방법(단위 테스트 이름 또는 실행 명령)을 1:1로 연결하고, 바꿀 파일과 바꾸지 않을 파일(보호 경로 `bin/`, `patches/`, `CLAUDE.md`, `.claude/`, `factory/`)을 명시하여 `docs/v6_jobs/s<번호>/spec.md`에 저장한다.
3. 파일별 작업 순서, 먼저 작성할 테스트, 위험한 지점(실패 모드)을 `docs/v6_jobs/s<번호>/plan.md`에 저장한다.
