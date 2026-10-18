# ARCHAEO STUDIO 6.1 — Antigravity(Gemini) 구현 명세서 (하네스 · 교차검증 개정판)

작성일: 2026-10-18 · 대상 저장소: `kesekikwon-netizen/gogohak` (기준 커밋 `9c54e8f`)
읽는 이: 이 저장소를 구현할 Antigravity 에이전트(Gemini 4 Argon, 사고 수준 최고)
이 파일은 6.0 명세서에 6.1 개정(하네스·교차검증·Claude 2차 인계·최종 DBpia AI 검수)을 반영한 최신판이다. 같은 폴더의 `ARCHAEO_STUDIO_6.1_Antigravity_구현명세서_하네스_교차검증.md`와 내용이 같다.

> **이 문서를 받은 에이전트에게.** 이 문서는 작업 지시서다. 0장 규칙을 먼저 읽고, 11장 단계를 **순서대로 하나씩** 진행하라. 단계가 끝날 때마다 완료 기준을 모두 확인하고, 12장 양식으로 보고한 뒤 사용자 승인을 받고 다음 단계로 넘어가라. 이 문서에 없는 기능은 만들지 마라. 모호한 곳은 추측해서 구현하지 말고 사용자에게 질문하라.

### 6.0 대비 바뀐 점
1. **하네스 계층 신설(4장).** Gemini·DBpia·PdfCraft·WordCraft 호출을 모두 감독기(supervisor)가 감싼다. 시간 초과·재시도·체크포인트 재개·화면 구조 변화 감지·사전 자가진단·구조화 로그가 들어간다.
2. **Gemini 최대 노력 고정(5장).** 4 Argon High + `작업` 모드를 **설정 후 다시 읽어 확인**하고, 한 번의 호출에는 한 가지 일만 시키며, 응답 끝 표식으로 잘림을 검출한다.
3. **독립 교차검증(7장).** 집필 대화와 분리된 검증 대화 3종(사실·인용·심사)과 결정적 검사를 합의 규칙으로 묶는다. 둘이 엇갈리면 자동 통과시키지 않는다.
4. **Claude 2차 다듬기 연결(8장).** 1차 결과를 "인계 묶음"으로 내보내고, 2차 원고를 다시 받아 결정적 검사를 한 번 더 통과시킨 뒤 내보낸다.
4-1. **최종 DBpia AI 문헌 검수(7.7).** 모든 작성이 끝난 최종 원고의 핵심 해석·인용 주장을 DBpia AI에 질의해, 학계 문헌과 어긋나는 주장과 빠진 핵심 선행연구를 찾는다.
5. 단계 순서 변경: **하네스를 먼저** 만들고(11장 2단계) 그 위에 기능을 올린다.

---

## 0. 반드시 지킬 규칙

### 0.1 기존 규칙 유지 (`rules/AGENTS.md`, `skills/archaeo-studio-pro/SKILL.md`)
1. **특정 유적 하드코딩 금지.** 유적명·지역명으로 분기하는 코드, 특정 유적용 고정 표·차트·모식도 금지. 테스트 자료도 `가상유적` 같은 가상 이름만 쓴다.
2. **사용자 경로 하드코딩 금지.** `os.homedir()`, `__dirname`, `%USERPROFILE%`, `%~dp0`만 쓴다.
3. **자료 날조 금지.** 보고서·수집 문헌에 없는 수치·연대·저자·쪽수를 만들지 않는다. 없으면 없는 대로 쓰고 "확인 필요"로 표시한다.
4. **Google Workspace 키워드 금지.** 프롬프트에 `@Google Workspace`, `@Gmail`, `@Google Drive`, `Google`을 넣지 않는다(부정문 포함).
5. **Gemini 입력 규칙 유지.** 줄바꿈이 든 문자열을 `execCommand('insertText')`로 넣지 않는다. `innerHTML` 대입 금지. `injectTextIntoGeminiEditor()` 경로만 쓴다.

### 0.2 개발 작업 규칙
1. **기존 동작을 깨지 않는다.** 새 엔진(v2)은 `scripts/v2/` 별도 모듈이며 사용자가 고를 때만 동작한다. 기존 v1 경로(`runArchaeologyDiscussionPipeline`)는 고치지 않는다. 단, 4장 하네스의 Gemini 드라이버는 v1도 나중에 쓸 수 있게 설계하되, v1에 연결하는 일은 사용자 승인 후 별도 단계로 한다.
2. **CI가 문자열로 찾는 이름을 지우거나 바꾸지 않는다**(`.github/workflows/archaeo-studio-ci.yml`).
3. **새 npm 의존성 금지.** Node 내장 모듈과 `node:test`, `node:assert`만 쓴다. `package.json`은 테스트 명령 용도로만 만든다(의존성 0개). Node 22의 전역 `WebSocket`·`fetch`를 쓴다(기존 코드와 동일).
4. `bin/`의 exe와 `patches/`는 수정하지 않는다.
5. `skills/archaeo-studio-pro/SKILL.md`를 고치면 `skills_backup/archaeo-studio-pro/SKILL.md`도 똑같이 고친다.
6. 루트의 `CLAUDE.md`, `.claude/`, `factory/`는 다른 작업이 쓰는 자리다. 건드리지 않는다.
7. 단계마다 별도 브랜치(`v6/s<번호>-<이름>`)에서 작업한다. main 직접 커밋·push·병합 금지. 병합은 사용자가 한다.
8. 테스트를 통과시키려고 기대값을 근거 없이 바꾸지 않는다. 바꿔야 하면 이유를 보고하고 승인받는다.

10. 모든 단계는 14장 개발 절차를 따른다.

---

## 1. 목표

**한 문장 목표:** 장시간 실행해도 멈추거나 조용히 틀리지 않고, 원고의 모든 수치·인용이 보고서 쪽수와 실제 문헌 구절로 교차검증된 고찰·논문을 만든다.

| 영역 | 5.0 (현재) | 6.1 (목표) |
| --- | --- | --- |
| 실행 안정성 | 함수마다 제각각 시간 초과, 실패 시 처음부터 | 감독기가 단계별 재시도·체크포인트 재개, 사전 자가진단 |
| 화면 변화 대응 | 셀렉터가 코드 곳곳에 흩어짐, 바뀌면 조용히 실패 | 셀렉터 한곳 관리 + 실행 전 탐침 검사로 즉시 "화면 구조 변경" 보고 |
| Gemini 사용 | 긴 프롬프트 1회로 전체 원고 | 한 호출 한 작업, 모드 확인, 잘림 검출 |
| 보고서 읽기 | 장별 긴 텍스트 | 쪽 단위 추출 → **사실 카드**(쪽수·원문 구절) |
| 문헌 | DBpia 결과를 통째로 주입 | **문헌 카드**(서지 + 인용 구절) |
| 집필 | 전체 생성 후 수정 | 목차 승인 → 절 단위, 문장마다 근거 표시 |
| 검증 | 각주 개수·글자 수 | 결정적 검사 + **독립 검증 대화 3종** + 합의 규칙 |
| 2차 다듬기 | 없음 | Claude 2차 원고 연결, 재검증 후 내보내기 |
| 결과물 | .docx(+PDF) | .docx(+PDF) + 근거 대조표 + 검증 매트릭스 + 실행 로그 |

유지: Antigravity 안 Gemini 웹(CDP), DBpia AI 수집 함수들, WordCraft·PdfCraft, 웹 스튜디오 디자인(`tokens.css`), MCP 도구 7개.

---

## 2. 전체 흐름

```
[H0 사전 자가진단] selftest — 하나라도 실패하면 시작하지 않고 원인과 조치를 표시
   │
[P1 사실 추출]  쪽 단위 추출 → 장 구분 → 장별 Gemini JSON 추출 → 원문 대조 → 사실 카드
   │  ◆게이트 G1
[P2 문헌 수집]  사실 카드 → DBpia 질의 → askDbpiaAi → 문헌 카드
   │  ◆게이트 G2
[P3 목차 설계]  Gemini 목차 JSON → 정리 → (사용자 승인 대기 또는 자동 승인)
   │  ◆게이트 G3
[P4 절 단위 집필] 절마다 새 프롬프트, 그 절의 카드만, 근거 표시 ⟦F12,L3⟧
   │
[P5 교차검증]   결정적 검사 V1~V8 → 독립 검증 대화 X1(사실)·X2(인용)·X3(심사)
   │            → 합의 규칙 → 통과 / 재작성(절마다 최대 2회) / 확인 필요
   │  ◆게이트 G5
[P6 조립]       각주 변환·표·차트·모식도·근거 대조표·검증 매트릭스
   │
[P7 인계(선택)] Claude 2차 다듬기용 인계 묶음 내보내기 → 2차 원고 수신
   │            → V1~V8 재검사 → 통과 시 2차 원고 채택
   │
[P7.5 최종 DBpia AI 검수] 최종 원고의 핵심 해석·인용 주장 → DBpia AI 질의
   │            → 지지 / 반론 / 누락 선행연구 판정 → 검수 보고서
   │  ◆게이트 G7 (반론 판정이 있으면 사용자 확인 후 내보내기)
   │
[P8 내보내기]   exportManuscriptToWordCraftAndPdfCraft()로 .docx/.pdf
[P9 누적 저장]  유적 라이브러리

모든 단계는 감독기(4장) 아래에서 실행된다: 시간 초과·재시도·체크포인트·로그.
```

---

## 3. 파일 구조

