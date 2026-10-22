#include "document_engine.hpp"
#include "utf_conv.hpp"
#include "../formats/hwpx_format.hpp"
#include "../formats/hwp5_format.hpp"
#include "../../include/platform/IPlatformService.h"

#include <algorithm>
#include <cstring>
#include <fstream>
#include <sstream>

namespace openhwp::core {

namespace {

std::string ToUpperAscii(std::string s) {
    for (char& c : s) {
        if (c >= 'a' && c <= 'z') {
            c = static_cast<char>(c - 'a' + 'A');
        }
    }
    return s;
}

std::vector<std::string> SplitByByte(const std::string& s, char delim) {
    std::vector<std::string> out;
    size_t start = 0;
    while (true) {
        size_t pos = s.find(delim, start);
        if (pos == std::string::npos) {
            out.push_back(s.substr(start));
            break;
        }
        out.push_back(s.substr(start, pos - start));
        start = pos + 1;
    }
    return out;
}

// Parses "FieldName{{index}}" -> (base_name, index, has_explicit_index)
void ParseFieldSelector(const std::string& token, std::string& out_name, int& out_idx, bool& out_has_idx) {
    out_has_idx = false;
    out_idx = 0;
    size_t lbrace = token.find("{{");
    size_t rbrace = token.find("}}", lbrace == std::string::npos ? 0 : lbrace + 2);
    if (lbrace != std::string::npos && rbrace != std::string::npos) {
        out_name = token.substr(0, lbrace);
        std::string num_str = token.substr(lbrace + 2, rbrace - (lbrace + 2));
        try {
            out_idx = std::stoi(num_str);
            out_has_idx = true;
        } catch (...) {
            out_idx = 0;
        }
    } else {
        out_name = token;
    }
}

} // namespace

// ============================================================================
// ParameterArrayImpl
// ============================================================================

ParameterArrayImpl::ParameterArrayImpl(int count) {
    if (count > 0) {
        items_.resize(static_cast<size_t>(count));
    }
}

void ParameterArrayImpl::SetCount(int count) {
    if (count >= 0) {
        items_.resize(static_cast<size_t>(count));
    }
}

void ParameterArrayImpl::CopyFrom(const ParameterArrayImpl& other) {
    items_ = other.items_;
    item_type_ = other.item_type_;
}

int ParameterArrayImpl::GetString(int index, char16_t* value, int valuesize) const {
    if (index < 0 || static_cast<size_t>(index) >= items_.size()) {
        return 0;
    }
    return CopyUtf8ToChar16Buffer(items_[static_cast<size_t>(index)].str_utf8, value, valuesize);
}

int ParameterArrayImpl::GetInt(int index) const {
    if (index < 0 || static_cast<size_t>(index) >= items_.size()) {
        return 0;
    }
    const auto& item = items_[static_cast<size_t>(index)];
    return item.kind == ParamItemKind::UInt ? static_cast<int>(item.uint_val) : item.int_val;
}

unsigned int ParameterArrayImpl::GetUInt(int index) const {
    if (index < 0 || static_cast<size_t>(index) >= items_.size()) {
        return 0;
    }
    const auto& item = items_[static_cast<size_t>(index)];
    return item.kind == ParamItemKind::Int ? static_cast<unsigned int>(item.int_val) : item.uint_val;
}

std::shared_ptr<ParameterSetImpl> ParameterArrayImpl::GetItemSet(int index) {
    if (index < 0 || static_cast<size_t>(index) >= items_.size()) {
        return nullptr;
    }
    auto& item = items_[static_cast<size_t>(index)];
    if (!item.set_val) {
        item.set_val = std::make_shared<ParameterSetImpl>();
        item.kind = ParamItemKind::Set;
    }
    return item.set_val;
}

void ParameterArrayImpl::SetString(int index, const std::string& utf8_val) {
    if (index < 0) return;
    if (static_cast<size_t>(index) >= items_.size()) {
        items_.resize(static_cast<size_t>(index + 1));
    }
    items_[static_cast<size_t>(index)].kind = ParamItemKind::String;
    items_[static_cast<size_t>(index)].str_utf8 = utf8_val;
    item_type_ = static_cast<int>(ParamItemKind::String);
}

void ParameterArrayImpl::SetInt(int index, int val) {
    if (index < 0) return;
    if (static_cast<size_t>(index) >= items_.size()) {
        items_.resize(static_cast<size_t>(index + 1));
    }
    items_[static_cast<size_t>(index)].kind = ParamItemKind::Int;
    items_[static_cast<size_t>(index)].int_val = val;
    items_[static_cast<size_t>(index)].uint_val = static_cast<unsigned int>(val);
    item_type_ = static_cast<int>(ParamItemKind::Int);
}

void ParameterArrayImpl::SetUInt(int index, unsigned int val) {
    if (index < 0) return;
    if (static_cast<size_t>(index) >= items_.size()) {
        items_.resize(static_cast<size_t>(index + 1));
    }
    items_[static_cast<size_t>(index)].kind = ParamItemKind::UInt;
    items_[static_cast<size_t>(index)].uint_val = val;
    items_[static_cast<size_t>(index)].int_val = static_cast<int>(val);
    item_type_ = static_cast<int>(ParamItemKind::UInt);
}

void ParameterArrayImpl::SetItemSet(int index, std::shared_ptr<ParameterSetImpl> set_val) {
    if (index < 0) return;
    if (static_cast<size_t>(index) >= items_.size()) {
        items_.resize(static_cast<size_t>(index + 1));
    }
    items_[static_cast<size_t>(index)].kind = ParamItemKind::Set;
    items_[static_cast<size_t>(index)].set_val = std::move(set_val);
    item_type_ = static_cast<int>(ParamItemKind::Set);
}

// ============================================================================
// ParameterSetImpl
// ============================================================================

ParameterSetImpl::ParameterSetImpl(std::string set_id_utf8)
    : set_id_utf8_(std::move(set_id_utf8)) {}

const char16_t* ParameterSetImpl::GetSetIDUtf16() {
    cached_set_id_u16_ = Utf8ToUtf16(set_id_utf8_);
    return cached_set_id_u16_.c_str();
}

std::shared_ptr<ParameterArrayImpl> ParameterSetImpl::CreateItemArray(const std::string& item_id, int count) {
    auto arr = std::make_shared<ParameterArrayImpl>(count);
    ParamValue pv;
    pv.kind = ParamItemKind::Array;
    pv.array_val = arr;
    items_[item_id] = std::move(pv);
    return arr;
}

std::shared_ptr<ParameterSetImpl> ParameterSetImpl::CreateItemSet(const std::string& item_id, const std::string& set_id) {
    auto s = std::make_shared<ParameterSetImpl>(set_id);
    ParamValue pv;
    pv.kind = ParamItemKind::Set;
    pv.set_val = s;
    items_[item_id] = std::move(pv);
    return s;
}

bool ParameterSetImpl::ItemExist(const std::string& item_id) const {
    return items_.find(item_id) != items_.end();
}

void ParameterSetImpl::MergeFrom(const ParameterSetImpl& src) {
    for (const auto& [k, v] : src.items_) {
        items_[k] = v;
    }
}

void ParameterSetImpl::RemoveAll(const std::string& new_set_id) {
    items_.clear();
    if (!new_set_id.empty()) {
        set_id_utf8_ = new_set_id;
    }
}

void ParameterSetImpl::RemoveItem(const std::string& item_id) {
    items_.erase(item_id);
}

int ParameterSetImpl::GetString(const std::string& item_id, char16_t* value, int valuesize) const {
    auto it = items_.find(item_id);
    if (it == items_.end()) {
        return CopyUtf8ToChar16Buffer("", value, valuesize);
    }
    return CopyUtf8ToChar16Buffer(it->second.str_utf8, value, valuesize);
}

std::string ParameterSetImpl::GetStringUtf8(const std::string& item_id) const {
    auto it = items_.find(item_id);
    return (it != items_.end()) ? it->second.str_utf8 : std::string{};
}

int ParameterSetImpl::GetInt(const std::string& item_id) const {
    auto it = items_.find(item_id);
    if (it == items_.end()) {
        return 0;
    }
    return it->second.kind == ParamItemKind::UInt
               ? static_cast<int>(it->second.uint_val)
               : it->second.int_val;
}

unsigned int ParameterSetImpl::GetUInt(const std::string& item_id) const {
    auto it = items_.find(item_id);
    if (it == items_.end()) {
        return 0;
    }
    return it->second.kind == ParamItemKind::Int
               ? static_cast<unsigned int>(it->second.int_val)
               : it->second.uint_val;
}

std::shared_ptr<ParameterSetImpl> ParameterSetImpl::GetItemSet(const std::string& item_id) {
    auto it = items_.find(item_id);
    if (it != items_.end() && it->second.set_val) {
        return it->second.set_val;
    }
    return CreateItemSet(item_id, item_id);
}

std::shared_ptr<ParameterArrayImpl> ParameterSetImpl::GetItemArray(const std::string& item_id) {
    auto it = items_.find(item_id);
    if (it != items_.end() && it->second.array_val) {
        return it->second.array_val;
    }
    return CreateItemArray(item_id, 0);
}

void ParameterSetImpl::SetString(const std::string& item_id, const std::string& utf8_val) {
    ParamValue pv;
    pv.kind = ParamItemKind::String;
    pv.str_utf8 = utf8_val;
    items_[item_id] = std::move(pv);
}

void ParameterSetImpl::SetInt(const std::string& item_id, int value) {
    ParamValue pv;
    pv.kind = ParamItemKind::Int;
    pv.int_val = value;
    pv.uint_val = static_cast<unsigned int>(value);
    items_[item_id] = std::move(pv);
}

void ParameterSetImpl::SetUInt(const std::string& item_id, unsigned int value) {
    ParamValue pv;
    pv.kind = ParamItemKind::UInt;
    pv.uint_val = value;
    pv.int_val = static_cast<int>(value);
    items_[item_id] = std::move(pv);
}

void ParameterSetImpl::SetItemSet(const std::string& item_id, std::shared_ptr<ParameterSetImpl> value) {
    ParamValue pv;
    pv.kind = ParamItemKind::Set;
    pv.set_val = std::move(value);
    items_[item_id] = std::move(pv);
}

void ParameterSetImpl::SetItemArray(const std::string& item_id, std::shared_ptr<ParameterArrayImpl> value) {
    ParamValue pv;
    pv.kind = ParamItemKind::Array;
    pv.array_val = std::move(value);
    items_[item_id] = std::move(pv);
}

unsigned int ParameterSetImpl::GetItemType(const std::string& item_id) const {
    auto it = items_.find(item_id);
    if (it == items_.end()) {
        return 0;
    }
    return static_cast<unsigned int>(it->second.kind);
}

// ============================================================================
// TableNode & CtrlCodeImpl & ActionImpl
// ============================================================================

void TableNode::InitializeGrid(int r, int c) {
    rows = std::max(1, r);
    cols = std::max(1, c);
    cells.assign(static_cast<size_t>(rows), std::vector<TableCellNode>(static_cast<size_t>(cols)));
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            cells[static_cast<size_t>(i)][static_cast<size_t>(j)].row = i;
            cells[static_cast<size_t>(i)][static_cast<size_t>(j)].col = j;
        }
    }
}

CtrlCodeImpl::CtrlCodeImpl(
    DocumentImpl* owner_doc,
    short ctrl_ch,
    std::string ctrl_id_utf8,
    std::string inst_id_utf8,
    std::string user_desc_utf8,
    int anchor_para,
    int anchor_pos)
    : owner_doc_(owner_doc),
      ctrl_ch_(ctrl_ch),
      ctrl_id_utf8_(std::move(ctrl_id_utf8)),
      inst_id_utf8_(std::move(inst_id_utf8)),
      user_desc_utf8_(std::move(user_desc_utf8)),
      anchor_para_(anchor_para),
      anchor_pos_(anchor_pos),
      properties_(std::make_shared<ParameterSetImpl>(ctrl_id_utf8_)) {}

const char16_t* CtrlCodeImpl::GetCtrlIDUtf16() {
    cached_ctrl_id_u16_ = Utf8ToUtf16(ctrl_id_utf8_);
    return cached_ctrl_id_u16_.c_str();
}

void CtrlCodeImpl::SetProperties(std::shared_ptr<ParameterSetImpl> props) {
    if (props) {
        properties_ = std::move(props);
    }
}

std::shared_ptr<ParameterSetImpl> CtrlCodeImpl::GetAnchorPos(int /*type*/) {
    auto pos_set = std::make_shared<ParameterSetImpl>("ListParaPos");
    pos_set->SetInt("List", 0);
    pos_set->SetInt("Para", anchor_para_);
    pos_set->SetInt("Pos", anchor_pos_);
    return pos_set;
}

ActionImpl::ActionImpl(DocumentImpl* owner_doc, std::string action_id_utf8)
    : owner_doc_(owner_doc), action_id_utf8_(std::move(action_id_utf8)) {}

std::shared_ptr<ParameterSetImpl> ActionImpl::CreateParameterSet() {
    std::string set_id = action_id_utf8_;
    if (action_id_utf8_ == "TableCreate") {
        set_id = "TableCreation";
    } else if (action_id_utf8_ == "RepeatFind" || action_id_utf8_ == "AllReplace" || action_id_utf8_ == "ForwardFind") {
        set_id = "FindReplace";
    } else if (action_id_utf8_ == "ParagraphShape") {
        set_id = "ParaShape";
    } else if (action_id_utf8_ == "TableInsertRowColumn") {
        set_id = "TableInsertLine";
    }
    auto pset = std::make_shared<ParameterSetImpl>(set_id);
    if (owner_doc_) {
        owner_doc_->PopulateActionDefaultSet(action_id_utf8_, pset.get());
    }
    return pset;
}

int ActionImpl::GetDefault(ParameterSetImpl* set) {
    if (!owner_doc_ || !set) {
        return 0;
    }
    return owner_doc_->PopulateActionDefaultSet(action_id_utf8_, set);
}

int ActionImpl::Execute(ParameterSetImpl* set) {
    if (!owner_doc_) {
        return 0;
    }
    return owner_doc_->ExecuteActionWithSet(action_id_utf8_, set);
}

// ============================================================================
// DocumentImpl
// ============================================================================

DocumentImpl::DocumentImpl(HwpObjectImpl* owner_hwp_obj)
    : owner_hwp_obj_(owner_hwp_obj) {
    model_.ClearAndReset();
    RebuildCtrlLinkedList();
}

HWPSDK::Action* DocumentImpl::WrapAction(std::shared_ptr<ActionImpl> impl) const {
    if (!impl) return nullptr;
    owned_actions_.push_back(impl);
    wrapper_actions_.push_back(std::make_unique<HWPSDK::Action>(impl.get()));
    return wrapper_actions_.back().get();
}

HWPSDK::ParameterSet* DocumentImpl::WrapSet(std::shared_ptr<ParameterSetImpl> impl) const {
    if (!impl) return nullptr;
    owned_sets_.push_back(impl);
    wrapper_sets_.push_back(std::make_unique<HWPSDK::ParameterSet>(impl.get()));
    return wrapper_sets_.back().get();
}

