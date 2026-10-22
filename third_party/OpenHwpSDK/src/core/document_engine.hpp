#pragma once

#include <filesystem>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "hwp_dom.hpp"

namespace openhwp::core {

class HWindowImpl;
class HwpObjectImpl;

/**
 * @brief SHDK::IObject 기반 Document 내부 엔진 구현체
 */
class DocumentImpl : public SHDK::IObject, public std::enable_shared_from_this<DocumentImpl> {
public:
    explicit DocumentImpl(HwpObjectImpl* owner_hwp_obj = nullptr);
    virtual ~DocumentImpl() = default;

    void Release() override {}

    // 파일 입출력
    bool Open(const std::filesystem::path& path, const std::string& format_utf8, const std::string& args_utf8);
    void Clear();
    bool Save(bool save_if_dirty);
    bool SaveAs(const std::filesystem::path& path, const std::string& format_utf8, const std::string& args_utf8);
    bool Insert(const std::filesystem::path& path, const std::string& format_utf8, const std::string& args_utf8);

    // 액션 및 ParameterSet
    std::shared_ptr<ActionImpl> CreateAction(const std::string& action_name_utf8);
    std::shared_ptr<ParameterSetImpl> CreateSet(const std::string& set_id_utf8);
    int Run(const std::string& act_id_utf8);
    int ExecuteActionWithSet(const std::string& act_id_utf8, ParameterSetImpl* set);
    int PopulateActionDefaultSet(const std::string& act_id_utf8, ParameterSetImpl* set);

    void LockCommand(const std::string& action_id_utf8, bool lock);
    bool IsCommandLock(const std::string& action_id_utf8) const;

    // 문서 상태 및 속성
    bool IsEmpty() const;
    int IsModified() const { return model_.modified ? 1 : 0; }
    bool IsPrivateInfoProtected() const { return model_.private_info_protected; }
    bool IsTrackChange() const { return model_.track_change; }
    void SetTrackChange(bool tc) { model_.track_change = tc; }
    bool IsTrackChangePassword() const { return model_.track_change_password; }
    bool IsSpellCheckCompleted() const { return model_.spell_check_completed; }
    bool IsReadOnlyMode() const { return model_.edit_mode == 0; }
    int GetEditMode() const { return model_.edit_mode; }
    void SetEditMode(int mode) { model_.edit_mode = mode; }
    int GetSelectionMode() const;

    int GetPath(char16_t* path_buf, int path_size) const;
    bool SetPath(const std::filesystem::path& path);

    // 모양(Shape) 제어
    std::shared_ptr<ParameterSetImpl> GetCellShape();
    void SetCellShape(ParameterSetImpl* set);
    std::shared_ptr<ParameterSetImpl> GetCharShape();
    void SetCharShape(ParameterSetImpl* set);
    std::shared_ptr<ParameterSetImpl> GetParaShape();
    void SetParaShape(ParameterSetImpl* set);

    // 컨트롤 연결 리스트 제어
    void RebuildCtrlLinkedList();
    std::shared_ptr<CtrlCodeImpl> GetSelectedCtrl();
    std::shared_ptr<CtrlCodeImpl> GetHeadCtrl();
    std::shared_ptr<CtrlCodeImpl> GetLastCtrl();
    std::shared_ptr<CtrlCodeImpl> GetParentCtrl();
    std::shared_ptr<CtrlCodeImpl> InsertCtrl(const std::string& ctrl_id_utf8, ParameterSetImpl* init_set);
    std::shared_ptr<CtrlCodeImpl> InsertExtraCtrl(
        const std::string& data_url_utf8,
        const std::string& extra_id_utf8,
        const std::string& extra_data_utf8);
    bool DeleteCtrl(CtrlCodeImpl* ctrl);
    bool SelectCtrl(const std::string& ctrl_inst_list_utf8, int option);
    bool UnSelectCtrl();
    std::shared_ptr<CtrlCodeImpl> InsertPicture(
        const std::filesystem::path& path,
        bool embedded,
        unsigned short size_option,
        bool reverse,
        bool watermark,
        unsigned short effect,
        int width_mm,
        int height_mm);
    bool InsertBackgroundPicture(
        const std::string& border_type_utf8,
        const std::filesystem::path& path,
        bool embedded,
        unsigned short fill_option,
        bool watermark,
        unsigned short effect,
        unsigned short brightness,
        unsigned short contrast);