```
scripts/
  gemini_mcp_server.js              (기존) v2 진입점·/v2 경로·--selftest 옵션 연결만 추가
  standalone_studio_app.js          (기존) 엔진 선택·자가진단·목차 승인·검증 결과 화면 추가
  archaeology_visuals_enricher.js   (기존) 그대로, v2가 함수를 불러 씀
  v2/
    config.default.json       모든 시간 초과·재시도·임계값 기본값
    index.js                  runPipelineV2() — 단계 정의를 감독기에 넘김
    harness/
      supervisor.js           단계 실행기: 시간 초과·재시도·체크포인트·재개
      errors.js               오류 코드 목록과 분류(재시도 가능/불가/사람 필요)
      events.js               구조화 로그(JSONL) 기록·요약
      checkpoint.js           run.json 상태 저장·복원(원자적 쓰기)
      selectors.json          Gemini·DBpia 화면 셀렉터 한곳 관리
      dom_probe.js            셀렉터 탐침: 화면 구조 변경 감지
      selftest.js             사전 자가진단(H0)
      gemini_driver.js        askGeminiWeb을 감싸는 안정 호출기(5장)
      dbpia_driver.js         askDbpiaAi를 감싸는 안정 호출기
      craft_driver.js         PdfCraft·WordCraft CLI 호출기(시간 초과·종료 코드 검사)
      fake_drivers.js         테스트용 가짜 드라이버(녹화 응답 재생)
    schemas.js  ids.js  evidence_store.js  markers.js
    page_extractor.js  fact_extractor.js  literature_cards.js  query_builder.js
    outline_planner.js  section_writer.js
    verify/
      deterministic.js        V1~V8 결정적 검사
      claim_splitter.js       절 원고 → 검증 단위 주장 목록
      fact_checker.js         X1 사실 검증 대화
      citation_checker.js     X2 인용 검증 대화
      critic.js               X3 심사 대화
      consensus.js            합의 규칙 → 문장별 판정
      dbpia_review.js         X4 최종 DBpia AI 문헌 검수(7.7)
    visuals_from_facts.js  crosswalk.js  assembler.js
    handoff.js                Claude 인계 묶음 내보내기·2차 원고 수신(8장)
    site_library.js
    prompts/
      fact_extract.md  outline.md  section_report.md  section_paper.md
      check_fact.md  check_citation.md  critic_report.md  critic_paper.md
      dbpia_review_query.md   X4 질의문 틀(DBpia AI에 보내는 질문 형식)
tests/
  fixtures/
    virtual_report.txt          가상유적 보고서 발췌([[PAGE n]] 포함)
    virtual_dbpia.json          가상 DBpia 응답
    virtual_manuscript.md       근거 표시가 든 가상 원고(일부러 틀린 문장 포함)
    gemini_replies/*.txt        녹화·작성한 Gemini 응답(정상·잘림·JSON 깨짐·빈 응답)
  existing_functions.test.js
  v2_*.test.js
  harness_*.test.js
package.json                    {"scripts":{"test":"node --test tests/"}} 만
```

### 3.1 작업 폴더 구조
`<outputDir>/<작업ID>/` (기본 `~/archaeology_reports/`, git에 넣지 않음)
```
run.json              단계별 상태·시도 횟수·체크포인트(재개 기준)
events.jsonl          구조화 로그(4.5)
selftest.json         실행 직전 자가진단 결과
pages/p0001.txt …     쪽별 원문
facts.json  literature.json  outline.json
sections/S2.r0.md …   절 원고(회차별)
verify/S2.r0.json …   절별 검증 매트릭스(7.6)
raw/                  Gemini 원응답 보관(디버깅·재현용, 회차별)
manuscript_v1.md      1차 조립 원고
handoff/              Claude 인계 묶음(8장)
manuscript_v2.md      2차 원고(있을 때)
dbpia_review.json     최종 DBpia AI 검수 결과(7.7)
dbpia_review.md       사람이 읽는 검수 보고서
crosswalk.md          근거 대조표
<제목>.docx / .pdf
```

---

## 4. 하네스 계층

목표: **어떤 실패도 조용히 지나가지 않고, 어떤 중단도 처음부터 다시 하지 않는다.**

### 4.1 감독기 (`harness/supervisor.js`)
모든 단계(P1~P9)와 그 안의 반복 작업(장별 추출, 질의별 수집, 절별 집필·검증)을 "작업 단위(step)"로 정의해 감독기에서 실행한다.

```js
// 작업 단위 정의
{
  id: "P4.S2.r0",                 // 단계.대상.회차 — 체크포인트 키
  run: async (ctx) => result,      // 실제 작업
  validate: (result) => ({ ok, code, detail }),  // 결과 검사(형식·내용)
  timeoutMs: cfg.timeouts.writeSection,
  retry: { max: 2, backoffMs: [5000, 20000], on: ["E_GEMINI_TIMEOUT", "E_TRUNCATED", "E_EMPTY_REPLY", "E_CDP_DISCONNECT"] },
  onExhausted: "needs-human" | "skip-with-flag" | "abort",
  saveAs: "sections/S2.r0.md"      // 성공 결과 저장 위치
}
```
규칙:
1. **체크포인트:** 작업 단위가 `validate` 통과 후 결과를 저장하면 `run.json`에 `done`으로 기록한다. 재실행 시 `done`인 단위는 건너뛴다(저장 파일이 있고 형식 검사를 통과할 때만).
2. **재개:** 스튜디오의 `[이어서 실행]` 버튼과 `/v2/resume` 경로로 마지막 미완료 단위부터 다시 시작한다. 앱이나 Antigravity가 꺼졌다 켜져도 같은 작업ID로 재개할 수 있어야 한다.
3. **시간 초과:** `Promise.race`로 감싸고, 시간 초과 시 진행 중인 Gemini 생성을 멈추고(`hardStopGeminiAndPipeline()`의 Gemini 정지 부분만 재사용) 오류 코드 `E_GEMINI_TIMEOUT`.
4. **재시도:** `retry.on`에 있는 오류만 재시도한다. 재시도 전에 4.4의 복구 동작을 먼저 한다. 재시도마다 **새 대화**로 시작한다(앞 시도의 잔여 맥락 차단).
5. **소진 시:** `needs-human`이면 파이프라인을 멈추고 화면에 원인·조치를 표시한다. `skip-with-flag`면 그 대상(예: 문헌 질의 1개)을 건너뛰고 결과에 표시를 남긴다. 어떤 단위가 어떤 정책인지는 4.6 표를 따른다.
6. **일시멈춤·중단:** 기존 `waitWhilePausedOrAborted()`를 작업 단위 사이와 재시도 사이에서 호출한다.
7. **동시 실행 1개:** 같은 PC에서 v2 작업은 한 번에 하나만. `~/archaeology_reports/.lock` 파일(작업ID·PID·시작 시각)로 막고, PID가 살아 있지 않으면 잠금을 풀 수 있게 한다.

### 4.2 셀렉터 한곳 관리와 화면 구조 변경 감지 (`selectors.json`, `dom_probe.js`)
1. Gemini·DBpia 화면에서 쓰는 모든 셀렉터를 `selectors.json`에 이름을 붙여 모은다. 예: `gemini.editor`, `gemini.sendButton`, `gemini.stopButton`, `gemini.responseContainer`, `gemini.modelPicker`, `gemini.workModeButton`(`gem-segmented-button[data-value="40"]`), `dbpia.searchInput`, `dbpia.answerContainer`. 셀렉터마다 후보를 배열로 둘 수 있다(앞에서부터 시도).
2. **v2 코드는 셀렉터 문자열을 직접 쓰지 않고 이름으로만 참조한다.** 기존 v1 코드의 셀렉터는 이번 판에서 옮기지 않는다(동작 변경 금지). 대신 기존 코드에서 쓰는 셀렉터를 조사해 `selectors.json`에 같은 값으로 등록한다.
3. `dom_probe.js`의 `probe(target, names[])`: 각 이름에 대해 `Runtime.evaluate`로 존재 여부·보임 여부·개수를 돌려준다. 필수 셀렉터가 없으면 `E_DOM_DRIFT` + 어떤 이름이 없는지.
4. 탐침 시점: H0 자가진단, 각 Gemini 호출 직전(편집기·전송 버튼만, 가볍게), 실패 직후(전체).
5. `E_DOM_DRIFT`는 재시도하지 않는다(화면 구조가 바뀐 것이므로). 화면에 "Gemini(또는 DBpia) 화면 구조가 바뀌었습니다: `<이름>`을 찾지 못함"을 띄우고, 그 시점의 DOM 일부(해당 영역 `outerHTML` 앞 4,000자)를 `raw/dom_<시각>.html`로 저장해 수리 근거로 남긴다. 개인정보가 담길 수 있으므로 이 파일은 작업 폴더 밖으로 보내지 않는다.

### 4.3 사전 자가진단 (`harness/selftest.js`, H0)
실행 방법 세 가지: 스튜디오 `[자가진단]` 버튼, `GET /v2/selftest`, `node scripts/gemini_mcp_server.js --selftest`(결과 JSON 출력, 실패 시 종료 코드 1).

