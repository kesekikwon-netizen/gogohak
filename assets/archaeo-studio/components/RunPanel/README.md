# RunPanel

실행 단추 둘(발굴조사보고서 고찰 실행 · 논문 실행), 작성 중 보조 단추(잠시 멈춤 · 그만두기), 디비피아 로그인, PDF로 내보내기.

- 받는 것: `mode`(`report` · `paper`), `state`(`idle` · `running` · `paused` · `done`), `loggedIn`, `onRun(mode)`, `onPause`, `onStop`, `onLogin`, `onExport`.
- 고른 문서 종류의 실행 단추만 흙색. 작성 중에는 두 실행 단추 모두 누를 수 없고, 그 아래 잠시 멈춤·그만두기가 나타납니다.
- 「PDF로 내보내기」는 작성 전후 언제나 있지만, 작성이 끝나면 `quiet`에서 `default`로 한 단계 올라옵니다.
- 디비피아 로그인 상태는 태그(로그인 필요 · 로그인됨)로 늘 보입니다.