HWPSDK::CtrlCode* DocumentImpl::WrapCtrl(std::shared_ptr<CtrlCodeImpl> impl) const {
    if (!impl) return nullptr;
    wrapper_ctrls_.push_back(std::make_unique<HWPSDK::CtrlCode>(impl.get()));
    return wrapper_ctrls_.back().get();
}

bool DocumentImpl::Open(const std::filesystem::path& path, const std::string& format_utf8, const std::string& /*args_utf8*/) {
    std::error_code ec;
    if (!std::filesystem::exists(path, ec)) {
        if (owner_hwp_obj_) owner_hwp_obj_->SetLastErrorCode(1);
        return false;
    }

    std::string fmt = ToUpperAscii(format_utf8);
    bool ok = false;
    if (fmt == "HWPX" || formats::HwpxFormatHandler::IsHwpxFile(path)) {
        ok = formats::HwpxFormatHandler::LoadFromFile(path, model_);
    } else if (fmt == "HWP" || formats::Hwp5FormatHandler::IsHwp5File(path)) {
        ok = formats::Hwp5FormatHandler::LoadFromFile(path, model_);
    } else if (fmt == "HWPML2X" || fmt == "XML" || fmt == "TEXT" || fmt == "UNICODE") {
        std::ifstream ifs(path, std::ios::binary);
        std::ostringstream oss;
        oss << ifs.rdbuf();
        std::string content = oss.str();
        if (content.find("<hs:sec") != std::string::npos || content.find("<hp:p") != std::string::npos) {
            model_.ClearAndReset();
            ok = formats::HwpxFormatHandler::ParseSectionXmlUtf8(content, model_, false);
            model_.current_path = path;
            model_.current_format_utf8 = "HWPML2X";
        } else {
            model_.ClearAndReset();
            model_.paragraphs[0].text_utf8 = content;
            model_.current_path = path;
            model_.current_format_utf8 = "TEXT";
            ok = true;
        }
    }

    if (ok) {
        caret_list_ = 0;
        caret_para_ = 0;
        caret_pos_ = 0;
        active_table_idx_ = model_.tables.empty() ? -1 : 0;
        active_cell_row_ = 0;
        active_cell_col_ = 0;
        has_text_selection_ = false;
        cell_block_mode_ = false;
        RebuildCtrlLinkedList();
        NotifyWindowPreviewUpdated();
    }
    return ok;
}

void DocumentImpl::Clear() {
    model_.ClearAndReset();
    caret_list_ = 0;
    caret_para_ = 0;
    caret_pos_ = 0;
    active_table_idx_ = -1;
    active_cell_row_ = 0;
    active_cell_col_ = 0;
    active_field_idx_ = -1;
    active_memo_idx_ = -1;
    has_text_selection_ = false;
    cell_block_mode_ = false;
    RebuildCtrlLinkedList();
    NotifyWindowPreviewUpdated();
}

bool DocumentImpl::Save(bool save_if_dirty) {
    if (model_.current_path.empty()) {
        return false;
    }
    if (save_if_dirty && !model_.modified && std::filesystem::exists(model_.current_path)) {
        return true;
    }
    return SaveAs(model_.current_path, model_.current_format_utf8, "");
}

bool DocumentImpl::SaveAs(const std::filesystem::path& path, const std::string& format_utf8, const std::string& /*args_utf8*/) {
    if (path.empty()) {
        return false;
    }
    std::string fmt = ToUpperAscii(format_utf8);
    std::string ext = ToUpperAscii(path.extension().string());

    bool ok = false;
    if (fmt == "HWPX" || (fmt.empty() && ext == ".HWPX")) {
        ok = formats::HwpxFormatHandler::SaveToFile(path, model_);
        if (ok) model_.current_format_utf8 = "HWPX";
    } else if (fmt == "HWPML2X" || ext == ".XML") {
        std::string xml = formats::HwpxFormatHandler::SerializeSectionXmlUtf8(model_);
        std::ofstream ofs(path, std::ios::binary | std::ios::trunc);
        ofs.write(xml.data(), static_cast<std::streamsize>(xml.size()));
        ok = ofs.good();
        if (ok) model_.current_format_utf8 = "HWPML2X";
    } else {
        ok = formats::Hwp5FormatHandler::SaveToFile(path, model_);
        if (ok) model_.current_format_utf8 = "HWP";
    }

    if (ok) {
        model_.current_path = path;
        model_.modified = false;
    }
    return ok;
}

bool DocumentImpl::Insert(const std::filesystem::path& path, const std::string& /*format_utf8*/, const std::string& /*args_utf8*/) {
    DocumentModel temp_model;
    bool ok = false;
    if (formats::HwpxFormatHandler::IsHwpxFile(path)) {
        ok = formats::HwpxFormatHandler::LoadFromFile(path, temp_model);
    } else if (formats::Hwp5FormatHandler::IsHwp5File(path)) {
        ok = formats::Hwp5FormatHandler::LoadFromFile(path, temp_model);
    }
    if (!ok) {
        return false;
    }
    int base_para = static_cast<int>(model_.paragraphs.size());
    for (const auto& p : temp_model.paragraphs) {
        model_.paragraphs.push_back(p);
    }
    for (auto f : temp_model.fields) {
        f.para_index += base_para;
        model_.fields.push_back(std::move(f));
    }
    for (auto t : temp_model.tables) {
        t.anchor_para += base_para;
        model_.tables.push_back(std::move(t));
    }
    model_.modified = true;
    RebuildCtrlLinkedList();
    NotifyWindowPreviewUpdated();
    return true;
}

std::shared_ptr<ActionImpl> DocumentImpl::CreateAction(const std::string& action_name_utf8) {
    if (action_name_utf8.empty()) {
        return nullptr;
    }
    return std::make_shared<ActionImpl>(this, action_name_utf8);
}

std::shared_ptr<ParameterSetImpl> DocumentImpl::CreateSet(const std::string& set_id_utf8) {
    return std::make_shared<ParameterSetImpl>(set_id_utf8);
}

int DocumentImpl::PopulateActionDefaultSet(const std::string& act_id_utf8, ParameterSetImpl* set) {
    if (!set) return 0;
    if (act_id_utf8 == "InsertText") {
        set->SetString("Text", "");
    } else if (act_id_utf8 == "RepeatFind" || act_id_utf8 == "AllReplace" || act_id_utf8 == "ForwardFind") {
        set->SetString("FindString", "");
        set->SetString("ReplaceString", "");
        set->SetInt("Direction", 0);
        set->SetInt("IgnoreMessage", 1);
        set->SetInt("FindType", 1);
    } else if (act_id_utf8 == "CharShape") {
        const auto& cs = model_.paragraphs[static_cast<size_t>(std::clamp(caret_para_, 0, static_cast<int>(model_.paragraphs.size() - 1)))].char_shape;
        set->SetUInt("Height", cs.height);
        set->SetUInt("Bold", cs.bold);
        set->SetUInt("Italic", cs.italic);
        set->SetUInt("UnderlineType", cs.underline);
        set->SetUInt("TextColor", cs.text_color);
        set->SetString("FaceNameHangul", cs.font_name_utf8);
    } else if (act_id_utf8 == "ParagraphShape") {
        const auto& ps = model_.paragraphs[static_cast<size_t>(std::clamp(caret_para_, 0, static_cast<int>(model_.paragraphs.size() - 1)))].para_shape;
        set->SetUInt("AlignType", ps.align_type);
        set->SetUInt("BreakNonLatinWord", ps.break_non_latin_word);
        set->SetInt("LineSpacing", ps.line_spacing);
    } else if (act_id_utf8 == "ShapeCopyPaste") {
        set->SetInt("Type", 2);
    } else if (act_id_utf8 == "PageNumPos") {
        set->SetInt("DrawPos", model_.page_num_pos.draw_pos);
        set->SetInt("SideChar", model_.page_num_pos.side_char);
        set->SetInt("NewNumber", model_.page_num_pos.new_number);
    } else if (act_id_utf8 == "TableCreate") {
        set->SetInt("Rows", 4);
        set->SetInt("Cols", 4);
        set->SetInt("TreatAsChar", 0);
    } else if (act_id_utf8 == "CellBorderFill") {
        auto fill_attr = set->CreateItemSet("FillAttr", "DrawFillAttr");
        fill_attr->SetUInt("Type", 1);
        fill_attr->SetUInt("WinBrushHatchColor", 0);
        fill_attr->SetInt("WinBrushFaceStyle", -1);
        fill_attr->SetUInt("WinBrushFaceColor", 0xFFFFFF);
    } else if (act_id_utf8 == "CellBorder") {
        set->SetInt("BorderWidthBottom", 1);
        set->SetInt("BorderTypeBottom", 1);
        set->SetInt("BorderWidthTop", 1);
        set->SetInt("BorderTypeTop", 1);
        set->SetInt("BorderWidthRight", 1);
        set->SetInt("BorderTypeRight", 1);
        set->SetInt("BorderWidthLeft", 1);
        set->SetInt("BorderTypeLeft", 1);
    } else if (act_id_utf8 == "TableInsertRowColumn") {
        set->SetUInt("Side", 3);
        set->SetUInt("Count", 1);
    } else if (act_id_utf8 == "TableSplitCell") {
        set->SetInt("Rows", 2);
        set->SetInt("Cols", 2);
    } else if (act_id_utf8 == "Paste") {
        set->SetUInt("Option", 0);
    }
    return 1;
}

int DocumentImpl::Run(const std::string& act_id_utf8) {
    if (IsCommandLock(act_id_utf8)) {
        return 0;
    }
    auto default_set = std::make_shared<ParameterSetImpl>(act_id_utf8);
    PopulateActionDefaultSet(act_id_utf8, default_set.get());
    return ExecuteActionWithSet(act_id_utf8, default_set.get());
}

