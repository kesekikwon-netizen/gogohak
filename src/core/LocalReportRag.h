#pragma once
// 로컬 발굴조사보고서 RAG 검색·인용 클라이언트 (순수 표준 C++20)
// - 표준 경로: /home/kwon/Disks/report1/report/_rag_index_and_search.py 및 report_text/_rag_index.db
// - OS 종속 호출 없이 strata::platform::IPlatformService 와 std::filesystem::path 만 사용한다.
#include <filesystem>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace strata::core {

struct LocalReportHit {
  std::string sigungu;
  std::string report_name;
  int page_no = 0;
  double score = 0.0;
  std::string snippet;
  std::string content;
  std::filesystem::path txt_path;
  std::filesystem::path pdf_path;
};

struct LocalReportStats {
  bool available = false;
  int report_count = 0;
  int region_count = 0;
  int chunk_count = 0;
  std::string summary_utf8;
};

class LocalReportRag {
 public:
  // 표준 스크립트 및 DB 경로 반환 (std::filesystem::path)
  static std::filesystem::path ragScriptPath();
  static std::filesystem::path pipelineScriptPath();
  static std::filesystem::path ragDbPath();

  // RAG 인덱스 통계 조회 (python3 _rag_index_and_search.py stats)
  static LocalReportStats queryStats();

  // 키워드 + 지역(--region) 기반 보고서 본문 검색 (--json 출력 파싱)
  static std::vector<LocalReportHit> search(std::string_view query_utf8,
                                            std::string_view region_utf8 = {},
                                            int top_k = 10);

  // _rag_index_and_search.py --json 출력을 파싱하는 순수 함수 (단위 테스트용)
  static std::vector<LocalReportHit> parseSearchJson(std::string_view json_utf8);

  // 보고서 작성 프롬프트 주입용 마크다운 블록 생성 (정확한 보고서명, p.페이지번호, txt_path, 본문 발췌 포함)
  static std::string formatPromptContext(std::span<const LocalReportHit> hits);

  // 원클릭 전체 파이프라인 백그라운드 실행 (/home/kwon/Disks/report1/report/run_pipeline.sh)
  static bool startPipelineDetached();
};

}  // namespace strata::core
