# 8단계 · v1 대비 v2 정량 평가 및 임계값 검증 — ② 명세 (Spec)

## 1. 생성 파일
- `docs/v6_평가결과.md`

## 2. 수정 파일
- 없음

## 3. 절대 수정하지 않는 보호 경로
- `bin/`, `patches/`, `CLAUDE.md`, `.claude/`, `factory/`, `.github/workflows/archaeo-studio-ci.yml`

## 4. 완료 기준 및 확인 방법
| 완료 기준 | 확인 명령 / 테스트 | 결과 |
| :--- | :--- | :--- |
| 1.2 5대 정량 품질 지표(근거 없는 수치 비율 0%, 미검증 인용 비율 0%, 각주-참고문헌 불일치 0건, 절별 분량 균형 >=0.35, 검증 오버헤드 <=60%) 달성 기록 | `node scripts/v2/tools/check.js` 및 `node --test tests/` | 통과 (docs/v6_평가결과.md) |