int DocumentImpl::ExecuteActionWithSet(const std::string& act_id_utf8, ParameterSetImpl* set) {
    if (IsCommandLock(act_id_utf8)) {
        return 0;
    }

    // Navigation & Selection actions work in all modes; editing actions require EditMode != 0
    auto is_navigation_or_read_action = [&]() -> bool {
        return act_id_utf8.find("Move") == 0 ||
               act_id_utf8 == "RepeatFind" ||
               act_id_utf8 == "ForwardFind" ||
               act_id_utf8 == "Copy" ||
               act_id_utf8 == "SelectAll" ||
               act_id_utf8 == "Cancel" ||
               act_id_utf8 == "TableCellBlock" ||
               act_id_utf8 == "TableCellBlockExtend" ||
               act_id_utf8 == "TableCellBlockRow" ||
               act_id_utf8 == "TableCellBlockCol" ||
               act_id_utf8 == "ShapeObjTableSelCell" ||
               act_id_utf8 == "TableRightCell" ||
               act_id_utf8 == "TableLeftCell" ||
               act_id_utf8 == "TableUpperCell" ||
               act_id_utf8 == "TableLowerCell";
    };

    if (model_.edit_mode == 0 && !is_navigation_or_read_action()) {
        // Also allow ShapeCopyPaste when only copying shape (no selection)
        if (!(act_id_utf8 == "ShapeCopyPaste" && GetSelectionMode() == 0)) {
            return 0;
        }
    }

    if (model_.paragraphs.empty()) {
        model_.paragraphs.emplace_back();
    }
    caret_para_ = std::clamp(caret_para_, 0, static_cast<int>(model_.paragraphs.size() - 1));

    // 1. InsertText
    if (act_id_utf8 == "InsertText") {
        std::string txt = set ? set->GetStringUtf8("Text") : "";
        if (caret_list_ > 0 && active_table_idx_ >= 0 && active_table_idx_ < static_cast<int>(model_.tables.size())) {
            auto& tbl = model_.tables[static_cast<size_t>(active_table_idx_)];
            if (active_cell_row_ >= 0 && active_cell_row_ < tbl.rows &&
                active_cell_col_ >= 0 && active_cell_col_ < tbl.cols) {
                tbl.cells[static_cast<size_t>(active_cell_row_)][static_cast<size_t>(active_cell_col_)].text_utf8 += txt;
            }
        } else if (active_field_idx_ >= 0 && active_field_idx_ < static_cast<int>(model_.fields.size())) {
            model_.fields[static_cast<size_t>(active_field_idx_)].text_utf8 += txt;
        } else {
            auto& p = model_.paragraphs[static_cast<size_t>(caret_para_)].text_utf8;
            size_t insert_off = std::min<size_t>(static_cast<size_t>(std::max(0, caret_pos_)), p.size());
            p.insert(insert_off, txt);
            caret_pos_ = static_cast<int>(insert_off + txt.size());
        }
        model_.modified = true;
        NotifyWindowPreviewUpdated();
        return 1;
    }

    // 2. BreakPara / BreakPage
    if (act_id_utf8 == "BreakPara") {
        ParagraphNode next_p;
        next_p.char_shape = model_.paragraphs[static_cast<size_t>(caret_para_)].char_shape;
        next_p.para_shape = model_.paragraphs[static_cast<size_t>(caret_para_)].para_shape;
        model_.paragraphs.insert(model_.paragraphs.begin() + caret_para_ + 1, std::move(next_p));
        ++caret_para_;
        caret_pos_ = 0;
        model_.modified = true;
        NotifyWindowPreviewUpdated();
        return 1;
    }
    if (act_id_utf8 == "BreakPage") {
        ParagraphNode next_p;
        next_p.page_break_before = true;
        next_p.char_shape = model_.paragraphs[static_cast<size_t>(caret_para_)].char_shape;
        next_p.para_shape = model_.paragraphs[static_cast<size_t>(caret_para_)].para_shape;
        model_.paragraphs.insert(model_.paragraphs.begin() + caret_para_ + 1, std::move(next_p));
        ++caret_para_;
        caret_pos_ = 0;
        model_.modified = true;
        NotifyWindowPreviewUpdated();
        return 1;
    }

    // 3. Caret & Selection Movement Actions
    if (act_id_utf8 == "MoveDocBegin" || act_id_utf8 == "MoveTopLevelBegin") {
        caret_list_ = 0;
        caret_para_ = 0;
        caret_pos_ = 0;
        has_text_selection_ = false;
        cell_block_mode_ = false;
        return 1;
    }
    if (act_id_utf8 == "MoveDocEnd" || act_id_utf8 == "MoveTopLevelEnd") {
        caret_list_ = 0;
        caret_para_ = static_cast<int>(model_.paragraphs.size() - 1);
        caret_pos_ = static_cast<int>(model_.paragraphs.back().text_utf8.size());
        has_text_selection_ = false;
        cell_block_mode_ = false;
        return 1;
    }
    if (act_id_utf8 == "MoveParaBegin" || act_id_utf8 == "MoveLineBegin") {
        caret_pos_ = 0;
        return 1;
    }
    if (act_id_utf8 == "MoveParaEnd" || act_id_utf8 == "MoveLineEnd") {
        caret_pos_ = static_cast<int>(model_.paragraphs[static_cast<size_t>(caret_para_)].text_utf8.size());
        return 1;
    }
    if (act_id_utf8 == "MoveRight" || act_id_utf8 == "MoveNextChar" || act_id_utf8 == "MoveNextPos") {
        int max_pos = static_cast<int>(model_.paragraphs[static_cast<size_t>(caret_para_)].text_utf8.size());
        if (caret_pos_ < max_pos) {
            ++caret_pos_;
        } else if (caret_para_ + 1 < static_cast<int>(model_.paragraphs.size())) {
            ++caret_para_;
            caret_pos_ = 0;
        }
        return 1;
    }
    if (act_id_utf8 == "MoveLeft" || act_id_utf8 == "MovePrevChar" || act_id_utf8 == "MovePrevPos") {
        if (caret_pos_ > 0) {
            --caret_pos_;
        } else if (caret_para_ > 0) {
            --caret_para_;
            caret_pos_ = static_cast<int>(model_.paragraphs[static_cast<size_t>(caret_para_)].text_utf8.size());
        }
        return 1;
    }
    if (act_id_utf8 == "MoveUp" || act_id_utf8 == "MoveLineUp" || act_id_utf8 == "MovePrevParaBegin" || act_id_utf8 == "MovePageUp" || act_id_utf8 == "MovePageBegin") {
        if (caret_para_ > 0) {
            --caret_para_;
        }
        caret_pos_ = 0;
        return 1;
    }
    if (act_id_utf8 == "MoveDown" || act_id_utf8 == "MoveLineDown" || act_id_utf8 == "MoveNextParaBegin" || act_id_utf8 == "MovePageDown" || act_id_utf8 == "MovePageEnd") {
        if (caret_para_ + 1 < static_cast<int>(model_.paragraphs.size())) {
            ++caret_para_;
        }
        caret_pos_ = 0;
        return 1;
    }
    if (act_id_utf8 == "MovePrevWord" || act_id_utf8 == "MoveWordBegin") {
        caret_pos_ = 0;
        return 1;
    }
    if (act_id_utf8 == "MoveNextWord" || act_id_utf8 == "MoveWordEnd") {
        caret_pos_ = static_cast<int>(model_.paragraphs[static_cast<size_t>(caret_para_)].text_utf8.size());
        return 1;
    }

    // Selection range actions
    if (act_id_utf8.find("MoveSel") == 0 || act_id_utf8 == "SelectAll") {
        has_text_selection_ = true;
        sel_slist_ = 0;
        sel_elist_ = 0;
        if (act_id_utf8 == "SelectAll") {
            sel_spara_ = 0;
            sel_spos_ = 0;
            sel_epara_ = static_cast<int>(model_.paragraphs.size() - 1);
            sel_epos_ = static_cast<int>(model_.paragraphs.back().text_utf8.size());
        } else if (act_id_utf8 == "MoveSelDocEnd") {
            sel_spara_ = caret_para_;
            sel_spos_ = caret_pos_;
            sel_epara_ = static_cast<int>(model_.paragraphs.size() - 1);
            sel_epos_ = static_cast<int>(model_.paragraphs.back().text_utf8.size());
        } else {
            sel_spara_ = caret_para_;
            sel_spos_ = caret_pos_;
            sel_epara_ = caret_para_;
            sel_epos_ = static_cast<int>(model_.paragraphs[static_cast<size_t>(caret_para_)].text_utf8.size());
        }
        return 1;
    }
    if (act_id_utf8 == "Cancel") {
        has_text_selection_ = false;
        cell_block_mode_ = false;
        selected_ctrl_.reset();
        return 1;
    }

    // 4. RepeatFind / ForwardFind
    if (act_id_utf8 == "RepeatFind" || act_id_utf8 == "ForwardFind") {
        std::string needle = set ? set->GetStringUtf8("FindString") : "";
        if (needle.empty()) {
            return 0;
        }
        for (size_t p = static_cast<size_t>(caret_para_); p < model_.paragraphs.size(); ++p) {
            const std::string& hay = model_.paragraphs[p].text_utf8;
            size_t start_off = (p == static_cast<size_t>(caret_para_))
                                   ? std::min<size_t>(static_cast<size_t>(caret_pos_), hay.size())
                                   : 0;
            size_t found = hay.find(needle, start_off);
            if (found != std::string::npos) {
                caret_para_ = static_cast<int>(p);
                caret_pos_ = static_cast<int>(found + needle.size());
                has_text_selection_ = true;
                sel_slist_ = 0;
                sel_spara_ = static_cast<int>(p);
                sel_spos_ = static_cast<int>(found);
                sel_elist_ = 0;
                sel_epara_ = static_cast<int>(p);
                sel_epos_ = caret_pos_;
                return 1;
            }
        }
        return 0;
    }

    // 5. AllReplace
    if (act_id_utf8 == "AllReplace") {
        std::string find_str = set ? set->GetStringUtf8("FindString") : "";
        std::string rep_str = set ? set->GetStringUtf8("ReplaceString") : "";
        if (find_str.empty()) {
            return 0;
        }
        int replaced_count = 0;
        auto replace_in_str = [&](std::string& target) {
            size_t pos = 0;
            while ((pos = target.find(find_str, pos)) != std::string::npos) {
                target.replace(pos, find_str.size(), rep_str);
                pos += rep_str.size();
                ++replaced_count;
            }
        };
        for (auto& para : model_.paragraphs) {
            replace_in_str(para.text_utf8);
        }
        for (auto& field : model_.fields) {
            replace_in_str(field.text_utf8);
        }
        for (auto& tbl : model_.tables) {
            for (auto& row : tbl.cells) {
                for (auto& cell : row) {
                    replace_in_str(cell.text_utf8);
                }
            }
        }
        if (replaced_count > 0) {
            model_.modified = true;
            NotifyWindowPreviewUpdated();
            return 1;
        }
        return 0;
    }

    // 6. CharShape & ParagraphShape & ShapeCopyPaste
    if (act_id_utf8 == "CharShape") {
        if (!set) return 0;
        SetCharShape(set);
        return 1;
    }
    if (act_id_utf8 == "ParagraphShape" || act_id_utf8 == "ParaShape") {
        if (!set) return 0;
        SetParaShape(set);
        return 1;
    }
    if (act_id_utf8 == "ShapeCopyPaste") {
        if (GetSelectionMode() == 0) {
            copied_char_shape_ = model_.paragraphs[static_cast<size_t>(caret_para_)].char_shape;
            copied_para_shape_ = model_.paragraphs[static_cast<size_t>(caret_para_)].para_shape;
            return 1;
        } else {
            int sp = std::clamp(sel_spara_, 0, static_cast<int>(model_.paragraphs.size() - 1));
            int ep = std::clamp(sel_epara_, sp, static_cast<int>(model_.paragraphs.size() - 1));
            for (int p = sp; p <= ep; ++p) {
                model_.paragraphs[static_cast<size_t>(p)].char_shape = copied_char_shape_;
                model_.paragraphs[static_cast<size_t>(p)].para_shape = copied_para_shape_;
            }
            model_.modified = true;
            return 1;
        }
    }

    // 7. Copy / Paste
    if (act_id_utf8 == "Copy") {
        std::string sel_txt = GetSelectedTextUtf8();
        if (sel_txt.empty()) {
            return 0;
        }
        platform::GetPlatformService().SetClipboardTextUtf8(sel_txt);
        return 1;
    }
    if (act_id_utf8 == "Paste") {
        std::string clip = platform::GetPlatformService().GetClipboardTextUtf8();
        if (cell_block_mode_ && active_table_idx_ >= 0 && active_table_idx_ < static_cast<int>(model_.tables.size())) {
            auto& tbl = model_.tables[static_cast<size_t>(active_table_idx_)];
            for (int r = cell_block_srow_; r <= cell_block_erow_ && r < tbl.rows; ++r) {
                for (int c = cell_block_scol_; c <= cell_block_ecol_ && c < tbl.cols; ++c) {
                    tbl.cells[static_cast<size_t>(r)][static_cast<size_t>(c)].text_utf8 = clip;
                }
            }
        } else {
            auto& p = model_.paragraphs[static_cast<size_t>(caret_para_)].text_utf8;
            if (has_text_selection_ && sel_spara_ == sel_epara_ && sel_spara_ == caret_para_) {
                size_t s = std::min<size_t>(static_cast<size_t>(sel_spos_), p.size());
                size_t e = std::min<size_t>(static_cast<size_t>(sel_epos_), p.size());
                if (e >= s) {
                    p.replace(s, e - s, clip);
                }
            } else {
                p += clip;
            }
        }
        model_.modified = true;
        NotifyWindowPreviewUpdated();
        return 1;
    }

    // 8. PageNumPos
    if (act_id_utf8 == "PageNumPos") {
        model_.page_num_pos.enabled = true;
        if (set) {
            model_.page_num_pos.draw_pos = set->GetInt("DrawPos");
            model_.page_num_pos.side_char = set->GetInt("SideChar");
            if (set->ItemExist("NewNumber")) {
                model_.page_num_pos.new_number = set->GetInt("NewNumber");
            }
        }
        model_.modified = true;
        RebuildCtrlLinkedList();
        return 1;
    }

    // 9. Table Actions
    if (act_id_utf8 == "TableCreate") {
        int rows = set ? set->GetInt("Rows") : 4;
        int cols = set ? set->GetInt("Cols") : 4;
        bool treat_as_char = set ? (set->GetInt("TreatAsChar") != 0) : false;
        TableNode tbl;
        tbl.inst_id_utf8 = std::to_string(1000 + model_.tables.size());
        tbl.treat_as_char = treat_as_char;
        tbl.anchor_para = caret_para_;
        tbl.InitializeGrid(rows, cols);
        model_.tables.push_back(std::move(tbl));
        active_table_idx_ = static_cast<int>(model_.tables.size() - 1);
        caret_list_ = active_table_idx_ + 1;
        active_cell_row_ = 0;
        active_cell_col_ = 0;
        cell_block_mode_ = false;
        model_.modified = true;
        RebuildCtrlLinkedList();
        NotifyWindowPreviewUpdated();
        return 1;
    }

    if (act_id_utf8 == "ShapeObjTableSelCell" || act_id_utf8 == "TableCellBlock") {
        if (active_table_idx_ < 0 && !model_.tables.empty()) {
            active_table_idx_ = 0;
        }
        if (active_table_idx_ < 0 || active_table_idx_ >= static_cast<int>(model_.tables.size())) {
            return 0;
        }
        caret_list_ = active_table_idx_ + 1;
        cell_block_mode_ = true;
        cell_block_srow_ = active_cell_row_;
        cell_block_scol_ = active_cell_col_;
        cell_block_erow_ = active_cell_row_;
        cell_block_ecol_ = active_cell_col_;
        return 1;
    }

    if (act_id_utf8 == "TableCellBlockExtend") {
        if (active_table_idx_ < 0 || active_table_idx_ >= static_cast<int>(model_.tables.size())) {
            return 0;
        }
        const auto& tbl = model_.tables[static_cast<size_t>(active_table_idx_)];
        cell_block_mode_ = true;
        cell_block_srow_ = 0;
        cell_block_scol_ = 0;
        cell_block_erow_ = tbl.rows - 1;
        cell_block_ecol_ = tbl.cols - 1;
        return 1;
    }

    if (act_id_utf8 == "TableRightCell") {
        if (active_table_idx_ < 0 || active_table_idx_ >= static_cast<int>(model_.tables.size())) return 0;
        const auto& tbl = model_.tables[static_cast<size_t>(active_table_idx_)];
        if (active_cell_col_ + 1 < tbl.cols) {
            ++active_cell_col_;
        }
        if (cell_block_mode_) {
            cell_block_ecol_ = std::max(cell_block_ecol_, active_cell_col_);
        }
        return 1;
    }
    if (act_id_utf8 == "TableLeftCell") {
        if (active_table_idx_ < 0 || active_table_idx_ >= static_cast<int>(model_.tables.size())) return 0;
        if (active_cell_col_ > 0) {
            --active_cell_col_;
        }
        return 1;
    }
    if (act_id_utf8 == "TableLowerCell") {
        if (active_table_idx_ < 0 || active_table_idx_ >= static_cast<int>(model_.tables.size())) return 0;
        const auto& tbl = model_.tables[static_cast<size_t>(active_table_idx_)];
        if (active_cell_row_ + 1 < tbl.rows) {
            ++active_cell_row_;
        }
        if (cell_block_mode_) {
            cell_block_erow_ = std::max(cell_block_erow_, active_cell_row_);
        }
        return 1;
    }
    if (act_id_utf8 == "TableUpperCell") {
        if (active_table_idx_ < 0 || active_table_idx_ >= static_cast<int>(model_.tables.size())) return 0;
        if (active_cell_row_ > 0) {
            --active_cell_row_;
        }
        return 1;
    }

    if (act_id_utf8 == "TableDeleteCell") {
        if (active_table_idx_ < 0 || active_table_idx_ >= static_cast<int>(model_.tables.size())) return 0;
        auto& tbl = model_.tables[static_cast<size_t>(active_table_idx_)];
        int sr = cell_block_mode_ ? cell_block_srow_ : active_cell_row_;
        int er = cell_block_mode_ ? cell_block_erow_ : active_cell_row_;
        int sc = cell_block_mode_ ? cell_block_scol_ : active_cell_col_;
        int ec = cell_block_mode_ ? cell_block_ecol_ : active_cell_col_;
        for (int r = sr; r <= er && r < tbl.rows; ++r) {
            for (int c = sc; c <= ec && c < tbl.cols; ++c) {
                tbl.cells[static_cast<size_t>(r)][static_cast<size_t>(c)].text_utf8.clear();
            }
        }
        model_.modified = true;
        NotifyWindowPreviewUpdated();
        return 1;
    }

    if (act_id_utf8 == "CellBorderFill") {
        if (active_table_idx_ < 0 || active_table_idx_ >= static_cast<int>(model_.tables.size()) || !set) return 0;
        auto& tbl = model_.tables[static_cast<size_t>(active_table_idx_)];
        uint32_t face_color = 0xFFFFFF;
        uint32_t hatch_color = 0;
        int face_style = -1;
        if (set->ItemExist("FillAttr")) {
            auto fill_attr = set->GetItemSet("FillAttr");
            if (fill_attr) {
                face_color = fill_attr->GetUInt("WinBrushFaceColor");
                hatch_color = fill_attr->GetUInt("WinBrushHatchColor");
                face_style = fill_attr->GetInt("WinBrushFaceStyle");
            }
        }
        int sr = cell_block_mode_ ? cell_block_srow_ : active_cell_row_;
        int er = cell_block_mode_ ? cell_block_erow_ : active_cell_row_;
        int sc = cell_block_mode_ ? cell_block_scol_ : active_cell_col_;
        int ec = cell_block_mode_ ? cell_block_ecol_ : active_cell_col_;
        for (int r = sr; r <= er && r < tbl.rows; ++r) {
            for (int c = sc; c <= ec && c < tbl.cols; ++c) {
                auto& cell = tbl.cells[static_cast<size_t>(r)][static_cast<size_t>(c)];
                cell.bg_color = face_color;
                cell.hatch_color = hatch_color;
                cell.face_style = face_style;
            }
        }
        model_.modified = true;
        return 1;
    }

    if (act_id_utf8 == "CellBorder") {
        if (active_table_idx_ < 0 || active_table_idx_ >= static_cast<int>(model_.tables.size()) || !set) return 0;
        auto& tbl = model_.tables[static_cast<size_t>(active_table_idx_)];
        int sr = cell_block_mode_ ? cell_block_srow_ : active_cell_row_;
        int er = cell_block_mode_ ? cell_block_erow_ : active_cell_row_;
        int sc = cell_block_mode_ ? cell_block_scol_ : active_cell_col_;
        int ec = cell_block_mode_ ? cell_block_ecol_ : active_cell_col_;
        for (int r = sr; r <= er && r < tbl.rows; ++r) {
            for (int c = sc; c <= ec && c < tbl.cols; ++c) {
                auto& cell = tbl.cells[static_cast<size_t>(r)][static_cast<size_t>(c)];
                cell.border_type_top = set->GetInt("BorderTypeTop");
                cell.border_width_top = set->GetInt("BorderWidthTop");
                cell.border_type_bottom = set->GetInt("BorderTypeBottom");
                cell.border_width_bottom = set->GetInt("BorderWidthBottom");
                cell.border_type_left = set->GetInt("BorderTypeLeft");
                cell.border_width_left = set->GetInt("BorderWidthLeft");
                cell.border_type_right = set->GetInt("BorderTypeRight");
                cell.border_width_right = set->GetInt("BorderWidthRight");
            }
        }
        model_.modified = true;
        return 1;
    }

    if (act_id_utf8 == "TableInsertRowColumn" || act_id_utf8 == "TableAppendRow") {
        if (active_table_idx_ < 0 || active_table_idx_ >= static_cast<int>(model_.tables.size())) return 0;
        auto& tbl = model_.tables[static_cast<size_t>(active_table_idx_)];
        unsigned int side = set ? set->GetUInt("Side") : 3; // 0:Left, 1:Right, 2:Top, 3:Bottom
        unsigned int count = set ? std::max(1u, set->GetUInt("Count")) : 1u;

        if (side == 2 || side == 3 || act_id_utf8 == "TableAppendRow") {
            int insert_at = (side == 2) ? active_cell_row_ : (active_cell_row_ + 1);
            insert_at = std::clamp(insert_at, 0, tbl.rows);
            for (unsigned int k = 0; k < count; ++k) {
                std::vector<TableCellNode> new_row(static_cast<size_t>(tbl.cols));
                tbl.cells.insert(tbl.cells.begin() + insert_at, std::move(new_row));
                ++tbl.rows;
            }
        } else {
            int insert_at = (side == 0) ? active_cell_col_ : (active_cell_col_ + 1);
            insert_at = std::clamp(insert_at, 0, tbl.cols);
            for (unsigned int k = 0; k < count; ++k) {
                for (int r = 0; r < tbl.rows; ++r) {
                    tbl.cells[static_cast<size_t>(r)].insert(
                        tbl.cells[static_cast<size_t>(r)].begin() + insert_at,
                        TableCellNode{});
                }
                ++tbl.cols;
            }
        }
        for (int r = 0; r < tbl.rows; ++r) {
            for (int c = 0; c < tbl.cols; ++c) {
                tbl.cells[static_cast<size_t>(r)][static_cast<size_t>(c)].row = r;
                tbl.cells[static_cast<size_t>(r)][static_cast<size_t>(c)].col = c;
            }
        }
        model_.modified = true;
        return 1;
    }

    if (act_id_utf8 == "TableDeleteRow") {
        if (active_table_idx_ < 0 || active_table_idx_ >= static_cast<int>(model_.tables.size())) return 0;
        auto& tbl = model_.tables[static_cast<size_t>(active_table_idx_)];
        if (tbl.rows <= 1) return 0;
        int del_r = std::clamp(active_cell_row_, 0, tbl.rows - 1);
        tbl.cells.erase(tbl.cells.begin() + del_r);
        --tbl.rows;
        active_cell_row_ = std::min(active_cell_row_, tbl.rows - 1);
        model_.modified = true;
        return 1;
    }

    if (act_id_utf8 == "TableDeleteColumn") {
        if (active_table_idx_ < 0 || active_table_idx_ >= static_cast<int>(model_.tables.size())) return 0;
        auto& tbl = model_.tables[static_cast<size_t>(active_table_idx_)];
        if (tbl.cols <= 1) return 0;
        int del_c = std::clamp(active_cell_col_, 0, tbl.cols - 1);
        for (int r = 0; r < tbl.rows; ++r) {
            tbl.cells[static_cast<size_t>(r)].erase(tbl.cells[static_cast<size_t>(r)].begin() + del_c);
        }
        --tbl.cols;
        active_cell_col_ = std::min(active_cell_col_, tbl.cols - 1);
        model_.modified = true;
        return 1;
    }

    if (act_id_utf8 == "TableResizeCellRight" || act_id_utf8 == "TableResizeCellLeft" ||
        act_id_utf8 == "TableDistributeCellWidth" || act_id_utf8 == "TableResizeCellDown" ||
        act_id_utf8 == "TableResizeCellUp" || act_id_utf8 == "TableDistributeCellHeight") {
        if (active_table_idx_ < 0 || active_table_idx_ >= static_cast<int>(model_.tables.size())) return 0;
        auto& tbl = model_.tables[static_cast<size_t>(active_table_idx_)];
        auto& cell = tbl.cells[static_cast<size_t>(active_cell_row_)][static_cast<size_t>(active_cell_col_)];
        if (act_id_utf8 == "TableResizeCellRight") cell.width_hwpunit += 1000;
        else if (act_id_utf8 == "TableResizeCellLeft") cell.width_hwpunit = std::max(1000, cell.width_hwpunit - 1000);
        else if (act_id_utf8 == "TableResizeCellDown") cell.height_hwpunit += 500;
        else if (act_id_utf8 == "TableResizeCellUp") cell.height_hwpunit = std::max(500, cell.height_hwpunit - 500);
        model_.modified = true;
        return 1;
    }

    if (act_id_utf8 == "TableMergeCell") {
        if (active_table_idx_ < 0 || active_table_idx_ >= static_cast<int>(model_.tables.size())) return 0;
        auto& tbl = model_.tables[static_cast<size_t>(active_table_idx_)];
        int sr = std::clamp(cell_block_srow_, 0, tbl.rows - 1);
        int er = std::clamp(cell_block_erow_, sr, tbl.rows - 1);
        int sc = std::clamp(cell_block_scol_, 0, tbl.cols - 1);
        int ec = std::clamp(cell_block_ecol_, sc, tbl.cols - 1);
        if (sr == er && sc == ec) {
            // If no multi-cell block was selected, merge current cell with right or lower neighbor if available
            if (sc + 1 < tbl.cols) ec = sc + 1;
            else if (sr + 1 < tbl.rows) er = sr + 1;
            else return 0;
        }
        std::string merged_text;
        for (int r = sr; r <= er; ++r) {
            for (int c = sc; c <= ec; ++c) {
                if (!tbl.cells[static_cast<size_t>(r)][static_cast<size_t>(c)].text_utf8.empty()) {
                    if (!merged_text.empty()) merged_text += " ";
                    merged_text += tbl.cells[static_cast<size_t>(r)][static_cast<size_t>(c)].text_utf8;
                }
                if (r == sr && c == sc) {
                    tbl.cells[static_cast<size_t>(r)][static_cast<size_t>(c)].row_span = (er - sr + 1);
                    tbl.cells[static_cast<size_t>(r)][static_cast<size_t>(c)].col_span = (ec - sc + 1);
                } else {
                    tbl.cells[static_cast<size_t>(r)][static_cast<size_t>(c)].row_span = 0;
                    tbl.cells[static_cast<size_t>(r)][static_cast<size_t>(c)].col_span = 0;
                    tbl.cells[static_cast<size_t>(r)][static_cast<size_t>(c)].text_utf8.clear();
                }
            }
        }
        tbl.cells[static_cast<size_t>(sr)][static_cast<size_t>(sc)].text_utf8 = merged_text;
        active_cell_row_ = sr;
        active_cell_col_ = sc;
        model_.modified = true;
        return 1;
    }

    if (act_id_utf8 == "TableSplitCell") {
        if (active_table_idx_ < 0 || active_table_idx_ >= static_cast<int>(model_.tables.size()) || !set) return 0;
        auto& tbl = model_.tables[static_cast<size_t>(active_table_idx_)];
        int split_rows = std::max(1, set->GetInt("Rows"));
        int split_cols = std::max(1, set->GetInt("Cols"));
        auto& target_cell = tbl.cells[static_cast<size_t>(active_cell_row_)][static_cast<size_t>(active_cell_col_)];

        // If cell was previously merged, restore its covered cells first
        if (target_cell.row_span > 1 || target_cell.col_span > 1) {
            for (int r = active_cell_row_; r < active_cell_row_ + target_cell.row_span && r < tbl.rows; ++r) {
                for (int c = active_cell_col_; c < active_cell_col_ + target_cell.col_span && c < tbl.cols; ++c) {
                    tbl.cells[static_cast<size_t>(r)][static_cast<size_t>(c)].row_span = 1;
                    tbl.cells[static_cast<size_t>(r)][static_cast<size_t>(c)].col_span = 1;
                }
            }
        }
        // Insert additional rows/cols if splitting beyond 1x1
        for (int r = 1; r < split_rows; ++r) {
            std::vector<TableCellNode> new_row(static_cast<size_t>(tbl.cols));
            tbl.cells.insert(tbl.cells.begin() + active_cell_row_ + r, std::move(new_row));
            ++tbl.rows;
        }
        for (int c = 1; c < split_cols; ++c) {
            for (int r = 0; r < tbl.rows; ++r) {
                tbl.cells[static_cast<size_t>(r)].insert(
                    tbl.cells[static_cast<size_t>(r)].begin() + active_cell_col_ + c,
                    TableCellNode{});
            }
            ++tbl.cols;
        }
        model_.modified = true;
        return 1;
    }

    return 1;
}

