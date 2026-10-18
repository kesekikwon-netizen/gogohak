# DocPage

A4 용지 한 장(210×297mm). 흰 종이(`page`)에 먹색(`page-ink`), 바탕체 계열. 발굴조사보고서 『Ⅳ. 고찰』 양식.

- 받는 것: `DocPage`(`footnotes`, `pageNumber`, `empty`, `scale`) 안에 `DocChapter`, `DocSection`, `DocPara`(`live`면 흙색 작성 커서), `FootnoteRef`(`n`), `DocTable`(`caption`, `columns`, `rows`), `References`(`title`, `items`).
- 본문 인용은 위첨자 `1)`. 쪽 아래 각주: `1) 저자명, 발행연도, 「논문 제목」, 『학술지명』 권호, 발행기관, 인용쪽(00~00쪽).`
- 문서 끝 `【참고문헌】`, 둘째 줄부터 내어쓰기. 표는 위에 「표 1. …」.
- 실행 전에는 `empty`로 「실행하면 이 자리에 『Ⅳ. 고찰』이 작성됩니다. 직접 써 넣으셔도 됩니다.」
- 글자 크기는 제안값입니다(README의 「A4 문서」).