| 번호 | 항목 | 확인 방법 | 실패 코드 | 사용자 조치 문구 |
| --- | --- | --- | --- | --- |
| T1 | Node 버전 | `process.versions.node` ≥ 18 | `E_ENV_NODE` | Node 18 이상 설치 |
| T2 | CDP 포트 | 기존 `getCdpPort()` | `E_CDP_PORT` | Antigravity 실행 |
| T3 | Gemini 웹뷰 | `ensureEmbeddedGeminiWebview()` 후 대상 존재 | `E_GEMINI_WEBVIEW` | Gemini 패널 열기 |
| T4 | Gemini 로그인 | 편집기 셀렉터 존재 + 로그인 버튼 부재 | `E_GEMINI_LOGIN` | Gemini 로그인 |
| T5 | 모델·모드 | 5.1 설정 후 다시 읽기로 확인 | `E_GEMINI_MODE` | 수동으로 4 Argon·작업 선택 |
| T6 | Gemini 셀렉터 | `probe(gemini.*)` | `E_DOM_DRIFT` | 개발 수리 필요 |
| T7 | DBpia 로그인 | 기존 `getDbpiaLoginStatus()` | `E_DBPIA_LOGIN` | DBpia 로그인 |
| T8 | DBpia 셀렉터 | `probe(dbpia.*)` | `E_DOM_DRIFT` | 개발 수리 필요 |
| T9 | PdfCraft CLI | `pdfcraft-cli` 존재 + 버전 출력 종료 코드 0 | `E_CRAFT_EXE` | 저장소 `bin/` 확인 |
| T10 | WordCraft CLI | `wordcraft-cli` 존재 + 종료 코드 0 | `E_CRAFT_EXE` | 저장소 `bin/` 확인 |
| T11 | 쓰기 권한·여유 공간 | 작업 폴더에 임시 파일 쓰기·지우기 | `E_DISK` | 다른 저장 위치 지정 |
| T12 | 잠금 | 4.1-7 잠금 없음 | `E_LOCKED` | 실행 중인 작업 종료 대기 |

- T7·T8은 문헌 수집을 끈 실행(사용자가 문헌 목록을 직접 준 경우)에서는 경고로만 처리한다.
- 결과는 `selftest.json`에 저장하고 스튜디오에 항목별 통과/실패로 보여 준다.

### 4.4 오류 분류와 자동 복구 (`harness/errors.js`)

| 코드 | 뜻 | 재시도 | 재시도 전 자동 복구 |
| --- | --- | --- | --- |
| `E_CDP_PORT` | CDP 포트 없음 | 아니오 | — (사람 필요) |
| `E_CDP_DISCONNECT` | 웹소켓 끊김·명령 시간 초과 | 예 | 웹뷰 대상 다시 찾기, 새 세션 |
| `E_GEMINI_TIMEOUT` | 응답 완료 대기 시간 초과 | 예 | Gemini 생성 정지 → 새 대화 |
| `E_EMPTY_REPLY` | 응답이 비었음 | 예 | 새 대화 |
| `E_TRUNCATED` | 끝 표식 없음(5.3) | 예 | 새 대화, 요청 범위 축소(카드 수 줄이기) |
| `E_JSON_PARSE` | JSON 응답 읽기 실패 | 예(1회는 같은 대화에서 "JSON만 다시") | 그다음은 새 대화 |
| `E_SCHEMA` | JSON은 읽혔으나 형식 위반 | 예 | 오류 내용을 넣어 재요청 |
| `E_GEMINI_MODE` | 모드 확인 실패 | 예(1회) | 모드 재설정 |
| `E_GEMINI_QUOTA` | 사용 한도 메시지 감지 | 아니오 | 체크포인트 저장 후 일시정지, 화면 안내 |
| `E_GEMINI_LOGIN` / `E_DBPIA_LOGIN` | 로그인 풀림 | 아니오 | 일시정지, 로그인 안내, 로그인 후 재개 |
| `E_DOM_DRIFT` | 셀렉터 없음 | 아니오 | DOM 일부 저장 |
| `E_DBPIA_EMPTY` | DBpia 답변·참고문헌 없음 | 예(1회) | 질의 단순화 버전으로 재시도 |
| `E_CRAFT_EXE` / `E_CRAFT_FAIL` | exe 없음·비정상 종료 | 실패 시 1회 | — |
| `E_GATE` | 게이트 미통과(4.7) | 아니오 | 원인 표시 |
| `E_ABORTED` | 사용자 중단 | 아니오 | 체크포인트 저장 |

- 한도·로그인 감지 문구는 `selectors.json`의 `gemini.quotaTextPatterns`, `gemini.loginIndicators`에 둔다(화면 문구가 바뀔 수 있으므로 코드에 박지 않는다).
- 사람에게 보이는 모든 오류 메시지는 한국어 한 줄 원인 + 한 줄 조치.

### 4.5 구조화 로그 (`harness/events.js`)
`events.jsonl` 한 줄 = 한 사건:
```json
{"t":"2026-10-18T21:03:11.402Z","job":"20261018-210002","step":"P4.S2.r0","attempt":2,"type":"retry","code":"E_TRUNCATED","ms":184233,"chars":5120,"detail":"끝 표식 없음"}
```
- `type`: `start`, `end`, `retry`, `fail`, `gate`, `probe`, `pause`, `resume`.
- 프롬프트·응답 전문은 로그에 넣지 않고 `raw/`에 파일로 두고 경로만 기록한다.
- 기존 `addPipelineLog()`에도 사람이 읽을 한 줄을 같이 남겨 스튜디오 진행 기록이 지금처럼 보이게 한다.
- `GET /v2/metrics?jobId=`: 단계별 소요 시간, 재시도 횟수, 오류 코드 집계, Gemini 호출 수·총 글자 수.

### 4.6 작업 단위별 정책

| 작업 단위 | 시간 초과 키 | 최대 재시도 | 소진 시 |
| --- | --- | --- | --- |
| P1 쪽 추출(파일 1개) | `craftExtract` | 1 | needs-human |
| P1 장별 사실 추출 | `geminiJson` | 2 | skip-with-flag(그 장은 "추출 실패"로 표시) |
| P2 DBpia 질의 1개 | `dbpiaQuery` | 1 | skip-with-flag |
| P3 목차 | `geminiJson` | 2 | needs-human |
| P4 절 집필 1회 | `writeSection` | 2 | needs-human |
| P5 검증 대화 1회 | `geminiJson` | 2 | 그 문장은 "검증 불가 → 확인 필요" |
| P7.5 DBpia 검수 질의 1개 | `dbpiaQuery` | 1 | skip-with-flag(그 주장은 "검수 미실시"로 표시) |
| P8 내보내기 | `craftExport` | 1 | needs-human(원고 Markdown은 이미 저장됨) |

`config.default.json` 초기값(실측 후 조정, 근거: 기존 코드의 CDP 명령 시간 초과 60초, Gemini 장문 생성은 수 분 소요 경험):
```json
{
  "timeouts": { "craftExtract": 120000, "geminiJson": 420000, "writeSection": 600000, "dbpiaQuery": 240000, "craftExport": 180000, "cdpCommand": 60000 },
  "retry": { "backoffMs": [5000, 20000] },
  "gemini": { "maxPromptChars": 12000, "minIntervalMs": 8000, "stableMs": 4000 },
  "gates": { "G1_minVerifiedRatio": 0.6, "G2_minVerifiedLiterature": 3, "G5_maxNeedsHumanRatio": 0.2 },
  "verify": { "doubleCheckKinds": ["dating", "chronology"], "maxRework": 2 },
  "dbpiaReview": { "enabled": true, "maxQueries": 10 }
}
```
게이트 임계값(`gates`)은 **잠정값**이다. 기존 기준이 없어 추정한 값이므로 11장 7단계 실측 후 사용자 확인을 받아 확정한다.

### 4.7 게이트(결정적 관문)
게이트는 다음 단계로 넘어가도 되는지 코드로 판정한다. 미통과 시 `E_GATE`로 멈추고 이유와 선택지를 화면에 보여 준다(계속/중단).

| 게이트 | 위치 | 판정 |
| --- | --- | --- |
| G1 | P1 후 | 사실 카드 1개 이상, `verified` 비율 ≥ `G1_minVerifiedRatio`, 추출 실패 장 목록 표시 |
| G2 | P2 후 | `verified` 문헌 카드 ≥ `G2_minVerifiedLiterature` (논문 모드는 미달 시 반드시 멈춤, 고찰 모드는 경고 후 계속 가능) |
| G3 | P3 후 | 목차 형식 검사 통과, 모든 주장에 존재하는 카드 번호, 사용자 승인(또는 자동 승인 설정) |
| G5 | P5 후 | 확인 필요 문장 비율 ≤ `G5_maxNeedsHumanRatio`, block 판정 0건(확인 필요로 넘긴 것 제외) |
| G7 | P7.5 후 | `반론` 판정 0건이면 통과. 1건 이상이면 멈추고 검수 보고서를 보여 준 뒤 사용자가 [그대로 내보내기 / 해당 절 재작성 / 중단] 선택 |

### 4.8 가짜 드라이버와 재생 테스트 (`fake_drivers.js`)
- `gemini_driver`, `dbpia_driver`, `craft_driver`는 같은 인터페이스의 가짜 구현을 가진다. 가짜는 `tests/fixtures/gemini_replies/`의 응답을 순서대로 돌려주거나, 지정한 오류(`E_TRUNCATED` 등)를 일으킨다.
- 파이프라인 전체(`runPipelineV2`)를 가짜 드라이버로 **Windows·Antigravity 없이** 끝까지 돌리는 테스트를 만든다. 이 테스트가 CI(ubuntu)에서도 돈다.
- 실기 실행 때 `raw/`에 남은 응답을 익명화해 fixture로 추가하면 회귀 테스트가 늘어난다(실제 보고서 문장·개인정보는 넣지 않는다. 가상 이름으로 바꾼다).

---

## 5. Gemini 최대 노력 운용 (`harness/gemini_driver.js`)

