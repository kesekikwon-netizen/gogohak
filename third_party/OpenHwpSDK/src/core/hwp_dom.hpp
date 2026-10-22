#pragma once

#include <cstdint>
#include <filesystem>
#include <map>
#include <memory>
#include <string>
#include <variant>
#include <vector>

#include "../../include/hwpsdk_include.h"
#include "../../include/platform/IPlatformService.h"

namespace openhwp::core {

class ParameterSetImpl;
class ParameterArrayImpl;
class DocumentImpl;

enum class ParamItemKind : unsigned int {
    None = 0,
    Int = 1,
    UInt = 2,
    String = 3,
    Set = 4,
    Array = 5
};

struct ParamValue {
    ParamItemKind kind = ParamItemKind::None;
    int int_val = 0;
    unsigned int uint_val = 0;
    std::string str_utf8;
    std::shared_ptr<ParameterSetImpl> set_val;
    std::shared_ptr<ParameterArrayImpl> array_val;
};

/**
 * @brief SHDK::IObject 기반 ParameterArray 내부 구현체
 */
class ParameterArrayImpl : public SHDK::IObject, public std::enable_shared_from_this<ParameterArrayImpl> {
public:
    explicit ParameterArrayImpl(int count = 0);
    virtual ~ParameterArrayImpl() = default;

    void Release() override {}

    int GetCount() const { return static_cast<int>(items_.size()); }
    void SetCount(int count);
    void CopyFrom(const ParameterArrayImpl& other);

    int GetString(int index, char16_t* value, int valuesize) const;
    int GetInt(int index) const;
    unsigned int GetUInt(int index) const;
    std::shared_ptr<ParameterSetImpl> GetItemSet(int index);

    void SetString(int index, const std::string& utf8_val);
    void SetInt(int index, int val);
    void SetUInt(int index, unsigned int val);
    void SetItemSet(int index, std::shared_ptr<ParameterSetImpl> set_val);
    int GetType() const { return item_type_; }

private:
    std::vector<ParamValue> items_;
    int item_type_ = 0;
};

/**
 * @brief SHDK::IObject 기반 ParameterSet 내부 구현체
 */
class ParameterSetImpl : public SHDK::IObject, public std::enable_shared_from_this<ParameterSetImpl> {
public:
    explicit ParameterSetImpl(std::string set_id_utf8 = "");
    virtual ~ParameterSetImpl() = default;

    void Release() override {}

    int GetCount() const { return static_cast<int>(items_.size()); }
    const char16_t* GetSetIDUtf16();
    const std::string& GetSetIDUtf8() const { return set_id_utf8_; }

    std::shared_ptr<ParameterArrayImpl> CreateItemArray(const std::string& item_id, int count);
    std::shared_ptr<ParameterSetImpl> CreateItemSet(const std::string& item_id, const std::string& set_id);
    bool ItemExist(const std::string& item_id) const;
    void MergeFrom(const ParameterSetImpl& src);
    void RemoveAll(const std::string& new_set_id);
    void RemoveItem(const std::string& item_id);

    int GetString(const std::string& item_id, char16_t* value, int valuesize) const;
    std::string GetStringUtf8(const std::string& item_id) const;
    int GetInt(const std::string& item_id) const;
    unsigned int GetUInt(const std::string& item_id) const;
    std::shared_ptr<ParameterSetImpl> GetItemSet(const std::string& item_id);
    std::shared_ptr<ParameterArrayImpl> GetItemArray(const std::string& item_id);

    void SetString(const std::string& item_id, const std::string& utf8_val);
    void SetInt(const std::string& item_id, int value);
    void SetUInt(const std::string& item_id, unsigned int value);
    void SetItemSet(const std::string& item_id, std::shared_ptr<ParameterSetImpl> value);
    void SetItemArray(const std::string& item_id, std::shared_ptr<ParameterArrayImpl> value);
    unsigned int GetItemType(const std::string& item_id) const;