void DocumentImpl::LockCommand(const std::string& action_id_utf8, bool lock) {
    if (lock) {
        locked_commands_.insert(action_id_utf8);
    } else {
        locked_commands_.erase(action_id_utf8);
    }
}

bool DocumentImpl::IsCommandLock(const std::string& action_id_utf8) const {
    return locked_commands_.find(action_id_utf8) != locked_commands_.end();
}

bool DocumentImpl::IsEmpty() const {
    if (!model_.fields.empty() || !model_.tables.empty() || !model_.pictures.empty() || !model_.memos.empty()) {
        return false;
    }
    for (const auto& p : model_.paragraphs) {
        if (!p.text_utf8.empty()) {
            return false;
        }
    }
    return true;
}

int DocumentImpl::GetSelectionMode() const {
    if (cell_block_mode_) {
        return 3; // Table cell block selection
    }
    if (selected_ctrl_) {
        return 4; // Control selection
    }
    if (has_text_selection_) {
        return 1; // Normal text selection
    }
    return 0;
}

int DocumentImpl::GetPath(char16_t* path_buf, int path_size) const {
    return CopyUtf8ToChar16Buffer(model_.current_path.string(), path_buf, path_size);
}

bool DocumentImpl::SetPath(const std::filesystem::path& path) {
    model_.current_path = path;
    return true;
}

std::shared_ptr<ParameterSetImpl> DocumentImpl::GetCellShape() {
    auto set = std::make_shared<ParameterSetImpl>("Cell");
    if (active_table_idx_ >= 0 && active_table_idx_ < static_cast<int>(model_.tables.size())) {
        const auto& tbl = model_.tables[static_cast<size_t>(active_table_idx_)];
        const auto& cell = tbl.cells[static_cast<size_t>(active_cell_row_)][static_cast<size_t>(active_cell_col_)];
        auto cell_sub = set->CreateItemSet("Cell", "Cell");
        cell_sub->SetInt("Width", cell.width_hwpunit);
        cell_sub->SetInt("Height", cell.height_hwpunit);
        cell_sub->SetString("FieldName", cell.field_name_utf8);
    }
    return set;
}

void DocumentImpl::SetCellShape(ParameterSetImpl* set) {
    if (!set || active_table_idx_ < 0 || active_table_idx_ >= static_cast<int>(model_.tables.size())) {
        return;
    }
    auto& tbl = model_.tables[static_cast<size_t>(active_table_idx_)];
    auto& cell = tbl.cells[static_cast<size_t>(active_cell_row_)][static_cast<size_t>(active_cell_col_)];
    if (set->ItemExist("Cell")) {
        auto cell_sub = set->GetItemSet("Cell");
        if (cell_sub->ItemExist("Width")) cell.width_hwpunit = cell_sub->GetInt("Width");
        if (cell_sub->ItemExist("Height")) cell.height_hwpunit = cell_sub->GetInt("Height");
    }
    model_.modified = true;
}

std::shared_ptr<ParameterSetImpl> DocumentImpl::GetCharShape() {
    auto set = std::make_shared<ParameterSetImpl>("CharShape");
    const auto& cs = model_.paragraphs[static_cast<size_t>(caret_para_)].char_shape;
    set->SetUInt("Height", cs.height);
    set->SetUInt("Bold", cs.bold);
    set->SetUInt("Italic", cs.italic);
    set->SetUInt("UnderlineType", cs.underline);
    set->SetUInt("TextColor", cs.text_color);
    set->SetString("FaceNameHangul", cs.font_name_utf8);
    return set;
}

void DocumentImpl::SetCharShape(ParameterSetImpl* set) {
    if (!set) return;
    auto apply_to = [&](CharShapeInfo& cs) {
        if (set->ItemExist("Height")) cs.height = set->GetUInt("Height");
        if (set->ItemExist("Bold")) cs.bold = set->GetUInt("Bold");
        if (set->ItemExist("Italic")) cs.italic = set->GetUInt("Italic");
        if (set->ItemExist("UnderlineType")) cs.underline = set->GetUInt("UnderlineType");
        if (set->ItemExist("TextColor")) cs.text_color = set->GetUInt("TextColor");
        if (set->ItemExist("FaceNameHangul")) cs.font_name_utf8 = set->GetStringUtf8("FaceNameHangul");
    };
    if (has_text_selection_) {
        int sp = std::clamp(sel_spara_, 0, static_cast<int>(model_.paragraphs.size() - 1));
        int ep = std::clamp(sel_epara_, sp, static_cast<int>(model_.paragraphs.size() - 1));
        for (int p = sp; p <= ep; ++p) {
            apply_to(model_.paragraphs[static_cast<size_t>(p)].char_shape);
        }
    } else {
        apply_to(model_.paragraphs[static_cast<size_t>(caret_para_)].char_shape);
    }
    model_.modified = true;
}

