#include "core/LocalReportRag.h"
#include "platform/IPlatformService.h"

#include <array>
#include <charconv>
#include <cstdlib>
#include <sstream>
#include <system_error>

// 표준 SQLite3 C ABI 전방 선언 (OSGeo4W sqlite3.lib 및 Linux libsqlite3.so.0 공통)
extern "C" {
struct sqlite3;
struct sqlite3_stmt;
int sqlite3_open_v2(const char* filename, sqlite3** ppDb, int flags, const char* zVfs);
int sqlite3_busy_timeout(sqlite3* db, int ms);
int sqlite3_prepare_v2(sqlite3* db, const char* zSql, int nByte, sqlite3_stmt** ppStmt, const char** pzTail);
int sqlite3_bind_text(sqlite3_stmt* pStmt, int idx, const char* val, int nByte, void (*del)(void*));
int sqlite3_bind_int(sqlite3_stmt* pStmt, int idx, int val);
int sqlite3_step(sqlite3_stmt* pStmt);
const unsigned char* sqlite3_column_text(sqlite3_stmt* pStmt, int iCol);
int sqlite3_column_int(sqlite3_stmt* pStmt, int iCol);
double sqlite3_column_double(sqlite3_stmt* pStmt, int iCol);
int sqlite3_finalize(sqlite3_stmt* pStmt);
int sqlite3_close(sqlite3* db);
}

