# 6단계 · 절별 집필·결정적 검사(V1~V8)·독립 교차검증(X1~X3)·조건부 시각자료(Nano Banana Pro)·조립 — ② 명세 (Spec)

## 1. 생성 파일
- `scripts/v2/prompts/section_report.md`
- `scripts/v2/prompts/section_paper.md`
- `scripts/v2/prompts/check_fact.md`
- `scripts/v2/prompts/check_citation.md`
- `scripts/v2/prompts/critic_report.md`
- `scripts/v2/prompts/critic_paper.md`
- `scripts/v2/section_writer.js`
- `scripts/v2/verify/claim_splitter.js`
- `scripts/v2/verify/deterministic.js`
- `scripts/v2/verify/fact_checker.js`
- `scripts/v2/verify/citation_checker.js`
- `scripts/v2/verify/critic.js`
- `scripts/v2/verify/consensus.js`
- `scripts/v2/crosswalk.js`
- `scripts/v2/assembler.js`
- `scripts/v2/visuals_from_facts.js`
- `scripts/v2/index.js`
- `tests/fixtures/virtual_manuscript.md`
- `tests/v2_verify_pipeline.test.js`

## 2. 수정 파일
- `scripts/archaeology_visuals_enricher.js (Nano Banana Pro 고고학 SVG 렌더러 추가)`
- `scripts/gemini_mcp_server.js (/v2/* 엔드포인트 연결)`
- `scripts/standalone_studio_app.js (v1/v2 엔진 선택 및 제어 UI 연결)`

## 3. 절대 수정하지 않는 보호 경로
- `bin/`, `patches/`, `CLAUDE.md`, `.claude/`, `factory/`, `.github/workflows/archaeo-studio-ci.yml`

## 4. 완료 기준 및 확인 방법
| 완료 기준 | 확인 명령 / 테스트 | 결과 |
| :--- | :--- | :--- |
| V1~V8 결정적 검사 전 항목 통과 및 위반 검출 | `node scripts/v2/tools/check.js` 및 `node --test tests/` | 통과 (tests/v2_verify_pipeline.test.js) |
| 7.3 검증인의 인용 구절 역검증 실패 시 검증불가 강제 전환 | `node scripts/v2/tools/check.js` 및 `node --test tests/` | 통과 (tests/v2_verify_pipeline.test.js) |
| 7.5 합의 규칙 표의 모든 행 판정 일치 및 2회 재작성 후 〔확인 필요〕 부착 | `node scripts/v2/tools/check.js` 및 `node --test tests/` | 통과 (tests/v2_verify_pipeline.test.js) |
| Nano Banana Pro 고고학 도면·차트·모식도 생성 및 E2E 파이프라인 완주 | `node scripts/v2/tools/check.js` 및 `node --test tests/` | 통과 (tests/v2_verify_pipeline.test.js) |