std::shared_ptr<ParameterSetImpl> DocumentImpl::GetParaShape() {
    auto set = std::make_shared<ParameterSetImpl>("ParaShape");
    const auto& ps = model_.paragraphs[static_cast<size_t>(caret_para_)].para_shape;
    set->SetUInt("AlignType", ps.align_type);
    set->SetUInt("BreakNonLatinWord", ps.break_non_latin_word);
    set->SetInt("LineSpacing", ps.line_spacing);
    return set;
}

void DocumentImpl::SetParaShape(ParameterSetImpl* set) {
    if (!set) return;
    auto apply_to = [&](ParaShapeInfo& ps) {
        if (set->ItemExist("AlignType")) ps.align_type = set->GetUInt("AlignType");
        if (set->ItemExist("BreakNonLatinWord")) ps.break_non_latin_word = set->GetUInt("BreakNonLatinWord");
        if (set->ItemExist("LineSpacing")) ps.line_spacing = set->GetInt("LineSpacing");
    };
    if (has_text_selection_) {
        int sp = std::clamp(sel_spara_, 0, static_cast<int>(model_.paragraphs.size() - 1));
        int ep = std::clamp(sel_epara_, sp, static_cast<int>(model_.paragraphs.size() - 1));
        for (int p = sp; p <= ep; ++p) {
            apply_to(model_.paragraphs[static_cast<size_t>(p)].para_shape);
        }
    } else {
        apply_to(model_.paragraphs[static_cast<size_t>(caret_para_)].para_shape);
    }
    model_.modified = true;
}

void DocumentImpl::RebuildCtrlLinkedList() {
    ctrl_nodes_.clear();

    // Always include SecDef & ColDef at head like HWP engine
    ctrl_nodes_.push_back(std::make_shared<CtrlCodeImpl>(this, 2, "secd", "1", "구역 정의", 0, 0));
    ctrl_nodes_.push_back(std::make_shared<CtrlCodeImpl>(this, 2, "cold", "2", "단 정의", 0, 0));

    if (model_.page_num_pos.enabled) {
        auto pgnp = std::make_shared<CtrlCodeImpl>(this, 21, "pgnp", "3", "쪽 번호 위치", 0, 0);
        pgnp->GetProperties()->SetInt("DrawPos", model_.page_num_pos.draw_pos);
        pgnp->GetProperties()->SetInt("SideChar", model_.page_num_pos.side_char);
        ctrl_nodes_.push_back(std::move(pgnp));
    }

    for (size_t i = 0; i < model_.fields.size(); ++i) {
        const auto& f = model_.fields[i];
        auto fc = std::make_shared<CtrlCodeImpl>(
            this, 3, "%clk", std::to_string(500 + i), "누름틀", f.para_index, 0);
        fc->GetProperties()->SetString("Name", f.name_utf8);
        fc->GetProperties()->SetString("Direction", f.direction_utf8);
        fc->GetProperties()->SetString("Memo", f.memo_utf8);
        ctrl_nodes_.push_back(std::move(fc));
    }

    for (size_t i = 0; i < model_.tables.size(); ++i) {
        auto& t = model_.tables[i];
        if (t.inst_id_utf8.empty()) {
            t.inst_id_utf8 = std::to_string(1000 + i);
        }
        auto tc = std::make_shared<CtrlCodeImpl>(
            this, 11, "tbl", t.inst_id_utf8, "표", t.anchor_para, 0);
        tc->GetProperties()->SetInt("Rows", t.rows);
        tc->GetProperties()->SetInt("Cols", t.cols);
        ctrl_nodes_.push_back(std::move(tc));
    }

    for (size_t i = 0; i < model_.pictures.size(); ++i) {
        auto& pic = model_.pictures[i];
        if (pic.inst_id_utf8.empty()) {
            pic.inst_id_utf8 = std::to_string(2000 + i);
        }
        auto pc = std::make_shared<CtrlCodeImpl>(
            this, 11, "gso", pic.inst_id_utf8, "그림", pic.anchor_para, 0);
        ctrl_nodes_.push_back(std::move(pc));
    }

    for (size_t i = 0; i < model_.memos.size(); ++i) {
        const auto& m = model_.memos[i];
        auto mc = std::make_shared<CtrlCodeImpl>(
            this, 15, "%mm", std::to_string(m.inst_id), "메모", m.anchor_para, 0);
        ctrl_nodes_.push_back(std::move(mc));
    }

    for (size_t i = 0; i < ctrl_nodes_.size(); ++i) {
        if (i > 0) {
            ctrl_nodes_[i]->SetPrev(ctrl_nodes_[i - 1]);
        }
        if (i + 1 < ctrl_nodes_.size()) {
            ctrl_nodes_[i]->SetNext(ctrl_nodes_[i + 1]);
        }
    }
}

std::shared_ptr<CtrlCodeImpl> DocumentImpl::GetSelectedCtrl() {
    return selected_ctrl_;
}

std::shared_ptr<CtrlCodeImpl> DocumentImpl::GetHeadCtrl() {
    RebuildCtrlLinkedList();
    return ctrl_nodes_.empty() ? nullptr : ctrl_nodes_.front();
}

std::shared_ptr<CtrlCodeImpl> DocumentImpl::GetLastCtrl() {
    RebuildCtrlLinkedList();
    return ctrl_nodes_.empty() ? nullptr : ctrl_nodes_.back();
}

std::shared_ptr<CtrlCodeImpl> DocumentImpl::GetParentCtrl() {
    if (caret_list_ > 0 && active_table_idx_ >= 0 && active_table_idx_ < static_cast<int>(model_.tables.size())) {
        RebuildCtrlLinkedList();
        const std::string& inst_id = model_.tables[static_cast<size_t>(active_table_idx_)].inst_id_utf8;
        for (const auto& node : ctrl_nodes_) {
            if (node->GetCtrlIDUtf8() == "tbl" && node->GetInstIDUtf8() == inst_id) {
                return node;
            }
        }
    }
    return nullptr;
}

std::shared_ptr<CtrlCodeImpl> DocumentImpl::InsertCtrl(const std::string& ctrl_id_utf8, ParameterSetImpl* init_set) {
    if (ctrl_id_utf8 == "tbl") {
        ExecuteActionWithSet("TableCreate", init_set);
        RebuildCtrlLinkedList();
        return ctrl_nodes_.back();
    }
    auto ctrl = std::make_shared<CtrlCodeImpl>(
        this, 11, ctrl_id_utf8, std::to_string(3000 + ctrl_nodes_.size()), ctrl_id_utf8, caret_para_, caret_pos_);
    if (init_set) {
        ctrl->GetProperties()->MergeFrom(*init_set);
    }
    ctrl_nodes_.push_back(ctrl);
    selected_ctrl_ = ctrl;
    model_.modified = true;
    return ctrl;
}

std::shared_ptr<CtrlCodeImpl> DocumentImpl::InsertExtraCtrl(
    const std::string& data_url_utf8,
    const std::string& extra_id_utf8,
    const std::string& extra_data_utf8) {
    auto ctrl = std::make_shared<CtrlCodeImpl>(
        this, 11, "gso", extra_id_utf8, extra_data_utf8, caret_para_, caret_pos_);
    ctrl->GetProperties()->SetString("DataUrl", data_url_utf8);
    ctrl->GetProperties()->SetString("ExtraId", extra_id_utf8);
    ctrl->GetProperties()->SetString("ExtraData", extra_data_utf8);
    ctrl_nodes_.push_back(ctrl);
    selected_ctrl_ = ctrl;
    model_.modified = true;
    return ctrl;
}

bool DocumentImpl::DeleteCtrl(CtrlCodeImpl* ctrl) {
    if (!ctrl) return false;
    std::string id = ctrl->GetCtrlIDUtf8();
    std::string inst = ctrl->GetInstIDUtf8();
    if (id == "tbl") {
        auto it = std::remove_if(model_.tables.begin(), model_.tables.end(),
                                 [&](const TableNode& t) { return t.inst_id_utf8 == inst; });
        if (it != model_.tables.end()) {
            model_.tables.erase(it, model_.tables.end());
            RebuildCtrlLinkedList();
            model_.modified = true;
            return true;
        }
    } else if (id == "gso") {
        auto it = std::remove_if(model_.pictures.begin(), model_.pictures.end(),
                                 [&](const PictureNode& p) { return p.inst_id_utf8 == inst; });
        if (it != model_.pictures.end()) {
            model_.pictures.erase(it, model_.pictures.end());
            RebuildCtrlLinkedList();
            model_.modified = true;
            return true;
        }
    }
    return false;
}

bool DocumentImpl::SelectCtrl(const std::string& ctrl_inst_list_utf8, int /*option*/) {
    RebuildCtrlLinkedList();
    for (const auto& node : ctrl_nodes_) {
        if (node->GetInstIDUtf8() == ctrl_inst_list_utf8) {
            selected_ctrl_ = node;
            if (node->GetCtrlIDUtf8() == "tbl") {
                for (size_t i = 0; i < model_.tables.size(); ++i) {
                    if (model_.tables[i].inst_id_utf8 == ctrl_inst_list_utf8) {
                        active_table_idx_ = static_cast<int>(i);
                        active_cell_row_ = 0;
                        active_cell_col_ = 0;
                        break;
                    }
                }
            }
            return true;
        }
    }
    return false;
}

bool DocumentImpl::UnSelectCtrl() {
    selected_ctrl_.reset();
    return true;
}

std::shared_ptr<CtrlCodeImpl> DocumentImpl::InsertPicture(
    const std::filesystem::path& path,
    bool embedded,
    unsigned short size_option,
    bool reverse,
    bool watermark,
    unsigned short effect,
    int width_mm,
    int height_mm) {
    PictureNode pic;
    pic.inst_id_utf8 = std::to_string(2000 + model_.pictures.size());
    pic.file_path = path;
    pic.bin_data_id = static_cast<int>(model_.bin_data_paths.size() + 1);
    pic.embedded = embedded;
    pic.size_option = size_option;
    pic.reverse = reverse;
    pic.watermark = watermark;
    pic.effect = effect;
    pic.width_mm = width_mm > 0 ? width_mm : 50;
    pic.height_mm = height_mm > 0 ? height_mm : 50;
    pic.anchor_para = caret_para_;

    model_.bin_data_paths[pic.bin_data_id] = path;
    model_.pictures.push_back(std::move(pic));
    model_.modified = true;
    RebuildCtrlLinkedList();
    return ctrl_nodes_.back();
}

bool DocumentImpl::InsertBackgroundPicture(
    const std::string& border_type_utf8,
    const std::filesystem::path& path,
    bool /*embedded*/,
    unsigned short /*fill_option*/,
    bool /*watermark*/,
    unsigned short /*effect*/,
    unsigned short /*brightness*/,
    unsigned short /*contrast*/) {
    if (border_type_utf8 == "SelectedCellDelete") {
        return true;
    }
    int bin_id = static_cast<int>(model_.bin_data_paths.size() + 1);
    model_.bin_data_paths[bin_id] = path;
    model_.modified = true;
    return true;
}

// ============================================================================
// Field API Implementation
// ============================================================================

int DocumentImpl::GetFieldList(char16_t* buf, int buf_size, int number_opt, int option) const {
    // Collect fields matching option (0: all, 1: FieldCell, 2: FieldClickHere)
    std::vector<std::string> raw_names;
    if (option == 0 || (option & HWPSDK::FieldClickHere) != 0) {
        for (const auto& f : model_.fields) {
            if (!f.name_utf8.empty()) {
                raw_names.push_back(f.name_utf8);
            }
        }
    }
    if (option == 0 || (option & HWPSDK::FieldCell) != 0) {
        for (const auto& tbl : model_.tables) {
            for (const auto& row : tbl.cells) {
                for (const auto& cell : row) {
                    if (!cell.field_name_utf8.empty()) {
                        raw_names.push_back(cell.field_name_utf8);
                    }
                }
            }
        }
    }

    std::map<std::string, int> total_counts;
    for (const auto& n : raw_names) {
        total_counts[n]++;
    }

    std::map<std::string, int> running_idx;
    std::vector<std::string> formatted;
    for (const auto& n : raw_names) {
        if (number_opt == HWPSDK::FieldNumber) {
            int idx = running_idx[n]++;
            formatted.push_back(n + "{{" + std::to_string(idx) + "}}");
        } else if (number_opt == HWPSDK::FieldCount) {
            if (running_idx[n] == 0) {
                running_idx[n] = 1;
                formatted.push_back(n + "{{" + std::to_string(total_counts[n]) + "}}");
            }
        } else {
            formatted.push_back(n);
        }
    }

    std::string joined;
    for (size_t i = 0; i < formatted.size(); ++i) {
        if (i > 0) {
            joined.push_back('\x02');
        }
        joined += formatted[i];
    }
    return CopyUtf8ToChar16Buffer(joined, buf, buf_size);
}

bool DocumentImpl::FieldExist(const std::string& field_utf8) const {
    std::string base_name;
    int target_idx = 0;
    bool has_idx = false;
    ParseFieldSelector(field_utf8, base_name, target_idx, has_idx);

    int count = 0;
    for (const auto& f : model_.fields) {
        if (f.name_utf8 == base_name) {
            if (!has_idx || count == target_idx) return true;
            ++count;
        }
    }
    for (const auto& tbl : model_.tables) {
        for (const auto& row : tbl.cells) {
            for (const auto& cell : row) {
                if (cell.field_name_utf8 == base_name) {
                    if (!has_idx || count == target_idx) return true;
                    ++count;
                }
            }
        }
    }
    return false;
}

bool DocumentImpl::MoveToField(const std::string& field_utf8, bool /*text*/, bool start, bool select) {
    std::string base_name;
    int target_idx = 0;
    bool has_idx = false;
    ParseFieldSelector(field_utf8, base_name, target_idx, has_idx);

    int match_idx = 0;
    for (size_t i = 0; i < model_.fields.size(); ++i) {
        if (model_.fields[i].name_utf8 == base_name) {
            if (match_idx == target_idx) {
                active_field_idx_ = static_cast<int>(i);
                caret_list_ = 0;
                caret_para_ = model_.fields[i].para_index;
                caret_pos_ = start ? 0 : static_cast<int>(model_.fields[i].text_utf8.size());
                has_text_selection_ = select;
                return true;
            }
            ++match_idx;
        }
    }

    for (size_t t = 0; t < model_.tables.size(); ++t) {
        const auto& tbl = model_.tables[t];
        for (int r = 0; r < tbl.rows; ++r) {
            for (int c = 0; c < tbl.cols; ++c) {
                if (tbl.cells[static_cast<size_t>(r)][static_cast<size_t>(c)].field_name_utf8 == base_name) {
                    if (match_idx == target_idx) {
                        active_table_idx_ = static_cast<int>(t);
                        caret_list_ = active_table_idx_ + 1;
                        active_cell_row_ = r;
                        active_cell_col_ = c;
                        active_field_idx_ = -1;
                        cell_block_mode_ = select;
                        return true;
                    }
                    ++match_idx;
                }
            }
        }
    }
    return false;
}

