#pragma once

#include <cstdint>
#include <filesystem>
#include <map>
#include <string>
#include <vector>

#include "../core/hwp_dom.hpp"

namespace openhwp::formats {

/**
 * @brief HWP 5.0 (OLE2 Compound File Binary + zlib 레코드 스트림) 리더/라이터
 */
class Hwp5FormatHandler {
public:
    /**
     * @brief 파일이 유효한 HWP 5.0 (OLE2 CFB) 파일인지 확인합니다.
     */
    static bool IsHwp5File(const std::filesystem::path& path);

    /**
     * @brief HWP 5.0 헤더 버전 및 암호화 정보를 조회합니다 (GetFileInfo 연동).
     */
    static bool InspectHeader(
        const std::filesystem::path& path,
        std::string& out_version_str,
        unsigned int& out_version_num,
        int& out_encrypted);

    /**
     * @brief HWP 5.0 파일을 읽어 DocumentModel에 로드합니다.
     */
    static bool LoadFromFile(const std::filesystem::path& path, core::DocumentModel& model);

    /**
     * @brief DocumentModel의 내용을 표준 HWP 5.0 (OLE2 CFB + Section0 레코드 스트림) 파일로 저장합니다.
     */
    static bool SaveToFile(const std::filesystem::path& path, const core::DocumentModel& model);

    /**
     * @brief HWP 5.0 바이너리 바이트 배열과 Base64 문자열 간 변환 (GetTextFile / SetTextFile "HWP" 포맷 지원)
     */
    static std::string EncodeBase64(const std::vector<uint8_t>& data);
    static std::vector<uint8_t> DecodeBase64(const std::string& b64);
};

} // namespace openhwp::formats
