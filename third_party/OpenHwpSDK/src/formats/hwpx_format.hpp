#pragma once

#include <cstdint>
#include <filesystem>
#include <map>
#include <string>
#include <vector>

#include "../core/hwp_dom.hpp"

namespace openhwp::formats {

/**
 * @brief HWPX (OWPML 표준 개방형 한글 문서: ZIP + XML) 및 HWPML2X 리더/라이터
 */
class HwpxFormatHandler {
public:
    /**
     * @brief 파일이 유효한 HWPX(ZIP 컨테이너) 파일인지 확인합니다.
     */
    static bool IsHwpxFile(const std::filesystem::path& path);

    /**
     * @brief HWPX 파일을 읽어 DocumentModel에 로드합니다.
     */
    static bool LoadFromFile(const std::filesystem::path& path, core::DocumentModel& model);

    /**
     * @brief DocumentModel의 내용을 표준 HWPX(ZIP + OWPML XML) 파일로 저장합니다.
     */
    static bool SaveToFile(const std::filesystem::path& path, const core::DocumentModel& model);

    /**
     * @brief DocumentModel을 HWPML2X / OWPML Section XML 문자열(UTF-8)로 직렬화합니다.
     */
    static std::string SerializeSectionXmlUtf8(const core::DocumentModel& model);

    /**
     * @brief HWPML2X / OWPML Section XML 문자열(UTF-8)을 파싱하여 DocumentModel에 반영합니다.
     */
    static bool ParseSectionXmlUtf8(const std::string& xml_utf8, core::DocumentModel& model, bool append_mode = false);

    /**
     * @brief 간단한 인메모리 ZIP 아카이브 읽기/쓰기 유틸리티 (zlib 기반)
     */
    static bool ReadZipEntries(const std::vector<uint8_t>& zip_bytes, std::map<std::string, std::string>& out_entries);
    static std::vector<uint8_t> WriteZipArchive(const std::vector<std::pair<std::string, std::string>>& entries);
};

} // namespace openhwp::formats
