# 🚀 ARCHAEO STUDIO 5.0 — 타 컴퓨터 이식 및 향후 확장 설계도

---

## 1. 다른 컴퓨터(새 PC)로 옮길 때 무엇이 필요한가?

### 1.1 필수 프로그램 (단 2가지)
1. **Antigravity (안티그라비티 PC 앱)**: 내부에 Google Gemini 웹 세션(`127.0.0.1:9223`)을 품고 있는 호스트 앱.
2. **Node.js (v18 이상)**: 로컬 브리지 서버(`gemini_mcp_server.js`)를 구동하는 런타임.
   *(※ Rust 컴파일러나 별도의 MS Word가 없어도 `bin/wordcraft/wordcraft.exe`, `wordcraft-cli.exe`, `pdfcraft.exe` 포터블 실행파일이 모두 포함되어 있어 즉시 작동합니다!)*

### 1.2 새 컴퓨터에서 1분 만에 끝내는 3단계 설치법
1. 새 컴퓨터에서 GitHub 저장소(`https://github.com/kesekikwon-netizen/gogohak.git`)를 클론하거나 다운로드합니다:
   ```powershell
   git clone https://github.com/kesekikwon-netizen/gogohak.git
   ```
2. 폴더 안의 **`새컴퓨터_자동설정.bat`** 파일을 더블클릭하여 실행합니다.
   - 이 배치 파일이 자동으로:
     - `%USERPROFILE%\.gemini\config\plugins\gemini-desktop` 경로에 플러그인·디자인·WordCraft/PdfCraft 포터블 바이너리를 복사하고,
     - `%USERPROFILE%\.gemini\config\skills\archaeo-studio-pro` 및 `rea-gemini-app-bridge` 스킬을 등록하며,
     - `%USERPROFILE%\.gemini\antigravity\mcp_config.json`에 `gemini-desktop` MCP 서버를 자동 등록하고,
     - 바탕화면에 **`ARCHAEO STUDIO 실행.bat`** 및 **`WordCraft (ARCHAEO 내장).lnk`** 바로가기를 생성합니다.
3. 안티그라비티 앱을 켜고 우측 하단 **`✨ Gemini`** 패널에서 본인의 구글 계정으로 1회 로그인(이후 자동 유지)한 뒤, 바탕화면의 **`ARCHAEO STUDIO 실행`**을 더블클릭하면 즉시 똑같은 환경이 열립니다.

---

## 2. 사용 방식 2가지 (원하는 방식 선택 가능)

### 방식 A: `WordCraft` 네이티브 데스크톱 앱 단독 실행 (좌측 ARCHAEO STUDIO + 우측 WordCraft A4 페이지)
- `bin\wordcraft\wordcraft-0.7.0-windows-x64-portable\wordcraft.exe`를 실행하면:
  - **좌측 도킹 패널(`ARCHAEO STUDIO`)**: Step 01~06 조사개요 입력, 1차 가공, DBpia 선행연구 수집, Gemini '작업' 모드 고찰/논문 집필 버튼 제공.
  - **우측 메인 캔버스(`WordCraft`)**: 생성된 `.docx` 원고(표 1·2, 반괄호 하단 각주 `1) 2)`, 네이티브 차트·모식도, `【참고문헌】`)가 즉시 열려 실시간 편집 및 `[PDF로 내보내기]` 가능.
  - 상단 타이틀바의 **`ARCHAEO STUDIO`** 버튼으로 좌측 스튜디오 패널을 언제든 열고 닫을 수 있습니다.

### 방식 B: 웹 스튜디오(`http://127.0.0.1:18787/studio`) + 우측 WordCraft 연동
- `ARCHAEOLOGY_STUDIO_실행.bat`를 실행하면 전용 Edge/Chrome 앱 모드로 **ARCHAEO STUDIO** 창이 열립니다.
- 업로드된 테라코타·한지 학술 디자인(`tokens.css`, `bundle.css`)이 100% 적용된 스튜디오에서 Step 01~06을 진행하고, **[WordCraft로 열기]** 또는 **[PDF로 내보내기]** 버튼을 누르면 즉시 네이티브 `WordCraft` / `PdfCraft`로 연결됩니다.

---

## 3. 향후 다른 앱·사이트를 추가 결합할 때의 확장 설계도

### 3.1 새로운 학술 DB 사이트 추가 (예: RISS, KCI, 국립문화유산연구원 문화유산지식e음)
1. `gemini_mcp_server.js`의 `openOrGetDbpiaTarget()` 함수를 복제하여 타깃 URL만 변경(예: `https://www.kci.go.kr` 또는 `https://portal.nrich.go.kr`).
2. CDP `Runtime.evaluate`로 검색창 셀렉터와 결과 목록 셀렉터를 지정하여 서지사항 배열(`[{ title, authors, journal, year, pages }]`)을 반환.
3. `buildAdaptiveMasterPrompt()`의 `[DBpia 학술논문 교차검증 지침]` 섹션에 수집된 서지사항을 합산 주입하면, Gemini가 본문 각주 `1)`, `2)`와 `【참고문헌】`에 자동 반영합니다.

### 3.2 WordCraft 차트·모식도 종류 확장
- 현재 지원되는 `wc-chart`(`column`, `bar`, `line`, `pie`, `scatter`) 및 `wc-diagram`(`process`, `hierarchy`, `matrix`, `cycle`, `basicList`) 외에, `wordcraft-cli`의 추가 명령(`insert.table`, `insert.picture`, `insert.shape`, `insert.equation`)을 `exportManuscriptToWordCraftAndPdfCraft()`의 토큰 치환 루프에 그대로 추가할 수 있습니다.
- 예: 유물 실측도·도면 이미지를 폴더에 넣으면 `[[WC_IMAGE_1]]` 토큰을 통해 `insert.picture={"path":"..."}`로 도판(Figure)까지 본문 적재적소에 자동 삽입하도록 확장 가능합니다.