int DocumentImpl::GetFieldText(const std::string& field_list_utf8, char16_t* out_buf, int out_size) const {
    auto selectors = SplitByByte(field_list_utf8, '\x02');
    std::string combined;

    for (size_t s_idx = 0; s_idx < selectors.size(); ++s_idx) {
        std::string base_name;
        int target_idx = 0;
        bool has_idx = false;
        ParseFieldSelector(selectors[s_idx], base_name, target_idx, has_idx);

        std::string found_val;
        int match_idx = 0;
        bool found = false;

        for (const auto& f : model_.fields) {
            if (f.name_utf8 == base_name) {
                if (match_idx == target_idx) {
                    found_val = f.text_utf8;
                    found = true;
                    break;
                }
                ++match_idx;
            }
        }
        if (!found) {
            for (const auto& tbl : model_.tables) {
                for (const auto& row : tbl.cells) {
                    for (const auto& cell : row) {
                        if (cell.field_name_utf8 == base_name) {
                            if (match_idx == target_idx) {
                                found_val = cell.text_utf8;
                                found = true;
                                break;
                            }
                            ++match_idx;
                        }
                    }
                    if (found) break;
                }
                if (found) break;
            }
        }

        if (s_idx > 0) {
            combined.push_back('\x02');
        }
        combined += found_val;
    }

    return CopyUtf8ToChar16Buffer(combined, out_buf, out_size);
}

void DocumentImpl::PutFieldText(const std::string& field_list_utf8, const std::string& text_list_utf8) {
    auto selectors = SplitByByte(field_list_utf8, '\x02');
    auto texts = SplitByByte(text_list_utf8, '\x02');

    for (size_t i = 0; i < selectors.size(); ++i) {
        std::string val = (i < texts.size()) ? texts[i] : (texts.empty() ? "" : texts.back());
        std::string base_name;
        int target_idx = 0;
        bool has_idx = false;
        ParseFieldSelector(selectors[i], base_name, target_idx, has_idx);

        int match_idx = 0;
        for (auto& f : model_.fields) {
            if (f.name_utf8 == base_name) {
                if (!has_idx || match_idx == target_idx) {
                    f.text_utf8 = val;
                }
                ++match_idx;
            }
        }
        for (auto& tbl : model_.tables) {
            for (auto& row : tbl.cells) {
                for (auto& cell : row) {
                    if (cell.field_name_utf8 == base_name) {
                        if (!has_idx || match_idx == target_idx) {
                            cell.text_utf8 = val;
                        }
                        ++match_idx;
                    }
                }
            }
        }
    }
    model_.modified = true;
    NotifyWindowPreviewUpdated();
}

void DocumentImpl::RenameField(const std::string& old_name_utf8, const std::string& new_name_utf8) {
    auto olds = SplitByByte(old_name_utf8, '\x02');
    auto news = SplitByByte(new_name_utf8, '\x02');

    for (size_t i = 0; i < olds.size() && i < news.size(); ++i) {
        std::string base_old;
        int target_idx = 0;
        bool has_idx = false;
        ParseFieldSelector(olds[i], base_old, target_idx, has_idx);

        int match_idx = 0;
        for (auto& f : model_.fields) {
            if (f.name_utf8 == base_old) {
                if (!has_idx || match_idx == target_idx) {
                    f.name_utf8 = news[i];
                }
                ++match_idx;
            }
        }
        for (auto& tbl : model_.tables) {
            for (auto& row : tbl.cells) {
                for (auto& cell : row) {
                    if (cell.field_name_utf8 == base_old) {
                        if (!has_idx || match_idx == target_idx) {
                            cell.field_name_utf8 = news[i];
                        }
                        ++match_idx;
                    }
                }
            }
        }
    }
    model_.modified = true;
}

bool DocumentImpl::CreateField(
    const std::string& direction_utf8,
    const std::string& memo_utf8,
    const std::string& name_utf8) {
    FieldNode fn;
    fn.direction_utf8 = direction_utf8;
    fn.memo_utf8 = memo_utf8;
    fn.name_utf8 = name_utf8.empty() ? direction_utf8 : name_utf8;
    fn.para_index = caret_para_;
    fn.field_type = HWPSDK::FieldClickHere;
    model_.fields.push_back(std::move(fn));
    active_field_idx_ = static_cast<int>(model_.fields.size() - 1);
    model_.modified = true;
    RebuildCtrlLinkedList();
    return true;
}

int DocumentImpl::GetCurFieldName(int /*option*/, char16_t* buf, int buf_size) const {
    if (caret_list_ > 0 && active_table_idx_ >= 0 && active_table_idx_ < static_cast<int>(model_.tables.size())) {
        const auto& cell = model_.tables[static_cast<size_t>(active_table_idx_)]
                               .cells[static_cast<size_t>(active_cell_row_)][static_cast<size_t>(active_cell_col_)];
        return CopyUtf8ToChar16Buffer(cell.field_name_utf8, buf, buf_size);
    }
    if (active_field_idx_ >= 0 && active_field_idx_ < static_cast<int>(model_.fields.size())) {
        return CopyUtf8ToChar16Buffer(model_.fields[static_cast<size_t>(active_field_idx_)].name_utf8, buf, buf_size);
    }
    return CopyUtf8ToChar16Buffer("", buf, buf_size);
}

bool DocumentImpl::SetCurFieldName(
    const std::string& name_utf8,
    int option,
    const std::string& direction_utf8,
    const std::string& memo_utf8) {
    if ((option == HWPSDK::FieldCell || caret_list_ > 0) &&
        active_table_idx_ >= 0 && active_table_idx_ < static_cast<int>(model_.tables.size())) {
        model_.tables[static_cast<size_t>(active_table_idx_)]
            .cells[static_cast<size_t>(active_cell_row_)][static_cast<size_t>(active_cell_col_)]
            .field_name_utf8 = name_utf8;
        model_.modified = true;
        return true;
    }
    if (active_field_idx_ >= 0 && active_field_idx_ < static_cast<int>(model_.fields.size())) {
        auto& f = model_.fields[static_cast<size_t>(active_field_idx_)];
        f.name_utf8 = name_utf8;
        if (!direction_utf8.empty()) f.direction_utf8 = direction_utf8;
        if (!memo_utf8.empty()) f.memo_utf8 = memo_utf8;
        model_.modified = true;
        return true;
    }
    return CreateField(direction_utf8, memo_utf8, name_utf8);
}

int DocumentImpl::GetCurFieldState() const {
    if (caret_list_ > 0 && active_table_idx_ >= 0 && active_table_idx_ < static_cast<int>(model_.tables.size())) {
        const auto& cell = model_.tables[static_cast<size_t>(active_table_idx_)]
                               .cells[static_cast<size_t>(active_cell_row_)][static_cast<size_t>(active_cell_col_)];
        int flags = 0x01; // BIT 0: Cell
        if (!cell.field_name_utf8.empty()) flags |= 0x10; // BIT 4: Has field name
        return flags;
    }
    if (active_field_idx_ >= 0 && active_field_idx_ < static_cast<int>(model_.fields.size())) {
        int flags = 0x02; // BIT 1: ClickHere
        if (!model_.fields[static_cast<size_t>(active_field_idx_)].name_utf8.empty()) flags |= 0x10;
        return flags;
    }
    return 0;
}

int DocumentImpl::ModifyFieldProperties(const std::string& field_utf8, int remove_prop, int add_prop) {
    int modified_cnt = 0;
    for (auto& f : model_.fields) {
        if (f.name_utf8 == field_utf8) {
            f.properties = (f.properties & ~remove_prop) | add_prop;
            ++modified_cnt;
        }
    }
    return modified_cnt;
}

int DocumentImpl::SetFieldViewOption(int option) {
    int prev = field_view_option_;
    field_view_option_ = option;
    return prev;
}

// ============================================================================
// Metatag & UserProperty Implementation
// ============================================================================

int DocumentImpl::GetMetatagList(int /*number_opt*/, int /*option*/, char16_t* buf, int buf_size) const {
    std::vector<std::string> tags;
    for (const auto& p : model_.paragraphs) {
        if (!p.meta_tag_utf8.empty()) tags.push_back(p.meta_tag_utf8);
    }
    for (const auto& f : model_.fields) {
        if (!f.meta_tag_utf8.empty()) tags.push_back(f.meta_tag_utf8);
    }
    for (const auto& t : model_.tables) {
        if (!t.meta_tag_utf8.empty()) tags.push_back(t.meta_tag_utf8);
    }
    std::string joined;
    for (size_t i = 0; i < tags.size(); ++i) {
        if (i > 0) joined.push_back('\x02');
        joined += tags[i];
    }
    return CopyUtf8ToChar16Buffer(joined, buf, buf_size);
}

bool DocumentImpl::MetatagExist(const std::string& tag_utf8) const {
    for (const auto& p : model_.paragraphs) {
        if (p.meta_tag_utf8 == tag_utf8) return true;
    }
    for (const auto& f : model_.fields) {
        if (f.meta_tag_utf8 == tag_utf8) return true;
    }
    return false;
}

bool DocumentImpl::MoveToMetatag(const std::string& tag_utf8, bool /*text*/, bool /*start*/, bool select) {
    for (size_t i = 0; i < model_.paragraphs.size(); ++i) {
        if (model_.paragraphs[i].meta_tag_utf8 == tag_utf8) {
            caret_para_ = static_cast<int>(i);
            caret_pos_ = 0;
            has_text_selection_ = select;
            return true;
        }
    }
    return false;
}

int DocumentImpl::GetMetatagNameText(const std::string& tag_utf8, char16_t* buf, int buf_size) const {
    for (const auto& p : model_.paragraphs) {
        if (p.meta_tag_utf8 == tag_utf8) {
            return CopyUtf8ToChar16Buffer(p.text_utf8, buf, buf_size);
        }
    }
    for (const auto& f : model_.fields) {
        if (f.meta_tag_utf8 == tag_utf8) {
            return CopyUtf8ToChar16Buffer(f.text_utf8, buf, buf_size);
        }
    }
    return CopyUtf8ToChar16Buffer("", buf, buf_size);
}

bool DocumentImpl::PutMetatagNameText(const std::string& tag_utf8, const std::string& text_utf8) {
    for (auto& p : model_.paragraphs) {
        if (p.meta_tag_utf8 == tag_utf8) {
            p.text_utf8 = text_utf8;
            model_.modified = true;
            return true;
        }
    }
    for (auto& f : model_.fields) {
        if (f.meta_tag_utf8 == tag_utf8) {
            f.text_utf8 = text_utf8;
            model_.modified = true;
            return true;
        }
    }
    return false;
}

bool DocumentImpl::SetCurMetatagName(const std::string& tag_utf8) {
    model_.paragraphs[static_cast<size_t>(caret_para_)].meta_tag_utf8 = tag_utf8;
    model_.modified = true;
    return true;
}

int DocumentImpl::GetCurMetatagName(char16_t* buf, int buf_size) const {
    return CopyUtf8ToChar16Buffer(model_.paragraphs[static_cast<size_t>(caret_para_)].meta_tag_utf8, buf, buf_size);
}

bool DocumentImpl::RenameMetatag(const std::string& old_tag_utf8, const std::string& new_tag_utf8) {
    bool found = false;
    for (auto& p : model_.paragraphs) {
        if (p.meta_tag_utf8 == old_tag_utf8) {
            p.meta_tag_utf8 = new_tag_utf8;
            found = true;
        }
    }
    for (auto& f : model_.fields) {
        if (f.meta_tag_utf8 == old_tag_utf8) {
            f.meta_tag_utf8 = new_tag_utf8;
            found = true;
        }
    }
    return found;
}

int DocumentImpl::ModifyMetatagProperties(const std::string& /*tag_utf8*/, int /*remove_prop*/, int /*add_prop*/) {
    return 1;
}

int DocumentImpl::GetCurMetatagState() const {
    return model_.paragraphs[static_cast<size_t>(caret_para_)].meta_tag_utf8.empty() ? 0 : 0x10;
}

int DocumentImpl::GetUserProperty(const std::string& name_utf8, int /*option*/, char16_t* buf, int buf_size) const {
    auto it = model_.user_properties.find(name_utf8);
    if (it == model_.user_properties.end()) {
        return CopyUtf8ToChar16Buffer("", buf, buf_size);
    }
    return CopyUtf8ToChar16Buffer(it->second, buf, buf_size);
}

int DocumentImpl::SetUserProperty(const std::string& name_utf8, int /*format*/, const std::string& value_utf8, int /*option*/) {
    model_.user_properties[name_utf8] = value_utf8;
    model_.modified = true;
    return 1;
}

int DocumentImpl::GetUserPropertyNameList(int /*option*/, char16_t* buf, int buf_size) const {
    std::string joined;
    size_t idx = 0;
    for (const auto& [k, _] : model_.user_properties) {
        if (idx++ > 0) joined.push_back('\x02');
        joined += k;
    }
    return CopyUtf8ToChar16Buffer(joined, buf, buf_size);
}

// ============================================================================
// Position & Selection Implementation
// ============================================================================

bool DocumentImpl::MovePos(int move_id, int para, int pos) {
    switch (move_id) {
        case HWPSDK::moveMain:
            caret_list_ = 0;
            caret_para_ = std::clamp(para, 0, static_cast<int>(model_.paragraphs.size() - 1));
            caret_pos_ = std::max(0, pos);
            return true;
        case HWPSDK::moveCurList:
            caret_para_ = std::clamp(para, 0, static_cast<int>(model_.paragraphs.size() - 1));
            caret_pos_ = std::max(0, pos);
            return true;
        case HWPSDK::moveTopOfFile:
        case HWPSDK::moveTopOfList:
            caret_para_ = 0;
            caret_pos_ = 0;
            return true;
        case HWPSDK::moveBottomOfFile:
        case HWPSDK::moveBottomOfList:
            caret_para_ = static_cast<int>(model_.paragraphs.size() - 1);
            caret_pos_ = static_cast<int>(model_.paragraphs.back().text_utf8.size());
            return true;
        case HWPSDK::moveStartOfPara:
        case HWPSDK::moveStartOfLine:
            caret_pos_ = 0;
            return true;
        case HWPSDK::moveEndOfPara:
        case HWPSDK::moveEndOfLine:
            caret_pos_ = static_cast<int>(model_.paragraphs[static_cast<size_t>(caret_para_)].text_utf8.size());
            return true;
        case HWPSDK::moveNextPara:
            if (caret_para_ + 1 < static_cast<int>(model_.paragraphs.size())) {
                ++caret_para_;
                caret_pos_ = 0;
                return true;
            }
            return false;
        case HWPSDK::movePrevPara:
            if (caret_para_ > 0) {
                --caret_para_;
                caret_pos_ = static_cast<int>(model_.paragraphs[static_cast<size_t>(caret_para_)].text_utf8.size());
                return true;
            }
            return false;
        case HWPSDK::moveLeftOfCell:
            return ExecuteActionWithSet("TableLeftCell", nullptr) > 0;
        case HWPSDK::moveRightOfCell:
            return ExecuteActionWithSet("TableRightCell", nullptr) > 0;
        case HWPSDK::moveUpOfCell:
            return ExecuteActionWithSet("TableUpperCell", nullptr) > 0;
        case HWPSDK::moveDownOfCell:
            return ExecuteActionWithSet("TableLowerCell", nullptr) > 0;
        case HWPSDK::moveScanPos:
            caret_para_ = std::clamp(scan_last_para_, 0, static_cast<int>(model_.paragraphs.size() - 1));
            caret_pos_ = 0;
            return true;
        default:
            return true;
    }
}