### 5.1 모델·모드 고정과 확인
1. 호출 전 기존 `ensureArgonHighAndWorkMode()`로 설정한다.
2. **설정 후 다시 읽어 확인**한다: 모델 선택 표시에 4 Argon과 사고 수준 최고가 보이는지, `작업` 버튼이 선택 상태인지(`aria-pressed`/`aria-selected`/선택 클래스 중 `selectors.json`에 정한 방식). 확인 실패면 1회 재설정, 그래도 실패면 `E_GEMINI_MODE`.
3. 새 대화를 열 때마다 다시 확인한다(새 대화에서 모드가 풀릴 수 있음 — 확인 항목).

### 5.2 한 호출 한 작업
- 한 번의 Gemini 호출은 한 가지 산출물만 만든다(장 1개의 사실, 목차 1개, 절 1개, 검증 1회).
- 프롬프트 첫머리에 역할·목표·입력·출력 형식·금지 사항·자기 점검 목록을 고정 순서로 둔다(8장 틀 참고).
- 프롬프트 길이는 `maxPromptChars` 이하. 넘으면 우선순위가 낮은 카드(관련도 낮은 문헌 카드 → 요약만 남김)부터 줄이고 로그를 남긴다. 사실 카드의 `quote`는 줄이지 않는다.
- 연속 호출 사이 최소 간격 `minIntervalMs`.

