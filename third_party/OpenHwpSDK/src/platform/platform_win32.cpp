#include "../../include/platform/IPlatformService.h"
#include "../core/utf_conv.hpp"

#include <filesystem>
#include <map>
#include <mutex>

#if defined(_WIN32)
#include <windows.h>
#include <objbase.h>
#include <oleauto.h>
#endif

namespace openhwp::platform {

namespace {

#if defined(_WIN32)
/**
 * @brief Windows 로컬 한글(HWPFrame.HwpObject) IDispatch COM 오토메이션 브릿지 구현체.
 * 모든 문자열 인자는 UTF-8 -> UTF-16(std::wstring / BSTR) 래퍼를 통해 변환합니다.
 */
class Win32ComHwpBridge final : public IComHwpBridge {
public:
    Win32ComHwpBridge() {
        HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
        com_initialized_ = SUCCEEDED(hr) || (hr == RPC_E_CHANGED_MODE);

        CLSID clsid{};
        std::wstring prog_id = core::Utf8ToWString("HWPFrame.HwpObject");
        if (SUCCEEDED(CLSIDFromProgID(prog_id.c_str(), &clsid))) {
            IDispatch* disp = nullptr;
            if (SUCCEEDED(CoCreateInstance(clsid, nullptr, CLSCTX_LOCAL_SERVER, IID_IDispatch, reinterpret_cast<void**>(&disp)))) {
                dispatch_ = disp;
            }
        }
    }

    ~Win32ComHwpBridge() override {
        if (dispatch_) {
            dispatch_->Release();
            dispatch_ = nullptr;
        }
        if (com_initialized_) {
            CoUninitialize();
        }
    }

    bool IsAvailable() const override { return dispatch_ != nullptr; }

    bool Open(const std::filesystem::path& path, const std::string& format_utf8, const std::string& args_utf8) override {
        if (!dispatch_) return false;
        std::wstring wpath = path.wstring();
        std::wstring wfmt = core::Utf8ToWString(format_utf8);
        std::wstring wargs = core::Utf8ToWString(args_utf8);

        VARIANT args[3];
        VariantInit(&args[0]);
        VariantInit(&args[1]);
        VariantInit(&args[2]);
        // IDispatch arguments are passed in reverse order
        args[0].vt = VT_BSTR; args[0].bstrVal = SysAllocString(wargs.c_str());
        args[1].vt = VT_BSTR; args[1].bstrVal = SysAllocString(wfmt.c_str());
        args[2].vt = VT_BSTR; args[2].bstrVal = SysAllocString(wpath.c_str());

        VARIANT result;
        VariantInit(&result);
        bool ok = InvokeMethod(L"Open", args, 3, &result);
        VariantClear(&args[0]);
        VariantClear(&args[1]);
        VariantClear(&args[2]);
        VariantClear(&result);
        return ok;
    }

    bool Save(const std::filesystem::path& /*path*/) override {
        if (!dispatch_) return false;
        VARIANT arg;
        VariantInit(&arg);
        arg.vt = VT_BOOL;
        arg.boolVal = VARIANT_TRUE;
        VARIANT result;
        VariantInit(&result);
        bool ok = InvokeMethod(L"Save", &arg, 1, &result);
        VariantClear(&result);
        return ok;
    }

    bool SaveAs(const std::filesystem::path& path, const std::string& format_utf8, const std::string& args_utf8) override {
        if (!dispatch_) return false;
        std::wstring wpath = path.wstring();
        std::wstring wfmt = core::Utf8ToWString(format_utf8.empty() ? "HWP" : format_utf8);
        std::wstring wargs = core::Utf8ToWString(args_utf8);

        VARIANT args[3];
        VariantInit(&args[0]);
        VariantInit(&args[1]);
        VariantInit(&args[2]);
        args[0].vt = VT_BSTR; args[0].bstrVal = SysAllocString(wargs.c_str());
        args[1].vt = VT_BSTR; args[1].bstrVal = SysAllocString(wfmt.c_str());
        args[2].vt = VT_BSTR; args[2].bstrVal = SysAllocString(wpath.c_str());

        VARIANT result;
        VariantInit(&result);
        bool ok = InvokeMethod(L"SaveAs", args, 3, &result);
        VariantClear(&args[0]);
        VariantClear(&args[1]);
        VariantClear(&args[2]);
        VariantClear(&result);
        return ok;
    }