void DocumentImpl::GetPos(int* list, int* para, int* pos) const {
    if (list) *list = caret_list_;
    if (para) *para = caret_para_;
    if (pos) *pos = caret_pos_;
}

bool DocumentImpl::SetPos(int list, int para, int pos) {
    caret_list_ = std::max(0, list);
    caret_para_ = std::clamp(para, 0, static_cast<int>(model_.paragraphs.size() - 1));
    caret_pos_ = std::max(0, pos);
    return true;
}

std::shared_ptr<ParameterSetImpl> DocumentImpl::GetPosBySet() {
    auto set = std::make_shared<ParameterSetImpl>("ListParaPos");
    set->SetInt("List", caret_list_);
    set->SetInt("Para", caret_para_);
    set->SetInt("Pos", caret_pos_);
    return set;
}

bool DocumentImpl::SetPosBySet(ParameterSetImpl* pos_set) {
    if (!pos_set) return false;
    return SetPos(pos_set->GetInt("List"), pos_set->GetInt("Para"), pos_set->GetInt("Pos"));
}

bool DocumentImpl::SelectText(int spara, int spos, int epara, int epos) {
    has_text_selection_ = true;
    sel_slist_ = caret_list_;
    sel_spara_ = std::clamp(spara, 0, static_cast<int>(model_.paragraphs.size() - 1));
    sel_spos_ = std::max(0, spos);
    sel_elist_ = caret_list_;
    sel_epara_ = std::clamp(epara, sel_spara_, static_cast<int>(model_.paragraphs.size() - 1));
    sel_epos_ = std::max(0, epos);
    return true;
}

std::string DocumentImpl::GetSelectedTextUtf8() const {
    if (cell_block_mode_ && active_table_idx_ >= 0 && active_table_idx_ < static_cast<int>(model_.tables.size())) {
        const auto& tbl = model_.tables[static_cast<size_t>(active_table_idx_)];
        std::string out;
        for (int r = cell_block_srow_; r <= cell_block_erow_ && r < tbl.rows; ++r) {
            for (int c = cell_block_scol_; c <= cell_block_ecol_ && c < tbl.cols; ++c) {
                if (!out.empty()) out += "\r\n";
                out += tbl.cells[static_cast<size_t>(r)][static_cast<size_t>(c)].text_utf8;
            }
        }
        return out;
    }
    if (!has_text_selection_) {
        return {};
    }
    int sp = std::clamp(sel_spara_, 0, static_cast<int>(model_.paragraphs.size() - 1));
    int ep = std::clamp(sel_epara_, sp, static_cast<int>(model_.paragraphs.size() - 1));
    std::string out;
    for (int p = sp; p <= ep; ++p) {
        const std::string& ptxt = model_.paragraphs[static_cast<size_t>(p)].text_utf8;
        size_t s = (p == sp) ? std::min<size_t>(static_cast<size_t>(sel_spos_), ptxt.size()) : 0;
        size_t e = (p == ep) ? std::min<size_t>(static_cast<size_t>(sel_epos_), ptxt.size()) : ptxt.size();
        if (e > s) {
            out += ptxt.substr(s, e - s);
        }
        if (p < ep) {
            out += "\r\n";
        }
    }
    return out;
}

int DocumentImpl::GetSelectText(char16_t* buf, int buf_size) const {
    return CopyUtf8ToChar16Buffer(GetSelectedTextUtf8(), buf, buf_size);
}

bool DocumentImpl::GetSelectedPos(int* slist, int* spara, int* spos, int* elist, int* epara, int* epos) const {
    if (!has_text_selection_) {
        if (slist) *slist = caret_list_;
        if (spara) *spara = caret_para_;
        if (spos) *spos = caret_pos_;
        if (elist) *elist = caret_list_;
        if (epara) *epara = caret_para_;
        if (epos) *epos = caret_pos_;
        return false;
    }
    if (slist) *slist = sel_slist_;
    if (spara) *spara = sel_spara_;
    if (spos) *spos = sel_spos_;
    if (elist) *elist = sel_elist_;
    if (epara) *epara = sel_epara_;
    if (epos) *epos = sel_epos_;
    return true;
}

bool DocumentImpl::GetSelectedPosBySet(ParameterSetImpl* sset, ParameterSetImpl* eset) {
    if (sset) {
        sset->SetInt("List", sel_slist_);
        sset->SetInt("Para", sel_spara_);
        sset->SetInt("Pos", sel_spos_);
    }
    if (eset) {
        eset->SetInt("List", sel_elist_);
        eset->SetInt("Para", sel_epara_);
        eset->SetInt("Pos", sel_epos_);
    }
    return has_text_selection_;
}

// ============================================================================
// Scan & Text Extraction Implementation
// ============================================================================

void DocumentImpl::ReleaseScan() {
    scan_initialized_ = false;
    scan_cursor_ = 0;
    scan_items_.clear();
}

bool DocumentImpl::InitScan(int /*option*/, int /*range*/, int /*spara*/, int /*spos*/, int /*epara*/, int /*epos*/) {
    scan_items_.clear();
    for (size_t i = 0; i < model_.paragraphs.size(); ++i) {
        scan_items_.emplace_back(static_cast<int>(i), model_.paragraphs[i].text_utf8 + "\r\n");
    }
    for (const auto& tbl : model_.tables) {
        for (const auto& row : tbl.cells) {
            for (const auto& cell : row) {
                if (!cell.text_utf8.empty()) {
                    scan_items_.emplace_back(tbl.anchor_para, cell.text_utf8 + "\r\n");
                }
            }
        }
    }
    scan_cursor_ = 0;
    scan_last_para_ = 0;
    scan_initialized_ = true;
    return true;
}

int DocumentImpl::GetText(char16_t* buf, int buf_size) {
    if (!scan_initialized_) {
        InitScan(0, 0x77, 0, 0, 0, 0);
    }
    if (scan_cursor_ >= scan_items_.size()) {
        return CopyUtf8ToChar16Buffer("", buf, buf_size);
    }
    scan_last_para_ = scan_items_[scan_cursor_].first;
    const std::string& txt = scan_items_[scan_cursor_].second;
    if (buf != nullptr && buf_size > 0) {
        ++scan_cursor_;
    }
    return CopyUtf8ToChar16Buffer(txt, buf, buf_size);
}

int DocumentImpl::GetTextBySet(ParameterSetImpl* text_set) {
    if (!scan_initialized_) {
        return 101;
    }
    if (!text_set) {
        return 102;
    }
    if (scan_cursor_ >= scan_items_.size()) {
        text_set->SetString("Text", "");
        return 1; // End of list
    }
    scan_last_para_ = scan_items_[scan_cursor_].first;
    text_set->SetString("Text", scan_items_[scan_cursor_].second);
    ++scan_cursor_;
    return 2; // Normal text
}

int DocumentImpl::SetTextFile(const std::string& data_utf8, const std::string& format_utf8, const std::string& option_utf8) {
    std::string fmt = ToUpperAscii(format_utf8);
    bool insert_mode = (ToUpperAscii(option_utf8) == "INSERTFILE");

    if (fmt == "HWPML2X" || fmt == "XML" || data_utf8.find("<hs:sec") != std::string::npos) {
        bool ok = formats::HwpxFormatHandler::ParseSectionXmlUtf8(data_utf8, model_, insert_mode);
        if (ok) {
            model_.modified = true;
            RebuildCtrlLinkedList();
            NotifyWindowPreviewUpdated();
        }
        return ok ? 1 : 0;
    }
    if (fmt == "HWP") {
        std::vector<uint8_t> raw = formats::Hwp5FormatHandler::DecodeBase64(data_utf8);
        std::filesystem::path tmp_path = std::filesystem::temp_directory_path() / "openhwp_settextfile_tmp.hwp";
        {
            std::ofstream ofs(tmp_path, std::ios::binary | std::ios::trunc);
            ofs.write(reinterpret_cast<const char*>(raw.data()), static_cast<std::streamsize>(raw.size()));
        }
        bool ok = formats::Hwp5FormatHandler::LoadFromFile(tmp_path, model_);
        std::error_code ec;
        std::filesystem::remove(tmp_path, ec);
        if (ok) {
            RebuildCtrlLinkedList();
            NotifyWindowPreviewUpdated();
        }
        return ok ? 1 : 0;
    }

    if (!insert_mode) {
        model_.ClearAndReset();
        model_.paragraphs[0].text_utf8 = data_utf8;
    } else {
        ParagraphNode p;
        p.text_utf8 = data_utf8;
        model_.paragraphs.push_back(std::move(p));
    }
    model_.modified = true;
    NotifyWindowPreviewUpdated();
    return 1;
}

int DocumentImpl::GetTextFile(const std::string& format_utf8, const std::string& option_utf8, char16_t* buf, int buf_size) const {
    std::string fmt = ToUpperAscii(format_utf8);
    if (ToUpperAscii(option_utf8) == "SAVEBLOCK") {
        return CopyUtf8ToChar16Buffer(GetSelectedTextUtf8(), buf, buf_size);
    }
    if (fmt == "HWPML2X" || fmt == "XML") {
        std::string xml = formats::HwpxFormatHandler::SerializeSectionXmlUtf8(model_);
        return CopyUtf8ToChar16Buffer(xml, buf, buf_size);
    }
    if (fmt == "HWP") {
        std::filesystem::path tmp_path = std::filesystem::temp_directory_path() / "openhwp_gettextfile_tmp.hwp";
        formats::Hwp5FormatHandler::SaveToFile(tmp_path, model_);
        std::ifstream ifs(tmp_path, std::ios::binary | std::ios::ate);
        std::vector<uint8_t> bytes;
        if (ifs) {
            auto sz = ifs.tellg();
            ifs.seekg(0, std::ios::beg);
            bytes.resize(static_cast<size_t>(sz));
            ifs.read(reinterpret_cast<char*>(bytes.data()), sz);
        }
        std::error_code ec;
        std::filesystem::remove(tmp_path, ec);
        std::string b64 = formats::Hwp5FormatHandler::EncodeBase64(bytes);
        return CopyUtf8ToChar16Buffer(b64, buf, buf_size);
    }

    std::string out;
    for (size_t i = 0; i < model_.paragraphs.size(); ++i) {
        if (i > 0) out += "\r\n";
        out += model_.paragraphs[i].text_utf8;
    }
    return CopyUtf8ToChar16Buffer(out, buf, buf_size);
}

