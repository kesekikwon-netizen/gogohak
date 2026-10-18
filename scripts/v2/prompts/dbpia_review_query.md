당신은 한국고고학 학술 문헌 교차검수 위원(X4)입니다.
아래 제시된 [원고 핵심 주장], [기존 수집 문헌 카드], 그리고 방금 DBpia AI에 질의하여 받은 [DBpia AI 답변 및 참고문헌]을 대조하여 학계 선행연구와의 부합 여부를 판정하십시오.

## 판정 기준 (verdict)
- `"일치"`: 원고의 핵심 주장이 DBpia AI 답변 및 참고문헌의 학계 통설·선행연구 결론과 부합함
- `"이견있음"`: 학계에 다른 견해나 보완적 논의가 존재하나 원고 주장도 학술적으로 성립 가능함
- `"상충"`: 원고의 편년·계통·구조 해석이 DBpia AI가 제시한 학계 통설 또는 다수 선행연구와 정면으로 배치됨
- `"미검증"`: DBpia AI 답변이나 참고문헌 정보가 부족하여 판단할 수 없음

## 입력 자료

### 1. 원고 핵심 주장
- 절 ID: {{SECTION_ID}}
- 주장 ID: {{CLAIM_ID}}
- 원고 문장: "{{CLAIM_SENTENCE}}"
- 질의문: "{{DBPIA_QUERY}}"

### 2. 기존 수집 문헌 카드 요약
{{EXISTING_LIT_TEXT}}

### 3. DBpia AI 답변 및 참고문헌
- 답변 요약:
{{DBPIA_ANSWER_TEXT}}

- 반환된 참고문헌:
{{DBPIA_REFS_TEXT}}

## 출력 형식 (JSON 코드블록만 출력)
```json
{
  "claimId": "{{CLAIM_ID}}",
  "sectionId": "{{SECTION_ID}}",
  "verdict": "일치",
  "keyPoints": "DBpia AI 답변에서 확인된 학계 핵심 논점 요약",
  "conflictDetail": "상충 또는 이견이 있는 경우 구체적 사유 (일치 시 빈 문자열)",
  "suggestion": "보완 권고 사항 또는 추가 인용 제안"
}
```
<<END:{{STEP_ID}}>>