    bool PutFieldText(const std::string& field_list_utf8, const std::string& text_list_utf8) override {
        if (!dispatch_) return false;
        std::wstring wfields = core::Utf8ToWString(field_list_utf8);
        std::wstring wtexts = core::Utf8ToWString(text_list_utf8);

        VARIANT args[2];
        VariantInit(&args[0]);
        VariantInit(&args[1]);
        args[0].vt = VT_BSTR; args[0].bstrVal = SysAllocString(wtexts.c_str());
        args[1].vt = VT_BSTR; args[1].bstrVal = SysAllocString(wfields.c_str());

        VARIANT result;
        VariantInit(&result);
        bool ok = InvokeMethod(L"PutFieldText", args, 2, &result);
        VariantClear(&args[0]);
        VariantClear(&args[1]);
        VariantClear(&result);
        return ok;
    }

    std::string GetFieldText(const std::string& field_list_utf8) override {
        if (!dispatch_) return {};
        std::wstring wfields = core::Utf8ToWString(field_list_utf8);
        VARIANT arg;
        VariantInit(&arg);
        arg.vt = VT_BSTR;
        arg.bstrVal = SysAllocString(wfields.c_str());

        VARIANT result;
        VariantInit(&result);
        std::string out_utf8;
        if (InvokeMethod(L"GetFieldText", &arg, 1, &result) && result.vt == VT_BSTR && result.bstrVal) {
            out_utf8 = core::WStringToUtf8(result.bstrVal);
        }
        VariantClear(&arg);
        VariantClear(&result);
        return out_utf8;
    }

    std::string GetFieldList(int number_opt, int field_opt) override {
        if (!dispatch_) return {};
        VARIANT args[2];
        VariantInit(&args[0]);
        VariantInit(&args[1]);
        args[0].vt = VT_I4; args[0].lVal = field_opt;
        args[1].vt = VT_I4; args[1].lVal = number_opt;

        VARIANT result;
        VariantInit(&result);
        std::string out_utf8;
        if (InvokeMethod(L"GetFieldList", args, 2, &result) && result.vt == VT_BSTR && result.bstrVal) {
            out_utf8 = core::WStringToUtf8(result.bstrVal);
        }
        VariantClear(&result);
        return out_utf8;
    }

    bool RunAction(const std::string& action_id_utf8) override {
        if (!dispatch_) return false;
        std::wstring wact = core::Utf8ToWString(action_id_utf8);
        VARIANT arg;
        VariantInit(&arg);
        arg.vt = VT_BSTR;
        arg.bstrVal = SysAllocString(wact.c_str());

        VARIANT result;
        VariantInit(&result);
        bool ok = InvokeMethod(L"Run", &arg, 1, &result);
        VariantClear(&arg);
        VariantClear(&result);
        return ok;
    }

private:
    bool InvokeMethod(const wchar_t* method_name, VARIANT* rev_args, UINT arg_count, VARIANT* out_result) {
        if (!dispatch_) return false;
        LPOLESTR names[1] = {const_cast<LPOLESTR>(method_name)};
        DISPID dispid = 0;
        if (FAILED(dispatch_->GetIDsOfNames(IID_NULL, names, 1, LOCALE_USER_DEFAULT, &dispid))) {
            return false;
        }
        DISPPARAMS params{};
        params.rgvarg = rev_args;
        params.cArgs = arg_count;
        HRESULT hr = dispatch_->Invoke(dispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, out_result, nullptr, nullptr);
        return SUCCEEDED(hr);
    }

    bool com_initialized_ = false;
    IDispatch* dispatch_ = nullptr;
};
#endif

class Win32PlatformService final : public IPlatformService {
public:
    HWND CreateViewerWindow(
        int x,
        int y,
        int width,
        int height,
        unsigned long style,
        HWND parent_hwnd,
        const ViewerWindowState& initial_state) override {
#if defined(_WIN32)
        std::wstring class_name = core::Utf8ToWString("STATIC");
        std::wstring title = core::Utf8ToWString(initial_state.rendered_preview_utf8);
        DWORD win_style = static_cast<DWORD>(style);
        if (parent_hwnd != nullptr) {
            win_style |= WS_CHILD | WS_VISIBLE | WS_BORDER;
        }
        HWND hwnd = CreateWindowExW(
            0,
            class_name.c_str(),
            title.c_str(),
            win_style,
            x,
            y,
            width,
            height,
             static_cast<HWND>(parent_hwnd),
            nullptr,
            GetModuleHandleW(nullptr),
            nullptr);
        if (hwnd != nullptr) {
            return hwnd;
        }
#else
        (void)parent_hwnd;
#endif
        std::lock_guard<std::mutex> lock(mutex_);
        uintptr_t id = next_fallback_id_++;
        ViewerWindowState st = initial_state;
        st.x = x;
        st.y = y;
        st.width = width;
        st.height = height;
        st.style = style;
        fallback_windows_[id] = std::move(st);
        return reinterpret_cast<HWND>(id);
    }