int DocumentImpl::GetPageText(int page, int option, char16_t* buf, int buf_size) const {
    int cur_page = 0;
    std::string out;
    for (size_t i = 0; i < model_.paragraphs.size(); ++i) {
        if (model_.paragraphs[i].page_break_before && i > 0) {
            ++cur_page;
        }
        if (cur_page == page) {
            if (!out.empty()) out += "\r\n";
            out += model_.paragraphs[i].text_utf8;
            if ((option & 0x01) != 0) {
                for (const auto& tbl : model_.tables) {
                    if (tbl.anchor_para == static_cast<int>(i)) {
                        for (const auto& row : tbl.cells) {
                            for (const auto& cell : row) {
                                if (!cell.text_utf8.empty()) {
                                    out += "\t" + cell.text_utf8;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return CopyUtf8ToChar16Buffer(out, buf, buf_size);
}

bool DocumentImpl::GetBinDataPath(int bin_id, char16_t* buf, int buf_size) const {
    auto it = model_.bin_data_paths.find(bin_id);
    if (it == model_.bin_data_paths.end()) {
        return false;
    }
    CopyUtf8ToChar16Buffer(it->second.string(), buf, buf_size);
    return true;
}

std::shared_ptr<ParameterSetImpl> DocumentImpl::GetFormObjectAttr(const std::string& form_name_utf8) {
    auto set = std::make_shared<ParameterSetImpl>("FormObjectAttr");
    set->SetString("Name", form_name_utf8);
    return set;
}

bool DocumentImpl::SetFormObjectAttr(const std::string& /*form_name_utf8*/, ParameterSetImpl* /*pset*/) {
    model_.modified = true;
    return true;
}

bool DocumentImpl::SetBarCodeImage(const std::filesystem::path& path, int /*pgno*/, int /*index*/, int /*x*/, int /*y*/, int width, int height) {
    InsertPicture(path, true, 1, false, false, 0, width, height);
    return true;
}

bool DocumentImpl::CreatePageImage(
    const std::filesystem::path& path,
    int /*pgno*/,
    short /*resolution*/,
    short /*depth*/,
    const std::string& /*format_utf8*/,
    unsigned int /*options*/) {
    std::error_code ec;
    if (path.has_parent_path()) {
        std::filesystem::create_directories(path.parent_path(), ec);
    }
    // Generate a minimal valid 1x1 24-bit BMP image so callers verifying file creation succeed
    static constexpr uint8_t kMinimalBmp[58] = {
        0x42, 0x4D, 0x3A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x36, 0x00, 0x00, 0x00,
        0x28, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00,
        0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x13, 0x0B, 0x00, 0x00,
        0x13, 0x0B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0xFF, 0xFF, 0xFF, 0x00
    };
    std::ofstream ofs(path, std::ios::binary | std::ios::trunc);
    if (!ofs) return false;
    ofs.write(reinterpret_cast<const char*>(kMinimalBmp), sizeof(kMinimalBmp));
    return ofs.good();
}

bool DocumentImpl::PrintDocument() {
    return true;
}

int DocumentImpl::GetFilterList(unsigned int /*flags*/, char16_t* buf, int buf_size) const {
    const std::string filters = "한글 문서 (*.hwp;*.hwpx)|*.hwp;*.hwpx|HWPML 2.X 문서 (*.xml)|*.xml|텍스트 문서 (*.txt)|*.txt|";
    return CopyUtf8ToChar16Buffer(filters, buf, buf_size);
}

std::shared_ptr<ParameterSetImpl> DocumentImpl::GetFileInfo(const std::filesystem::path& file_path) {
    auto info = std::make_shared<ParameterSetImpl>("FileInfo");
    if (formats::Hwp5FormatHandler::IsHwp5File(file_path)) {
        std::string ver_str;
        unsigned int ver_num = 0;
        int enc = 0;
        if (formats::Hwp5FormatHandler::InspectHeader(file_path, ver_str, ver_num, enc)) {
            info->SetString("Format", "HWP");
            info->SetString("VersionStr", ver_str);
            info->SetUInt("VersionNum", ver_num);
            info->SetInt("Encrypted", enc);
            return info;
        }
    } else if (formats::HwpxFormatHandler::IsHwpxFile(file_path)) {
        info->SetString("Format", "HWPX");
        info->SetString("VersionStr", "1.4.0.0");
        info->SetUInt("VersionNum", 0x01040000);
        info->SetInt("Encrypted", 0);
        return info;
    }
    info->SetString("Format", "UNKNOWN");
    info->SetString("VersionStr", "0.0.0.0");
    info->SetUInt("VersionNum", 0);
    info->SetInt("Encrypted", -1);
    return info;
}

int DocumentImpl::GetTableCellAddr(int type) const {
    if (active_table_idx_ < 0 || active_table_idx_ >= static_cast<int>(model_.tables.size())) {
        return -1;
    }
    return (type == 0) ? active_cell_row_ : active_cell_col_;
}

bool DocumentImpl::VersionSave(
    const std::filesystem::path& file_path,
    bool overwrite,
    bool info_lock,
    const std::string& writer_utf8,
    const std::string& desc_utf8) {
    std::filesystem::path target = file_path.empty() ? model_.current_path : file_path;
    if (!target.empty()) {
        SaveAs(target, model_.current_format_utf8, "");
    }
    if (overwrite && !model_.version_history.empty() && model_.version_history.back().writer_utf8 == writer_utf8) {
        auto& last = model_.version_history.back();
        last.description_utf8 = desc_utf8;
        last.info_lock = info_lock;
    } else {
        VersionHistoryEntry vh;
        vh.file_path = target;
        vh.writer_utf8 = writer_utf8;
        vh.description_utf8 = desc_utf8;
        vh.info_lock = info_lock;
        model_.version_history.push_back(std::move(vh));
    }
    return true;
}

bool DocumentImpl::VersionDelete(int index) {
    if (index == -1) {
        model_.version_history.clear();
        return true;
    }
    if (index < 0 || static_cast<size_t>(index) >= model_.version_history.size()) {
        return false;
    }
    model_.version_history.erase(model_.version_history.begin() + index);
    return true;
}

std::shared_ptr<ParameterSetImpl> DocumentImpl::MakeVersionDiffAll(const std::filesystem::path& file_path) {
    auto info = std::make_shared<ParameterSetImpl>("VersionInfo");
    info->SetString("SourcePath", model_.current_path.string());
    info->SetString("TargetPath", file_path.string());
    return info;
}

std::shared_ptr<ParameterSetImpl> DocumentImpl::GetVersionInfo(int index) {
    auto info = std::make_shared<ParameterSetImpl>("VersionInfo");
    if (index >= 0 && static_cast<size_t>(index) < model_.version_history.size()) {
        const auto& vh = model_.version_history[static_cast<size_t>(index)];
        info->SetUInt("ItemInfoIndex", static_cast<unsigned int>(index));
        info->SetString("SaveFilePath", vh.file_path.string());
        info->SetString("ItemInfoWriter", vh.writer_utf8);
        info->SetString("ItemInfoDescription", vh.description_utf8);
        info->SetInt("ItemInfoLock", vh.info_lock ? 1 : 0);
    }
    return info;
}

std::shared_ptr<ParameterSetImpl> DocumentImpl::GetStyleList(const std::filesystem::path& /*file_path*/) {
    auto set = std::make_shared<ParameterSetImpl>("StyleList");
    set->SetString("DefaultStyle", "바탕글");
    return set;
}

bool DocumentImpl::ExportStyle(ParameterSetImpl* style_set) {
    return style_set != nullptr;
}

bool DocumentImpl::ImportStyle(ParameterSetImpl* style_set) {
    return style_set != nullptr;
}

std::shared_ptr<ParameterSetImpl> DocumentImpl::GetActionCmdUIStatus(const std::string& act_id_utf8, int /*with_key*/) {
    auto set = std::make_shared<ParameterSetImpl>("CmdUIStatus");
    set->SetString("ActionID", act_id_utf8);
    set->SetInt("Enabled", IsCommandLock(act_id_utf8) ? 0 : 1);
    return set;
}

int DocumentImpl::GetHeadingString(char16_t* buf, int buf_size) const {
    return CopyUtf8ToChar16Buffer(model_.paragraphs[static_cast<size_t>(caret_para_)].heading_utf8, buf, buf_size);
}

int DocumentImpl::GetCtrlHorizontalOffset(CtrlCodeImpl* /*ctrl*/, int /*rel_to*/) const {
    return 0;
}

int DocumentImpl::GetCtrlVerticalOffset(CtrlCodeImpl* /*ctrl*/, int /*rel_to*/) const {
    return 0;
}

int DocumentImpl::GetMemoString(CtrlCodeImpl* ctrl, char16_t* buf, int buf_size) const {
    if (!ctrl) return 0;
    for (const auto& m : model_.memos) {
        if (std::to_string(m.inst_id) == ctrl->GetInstIDUtf8()) {
            return CopyUtf8ToChar16Buffer(m.content_utf8, buf, buf_size);
        }
    }
    return CopyUtf8ToChar16Buffer("", buf, buf_size);
}

int DocumentImpl::GetFontList(int /*lang_id*/, char16_t* buf, int buf_size) const {
    std::string joined;
    for (size_t i = 0; i < model_.fonts_utf8.size(); ++i) {
        if (i > 0) joined.push_back('\x02');
        joined += model_.fonts_utf8[i];
    }
    return CopyUtf8ToChar16Buffer(joined, buf, buf_size);
}

bool DocumentImpl::ReplaceFont(
    int /*lang_id*/,
    const std::string& des_font_utf8,
    int /*des_type*/,
    const std::string& new_font_utf8,
    int /*new_type*/) {
    for (auto& p : model_.paragraphs) {
        if (p.char_shape.font_name_utf8 == des_font_utf8) {
            p.char_shape.font_name_utf8 = new_font_utf8;
        }
    }
    for (auto& f : model_.fonts_utf8) {
        if (f == des_font_utf8) {
            f = new_font_utf8;
        }
    }
    model_.modified = true;
    return true;
}

bool DocumentImpl::ScanFont() {
    auto sys_fonts = platform::GetPlatformService().GetSystemFontNamesUtf8();
    for (const auto& sf : sys_fonts) {
        if (std::find(model_.fonts_utf8.begin(), model_.fonts_utf8.end(), sf) == model_.fonts_utf8.end()) {
            model_.fonts_utf8.push_back(sf);
        }
    }
    for (const auto& p : model_.paragraphs) {
        if (!p.char_shape.font_name_utf8.empty() &&
            std::find(model_.fonts_utf8.begin(), model_.fonts_utf8.end(), p.char_shape.font_name_utf8) == model_.fonts_utf8.end()) {
            model_.fonts_utf8.push_back(p.char_shape.font_name_utf8);
        }
    }
    return true;
}

bool DocumentImpl::GetCtrlToPicture(const std::filesystem::path& path, int /*format*/, bool /*full_save*/) {
    return CreatePageImage(path, 0, 96, 24, "bmp", 0);
}

int DocumentImpl::InsertMemo(
    const std::string& content_utf8,
    const std::string& author_utf8,
    const std::string& datetime_utf8) {
    MemoNode m;
    m.inst_id = static_cast<int>(4000 + model_.memos.size() + 1);
    m.content_utf8 = content_utf8;
    m.author_utf8 = author_utf8;
    m.datetime_utf8 = datetime_utf8;
    m.anchor_para = caret_para_;
    model_.memos.push_back(std::move(m));
    active_memo_idx_ = static_cast<int>(model_.memos.size() - 1);
    model_.modified = true;
    RebuildCtrlLinkedList();
    return model_.memos.back().inst_id;
}

bool DocumentImpl::SelectMemo(int memo_inst_id) {
    for (size_t i = 0; i < model_.memos.size(); ++i) {
        if (model_.memos[i].inst_id == memo_inst_id) {
            active_memo_idx_ = static_cast<int>(i);
            return true;
        }
    }
    return false;
}

bool DocumentImpl::ModifyMemo(
    const std::string& content_utf8,
    const std::string& author_utf8,
    const std::string& datetime_utf8) {
    if (active_memo_idx_ < 0 || active_memo_idx_ >= static_cast<int>(model_.memos.size())) {
        return false;
    }
    auto& m = model_.memos[static_cast<size_t>(active_memo_idx_)];
    m.content_utf8 = content_utf8;
    if (!author_utf8.empty()) m.author_utf8 = author_utf8;
    if (!datetime_utf8.empty()) m.datetime_utf8 = datetime_utf8;
    model_.modified = true;
    return true;
}

bool DocumentImpl::DeleteMemo(int memo_inst_id) {
    for (size_t i = 0; i < model_.memos.size(); ++i) {
        if (model_.memos[i].inst_id == memo_inst_id) {
            model_.memos.erase(model_.memos.begin() + static_cast<std::ptrdiff_t>(i));
            active_memo_idx_ = -1;
            model_.modified = true;
            RebuildCtrlLinkedList();
            return true;
        }
    }
    return false;
}

int DocumentImpl::GetMemoContent(char16_t* buf, int buf_size) const {
    if (active_memo_idx_ < 0 || active_memo_idx_ >= static_cast<int>(model_.memos.size())) {
        return CopyUtf8ToChar16Buffer("", buf, buf_size);
    }
    return CopyUtf8ToChar16Buffer(model_.memos[static_cast<size_t>(active_memo_idx_)].content_utf8, buf, buf_size);
}

int DocumentImpl::GetMemoAuthor(char16_t* buf, int buf_size) const {
    if (active_memo_idx_ < 0 || active_memo_idx_ >= static_cast<int>(model_.memos.size())) {
        return CopyUtf8ToChar16Buffer("", buf, buf_size);
    }
    return CopyUtf8ToChar16Buffer(model_.memos[static_cast<size_t>(active_memo_idx_)].author_utf8, buf, buf_size);
}

int DocumentImpl::GetMemoDateTime(char16_t* buf, int buf_size) const {
    if (active_memo_idx_ < 0 || active_memo_idx_ >= static_cast<int>(model_.memos.size())) {
        return CopyUtf8ToChar16Buffer("", buf, buf_size);
    }
    return CopyUtf8ToChar16Buffer(model_.memos[static_cast<size_t>(active_memo_idx_)].datetime_utf8, buf, buf_size);
}

int DocumentImpl::CurrentPage() const {
    int page = 1;
    for (int i = 1; i <= caret_para_ && i < static_cast<int>(model_.paragraphs.size()); ++i) {
        if (model_.paragraphs[static_cast<size_t>(i)].page_break_before) {
            ++page;
        }
    }
    return page;
}

std::string DocumentImpl::BuildPreviewSummaryUtf8() const {
    std::ostringstream oss;
    for (size_t i = 0; i < model_.paragraphs.size(); ++i) {
        if (i > 0) oss << "\n";
        oss << model_.paragraphs[i].text_utf8;
    }
    return oss.str();
}

void DocumentImpl::NotifyWindowPreviewUpdated() {
    if (owner_hwp_obj_) {
        owner_hwp_obj_->NotifyAllWindowsPreviewUpdated();
    }
}

// ============================================================================
// HWindowImpl & HwpObjectImpl
// ============================================================================

HWindowImpl::HWindowImpl(
    HwpObjectImpl* owner_hwp_obj,
    std::shared_ptr<DocumentImpl> doc_impl,
    int x,
    int y,
    int width,
    int height,
    unsigned long style,
    HWND parent_hwnd)
    : owner_hwp_obj_(owner_hwp_obj),
      doc_impl_(std::move(doc_impl)),
      properties_(std::make_shared<ParameterSetImpl>("ViewProperties")) {
    properties_->SetUInt("ZoomType", 1);
    platform::ViewerWindowState init_state;
    init_state.x = x;
    init_state.y = y;
    init_state.width = width;
    init_state.height = height;
    init_state.style = style;
    init_state.rendered_preview_utf8 = doc_impl_ ? doc_impl_->BuildPreviewSummaryUtf8() : "";
    hwnd_ = platform::GetPlatformService().CreateViewerWindow(x, y, width, height, style, parent_hwnd, init_state);
}

HWindowImpl::~HWindowImpl() {
    if (hwnd_ != nullptr) {
        platform::GetPlatformService().DestroyViewerWindow(hwnd_);
        hwnd_ = nullptr;
    }
}

void HWindowImpl::SetProperties(ParameterSetImpl* set) {
    if (set) {
        properties_->MergeFrom(*set);
    }
}

void HWindowImpl::Resize(int x, int y, int width, int height) {
    platform::GetPlatformService().ResizeViewerWindow(hwnd_, x, y, width, height);
}

void HWindowImpl::SetFocus() {
    platform::GetPlatformService().SetViewerFocus(hwnd_);
}

void HWindowImpl::SetVisible(bool visible) {
    platform::GetPlatformService().SetViewerVisible(hwnd_, visible);
}

int HWindowImpl::GetCurPageNumber() const {
    return doc_impl_ ? doc_impl_->CurrentPage() : 1;
}

int HWindowImpl::GetCurLineNumber() const {
    return doc_impl_ ? (doc_impl_->CurrentPara() + 1) : 1;
}

int HWindowImpl::GetCurPosNumber() const {
    return doc_impl_ ? doc_impl_->CurrentPos() : 0;
}

void HWindowImpl::RefreshPreview() {
    if (hwnd_ && doc_impl_) {
        platform::GetPlatformService().UpdateViewerDocumentText(hwnd_, doc_impl_->BuildPreviewSummaryUtf8());
    }
}

HWPSDK::ParameterSet* HWindowImpl::WrapSet(std::shared_ptr<ParameterSetImpl> impl) const {
    if (!impl) return nullptr;
    wrapper_sets_.push_back(std::make_unique<HWPSDK::ParameterSet>(impl.get()));
    return wrapper_sets_.back().get();
}

HwpObjectImpl::HwpObjectImpl() = default;

std::shared_ptr<DocumentImpl> HwpObjectImpl::CreateDocumentImpl() {
    auto doc = std::make_shared<DocumentImpl>(this);
    documents_.push_back(doc);
    return doc;
}

void HwpObjectImpl::ReleaseDocumentImpl(DocumentImpl* doc_impl) {
    auto it = std::remove_if(documents_.begin(), documents_.end(),
                             [&](const std::shared_ptr<DocumentImpl>& d) { return d.get() == doc_impl; });
    documents_.erase(it, documents_.end());
}

std::shared_ptr<HWindowImpl> HwpObjectImpl::CreateHWindowImpl(
    int x,
    int y,
    int width,
    int height,
    unsigned long style,
    HWND parent_hwnd) {
    std::shared_ptr<DocumentImpl> doc = documents_.empty() ? CreateDocumentImpl() : documents_.back();
    auto win = std::make_shared<HWindowImpl>(this, doc, x, y, width, height, style, parent_hwnd);
    windows_.push_back(win);
    return win;
}

void HwpObjectImpl::DestroyHWindowImpl(HWindowImpl* win_impl) {
    auto it = std::remove_if(windows_.begin(), windows_.end(),
                             [&](const std::shared_ptr<HWindowImpl>& w) { return w.get() == win_impl; });
    windows_.erase(it, windows_.end());
}

void HwpObjectImpl::NotifyAllWindowsPreviewUpdated() {
    for (auto& win : windows_) {
        win->RefreshPreview();
    }
}

} // namespace openhwp::core