    // 필드(Field) API
    int GetFieldList(char16_t* buf, int buf_size, int number_opt, int option) const;
    bool FieldExist(const std::string& field_utf8) const;
    bool MoveToField(const std::string& field_utf8, bool text, bool start, bool select);
    int GetFieldText(const std::string& field_list_utf8, char16_t* out_buf, int out_size) const;
    void PutFieldText(const std::string& field_list_utf8, const std::string& text_list_utf8);
    void RenameField(const std::string& old_name_utf8, const std::string& new_name_utf8);
    bool CreateField(const std::string& direction_utf8, const std::string& memo_utf8, const std::string& name_utf8);
    int GetCurFieldName(int option, char16_t* buf, int buf_size) const;
    bool SetCurFieldName(const std::string& name_utf8, int option, const std::string& direction_utf8, const std::string& memo_utf8);
    int GetCurFieldState() const;
    int ModifyFieldProperties(const std::string& field_utf8, int remove_prop, int add_prop);
    int SetFieldViewOption(int option);

    // 메타태그(Metatag) 및 UserProperty API
    int GetMetatagList(int number_opt, int option, char16_t* buf, int buf_size) const;
    bool MetatagExist(const std::string& tag_utf8) const;
    bool MoveToMetatag(const std::string& tag_utf8, bool text, bool start, bool select);
    int GetMetatagNameText(const std::string& tag_utf8, char16_t* buf, int buf_size) const;
    bool PutMetatagNameText(const std::string& tag_utf8, const std::string& text_utf8);
    bool SetCurMetatagName(const std::string& tag_utf8);
    int GetCurMetatagName(char16_t* buf, int buf_size) const;
    bool RenameMetatag(const std::string& old_tag_utf8, const std::string& new_tag_utf8);
    int ModifyMetatagProperties(const std::string& tag_utf8, int remove_prop, int add_prop);
    int GetCurMetatagState() const;
    int GetUserProperty(const std::string& name_utf8, int option, char16_t* buf, int buf_size) const;
    int SetUserProperty(const std::string& name_utf8, int format, const std::string& value_utf8, int option);
    int GetUserPropertyNameList(int option, char16_t* buf, int buf_size) const;

    // 캐럿 및 선택 블록 API
    bool MovePos(int move_id, int para, int pos);
    void GetPos(int* list, int* para, int* pos) const;
    bool SetPos(int list, int para, int pos);
    std::shared_ptr<ParameterSetImpl> GetPosBySet();
    bool SetPosBySet(ParameterSetImpl* pos_set);
    bool SelectText(int spara, int spos, int epara, int epos);
    int GetSelectText(char16_t* buf, int buf_size) const;
    bool GetSelectedPos(int* slist, int* spara, int* spos, int* elist, int* epara, int* epos) const;
    bool GetSelectedPosBySet(ParameterSetImpl* sset, ParameterSetImpl* eset);

    // 텍스트 스캔 및 추출 API
    void ReleaseScan();
    bool InitScan(int option, int range, int spara, int spos, int epara, int epos);
    int GetText(char16_t* buf, int buf_size);
    int GetTextBySet(ParameterSetImpl* text_set);
    int SetTextFile(const std::string& data_utf8, const std::string& format_utf8, const std::string& option_utf8);
    int GetTextFile(const std::string& format_utf8, const std::string& option_utf8, char16_t* buf, int buf_size) const;
    int GetPageText(int page, int option, char16_t* buf, int buf_size) const;

