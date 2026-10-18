# 6단계 · 절별 집필·결정적 검사(V1~V8)·독립 교차검증(X1~X3)·조건부 시각자료(Nano Banana Pro)·조립 — ⑤ 검증 (Verify)

- 실행 명령:
  1. `node --check scripts/gemini_mcp_server.js` → 종료 코드 `0`
  2. `node --check scripts/standalone_studio_app.js` → 종료 코드 `0`
  3. `node --check scripts/archaeology_visuals_enricher.js` → 종료 코드 `0`
  4. `node --test tests/` → 종료 코드 `0` (전체 통과)
  5. `node scripts/v2/tools/check.js` → 종료 코드 `0`
- 미실행 항목: 실기 CDP 브리지 라이브 웹뷰 왕복 호출(`--selftest` 실기 세션은 Antigravity CDP 포트 활성 환경에서 실행).
