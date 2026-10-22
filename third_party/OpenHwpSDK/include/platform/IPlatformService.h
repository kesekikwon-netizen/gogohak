#pragma once

#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

#include "../SdkDef.h"

namespace openhwp::platform {

/**
 * @brief 뷰어/에디터 창(HWindow)의 플랫폼 독립 상태 구조체
 */
struct ViewerWindowState {
    int x = 0;
    int y = 0;
    int width = 800;
    int height = 600;
    unsigned long style = 0;
    bool visible = true;
    int zoom_type = 1; // 1: PageFit, 2: WidthFit
    std::string rendered_preview_utf8;
};

/**
 * @brief Windows 로컬 한글(HWPFrame.HwpObject COM) 선택적 연동 브릿지 인터페이스.
 * 리눅스 및 기본(독립형) 모드에서는 nullptr 또는 비활성 스텁으로 동작하며,
 * 윈도우에서 사용자가 COM 브릿지 모드를 활성화한 경우 로컬 한글 프로세스와 동기화합니다.
 */
class IComHwpBridge {
public:
    virtual ~IComHwpBridge() = default;

    virtual bool IsAvailable() const = 0;
    virtual bool Open(const std::filesystem::path& path, const std::string& format_utf8, const std::string& args_utf8) = 0;
    virtual bool Save(const std::filesystem::path& path) = 0;
    virtual bool SaveAs(const std::filesystem::path& path, const std::string& format_utf8, const std::string& args_utf8) = 0;
    virtual bool PutFieldText(const std::string& field_list_utf8, const std::string& text_list_utf8) = 0;
    virtual std::string GetFieldText(const std::string& field_list_utf8) = 0;
    virtual std::string GetFieldList(int number_opt, int field_opt) = 0;
    virtual bool RunAction(const std::string& action_id_utf8) = 0;
};

/**
 * @brief OS 종속 기능(뷰어 윈도우 핸들 제어, 클립보드, 로컬 COM 브릿지, 인코딩 래퍼)을
 * 추상화하는 플랫폼 서비스 인터페이스.
 * 구현체는 platform_linux.cpp 및 platform_win32.cpp로 분리됩니다.
 */
class IPlatformService {
public:
    virtual ~IPlatformService() = default;

    // 1. HWindow GUI 뷰어 창 생성/제어 추상화
    virtual HWND CreateViewerWindow(
        int x,
        int y,
        int width,
        int height,
        unsigned long style,
        HWND parent_hwnd,
        const ViewerWindowState& initial_state) = 0;

    virtual void DestroyViewerWindow(HWND hwnd) = 0;
    virtual void ResizeViewerWindow(HWND hwnd, int x, int y, int width, int height) = 0;
    virtual void SetViewerFocus(HWND hwnd) = 0;
    virtual void SetViewerVisible(HWND hwnd, bool visible) = 0;
    virtual void UpdateViewerDocumentText(HWND hwnd, const std::string& preview_text_utf8) = 0;

    // 2. 시스템 클립보드 추상화 (Copy / Paste 액션 연동)
    virtual bool SetClipboardTextUtf8(const std::string& text_utf8) = 0;
    virtual std::string GetClipboardTextUtf8() = 0;

    // 3. 시스템 폰트 목록 조회 (GetFontList / ScanFont 연동)
    virtual std::vector<std::string> GetSystemFontNamesUtf8() = 0;

    // 4. 선택적 Windows 로컬 한글(COM) 브릿지 팩토리
    virtual std::unique_ptr<IComHwpBridge> CreateComHwpBridge() = 0;
};

/**
 * @brief 현재 OS 환경에 맞는 IPlatformService 싱글턴 인스턴스를 반환합니다.
 */
IPlatformService& GetPlatformService();

} // namespace openhwp::platform