namespace strata::core {
namespace {

constexpr int kSqliteOk = 0;
constexpr int kSqliteRow = 100;
constexpr int kSqliteOpenReadonly = 0x00000001;
constexpr int kSqliteOpenNomutex = 0x00008000;

std::string colStr(sqlite3_stmt* st, int col) {
  const unsigned char* p = ::sqlite3_column_text(st, col);
  return p ? std::string(reinterpret_cast<const char*>(p)) : std::string();
}

std::string buildFtsQuery(std::string_view raw) {
  std::string out;
  std::size_t i = 0;
  while (i < raw.size()) {
    while (i < raw.size() && (raw[i] == ' ' || raw[i] == '\t' || raw[i] == '\n' || raw[i] == '\r')) ++i;
    if (i >= raw.size()) break;
    std::string tok;
    while (i < raw.size() && raw[i] != ' ' && raw[i] != '\t' && raw[i] != '\n' && raw[i] != '\r') {
      const char c = raw[i++];
      if (c != '"' && c != '\'' && c != '(' && c != ')' && c != '*' && c != '^' && c != ':')
        tok.push_back(c);
    }
    if (!tok.empty()) {
      if (!out.empty()) out += " AND ";
      out += "\"" + tok + "\"";
    }
  }
  return out.empty() ? "\"\"" : out;
}

std::string unescapeJsonString(std::string_view s) {
  std::string out;
  out.reserve(s.size());
  for (std::size_t i = 0; i < s.size(); ++i) {
    if (s[i] == '\\' && i + 1 < s.size()) {
      const char next = s[++i];
      switch (next) {
        case '"': out.push_back('"'); break;
        case '\\': out.push_back('\\'); break;
        case '/': out.push_back('/'); break;
        case 'n': out.push_back('\n'); break;
        case 'r': out.push_back('\r'); break;
        case 't': out.push_back('\t'); break;
        default: out.push_back(next); break;
      }
    } else {
      out.push_back(s[i]);
    }
  }
  return out;
}

std::string extractJsonStringField(std::string_view obj, std::string_view key) {
  const std::string pattern = "\"" + std::string(key) + "\"";
  std::size_t pos = obj.find(pattern);
  if (pos == std::string_view::npos) return {};
  pos = obj.find(':', pos + pattern.size());
  if (pos == std::string_view::npos) return {};
  pos = obj.find('"', pos + 1);
  if (pos == std::string_view::npos) return {};
  ++pos;
  std::size_t end = pos;
  while (end < obj.size()) {
    if (obj[end] == '\\') { end += 2; continue; }
    if (obj[end] == '"') break;
    ++end;
  }
  return end <= obj.size() ? unescapeJsonString(obj.substr(pos, end - pos)) : std::string();
}

int extractJsonIntField(std::string_view obj, std::string_view key) {
  const std::string pattern = "\"" + std::string(key) + "\"";
  std::size_t pos = obj.find(pattern);
  if (pos == std::string_view::npos) return 0;
  pos = obj.find(':', pos + pattern.size());
  if (pos == std::string_view::npos) return 0;
  ++pos;
  while (pos < obj.size() && (obj[pos] == ' ' || obj[pos] == '\t')) ++pos;
  int val = 0;
  std::from_chars(obj.data() + pos, obj.data() + obj.size(), val);
  return val;
}

double extractJsonDoubleField(std::string_view obj, std::string_view key) {
  const std::string pattern = "\"" + std::string(key) + "\"";
  std::size_t pos = obj.find(pattern);
  if (pos == std::string_view::npos) return 0.0;
  pos = obj.find(':', pos + pattern.size());
  if (pos == std::string_view::npos) return 0.0;
  ++pos;
  while (pos < obj.size() && (obj[pos] == ' ' || obj[pos] == '\t')) ++pos;
  return std::strtod(std::string(obj.substr(pos, 32)).c_str(), nullptr);
}

}  // namespace

std::filesystem::path LocalReportRag::ragScriptPath() {
  return strata::platform::platformService().reportPdfRootDir() / "_rag_index_and_search.py";
}

std::filesystem::path LocalReportRag::pipelineScriptPath() {
  return strata::platform::platformService().reportPdfRootDir() / "run_pipeline.sh";
}

std::filesystem::path LocalReportRag::ragDbPath() {
  return strata::platform::platformService().reportRagDbPath();
}

LocalReportStats LocalReportRag::queryStats() {
  LocalReportStats st;
  const auto db_path = ragDbPath();
  std::error_code ec;
  if (!std::filesystem::exists(db_path, ec)) return st;

  sqlite3* db = nullptr;
  const std::string db_utf8 = db_path.string();
  if (::sqlite3_open_v2(db_utf8.c_str(), &db, kSqliteOpenReadonly | kSqliteOpenNomutex, nullptr) != kSqliteOk) {
    if (db) ::sqlite3_close(db);
    return st;
  }
  ::sqlite3_busy_timeout(db, 2000);

  auto queryInt = [&](const char* sql) {
    sqlite3_stmt* stmt = nullptr;
    int val = 0;
    if (::sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == kSqliteOk) {
      if (::sqlite3_step(stmt) == kSqliteRow) val = ::sqlite3_column_int(stmt, 0);
      ::sqlite3_finalize(stmt);
    }
    return val;
  };
  st.report_count = queryInt("SELECT COUNT(*) FROM documents");
  st.region_count = queryInt("SELECT COUNT(DISTINCT sigungu) FROM documents");
  st.chunk_count = queryInt("SELECT COUNT(*) FROM chunks_fts");
  ::sqlite3_close(db);

  st.available = (st.report_count > 0 || st.chunk_count > 0);
  std::ostringstream oss;
  oss << "인덱싱된 보고서 수: " << st.report_count << "권 | 지역(시군구) 수: " << st.region_count
      << "개 | 총 검색 청크(페이지 블록): " << st.chunk_count << "개";
  st.summary_utf8 = oss.str();
  return st;
}

std::vector<LocalReportHit> LocalReportRag::parseSearchJson(std::string_view json_utf8) {
  std::vector<LocalReportHit> hits;
  std::size_t pos = 0;
  while (pos < json_utf8.size()) {
    const std::size_t obj_start = json_utf8.find('{', pos);
    if (obj_start == std::string_view::npos) break;
    int depth = 0;
    bool in_str = false;
    std::size_t obj_end = obj_start;
    for (; obj_end < json_utf8.size(); ++obj_end) {
      const char c = json_utf8[obj_end];
      if (in_str && c == '\\') { ++obj_end; continue; }
      if (c == '"') in_str = !in_str;
      else if (!in_str && c == '{') ++depth;
      else if (!in_str && c == '}' && --depth == 0) break;
    }
    if (obj_end >= json_utf8.size()) break;
    const std::string_view obj = json_utf8.substr(obj_start, obj_end - obj_start + 1);
    LocalReportHit h;
    h.sigungu = extractJsonStringField(obj, "sigungu");
    h.report_name = extractJsonStringField(obj, "report_name");
    h.page_no = extractJsonIntField(obj, "page_no");
    h.score = extractJsonDoubleField(obj, "score");
    h.snippet = extractJsonStringField(obj, "snippet");
    h.content = extractJsonStringField(obj, "content");
    h.txt_path = std::filesystem::path(extractJsonStringField(obj, "txt_path"));
    h.pdf_path = std::filesystem::path(extractJsonStringField(obj, "pdf_path"));
    if (!h.report_name.empty()) hits.push_back(std::move(h));
    pos = obj_end + 1;
  }
  return hits;
}

std::vector<LocalReportHit> LocalReportRag::search(std::string_view query_utf8,
                                                   std::string_view region_utf8,
                                                   int top_k) {
  std::vector<LocalReportHit> hits;
  if (query_utf8.empty()) return hits;
  const auto& pal = strata::platform::platformService();
  const auto db_path = ragDbPath();
  std::error_code ec;
  if (!std::filesystem::exists(db_path, ec)) return hits;

  sqlite3* db = nullptr;
  const std::string db_str = db_path.string();
  if (::sqlite3_open_v2(db_str.c_str(), &db, kSqliteOpenReadonly | kSqliteOpenNomutex, nullptr) != kSqliteOk) {
    if (db) ::sqlite3_close(db);
    return hits;
  }
  ::sqlite3_busy_timeout(db, 2000);

  const std::string fts_q = buildFtsQuery(query_utf8);
  std::string sql =
      "SELECT rel_path, sigungu, report_name, page_no, "
      "snippet(chunks_fts, 4, '[[', ']]', ' ... ', 64), content, bm25(chunks_fts) AS score "
      "FROM chunks_fts WHERE chunks_fts MATCH ?";
  if (!region_utf8.empty()) sql += " AND sigungu LIKE ?";
  sql += " ORDER BY score LIMIT ?";

  sqlite3_stmt* stmt = nullptr;
  if (::sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == kSqliteOk) {
    int bind_idx = 1;
    ::sqlite3_bind_text(stmt, bind_idx++, fts_q.c_str(), static_cast<int>(fts_q.size()), nullptr);
    std::string region_like;
    if (!region_utf8.empty()) {
      region_like = "%" + std::string(region_utf8) + "%";
      ::sqlite3_bind_text(stmt, bind_idx++, region_like.c_str(), static_cast<int>(region_like.size()), nullptr);
    }
    ::sqlite3_bind_int(stmt, bind_idx, top_k > 0 ? top_k : 10);

    const auto txt_root = pal.reportTextRootDir();
    const auto pdf_root = pal.reportPdfRootDir();
    while (::sqlite3_step(stmt) == kSqliteRow) {
      LocalReportHit h;
      const std::filesystem::path rel(colStr(stmt, 0));
      h.sigungu = colStr(stmt, 1);
      h.report_name = colStr(stmt, 2);
      h.page_no = ::sqlite3_column_int(stmt, 3);
      h.snippet = colStr(stmt, 4);
      h.content = colStr(stmt, 5);
      if (h.content.size() > 800) h.content.resize(800);
      h.score = ::sqlite3_column_double(stmt, 6);
      h.txt_path = txt_root / rel;
      auto pdf_rel = pdf_root / rel;
      pdf_rel.replace_extension(".pdf");
      h.pdf_path = std::move(pdf_rel);
      hits.push_back(std::move(h));
    }
    ::sqlite3_finalize(stmt);
  }
  ::sqlite3_close(db);
  return hits;
}

std::string LocalReportRag::formatPromptContext(std::span<const LocalReportHit> hits) {
  std::ostringstream oss;
  oss << "## [로컬 발굴조사보고서 RAG 원문 발췌 (" << hits.size() << "건)]\n";
  if (hits.empty()) {
    oss << "- (일치하는 로컬 발굴조사보고서 원문 없음 — 해당 유적의 세부 유구·유물 제원은 '발굴보고서 원문 미확보'로 명시할 것)\n";
    return oss.str();
  }
  for (std::size_t i = 0; i < hits.size(); ++i) {
    const auto& h = hits[i];
    oss << "- **[R" << (i + 1) << "] [" << h.sigungu << "] 『" << h.report_name << "』 (p." << h.page_no << ")**\n"
        << "  * 텍스트 원문 경로: `" << h.txt_path.string() << "`\n"
        << "  * 페이지 원문 발췌([Page " << h.page_no << "]): " << h.content << "\n";
  }
  return oss.str();
}

bool LocalReportRag::startPipelineDetached() {
  const auto& pal = strata::platform::platformService();
  const auto script = pipelineScriptPath();
  std::error_code ec;
  if (!std::filesystem::exists(script, ec)) return false;
  const std::array<std::string, 1> args{script.string()};
  return pal.spawnDetached(pal.resolveExecutable("bash"), args, script.parent_path());
}

}  // namespace strata::core