    const std::map<std::string, ParamValue>& Items() const { return items_; }

private:
    std::string set_id_utf8_;
    std::u16string cached_set_id_u16_;
    std::map<std::string, ParamValue> items_;
};

struct CharShapeInfo {
    unsigned int height = 1000; // 10pt = 1000 HWPUNIT
    unsigned int bold = 0;
    unsigned int italic = 0;
    unsigned int underline = 0;
    unsigned int text_color = 0x000000;
    std::string font_name_utf8 = "함초롬바탕";
};

struct ParaShapeInfo {
    unsigned int align_type = 0; // 0: Justify, 1: Left, 2: Right, 3: Center
    unsigned int break_non_latin_word = 0;
    int line_spacing = 160;
};

struct FieldNode {
    std::string name_utf8;
    std::string direction_utf8;
    std::string memo_utf8;
    std::string text_utf8;
    std::string meta_tag_utf8;
    int field_type = HWPSDK::FieldClickHere; // FieldClickHere(2), FieldCell(1), FieldTextbox(8)
    int properties = HWPSDK::FIELD_CLICKHERE;
    int para_index = 0;
};

struct TableCellNode {
    int row = 0;
    int col = 0;
    int row_span = 1;
    int col_span = 1;
    int width_hwpunit = 10000;
    int height_hwpunit = 2000;
    std::string text_utf8;
    std::string field_name_utf8;
    std::string meta_tag_utf8;
    uint32_t bg_color = 0xFFFFFF;
    int face_style = -1;
    uint32_t hatch_color = 0;
    int border_type_top = 1;
    int border_width_top = 1;
    int border_type_bottom = 1;
    int border_width_bottom = 1;
    int border_type_left = 1;
    int border_width_left = 1;
    int border_type_right = 1;
    int border_width_right = 1;
    CharShapeInfo char_shape;
    ParaShapeInfo para_shape;
};

struct TableNode {
    std::string inst_id_utf8;
    int rows = 0;
    int cols = 0;
    bool treat_as_char = false;
    std::string meta_tag_utf8;
    int anchor_para = 0;
    std::vector<std::vector<TableCellNode>> cells;

    void InitializeGrid(int r, int c);
};

struct PictureNode {
    std::string inst_id_utf8;
    std::filesystem::path file_path;
    int bin_data_id = 0;
    bool embedded = true;
    unsigned short size_option = 0;
    bool reverse = false;
    bool watermark = false;
    unsigned short effect = 0;
    int width_mm = 50;
    int height_mm = 50;
    std::string meta_tag_utf8;
    int anchor_para = 0;
};

struct MemoNode {
    int inst_id = 0;
    std::string content_utf8;
    std::string author_utf8;
    std::string datetime_utf8;
    int anchor_para = 0;
};

struct PageNumPosInfo {
    bool enabled = false;
    int draw_pos = 0;
    int side_char = 0;
    int new_number = 1;
};

struct ParagraphNode {
    std::string text_utf8;
    CharShapeInfo char_shape;
    ParaShapeInfo para_shape;
    bool page_break_before = false;
    std::string heading_utf8;
    std::string meta_tag_utf8;
};

struct VersionHistoryEntry {
    std::filesystem::path file_path;
    std::string writer_utf8;
    std::string description_utf8;
    bool info_lock = false;
    uint32_t time_hi = 0;
    uint32_t time_lo = 0;
};

/**
 * @brief SHDK::IObject 기반 CtrlCode 내부 구현체
 */
class CtrlCodeImpl : public SHDK::IObject, public std::enable_shared_from_this<CtrlCodeImpl> {
public:
    CtrlCodeImpl(
        DocumentImpl* owner_doc,
        short ctrl_ch,
        std::string ctrl_id_utf8,
        std::string inst_id_utf8,
        std::string user_desc_utf8,
        int anchor_para = 0,
        int anchor_pos = 0);
    virtual ~CtrlCodeImpl() = default;

