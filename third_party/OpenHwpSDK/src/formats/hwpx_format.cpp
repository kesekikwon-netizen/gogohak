#include "hwpx_format.hpp"

#include <algorithm>
#include <cstring>
#include <fstream>
#include <sstream>
#include <zlib.h>

namespace openhwp::formats {

namespace {

uint16_t ReadLe16(const uint8_t* p) {
    return static_cast<uint16_t>(p[0]) | (static_cast<uint16_t>(p[1]) << 8);
}

uint32_t ReadLe32(const uint8_t* p) {
    return static_cast<uint32_t>(p[0]) |
           (static_cast<uint32_t>(p[1]) << 8) |
           (static_cast<uint32_t>(p[2]) << 16) |
           (static_cast<uint32_t>(p[3]) << 24);
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

bool RawDeflate(const std::string& input, std::string& output) {
    z_stream strm{};
    if (deflateInit2(&strm, Z_DEFAULT_COMPRESSION, Z_DEFLATED, -MAX_WBITS, 8, Z_DEFAULT_STRATEGY) != Z_OK) {
        return false;
    }
    output.resize(deflateBound(&strm, static_cast<uLong>(input.size())) + 16);
    strm.next_in = reinterpret_cast<Bytef*>(const_cast<char*>(input.data()));
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

bool RawInflate(const uint8_t* data, size_t comp_size, size_t uncomp_size, std::string& output) {
    if (uncomp_size == 0) {
        output.clear();
        return true;
    }
    z_stream strm{};
    if (inflateInit2(&strm, -MAX_WBITS) != Z_OK) {
        return false;
    }
    output.resize(uncomp_size);
    strm.next_in = const_cast<Bytef*>(reinterpret_cast<const Bytef*>(data));
    strm.avail_in = static_cast<uInt>(comp_size);
    strm.next_out = reinterpret_cast<Bytef*>(output.data());
    strm.avail_out = static_cast<uInt>(output.size());

    int ret = inflate(&strm, Z_FINISH);
    inflateEnd(&strm);
    if (ret != Z_STREAM_END && ret != Z_OK) {
        return false;
    }
    output.resize(strm.total_out);
    return true;
}

std::string EscapeXml(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    for (char c : s) {
        switch (c) {
            case '&': out += "&amp;"; break;
            case '<': out += "&lt;"; break;
            case '>': out += "&gt;"; break;
            case '"': out += "&quot;"; break;
            case '\'': out += "&apos;"; break;
            default: out.push_back(c); break;
        }
    }
    return out;
}

std::string UnescapeXml(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    for (size_t i = 0; i < s.size(); ++i) {
        if (s[i] == '&') {
            if (s.compare(i, 5, "&amp;") == 0) {
                out.push_back('&');
                i += 4;
            } else if (s.compare(i, 4, "&lt;") == 0) {
                out.push_back('<');
                i += 3;
            } else if (s.compare(i, 4, "&gt;") == 0) {
                out.push_back('>');
                i += 3;
            } else if (s.compare(i, 6, "&quot;") == 0) {
                out.push_back('"');
                i += 5;
            } else if (s.compare(i, 6, "&apos;") == 0) {
                out.push_back('\'');
                i += 5;
            } else {
                out.push_back(s[i]);
            }
        } else {
            out.push_back(s[i]);
        }
    }
    return out;
}

std::string ExtractAttr(const std::string& tag, const std::string& attr_name) {
    std::string key = attr_name + "=\"";
    size_t pos = tag.find(key);
    if (pos == std::string::npos) {
        return "";
    }
    pos += key.size();
    size_t end = tag.find('"', pos);
    if (end == std::string::npos) {
        return "";
    }
    return UnescapeXml(tag.substr(pos, end - pos));
}

int ExtractIntAttr(const std::string& tag, const std::string& attr_name, int default_val = 0) {
    std::string val = ExtractAttr(tag, attr_name);
    if (val.empty()) {
        return default_val;
    }
    try {
        return std::stoi(val);
    } catch (...) {
        return default_val;
    }
}

uint32_t ExtractUIntAttr(const std::string& tag, const std::string& attr_name, uint32_t default_val = 0) {
    std::string val = ExtractAttr(tag, attr_name);
    if (val.empty()) {
        return default_val;
    }
    try {
        return static_cast<uint32_t>(std::stoul(val, nullptr, 0));
    } catch (...) {
        return default_val;
    }
}

std::string CollectAllHpText(const std::string& xml_chunk) {
    std::string result;
    size_t pos = 0;
    while (true) {
        size_t t_open = xml_chunk.find("<hp:t", pos);
        if (t_open == std::string::npos) {
            break;
        }
        size_t tag_close = xml_chunk.find('>', t_open);
        if (tag_close == std::string::npos) {
            break;
        }
        if (tag_close > t_open && xml_chunk[tag_close - 1] == '/') {
            pos = tag_close + 1;
            continue;
        }
        size_t t_end = xml_chunk.find("</hp:t>", tag_close + 1);
        if (t_end == std::string::npos) {
            break;
        }
        result += UnescapeXml(xml_chunk.substr(tag_close + 1, t_end - (tag_close + 1)));
        pos = t_end + 7;
    }
    return result;
}

} // namespace

bool HwpxFormatHandler::IsHwpxFile(const std::filesystem::path& path) {
    std::error_code ec;
    if (!std::filesystem::exists(path, ec)) {
        return false;
    }
    std::ifstream ifs(path, std::ios::binary);
    if (!ifs) {
        return false;
    }
    uint8_t magic[4] = {0};
    ifs.read(reinterpret_cast<char*>(magic), 4);
    if (ifs.gcount() < 4) {
        return false;
    }
    return magic[0] == 0x50 && magic[1] == 0x4B && magic[2] == 0x03 && magic[3] == 0x04;
}

bool HwpxFormatHandler::ReadZipEntries(
    const std::vector<uint8_t>& zip_bytes,
    std::map<std::string, std::string>& out_entries) {
    out_entries.clear();
    if (zip_bytes.size() < 22) {
        return false;
    }

    // Locate End of Central Directory (EOCD: 0x06054b50)
    size_t eocd_pos = std::string::npos;
    size_t min_search = zip_bytes.size() > 65557 ? zip_bytes.size() - 65557 : 0;
    for (size_t i = zip_bytes.size() - 22; ; --i) {
        if (ReadLe32(&zip_bytes[i]) == 0x06054B50) {
            eocd_pos = i;
            break;
        }
        if (i == min_search) {
            break;
        }
    }
    if (eocd_pos == std::string::npos) {
        return false;
    }

    uint16_t total_entries = ReadLe16(&zip_bytes[eocd_pos + 10]);
    uint32_t cd_offset = ReadLe32(&zip_bytes[eocd_pos + 16]);
    size_t pos = cd_offset;

    for (uint16_t idx = 0; idx < total_entries && pos + 46 <= zip_bytes.size(); ++idx) {
        if (ReadLe32(&zip_bytes[pos]) != 0x02014B50) {
            break;
        }
        uint16_t method = ReadLe16(&zip_bytes[pos + 10]);
        uint32_t comp_size = ReadLe32(&zip_bytes[pos + 20]);
        uint32_t uncomp_size = ReadLe32(&zip_bytes[pos + 24]);
        uint16_t name_len = ReadLe16(&zip_bytes[pos + 28]);
        uint16_t extra_len = ReadLe16(&zip_bytes[pos + 30]);
        uint16_t comment_len = ReadLe16(&zip_bytes[pos + 32]);
        uint32_t local_header_off = ReadLe32(&zip_bytes[pos + 42]);

        if (pos + 46 + name_len > zip_bytes.size()) {
            break;
        }
        std::string entry_name(reinterpret_cast<const char*>(&zip_bytes[pos + 46]), name_len);
        pos += 46 + name_len + extra_len + comment_len;

        if (local_header_off + 30 > zip_bytes.size()) {
            continue;
        }
        if (ReadLe32(&zip_bytes[local_header_off]) != 0x04034B50) {
            continue;
        }
        uint16_t l_name_len = ReadLe16(&zip_bytes[local_header_off + 26]);
        uint16_t l_extra_len = ReadLe16(&zip_bytes[local_header_off + 28]);
        size_t data_off = local_header_off + 30 + l_name_len + l_extra_len;
        if (data_off + comp_size > zip_bytes.size()) {
            continue;
        }

        if (method == 0) {
            out_entries[entry_name] = std::string(
                reinterpret_cast<const char*>(&zip_bytes[data_off]),
                comp_size);
        } else if (method == 8) {
            std::string inflated;
            if (RawInflate(&zip_bytes[data_off], comp_size, uncomp_size, inflated)) {
                out_entries[entry_name] = std::move(inflated);
            }
        }
    }

    return !out_entries.empty();
}

std::vector<uint8_t> HwpxFormatHandler::WriteZipArchive(
    const std::vector<std::pair<std::string, std::string>>& entries) {
    struct CdRecord {
        std::string name;
        uint16_t method = 0;
        uint32_t crc = 0;
        uint32_t comp_size = 0;
        uint32_t uncomp_size = 0;
        uint32_t local_offset = 0;
    };

    std::vector<uint8_t> out;
    std::vector<CdRecord> cd_records;

    for (const auto& [name, content] : entries) {
        CdRecord rec;
        rec.name = name;
        rec.uncomp_size = static_cast<uint32_t>(content.size());
        rec.crc = crc32(0L, reinterpret_cast<const Bytef*>(content.data()), static_cast<uInt>(content.size()));
        rec.local_offset = static_cast<uint32_t>(out.size());

        std::string compressed;
        bool store_uncompressed = (name == "mimetype" || content.size() < 32);
        if (!store_uncompressed && RawDeflate(content, compressed) && compressed.size() < content.size()) {
            rec.method = 8;
            rec.comp_size = static_cast<uint32_t>(compressed.size());
        } else {
            rec.method = 0;
            rec.comp_size = rec.uncomp_size;
        }

        // Local file header (0x04034B50)
        AppendLe32(out, 0x04034B50);
        AppendLe16(out, 20); // version needed
        AppendLe16(out, 0);  // flags
        AppendLe16(out, rec.method);
        AppendLe16(out, 0);  // mod time
        AppendLe16(out, 0);  // mod date
        AppendLe32(out, rec.crc);
        AppendLe32(out, rec.comp_size);
        AppendLe32(out, rec.uncomp_size);
        AppendLe16(out, static_cast<uint16_t>(rec.name.size()));
        AppendLe16(out, 0);  // extra len
        out.insert(out.end(), rec.name.begin(), rec.name.end());

        if (rec.method == 8) {
            out.insert(out.end(), compressed.begin(), compressed.end());
        } else {
            out.insert(out.end(), content.begin(), content.end());
        }

        cd_records.push_back(rec);
    }

    uint32_t cd_start = static_cast<uint32_t>(out.size());
    for (const auto& rec : cd_records) {
        AppendLe32(out, 0x02014B50);
        AppendLe16(out, 20); // version made by
        AppendLe16(out, 20); // version needed
        AppendLe16(out, 0);  // flags
        AppendLe16(out, rec.method);
        AppendLe16(out, 0);  // time
        AppendLe16(out, 0);  // date
        AppendLe32(out, rec.crc);
        AppendLe32(out, rec.comp_size);
        AppendLe32(out, rec.uncomp_size);
        AppendLe16(out, static_cast<uint16_t>(rec.name.size()));
        AppendLe16(out, 0);  // extra
        AppendLe16(out, 0);  // comment
        AppendLe16(out, 0);  // disk start
        AppendLe16(out, 0);  // internal attr
        AppendLe32(out, 0);  // external attr
        AppendLe32(out, rec.local_offset);
        out.insert(out.end(), rec.name.begin(), rec.name.end());
    }
    uint32_t cd_size = static_cast<uint32_t>(out.size()) - cd_start;

    // EOCD (0x06054B50)
    AppendLe32(out, 0x06054B50);
    AppendLe16(out, 0);
    AppendLe16(out, 0);
    AppendLe16(out, static_cast<uint16_t>(cd_records.size()));
    AppendLe16(out, static_cast<uint16_t>(cd_records.size()));
    AppendLe32(out, cd_size);
    AppendLe32(out, cd_start);
    AppendLe16(out, 0);

    return out;
}

std::string HwpxFormatHandler::SerializeSectionXmlUtf8(const core::DocumentModel& model) {
    std::ostringstream oss;
    oss << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n";
    oss << "<hs:sec xmlns:hs=\"http://www.hancom.co.kr/hwpml/2011/section\" "
           "xmlns:hp=\"http://www.hancom.co.kr/hwpml/2011/paragraph\" "
           "xmlns:hc=\"http://www.hancom.co.kr/hwpml/2011/core\">\n";

    if (model.page_num_pos.enabled) {
        oss << "  <hp:pageNum pos=\"" << model.page_num_pos.draw_pos
            << "\" sideChar=\"" << model.page_num_pos.side_char
            << "\" startPage=\"" << model.page_num_pos.new_number << "\"/>\n";
    }

    // Serialize User Properties
    for (const auto& [k, v] : model.user_properties) {
        oss << "  <hp:userProp name=\"" << EscapeXml(k) << "\" value=\"" << EscapeXml(v) << "\"/>\n";
    }

    // Serialize Paragraphs, inline Fields, Tables, Pictures, Memos
    for (size_t p_idx = 0; p_idx < model.paragraphs.size(); ++p_idx) {
        const auto& para = model.paragraphs[p_idx];
        oss << "  <hp:p id=\"" << p_idx << "\" pageBreak=\"" << (para.page_break_before ? 1 : 0)
            << "\" align=\"" << para.para_shape.align_type
            << "\" breakNonLatinWord=\"" << para.para_shape.break_non_latin_word
            << "\" lineSpacing=\"" << para.para_shape.line_spacing
            << "\" charHeight=\"" << para.char_shape.height
            << "\" bold=\"" << para.char_shape.bold
            << "\" italic=\"" << para.char_shape.italic
            << "\" underline=\"" << para.char_shape.underline
            << "\" textColor=\"" << para.char_shape.text_color
            << "\" fontName=\"" << EscapeXml(para.char_shape.font_name_utf8) << "\"";
        if (!para.heading_utf8.empty()) {
            oss << " heading=\"" << EscapeXml(para.heading_utf8) << "\"";
        }
        if (!para.meta_tag_utf8.empty()) {
            oss << " metaTag=\"" << EscapeXml(para.meta_tag_utf8) << "\"";
        }
        oss << ">\n";

        oss << "    <hp:run>\n";

        // Fields anchored at this paragraph
        for (const auto& field : model.fields) {
            if (field.para_index == static_cast<int>(p_idx)) {
                oss << "      <hp:fieldBegin type=\"CLICK_HERE\" name=\"" << EscapeXml(field.name_utf8)
                    << "\" direction=\"" << EscapeXml(field.direction_utf8)
                    << "\" memo=\"" << EscapeXml(field.memo_utf8)
                    << "\" metaTag=\"" << EscapeXml(field.meta_tag_utf8)
                    << "\" properties=\"" << field.properties << "\"/>"
                    << "<hp:t>" << EscapeXml(field.text_utf8) << "</hp:t><hp:fieldEnd/>\n";
            }
        }

        // Paragraph text
        oss << "      <hp:t>" << EscapeXml(para.text_utf8) << "</hp:t>\n";

        // Tables anchored at this paragraph
        for (const auto& tbl : model.tables) {
            if (tbl.anchor_para == static_cast<int>(p_idx)) {
                oss << "      <hp:tbl id=\"" << EscapeXml(tbl.inst_id_utf8)
                    << "\" rowCnt=\"" << tbl.rows << "\" colCnt=\"" << tbl.cols
                    << "\" treatAsChar=\"" << (tbl.treat_as_char ? 1 : 0)
                    << "\" metaTag=\"" << EscapeXml(tbl.meta_tag_utf8) << "\">\n";
                for (int r = 0; r < tbl.rows && r < static_cast<int>(tbl.cells.size()); ++r) {
                    oss << "        <hp:tr>\n";
                    for (int c = 0; c < tbl.cols && c < static_cast<int>(tbl.cells[r].size()); ++c) {
                        const auto& cell = tbl.cells[r][c];
                        if (cell.row_span <= 0 || cell.col_span <= 0) {
                            continue; // Covered by a merged cell
                        }
                        oss << "          <hp:tc name=\"" << EscapeXml(cell.field_name_utf8)
                            << "\" metaTag=\"" << EscapeXml(cell.meta_tag_utf8)
                            << "\" bgColor=\"" << cell.bg_color
                            << "\" faceStyle=\"" << cell.face_style
                            << "\" hatchColor=\"" << cell.hatch_color
                            << "\" borderTop=\"" << cell.border_type_top << ":" << cell.border_width_top
                            << "\" borderBottom=\"" << cell.border_type_bottom << ":" << cell.border_width_bottom
                            << "\" borderLeft=\"" << cell.border_type_left << ":" << cell.border_width_left
                            << "\" borderRight=\"" << cell.border_type_right << ":" << cell.border_width_right
                            << "\">\n";
                        oss << "            <hp:cellAddr colAddr=\"" << c << "\" rowAddr=\"" << r << "\"/>\n";
                        oss << "            <hp:cellSpan colSpan=\"" << cell.col_span << "\" rowSpan=\"" << cell.row_span << "\"/>\n";
                        oss << "            <hp:cellSz width=\"" << cell.width_hwpunit << "\" height=\"" << cell.height_hwpunit << "\"/>\n";
                        oss << "            <hp:subList><hp:p><hp:run><hp:t>"
                            << EscapeXml(cell.text_utf8)
                            << "</hp:t></hp:run></hp:p></hp:subList>\n";
                        oss << "          </hp:tc>\n";
                    }
                    oss << "        </hp:tr>\n";
                }
                oss << "      </hp:tbl>\n";
            }
        }

        // Pictures anchored at this paragraph
        for (const auto& pic : model.pictures) {
            if (pic.anchor_para == static_cast<int>(p_idx)) {
                oss << "      <hp:pic id=\"" << EscapeXml(pic.inst_id_utf8)
                    << "\" path=\"" << EscapeXml(pic.file_path.string())
                    << "\" binDataId=\"" << pic.bin_data_id
                    << "\" widthMm=\"" << pic.width_mm << "\" heightMm=\"" << pic.height_mm
                    << "\" metaTag=\"" << EscapeXml(pic.meta_tag_utf8) << "\"/>\n";
            }
        }

        // Memos anchored at this paragraph
        for (const auto& memo : model.memos) {
            if (memo.anchor_para == static_cast<int>(p_idx)) {
                oss << "      <hp:memo id=\"" << memo.inst_id
                    << "\" author=\"" << EscapeXml(memo.author_utf8)
                    << "\" dateTime=\"" << EscapeXml(memo.datetime_utf8)
                    << "\"><hp:t>" << EscapeXml(memo.content_utf8) << "</hp:t></hp:memo>\n";
            }
        }

        oss << "    </hp:run>\n";
        oss << "  </hp:p>\n";
    }

    oss << "</hs:sec>\n";
    return oss.str();
}

bool HwpxFormatHandler::ParseSectionXmlUtf8(
    const std::string& xml_utf8,
    core::DocumentModel& model,
    bool append_mode) {
    if (!append_mode) {
        model.paragraphs.clear();
        model.fields.clear();
        model.tables.clear();
        model.pictures.clear();
        model.memos.clear();
    }

    // Parse <hp:pageNum .../>
    size_t pn_pos = xml_utf8.find("<hp:pageNum");
    if (pn_pos != std::string::npos) {
        size_t pn_end = xml_utf8.find('>', pn_pos);
        if (pn_end != std::string::npos) {
            std::string tag = xml_utf8.substr(pn_pos, pn_end - pn_pos + 1);
            model.page_num_pos.enabled = true;
            model.page_num_pos.draw_pos = ExtractIntAttr(tag, "pos", 0);
            model.page_num_pos.side_char = ExtractIntAttr(tag, "sideChar", 0);
            model.page_num_pos.new_number = ExtractIntAttr(tag, "startPage", 1);
        }
    }

    // Parse <hp:userProp .../>
    size_t up_pos = 0;
    while ((up_pos = xml_utf8.find("<hp:userProp", up_pos)) != std::string::npos) {
        size_t up_end = xml_utf8.find('>', up_pos);
        if (up_end == std::string::npos) {
            break;
        }
        std::string tag = xml_utf8.substr(up_pos, up_end - up_pos + 1);
        std::string name = ExtractAttr(tag, "name");
        std::string val = ExtractAttr(tag, "value");
        if (!name.empty()) {
            model.user_properties[name] = val;
        }
        up_pos = up_end + 1;
    }

    // Parse top-level <hp:p> blocks (avoiding nested <hp:p> inside <hp:subList>)
    size_t pos = 0;
    while (pos < xml_utf8.size()) {
        size_t p_start = xml_utf8.find("<hp:p", pos);
        if (p_start == std::string::npos) {
            break;
        }
        char next_ch = (p_start + 5 < xml_utf8.size()) ? xml_utf8[p_start + 5] : '\0';
        if (next_ch != ' ' && next_ch != '>' && next_ch != '/') {
            pos = p_start + 5;
            continue;
        }

        size_t p_open_end = xml_utf8.find('>', p_start);
        if (p_open_end == std::string::npos) {
            break;
        }
        std::string p_open_tag = xml_utf8.substr(p_start, p_open_end - p_start + 1);
        if (p_open_tag.size() >= 2 && p_open_tag[p_open_tag.size() - 2] == '/') {
            core::ParagraphNode empty_p;
            model.paragraphs.push_back(empty_p);
            pos = p_open_end + 1;
            continue;
        }

        // Find matching </hp:p> with nesting depth counter
        int depth = 1;
        size_t scan = p_open_end + 1;
        size_t p_close_start = std::string::npos;
        while (scan < xml_utf8.size() && depth > 0) {
            size_t next_open = xml_utf8.find("<hp:p", scan);
            size_t next_close = xml_utf8.find("</hp:p>", scan);
            if (next_close == std::string::npos) {
                break;
            }
            if (next_open != std::string::npos && next_open < next_close) {
                char ch = (next_open + 5 < xml_utf8.size()) ? xml_utf8[next_open + 5] : '\0';
                size_t open_gt = xml_utf8.find('>', next_open);
                if ((ch == ' ' || ch == '>') && open_gt != std::string::npos && xml_utf8[open_gt - 1] != '/') {
                    ++depth;
                }
                scan = (open_gt != std::string::npos) ? (open_gt + 1) : (next_open + 5);
            } else {
                --depth;
                if (depth == 0) {
                    p_close_start = next_close;
                    break;
                }
                scan = next_close + 7;
            }
        }

        if (p_close_start == std::string::npos) {
            break;
        }

        std::string p_body = xml_utf8.substr(p_open_end + 1, p_close_start - (p_open_end + 1));
        int current_para_index = static_cast<int>(model.paragraphs.size());

        core::ParagraphNode pnode;
        pnode.page_break_before = (ExtractIntAttr(p_open_tag, "pageBreak", 0) != 0);
        pnode.para_shape.align_type = static_cast<unsigned int>(ExtractIntAttr(p_open_tag, "align", 0));
        pnode.para_shape.break_non_latin_word = static_cast<unsigned int>(ExtractIntAttr(p_open_tag, "breakNonLatinWord", 0));
        pnode.para_shape.line_spacing = ExtractIntAttr(p_open_tag, "lineSpacing", 160);
        pnode.char_shape.height = ExtractUIntAttr(p_open_tag, "charHeight", 1000);
        pnode.char_shape.bold = ExtractUIntAttr(p_open_tag, "bold", 0);
        pnode.char_shape.italic = ExtractUIntAttr(p_open_tag, "italic", 0);
        pnode.char_shape.underline = ExtractUIntAttr(p_open_tag, "underline", 0);
        pnode.char_shape.text_color = ExtractUIntAttr(p_open_tag, "textColor", 0);
        std::string font_attr = ExtractAttr(p_open_tag, "fontName");
        if (!font_attr.empty()) {
            pnode.char_shape.font_name_utf8 = font_attr;
        }
        pnode.heading_utf8 = ExtractAttr(p_open_tag, "heading");
        pnode.meta_tag_utf8 = ExtractAttr(p_open_tag, "metaTag");

        // Strip out <hp:tbl>...</hp:tbl>, <hp:fieldBegin.../>...<hp:fieldEnd/>, <hp:memo>...</hp:memo>
        // before collecting direct paragraph text
        std::string body_without_children = p_body;

        // 1. Parse Tables inside this paragraph
        size_t tbl_pos = 0;
        while ((tbl_pos = p_body.find("<hp:tbl", tbl_pos)) != std::string::npos) {
            size_t tbl_open_end = p_body.find('>', tbl_pos);
            size_t tbl_close = p_body.find("</hp:tbl>", tbl_pos);
            if (tbl_open_end == std::string::npos || tbl_close == std::string::npos) {
                break;
            }
            std::string tbl_tag = p_body.substr(tbl_pos, tbl_open_end - tbl_pos + 1);
            std::string tbl_inner = p_body.substr(tbl_open_end + 1, tbl_close - (tbl_open_end + 1));

            core::TableNode tbl;
            tbl.inst_id_utf8 = ExtractAttr(tbl_tag, "id");
            if (tbl.inst_id_utf8.empty()) {
                tbl.inst_id_utf8 = std::to_string(1000 + model.tables.size());
            }
            int rows = ExtractIntAttr(tbl_tag, "rowCnt", 1);
            int cols = ExtractIntAttr(tbl_tag, "colCnt", 1);
            tbl.treat_as_char = (ExtractIntAttr(tbl_tag, "treatAsChar", 0) != 0);
            tbl.meta_tag_utf8 = ExtractAttr(tbl_tag, "metaTag");
            tbl.anchor_para = current_para_index;
            tbl.InitializeGrid(rows, cols);

            // Parse <hp:tc> cells
            size_t tc_pos = 0;
            int fallback_row = 0;
            int fallback_col = 0;
            while ((tc_pos = tbl_inner.find("<hp:tc", tc_pos)) != std::string::npos) {
                size_t tc_open_end = tbl_inner.find('>', tc_pos);
                size_t tc_close = tbl_inner.find("</hp:tc>", tc_pos);
                if (tc_open_end == std::string::npos || tc_close == std::string::npos) {
                    break;
                }
                std::string tc_tag = tbl_inner.substr(tc_pos, tc_open_end - tc_pos + 1);
                std::string tc_inner = tbl_inner.substr(tc_open_end + 1, tc_close - (tc_open_end + 1));

                int r = fallback_row;
                int c = fallback_col;
                size_t addr_pos = tc_inner.find("<hp:cellAddr");
                if (addr_pos != std::string::npos) {
                    size_t addr_end = tc_inner.find('>', addr_pos);
                    if (addr_end != std::string::npos) {
                        std::string addr_tag = tc_inner.substr(addr_pos, addr_end - addr_pos + 1);
                        r = ExtractIntAttr(addr_tag, "rowAddr", r);
                        c = ExtractIntAttr(addr_tag, "colAddr", c);
                    }
                }

                int r_span = 1;
                int c_span = 1;
                size_t span_pos = tc_inner.find("<hp:cellSpan");
                if (span_pos != std::string::npos) {
                    size_t span_end = tc_inner.find('>', span_pos);
                    if (span_end != std::string::npos) {
                        std::string span_tag = tc_inner.substr(span_pos, span_end - span_pos + 1);
                        r_span = ExtractIntAttr(span_tag, "rowSpan", 1);
                        c_span = ExtractIntAttr(span_tag, "colSpan", 1);
                    }
                }

                int w_hu = 10000;
                int h_hu = 2000;
                size_t sz_pos = tc_inner.find("<hp:cellSz");
                if (sz_pos != std::string::npos) {
                    size_t sz_end = tc_inner.find('>', sz_pos);
                    if (sz_end != std::string::npos) {
                        std::string sz_tag = tc_inner.substr(sz_pos, sz_end - sz_pos + 1);
                        w_hu = ExtractIntAttr(sz_tag, "width", 10000);
                        h_hu = ExtractIntAttr(sz_tag, "height", 2000);
                    }
                }

                if (r >= 0 && r < tbl.rows && c >= 0 && c < tbl.cols) {
                    auto& cell = tbl.cells[r][c];
                    cell.row = r;
                    cell.col = c;
                    cell.row_span = r_span;
                    cell.col_span = c_span;
                    cell.width_hwpunit = w_hu;
                    cell.height_hwpunit = h_hu;
                    cell.field_name_utf8 = ExtractAttr(tc_tag, "name");
                    cell.meta_tag_utf8 = ExtractAttr(tc_tag, "metaTag");
                    cell.bg_color = ExtractUIntAttr(tc_tag, "bgColor", 0xFFFFFF);
                    cell.face_style = ExtractIntAttr(tc_tag, "faceStyle", -1);
                    cell.hatch_color = ExtractUIntAttr(tc_tag, "hatchColor", 0);
                    cell.text_utf8 = CollectAllHpText(tc_inner);

                    for (int dr = 0; dr < r_span; ++dr) {
                        for (int dc = 0; dc < c_span; ++dc) {
                            if (dr == 0 && dc == 0) {
                                continue;
                            }
                            if (r + dr < tbl.rows && c + dc < tbl.cols) {
                                tbl.cells[r + dr][c + dc].row_span = 0;
                                tbl.cells[r + dr][c + dc].col_span = 0;
                            }
                        }
                    }
                }

                ++fallback_col;
                if (fallback_col >= tbl.cols) {
                    fallback_col = 0;
                    ++fallback_row;
                }
                tc_pos = tc_close + 8;
            }

            model.tables.push_back(std::move(tbl));
            tbl_pos = tbl_close + 9;
        }

        // Remove <hp:tbl>...</hp:tbl> from body_without_children
        while (true) {
            size_t t1 = body_without_children.find("<hp:tbl");
            if (t1 == std::string::npos) {
                break;
            }
            size_t t2 = body_without_children.find("</hp:tbl>", t1);
            if (t2 == std::string::npos) {
                break;
            }
            body_without_children.erase(t1, (t2 + 9) - t1);
        }

        // 2. Parse Fields (<hp:fieldBegin .../> ... <hp:fieldEnd/>)
        while (true) {
            size_t fb = body_without_children.find("<hp:fieldBegin");
            if (fb == std::string::npos) {
                break;
            }
            size_t fb_end = body_without_children.find('>', fb);
            size_t fe = body_without_children.find("<hp:fieldEnd", fb);
            if (fb_end == std::string::npos || fe == std::string::npos) {
                break;
            }
            size_t fe_end = body_without_children.find('>', fe);
            if (fe_end == std::string::npos) {
                break;
            }

            std::string fb_tag = body_without_children.substr(fb, fb_end - fb + 1);
            std::string f_inner = body_without_children.substr(fb_end + 1, fe - (fb_end + 1));

            core::FieldNode fnode;
            fnode.name_utf8 = ExtractAttr(fb_tag, "name");
            fnode.direction_utf8 = ExtractAttr(fb_tag, "direction");
            fnode.memo_utf8 = ExtractAttr(fb_tag, "memo");
            fnode.meta_tag_utf8 = ExtractAttr(fb_tag, "metaTag");
            fnode.properties = ExtractIntAttr(fb_tag, "properties", HWPSDK::FIELD_CLICKHERE);
            fnode.field_type = HWPSDK::FieldClickHere;
            fnode.para_index = current_para_index;
            fnode.text_utf8 = CollectAllHpText(f_inner);
            model.fields.push_back(std::move(fnode));

            body_without_children.erase(fb, (fe_end + 1) - fb);
        }

        // 3. Parse Pictures (<hp:pic .../>)
        size_t pic_pos = 0;
        while ((pic_pos = body_without_children.find("<hp:pic", pic_pos)) != std::string::npos) {
            size_t pic_end = body_without_children.find('>', pic_pos);
            if (pic_end == std::string::npos) {
                break;
            }
            std::string pic_tag = body_without_children.substr(pic_pos, pic_end - pic_pos + 1);
            core::PictureNode pic;
            pic.inst_id_utf8 = ExtractAttr(pic_tag, "id");
            pic.file_path = std::filesystem::path(ExtractAttr(pic_tag, "path"));
            pic.bin_data_id = ExtractIntAttr(pic_tag, "binDataId", 1);
            pic.width_mm = ExtractIntAttr(pic_tag, "widthMm", 50);
            pic.height_mm = ExtractIntAttr(pic_tag, "heightMm", 50);
            pic.meta_tag_utf8 = ExtractAttr(pic_tag, "metaTag");
            pic.anchor_para = current_para_index;
            model.pictures.push_back(std::move(pic));
            pic_pos = pic_end + 1;
        }

        // 4. Parse Memos (<hp:memo ...>...</hp:memo>)
        while (true) {
            size_t m1 = body_without_children.find("<hp:memo");
            if (m1 == std::string::npos) {
                break;
            }
            size_t m1_end = body_without_children.find('>', m1);
            size_t m2 = body_without_children.find("</hp:memo>", m1);
            if (m1_end == std::string::npos || m2 == std::string::npos) {
                break;
            }
            std::string m_tag = body_without_children.substr(m1, m1_end - m1 + 1);
            std::string m_inner = body_without_children.substr(m1_end + 1, m2 - (m1_end + 1));

            core::MemoNode memo;
            memo.inst_id = ExtractIntAttr(m_tag, "id", static_cast<int>(model.memos.size() + 1));
            memo.author_utf8 = ExtractAttr(m_tag, "author");
            memo.datetime_utf8 = ExtractAttr(m_tag, "dateTime");
            memo.content_utf8 = CollectAllHpText(m_inner);
            memo.anchor_para = current_para_index;
            model.memos.push_back(std::move(memo));

            body_without_children.erase(m1, (m2 + 10) - m1);
        }

        // 5. Collect remaining paragraph text
        pnode.text_utf8 = CollectAllHpText(body_without_children);
        model.paragraphs.push_back(std::move(pnode));

        pos = p_close_start + 7;
    }

    if (model.paragraphs.empty()) {
        model.paragraphs.emplace_back();
    }
    return true;
}

bool HwpxFormatHandler::LoadFromFile(const std::filesystem::path& path, core::DocumentModel& model) {
    std::ifstream ifs(path, std::ios::binary | std::ios::ate);
    if (!ifs) {
        return false;
    }
    std::streamsize size = ifs.tellg();
    if (size <= 0) {
        return false;
    }
    ifs.seekg(0, std::ios::beg);
    std::vector<uint8_t> buf(static_cast<size_t>(size));
    if (!ifs.read(reinterpret_cast<char*>(buf.data()), size)) {
        return false;
    }

    std::map<std::string, std::string> entries;
    if (!ReadZipEntries(buf, entries)) {
        return false;
    }

    model.ClearAndReset();
    model.paragraphs.clear();

    // Load all Contents/section*.xml entries in order
    bool found_section = false;
    for (const auto& [entry_name, xml_content] : entries) {
        if (entry_name.find("Contents/section") == 0 && entry_name.rfind(".xml") != std::string::npos) {
            ParseSectionXmlUtf8(xml_content, model, found_section);
            found_section = true;
        }
    }

    if (!found_section) {
        return false;
    }
    if (model.paragraphs.empty()) {
        model.paragraphs.emplace_back();
    }
    model.current_path = path;
    model.current_format_utf8 = "HWPX";
    model.modified = false;
    return true;
}

bool HwpxFormatHandler::SaveToFile(const std::filesystem::path& path, const core::DocumentModel& model) {
    std::error_code ec;
    if (path.has_parent_path()) {
        std::filesystem::create_directories(path.parent_path(), ec);
    }

    const std::string mimetype_content = "application/hwp+zip";
    const std::string container_xml =
        "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
        "<ocf:container xmlns:ocf=\"urn:oasis:names:tc:opendocument:xmlns:container\">\n"
        "  <ocf:rootfiles>\n"
        "    <ocf:rootfile full-path=\"Contents/content.hpf\" media-type=\"application/hwpml-package+xml\"/>\n"
        "  </ocf:rootfiles>\n"
        "</ocf:container>\n";

    const std::string content_hpf =
        "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
        "<opf:package xmlns:opf=\"http://www.idpf.org/2007/opf/\" version=\"1.2\">\n"
        "  <opf:manifest>\n"
        "    <opf:item id=\"header\" href=\"Contents/header.xml\" media-type=\"application/xml\"/>\n"
        "    <opf:item id=\"section0\" href=\"Contents/section0.xml\" media-type=\"application/xml\"/>\n"
        "  </opf:manifest>\n"
        "  <opf:spine>\n"
        "    <opf:itemref idref=\"header\" linear=\"no\"/>\n"
        "    <opf:itemref idref=\"section0\" linear=\"yes\"/>\n"
        "  </opf:spine>\n"
        "</opf:package>\n";

    const std::string header_xml =
        "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
        "<hh:head xmlns:hh=\"http://www.hancom.co.kr/hwpml/2011/head\" version=\"1.4\" secCnt=\"1\">\n"
        "  <hh:beginNum page=\"1\" footnote=\"1\" endnote=\"1\" pic=\"1\" tbl=\"1\" equation=\"1\"/>\n"
        "</hh:head>\n";

    const std::string section0_xml = SerializeSectionXmlUtf8(model);

    std::vector<std::pair<std::string, std::string>> entries = {
        {"mimetype", mimetype_content},
        {"META-INF/container.xml", container_xml},
        {"Contents/content.hpf", content_hpf},
        {"Contents/header.xml", header_xml},
        {"Contents/section0.xml", section0_xml},
    };

    std::vector<uint8_t> zip_bytes = WriteZipArchive(entries);
    std::ofstream ofs(path, std::ios::binary | std::ios::trunc);
    if (!ofs) {
        return false;
    }
    ofs.write(reinterpret_cast<const char*>(zip_bytes.data()), static_cast<std::streamsize>(zip_bytes.size()));
    return ofs.good();
}

} // namespace openhwp::formats
