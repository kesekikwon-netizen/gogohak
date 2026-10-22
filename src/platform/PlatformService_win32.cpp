#include "platform/IPlatformService.h"

#include <windows.h>

#include <array>
#include <cstdlib>
#include <system_error>
#include <vector>

namespace strata::platform {
namespace {

// Windows API 호출 경계 전용 UTF-8 <-> UTF-16 변환 래퍼
std::wstring utf8ToWide(std::string_view utf8) {
  if (utf8.empty()) return {};
  const int len = ::MultiByteToWideChar(CP_UTF8, 0, utf8.data(), static_cast<int>(utf8.size()), nullptr, 0);
  if (len <= 0) return {};
  std::wstring out(static_cast<std::size_t>(len), L'\0');
  ::MultiByteToWideChar(CP_UTF8, 0, utf8.data(), static_cast<int>(utf8.size()), out.data(), len);
  return out;
}

std::filesystem::path envPath(const wchar_t* var_name, const std::filesystem::path& fallback) {
  wchar_t buf[MAX_PATH];
  const DWORD n = ::GetEnvironmentVariableW(var_name, buf, MAX_PATH);
  if (n > 0 && n < MAX_PATH) return std::filesystem::path(std::wstring(buf, n));
  return fallback;
}

std::filesystem::path detectWinReportRoot(std::string_view sub_name) {
  const std::array<std::filesystem::path, 4> roots = {
      std::filesystem::path("E:/") / sub_name,
      std::filesystem::path("D:/") / sub_name,
      std::filesystem::path("F:/") / sub_name,
      std::filesystem::path("C:/report1") / sub_name,
  };
  std::error_code ec;
  for (const auto& r : roots) {
    if (std::filesystem::exists(r, ec)) return r;
  }
  return roots[0];
}

std::wstring quoteWinArg(std::wstring_view arg) {
  if (!arg.empty() && arg.find_first_of(L" \t\"") == std::wstring_view::npos) return std::wstring(arg);
  std::wstring out = L"\"";
  for (wchar_t c : arg) {
    if (c == L'"') out += L"\\\"";
    else out += c;
  }
  out += L"\"";
  return out;
}

class Win32PlatformService final : public IPlatformService {
 public:
  std::filesystem::path appDataRoamingDir() const override {
    return envPath(L"APPDATA", std::filesystem::path("C:/Users/Public"));
  }

  std::filesystem::path appDataLocalDir() const override {
    return envPath(L"LOCALAPPDATA", std::filesystem::path("C:/Users/Public"));
  }

  std::filesystem::path documentsDir() const override {
    return envPath(L"USERPROFILE", std::filesystem::path("C:/Users/Public")) / "Documents";
  }

  std::filesystem::path tempDir() const override {
    std::error_code ec;
    const auto p = std::filesystem::temp_directory_path(ec);
    return ec ? std::filesystem::path("C:/Temp") : p;
  }

  std::filesystem::path reportPdfRootDir() const override {
    return detectWinReportRoot("report");
  }

  std::filesystem::path reportTextRootDir() const override {
    return detectWinReportRoot("report_text");
  }

  std::filesystem::path reportRagDbPath() const override {
    return reportTextRootDir() / "_rag_index.db";
  }

  std::filesystem::path resolveExecutable(std::string_view base_name) const override {
    const std::wstring wname = utf8ToWide(base_name);
    wchar_t buf[MAX_PATH];
    const DWORD n = ::SearchPathW(nullptr, wname.c_str(), L".exe", MAX_PATH, buf, nullptr);
    if (n > 0 && n < MAX_PATH) return std::filesystem::path(std::wstring(buf, n));
    return std::filesystem::path(wname);
  }

  ProcessResult runProcess(const std::filesystem::path& executable,
                           std::span<const std::string> args_utf8,
                           const std::filesystem::path& working_dir,
                           std::uint32_t timeout_ms) const override {
    ProcessResult res;
    SECURITY_ATTRIBUTES sa{sizeof(SECURITY_ATTRIBUTES), nullptr, TRUE};
    HANDLE out_read = nullptr, out_write = nullptr;
    if (!::CreatePipe(&out_read, &out_write, &sa, 0)) return res;
    ::SetHandleInformation(out_read, HANDLE_FLAG_INHERIT, 0);

    std::wstring cmd = quoteWinArg(executable.wstring());
    for (const auto& a : args_utf8) {
      cmd += L" ";
      cmd += quoteWinArg(utf8ToWide(a));
    }

    STARTUPINFOW si{};
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;
    si.hStdOutput = out_write;
    si.hStdError = out_write;

    PROCESS_INFORMATION pi{};
    const std::wstring wdir = working_dir.empty() ? std::wstring() : working_dir.wstring();
    const BOOL ok = ::CreateProcessW(nullptr, cmd.data(), nullptr, nullptr, TRUE, CREATE_NO_WINDOW,
                                     nullptr, wdir.empty() ? nullptr : wdir.c_str(), &si, &pi);
    ::CloseHandle(out_write);
    if (!ok) {
      ::CloseHandle(out_read);
      return res;
    }

    const DWORD wait_rc = ::WaitForSingleObject(pi.hProcess, timeout_ms);
    if (wait_rc == WAIT_TIMEOUT) {
      res.timed_out = true;
      ::TerminateProcess(pi.hProcess, 1);
    }

    char buf[4096];
    DWORD read_bytes = 0;
    while (::ReadFile(out_read, buf, sizeof(buf), &read_bytes, nullptr) && read_bytes > 0) {
      res.stdout_utf8.append(buf, read_bytes);
    }
    DWORD exit_code = 1;
    if (::GetExitCodeProcess(pi.hProcess, &exit_code)) res.exit_code = static_cast<int>(exit_code);
    ::CloseHandle(out_read);
    ::CloseHandle(pi.hThread);
    ::CloseHandle(pi.hProcess);
    return res;
  }

  bool spawnDetached(const std::filesystem::path& executable,
                     std::span<const std::string> args_utf8,
                     const std::filesystem::path& working_dir) const override {
    std::wstring cmd = quoteWinArg(executable.wstring());
    for (const auto& a : args_utf8) {
      cmd += L" ";
      cmd += quoteWinArg(utf8ToWide(a));
    }
    STARTUPINFOW si{};
    si.cb = sizeof(si);
    PROCESS_INFORMATION pi{};
    const std::wstring wdir = working_dir.empty() ? std::wstring() : working_dir.wstring();
    if (!::CreateProcessW(nullptr, cmd.data(), nullptr, nullptr, FALSE, DETACHED_PROCESS, nullptr,
                          wdir.empty() ? nullptr : wdir.c_str(), &si, &pi)) {
      return false;
    }
    ::CloseHandle(pi.hThread);
    ::CloseHandle(pi.hProcess);
    return true;
  }

  void terminateProcessByName(std::string_view process_base_name) const override {
    std::string target(process_base_name);
    if (!target.ends_with(".exe")) target += ".exe";
    const std::array<std::string, 3> args{"/F", "/IM", target};
    (void)runProcess(resolveExecutable("taskkill"), args, {}, 3000);
  }

  bool atomicReplaceFile(const std::filesystem::path& source_tmp,
                         const std::filesystem::path& target_final) const override {
    return ::MoveFileExW(source_tmp.c_str(), target_final.c_str(),
                         MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) != 0;
  }
};

}  // namespace

const IPlatformService& platformService() noexcept {
  static const Win32PlatformService kInstance;
  return kInstance;
}

}  // namespace strata::platform
