#include "utf_conv.hpp"

#include <algorithm>
#include <cstring>

namespace openhwp::utf {

std::string ToUtf8(std::u16string_view u16_str) {
    std::string out;
    out.reserve(u16_str.size() * 3);

    for (size_t i = 0; i < u16_str.size(); ++i) {
        uint32_t cp = static_cast<uint16_t>(u16_str[i]);
        if (cp >= 0xD800 && cp <= 0xDBFF) {
            if (i + 1 < u16_str.size()) {
                uint32_t low = static_cast<uint16_t>(u16_str[i + 1]);
                if (low >= 0xDC00 && low <= 0xDFFF) {
                    cp = 0x10000 + (((cp - 0xD800) << 10) | (low - 0xDC00));
                    ++i;
                }
            }
        }

        if (cp <= 0x7F) {
            out.push_back(static_cast<char>(cp));
        } else if (cp <= 0x7FF) {
            out.push_back(static_cast<char>(0xC0 | ((cp >> 6) & 0x1F)));
            out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
        } else if (cp <= 0xFFFF) {
            out.push_back(static_cast<char>(0xE0 | ((cp >> 12) & 0x0F)));
            out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
            out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
        } else {
            out.push_back(static_cast<char>(0xF0 | ((cp >> 18) & 0x07)));
            out.push_back(static_cast<char>(0x80 | ((cp >> 12) & 0x3F)));
            out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
            out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
        }
    }
    return out;
}

std::string ToUtf8(const char16_t* u16_ptr) {
    if (u16_ptr == nullptr) {
        return {};
    }
    return ToUtf8(std::u16string_view(u16_ptr));
}

std::u16string ToUtf16(std::string_view utf8_str) {
    std::u16string out;
    out.reserve(utf8_str.size());

    size_t i = 0;
    const size_t n = utf8_str.size();
    while (i < n) {
        uint8_t c = static_cast<uint8_t>(utf8_str[i]);
        uint32_t cp = 0;
        size_t extra = 0;

        if ((c & 0x80) == 0) {
            cp = c;
            extra = 0;
        } else if ((c & 0xE0) == 0xC0) {
            cp = c & 0x1F;
            extra = 1;
        } else if ((c & 0xF0) == 0xE0) {
            cp = c & 0x0F;
            extra = 2;
        } else if ((c & 0xF8) == 0xF0) {
            cp = c & 0x07;
            extra = 3;
        } else {
            cp = 0xFFFD;
            extra = 0;
        }

        if (i + extra >= n) {
            cp = 0xFFFD;
            extra = 0;
        } else {
            for (size_t j = 1; j <= extra; ++j) {
                uint8_t cc = static_cast<uint8_t>(utf8_str[i + j]);
                if ((cc & 0xC0) != 0x80) {
                    cp = 0xFFFD;
                    extra = j - 1;
                    break;
                }
                cp = (cp << 6) | (cc & 0x3F);
            }
        }
        i += 1 + extra;

        if (cp <= 0xFFFF) {
            out.push_back(static_cast<char16_t>(cp));
        } else if (cp <= 0x10FFFF) {
            cp -= 0x10000;
            out.push_back(static_cast<char16_t>(0xD800 | ((cp >> 10) & 0x3FF)));
            out.push_back(static_cast<char16_t>(0xDC00 | (cp & 0x3FF)));
        } else {
            out.push_back(static_cast<char16_t>(0xFFFD));
        }
    }
    return out;
}

int WriteToUtf16Buffer(std::string_view utf8_str, char16_t* out_buf, int buf_size) {
    std::u16string u16 = ToUtf16(utf8_str);
    const int required = static_cast<int>(u16.size()) + 1;
    if (out_buf == nullptr || buf_size <= 0) {
        return required;
    }
    const int copy_len = std::min(static_cast<int>(u16.size()), buf_size - 1);
    if (copy_len > 0) {
        std::memcpy(out_buf, u16.data(), static_cast<size_t>(copy_len) * sizeof(char16_t));
    }
    out_buf[copy_len] = u'\0';
    return required;
}

std::filesystem::path PathFromUtf16(const char16_t* u16_path) {
    if (u16_path == nullptr) {
        return {};
    }
    std::string utf8 = ToUtf8(u16_path);
    std::u8string u8(reinterpret_cast<const char8_t*>(utf8.data()), utf8.size());
    return std::filesystem::path(u8);
}

std::string PathToUtf8(const std::filesystem::path& path) {
    std::u8string u8 = path.u8string();
    return std::string(reinterpret_cast<const char*>(u8.data()), u8.size());
}

} // namespace openhwp::utf
