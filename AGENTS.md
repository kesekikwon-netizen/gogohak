# Strata (ka-hgis) — Antigravity Workspace Rules

이 저장소의 기본 지침인 `CLAUDE.md`와 함께 아래의 필수 실행·검증·도메인 규칙을 항상 준수한다.

## 1. 실행 및 빌드 환경 가드레일 (`Qt6Xml.dll` 모달 오류 방지)
- **절대 `build/Release/ka-hgis.exe`를 환경 설정 없이 직접 실행하지 않는다.**
  - OSGeo4W/Qt6 경로 없이 `build/Release/ka-hgis.exe`를 호출하면 사용자 화면에 `Qt6Xml.dll이(가) 없어 코드 실행을 진행할 수 없습니다.` 모달 오류 창이 뜨고 프로세스가 멈춘다.
  - 빌드 폴더의 실행 파일을 스모크 테스트할 때는 반드시 같은 프로세스 안에서 `. ./scripts/dev-env.ps1`을 먼저 로드한 뒤 실행하거나, 모든 DLL이 동봉된 포터블 폴더(`WorkingDirectory`를 해당 포터블 폴더로 지정)에서 실행한다.
  - 사용자용 앱 실행은 `wscript.exe scripts/start-ka-hgis.vbs` 또는 포터블 폴더의 `ka-hgis.exe`를 사용한다.
- **CMake 빌드 시 PATH 설정 필수**:
  - 커밋 후 `HEAD` 해시 변경으로 CMake가 재구성될 때 `AUTORCC`(`D:/OSGeo4W/apps/Qt6/bin/rcc.exe`)가 실행되므로, `cmake --build` 호출 전에도 반드시 `. ./scripts/dev-env.ps1` (또는 `D:\OSGeo4W\apps\Qt6\bin;D:\OSGeo4W\apps\qgis-dev\bin;D:\OSGeo4W\bin` PATH 추가)을 먼저 수행한다.

## 2. 공간 연산 및 레이어 표시 규칙
- **주변 거리 경계(`500m`/`1000m`) 및 반경 내 유적 추출**:
  - `BufferAnalysis::addDistanceRing` 및 `NearbyHeritageExport` 등 주변 반경 계산은 모서리 대각선 거리가 $\sqrt{2}$배(`707~710m`)로 늘어나고 미세 변이 왜곡되는 `JoinStyle::Miter`를 사용하지 않는다.
  - 항상 모든 방향(대각선 모서리 포함)에서 실제 거리가 정확히 일치하는 **`Qgis::JoinStyle::Round` (36 segments)** 및 유클리드 최단거리(`sitePoly.distance(g) <= radius`)를 사용한다.
- **숨겨진 도메인 레이어(`survey_area` 등) 편집 시 가시성 복구**:
  - 기존 조사 파일(`*.gpkg`)에 `survey_area`(조사구역) 등 도메인 레이어가 이미 존재하지만 레이어 트리에서 체크 해제(`!node->isVisible()`)되어 있을 수 있다.
  - 조사구역 이어그리기·대지 경계 생성·도형 추가 시에는 반드시 레이어 트리 노드의 `setItemVisibilityChecked(true)`를 호출하고 캔버스를 갱신하여 즉시 화면과 목록에 보이게 한다.
- **사용자 원본 조사 파일 보호**:
  - 사용자가 작업 중인 외부 `*.gpkg`/`*.qgz` 조사 파일은 문제 분석 시 읽기 전용으로만 확인하며 절대 직접 수정하거나 덮어쓰지 않는다.

## 3. 전역 `clang-format` 훅 및 300줄 제한(`scorecard.ps1 -LineLimitOnly`) 대응
- 이 PC에는 파일 편집 도구(`write_to_file`, `replace_file_content`) 실행 후 파일 전체에 `clang-format -i`를 자동 실행하는 전역 훅(`cpp_clang_format.py`)이 활성화되어 있다.
- **새 C++ 파일(`src/`, `tests/`)**: `clang-format`(100자 줄바꿈) 적용 후에도 **300줄 이하**가 유지되도록 여유 있게(285줄 이하) 작성하고, 편집 직후 `powershell -NoProfile -ExecutionPolicy Bypass -File scripts/scorecard.ps1 -LineLimitOnly`로 검증한다.
- **기존 300줄 초과 파일(`docs/quality/line-limit-baseline.txt` 등록 파일)**: 전체 파일 자동 포맷으로 인해 무관한 줄이 바뀌거나 기준 줄 수를 초과하지 않도록, 수정한 줄만 남기고 무관한 포맷 diff는 정리한다.
- **기존 테스트 기댓값 변경 시**: `CLAUDE.md` 규칙 7에 따라 커밋 메시지에 반드시 `시험 변경: <이유>` 줄을 포함한다.

## 4. 저장소·워크트리·포터블 동기화
- 현재 워크트리의 원격 저장소는 `https://github.com/kesekikwon-netizen/hgis.git` (`origin/main`)이며, 로컬 `D:\hgis` 클론과 같은 원격 저장소를 바라본다.
- 커밋 및 검증 완료 후에는:
  1. `git push origin HEAD:main`으로 원격 `main`에 푸시하고,
  2. `git -C D:/hgis pull --ff-only origin main`으로 `D:\hgis`도 최신 상태로 맞추며,
  3. 바탕화면에 개인용 포터블 폴더(`Strata-포터블-개인용-*`)가 존재하고 사용자가 포터블/실행 갱신을 원할 때는 최신 `build/Release/ka-hgis.exe`를 해당 포터블 폴더에도 복사하고 포터블 폴더 기준 `--smoke-quit`(종료 코드 `0`)으로 확인한다.
