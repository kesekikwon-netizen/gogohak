# PromptField

무엇을 쓸지 적는 「요청」 칸 + 1차 가공 안내 + 가공된 요청 보기.

- 받는 것: `value`/`defaultValue`, `onChange`, `placeholder`, `rows`, `processed`(가공된 요청 글), `showProcessed`/`onToggleProcessed`.
- 안내 문구는 그대로: 「입력하신 요청은 고고학 학술 양식에 맞춰 1차 가공되어 전달됩니다.」
- 가공된 요청은 접어 두고, 「▸ 가공된 요청 보기」로 펼칩니다. 고치는 곳이 아니라 확인하는 곳입니다.
