#include "../../include/platform/IPlatformService.h"

#include <algorithm>
#include <filesystem>
#include <map>
#include <mutex>

namespace openhwp::platform {

namespace {

class LinuxComHwpBridgeStub final : public IComHwpBridge {
public:
    bool IsAvailable() const override { return false; }

    bool Open(const std::filesystem::path&, const std::string&, const std::string&) override {
        return false;
    }
    bool Save(const std::filesystem::path&) override { return false; }
    bool SaveAs(const std::filesystem::path&, const std::string&, const std::string&) override {
        return false;
    }
    bool PutFieldText(const std::string&, const std::string&) override { return false; }
    std::string GetFieldText(const std::string&) override { return {}; }
    std::string GetFieldList(int, int) override { return {}; }
    bool RunAction(const std::string&) override { return false; }
};

class LinuxPlatformService final : public IPlatformService {
public:
    HWND CreateViewerWindow(
        int x,
        int y,
        int width,
        int height,
        unsigned long style,
        HWND /*parent_hwnd*/,
        const ViewerWindowState& initial_state) override {
        std::lock_guard<std::mutex> lock(mutex_);
        uintptr_t id = next_handle_id_++;
        ViewerWindowState state = initial_state;
        state.x = x;
        state.y = y;
        state.width = width;
        state.height = height;
        state.style = style;
        state.visible = true;
        windows_[id] = std::move(state);
        return reinterpret_cast<HWND>(id);
    }

    void DestroyViewerWindow(HWND hwnd) override {
        std::lock_guard<std::mutex> lock(mutex_);
        uintptr_t id = reinterpret_cast<uintptr_t>(hwnd);
        windows_.erase(id);
    }

    void ResizeViewerWindow(HWND hwnd, int x, int y, int width, int height) override {
        std::lock_guard<std::mutex> lock(mutex_);
        uintptr_t id = reinterpret_cast<uintptr_t>(hwnd);
        auto it = windows_.find(id);
        if (it != windows_.end()) {
            it->second.x = x;
            it->second.y = y;
            it->second.width = width;
            it->second.height = height;
        }
    }

    void SetViewerFocus(HWND hwnd) override {
        std::lock_guard<std::mutex> lock(mutex_);
        focused_handle_ = reinterpret_cast<uintptr_t>(hwnd);
    }

    void SetViewerVisible(HWND hwnd, bool visible) override {
        std::lock_guard<std::mutex> lock(mutex_);
        uintptr_t id = reinterpret_cast<uintptr_t>(hwnd);
        auto it = windows_.find(id);
        if (it != windows_.end()) {
            it->second.visible = visible;
        }
    }

    void UpdateViewerDocumentText(HWND hwnd, const std::string& preview_text_utf8) override {
        std::lock_guard<std::mutex> lock(mutex_);
        uintptr_t id = reinterpret_cast<uintptr_t>(hwnd);
        auto it = windows_.find(id);
        if (it != windows_.end()) {
            it->second.rendered_preview_utf8 = preview_text_utf8;
        }
    }

    bool SetClipboardTextUtf8(const std::string& text_utf8) override {
        std::lock_guard<std::mutex> lock(mutex_);
        clipboard_utf8_ = text_utf8;
        return true;
    }

    std::string GetClipboardTextUtf8() override {
        std::lock_guard<std::mutex> lock(mutex_);
        return clipboard_utf8_;
    }

    std::vector<std::string> GetSystemFontNamesUtf8() override {
        std::vector<std::string> fonts = {
            "함초롬바탕",
            "함초롬돋움",
            "맑은 고딕",
            "바탕",
            "돋움",
            "굴림",
            "궁서",
            "Noto Sans CJK KR",
            "NanumGothic",
        };
        std::filesystem::path font_root{"/usr/share/fonts"};
        std::error_code ec;
        if (std::filesystem::exists(font_root, ec)) {
            int scanned = 0;
            for (const auto& entry : std::filesystem::recursive_directory_iterator(font_root, ec)) {
                if (ec || scanned >= 16) {
                    break;
                }
                if (entry.is_regular_file(ec)) {
                    auto ext = entry.path().extension().string();
                    if (ext == ".ttf" || ext == ".otf" || ext == ".ttc") {
                        std::string stem = entry.path().stem().string();
                        if (std::find(fonts.begin(), fonts.end(), stem) == fonts.end()) {
                            fonts.push_back(stem);
                            ++scanned;
                        }
                    }
                }
            }
        }
        return fonts;
    }

    std::unique_ptr<IComHwpBridge> CreateComHwpBridge() override {
        return std::make_unique<LinuxComHwpBridgeStub>();
    }

private:
    std::mutex mutex_;
    uintptr_t next_handle_id_ = 0x1000;
    uintptr_t focused_handle_ = 0;
    std::map<uintptr_t, ViewerWindowState> windows_;
    std::string clipboard_utf8_;
};

} // namespace

IPlatformService& GetPlatformService() {
    static LinuxPlatformService instance;
    return instance;
}

} // namespace openhwp::platform
