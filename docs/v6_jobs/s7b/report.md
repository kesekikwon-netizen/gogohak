# 7-1단계 · 최종 DBpia AI 문헌 검수 (P7.5 · X4 · G7) — ⑦ 단계 보고서 (12장 · 14.6 양식)

## 14.6 사람 검토 대상 변경 여부
- G7 최종 문헌 검수 게이트 (14.6 검토 대상)

## 12장 보고 항목
- **브랜치**: `v6/s7b-dbpia-review`
- **커밋 SHA**: `ecefa47`
- **만든 파일**: scripts/v2/prompts/dbpia_review_query.md, scripts/v2/verify/dbpia_review.js, tests/v2_dbpia_review.test.js
- **바꾼 파일**: 없음
- **검증 명령 및 종료 코드**: `node scripts/v2/tools/check.js` (`0`), `node --test tests/` (`0`)

### 완료 기준 대조표
| 완료 기준 | 판정 |
| :--- | :--- |
| 핵심 주장 최대 5건 선별 및 당해 유적 고유명·유구 번호 제거 질의 생성 | 통과 (tests/v2_dbpia_review.test.js) |
| 학계 통설과 상충 시 G7 중단, 신규 문헌(L_new1) 추출, 사용자 선택 전 원고 자동 수정 금지 | 통과 (tests/v2_dbpia_review.test.js) |
