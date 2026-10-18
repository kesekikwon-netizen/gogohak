# 🏛️ ARCHAEO STUDIO 5.0 — 전체 개발 방법 및 아키텍처 마스터 설계도
**(발굴조사보고서 『Ⅳ. 고찰』 · 학술논문 『논문』 × WordCraft 네이티브 통합 × Gemini '작업' 모드 × DBpia AI)**

---

## 1. 시스템 핵심 철학 및 작동 원리

### 1.1 왜 안티그라비티(Antigravity) 코딩 토큰이 소모되지 않는가?
- **안티그라비티 에이전트 쿼터 0% 소모**: 본 워크스테이션은 안티그라비티 에이전트 API를 호출하지 않습니다.
- 대신 안티그라비티 PC 앱(`127.0.0.1:9223` CDP 포트) 내부에 임베딩된 **Google Gemini 웹 세션(`https://gemini.google.com/app`, '작업' 모드 / 4 Argon High)** 및 **DBpia AI 웹뷰**에 로컬 브리지 서버(`gemini_mcp_server.js --bridge-only`, 포트 `18787`)가 직접 접속하여 프롬프트를 주입하고 응답을 추출합니다.
- 따라서 수십 편, 수백 쪽의 고고학 발굴조사보고서 『Ⅳ. 고찰』 및 학술지 투고 『논문』을 반복 생성해도 **안티그라비티 코딩 쿼터는 단 1%도 깎이지 않습니다.**

### 1.2 WordCraft 중심 단일 편집 · 필요시 PDF 변환 (PdfCraft)
- 모든 집필·편집·표·차트·모식도·각주 작업은 **오직 WordCraft(`.docx`)**를 중심으로 이루어집니다.
- `PdfCraft`(`pdfcraft.exe`)는 고찰/논문 작성 단계에서는 관여하지 않으며, 사용자가 최종 완성본을 **[PDF로 내보내기]** 클릭했을 때만 호출됩니다.

---

## 2. REA(Reverse Engineer Anything)를 활용한 역공학 및 통합 개발 방법

### 2.1 REA 정적·동적 분석 워크플로우
1. **웹사이트(Gemini 웹 / DBpia AI) DOM & 이벤트 리스너 역공학**:
   - CDP(`http://127.0.0.1:9223/json/list`)로 타깃 탭의 WebSocket 디버거 URL을 획득.
   - `Runtime.evaluate`를 통해 Gemini의 모드 스위처(`작업` 모드 / `빠른 답변` / `사고` / `Pro`) DOM 트리, 프롬프트 에디터(`div[contenteditable="true"][role="textbox"]`), 전송 버튼(`button[aria-label*="보내기"]`), 응답 컨테이너(`message-content`, `model-response-text`)를 정밀 매핑.
   - DBpia AI(`https://www.dbpia.co.kr`)의 AI 검색 입력창 및 참고문헌 카드 리스트(`a[href*="nodeId"]`, 서지정보 블록)를 추출하여 실제 국내 학술논문·발굴조사보고서 서지사항을 수집.
2. **네이티브 앱(`WordCraft` / `PdfCraft`) 바이너리 및 소스 역공학**:
   - `wordcraft-cli.exe --help-json` 및 `crates/engine`, `crates/docx`, `crates/ui-egui` 분석을 통해 OOXML 조작 명령 세트를 완전 해독:
     - `design.themeFonts`: `{"heading": "Malgun Gothic", "body": "Malgun Gothic"}` → 한글 자모 깨짐(`□□□`) 원천 차단
     - `references.footnote`: `{"text": "저자(연도), 『보고서명』, 쪽수.", "mark": "1)"}` → 한국 고고학 표준 반괄호(`1)`, `2)`) 하단 각주 생성
     - `insert.chart`: `{"kind": "column|bar|line|pie", "title": "...", "categories": [...], "series": [...]}` → 편집 가능한 네이티브 OOXML 차트 삽입
     - `insert.smartArt`: `{"layout": "process|hierarchy|cycle", "items": [...]}` → 분기·편년·계통 모식도 삽입
   - `WordCraft` UI(`crates/ui-egui/src/archaeo_studio.rs`)에 **ARCHAEO STUDIO 좌측 도킹 패널**을 내장하여, 좌측 6단계 고찰 스튜디오에서 집필한 원고가 우측 WordCraft A4 페이지에 즉시 열리도록 직결.

---

## 3. 한국 고고학 발굴조사보고서 『Ⅳ. 고찰』 & 『논문』 6대 핵심 품질 엔진

### ① 1차 가공 (프롬프트 엔지니어링 자동 설계)
- 사용자가 입력한 유적명·유구 현황·출토유물·절대연대 메모를 먼저 학술 논증 구조로 1차 가공(`POST /first-stage-prompt`, `buildFirstStageEngineeredPrompt()`)하여 조사자가 집필 전에 논증 방향을 검토·보완할 수 있게 합니다.

### ② 없는 자료 날조 절대 금지 원칙 (`없는 자료는 없는 대로`)
- 방사성탄소연대(AMS), 자연과학분석, 특정 유구 측정치 등 사용자가 입력하지 않은 데이터는 **절대 임의의 수치(예: 가짜 BP 연대, 가짜 비율)로 지어내지 않습니다.**
- 해당 자료가 없을 경우 *"현재까지 자연과학적 절대연대 측정 자료가 확보되지 않았으므로, 유구 중복관계와 출토 유물의 형식학적 상대편년을 중심으로 검토한다"*라고 명시하고 상대편년·층위 논증에 집중합니다.