### 5.3 응답 완료 판정과 잘림 검출
1. 완료 판정: 정지 버튼이 사라지고 응답 텍스트 길이가 `stableMs` 동안 변하지 않으면 완료.
2. **끝 표식:** 모든 프롬프트는 응답 마지막 줄에 `<<END:{작업ID}>>`를 쓰게 한다(예: `<<END:P4.S2.r0>>`). 표식이 없으면 `E_TRUNCATED`. 표식은 결과 저장 전에 제거한다.
3. JSON 응답은 마지막 ```json 블록만 읽는다(기존 `parseJsonFromGeminiReply()`), 그다음 `schemas.js`로 형식 검사.
4. 원응답 전문은 항상 `raw/<작업ID>.<시도>.txt`에 저장한다.

### 5.4 대화 분리 원칙
- 역할이 바뀔 때마다 새 대화(`resetGeminiChat()`): 추출 / 목차 / 집필 / X1 / X2 / X3은 서로 다른 대화.
- 같은 절의 재작성은 **새 대화**에서 하고, 앞 원고와 지적 사항만 넣는다(앞 대화의 추론은 넘기지 않음).
- 검증 대화에는 집필 프롬프트·집필자의 설명을 넣지 않는다(7.2 "블라인드").

---

## 6. 데이터 형식

`scripts/v2/schemas.js`에 형식별 검사 함수(`validateX(obj) → {ok, errors[]}`)로 구현한다.

### 6.1 사실 카드 (FactCard)
```json
{
  "id": "F12",
  "kind": "feature | artifact | stratum | dating | chronology | site | other",
  "subject": "3호 주거지",
  "attribute": "장축 길이",
  "value": "6.2",
  "unit": "m",
  "quote": "3호 주거지는 장축 6.2m, 단축 4.1m로",
  "source": { "file": "보고서.pdf", "page": 57, "chapter": "Ⅲ" },
  "confidence": "verified | unverified",
  "note": ""
}
```
**원문 대조(결정적):** ① 공백·줄바꿈 정규화 후 `quote`가 해당 쪽 원문의 부분 문자열 ② 숫자 `value`가 `quote` 안에 있음(`6.2`=`6.20`). 하나라도 실패하면 `unverified`. `page`를 모르면 `null`, 대조표에 "쪽수 미상".

### 6.2 문헌 카드 (LiteratureCard)
```json
{
  "id": "L3", "nodeId": "NODE01234567",
  "authors": ["홍길동"], "year": "2015", "title": "논문 제목", "journal": "학술지명",
  "volume": "", "issue": "", "pages": "1~30",
  "quotes": ["hovers 인용 구절 1", "인용 구절 2"],
  "summary": "DBpia AI 요점 중 이 논문 관련 부분",
  "query": "이 논문을 찾은 질의",
  "verified": true
}
```
출처는 `extractLatestDbpiaAnswer()`·`enrichDbpiaReferencesMetadata()` 반환값뿐. 저자·연도가 비면 `verified:false`(각주 사용 금지). 같은 `nodeId`는 합치고 `quotes`를 합친다. `quotes`가 비면 그 카드는 "서지만 있음"으로 표시하고 X2에서 "구절 없음" 판정을 받는다.

### 6.3 목차 (Outline)
```json
{
  "docType": "report | paper", "title": "Ⅳ. 고찰", "approved": false,
  "omitted": [{ "heading": "연대 분석", "reason": "연대 자료 없음" }],
  "sections": [{
    "id": "S2", "heading": "1. 주거지의 구조와 축조 방식",
    "claims": [{ "id": "S2.C1", "text": "…로 파악된다", "evidence": ["F12", "F13", "L3"] }],
    "visuals": [{ "type": "table", "purpose": "유구 제원 일람", "evidence": ["F12","F13","F14"] }],
    "targetChars": 1200
  }]
}
```
없는 카드 번호를 가진 주장, 근거 없는 주장, 주장 없는 절은 `outline_planner.js`가 제거하고 로그를 남긴다. 기본 절: 고찰 = 머리말 / 유구 분석 / 유물·편년 / 주변 유적 비교와 의의 / 맺음말. 논문 = 서론(문제 제기·연구사) / 자료 검토 / 분석 / 논의 / 결론.

### 6.4 근거 표시 (Marker)
`…로 판단된다⟦F12,L3⟧.` 정규식 `/⟦((?:[FL]\d+)(?:\s*,\s*[FL]\d+)*)⟧/g`. 대비 형식 `[[F12,L3]]`도 같은 뜻. 조립 시 F 표시는 지우고(대조표로), L 표시는 각주 `1)`로 바꾼다.

### 6.5 주장 단위 (Claim)
```json
{ "id": "S2.r0.K4", "sentence": "원문 문장", "atoms": ["3호 주거지 장축 6.2m", "평면 방형"], "markers": ["F12"], "kind": "fact | interpretation | citation" }
```
`claim_splitter.js`가 문장 단위로 자르고(결정적), 숫자·연대·저자명이 든 문장을 `fact`/`citation`으로 분류한다. `atoms`(한 문장 안의 개별 사실)는 X1 대화에서 함께 만든다.

### 6.6 검증 매트릭스 (VerifyMatrix) — 7.6 참고

---

## 7. 교차검증 설계

원칙: **쓴 대화는 검증하지 않는다. 검증은 근거 원문과 대조한다. 코드 판정과 모델 판정이 엇갈리면 사람에게 넘긴다.**

### 7.1 결정적 검사 V1~V8 (`verify/deterministic.js`)

| 번호 | 검사 | 실패 기준 | 등급 |
| --- | --- | --- | --- |
| V1 | 근거 표시 존재 | 숫자(m·cm·㎡·점·기·BP·년 등)가 든 문장에 F 표시 없음 | block |
| V2 | 수치 일치 | 문장 숫자가 가리키는 F 카드 `value`와 다름(m↔cm 환산 허용) | block |
| V3 | 카드 실재 | 존재하지 않는 카드 번호 | block |
| V4 | 인용 서지 | L 표시 문장의 저자명이 카드 저자와 다름 / `verified:false` 카드 사용 | block |
| V5 | 미확인 사실 | `unverified` F 카드 사용 | warn |
| V6 | 목차 이행 | 절 주장의 근거 카드 중 절반 이상이 원고에 없음 | warn |
| V7 | 금지 표현 | `저자미상`, `연도미상`, Google Workspace 키워드 | block |
| V8 | 표·차트 칸 근거 | 표·차트의 값 칸에 근거 카드가 연결되지 않음 | block |

### 7.2 독립 검증 대화 3종
모두 **새 대화**, **블라인드**(집필 프롬프트·의도 미제공), **JSON 출력**, 끝 표식 사용.

**X1 사실 검증 (`fact_checker.js`, `prompts/check_fact.md`)**
- 입력: 절의 `fact` 주장 목록(문장 + 표시된 F 카드 + 그 카드 쪽의 원문 앞뒤 600자).
- 지시: "각 문장을 개별 사실(atom)로 나누고, 각 사실이 제시된 원문에 **글자로 적혀 있는지만** 판정하라. 추론으로 맞을 것 같다는 판단은 '근거없음'이다."
- 출력: `[{claimId, atom, verdict: "일치|불일치|근거없음", evidenceQuote, page}]`. `evidenceQuote`는 원문에서 그대로 복사.

**X2 인용 검증 (`citation_checker.js`, `prompts/check_citation.md`)**
- 입력: `citation` 주장(문장 + L 카드 서지·`quotes`·`summary`).
- 지시: "문장이 그 논문의 주장으로 제시한 내용이 인용 구절에 의해 뒷받침되는지 판정하라."
- 출력: `[{claimId, verdict: "지지|부분지지|불지지|구절없음", evidenceQuote}]`.

**X3 심사 (`critic.js`, `prompts/critic_report.md` / `critic_paper.md`)**
- 입력: 절 원고(근거 표시 제거본) + 절 주장 목록.
- 역할: 고찰은 국가유산 발굴조사보고서 평가위원, 논문은 『한국고고학보』 심사위원.
- 관점: 논증이 근거에서 나오는가, 과도한 단정, 용어 정확성, 비교 유적 선택의 타당성, 절 구성.
- 출력: `{verdict: "accept|revise", issues:[{type, sentence, comment, suggestion}]}`. 칭찬 금지.

### 7.3 결정적 재확인 (모델 판정 검산)
- X1이 `일치`라고 한 경우 `evidenceQuote`가 실제 원문의 부분 문자열인지 코드로 확인한다. 아니면 그 판정은 무효(`검증불가`).
- X2의 `evidenceQuote`도 L 카드 `quotes`·`summary` 안에 있는지 확인한다.
- 이 단계 덕분에 검증 모델이 근거를 지어내도 통과하지 못한다.

### 7.4 이중 검증
`verify.doubleCheckKinds`(기본: 연대·편년)에 해당하는 주장은 X1을 **두 번** 별도 대화에서 돌린다. 두 번째는 원문 발췌의 순서를 바꿔 넣는다. 두 판정이 다르면 `확인 필요`.

### 7.5 합의 규칙 (`verify/consensus.js`)
문장별 최종 판정:

| 결정적 검사(V) | X1/X2 판정(재확인 후) | 최종 |
| --- | --- | --- |
| block 있음 | 무엇이든 | **재작성** |
| 통과 | 일치/지지 | **통과** |
| 통과 | 불일치/불지지 | **재작성** (지적 내용 포함) |
| 통과 | 근거없음/구절없음/부분지지 | **재작성 1회 → 그래도면 확인 필요** |
| 통과 | 검증불가(형식 오류·재확인 실패·이중 검증 불일치) | **확인 필요** |
| warn만 있음 | 일치/지지 | 통과 + 꼬리표 |

- X3의 `revise` 지적은 문체·구성 수정용이다. 사실 판정을 뒤집지 않는다. 재작성 프롬프트에 함께 넣되, 재작성 후에도 X3 지적이 남으면 경고로만 기록한다.
- 재작성은 절마다 `maxRework`(기본 2)회. 소진 후 남은 문장은 지우지 않고 `〔확인 필요: 사유〕`를 붙인다.
- **재작성된 절은 V·X1·X2를 처음부터 다시 받는다.** 앞 회차 통과 결과를 재사용하지 않는다.

### 7.6 검증 매트릭스 (`verify/S2.r1.json`)
```json
{
  "sectionId": "S2", "round": 1,
  "rows": [{
    "claimId": "S2.r1.K4", "sentence": "…",
    "deterministic": { "V1": "pass", "V2": "pass", "V3": "pass" },
    "x1": [{ "atom": "3호 주거지 장축 6.2m", "verdict": "일치", "evidenceQuote": "…", "page": 57, "rechecked": true }],
    "x2": [], "double": null,
    "final": "pass | rework | needs-human", "reason": ""
  }],
  "critic": { "verdict": "revise", "issues": [] },
  "summary": { "pass": 14, "rework": 2, "needsHuman": 1 }
}
```
스튜디오 완료 화면에 절별 요약과 "확인 필요" 문장 목록(쪽수 링크 포함)을 보여 준다.

### 7.7 최종 DBpia AI 문헌 검수 (X4, `verify/dbpia_review.js`)
모든 작성(1차 검증, 선택적으로 Claude 2차 다듬기와 재검사)이 끝난 **최종 원고**에 대해, 마지막으로 외부 학술 문헌과 맞서 보는 단계다. Gemini 검증(X1~X3)은 이미 모은 카드 안에서만 판단하므로, 카드 밖의 반론이나 빠진 핵심 연구를 찾는 역할을 DBpia AI가 맡는다.

**무엇을 검수하고 무엇은 하지 않는가**
- 대상: `interpretation`(편년·성격·계통·의의 판단)과 `citation`(선행연구 인용) 주장.
- 제외: 유구 크기·유물 수량 같은 보고서 고유 사실. DBpia에는 이 유적 보고서 원문이 없으므로 이 판단은 X1과 결정적 검사에만 맡긴다.
- 원고 전체를 DBpia AI에 붙여 넣지 않는다. DBpia AI는 검색 질의응답 도구라 긴 원고 심사에 맞지 않고, 입력 길이 한계도 확인되지 않았다. 주장마다 짧은 질문으로 바꿔 보낸다.

**흐름**
1. 대상 주장 고르기: 절마다 핵심 주장(목차 `claims`) 중심으로 최대 `dbpiaReview.maxQueries`개(기본 10). 연대·편년 주장을 우선한다. 고른 목록을 검수 보고서에 남긴다.
2. 질의 만들기(`prompts/dbpia_review_query.md`, 결정적 틀): 주장을 판단이 담긴 의문문으로 바꾼다. 예: "중부지역 凸자형 주거지의 연대는 원삼국시대로 보는 것이 학계의 일반적 견해인가? 이견이 있는 연구도 알려 줘." 유적명은 넣지 않고 시대·지역·유구·유물 용어로 쓴다(유적명 하드코딩 금지 규칙과 같은 이유, 그리고 DBpia에 그 유적 자료가 없을 수 있음).
3. 질의 실행: 기존 `askDbpiaAi()`를 `dbpia_driver`로 감싸 실행. 결과는 6.2 형식의 문헌 카드로 바꿔 `literature.json`과 별도로 `dbpia_review.json`에 담는다.
4. 판정: Gemini 새 대화(블라인드)에 "주장 + DBpia 요점 + 인용 구절"을 주고 아래 셋 중 하나로 판정하게 한다. 판정 근거 구절은 7.3과 같이 코드로 실재 여부를 재확인한다.
   - `지지` — DBpia 문헌이 주장과 같은 방향.
   - `반론` — 주장과 다른 견해를 낸 문헌이 있음(구절 필수).
   - `판단불가` — 관련 문헌을 찾지 못함.
   추가로 `누락선행연구`: 원고의 문헌 카드에 없는데 DBpia 답변이 핵심 근거로 든 논문 목록.
5. 처리 규칙:
   - `반론`이 있으면 원고를 자동으로 고치지 않는다. 게이트 G7에서 멈추고 사용자에게 보여 준다. 사용자가 "재작성"을 고르면 그 절만 P4로 돌아가며, 반론 문헌을 새 문헌 카드로 추가해 "이견을 소개하고 본 유적 자료로 어느 쪽인지 논하라"는 지적과 함께 재작성한다. 재작성된 절은 P5 검증을 처음부터 다시 받는다.
   - `누락선행연구`는 원고에 자동 삽입하지 않고 제안 목록으로만 둔다. 사용자가 고르면 문헌 카드로 추가해 같은 방식으로 재작성·재검증한다.
   - `판단불가`는 경고 없이 보고서에만 남긴다(문헌이 없다는 것이 틀렸다는 뜻은 아님).
6. 결과물: `dbpia_review.md` — 주장 / 판정 / 근거 문헌(저자·연도·제목) / 인용 구절 / 조치. 스튜디오 완료 화면과 근거 대조표에 판정 열을 추가한다.

**주의**
- 이 단계는 DBpia 로그인이 필요하다. 로그인이 풀려 있으면 `E_DBPIA_LOGIN`으로 일시정지하고, 사용자가 "검수 건너뛰기"를 고르면 결과물에 "최종 DBpia 검수 미실시"를 표시한다.
- DBpia 사용량·응답 시간이 실측되지 않았으므로 `maxQueries` 기본값 10은 잠정값이다(8단계에서 조정).
- 사용자가 끌 수 있다(`dbpiaReview.enabled`, 스튜디오 체크박스).

---

## 8. Claude 2차 다듬기 연결 (`handoff.js`)

1차는 Gemini(수집·초안·1차 검증), 2차는 Claude(정밀 다듬기)다. 이 앱은 묶음을 내보내고 2차 원고를 받아 **다시 검증**하는 부분만 구현한다. Claude 쪽 도구는 이 저장소 범위 밖이다.

### 8.1 인계 묶음 (`<작업폴더>/handoff/`)
```
manifest.json        형식 버전 "handoff/1", 작업ID, 문서 종류, 유적명, 생성 시각, 파일 목록과 SHA-256
manuscript_v1.md     1차 원고(근거 표시 ⟦⟧ 유지, 각주 변환 전)
outline.json         승인된 목차
facts.json           사실 카드
literature.json      문헌 카드
pages/               보고서 쪽별 원문
verify/              마지막 회차 검증 매트릭스
needs_human.md       확인 필요 문장 목록
README_인계.md       2차 작업 규칙(아래 8.2)을 사람이 읽을 수 있게 정리
```
스튜디오 완료 화면 `[2차 다듬기용 내보내기]` 버튼, `POST /v2/handoff/export`.

### 8.2 2차 작업 규칙 (README_인계.md에 그대로 씀)
1. 근거 표시 `⟦F12,L3⟧`를 지우지 말 것. 문장을 고치면 표시도 그 문장에 남길 것.
2. 새 사실·수치·인용을 추가하려면 반드시 기존 카드 번호를 달 것. 카드에 없는 내용은 추가하지 말 것.
3. 바꾼 이유는 `changes.json`에 `[{sectionId, before, after, reason}]`로 남길 것.
4. 결과는 `manuscript_v2.md`, `changes.json` 두 파일로 같은 폴더에 둘 것.

### 8.3 2차 원고 수신
- `POST /v2/handoff/import` 또는 `[2차 원고 불러오기]`: `handoff/manuscript_v2.md` 읽기.
- 수신 검사: ① `manifest.json`의 작업ID 일치 ② 7.1 V1~V8 전부 재실행 ③ 1차에 없던 카드 번호·새 숫자가 있으면 그 문장은 X1/X2를 다시 돌림(사용자가 원하면 건너뛰기 가능하나 그 경우 "미검증"으로 표시).
- 통과하면 2차 원고로 P6을 다시 진행하고, 그다음 P7.5 최종 DBpia 검수 → P8 내보내기로 간다. 실패 문장은 화면에 보여 주고 1차 문장으로 되돌릴지 사용자가 고른다.
- 2차 다듬기를 하지 않으면 1차 원고가 그대로 P7.5로 간다. 즉 **DBpia 검수는 어느 경로든 내보내기 직전 최종 원고에 한 번** 실행된다.

---

## 9. 모듈 인터페이스

순수 함수(입력→출력)와 입출력 함수를 나눈다. CDP·파일 입출력은 `harness/*_driver.js`, `evidence_store.js`, `page_extractor.js`, `index.js`에만 둔다.

| 모듈 | 주요 함수 | 비고 |
| --- | --- | --- |
| `harness/supervisor.js` | `runSteps(job, steps[], deps) → {status, results}` | 4.1 |
| `harness/checkpoint.js` | `load(job)`, `markDone(job, stepId, file)`, `isDone(job, stepId)` | 임시 파일에 쓰고 이름 변경 |
| `harness/selftest.js` | `runSelftest(deps) → {ok, items[]}` | 4.3 |
| `harness/dom_probe.js` | `probe(target, names[]) → {ok, missing[]}` | 4.2 |
| `harness/gemini_driver.js` | `ask({jobStep, prompt, expectJson, newChat}) → {text, json?, rawPath}` | 5장, 내부에서 `askGeminiWeb`·`resetGeminiChat`·`ensureArgonHighAndWorkMode` 사용 |
| `harness/dbpia_driver.js` | `query(q) → dbpiaResult` | `askDbpiaAi` 감쌈 |
| `harness/craft_driver.js` | `pdfInfo(path)`, `pdfPageText(path, n)`, `exportDocx(...)` | `pdfcraft-cli text <파일> --page N` |
| `page_extractor.js` | `extractPages(filePath) → [{page, text}]` | PDF 쪽 단위, DOCX·TXT는 `page:null` |
| `fact_extractor.js` | `splitChapters(pages)`, `buildFactPrompt(...)`, `parseFactReply(text)`, `verifyFactAgainstPages(card, pages)` | 장 구분은 기존 `extractStructuredArchaeologyReportFacts()` 규칙 재사용 |
| `literature_cards.js` | `toLiteratureCards(dbpiaResult, query)`, `mergeLiterature(cards)` | |
| `query_builder.js` | `buildQueriesFromFacts(facts, docType, userFocus)` | 기존 `buildMultiFacetedDbpiaQueries()` 감쌈, 최대 8개 |
| `outline_planner.js` | `buildOutlinePrompt(...)`, `parseOutline(text, cardIds)` | |
| `section_writer.js` | `buildSectionPrompt(section, cards, docType, prevSummary, fixes)`, `parseSectionReply(text)` | |
| `markers.js` | `parseMarkers`, `stripFactMarkers`, `markersToFootnotes(md, lit)` | 각주 문장은 `formatVerifiedCitationLine()` |
| `verify/*` | `runDeterministic`, `splitClaims`, `checkFacts`, `checkCitations`, `critique`, `decide` | 7장 |
| `visuals_from_facts.js` | `planVisuals(facts, lit, outline) → {specs[], skipped[]}` | 10장 |
| `crosswalk.js` | `buildCrosswalk(md, facts, lit, matrices)` | 문장 / 판정 / 근거 카드 / 원문 구절 / 쪽수 |
| `assembler.js` | `assemble(outline, sections, facts, lit)` | 각주·참고문헌 1:1, 가나다순 |
| `handoff.js` | `exportHandoff(job)`, `importHandoff(job)` | 8장 |
| `index.js` | `runPipelineV2(options)`, `resumePipelineV2(jobId)` | `pipelineState`, `addPipelineLog()`로 스튜디오 표시 유지 |

### 9.1 HTTP·MCP
기존 브리지(`127.0.0.1:18787`)에 추가, 기존 경로는 그대로.

| 경로 | 방식 | 용도 |
| --- | --- | --- |
| `/v2/selftest` | GET | 자가진단 결과 |
| `/v2/run` | POST | `{docType, userFocus, reportText?, datingText?, targetPath?, autoApproveOutline?, literatureMode?: "dbpia"|"manual"}` → `{ok, jobId}` |
| `/v2/resume` | POST | `{jobId}` |
| `/v2/state` | GET | 단계, 절별 상태·회차, 카드 수, 게이트 결과, 현재 오류 |
| `/v2/outline/approve` | POST | `{jobId, outline}` |
| `/v2/gate/continue` | POST | `{jobId, gate}` 게이트 경고 무시하고 계속 |
| `/v2/evidence` | GET | `?jobId=&type=facts|literature` |
| `/v2/verify` | GET | `?jobId=&section=` 검증 매트릭스 |
| `/v2/crosswalk` | GET | 근거 대조표 |
| `/v2/metrics` | GET | 4.5 |
| `/v2/handoff/export`, `/v2/handoff/import` | POST | 8장 |
| `/v2/dbpia-review` | GET / POST | 검수 결과 조회 / 검수 다시 실행·G7 선택(`{jobId, action: "export"|"rework"|"abort", sections?}`) |

MCP: 기존 `archaeology_auto_discussion`에 선택 인자 `engine`(`"v1"` 기본 / `"v2"`), `autoApproveOutline`(MCP 기본 `true`), `resumeJobId` 추가. 새 도구는 만들지 않는다.

`pipelineState.phase` 새 값: `V2_H0_SELFTEST`, `V2_P1_FACTS`, `V2_P2_LITERATURE`, `V2_P3_OUTLINE`, `V2_P3_WAIT_APPROVAL`, `V2_P4_WRITING`, `V2_P5_VERIFY`, `V2_P6_ASSEMBLE`, `V2_P7_HANDOFF_WAIT`, `V2_P75_DBPIA_REVIEW`, `V2_P8_EXPORT`, `V2_P9_LIBRARY`, `V2_DONE`, `V2_PAUSED`, `V2_NEEDS_HUMAN`.

---

## 10. 표·차트·모식도 규칙

개수 목표 대신 조건으로 만든다. 못 채우면 만들지 않고 `skipped`에 이유를 남긴다. 모든 값 칸은 카드 번호를 가진다(V8).

| 시각 자료 | 조건 | 출처 |
| --- | --- | --- |
| 유구 제원 일람표 | 크기 값이 있는 유구 카드 2개 이상 | F |
| 유물 속성 비교표 | 유물 카드 3개 이상 | F |
| 주변 유적 대비표 | 다른 유적을 다룬 L 카드 2개 이상 | L |
| 선행연구 쟁점표 | `verified` L 카드 3개 이상 | L |
| 유구 규모 차트 | 같은 속성 숫자 값 3개 이상 | F |
| 연대 차트 | 연대 카드 2개 이상(없으면 절대 만들지 않음) | F |
| 층위 모식도 | 층위 카드 2개 이상 | F |
| 단계 변천 모식도 | 중복관계·편년 단계 카드 2개 이상 | F |

그리기는 기존 `buildComparativeChartsFromData`, `buildSchematicDiagramsFromData`, `buildArtifactAndLiteratureTables`, `normalizeWcDiagramSpec`에 명세를 넘겨 한다. 기존 함수 동작은 바꾸지 않고, 필요하면 `visuals_from_facts.js`에 감싸는 함수를 둔다. 기존 `evaluateArchaeologyManuscriptQuality()`(C1~C8)는 최종 원고에 참고로 돌리되 C4·C5(개수)는 실패로 치지 않는다.

---

## 11. 단계별 작업 순서

**공통 검증(모든 단계, 전부 종료 코드 0):**
```bash
node --check scripts/gemini_mcp_server.js
node --check scripts/standalone_studio_app.js
node --check scripts/archaeology_visuals_enricher.js
node --test tests/
```
+ `archaeo-studio-ci.yml`의 두 인라인 `node -e` 검사를 로컬에서 그대로 실행해 통과.

### 1단계 · 테스트 기반 (`v6/s1-tests`)
- 할 일: `package.json`(의존성 없음), `tests/existing_functions.test.js`(기존 함수 회귀: `prepareMarkdownAndFootnotesForWordCraft`의 연도 괄호·표 안 토큰, `repairCollapsedMarkdownTables`, `normalizeWcDiagramSpec`, `parseFeatureDimensionsFromMarkdown`, `replaceInlineFootnoteSafely`, `evaluateArchaeologyManuscriptQuality`), `extractStructuredArchaeologyReportFacts`를 `module.exports`에 추가(동작 변경 없음), `tests/fixtures/virtual_report.txt` 직접 작성.
- 완료 기준
  - [ ] 공통 검증 통과, 실패 0.
  - [ ] 기존 파일 변경은 `module.exports` 한 줄뿐(`git diff --stat` 보고).

### 2단계 · 하네스 골격 (`v6/s2-harness`) ★ 안정성의 핵심
- 할 일: `config.default.json`, `harness/` 전 모듈, `fake_drivers.js`, `selectors.json`(기존 코드 셀렉터 조사·등록), `--selftest` 옵션과 `/v2/selftest`.
- 완료 기준
  - [ ] 감독기 테스트(가짜 드라이버): 시간 초과 → 재시도 → 성공 / 재시도 소진 → `needs-human` / 재시도 대상 아닌 오류는 즉시 멈춤 / 중간 중단 후 재개 시 `done` 단위를 다시 실행하지 않음 / 저장 파일이 깨졌으면 `done`이어도 다시 실행.
  - [ ] 끝 표식 없는 응답 → `E_TRUNCATED`, 빈 응답 → `E_EMPTY_REPLY`, 깨진 JSON → 같은 대화 1회 재요청 후 새 대화 테스트.
  - [ ] 잠금 파일: 동시 실행 거부, 죽은 PID 잠금 해제 테스트.
  - [ ] `selectors.json`에 등록한 각 셀렉터가 기존 코드의 어느 줄에서 쓰이는지 표로 보고.
  - [ ] **Windows 실기:** `node scripts/gemini_mcp_server.js --selftest`를 ① Antigravity 끈 상태 ② 켠 상태 ③ DBpia 로그아웃 상태에서 실행, 각 결과 JSON과 종료 코드 보고.
  - [ ] 실기에서 Gemini에 짧은 요청 3회(`gemini_driver.ask`)를 보내 모드 확인·끝 표식·응답 시간 측정 결과 보고.

### 3단계 · 데이터 형식·저장소 (`v6/s3-schemas`)
- 할 일: `schemas.js`, `ids.js`, `evidence_store.js`, `markers.js`.
- 완료 기준
  - [ ] 6장 모든 형식의 올바른 예·틀린 예 테스트.
  - [ ] `markers.js`: `⟦⟧`·`[[ ]]` 파싱, F 제거 후 문장부호 정상, L → 각주 번호 등장 순 1부터, 같은 L 재등장 시 같은 번호.

### 4단계 · 사실 추출 P1 + G1 (`v6/s4-facts`)
- 할 일: `page_extractor.js`, `fact_extractor.js`, `prompts/fact_extract.md`, 게이트 G1.
- 완료 기준
  - [ ] 가상 보고서로 장 구분 테스트, 가짜 응답으로 `verified/unverified` 분리 테스트(원문에 없는 수치 → 반드시 `unverified`).
  - [ ] **실기:** 사용자 제공 보고서 PDF 1권으로 `pages/` 수 = `pdfcraft-cli info` 쪽수 확인. 스캔본이라 텍스트가 비면 그대로 보고(한글 OCR 한계 — 확인 항목).
  - [ ] 실기 장 1개 추출: 카드 수, `verified` 비율, 실패 사례 3개, 재시도 발생 횟수 보고.

### 5단계 · 문헌 P2 + 목차 P3 + G2·G3 (`v6/s5-lit-outline`)
- 할 일: `literature_cards.js`, `query_builder.js`, `outline_planner.js`, `prompts/outline.md`, 게이트.
- 완료 기준
  - [ ] 문헌 카드 변환·중복 합치기·`verified` 판정 테스트.
  - [ ] 목차 정리(없는 카드·근거 없는 주장·빈 절 제거) 테스트.
  - [ ] 질의에 유적명 하드코딩이 없음(입력에서만 옴) 테스트.
  - [ ] 실기 DBpia 질의 3개: 문헌 카드 수, `verified` 수, `quotes` 있는 카드 수 보고.

### 6단계 · 집필 P4 + 교차검증 P5 + 조립 P6 + 화면 (`v6/s6-write-verify`)
- 할 일: `section_writer.js`, `verify/` 전체, 프롬프트 6개, `visuals_from_facts.js`, `crosswalk.js`, `assembler.js`, `index.js`, 서버 `/v2/*`, MCP 인자, 스튜디오(엔진 선택·자가진단·목차 승인·게이트 안내·검증 결과·`[이어서 실행]`). 디자인은 `tokens.css` 변수만.
- 완료 기준
  - [ ] V1~V8 각각 통과·실패 테스트(`virtual_manuscript.md`).
  - [ ] 7.3 재확인: 가짜 X1이 원문에 없는 `evidenceQuote`로 `일치`를 주면 `검증불가`가 되는지 테스트.
  - [ ] 7.5 합의 규칙 표의 모든 행을 테스트로 고정.
  - [ ] 재작성 2회 후 `〔확인 필요〕` 부착, 재작성 절은 검증을 처음부터 다시 받는지 테스트.
  - [ ] 가짜 드라이버로 `runPipelineV2` 전체 완주 테스트(CI에서도 통과).
  - [ ] `standalone_studio_app.js` 인라인 스크립트가 CI `new vm.Script` 검사 통과(템플릿 리터럴 안 줄바꿈은 `\\n`).
  - [ ] 실기: v1 실행이 기존과 같은지 1회 확인(회귀 없음).
  - [ ] 실기: v2로 보고서 1권 완주. 단계별 소요 시간, 재시도·오류 코드 집계(`/v2/metrics`), 절별 재작성 횟수, 확인 필요 건수 보고.
  - [ ] 실기: 실행 중 Antigravity를 강제로 닫았다 켜고 `[이어서 실행]`으로 재개되는지 확인.

### 7단계 · Claude 인계 P7 (`v6/s7-handoff`)
- 할 일: `handoff.js`, 화면 버튼 2개, `/v2/handoff/*`.
- 완료 기준
  - [ ] 내보낸 묶음의 `manifest.json` SHA-256이 실제 파일과 일치하는 테스트.
  - [ ] 가짜 `manuscript_v2.md`(① 정상 다듬기 ② 근거 표시 삭제 ③ 카드에 없는 숫자 추가)로 수신 검사 결과가 각각 통과 / V1 실패 / 재검증 대상 표시가 되는지 테스트.

### 7-1단계 · 최종 DBpia AI 검수 P7.5 (`v6/s7b-dbpia-review`)
- 할 일: `verify/dbpia_review.js`, `prompts/dbpia_review_query.md`, 게이트 G7, 화면(검수 보고서·G7 선택지·끄기 체크박스), `/v2/dbpia-review`.
- 완료 기준
  - [ ] 대상 주장 선택 테스트: 보고서 고유 수치 문장은 제외, 연대·편년 주장 우선, `maxQueries` 상한 준수.
  - [ ] 질의문에 유적명이 들어가지 않는지 테스트.
  - [ ] 가짜 DBpia 응답으로 `지지`/`반론`/`판단불가`/`누락선행연구` 판정 테스트, 근거 구절 재확인 실패 시 `판단불가`로 내려가는지 테스트.
  - [ ] `반론` 1건 이상이면 G7에서 멈추고, "재작성" 선택 시 그 절만 P4→P5를 다시 거치는지 가짜 드라이버로 테스트.
  - [ ] 실기: 완주한 원고 1건으로 검수 실행, 질의 수·판정 분포·소요 시간·반론 사례 보고.

### 8단계 · 실보고서 평가와 기본값 확정 (`v6/s8-eval`)
- 할 일: 사용자 제공 보고서 2~3권(1권 이상은 사람이 쓴 고찰 있음)을 v1·v2로 처리해 비교.
- 비교 항목: 원고 수치의 보고서 일치율(무작위 20개, 사람 확인), 각주 중 실제 문헌 비율, 확인 필요 건수, 완주율(재시도 포함), 평균 소요 시간, 사용자 주관 평가.
- 완료 기준
  - [ ] `docs/v6_평가결과.md` 작성(원문·개인정보 제외).
  - [ ] `config.default.json`의 시간 초과·게이트 임계값 조정안과 근거(실측값) 제시 → 사용자 승인.
  - [ ] 사용자가 v2 기본 엔진 전환 여부 결정. 그 전까지 기본값 v1.

### 9단계 · 유적 라이브러리 P9 (`v6/s9-library`) — 8단계 승인 후
- [ ] 같은 유적 두 번 처리해도 카드 중복 없음(테스트).
- [ ] 라이브러리 카드를 쓴 문장은 대조표에 "라이브러리: <유적키>/F12" 표시.

### 10단계 · 문서·스킬 갱신 (`v6/s10-docs`)
- [ ] `README.md`에 v2 사용법·자가진단·재개·인계 방법.
- [ ] `skills/archaeo-studio-pro/SKILL.md`와 `skills_backup/...`에 6.1 절 추가, `cmp` 결과 동일.
- [ ] CI 문서 존재 검사 통과.

---

## 12. 단계 보고 양식

```
## <단계 번호·이름> 보고
- 브랜치 / 기준 커밋 / 작업 커밋:
- 바꾼 파일:
- 완료 기준별 결과: (항목마다 통과/실패/미실행 + 실행 명령 + 종료 코드)
- 실기 확인 결과: (Gemini·DBpia·PdfCraft·WordCraft, 재시도·오류 코드 집계 포함)
- 기존부터 있던 실패 / 이번 변경으로 생긴 실패:
- 남은 위험:
- 되돌리는 방법:
- 사용자에게 필요한 결정:
```
실행하지 않은 검증을 통과로 쓰지 않는다.

---

## 13. 아직 확인되지 않은 사항

| 항목 | 왜 중요한가 | 확인 시점 |
| --- | --- | --- |
| `pdfcraft-cli text --page N`이 저장소의 0.8.0 exe에서 동작하는지 | 쪽수 근거의 기반 | 4단계 (README에는 있음) |
| 스캔 PDF 한글 OCR 품질 | 사실 카드가 비어 버림 | 4단계 |
| 새 대화를 열면 4 Argon·작업 모드가 유지되는지 | 최대 노력 보장 | 2단계 |
| Gemini가 `⟦⟧`와 `<<END:…>>`를 그대로 돌려주는지 | 근거 표시·잘림 검출 | 2단계 |
| 사용 한도·로그인 풀림 화면 문구 | 자동 일시정지 | 2단계 실기에서 수집 |
| 절 단위 + 검증 3종으로 늘어난 호출 수와 총 소요 시간 | 한 권 처리 시간 | 6·8단계 |
| 게이트 임계값(4.6) | 잠정값 | 8단계 실측 후 사용자 승인 |
| DBpia AI가 판단이 담긴 의문문에 이견 문헌까지 답하는지, 질의당 시간·사용 한도 | 최종 검수의 실효성 | 7-1단계 실기 |
| DBpia 화면·응답 구조 변경 | 수집 중단 | 매 단계 자가진단 |
| 제출처의 HWP 요구 여부 | 출력 형식 | 사용자 확인 |

---

## 부록 A. 기존 코드에서 재사용할 함수 (기준 커밋 `9c54e8f`)

| 함수 | 위치 | v2에서의 쓰임 |
| --- | --- | --- |
| `withCdpSession` | `gemini_mcp_server.js:263` | 드라이버의 CDP 세션(명령 시간 초과 60초 기존값) |
| `getCdpPort` | `:198` | T2 |
| `ensureEmbeddedGeminiWebview` | `:492` | T3, 웹뷰 대상 재탐색 |
| `ensureArgonHighAndWorkMode` | `:813` | 5.1 |
| `getGeminiStatus` | `:906` | T4 보조 |
| `resetGeminiChat` | `:967` | 대화 분리 |
| `injectTextIntoGeminiEditor` | `:977` | 입력(직접 쓰지 말고 `askGeminiWeb` 경유) |
| `askGeminiWeb` | `:2085` | 드라이버 내부 호출 |
| `hardStopGeminiAndPipeline` | `:361` | 시간 초과 시 Gemini 정지 부분 참고 |
| `getDbpiaLoginStatus` | `:2491` | T7 |
| `askDbpiaAi` / `extractLatestDbpiaAnswer` / `enrichDbpiaReferencesMetadata` | `:2988` / `:2839` / `:2765` | 문헌 수집 |
| `extractTextAndInfoWithCraft` | `:1090` | 쪽 추출 실패 시 대비 |
| `extractStructuredArchaeologyReportFacts` | `:3733` | 장 구분 규칙 |
| `buildMultiFacetedDbpiaQueries` | `:3964` | 질의 생성 |
| `parseJsonFromGeminiReply` | `:3159` | JSON 읽기 |
| `formatVerifiedCitationLine` | `:1287` | 각주 문장 |
| `evaluateArchaeologyManuscriptQuality` | `:4423` | 최종 참고 평가 |
| `exportManuscriptToWordCraftAndPdfCraft` | `:1650` | 내보내기 |
| `addPipelineLog` / `waitWhilePausedOrAborted` | `:154` / `:168` | 진행 표시·멈춤 |
| `buildComparativeChartsFromData` 외 | `archaeology_visuals_enricher.js` | 표·차트·모식도 |
| `renderDbpiaLiveBoard` / `renderManuscriptOntoA4Page` | `standalone_studio_app.js:1099` / `:1190` | 화면 표시 |

---

## 14. 개발 절차

11장의 각 단계는 아래 순서로 진행한다. 순서를 건너뛰지 않는다. 단계마다 결과물을 파일로 커밋하고, 다음 순서는 그 파일을 읽고 시작한다. 이 커밋 사슬이 "누가 무엇을 요청했고, 에이전트가 무엇을 만들었고, 누가 승인했는지"의 기록이다.

### 14.1 순서와 결과물

| 순서 | 하는 일 | 결과물 (`docs/v6_jobs/s<번호>/`) | 다음으로 넘어가는 조건 |
| --- | --- | --- | --- |
| ① 요청 | 이번 단계에서 할 일을 11장에서 옮겨 적고, 모호한 점을 질문으로 정리 | `intent.md` | 사용자가 질문에 답함 |
| ② 명세 | 완료 기준마다 확인 방법(테스트 이름 또는 실행 명령)을 연결, 바꿀 파일과 바꾸지 않을 파일 명시 | `spec.md` | 사용자 승인 |
| ③ 계획 | 파일별 작업 순서, 먼저 쓸 테스트, 위험한 지점 | `plan.md` (Antigravity Implementation Plan을 저장) | 계획에 테스트가 먼저 나옴 |
| ④ 구현 | 테스트 먼저 작성 → 실패 확인 → 최소 구현 | 코드 커밋 | 로컬 관문(14.3) 통과 |
| ⑤ 검증 | 11장 공통 검증 + 단계 완료 기준 실행, 명령·종료 코드·로그 위치 기록 | `verify.md` | 미실행 항목이 "미실행"으로 표시됨 |
| ⑥ 자기 검토 | 새 대화에서 diff만 보고 0장 규칙 위반·결함을 찾음(구현한 대화와 분리) | `review.md` | 발견 사항 처리 또는 사유 기록 |
| ⑦ 보고 | 12장 양식 보고, 승인 대상 커밋 SHA 명시 | `report.md` | 사용자 승인 |

- 결과물 폴더는 `docs/v6_jobs/`다. 루트 `factory/`는 다른 작업의 자리이므로 쓰지 않는다(0.2-6).
- `verify.md`의 로그 원문은 커밋하지 않는다. 로그는 `<작업폴더>/logs/`에 두고 경로와 요약만 적는다.
- 승인 후 코드가 바뀌면 ⑤부터 다시 하고 다시 승인받는다.

### 14.2 수정 반복 한도
- ④~⑥에서 같은 실패를 고치는 반복은 **최대 2회**다. 2회 뒤에도 남으면 멈추고 `report.md`에 `needs-human`으로 적어 사용자에게 넘긴다.
- 기존부터 있던 실패와 이번 변경으로 생긴 실패를 구분해 적는다. 기준 커밋에서 같은 명령을 실행해 비교한다.

### 14.3 강제 장치 (규칙은 문서, 강제는 기계)
Antigravity의 규칙·워크플로는 권고다. 어기면 안 되는 규칙은 git 훅과 CI로 막는다. 이렇게 하면 Antigravity가 만든 변경과 Claude가 2차로 다듬은 변경이 같은 관문을 지난다.

1. **로컬 관문** `scripts/v2/tools/check.js` (의존성 없음, Node 내장만)
   - 11장 공통 검증 4개 명령 + CI 인라인 `node -e` 검사 2개를 차례로 실행하고 하나라도 실패하면 종료 코드 1.
   - 금지 검사: 변경된 파일에 `@Google Workspace`·`@Gmail`·`@Google Drive`, `innerHTML =`, 사용자 절대경로(`C:\\Users\\`, `/home/`) 패턴이 있으면 실패.
   - 보호 경로 검사: 변경 목록에 `bin/`, `patches/`, `CLAUDE.md`, `.claude/`, `factory/`가 있으면 실패.
   - `skills/archaeo-studio-pro/SKILL.md`가 바뀌었으면 `skills_backup/` 쪽과 내용이 같은지 비교.
2. **git 훅** `.githooks/pre-commit` → `node scripts/v2/tools/check.js --staged`. 사용자가 한 번 `git config core.hooksPath .githooks`로 켠다(자동으로 켜지 않는다).
3. **CI** `.github/workflows/v6-check.yml`(새 파일): `v6/**` 브랜치 push와 PR에서 `node scripts/v2/tools/check.js`와 `node --test tests/` 실행. 기존 `archaeo-studio-ci.yml`은 고치지 않는다. CI 파일 추가는 사람 검토 대상이므로 1단계 보고에서 승인받는다.
4. 훅이 막을 때는 이유와 고칠 방법을 한 줄로 출력한다.

### 14.4 회귀 사례 묶음 (eval)
- 7장 검사(V1~V8, 7.3 재확인, 7.5 합의 규칙, 7.7 판정)는 `tests/`의 고정 사례로 둔다. 사례는 가상 유적 이름만 쓴다.
- 실기에서 잘못 통과했거나 잘못 막힌 사례가 나오면, 익명화해 `tests/fixtures/`에 사례를 하나 추가하고 기대 결과를 테스트로 고정한다. 고친 커밋과 같은 커밋에 넣는다.
- 사례 통과율이 이전보다 떨어지는 변경(프롬프트·설정 변경 포함)은 병합하지 않고 보고한다.

### 14.5 Antigravity 설정
- 터미널 명령 자동 실행은 **확인 후 실행**으로 둔다. 자동 허용 목록은 `node --check`, `node --test`, `node scripts/v2/tools/check.js`, `git status`, `git diff`, `git log`만 둔다.
- 금지 목록: `git push`, `git merge`, `git reset --hard`, `git clean`, `rm -rf`, `del /s`, `npm install`.
- 작업 공간 워크플로(`.agent/workflows/`)를 1단계에서 만든다. 각 파일은 14.1의 순서 하나를 수행한다.
  - `/v6-intent`, `/v6-spec`, `/v6-build`, `/v6-verify`, `/v6-review`, `/v6-report`
- 단계마다 별도 브랜치(`v6/s<번호>-<이름>`)와 별도 git worktree에서 작업한다. 동시에 진행하는 단계는 1개다.

### 14.6 사람이 반드시 검토하는 변경
다음이 포함되면 자기 검토 통과와 상관없이 `report.md` 맨 앞에 표시하고 사용자 승인을 받는다.
- 게이트 임계값·시간 초과 등 `config.default.json` 기본값
- 사실 카드·문헌 카드·근거 표시 등 데이터 형식(기존 저장 파일 호환)
- 원본 보고서 PDF나 사용자 파일을 덮어쓰거나 지우는 코드
- `package.json`, CI, 패키징, `bin/` 관련
- 로그인 정보·쿠키·토큰을 다루는 코드
