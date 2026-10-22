#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <string_view>

namespace openhwp::utf {

/**
 * @brief UTF-16 (char16_t) 문자열을 내부 표준 UTF-8 (std::string)로 변환합니다.
 */
std::string ToUtf8(std::u16string_view u16_str);

/**
 * @brief 널 종료 UTF-16 포인터(const char16_t*)를 내부 표준 UTF-8 (std::string)로 변환합니다.
 */
std::string ToUtf8(const char16_t* u16_ptr);

/**
 * @brief 내부 표준 UTF-8 (std::string_view)을 UTF-16 (std::u16string)으로 변환합니다.
 */
std::u16string ToUtf16(std::string_view utf8_str);

/**
 * @brief UTF-8 문자열 결과를 HWPSDK 호환 char16_t 출력 버퍼에 복사합니다.
 * @param utf8_str 원본 UTF-8 문자열
 * @param out_buf 호출자 제공 UTF-16 버퍼 (nullptr이면 필요한 버퍼 크기만 반환)
 * @param buf_size 버퍼 크기 (char16_t 원소 개수 단위)
 * @return 널 종료 문자를 포함한 char16_t 원소 개수 (또는 복사된 길이)
 */
int WriteToUtf16Buffer(std::string_view utf8_str, char16_t* out_buf, int buf_size);

/**
 * @brief UTF-16 경로 문자열을 크로스 플랫폼 std::filesystem::path로 안전하게 변환합니다.
 */
std::filesystem::path PathFromUtf16(const char16_t* u16_path);

/**
 * @brief std::filesystem::path를 내부 표준 UTF-8 문자열로 변환합니다.
 */
std::string PathToUtf8(const std::filesystem::path& path);

} // namespace openhwp::utf

namespace openhwp::core {

inline std::string Utf16ToUtf8(const char16_t* u16_ptr) {
    return utf::ToUtf8(u16_ptr);
}

inline std::u16string Utf8ToUtf16(std::string_view utf8_str) {
    return utf::ToUtf16(utf8_str);
}

inline int CopyUtf8ToChar16Buffer(std::string_view utf8_str, char16_t* out_buf, int buf_size) {
    return utf::WriteToUtf16Buffer(utf8_str, out_buf, buf_size);
}

inline std::filesystem::path Utf16ToPath(const char16_t* u16_path) {
    return utf::PathFromUtf16(u16_path);
}

inline std::u16string PathToUtf16(const std::filesystem::path& path) {
    return utf::ToUtf16(utf::PathToUtf8(path));
}

inline std::wstring Utf8ToWString(std::string_view utf8_str) {
    std::u16string u16 = utf::ToUtf16(utf8_str);
    return std::wstring(u16.begin(), u16.end());
}

inline std::string WStringToUtf8(const wchar_t* wstr) {
    if (!wstr) return {};
    std::u16string u16;
    while (*wstr) {
        u16.push_back(static_cast<char16_t>(*wstr++));
    }
    return utf::ToUtf8(u16);
}

} // namespace openhwp::core
