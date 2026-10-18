# 🏛️ ARCHAEO STUDIO 5.0 (gogohak)
**한국 고고학 발굴조사보고서 『Ⅳ. 고찰(考察)』 · 학술지 투고 『논문』 자동 집필·조판 워크스테이션**  
*(WordCraft 네이티브 통합 × Google Gemini '작업' 모드 × DBpia AI × PdfCraft 필요시 변환)*

---

## ✨ 핵심 특징

1. **안티그라비티(Antigravity) 코딩 토큰 0% 소모**
   - 안티그라비티 내부의 **Google Gemini 웹 세션(`https://gemini.google.com/app`, '작업' 모드 / 4 Argon High)** 및 **DBpia AI**를 직접 구동하여 수백 쪽의 고찰·논문을 작성해도 에이전트 코딩 쿼터가 전혀 줄어들지 않습니다.
2. **좌측 ARCHAEO STUDIO (Step 01~06) × 우측 WordCraft(`.docx`) 일체형 작업**
   - **방식 A (네이티브 통합)**: `WordCraft`(`wordcraft.exe`)를 켜면 좌측에 **ARCHAEO STUDIO 도킹 패널**이 바로 열리고, 우측 A4 캔버스에서 실시간으로 `.docx` 원고·표·각주·차트·모식도를 편집합니다.
   - **방식 B (전용 웹 스튜디오)**: `ARCHAEOLOGY_STUDIO_실행.bat` 실행 시 테라코타·한지 학술 테마가 적용된 풀스크린 스튜디오(`http://127.0.0.1:18787/studio`)와 `WordCraft`가 연동됩니다.
   - **PDF 변환(`PdfCraft`)**: 평소 모든 작업은 `WordCraft(.docx)` 중심으로 진행하며, 완성 후 **[PDF로 내보내기]** 버튼을 누를 때만 PDF로 변환됩니다.
3. **고고학 발굴조사보고서 『Ⅳ. 고찰』 · 『논문』 6대 핵심 품질 보증**
   - **1차 가공(프롬프트 엔지니어링)**: 조사개요·유구·유물 메모를 고고학 논증 구조 지시문으로 1차 가공 후 검토 가능
   - **없는 자료 날조 절대 금지(`없는 자료는 없는 대로`)**: 절대연대(AMS)·자연과학분석 자료가 없으면 임의 수치를 지어내지 않고 상대편년·형식학 논증에 집중
   - **유구 중복관계·층위 선후관계 최우선 논증**: 물리적 파괴·중복 및 층위 관계를 제1절 편년의 최우선 근거로 서술
   - **필수 학술 표 2종 자동 편성**: `<표 1> 주요 유구·출토유물 속성 일람표`, `<표 2> 주변 병행유적 비교 대비표`
   - **Gemini 직접 생성 네이티브 차트(`wc-chart`) & 분기·계통 모식도(`wc-diagram`)**: `.docx` 내부에 편집 가능한 진짜 OOXML 차트·다이어그램 개체로 자동 삽입
   - **한국 고고학 표준 반괄호 하단 각주 `1)`, `2)` & `【참고문헌】` 1:1 검증**: 본문 인용 각주를 페이지 하단 분리선 아래 진짜 각주로 변환하고 `『보고서/학술지』`, `「논문」`, `00~00쪽` 규격 및 1:1 일치 검증 수행
4. **한글 폰트 깨짐(`□□□`) 완벽 해결**
   - `.docx` 테마 폰트(`Malgun Gothic`) 자동 고정 및 `WordCraft` / `PdfCraft` 시스템 한글 폰트 폴백 탑재 완료.

---

## 🚀 새 컴퓨터에서 1클릭 자동 설치 및 실행 방법

1. **사전 준비**: 새 컴퓨터에 **Antigravity**와 **Node.js (v18+)**만 설치되어 있으면 됩니다.
2. **저장소 클론 및 자동 설정**:
   ```powershell
   git clone https://github.com/kesekikwon-netizen/gogohak.git
   cd gogohak
   .\새컴퓨터_자동설정.bat
   ```
3. **실행**:
   - 바탕화면에 생성된 **`ARCHAEO_STUDIO_실행.bat`** 더블클릭 (또는 `bin\wordcraft\wordcraft-0.7.0-windows-x64-portable\wordcraft.exe` 실행)

---

## 📘 상세 설계도 문서
- **[1_ARCHAEO_STUDIO_전체개발방법_설계도.md](./1_ARCHAEO_STUDIO_전체개발방법_설계도.md)**: 전체 아키텍처, REA 역공학 기법, Gemini×DBpia×WordCraft 연동 원리, 각주·차트·모식도·한글 폰트 해결 방법 총정리
- **[2_ARCHAEO_STUDIO_타컴퓨터이식_및_향후확장_설계도.md](./2_ARCHAEO_STUDIO_타컴퓨터이식_및_향후확장_설계도.md)**: 타 PC 이식 절차 및 향후 학술 DB·도판 자동 삽입 확장 가이드
