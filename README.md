# 🏛️ ARCHAEO STUDIO 6.1 (gogohak)
**한국 고고학 발굴조사보고서 『Ⅳ. 고찰(考察)』 · 학술지 투고 『논문』 자동 집필·검증·조판 워크스테이션**  
*(하네스 감독기 × 근거 저장소(F/R/D) × Gemini 4 Argon High '작업' 모드 × DBpia AI × 독립 교차검증(V1~V8·X1~X4) × Claude 2차 인계 × WordCraft/PdfCraft 네이티브 통합)*

---

## ✨ 핵심 특징

1. **안티그라비티(Antigravity) 코딩 토큰 0% 소모 & 하네스(Supervisor) 감독 계층 (`scripts/v2/harness/`)**
   - 안티그라비티 내부의 **Google Gemini 웹 세션(`https://gemini.google.com/app`, '작업' 모드 / 4 Argon High)** 및 **DBpia AI**를 직접 구동하여 에이전트 코딩 쿼터 소모가 0%입니다.
   - 모든 외부 도구 호출(Gemini·DBpia·PdfCraft·WordCraft)을 **하네스 감독기(`supervisor.js`)**가 감싸 시간 초과·재시도·체크포인트 재개(`checkpoint.js`)·화면 구조 변화 감지(`dom_Contract.js`)·사전 자가진단(`preflight.js`)·구조화 로그(`run_logger.js`)를 보장합니다.
2. **근거 중심 9단계 파이프라인 (`P1~P9`, `scripts/v2/`) — 원자료 날조 원천 차단**
   - **쪽 보존 추출(`page_extractor.js`) & 사실표 추출(`fact_extractor.js`)**: 보고서 PDF를 쪽 단위로 분리하고 모든 유구·유물·수치·연대를 `F-0001`(근거 쪽·원문 인용 포함)로, 비교 문헌을 `R-001`로, 연대 시료를 `D-01`로 근거 저장소(`evidence_store.js`)에 등록합니다.
   - **절 단위 분할 집필(`section_writer.js`) & 근거 표지(`markers.js`)**: 모든 정량 수치 뒤에는 `[F-xxxx]`, 모든 문헌 주장 뒤에는 `[R-xxx]` 표지를 강제하며, 근거 없는 수치는 생성 단계에서 거부합니다.
3. **결정적 검사(`V1~V8`) × 독립 교차검증 대화 3종(`X1~X3`) × 합의 규칙(`X4`) × 최종 DBpia AI 문헌 검수(`P7.5`)**
   - **결정적 검사(`verify/deterministic.js`)**: 미등록 표지(`V1`), 본문 수치-사실표 대조(`V2`), 연대 날조(`V3`), 각주-참고문헌 1:1(`V4`), 표·차트·모식도 원본 일치(`V5`), 금지 구문(`V6`), 절 분량·종결어미(`V7`), 원문 문장 대조(`V8`).
   - **독립 교차검증(`verify/cross_verifier.js`)**: 집필 대화와 완전히 분리된 새 Gemini 대화 3개(`X1` 사실 대조, `X2` 인용·논지 검증, `X3` 심사위원 동료심사)를 열고 `X4` 합의 규칙(`verify/consensus.js`)으로 판정합니다.
   - **Claude 2차 다듬기 인계(`handoff.js`, `G6`) & 최종 DBpia AI 문헌 검수(`verify/dbpia_review.js`, `G7`)**: 1차 원고와 근거 묶음을 내보내고 Claude 2차 원고를 재검증(`V1~V8`)한 뒤, 최종 원고의 핵심 주장을 DBpia AI에 재질의해 학계 정설 위배(`contradicted`)나 누락 필독 문헌(`missing_must_cite`)을 최종 점검합니다.
4. **Nano Banana 최신 버전 전문가 디자이너급 고고학 모식도(`wc-diagram`) · 차트(`wc-chart`) 렌더러 (`archaeology_visuals_enricher.js`)**
   - 사실표(`F-*`)와 문헌 카드(`R-*`)에서 직접 도출된 데이터만으로 차트·모식도·비교표를 생성하며(`visuals_from_facts.js`), Studio A4 캔버스와 WordCraft(`.docx`)에 전문가 학술 출판 수준의 Nano Banana Pro 고고학 팔레트·그라데이션·드롭섀도·정밀 타이포그래피 SVG 및 OOXML 개체로 렌더링합니다.

---

## 🚀 새 컴퓨터에서 1클릭 자동 설치 및 실행 방법

1. **사전 준비**: 새 컴퓨터에 **Antigravity**와 **Node.js (v18+)**만 설치되어 있으면 됩니다.
2. **저장소 클론 및 자동 설정**:
   ```powershell
   git clone https://github.com/kesekikwon-netizen/gogohak.git
   cd gogohak
   .\새컴퓨터_자동설정.bat
   ```
3. **실행 및 검증**:
   - 바탕화면에 생성된 **`ARCHAEO_STUDIO_실행.bat`** 더블클릭 (또는 `bin\wordcraft\wordcraft-0.7.0-windows-x64-portable\wordcraft.exe` 실행)
   - 파이프라인 버전 선택: 기본 `v2` (6.1 하네스·교차검증 파이프라인) 또는 `--pipeline=v1` (`ARCHAEO_PIPELINE=v1`)으로 레거시 전환 가능
   - 로컬 검증 관문 실행: `node scripts/v2/tools/check.js` (단위 테스트 33종 + 문법 검사 + 금지 패턴 검사 + 스킬 동기화 검사)

---

## 📘 상세 설계도 문서
- **[ARCHAEO_STUDIO_6.1_Antigravity_구현명세서_하네스_교차검증.md](./ARCHAEO_STUDIO_6.1_Antigravity_구현명세서_하네스_교차검증.md)**: ARCHAEO STUDIO 6.1 전체 아키텍처, 하네스, 근거 저장소, V1~V8 / X1~X4 교차검증, Claude 2차 인계, 최종 DBpia AI 문헌 검수, 14장 개발 절차(SDLC) 명세서
- **[1_ARCHAEO_STUDIO_전체개발방법_설계도.md](./1_ARCHAEO_STUDIO_전체개발방법_설계도.md)**: 전체 아키텍처, REA 역공학 기법, Gemini×DBpia×WordCraft 연동 원리, 각주·차트·모식도·한글 폰트·6.1 하네스/교차검증 총정리
- **[2_ARCHAEO_STUDIO_타컴퓨터이식_및_향후확장_설계도.md](./2_ARCHAEO_STUDIO_타컴퓨터이식_및_향후확장_설계도.md)**: 타 PC 이식 절차 및 향후 학술 DB·도판 자동 삽입 확장 가이드
- **[docs/v6_평가결과.md](./docs/v6_평가결과.md)**: 가상 보고서 3종에 대한 `v1` 대 `v2` 정량 비교 평가 결과

