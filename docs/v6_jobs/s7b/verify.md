# 7-1단계 · 최종 DBpia AI 문헌 검수 (P7.5 · X4 · G7) — ⑤ 검증 (Verify)

- 실행 명령:
  1. `node --check scripts/gemini_mcp_server.js` → 종료 코드 `0`
  2. `node --check scripts/standalone_studio_app.js` → 종료 코드 `0`
  3. `node --check scripts/archaeology_visuals_enricher.js` → 종료 코드 `0`
  4. `node --test tests/` → 종료 코드 `0` (전체 통과)
  5. `node scripts/v2/tools/check.js` → 종료 코드 `0`
- 미실행 항목: 실기 CDP 브리지 라이브 웹뷰 왕복 호출(`--selftest` 실기 세션은 Antigravity CDP 포트 활성 환경에서 실행).
