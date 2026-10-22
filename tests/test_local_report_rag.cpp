// 로컬 발굴조사보고서 RAG 클라이언트 및 플랫폼 추상화 계층(IPlatformService) 순수 C++20 단위 테스트
#include "core/LocalReportRag.h"
#include "platform/IPlatformService.h"

#include <cassert>
#include <filesystem>
#include <iostream>
#include <string>

int main() {
  [[maybe_unused]] const auto& pal = strata::platform::platformService();

  // 1. 플랫폼 경로(std::filesystem::path) 기본 검증
  assert(!pal.tempDir().empty());
  assert(!pal.reportPdfRootDir().empty());
  assert(!pal.reportTextRootDir().empty());
  assert(pal.reportRagDbPath().filename() == "_rag_index.db");

  // 2. _rag_index_and_search.py --json 출력 파서 검증
  constexpr std::string_view kSampleJson = R"json([
    {
      "sigungu": "경주시",
      "report_name": "경주 성건동 320-1번지 유적 발굴조사보고서.pdf",
      "page_no": 78,
      "score": -12.3456,
      "snippet": "1호 [[주구석곽묘]] 내부에서 [[토기]] 출토",
      "content": "1호 주구석곽묘는 평면 장방형으로 내부에서 단경호와 고배 등 토기류가 출토되었다.",
      "txt_path": "/home/kwon/Disks/report1/report_text/경주시/경주 성건동 320-1번지 유적 발굴조사보고서.txt",
      "pdf_path": "/home/kwon/Disks/report1/report/경주시/경주 성건동 320-1번지 유적 발굴조사보고서.pdf"
    }
  ])json";

  const auto parsed = strata::core::LocalReportRag::parseSearchJson(kSampleJson);
  assert(parsed.size() == 1);
  assert(parsed[0].sigungu == "경주시");
  assert(parsed[0].report_name == "경주 성건동 320-1번지 유적 발굴조사보고서.pdf");
  assert(parsed[0].page_no == 78);
  assert(parsed[0].content.find("주구석곽묘") != std::string::npos);
  assert(parsed[0].txt_path.filename() == "경주 성건동 320-1번지 유적 발굴조사보고서.txt");

  // 3. 보고서 프롬프트 컨텍스트 포맷 및 원문 미확보 안내 검증
  const std::string ctx_with_hit = strata::core::LocalReportRag::formatPromptContext(parsed);
  assert(ctx_with_hit.find("[R1] [경주시]") != std::string::npos);
  assert(ctx_with_hit.find("(p.78)") != std::string::npos);
  assert(ctx_with_hit.find("[Page 78]") != std::string::npos);

  const std::vector<strata::core::LocalReportHit> empty_hits;
  const std::string ctx_empty = strata::core::LocalReportRag::formatPromptContext(empty_hits);
  assert(ctx_empty.find("원문 미확보") != std::string::npos);

  // 4. 실제 로컬 발굴조사보고서 RAG DB가 존재할 경우 네이티브 C++20 SQLite3 FTS5 직접 조회 검증
  std::error_code ec;
  if (std::filesystem::exists(strata::core::LocalReportRag::ragDbPath(), ec)) {
    const auto st = strata::core::LocalReportRag::queryStats();
    assert(st.available);
    assert(st.report_count > 0);
    assert(st.chunk_count > 0);
    std::cout << "[PASS] LocalReportRag native SQLite3 stats: " << st.summary_utf8 << "\n";

    const auto hits = strata::core::LocalReportRag::search("유적", "", 3);
    assert(!hits.empty());
    assert(!hits[0].report_name.empty());
    assert(hits[0].page_no > 0);
    std::cout << "[PASS] LocalReportRag native SQLite3 FTS5 search hit[0]: [" << hits[0].sigungu << "] "
              << hits[0].report_name << " (p." << hits[0].page_no << ")\n";
  }

  std::cout << "[PASS] All LocalReportRag & SQLite3 FTS5 C++20 tests succeeded.\n";
  return 0;
}
