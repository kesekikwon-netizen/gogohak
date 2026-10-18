# 3단계 · 자료 구조·근거 저장소·표식 파서 (6장) — ② 명세 (Spec)

## 1. 생성 파일
- `scripts/v2/ids.js`
- `scripts/v2/schemas.js`
- `scripts/v2/evidence_store.js`
- `scripts/v2/markers.js`
- `tests/v2_schemas_markers.test.js`

## 2. 수정 파일
- 없음

## 3. 절대 수정하지 않는 보호 경로
- `bin/`, `patches/`, `CLAUDE.md`, `.claude/`, `factory/`, `.github/workflows/archaeo-studio-ci.yml`

## 4. 완료 기준 및 확인 방법
| 완료 기준 | 확인 명령 / 테스트 | 결과 |
| :--- | :--- | :--- |
| 6장 모든 스키마(FactCard, LiteratureCard, Outline, Claim, VerifyMatrix) 검증 | `node scripts/v2/tools/check.js` 및 `node --test tests/` | 통과 (tests/v2_schemas_markers.test.js) |
| ⟦F12,L3⟧ 및 [[F12,L3]] 파싱, F 제거 후 문장부호 정상 유지, L 등장 순 각주 1:1 매핑 | `node scripts/v2/tools/check.js` 및 `node --test tests/` | 통과 (tests/v2_schemas_markers.test.js) |