    // 기타 문서 API (BinData, FormObject, PageImage, Version, Style, Font, Memo)
    bool GetBinDataPath(int bin_id, char16_t* buf, int buf_size) const;
    std::shared_ptr<ParameterSetImpl> GetFormObjectAttr(const std::string& form_name_utf8);
    bool SetFormObjectAttr(const std::string& form_name_utf8, ParameterSetImpl* pset);
    bool SetBarCodeImage(const std::filesystem::path& path, int pgno, int index, int x, int y, int width, int height);
    bool CreatePageImage(const std::filesystem::path& path, int pgno, short resolution, short depth, const std::string& format_utf8, unsigned int options);
    bool PrintDocument();
    int GetFilterList(unsigned int flags, char16_t* buf, int buf_size) const;
    std::shared_ptr<ParameterSetImpl> GetFileInfo(const std::filesystem::path& file_path);
    int GetTableCellAddr(int type) const;

    bool VersionSave(const std::filesystem::path& file_path, bool overwrite, bool info_lock, const std::string& writer_utf8, const std::string& desc_utf8);
    bool VersionDelete(int index);
    std::shared_ptr<ParameterSetImpl> MakeVersionDiffAll(const std::filesystem::path& file_path);
    std::shared_ptr<ParameterSetImpl> GetVersionInfo(int index);
    int GetVersionHistoryCount() const { return static_cast<int>(model_.version_history.size()); }

    std::shared_ptr<ParameterSetImpl> GetStyleList(const std::filesystem::path& file_path);
    bool ExportStyle(ParameterSetImpl* style_set);
    bool ImportStyle(ParameterSetImpl* style_set);
    std::shared_ptr<ParameterSetImpl> GetActionCmdUIStatus(const std::string& act_id_utf8, int with_key);
    int GetHeadingString(char16_t* buf, int buf_size) const;
    int GetCtrlHorizontalOffset(CtrlCodeImpl* ctrl, int rel_to) const;
    int GetCtrlVerticalOffset(CtrlCodeImpl* ctrl, int rel_to) const;

    int GetMemoString(CtrlCodeImpl* ctrl, char16_t* buf, int buf_size) const;
    int GetFontList(int lang_id, char16_t* buf, int buf_size) const;
    bool ReplaceFont(int lang_id, const std::string& des_font_utf8, int des_type, const std::string& new_font_utf8, int new_type);
    bool ScanFont();
    bool GetCtrlToPicture(const std::filesystem::path& path, int format, bool full_save);

    int InsertMemo(const std::string& content_utf8, const std::string& author_utf8, const std::string& datetime_utf8);
    bool SelectMemo(int memo_inst_id);
    bool ModifyMemo(const std::string& content_utf8, const std::string& author_utf8, const std::string& datetime_utf8);
    bool DeleteMemo(int memo_inst_id);
    int GetMemoContent(char16_t* buf, int buf_size) const;
    int GetMemoAuthor(char16_t* buf, int buf_size) const;
    int GetMemoDateTime(char16_t* buf, int buf_size) const;

    DocumentModel& Model() { return model_; }
    const DocumentModel& Model() const { return model_; }
    int CurrentPara() const { return caret_para_; }
    int CurrentPos() const { return caret_pos_; }
    int CurrentPage() const;
    std::string BuildPreviewSummaryUtf8() const;

private:
    void NotifyWindowPreviewUpdated();
    std::string GetSelectedTextUtf8() const;

    HwpObjectImpl* owner_hwp_obj_ = nullptr;
    DocumentModel model_;
    std::set<std::string> locked_commands_;

    // Caret & Selection State
    int caret_list_ = 0; // 0: main body, >=1: inside table (table_idx + 1)
    int caret_para_ = 0;
    int caret_pos_ = 0;
    int active_table_idx_ = -1;
    int active_cell_row_ = 0;
    int active_cell_col_ = 0;
    int active_field_idx_ = -1;
    int active_memo_idx_ = -1;

    bool has_text_selection_ = false;
    int sel_slist_ = 0;
    int sel_spara_ = 0;
    int sel_spos_ = 0;
    int sel_elist_ = 0;
    int sel_epara_ = 0;
    int sel_epos_ = 0;

    bool cell_block_mode_ = false;
    int cell_block_srow_ = 0;
    int cell_block_scol_ = 0;
    int cell_block_erow_ = 0;
    int cell_block_ecol_ = 0;

