#include "platform/IPlatformService.h"

#include <poll.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <array>
#include <chrono>
#include <cstdlib>
#include <system_error>
#include <vector>

namespace strata::platform {
namespace {

std::filesystem::path homeDir() {
  if (const char* h = std::getenv("HOME"); h && *h) return std::filesystem::path(h);
  return std::filesystem::path("/tmp");
}

std::filesystem::path detectReportRoot(std::string_view sub_name) {
  const std::array<std::filesystem::path, 3> roots = {
      homeDir() / "Disks" / "report1" / sub_name,
      std::filesystem::path("/mnt/data-drawing") / sub_name,
      homeDir() / "hgis" / (sub_name == "report" ? "rag_reports_pdf" : "rag_reports_txt"),
  };
  std::error_code ec;
  for (const auto& candidate : roots) {
    if (std::filesystem::exists(candidate, ec)) return candidate;
  }
  return roots[0];
}

class LinuxPlatformService final : public IPlatformService {
 public:
  std::filesystem::path appDataRoamingDir() const override {
    if (const char* xdg = std::getenv("XDG_CONFIG_HOME"); xdg && *xdg)
      return std::filesystem::path(xdg);
    return homeDir() / ".config";
  }

  std::filesystem::path appDataLocalDir() const override {
    if (const char* xdg = std::getenv("XDG_DATA_HOME"); xdg && *xdg)
      return std::filesystem::path(xdg);
    return homeDir() / ".local" / "share";
  }

  std::filesystem::path documentsDir() const override {
    std::error_code ec;
    const auto kr = homeDir() / "문서";
    if (std::filesystem::exists(kr, ec)) return kr;
    const auto en = homeDir() / "Documents";
    if (std::filesystem::exists(en, ec)) return en;
    return homeDir();
  }

  std::filesystem::path tempDir() const override {
    std::error_code ec;
    const auto p = std::filesystem::temp_directory_path(ec);
    return ec ? std::filesystem::path("/tmp") : p;
  }

  std::filesystem::path reportPdfRootDir() const override {
    return detectReportRoot("report");
  }

  std::filesystem::path reportTextRootDir() const override {
    return detectReportRoot("report_text");
  }

  std::filesystem::path reportRagDbPath() const override {
    return reportTextRootDir() / "_rag_index.db";
  }

  std::filesystem::path resolveExecutable(std::string_view base_name) const override {
    std::error_code ec;
    const std::filesystem::path direct(base_name);
    if (direct.is_absolute() && std::filesystem::exists(direct, ec)) return direct;

    const char* path_env = std::getenv("PATH");
    if (!path_env) return direct;
    std::string_view sv(path_env);
    std::size_t start = 0;
    while (start <= sv.size()) {
      const std::size_t colon = sv.find(':', start);
      const auto dir_sv = sv.substr(start, colon == std::string_view::npos ? sv.size() - start : colon - start);
      if (!dir_sv.empty()) {
        const auto candidate = std::filesystem::path(dir_sv) / base_name;
        if (::access(candidate.c_str(), X_OK) == 0) return candidate;
      }
      if (colon == std::string_view::npos) break;
      start = colon + 1;
    }
    return direct;
  }