    void Release() override {}

    short GetCtrlCh() const { return ctrl_ch_; }
    const char16_t* GetCtrlIDUtf16();
    const std::string& GetCtrlIDUtf8() const { return ctrl_id_utf8_; }
    const std::string& GetInstIDUtf8() const { return inst_id_utf8_; }
    const std::string& GetUserDescUtf8() const { return user_desc_utf8_; }

    std::shared_ptr<CtrlCodeImpl> GetNext() const { return next_; }
    std::shared_ptr<CtrlCodeImpl> GetPrev() const { return prev_.lock(); }
    void SetNext(std::shared_ptr<CtrlCodeImpl> next) { next_ = std::move(next); }
    void SetPrev(std::shared_ptr<CtrlCodeImpl> prev) { prev_ = prev; }

    std::shared_ptr<ParameterSetImpl> GetProperties() { return properties_; }
    void SetProperties(std::shared_ptr<ParameterSetImpl> props);

    std::shared_ptr<ParameterSetImpl> GetAnchorPos(int type);
    int AnchorPara() const { return anchor_para_; }
    int AnchorPos() const { return anchor_pos_; }

private:
    DocumentImpl* owner_doc_ = nullptr;
    short ctrl_ch_ = 11;
    std::string ctrl_id_utf8_;
    std::u16string cached_ctrl_id_u16_;
    std::string inst_id_utf8_;
    std::string user_desc_utf8_;
    int anchor_para_ = 0;
    int anchor_pos_ = 0;
    std::shared_ptr<ParameterSetImpl> properties_;
    std::shared_ptr<CtrlCodeImpl> next_;
    std::weak_ptr<CtrlCodeImpl> prev_;
};

/**
 * @brief SHDK::IObject 기반 Action 내부 구현체
 */
class ActionImpl : public SHDK::IObject {
public:
    ActionImpl(DocumentImpl* owner_doc, std::string action_id_utf8);
    virtual ~ActionImpl() = default;

    void Release() override {}

    std::shared_ptr<ParameterSetImpl> CreateParameterSet();
    int GetDefault(ParameterSetImpl* set);
    int Execute(ParameterSetImpl* set);

    const std::string& GetActionIDUtf8() const { return action_id_utf8_; }

private:
    DocumentImpl* owner_doc_ = nullptr;
    std::string action_id_utf8_;
};

/**
 * @brief 문서 모델 전체를 보관하는 순수 C++20 구조체
 */
struct DocumentModel {
    std::filesystem::path current_path;
    std::string current_format_utf8 = "HWPX";
    bool modified = false;
    bool track_change = false;
    bool track_change_password = false;
    bool spell_check_completed = true;
    bool private_info_protected = false;
    int edit_mode = 1; // 0: ReadOnly/Viewer, 1: EditMode, 2: FormFillMode

    std::vector<ParagraphNode> paragraphs;
    std::vector<FieldNode> fields;
    std::vector<TableNode> tables;
    std::vector<PictureNode> pictures;
    std::vector<MemoNode> memos;
    std::map<int, std::filesystem::path> bin_data_paths;
    std::map<std::string, std::string> user_properties;
    std::vector<VersionHistoryEntry> version_history;
    std::vector<std::string> fonts_utf8 = {"함초롬바탕", "함초롬돋움", "맑은 고딕", "바탕", "돋움", "굴림"};
    PageNumPosInfo page_num_pos;

    void ClearAndReset() {
        current_path.clear();
        modified = false;
        paragraphs.clear();
        ParagraphNode initial_para;
        paragraphs.push_back(initial_para);
        fields.clear();
        tables.clear();
        pictures.clear();
        memos.clear();
        bin_data_paths.clear();
        user_properties.clear();
        page_num_pos = PageNumPosInfo{};
    }
};

} // namespace openhwp::core