### ③ 유구 중복관계 · 층위 선후관계 최우선 논증
- 고고학 편년의 가장 확실한 물리적 근거인 **유구 간 파괴·중복관계(선행 유구 → 후행 유구)** 및 **퇴적 층위(Stratigraphy) 선후관계**를 제1절에서 최우선으로 분석하여 분기(Phase) 설정의 뼈대로 삼습니다.

### ④ 필수 학술 표 2종 자동 편성
- **`<표 1> 주요 유구·출토유물 속성 일람표`**: 유구번호, 유구유형, 평면형태, 규모(길이×너비×깊이), 주축방향, 주요 출토유물, 비고(중복관계·단계).
- **`<표 2> 주변 병행유적 비교 대비표`**: 유적명, 입지, 중심묘제/주거유형, 대표 표지유물, 중심연대(단계), 본 유적과의 관계.

### ⑤ Gemini 직접 생성 네이티브 차트(`wc-chart`) & 모식도(`wc-diagram`)
- 별도 외부 앱 없이 Gemini가 본문 맥락에 맞추어 `wc-chart`(시대별/유구별 출토 비율·속성 분포 차트)와 `wc-diagram`(1기→2기→3기 분기 변천 및 공간 위계 모식도) JSON 블록을 직접 생성합니다.
- 브리지의 `exportManuscriptToWordCraftAndPdfCraft()`가 이를 파싱하여:
  - 웹 스튜디오 A4 프리뷰에서는 고해상도 인터랙티브 SVG 학술 차트·모식도로 렌더링하고,
  - `WordCraft(.docx)`에서는 진짜 OOXML `insert.chart` 및 `insert.smartArt` 개체로 변환·삽입합니다.

### ⑥ 한국 고고학 표준 반괄호 각주 `1)`, `2)` & `【참고문헌】` 1:1 검증
- 본문 내 인용은 한국고고학보·문화재청 발굴조사보고서 규격인 `1)`, `2)`, `3)` 반괄호 각주를 사용합니다.
- 소제목 번호(`1) 토기류`, `2) 철기류`)와 본문 인용 각주(`...확인된다1).`, `...비정된다2) 참조.`)를 정규식으로 엄격히 구분하여, 본문 한 줄에 여러 개의 각주가 있어도 하나도 빠짐없이 `[[WC_FN_n]]` 토큰으로 치환 후 `select.text` + `references.footnote`로 페이지 하단 분리선 아래에 배치합니다.
- 말미 `【참고문헌】`은 `『단행본·보고서·학술지』`, `「학술논문」`, `00~00쪽` 부호를 준수하며, 본문 각주 인용 문헌과 `【참고문헌】` 목록의 **1:1 일치 여부**를 자동 검증합니다.

---

## 4. 한글 폰트 깨짐(`□□□`) 완벽 해결 구조

1. **`.docx` 문서 내부 테마 폰트 고정**:
   - `wordcraft-cli` 실행 시 첫 명령으로 `design.themeFonts={"heading":"Malgun Gothic","body":"Malgun Gothic"}`를 주입하여 제목·본문·표·각주·차트·모식도 전체가 `맑은 고딕(Malgun Gothic)`을 기본 폰트로 사용하도록 강제합니다.
2. **`WordCraft` 네이티브 앱 한글 폴백 탑재 (`crates/ui-egui/src/theme.rs`, `crates/fonts/src/fontdb.rs`)**:
   - Windows의 `C:\Windows\Fonts\malgun.ttf`, `malgunbd.ttf`, `batang.ttc`를 시스템 CJK 폴백으로 즉시 로드하여 UI 메뉴·좌측 스튜디오 패널·캔버스 어디에서도 `□□□`가 발생하지 않습니다.
3. **`PdfCraft` 네이티브 앱 한글 폴백 탑재 (`crates/ui-egui/src/system_fonts.rs`, `theme.rs`)**:
   - `korean_fallback()`(`SYSTEM_KOREAN_FALLBACK`)을 모든 폰트 패밀리에 등록하여 PDF 변환 및 뷰어 UI에서도 한글이 100% 정상 표시됩니다.

---

## 5. 디렉터리 및 파일 구조

```text
gemini-desktop/
├── 1_ARCHAEO_STUDIO_전체개발방법_설계도.md       # 본 마스터 개발 설계도
├── 2_ARCHAEO_STUDIO_타컴퓨터이식_및_향후확장_설계도.md # 타 PC 이식 및 확장 설계도
├── 새컴퓨터_자동설정.bat                          # 새 PC 1클릭 자동 설치 스크립트
├── ARCHAEOLOGY_STUDIO_실행.bat                   # ARCHAEO STUDIO + 브리지 1클릭 실행기
├── assets/
│   └── archaeo-studio/                           # ARCHAEO STUDIO 디자인 시스템 (tokens.css, bundle.css 등)
├── scripts/
│   ├── gemini_mcp_server.js                      # Gemini '작업' 모드 × DBpia × WordCraft 조판 엔진
│   └── standalone_studio_app.js                  # ARCHAEO STUDIO 6단계 UI & A4 라이브 에디터
├── bin/
│   ├── wordcraft/wordcraft-0.7.0-windows-x64-portable/
│   │   ├── wordcraft.exe                         # ARCHAEO STUDIO 좌측 패널 내장 + 한글 패치 완료
│   │   └── wordcraft-cli.exe                     # OOXML 표·각주·차트·모식도 CLI 엔진
│   └── pdfcraft/pdfcraft-0.8.0-windows-x64-portable/
│       ├── pdfcraft.exe                          # 한글 UI 폰트 패치 완료
│       └── pdfcraft-cli.exe                      # PDF 변환 CLI 엔진
└── patches/
    ├── wordcraft/                                # WordCraft Rust 소스 패치 파일 원본
    └── pdfcraft/                                 # PdfCraft Rust 소스 패치 파일 원본
```
