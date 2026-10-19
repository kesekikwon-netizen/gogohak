# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## 개요

ARCHAEO STUDIO 6.1: 한국 고고학 발굴조사보고서 『Ⅳ. 고찰』과 학술 논문을 근거 기반으로 집필·검증·조판하는 Windows 전용 Node.js 도구. 빌드 단계와 npm 의존성이 없다(`package.json`에 dependencies 없음, Node 18+ 표준 모듈만 사용). 사용자와 문서는 모두 한국어로 작성한다.

제품 코드 구현은 Antigravity(Gemini)가 담당하고, 명세는 `ARCHAEO_STUDIO_6.1_Antigravity_구현명세서_하네스_교차검증.md`(특히 0장 금지 규칙, 14장 개발 절차)가 기준이다. 충돌 시 명세서와 `rules/AGENTS.md`를 따른다.

## 명령

```sh
node scripts/v2/tools/check.js            # 로컬 관문 전체 (CI v6-check와 동일)
node scripts/v2/tools/check.js --staged   # pre-commit 훅이 쓰는 형태 (git config core.hooksPath .githooks)
node --test tests/                        # 단위 테스트 전체 (npm test 동일)
node --test tests/v2_facts.test.js        # 파일 하나
node --test --test-name-pattern="verifyFactAgainstPages" tests/   # 테스트 하나
node --check scripts/gemini_mcp_server.js # 구문 검사 (CI는 enricher·studio_app도 검사)
```

실행(Windows 실기기에서만 의미 있음): `ARCHAEOLOGY_STUDIO_실행.bat` → `node scripts/gemini_mcp_server.js --standalone`. `--selftest`는 CDP·Gemini·DBpia 연결 사전 진단이다. Linux 클라우드 환경에서는 테스트와 관문만 실행 가능하고, Gemini 웹·DBpia·WordCraft 연동은 확인할 수 없다.

## 구조

- `scripts/gemini_mcp_server.js` (~7.4k줄): MCP 브리지 + 로컬 HTTP 서버(`127.0.0.1:18787`). Antigravity 내장 Gemini 웹과 DBpia AI를 CDP로 조종하고, 레거시(v1) 파이프라인과 전역 `pipelineState`를 가진다.
- `scripts/standalone_studio_app.js`: `getStandaloneStudioHtml()`이 스튜디오 UI 전체를 HTML 문자열로 반환한다. 인라인 `<script>`는 CI가 `vm.Script`로 컴파일 검사하므로 문자열 안의 JS 구문 오류도 CI 실패가 된다.
- `scripts/archaeology_visuals_enricher.js`: 원고 Markdown에 `wc-chart`/`wc-diagram` 블록과 표를 보강한다.
- `scripts/v2/`: 6.1 근거 중심 파이프라인. `index.js`가 P1~P9 단계를 `harness/supervisor.js`의 `runSteps`로 엮고, 진행 상황을 `notifyLegacyPipeline`으로 v1 서버의 `pipelineState`에 반영한다(v1·v2가 하나의 서버/UI를 공유).
  - 근거 저장소(`evidence_store.js`, `ids.js`): 사실 `F-0001`, 문헌 `R-001`, 연대 `D-01`. 본문 수치 뒤 `[F-xxxx]`, 문헌 주장 뒤 `[R-xxx]` 표지(`markers.js`)가 강제된다.
  - `verify/`: `deterministic.js`(V1~V8 결정적 검사), `fact_checker`/`citation_checker`/`critic`(X1~X3 독립 Gemini 대화), `consensus.js`(X4 합의·관문 G5), `dbpia_review.js`(G7 최종 DBpia 검수).
  - `harness/`: 감독기·체크포인트 재개·오류 분류·이벤트 로그·셀렉터 탐침(`selectors.json`, `dom_probe.js`). `fake_drivers.js`가 테스트용 가짜 Gemini/DBpia/Craft 드라이버다.
  - `prompts/*.md`: 단계별 Gemini 프롬프트. `config.default.json`: 기본 설정.
- 작업 산출물은 `~/archaeology_reports/jobs/<jobId>/`에 쓰인다(저장소 밖).
- `tests/`: `node:test` 기반. 외부 연결 없이 `tests/fixtures/`의 가상 보고서·DBpia 응답과 가짜 드라이버로 돈다.
- `bin/`(WordCraft·PdfCraft 사전빌드 exe), `patches/`: 바이너리·패치. 수정하지 않는다.

## 관문(check.js) 규칙과 함정

- 보호 경로 `bin/`, `patches/`, `CLAUDE.md`, `.claude/`, `factory/`가 변경 목록(기준 커밋 `9c54e8f`..HEAD + 작업 트리)에 있으면 실패한다. 이 파일 자체도 보호 경로라서, CLAUDE.md나 `.claude/`를 커밋하면 `v6-check`가 실패한다. Claude 쪽 설정 파일을 추가할 때는 이 충돌을 사용자에게 알리고, `check.js` 수정은 사람 승인 없이 하지 않는다.
- 추가된 줄에서 금지 패턴을 검사한다: `innerHTML =` 직접 대입(텍스트/DOM API 사용), 사용자 절대경로 하드코딩(`os.homedir()`·`__dirname`·`%USERPROFILE%`·`%~dp0` 사용), 워크스페이스/메일/드라이브 멘션 키워드. 명세서·README·`docs/v6_jobs/`·`.agent/workflows/`·`skills*/`는 예외.
- `skills/archaeo-studio-pro/SKILL.md`와 `skills_backup/archaeo-studio-pro/SKILL.md`는 내용이 같아야 한다. 하나를 고치면 둘 다 고친다.
- `.github/workflows/archaeo-studio-ci.yml`은 서버·UI 코드에 특정 함수명/문자열(`extractStructuredArchaeologyReportFacts`, `renderManuscriptOntoA4Page` 등)이 남아 있는지 검사한다. 이름을 바꾸거나 지우면 CI가 깨진다.
- 특정 유적명 분기나 고정 수치를 하드코딩하지 않는다. 모든 표·차트·각주는 입력 자료와 DBpia 결과에서만 생성한다(`rules/AGENTS.md`).

## 개발 기록

Antigravity 작업은 `docs/v6_jobs/s<번호>/`에 intent → spec → plan → verify → review → report 순으로 남기고, 절차는 `.agent/workflows/v6-*.md`에 있다.