    void DestroyViewerWindow(HWND hwnd) override {
#if defined(_WIN32)
        if (hwnd && IsWindow(static_cast<HWND>(hwnd))) {
            DestroyWindow(static_cast<HWND>(hwnd));
            return;
        }
#endif
        std::lock_guard<std::mutex> lock(mutex_);
        fallback_windows_.erase(reinterpret_cast<uintptr_t>(hwnd));
    }

    void ResizeViewerWindow(HWND hwnd, int x, int y, int width, int height) override {
#if defined(_WIN32)
        if (hwnd && IsWindow(static_cast<HWND>(hwnd))) {
            MoveWindow(static_cast<HWND>(hwnd), x, y, width, height, TRUE);
            return;
        }
#endif
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = fallback_windows_.find(reinterpret_cast<uintptr_t>(hwnd));
        if (it != fallback_windows_.end()) {
            it->second.x = x;
            it->second.y = y;
            it->second.width = width;
            it->second.height = height;
        }
    }

    void SetViewerFocus(HWND hwnd) override {
#if defined(_WIN32)
        if (hwnd && IsWindow(static_cast<HWND>(hwnd))) {
            SetFocus(static_cast<HWND>(hwnd));
        }
#else
        (void)hwnd;
#endif
    }

    void SetViewerVisible(HWND hwnd, bool visible) override {
#if defined(_WIN32)
        if (hwnd && IsWindow(static_cast<HWND>(hwnd))) {
            ShowWindow(static_cast<HWND>(hwnd), visible ? SW_SHOW : SW_HIDE);
            return;
        }
#endif
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = fallback_windows_.find(reinterpret_cast<uintptr_t>(hwnd));
        if (it != fallback_windows_.end()) {
            it->second.visible = visible;
        }
    }

    void UpdateViewerDocumentText(HWND hwnd, const std::string& preview_text_utf8) override {
#if defined(_WIN32)
        if (hwnd && IsWindow(static_cast<HWND>(hwnd))) {
            std::wstring wtext = core::Utf8ToWString(preview_text_utf8);
            SetWindowTextW(static_cast<HWND>(hwnd), wtext.c_str());
            return;
        }
#endif
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = fallback_windows_.find(reinterpret_cast<uintptr_t>(hwnd));
        if (it != fallback_windows_.end()) {
            it->second.rendered_preview_utf8 = preview_text_utf8;
        }
    }

    bool SetClipboardTextUtf8(const std::string& text_utf8) override {
#if defined(_WIN32)
        if (OpenClipboard(nullptr)) {
            EmptyClipboard();
            std::wstring wtext = core::Utf8ToWString(text_utf8);
            size_t bytes = (wtext.size() + 1) * sizeof(wchar_t);
            HGLOBAL hmem = GlobalAlloc(GMEM_MOVEABLE, bytes);
            if (hmem) {
                void* ptr = GlobalLock(hmem);
                if (ptr) {
                    memcpy(ptr, wtext.c_str(), bytes);
                    GlobalUnlock(hmem);
                    SetClipboardData(CF_UNICODETEXT, hmem);
                }
            }
            CloseClipboard();
            return true;
        }
#endif
        std::lock_guard<std::mutex> lock(mutex_);
        clipboard_utf8_ = text_utf8;
        return true;
    }

    std::string GetClipboardTextUtf8() override {
#if defined(_WIN32)
        if (OpenClipboard(nullptr)) {
            HANDLE hdata = GetClipboardData(CF_UNICODETEXT);
            std::string out_utf8;
            if (hdata) {
                const wchar_t* wptr = static_cast<const wchar_t*>(GlobalLock(hdata));
                if (wptr) {
                    out_utf8 = core::WStringToUtf8(wptr);
                    GlobalUnlock(hdata);
                }
            }
            CloseClipboard();
            if (!out_utf8.empty()) {
                return out_utf8;
            }
        }
#endif
        std::lock_guard<std::mutex> lock(mutex_);
        return clipboard_utf8_;
    }

    std::vector<std::string> GetSystemFontNamesUtf8() override {
        return {
            "함초롬바탕",
            "함초롬돋움",
            "맑은 고딕",
            "바탕",
            "돋움",
            "굴림",
            "궁서",
            "Arial",
            "Times New Roman",
        };
    }

    std::unique_ptr<IComHwpBridge> CreateComHwpBridge() override {
#if defined(_WIN32)
        return std::make_unique<Win32ComHwpBridge>();
#else
        return nullptr;
#endif
    }

private:
    std::mutex mutex_;
    uintptr_t next_fallback_id_ = 0x2000;
    std::map<uintptr_t, ViewerWindowState> fallback_windows_;
    std::string clipboard_utf8_;
};

} // namespace

IPlatformService& GetPlatformService() {
    static Win32PlatformService instance;
    return instance;
}

} // namespace openhwp::platform
