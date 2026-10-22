#include "hwp5_format.hpp"
#include "hwpx_format.hpp"
#include "../core/utf_conv.hpp"

#include <algorithm>
#include <cstring>
#include <fstream>
#include <sstream>
#include <zlib.h>

namespace openhwp::formats {

namespace {

constexpr uint8_t kOleMagic[8] = {0xD0, 0xCF, 0x11, 0xE0, 0xA1, 0xB1, 0x1A, 0xE1};
constexpr uint32_t kEndOfChain = 0xFFFFFFFE;
constexpr uint32_t kFreeSect = 0xFFFFFFFF;
constexpr uint32_t kFatSect = 0xFFFFFFFD;

// HWP 5.0 Tag IDs (HWPTAG_BEGIN = 16)
constexpr uint16_t kHwpTagBegin = 16;
constexpr uint16_t kHwpTagParaHeader = kHwpTagBegin + 50; // 66
constexpr uint16_t kHwpTagParaText = kHwpTagBegin + 51;   // 67
constexpr uint16_t kHwpTagCtrlHeader = kHwpTagBegin + 55; // 71
constexpr uint16_t kHwpTagListHeader = kHwpTagBegin + 56; // 72
constexpr uint16_t kHwpTagTable = kHwpTagBegin + 61;      // 77

uint16_t ReadLe16(const uint8_t* p) {
    return static_cast<uint16_t>(p[0]) | (static_cast<uint16_t>(p[1]) << 8);
}

uint32_t ReadLe32(const uint8_t* p) {
    return static_cast<uint32_t>(p[0]) |
           (static_cast<uint32_t>(p[1]) << 8) |
           (static_cast<uint32_t>(p[2]) << 16) |
           (static_cast<uint32_t>(p[3]) << 24);
}

void WriteLe16(uint8_t* p, uint16_t v) {
    p[0] = static_cast<uint8_t>(v & 0xFF);
    p[1] = static_cast<uint8_t>((v >> 8) & 0xFF);
}

void WriteLe32(uint8_t* p, uint32_t v) {
    p[0] = static_cast<uint8_t>(v & 0xFF);
    p[1] = static_cast<uint8_t>((v >> 8) & 0xFF);
    p[2] = static_cast<uint8_t>((v >> 16) & 0xFF);
    p[3] = static_cast<uint8_t>((v >> 24) & 0xFF);
}

void AppendLe16(std::vector<uint8_t>& buf, uint16_t v) {
    buf.push_back(static_cast<uint8_t>(v & 0xFF));
    buf.push_back(static_cast<uint8_t>((v >> 8) & 0xFF));
}

void AppendLe32(std::vector<uint8_t>& buf, uint32_t v) {
    buf.push_back(static_cast<uint8_t>(v & 0xFF));
    buf.push_back(static_cast<uint8_t>((v >> 8) & 0xFF));
    buf.push_back(static_cast<uint8_t>((v >> 16) & 0xFF));
    buf.push_back(static_cast<uint8_t>((v >> 24) & 0xFF));
}

bool ZlibRawInflate(const std::vector<uint8_t>& input, std::vector<uint8_t>& output) {
    if (input.empty()) {
        output.clear();
        return true;
    }
    z_stream strm{};
    if (inflateInit2(&strm, -MAX_WBITS) != Z_OK) {
        return false;
    }
    output.clear();
    std::vector<uint8_t> chunk(16384);
    strm.next_in = const_cast<Bytef*>(reinterpret_cast<const Bytef*>(input.data()));
    strm.avail_in = static_cast<uInt>(input.size());

    int ret = Z_OK;
    while (ret == Z_OK) {
        strm.next_out = reinterpret_cast<Bytef*>(chunk.data());
        strm.avail_out = static_cast<uInt>(chunk.size());
        ret = inflate(&strm, Z_NO_FLUSH);
        if (ret != Z_OK && ret != Z_STREAM_END && ret != Z_BUF_ERROR) {
            inflateEnd(&strm);
            return false;
        }
        size_t have = chunk.size() - strm.avail_out;
        output.insert(output.end(), chunk.begin(), chunk.begin() + static_cast<std::ptrdiff_t>(have));
        if (ret == Z_BUF_ERROR && strm.avail_in == 0) {
            break;
        }
    }
    inflateEnd(&strm);
    return true;
}

bool ZlibRawDeflate(const std::vector<uint8_t>& input, std::vector<uint8_t>& output) {
    z_stream strm{};
    if (deflateInit2(&strm, Z_DEFAULT_COMPRESSION, Z_DEFLATED, -MAX_WBITS, 8, Z_DEFAULT_STRATEGY) != Z_OK) {
        return false;
    }
    output.resize(deflateBound(&strm, static_cast<uLong>(input.size())) + 16);
    strm.next_in = const_cast<Bytef*>(reinterpret_cast<const Bytef*>(input.data()));
    strm.avail_in = static_cast<uInt>(input.size());
    strm.next_out = reinterpret_cast<Bytef*>(output.data());
    strm.avail_out = static_cast<uInt>(output.size());

    int ret = deflate(&strm, Z_FINISH);
    deflateEnd(&strm);
    if (ret != Z_STREAM_END) {
        return false;
    }
    output.resize(strm.total_out);
    return true;
}

struct CfbDirEntry {
    std::string name_utf8;
    uint8_t obj_type = 0; // 1: Storage, 2: Stream, 5: Root
    uint32_t start_sect = kEndOfChain;
    uint32_t stream_size = 0;
};

bool ReadCfbFile(const std::filesystem::path& path, std::map<std::string, std::vector<uint8_t>>& out_streams) {
    out_streams.clear();
    std::ifstream ifs(path, std::ios::binary | std::ios::ate);
    if (!ifs) {
        return false;
    }
    std::streamsize file_len = ifs.tellg();
    if (file_len < 512) {
        return false;
    }
    ifs.seekg(0, std::ios::beg);
    std::vector<uint8_t> data(static_cast<size_t>(file_len));
    if (!ifs.read(reinterpret_cast<char*>(data.data()), file_len)) {
        return false;
    }

    if (std::memcmp(data.data(), kOleMagic, 8) != 0) {
        return false;
    }

    uint16_t sector_shift = ReadLe16(&data[30]);
    uint16_t mini_shift = ReadLe16(&data[32]);
    size_t sector_size = static_cast<size_t>(1) << sector_shift;
    size_t mini_sector_size = static_cast<size_t>(1) << mini_shift;
    if (sector_size < 512 || sector_size > 4096) {
        return false;
    }

    uint32_t num_fat_sects = ReadLe32(&data[44]);
    uint32_t first_dir_sect = ReadLe32(&data[48]);
    uint32_t mini_cutoff = ReadLe32(&data[56]);
    uint32_t first_minifat_sect = ReadLe32(&data[60]);
    uint32_t num_minifat_sects = ReadLe32(&data[64]);
    uint32_t first_difat_sect = ReadLe32(&data[68]);
    uint32_t num_difat_sects = ReadLe32(&data[72]);

    auto sector_offset = [&](uint32_t sect_id) -> size_t {
        return static_cast<size_t>(sect_id + 1) * sector_size;
    };

    // Collect DIFAT -> FAT sector IDs
    std::vector<uint32_t> fat_sector_ids;
    for (int i = 0; i < 109 && fat_sector_ids.size() < num_fat_sects; ++i) {
        uint32_t sid = ReadLe32(&data[76 + i * 4]);
        if (sid != kFreeSect && sid != kEndOfChain) {
            fat_sector_ids.push_back(sid);
        }
    }
    uint32_t cur_difat = first_difat_sect;
    size_t entries_per_sect = sector_size / 4;
    for (uint32_t d = 0; d < num_difat_sects && cur_difat != kEndOfChain && cur_difat != kFreeSect; ++d) {
        size_t off = sector_offset(cur_difat);
        if (off + sector_size > data.size()) {
            break;
        }
        for (size_t i = 0; i + 1 < entries_per_sect && fat_sector_ids.size() < num_fat_sects; ++i) {
            uint32_t sid = ReadLe32(&data[off + i * 4]);
            if (sid != kFreeSect && sid != kEndOfChain) {
                fat_sector_ids.push_back(sid);
            }
        }
        cur_difat = ReadLe32(&data[off + (entries_per_sect - 1) * 4]);
    }

    // Build FAT table
    std::vector<uint32_t> fat;
    fat.reserve(fat_sector_ids.size() * entries_per_sect);
    for (uint32_t sid : fat_sector_ids) {
        size_t off = sector_offset(sid);
        if (off + sector_size > data.size()) {
            break;
        }
        for (size_t i = 0; i < entries_per_sect; ++i) {
            fat.push_back(ReadLe32(&data[off + i * 4]));
        }
    }

    auto read_chain = [&](uint32_t start_sect, uint32_t byte_len, bool exact_len) -> std::vector<uint8_t> {
        std::vector<uint8_t> out;
        uint32_t cur = start_sect;
        size_t max_steps = fat.size() + 1;
        size_t steps = 0;
        while (cur != kEndOfChain && cur != kFreeSect && cur < fat.size() && steps < max_steps) {
            size_t off = sector_offset(cur);
            if (off + sector_size > data.size()) {
                break;
            }
            out.insert(out.end(), data.begin() + static_cast<std::ptrdiff_t>(off),
                       data.begin() + static_cast<std::ptrdiff_t>(off + sector_size));
            if (exact_len && out.size() >= byte_len) {
                break;
            }
            cur = fat[cur];
            ++steps;
        }
        if (exact_len && out.size() > byte_len) {
            out.resize(byte_len);
        }
        return out;
    };

    // Read Directory Stream
    std::vector<uint8_t> dir_bytes = read_chain(first_dir_sect, 0, false);
    if (dir_bytes.size() < 128) {
        return false;
    }

    std::vector<CfbDirEntry> dir_entries;
    for (size_t off = 0; off + 128 <= dir_bytes.size(); off += 128) {
        uint8_t obj_type = dir_bytes[off + 66];
        if (obj_type == 0) {
            continue;
        }
        uint16_t name_cb = ReadLe16(&dir_bytes[off + 64]);
        std::u16string name_u16;
        if (name_cb >= 2 && name_cb <= 64) {
            size_t chars = (name_cb / 2) - 1;
            for (size_t i = 0; i < chars; ++i) {
                char16_t ch = static_cast<char16_t>(ReadLe16(&dir_bytes[off + i * 2]));
                if (ch == 0) {
                    break;
                }
                name_u16.push_back(ch);
            }
        }
        CfbDirEntry entry;
        entry.name_utf8 = core::Utf16ToUtf8(name_u16.c_str());
        entry.obj_type = obj_type;
        entry.start_sect = ReadLe32(&dir_bytes[off + 116]);
        entry.stream_size = ReadLe32(&dir_bytes[off + 120]);
        dir_entries.push_back(std::move(entry));
    }

    if (dir_entries.empty()) {
        return false;
    }

    // Build MiniFAT and MiniStream if present
    std::vector<uint32_t> minifat;
    if (first_minifat_sect != kEndOfChain && first_minifat_sect != kFreeSect && num_minifat_sects > 0) {
        std::vector<uint8_t> mf_bytes = read_chain(first_minifat_sect, 0, false);
        for (size_t i = 0; i + 4 <= mf_bytes.size(); i += 4) {
            minifat.push_back(ReadLe32(&mf_bytes[i]));
        }
    }

    std::vector<uint8_t> mini_stream;
    if (dir_entries[0].obj_type == 5 &&
        dir_entries[0].start_sect != kEndOfChain &&
        dir_entries[0].start_sect != kFreeSect &&
        dir_entries[0].stream_size > 0) {
        mini_stream = read_chain(dir_entries[0].start_sect, dir_entries[0].stream_size, true);
    }

    auto read_mini_chain = [&](uint32_t start_mini_sect, uint32_t byte_len) -> std::vector<uint8_t> {
        std::vector<uint8_t> out;
        uint32_t cur = start_mini_sect;
        size_t steps = 0;
        size_t max_steps = minifat.size() + 1;
        while (cur != kEndOfChain && cur != kFreeSect && cur < minifat.size() && steps < max_steps) {
            size_t off = static_cast<size_t>(cur) * mini_sector_size;
            if (off >= mini_stream.size()) {
                break;
            }
            size_t avail = std::min(mini_sector_size, mini_stream.size() - off);
            out.insert(out.end(),
                       mini_stream.begin() + static_cast<std::ptrdiff_t>(off),
                       mini_stream.begin() + static_cast<std::ptrdiff_t>(off + avail));
            if (out.size() >= byte_len) {
                break;
            }
            cur = minifat[cur];
            ++steps;
        }
        if (out.size() > byte_len) {
            out.resize(byte_len);
        }
        return out;
    };

    for (const auto& entry : dir_entries) {
        if (entry.obj_type != 2) { // Stream only
            continue;
        }
        if (entry.stream_size == 0) {
            out_streams[entry.name_utf8] = {};
            continue;
        }
        if (entry.stream_size < mini_cutoff && !mini_stream.empty() && !minifat.empty()) {
            out_streams[entry.name_utf8] = read_mini_chain(entry.start_sect, entry.stream_size);
        } else {
            out_streams[entry.name_utf8] = read_chain(entry.start_sect, entry.stream_size, true);
        }
    }

    return !out_streams.empty();
}

void WriteDirEntry(
    uint8_t* entry_buf,
    const std::string& name_utf8,
    uint8_t obj_type,
    uint8_t color,
    uint32_t left_id,
    uint32_t right_id,
    uint32_t child_id,
    uint32_t start_sect,
    uint32_t stream_size) {
    std::memset(entry_buf, 0, 128);
    std::u16string u16 = core::Utf8ToUtf16(name_utf8);
    size_t max_chars = std::min<size_t>(u16.size(), 31);
    for (size_t i = 0; i < max_chars; ++i) {
        WriteLe16(&entry_buf[i * 2], static_cast<uint16_t>(u16[i]));
    }
    WriteLe16(&entry_buf[max_chars * 2], 0);
    WriteLe16(&entry_buf[64], static_cast<uint16_t>((max_chars + 1) * 2));
    entry_buf[66] = obj_type;
    entry_buf[67] = color;
    WriteLe32(&entry_buf[68], left_id);
    WriteLe32(&entry_buf[72], right_id);
    WriteLe32(&entry_buf[76], child_id);
    WriteLe32(&entry_buf[116], start_sect);
    WriteLe32(&entry_buf[120], stream_size);
}

std::vector<uint8_t> BuildCfbFile(
    const std::vector<std::pair<std::string, std::vector<uint8_t>>>& streams) {
    // We set mini_stream_cutoff = 0 in the CFB header so all streams use standard 512-byte sectors.
    // Directory Layout (2 sectors = 8 entries):
    // 0: Root Entry (Storage=5, child=1)
    // 1: BodyText (Storage=1, left=2, right=3, child=4)
    // 2: FileHeader (Stream=2)
    // 3: DocInfo (Stream=2, left=5, right=6)
    // 4: Section0 (Stream=2, inside BodyText)
    // 5: PrvText (Stream=2)
    // 6: OpenHwpXml (Stream=2)

    auto find_stream = [&](const std::string& name) -> std::vector<uint8_t> {
        for (const auto& [k, v] : streams) {
            if (k == name) {
                return v;
            }
        }
        return {};
    };

    std::vector<uint8_t> s_file_header = find_stream("FileHeader");
    std::vector<uint8_t> s_doc_info = find_stream("DocInfo");
    std::vector<uint8_t> s_section0 = find_stream("Section0");
    std::vector<uint8_t> s_prv_text = find_stream("PrvText");
    std::vector<uint8_t> s_openhwp_xml = find_stream("OpenHwpXml");

    std::vector<std::vector<uint8_t>> sectors;
    std::vector<uint32_t> fat;

    auto allocate_stream = [&](const std::vector<uint8_t>& bytes, uint32_t& out_start, uint32_t& out_size) {
        out_size = static_cast<uint32_t>(bytes.size());
        if (bytes.empty()) {
            out_start = kEndOfChain;
            return;
        }
        out_start = static_cast<uint32_t>(sectors.size());
        size_t num_sects = (bytes.size() + 511) / 512;
        for (size_t i = 0; i < num_sects; ++i) {
            std::vector<uint8_t> sect(512, 0);
            size_t off = i * 512;
            size_t copy_len = std::min<size_t>(512, bytes.size() - off);
            std::memcpy(sect.data(), &bytes[off], copy_len);
            sectors.push_back(std::move(sect));
            if (i + 1 < num_sects) {
                fat.push_back(static_cast<uint32_t>(sectors.size()));
            } else {
                fat.push_back(kEndOfChain);
            }
        }
    };

    // Reserve 2 sectors (0 and 1) for Directory
    uint32_t dir_start_sect = 0;
    sectors.push_back(std::vector<uint8_t>(512, 0));
    sectors.push_back(std::vector<uint8_t>(512, 0));
    fat.push_back(1);
    fat.push_back(kEndOfChain);

    uint32_t fh_start = 0, fh_size = 0;
    uint32_t di_start = 0, di_size = 0;
    uint32_t s0_start = 0, s0_size = 0;
    uint32_t pv_start = 0, pv_size = 0;
    uint32_t ox_start = 0, ox_size = 0;

    allocate_stream(s_file_header, fh_start, fh_size);
    allocate_stream(s_doc_info, di_start, di_size);
    allocate_stream(s_section0, s0_start, s0_size);
    allocate_stream(s_prv_text, pv_start, pv_size);
    allocate_stream(s_openhwp_xml, ox_start, ox_size);

    // Fill Directory entries in sectors[0] and sectors[1]
    WriteDirEntry(&sectors[0][0], "Root Entry", 5, 1, kFreeSect, kFreeSect, 1, kEndOfChain, 0);
    WriteDirEntry(&sectors[0][128], "BodyText", 1, 1, 2, 3, 4, kEndOfChain, 0);
    WriteDirEntry(&sectors[0][256], "FileHeader", 2, 1, kFreeSect, kFreeSect, kFreeSect, fh_start, fh_size);
    WriteDirEntry(&sectors[0][384], "DocInfo", 2, 1, 5, 6, kFreeSect, di_start, di_size);

    WriteDirEntry(&sectors[1][0], "Section0", 2, 1, kFreeSect, kFreeSect, kFreeSect, s0_start, s0_size);
    WriteDirEntry(&sectors[1][128], "PrvText", 2, 1, kFreeSect, kFreeSect, kFreeSect, pv_start, pv_size);
    WriteDirEntry(&sectors[1][256], "OpenHwpXml", 2, 1, kFreeSect, kFreeSect, kFreeSect, ox_start, ox_size);

    // Allocate FAT sectors
    size_t total_sects_estimate = sectors.size() + 4;
    size_t num_fat_sects = (total_sects_estimate + 127) / 128;
    std::vector<uint32_t> fat_sect_ids;
    for (size_t i = 0; i < num_fat_sects; ++i) {
        uint32_t sid = static_cast<uint32_t>(sectors.size() + i);
        fat_sect_ids.push_back(sid);
        fat.push_back(kFatSect);
    }

    for (size_t i = 0; i < num_fat_sects; ++i) {
        std::vector<uint8_t> fat_sect(512, 0xFF);
        for (size_t j = 0; j < 128; ++j) {
            size_t idx = i * 128 + j;
            if (idx < fat.size()) {
                WriteLe32(&fat_sect[j * 4], fat[idx]);
            }
        }
        sectors.push_back(std::move(fat_sect));
    }

    // Build 512-byte CFB Header
    std::vector<uint8_t> out(512, 0);
    std::memcpy(out.data(), kOleMagic, 8);
    WriteLe16(&out[24], 0x003E); // Minor version
    WriteLe16(&out[26], 0x0003); // Major version 3 (512-byte sectors)
    WriteLe16(&out[28], 0xFFFE); // Little-endian byte order
    WriteLe16(&out[30], 9);      // Sector shift (2^9 = 512)
    WriteLe16(&out[32], 6);      // Mini sector shift (2^6 = 64)
    WriteLe32(&out[44], static_cast<uint32_t>(num_fat_sects));
    WriteLe32(&out[48], dir_start_sect);
    WriteLe32(&out[56], 0);      // Mini stream cutoff = 0 (all streams in standard FAT)
    WriteLe32(&out[60], kEndOfChain);
    WriteLe32(&out[64], 0);
    WriteLe32(&out[68], kEndOfChain);
    WriteLe32(&out[72], 0);

    for (int i = 0; i < 109; ++i) {
        uint32_t sid = (static_cast<size_t>(i) < fat_sect_ids.size()) ? fat_sect_ids[static_cast<size_t>(i)] : kFreeSect;
        WriteLe32(&out[76 + i * 4], sid);
    }

    for (const auto& sect : sectors) {
        out.insert(out.end(), sect.begin(), sect.end());
    }
    return out;
}

void AppendHwpRecord(
    std::vector<uint8_t>& stream,
    uint16_t tag_id,
    uint16_t level,
    const std::vector<uint8_t>& payload) {
    uint32_t size = static_cast<uint32_t>(payload.size());
    if (size < 0xFFF) {
        uint32_t hdr = (static_cast<uint32_t>(tag_id) & 0x3FF) |
                       ((static_cast<uint32_t>(level) & 0x3FF) << 10) |
                       ((size & 0xFFF) << 20);
        AppendLe32(stream, hdr);
    } else {
        uint32_t hdr = (static_cast<uint32_t>(tag_id) & 0x3FF) |
                       ((static_cast<uint32_t>(level) & 0x3FF) << 10) |
                       (0xFFFU << 20);
        AppendLe32(stream, hdr);
        AppendLe32(stream, size);
    }
    stream.insert(stream.end(), payload.begin(), payload.end());
}

std::string ExtractParaTextUtf8FromHwpRecord(const uint8_t* payload, size_t len) {
    // In HWP 5.0 HWPTAG_PARA_TEXT, characters are UTF-16LE.
    // Control characters < 32:
    //  - Inline/Extended controls (1, 2, 3, 11, 12, 14, 15, 16, 17, 18, 21, 22, 23) occupy 8 WCHARs (16 bytes total)
    //  - Char controls (0, 4, 5, 6, 7, 8, 9, 10, 13, 19, 20, 24, 25, 26, 27, 28, 29, 30, 31) occupy 1 WCHAR (2 bytes)
    std::u16string u16;
    size_t i = 0;
    while (i + 1 < len) {
        uint16_t ch = ReadLe16(&payload[i]);
        if (ch < 32) {
            switch (ch) {
                case 1: case 2: case 3: case 11: case 12: case 14:
                case 15: case 16: case 17: case 18: case 21: case 22: case 23:
                    i += 16; // 8 WCHARs
                    continue;
                case 9:
                    u16.push_back(u'\t');
                    i += 2;
                    continue;
                case 10:
                    u16.push_back(u'\n');
                    i += 2;
                    continue;
                case 13:
                    // Paragraph end marker
                    i += 2;
                    continue;
                default:
                    i += 2;
                    continue;
            }
        }
        u16.push_back(static_cast<char16_t>(ch));
        i += 2;
    }
    return core::Utf16ToUtf8(u16.c_str());
}

void ParseHwpSectionStream(const std::vector<uint8_t>& sec_bytes, core::DocumentModel& model) {
    size_t pos = 0;
    while (pos + 4 <= sec_bytes.size()) {
        uint32_t hdr = ReadLe32(&sec_bytes[pos]);
        pos += 4;
        uint16_t tag_id = static_cast<uint16_t>(hdr & 0x3FF);
        uint16_t level = static_cast<uint16_t>((hdr >> 10) & 0x3FF);
        uint32_t size = (hdr >> 20) & 0xFFF;
        if (size == 0xFFF) {
            if (pos + 4 > sec_bytes.size()) {
                break;
            }
            size = ReadLe32(&sec_bytes[pos]);
            pos += 4;
        }
        if (pos + size > sec_bytes.size()) {
            break;
        }
        const uint8_t* payload = &sec_bytes[pos];

        if (tag_id == kHwpTagParaText && size > 0) {
            std::string text = ExtractParaTextUtf8FromHwpRecord(payload, size);
            if (level == 0) {
                core::ParagraphNode pnode;
                pnode.text_utf8 = text;
                model.paragraphs.push_back(std::move(pnode));
            } else if (!model.tables.empty()) {
                // Assign cell text to next empty cell in last table if applicable
                auto& tbl = model.tables.back();
                bool assigned = false;
                for (int r = 0; r < tbl.rows && !assigned; ++r) {
                    for (int c = 0; c < tbl.cols && !assigned; ++c) {
                        if (tbl.cells[r][c].text_utf8.empty()) {
                            tbl.cells[r][c].text_utf8 = text;
                            assigned = true;
                        }
                    }
                }
            }
        } else if (tag_id == kHwpTagTable && size >= 8) {
            uint16_t rows = ReadLe16(&payload[4]);
            uint16_t cols = ReadLe16(&payload[6]);
            if (rows > 0 && rows <= 256 && cols > 0 && cols <= 256) {
                core::TableNode tbl;
                tbl.inst_id_utf8 = std::to_string(1000 + model.tables.size());
                tbl.anchor_para = model.paragraphs.empty() ? 0 : static_cast<int>(model.paragraphs.size() - 1);
                tbl.InitializeGrid(rows, cols);
                model.tables.push_back(std::move(tbl));
            }
        }

        pos += size;
    }
}

} // namespace

bool Hwp5FormatHandler::IsHwp5File(const std::filesystem::path& path) {
    std::error_code ec;
    if (!std::filesystem::exists(path, ec)) {
        return false;
    }
    std::ifstream ifs(path, std::ios::binary);
    if (!ifs) {
        return false;
    }
    uint8_t magic[8] = {0};
    ifs.read(reinterpret_cast<char*>(magic), 8);
    if (ifs.gcount() < 8) {
        return false;
    }
    return std::memcmp(magic, kOleMagic, 8) == 0;
}

bool Hwp5FormatHandler::InspectHeader(
    const std::filesystem::path& path,
    std::string& out_version_str,
    unsigned int& out_version_num,
    int& out_encrypted) {
    std::map<std::string, std::vector<uint8_t>> streams;
    if (!ReadCfbFile(path, streams)) {
        return false;
    }
    auto it = streams.find("FileHeader");
    if (it == streams.end() || it->second.size() < 40) {
        return false;
    }
    const auto& fh = it->second;
    if ( std::memcmp(fh.data(), "HWP Document File", 17) != 0) {
        return false;
    }
    uint32_t ver = ReadLe32(&fh[32]);
    uint32_t flags = ReadLe32(&fh[36]);
    out_version_num = ver;
    unsigned int major = (ver >> 24) & 0xFF;
    unsigned int minor = (ver >> 16) & 0xFF;
    unsigned int build = (ver >> 8) & 0xFF;
    unsigned int rev = ver & 0xFF;
    out_version_str = std::to_string(major) + "." + std::to_string(minor) + "." +
                      std::to_string(build) + "." + std::to_string(rev);
    out_encrypted = ((flags & 0x02) != 0) ? 1 : 0;
    return true;
}

bool Hwp5FormatHandler::LoadFromFile(const std::filesystem::path& path, core::DocumentModel& model) {
    std::map<std::string, std::vector<uint8_t>> streams;
    if (!ReadCfbFile(path, streams)) {
        return false;
    }

    auto fh_it = streams.find("FileHeader");
    if (fh_it == streams.end() || fh_it->second.size() < 40) {
        return false;
    }
    uint32_t flags = ReadLe32(&fh_it->second[36]);
    bool compressed = (flags & 0x01) != 0;

    model.ClearAndReset();

    // 1. If OpenHwpXml stream exists (written by OpenHwpSDK), parse full fidelity DOM
    auto ox_it = streams.find("OpenHwpXml");
    if (ox_it != streams.end() && !ox_it->second.empty()) {
        std::vector<uint8_t> xml_bytes;
        if (compressed) {
            if (!ZlibRawInflate(ox_it->second, xml_bytes)) {
                xml_bytes = ox_it->second;
            }
        } else {
            xml_bytes = ox_it->second;
        }
        std::string xml_utf8(reinterpret_cast<const char*>(xml_bytes.data()), xml_bytes.size());
        if (HwpxFormatHandler::ParseSectionXmlUtf8(xml_utf8, model, false)) {
            model.current_path = path;
            model.current_format_utf8 = "HWP";
            model.modified = false;
            return true;
        }
    }

    // 2. Standard HWP 5.0 BodyText/Section0 parsing
    model.paragraphs.clear();
    for (const auto& [name, raw_bytes] : streams) {
        if (name.find("Section") == 0) {
            std::vector<uint8_t> sec_bytes;
            if (compressed) {
                if (!ZlibRawInflate(raw_bytes, sec_bytes)) {
                    sec_bytes = raw_bytes;
                }
            } else {
                sec_bytes = raw_bytes;
            }
            ParseHwpSectionStream(sec_bytes, model);
        }
    }

    // Fallback to PrvText if Section0 had no paragraphs
    if (model.paragraphs.empty()) {
        auto pv_it = streams.find("PrvText");
        if (pv_it != streams.end() && pv_it->second.size() >= 2) {
            std::u16string u16;
            for (size_t i = 0; i + 1 < pv_it->second.size(); i += 2) {
                char16_t ch = static_cast<char16_t>(ReadLe16(&pv_it->second[i]));
                if (ch == 0) {
                    break;
                }
                u16.push_back(ch);
            }
            core::ParagraphNode pnode;
            pnode.text_utf8 = core::Utf16ToUtf8(u16.c_str());
            model.paragraphs.push_back(std::move(pnode));
        }
    }

    if (model.paragraphs.empty()) {
        model.paragraphs.emplace_back();
    }

    model.current_path = path;
    model.current_format_utf8 = "HWP";
    model.modified = false;
    return true;
}

bool Hwp5FormatHandler::SaveToFile(const std::filesystem::path& path, const core::DocumentModel& model) {
    std::error_code ec;
    if (path.has_parent_path()) {
        std::filesystem::create_directories(path.parent_path(), ec);
    }

    // 1. Build FileHeader (256 bytes)
    std::vector<uint8_t> file_header(256, 0);
    const char kSig[] = "HWP Document File";
    std::memcpy(file_header.data(), kSig, sizeof(kSig) - 1);
    WriteLe32(&file_header[32], 0x05010001); // Version 5.1.0.1
    WriteLe32(&file_header[36], 0x00000001); // Bit 0 = Compressed

    // 2. Build DocInfo record stream and compress
    std::vector<uint8_t> doc_info_raw;
    std::vector<uint8_t> doc_props(26, 0);
    WriteLe16(&doc_props[0], 1); // 1 section
    AppendHwpRecord(doc_info_raw, kHwpTagBegin, 0, doc_props);
    std::vector<uint8_t> doc_info_comp;
    ZlibRawDeflate(doc_info_raw, doc_info_comp);

    // 3. Build Section0 record stream (HWPTAG_PARA_HEADER + HWPTAG_PARA_TEXT + HWPTAG_TABLE)
    std::vector<uint8_t> sec0_raw;
    std::u16string prv_text_u16;

    for (size_t p_idx = 0; p_idx < model.paragraphs.size(); ++p_idx) {
        const auto& para = model.paragraphs[p_idx];
        std::u16string p_u16 = core::Utf8ToUtf16(para.text_utf8);
        if (!prv_text_u16.empty() && !p_u16.empty()) {
            prv_text_u16.push_back(u'\r');
            prv_text_u16.push_back(u'\n');
        }
        prv_text_u16 += p_u16;

        std::vector<uint8_t> para_hdr(22, 0);
        uint32_t nchars = static_cast<uint32_t>(p_u16.size() + 1);
        if (p_idx + 1 == model.paragraphs.size()) {
            nchars |= 0x80000000U;
        }
        WriteLe32(&para_hdr[0], nchars);
        AppendHwpRecord(sec0_raw, kHwpTagParaHeader, 0, para_hdr);

        std::vector<uint8_t> para_txt;
        for (char16_t ch : p_u16) {
            AppendLe16(para_txt, static_cast<uint16_t>(ch));
        }
        AppendLe16(para_txt, 13); // Para break char
        AppendHwpRecord(sec0_raw, kHwpTagParaText, 0, para_txt);

        // Append any table anchored at this paragraph
        for (const auto& tbl : model.tables) {
            if (tbl.anchor_para == static_cast<int>(p_idx)) {
                std::vector<uint8_t> ctrl_hdr = {' ', 'l', 'b', 't'}; // 'tbl ' in little-endian
                AppendHwpRecord(sec0_raw, kHwpTagCtrlHeader, 1, ctrl_hdr);

                std::vector<uint8_t> tbl_rec(18, 0);
                WriteLe16(&tbl_rec[4], static_cast<uint16_t>(tbl.rows));
                WriteLe16(&tbl_rec[6], static_cast<uint16_t>(tbl.cols));
                AppendHwpRecord(sec0_raw, kHwpTagTable, 1, tbl_rec);
            }
        }
    }

    std::vector<uint8_t> sec0_comp;
    ZlibRawDeflate(sec0_raw, sec0_comp);

    // 4. Build PrvText UTF-16LE stream
    std::vector<uint8_t> prv_text_bytes;
    for (char16_t ch : prv_text_u16) {
        AppendLe16(prv_text_bytes, static_cast<uint16_t>(ch));
    }
    AppendLe16(prv_text_bytes, 0);

    // 5. Build OpenHwpXml stream (compressed OWPML XML for 100% lossless round-trip of fields/tables/shapes)
    std::string xml_utf8 = HwpxFormatHandler::SerializeSectionXmlUtf8(model);
    std::vector<uint8_t> xml_raw(xml_utf8.begin(), xml_utf8.end());
    std::vector<uint8_t> xml_comp;
    ZlibRawDeflate(xml_raw, xml_comp);

    std::vector<std::pair<std::string, std::vector<uint8_t>>> cfb_streams = {
        {"FileHeader", file_header},
        {"DocInfo", doc_info_comp},
        {"Section0", sec0_comp},
        {"PrvText", prv_text_bytes},
        {"OpenHwpXml", xml_comp},
    };

    std::vector<uint8_t> cfb_bytes = BuildCfbFile(cfb_streams);
    std::ofstream ofs(path, std::ios::binary | std::ios::trunc);
    if (!ofs) {
        return false;
    }
    ofs.write(reinterpret_cast<const char*>(cfb_bytes.data()), static_cast<std::streamsize>(cfb_bytes.size()));
    return ofs.good();
}

std::string Hwp5FormatHandler::EncodeBase64(const std::vector<uint8_t>& data) {
    static constexpr char kTable[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string out;
    out.reserve(((data.size() + 2) / 3) * 4);
    for (size_t i = 0; i < data.size(); i += 3) {
        uint32_t n = static_cast<uint32_t>(data[i]) << 16;
        if (i + 1 < data.size()) {
            n |= static_cast<uint32_t>(data[i + 1]) << 8;
        }
        if (i + 2 < data.size()) {
            n |= static_cast<uint32_t>(data[i + 2]);
        }
        out.push_back(kTable[(n >> 18) & 0x3F]);
        out.push_back(kTable[(n >> 12) & 0x3F]);
        out.push_back((i + 1 < data.size()) ? kTable[(n >> 6) & 0x3F] : '=');
        out.push_back((i + 2 < data.size()) ? kTable[n & 0x3F] : '=');
    }
    return out;
}

std::vector<uint8_t> Hwp5FormatHandler::DecodeBase64(const std::string& b64) {
    auto val_of = [](unsigned char c) -> int {
        if (c >= 'A' && c <= 'Z') return c - 'A';
        if (c >= 'a' && c <= 'z') return c - 'a' + 26;
        if (c >= '0' && c <= '9') return c - '0' + 52;
        if (c == '+') return 62;
        if (c == '/') return 63;
        return -1;
    };

    std::vector<uint8_t> out;
    uint32_t buf = 0;
    int bits = 0;
    for (unsigned char c : b64) {
        if (c == '=') {
            break;
        }
        int v = val_of(c);
        if (v < 0) {
            continue;
        }
        buf = (buf << 6) | static_cast<uint32_t>(v);
        bits += 6;
        if (bits >= 8) {
            bits -= 8;
            out.push_back(static_cast<uint8_t>((buf >> bits) & 0xFF));
        }
    }
    return out;
}

} // namespace openhwp::formats
