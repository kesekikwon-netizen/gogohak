# 3단계 · 자료 구조·근거 저장소·표식 파서 (6장) — ⑦ 단계 보고서 (12장 · 14.6 양식)

## 14.6 사람 검토 대상 변경 여부
- 사실 카드·문헌 카드·근거 표시 데이터 형식 정의 (14.6 검토 대상)

## 12장 보고 항목
- **브랜치**: `v6/s3-schemas`
- **커밋 SHA**: `65fd1b0`
- **만든 파일**: scripts/v2/ids.js, scripts/v2/schemas.js, scripts/v2/evidence_store.js, scripts/v2/markers.js, tests/v2_schemas_markers.test.js
- **바꾼 파일**: 없음
- **검증 명령 및 종료 코드**: `node scripts/v2/tools/check.js` (`0`), `node --test tests/` (`0`)

### 완료 기준 대조표
| 완료 기준 | 판정 |
| :--- | :--- |
| 6장 모든 스키마(FactCard, LiteratureCard, Outline, Claim, VerifyMatrix) 검증 | 통과 (tests/v2_schemas_markers.test.js) |
| ⟦F12,L3⟧ 및 [[F12,L3]] 파싱, F 제거 후 문장부호 정상 유지, L 등장 순 각주 1:1 매핑 | 통과 (tests/v2_schemas_markers.test.js) |
