#pragma once
// 플랫폼 추상화 계층 (Platform Abstraction Layer — PAL)
// - 비즈니스 로직 내부에 #ifdef _WIN32 나 OS 종속 헤더(<windows.h>, <unistd.h> 등)를 섞지 않기 위한 순수 C++20 인터페이스.
// - 모든 파일 경로는 std::filesystem::path 만 사용하고, 문자열은 UTF-8(std::string)을 기준으로 한다.
#include <cstdint>
#include <filesystem>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace strata::platform {

struct ProcessResult {
  int exit_code = -1;
  std::string stdout_utf8;
  std::string stderr_utf8;
  bool timed_out = false;
};

class IPlatformService {
 public:
  virtual ~IPlatformService() = default;

  // 1. 표준 디렉터리 및 로컬 발굴조사보고서 RAG 루트 탐색 (항상 std::filesystem::path 반환)
  virtual std::filesystem::path appDataRoamingDir() const = 0;
  virtual std::filesystem::path appDataLocalDir() const = 0;
  virtual std::filesystem::path documentsDir() const = 0;
  virtual std::filesystem::path tempDir() const = 0;
  virtual std::filesystem::path reportPdfRootDir() const = 0;
  virtual std::filesystem::path reportTextRootDir() const = 0;
  virtual std::filesystem::path reportRagDbPath() const = 0;
  virtual std::filesystem::path resolveExecutable(std::string_view base_name) const = 0;

  // 2. 외부 프로세스 동기/비동기 실행 및 프로세스 종료 (UTF-8 인자 전달)
  virtual ProcessResult runProcess(
      const std::filesystem::path& executable,
      std::span<const std::string> args_utf8,
      const std::filesystem::path& working_dir = {},
      std::uint32_t timeout_ms = 15000) const = 0;

  virtual bool spawnDetached(
      const std::filesystem::path& executable,
      std::span<const std::string> args_utf8,
      const std::filesystem::path& working_dir = {}) const = 0;

  virtual void terminateProcessByName(std::string_view process_base_name) const = 0;

  // 3. 원자적 파일 교체 (임시 파일 -> 최종 파일)
  virtual bool atomicReplaceFile(
      const std::filesystem::path& source_tmp,
      const std::filesystem::path& target_final) const = 0;
};

// CMake 에서 조건부로 링크된 플랫폼 구현체(*_linux.cpp 또는 *_win32.cpp) 싱글턴 반환
const IPlatformService& platformService() noexcept;

}  // namespace strata::platform