    // Copied shape clipboard for ShapeCopyPaste
    CharShapeInfo copied_char_shape_;
    ParaShapeInfo copied_para_shape_;
    int field_view_option_ = 2;

    // Scan state (InitScan / GetText / ReleaseScan)
    bool scan_initialized_ = false;
    size_t scan_cursor_ = 0;
    int scan_last_para_ = 0;
    std::vector<std::pair<int, std::string>> scan_items_;

    // Linked list of CtrlCodeImpl
    std::vector<std::shared_ptr<CtrlCodeImpl>> ctrl_nodes_;
    std::shared_ptr<CtrlCodeImpl> selected_ctrl_;

    // Retained child objects so raw pointers returned by SDK remain valid until Document destruction
    mutable std::vector<std::shared_ptr<ActionImpl>> owned_actions_;
    mutable std::vector<std::shared_ptr<ParameterSetImpl>> owned_sets_;
    mutable std::vector<std::unique_ptr<HWPSDK::Action>> wrapper_actions_;
    mutable std::vector<std::unique_ptr<HWPSDK::ParameterSet>> wrapper_sets_;
    mutable std::vector<std::unique_ptr<HWPSDK::CtrlCode>> wrapper_ctrls_;

public:
    HWPSDK::Action* WrapAction(std::shared_ptr<ActionImpl> impl) const;
    HWPSDK::ParameterSet* WrapSet(std::shared_ptr<ParameterSetImpl> impl) const;
    HWPSDK::CtrlCode* WrapCtrl(std::shared_ptr<CtrlCodeImpl> impl) const;
};

/**
 * @brief SHDK::IObject 기반 HWindow 내부 구현체
 */
class HWindowImpl : public SHDK::IObject {
public:
    HWindowImpl(
        HwpObjectImpl* owner_hwp_obj,
        std::shared_ptr<DocumentImpl> doc_impl,
        int x,
        int y,
        int width,
        int height,
        unsigned long style,
        HWND parent_hwnd);
    virtual ~HWindowImpl();

    void Release() override {}

    std::shared_ptr<DocumentImpl> GetDocumentImpl() const { return doc_impl_; }
    std::shared_ptr<ParameterSetImpl> GetProperties() { return properties_; }
    void SetProperties(ParameterSetImpl* set);
    HWND GetWindowHandle() const { return hwnd_; }
    void Resize(int x, int y, int width, int height);
    void SetFocus();
    void SetVisible(bool visible);
    int GetCurPageNumber() const;
    int GetCurLineNumber() const;
    int GetCurPosNumber() const;
    void RefreshPreview();

    HWPSDK::ParameterSet* WrapSet(std::shared_ptr<ParameterSetImpl> impl) const;

private:
    HwpObjectImpl* owner_hwp_obj_ = nullptr;
    std::shared_ptr<DocumentImpl> doc_impl_;
    std::shared_ptr<ParameterSetImpl> properties_;
    HWND hwnd_ = nullptr;
    mutable std::vector<std::unique_ptr<HWPSDK::ParameterSet>> wrapper_sets_;
};

/**
 * @brief SHDK::IObject 기반 HwpObject 내부 구현체
 */
class HwpObjectImpl : public SHDK::IObject {
public:
    HwpObjectImpl();
    virtual ~HwpObjectImpl() = default;

    void Release() override {}

    std::shared_ptr<DocumentImpl> CreateDocumentImpl();
    void ReleaseDocumentImpl(DocumentImpl* doc_impl);
    std::shared_ptr<HWindowImpl> CreateHWindowImpl(int x, int y, int width, int height, unsigned long style, HWND parent_hwnd);
    void DestroyHWindowImpl(HWindowImpl* win_impl);
    int GetLastErrorCode() const { return last_error_code_; }
    void SetLastErrorCode(int code) { last_error_code_ = code; }
    void NotifyAllWindowsPreviewUpdated();

private:
    int last_error_code_ = 0;
    std::vector<std::shared_ptr<DocumentImpl>> documents_;
    std::vector<std::shared_ptr<HWindowImpl>> windows_;
};

} // namespace openhwp::core
