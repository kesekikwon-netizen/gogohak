# LogPanel

진행 기록. 위에 진행 막대, 아래에 시각 · 종류 · 한 문장.

- 받는 것: `title`(단계 제목과 겹치면 `false`), `progress`, `progressLabel`, `entries`(`{time, kind, text}`), `empty`.
- `kind`: `collect` 수집(황동) · `write` 작성(세이지) · `note` 알림 · `warn` 확인(갈색) · `error` 오류(붉은 글자) · `done` 완료(ok).
- 글은 한 일을 한 문장으로: 「논문 12건을 찾았습니다」, 「연대분석 자료가 비어 있어 연대 수치는 쓰지 않습니다」. 빠진 자료를 어떻게 다뤘는지 꼭 남깁니다.
- 비어 있으면 「실행하면 진행 기록이 여기에 쌓입니다.」