  ProcessResult runProcess(const std::filesystem::path& executable,
                           std::span<const std::string> args_utf8,
                           const std::filesystem::path& working_dir,
                           std::uint32_t timeout_ms) const override {
    ProcessResult res;
    int out_pipe[2]{-1, -1};
    int err_pipe[2]{-1, -1};
    if (::pipe(out_pipe) != 0) return res;
    if (::pipe(err_pipe) != 0) {
      ::close(out_pipe[0]);
      ::close(out_pipe[1]);
      return res;
    }

    const pid_t pid = ::fork();
    if (pid < 0) {
      ::close(out_pipe[0]);
      ::close(out_pipe[1]);
      ::close(err_pipe[0]);
      ::close(err_pipe[1]);
      return res;
    }
    if (pid == 0) {
      ::dup2(out_pipe[1], STDOUT_FILENO);
      ::dup2(err_pipe[1], STDERR_FILENO);
      ::close(out_pipe[0]);
      ::close(out_pipe[1]);
      ::close(err_pipe[0]);
      ::close(err_pipe[1]);
      if (!working_dir.empty()) {
        [[maybe_unused]] const int rc = ::chdir(working_dir.c_str());
      }
      std::vector<char*> argv;
      std::string exe_str = executable.string();
      argv.push_back(exe_str.data());
      std::vector<std::string> arg_copy(args_utf8.begin(), args_utf8.end());
      for (auto& s : arg_copy) argv.push_back(s.data());
      argv.push_back(nullptr);
      ::execvp(exe_str.c_str(), argv.data());
      ::_exit(127);
    }

    ::close(out_pipe[1]);
    ::close(err_pipe[1]);

    const auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeout_ms);
    std::array<pollfd, 2> fds{{{out_pipe[0], POLLIN, 0}, {err_pipe[0], POLLIN, 0}}};
    int open_fds = 2;
    char buf[4096];

    while (open_fds > 0) {
      const auto now = std::chrono::steady_clock::now();
      if (now >= deadline) {
        res.timed_out = true;
        ::kill(pid, SIGKILL);
        break;
      }
      const auto rem = std::chrono::duration_cast<std::chrono::milliseconds>(deadline - now).count();
      const int prc = ::poll(fds.data(), fds.size(), static_cast<int>(rem));
      if (prc <= 0) continue;
      for (std::size_t i = 0; i < fds.size(); ++i) {
        if (fds[i].fd < 0) continue;
        if (fds[i].revents & (POLLIN | POLLHUP | POLLERR)) {
          const ssize_t n = ::read(fds[i].fd, buf, sizeof(buf));
          if (n > 0) {
            (i == 0 ? res.stdout_utf8 : res.stderr_utf8).append(buf, static_cast<std::size_t>(n));
          } else {
            ::close(fds[i].fd);
            fds[i].fd = -1;
            --open_fds;
          }
        }
      }
    }
    if (fds[0].fd >= 0) ::close(fds[0].fd);
    if (fds[1].fd >= 0) ::close(fds[1].fd);

    int status = 0;
    ::waitpid(pid, &status, 0);
    if (WIFEXITED(status)) res.exit_code = WEXITSTATUS(status);
    return res;
  }

  bool spawnDetached(const std::filesystem::path& executable,
                     std::span<const std::string> args_utf8,
                     const std::filesystem::path& working_dir) const override {
    const pid_t pid = ::fork();
    if (pid < 0) return false;
    if (pid == 0) {
      ::setsid();
      if (!working_dir.empty()) {
        [[maybe_unused]] const int rc = ::chdir(working_dir.c_str());
      }
      std::vector<char*> argv;
      std::string exe_str = executable.string();
      argv.push_back(exe_str.data());
      std::vector<std::string> arg_copy(args_utf8.begin(), args_utf8.end());
      for (auto& s : arg_copy) argv.push_back(s.data());
      argv.push_back(nullptr);
      ::execvp(exe_str.c_str(), argv.data());
      ::_exit(127);
    }
    return true;
  }

  void terminateProcessByName(std::string_view process_base_name) const override {
    const std::array<std::string, 2> args{"-f", std::string(process_base_name)};
    (void)runProcess(resolveExecutable("pkill"), args, {}, 2000);
  }

  bool atomicReplaceFile(const std::filesystem::path& source_tmp,
                         const std::filesystem::path& target_final) const override {
    std::error_code ec;
    std::filesystem::rename(source_tmp, target_final, ec);
    if (!ec) return true;
    std::filesystem::copy_file(source_tmp, target_final,
                               std::filesystem::copy_options::overwrite_existing, ec);
    std::error_code rm_ec;
    std::filesystem::remove(source_tmp, rm_ec);
    return !ec;
  }
};

}  // namespace

const IPlatformService& platformService() noexcept {
  static const LinuxPlatformService kInstance;
  return kInstance;
}

}  // namespace strata::platform
