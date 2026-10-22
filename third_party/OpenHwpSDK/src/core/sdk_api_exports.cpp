#include "document_engine.hpp"
#include "utf_conv.hpp"

namespace HWPSDK {

using namespace openhwp::core;

namespace {

ParameterArrayImpl* AsArrayImpl(SHDK::IObject* p) {
    return static_cast<ParameterArrayImpl*>(p);
}

ParameterSetImpl* AsSetImpl(SHDK::IObject* p) {
    return static_cast<ParameterSetImpl*>(p);
}

CtrlCodeImpl* AsCtrlImpl(SHDK::IObject* p) {
    return static_cast<CtrlCodeImpl*>(p);
}

ActionImpl* AsActionImpl(SHDK::IObject* p) {
    return static_cast<ActionImpl*>(p);
}

DocumentImpl* AsDocImpl(SHDK::IObject* p) {
    return static_cast<DocumentImpl*>(p);
}

HWindowImpl* AsWinImpl(SHDK::IObject* p) {
    return static_cast<HWindowImpl*>(p);
}

HwpObjectImpl* AsHwpObjImpl(SHDK::IObject* p) {
    return static_cast<HwpObjectImpl*>(p);
}

} // namespace

// ============================================================================
// Application
// ============================================================================

Application::Application() : m_pObject(nullptr) {}

Application::~Application() {
    ExitInstance();
}

int Application::InitInstance() {
    if (!m_pObject) {
        auto* impl = new HwpObjectImpl();
        m_pObject = new HwpObject(impl);
    }
    return 1; // TRUE (No license dongle/file restriction in OpenHwpSDK!)
}

void Application::ExitInstance() {
    if (m_pObject) {
        auto* impl = AsHwpObjImpl(m_pObject->GetImpl());
        delete m_pObject;
        m_pObject = nullptr;
        delete impl;
    }
}

int Application::GetVersion() {
    return 0x0D3C0060; // 13.60.0.96 compatible version number
}

void Application::SetLicensePath(const char* /*path*/) {
    // OpenHwpSDK is license-free; kept for 100% binary/source compatibility
}

HwpObject* Application::GetHwpObject() {
    if (!m_pObject) {
        InitInstance();
    }
    return m_pObject;
}

// ============================================================================
// HwpObject
// ============================================================================

HwpObject::HwpObject(SHDK::IObject* pObj) : m_pImpl(pObj) {}

HwpObject::~HwpObject() = default;

void HwpObject::Release() {
    delete this;
}

Document* HwpObject::CreateDocument() {
    auto* impl = AsHwpObjImpl(m_pImpl);
    if (!impl) return nullptr;
    auto doc_impl = impl->CreateDocumentImpl();
    return new Document(doc_impl.get());
}

void HwpObject::ReleaseDocument(Document* document) {
    if (!document) return;
    auto* impl = AsHwpObjImpl(m_pImpl);
    if (impl) {
        impl->ReleaseDocumentImpl(AsDocImpl(document->GetImpl()));
    }
    document->Release();
}

HWindow* HwpObject::CreateHWindow(int x, int y, int width, int height, unsigned long style, HWND hwnd) {
    auto* impl = AsHwpObjImpl(m_pImpl);
    if (!impl) return nullptr;
    auto win_impl = impl->CreateHWindowImpl(x, y, width, height, style, hwnd);
    return new HWindow(win_impl.get());
}

void HwpObject::DestroyHWindow(HWindow* window) {
    if (!window) return;
    auto* impl = AsHwpObjImpl(m_pImpl);
    if (impl) {
        impl->DestroyHWindowImpl(AsWinImpl(window->GetImpl()));
    }
    window->Release();
}

int HwpObject::GetLastErrorCode() {
    auto* impl = AsHwpObjImpl(m_pImpl);
    return impl ? impl->GetLastErrorCode() : 0;
}

SHDK::IObject* HwpObject::GetImpl() {
    return m_pImpl;
}

// ============================================================================
// HWindow
// ============================================================================

HWindow::HWindow(SHDK::IObject* pObj) : m_pDocument(nullptr), m_pImpl(pObj) {
    auto* impl = AsWinImpl(m_pImpl);
    if (impl && impl->GetDocumentImpl()) {
        m_pDocument = new Document(impl->GetDocumentImpl().get());
    }
}

HWindow::~HWindow() {
    delete m_pDocument;
    m_pDocument = nullptr;
}

void HWindow::Release() {
    delete this;
}

Document* HWindow::GetDocument() {
    return m_pDocument;
}

ParameterSet* HWindow::GetProperties() {
    auto* impl = AsWinImpl(m_pImpl);
    if (!impl) return nullptr;
    return new ParameterSet(impl->GetProperties().get());
}

void HWindow::SetProperties(ParameterSet* newvalue) {
    auto* impl = AsWinImpl(m_pImpl);
    if (impl && newvalue) {
        impl->SetProperties(AsSetImpl(newvalue->GetImpl()));
    }
}

HWND HWindow::GetWindowHandle() {
    auto* impl = AsWinImpl(m_pImpl);
    return impl ? impl->GetWindowHandle() : nullptr;
}

void HWindow::Resize(int x, int y, int width, int height) {
    auto* impl = AsWinImpl(m_pImpl);
    if (impl) impl->Resize(x, y, width, height);
}

void HWindow::SetFocus() {
    auto* impl = AsWinImpl(m_pImpl);
    if (impl) impl->SetFocus();
}

void HWindow::SetVisible(bool bVisible) {
    auto* impl = AsWinImpl(m_pImpl);
    if (impl) impl->SetVisible(bVisible);
}

int HWindow::GetCurPageNumber() {
    auto* impl = AsWinImpl(m_pImpl);
    return impl ? impl->GetCurPageNumber() : 1;
}

int HWindow::GetCurLineNumber() {
    auto* impl = AsWinImpl(m_pImpl);
    return impl ? impl->GetCurLineNumber() : 1;
}

int HWindow::GetCurPosNumber() {
    auto* impl = AsWinImpl(m_pImpl);
    return impl ? impl->GetCurPosNumber() : 0;
}

SHDK::IObject* HWindow::GetImpl() {
    return m_pImpl;
}

// ============================================================================
// Action
// ============================================================================

Action::Action(SHDK::IObject* obj) : m_pImpl(obj) {}

Action::~Action() = default;

void Action::Release() {
    delete this;
}

ParameterSet* Action::CreateParameterSet() {
    auto* impl = AsActionImpl(m_pImpl);
    if (!impl) return nullptr;
    auto set_impl = impl->CreateParameterSet();
    // Retain shared_ptr inside a heap-allocated holder or leak-free static pool
    static std::vector<std::shared_ptr<ParameterSetImpl>> s_action_sets;
    s_action_sets.push_back(set_impl);
    return new ParameterSet(set_impl.get());
}

int Action::GetDefault(ParameterSet* set) {
    auto* impl = AsActionImpl(m_pImpl);
    if (!impl || !set) return 0;
    return impl->GetDefault(AsSetImpl(set->GetImpl()));
}

int Action::Execute(ParameterSet* set) {
    auto* impl = AsActionImpl(m_pImpl);
    if (!impl) return 0;
    return impl->Execute(set ? AsSetImpl(set->GetImpl()) : nullptr);
}

SHDK::IObject* Action::GetImpl() {
    return m_pImpl;
}

// ============================================================================
// ParameterSet
// ============================================================================

ParameterSet::ParameterSet(SHDK::IObject* pSet) : m_pImpl(pSet) {}

ParameterSet::~ParameterSet() = default;

void ParameterSet::Release() {
    delete this;
}

int ParameterSet::GetCount() {
    auto* impl = AsSetImpl(m_pImpl);
    return impl ? impl->GetCount() : 0;
}

const char16_t* ParameterSet::GetSetID() {
    auto* impl = AsSetImpl(m_pImpl);
    return impl ? impl->GetSetIDUtf16() : u"";
}

ParameterArray* ParameterSet::CreateItemArray(const char16_t* itemid, int count) {
    auto* impl = AsSetImpl(m_pImpl);
    if (!impl) return nullptr;
    auto arr = impl->CreateItemArray(Utf16ToUtf8(itemid), count);
    return new ParameterArray(arr.get());
}

ParameterSet* ParameterSet::CreateItemSet(const char16_t* itemid, const char16_t* setid) {
    auto* impl = AsSetImpl(m_pImpl);
    if (!impl) return nullptr;
    auto sub = impl->CreateItemSet(Utf16ToUtf8(itemid), Utf16ToUtf8(setid));
    return new ParameterSet(sub.get());
}

bool ParameterSet::ItemExist(const char16_t* itemid) {
    auto* impl = AsSetImpl(m_pImpl);
    return impl ? impl->ItemExist(Utf16ToUtf8(itemid)) : false;
}

void ParameterSet::Merge(ParameterSet* srcset) {
    auto* impl = AsSetImpl(m_pImpl);
    if (impl && srcset && srcset->GetImpl()) {
        impl->MergeFrom(*AsSetImpl(srcset->GetImpl()));
    }
}

void ParameterSet::RemoveAll(const char16_t* setid) {
    auto* impl = AsSetImpl(m_pImpl);
    if (impl) impl->RemoveAll(Utf16ToUtf8(setid));
}

void ParameterSet::RemoveItem(const char16_t* itemid) {
    auto* impl = AsSetImpl(m_pImpl);
    if (impl) impl->RemoveItem(Utf16ToUtf8(itemid));
}

int ParameterSet::GetString(const char16_t* itemid, char16_t* value, int valuesize) {
    auto* impl = AsSetImpl(m_pImpl);
    return impl ? impl->GetString(Utf16ToUtf8(itemid), value, valuesize) : 0;
}

int ParameterSet::GetInt(const char16_t* itemid) {
    auto* impl = AsSetImpl(m_pImpl);
    return impl ? impl->GetInt(Utf16ToUtf8(itemid)) : 0;
}

unsigned int ParameterSet::GetUInt(const char16_t* itemid) {
    auto* impl = AsSetImpl(m_pImpl);
    return impl ? impl->GetUInt(Utf16ToUtf8(itemid)) : 0;
}

ParameterSet* ParameterSet::GetItemSet(const char16_t* itemid) {
    auto* impl = AsSetImpl(m_pImpl);
    if (!impl) return nullptr;
    auto sub = impl->GetItemSet(Utf16ToUtf8(itemid));
    return sub ? new ParameterSet(sub.get()) : nullptr;
}

ParameterArray* ParameterSet::GetItemArray(const char16_t* itemid) {
    auto* impl = AsSetImpl(m_pImpl);
    if (!impl) return nullptr;
    auto arr = impl->GetItemArray(Utf16ToUtf8(itemid));
    return arr ? new ParameterArray(arr.get()) : nullptr;
}

void ParameterSet::SetString(const char16_t* itemid, const char16_t* value) {
    auto* impl = AsSetImpl(m_pImpl);
    if (impl) impl->SetString(Utf16ToUtf8(itemid), Utf16ToUtf8(value));
}

void ParameterSet::SetInt(const char16_t* itemid, int value) {
    auto* impl = AsSetImpl(m_pImpl);
    if (impl) impl->SetInt(Utf16ToUtf8(itemid), value);
}

void ParameterSet::SetUInt(const char16_t* itemid, unsigned int value) {
    auto* impl = AsSetImpl(m_pImpl);
    if (impl) impl->SetUInt(Utf16ToUtf8(itemid), value);
}

void ParameterSet::SetItemSet(const char16_t* itemid, ParameterSet* value) {
    auto* impl = AsSetImpl(m_pImpl);
    if (impl && value && value->GetImpl()) {
        auto copy = std::make_shared<ParameterSetImpl>(AsSetImpl(value->GetImpl())->GetSetIDUtf8());
        copy->MergeFrom(*AsSetImpl(value->GetImpl()));
        impl->SetItemSet(Utf16ToUtf8(itemid), std::move(copy));
    }
}

void ParameterSet::SetItemArray(const char16_t* itemid, ParameterArray* value) {
    auto* impl = AsSetImpl(m_pImpl);
    if (impl && value && value->GetImpl()) {
        auto copy = std::make_shared<ParameterArrayImpl>();
        copy->CopyFrom(*AsArrayImpl(value->GetImpl()));
        impl->SetItemArray(Utf16ToUtf8(itemid), std::move(copy));
    }
}

unsigned int ParameterSet::GetItemType(const char16_t* itemid) {
    auto* impl = AsSetImpl(m_pImpl);
    return impl ? impl->GetItemType(Utf16ToUtf8(itemid)) : 0;
}

SHDK::IObject* ParameterSet::GetImpl() {
    return m_pImpl;
}

// ============================================================================
// ParameterArray
// ============================================================================

ParameterArray::ParameterArray(SHDK::IObject* obj) : m_pImpl(obj) {}

ParameterArray::~ParameterArray() = default;

void ParameterArray::Release() {
    delete this;
}

int ParameterArray::GetCount() {
    auto* impl = AsArrayImpl(m_pImpl);
    return impl ? impl->GetCount() : 0;
}

void ParameterArray::SetCount(int count) {
    auto* impl = AsArrayImpl(m_pImpl);
    if (impl) impl->SetCount(count);
}

void ParameterArray::Copy(ParameterArray* srcarray) {
    auto* impl = AsArrayImpl(m_pImpl);
    if (impl && srcarray && srcarray->GetImpl()) {
        impl->CopyFrom(*AsArrayImpl(srcarray->GetImpl()));
    }
}

int ParameterArray::GetString(int index, char16_t* value, int valuesize) {
    auto* impl = AsArrayImpl(m_pImpl);
    return impl ? impl->GetString(index, value, valuesize) : 0;
}

int ParameterArray::GetInt(int index) {
    auto* impl = AsArrayImpl(m_pImpl);
    return impl ? impl->GetInt(index) : 0;
}

unsigned int ParameterArray::GetUInt(int index) {
    auto* impl = AsArrayImpl(m_pImpl);
    return impl ? impl->GetUInt(index) : 0;
}

ParameterSet* ParameterArray::GetItemSet(int index) {
    auto* impl = AsArrayImpl(m_pImpl);
    if (!impl) return nullptr;
    auto sub = impl->GetItemSet(index);
    return sub ? new ParameterSet(sub.get()) : nullptr;
}

void ParameterArray::SetString(int index, const char16_t* value) {
    auto* impl = AsArrayImpl(m_pImpl);
    if (impl) impl->SetString(index, Utf16ToUtf8(value));
}

void ParameterArray::SetInt(int index, int value) {
    auto* impl = AsArrayImpl(m_pImpl);
    if (impl) impl->SetInt(index, value);
}

void ParameterArray::SetUInt(int index, unsigned int value) {
    auto* impl = AsArrayImpl(m_pImpl);
    if (impl) impl->SetUInt(index, value);
}

void ParameterArray::SetItemSet(int index, ParameterSet* value) {
    auto* impl = AsArrayImpl(m_pImpl);
    if (impl && value && value->GetImpl()) {
        auto copy = std::make_shared<ParameterSetImpl>(AsSetImpl(value->GetImpl())->GetSetIDUtf8());
        copy->MergeFrom(*AsSetImpl(value->GetImpl()));
        impl->SetItemSet(index, std::move(copy));
    }
}

int ParameterArray::GetType() {
    auto* impl = AsArrayImpl(m_pImpl);
    return impl ? impl->GetType() : 0;
}

SHDK::IObject* ParameterArray::GetImpl() {
    return m_pImpl;
}

// ============================================================================
// CtrlCode
// ============================================================================

CtrlCode::CtrlCode(SHDK::IObject* pCtrl) : m_pImpl(pCtrl) {}

CtrlCode::~CtrlCode() = default;

void CtrlCode::Release() {
    delete this;
}

short CtrlCode::GetCtrlCh() {
    auto* impl = AsCtrlImpl(m_pImpl);
    return impl ? impl->GetCtrlCh() : 0;
}

const char16_t* CtrlCode::GetCtrlID() {
    auto* impl = AsCtrlImpl(m_pImpl);
    return impl ? impl->GetCtrlIDUtf16() : u"";
}

CtrlCode* CtrlCode::GetNext() {
    auto* impl = AsCtrlImpl(m_pImpl);
    if (!impl || !impl->GetNext()) return nullptr;
    return new CtrlCode(impl->GetNext().get());
}

CtrlCode* CtrlCode::GetPrev() {
    auto* impl = AsCtrlImpl(m_pImpl);
    if (!impl || !impl->GetPrev()) return nullptr;
    return new CtrlCode(impl->GetPrev().get());
}

ParameterSet* CtrlCode::GetProperties() {
    auto* impl = AsCtrlImpl(m_pImpl);
    if (!impl || !impl->GetProperties()) return nullptr;
    return new ParameterSet(impl->GetProperties().get());
}

void CtrlCode::SetProperties(ParameterSet* newValue) {
    auto* impl = AsCtrlImpl(m_pImpl);
    if (impl && newValue && newValue->GetImpl()) {
        impl->GetProperties()->MergeFrom(*AsSetImpl(newValue->GetImpl()));
    }
}

int CtrlCode::GetUserDesc(char16_t* value, int valuesize) {
    auto* impl = AsCtrlImpl(m_pImpl);
    return impl ? CopyUtf8ToChar16Buffer(impl->GetUserDescUtf8(), value, valuesize) : 0;
}

ParameterSet* CtrlCode::GetAnchorPos(int type) {
    auto* impl = AsCtrlImpl(m_pImpl);
    if (!impl) return nullptr;
    static std::vector<std::shared_ptr<ParameterSetImpl>> s_anchor_sets;
    auto pos_set = impl->GetAnchorPos(type);
    s_anchor_sets.push_back(pos_set);
    return new ParameterSet(pos_set.get());
}

int CtrlCode::GetCtrlInstID(char16_t* value, int valuesize) {
    auto* impl = AsCtrlImpl(m_pImpl);
    return impl ? CopyUtf8ToChar16Buffer(impl->GetInstIDUtf8(), value, valuesize) : 0;
}

SHDK::IObject* CtrlCode::GetImpl() {
    return m_pImpl;
}

// ============================================================================
// Document
// ============================================================================

Document::Document(SHDK::IObject* doc) : m_pImpl(doc) {}

Document::~Document() = default;

void Document::Release() {
    delete this;
}

bool Document::Open(const char16_t* path, const char16_t* format, const char16_t* args) {
    auto* impl = AsDocImpl(m_pImpl);
    return impl ? impl->Open(Utf16ToPath(path), Utf16ToUtf8(format), Utf16ToUtf8(args)) : false;
}

void Document::Clear() {
    auto* impl = AsDocImpl(m_pImpl);
    if (impl) impl->Clear();
}

bool Document::Save(bool save_if_dirty) {
    auto* impl = AsDocImpl(m_pImpl);
    return impl ? impl->Save(save_if_dirty) : false;
}

bool Document::SaveAs(const char16_t* path, const char16_t* format, const char16_t* args) {
    auto* impl = AsDocImpl(m_pImpl);
    return impl ? impl->SaveAs(Utf16ToPath(path), Utf16ToUtf8(format), Utf16ToUtf8(args)) : false;
}

Action* Document::CreateAction(const char16_t* actionname) {
    auto* impl = AsDocImpl(m_pImpl);
    if (!impl) return nullptr;
    auto act_impl = impl->CreateAction(Utf16ToUtf8(actionname));
    if (!act_impl) return nullptr;
    impl->WrapAction(act_impl);
    return new Action(act_impl.get());
}

ParameterSet* Document::CreateSet(const char16_t* setid) {
    auto* impl = AsDocImpl(m_pImpl);
    if (!impl) return nullptr;
    auto set_impl = impl->CreateSet(Utf16ToUtf8(setid));
    impl->WrapSet(set_impl);
    return new ParameterSet(set_impl.get());
}

int Document::Run(const char16_t* actid) {
    auto* impl = AsDocImpl(m_pImpl);
    return impl ? impl->Run(Utf16ToUtf8(actid)) : 0;
}

void Document::LockCommand(const char16_t* actionID, bool lock) {
    auto* impl = AsDocImpl(m_pImpl);
    if (impl) impl->LockCommand(Utf16ToUtf8(actionID), lock);
}

bool Document::IsCommandLock(const char16_t* actionID) {
    auto* impl = AsDocImpl(m_pImpl);
    return impl ? impl->IsCommandLock(Utf16ToUtf8(actionID)) : false;
}

bool Document::IsEmpty() {
    auto* impl = AsDocImpl(m_pImpl);
    return impl ? impl->IsEmpty() : true;
}

int Document::IsModified() {
    auto* impl = AsDocImpl(m_pImpl);
    return impl ? impl->IsModified() : 0;
}

bool Document::IsPrivateInfoProtected() {
    auto* impl = AsDocImpl(m_pImpl);
    return impl ? impl->IsPrivateInfoProtected() : false;
}

bool Document::IsTrackChange() {
    auto* impl = AsDocImpl(m_pImpl);
    return impl ? impl->IsTrackChange() : false;
}

void Document::SetTrackChange(bool trackchange) {
    auto* impl = AsDocImpl(m_pImpl);
    if (impl) impl->SetTrackChange(trackchange);
}

bool Document::IsTrackChangePassword() {
    auto* impl = AsDocImpl(m_pImpl);
    return impl ? impl->IsTrackChangePassword() : false;
}

bool Document::IsSpellCheckCompleted() {
    auto* impl = AsDocImpl(m_pImpl);
    return impl ? impl->IsSpellCheckCompleted() : true;
}

bool Document::IsReadOnlyMode() {
    auto* impl = AsDocImpl(m_pImpl);
    return impl ? impl->IsReadOnlyMode() : false;
}

int Document::GetEditMode() {
    auto* impl = AsDocImpl(m_pImpl);
    return impl ? impl->GetEditMode() : 1;
}

void Document::SetEditMode(int mode) {
    auto* impl = AsDocImpl(m_pImpl);
    if (impl) impl->SetEditMode(mode);
}

int Document::GetSelectionMode() {
    auto* impl = AsDocImpl(m_pImpl);
    return impl ? impl->GetSelectionMode() : 0;
}

int Document::GetPath(char16_t* path, int pathsize) {
    auto* impl = AsDocImpl(m_pImpl);
    return impl ? impl->GetPath(path, pathsize) : 0;
}

bool Document::SetPath(const char16_t* path) {
    auto* impl = AsDocImpl(m_pImpl);
    return impl ? impl->SetPath(Utf16ToPath(path)) : false;
}

ParameterSet* Document::GetCellShape() {
    auto* impl = AsDocImpl(m_pImpl);
    if (!impl) return nullptr;
    auto s = impl->GetCellShape();
    impl->WrapSet(s);
    return new ParameterSet(s.get());
}

void Document::SetCellShape(ParameterSet* newvalue) {
    auto* impl = AsDocImpl(m_pImpl);
    if (impl && newvalue) impl->SetCellShape(AsSetImpl(newvalue->GetImpl()));
}

ParameterSet* Document::GetCharShape() {
    auto* impl = AsDocImpl(m_pImpl);
    if (!impl) return nullptr;
    auto s = impl->GetCharShape();
    impl->WrapSet(s);
    return new ParameterSet(s.get());
}

void Document::SetCharShape(ParameterSet* newvalue) {
    auto* impl = AsDocImpl(m_pImpl);
    if (impl && newvalue) impl->SetCharShape(AsSetImpl(newvalue->GetImpl()));
}

ParameterSet* Document::GetParaShape() {
    auto* impl = AsDocImpl(m_pImpl);
    if (!impl) return nullptr;
    auto s = impl->GetParaShape();
    impl->WrapSet(s);
    return new ParameterSet(s.get());
}

void Document::SetParaShape(ParameterSet* newvalue) {
    auto* impl = AsDocImpl(m_pImpl);
    if (impl && newvalue) impl->SetParaShape(AsSetImpl(newvalue->GetImpl()));
}

CtrlCode* Document::GetSelectedCtrl() {
    auto* impl = AsDocImpl(m_pImpl);
    if (!impl || !impl->GetSelectedCtrl()) return nullptr;
    return new CtrlCode(impl->GetSelectedCtrl().get());
}

CtrlCode* Document::GetHeadCtrl() {
    auto* impl = AsDocImpl(m_pImpl);
    if (!impl || !impl->GetHeadCtrl()) return nullptr;
    return new CtrlCode(impl->GetHeadCtrl().get());
}

CtrlCode* Document::GetLastCtrl() {
    auto* impl = AsDocImpl(m_pImpl);
    if (!impl || !impl->GetLastCtrl()) return nullptr;
    return new CtrlCode(impl->GetLastCtrl().get());
}

CtrlCode* Document::GetParentCtrl() {
    auto* impl = AsDocImpl(m_pImpl);
    if (!impl || !impl->GetParentCtrl()) return nullptr;
    return new CtrlCode(impl->GetParentCtrl().get());
}

int Document::GetFieldList(char16_t* filelist, int filelistsize, int number, int option) {
    auto* impl = AsDocImpl(m_pImpl);
    return impl ? impl->GetFieldList(filelist, filelistsize, number, option) : 0;
}

bool Document::FieldExist(const char16_t* field) {
    auto* impl = AsDocImpl(m_pImpl);
    return impl ? impl->FieldExist(Utf16ToUtf8(field)) : false;
}

bool Document::MoveToField(const char16_t* field, bool text, bool start, bool select) {
    auto* impl = AsDocImpl(m_pImpl);
    return impl ? impl->MoveToField(Utf16ToUtf8(field), text, start, select) : false;
}

int Document::GetFieldText(const char16_t* fieldlist, char16_t* fieldtext, int fieldtextsize) {
    auto* impl = AsDocImpl(m_pImpl);
    return impl ? impl->GetFieldText(Utf16ToUtf8(fieldlist), fieldtext, fieldtextsize) : 0;
}

void Document::PutFieldText(const char16_t* fieldlist, const char16_t* textlist) {
    auto* impl = AsDocImpl(m_pImpl);
    if (impl) impl->PutFieldText(Utf16ToUtf8(fieldlist), Utf16ToUtf8(textlist));
}

void Document::RenameField(const char16_t* oldname, const char16_t* newname) {
    auto* impl = AsDocImpl(m_pImpl);
    if (impl) impl->RenameField(Utf16ToUtf8(oldname), Utf16ToUtf8(newname));
}

bool Document::CreateField(const char16_t* direction, const char16_t* memo, const char16_t* name) {
    auto* impl = AsDocImpl(m_pImpl);
    return impl ? impl->CreateField(Utf16ToUtf8(direction), Utf16ToUtf8(memo), Utf16ToUtf8(name)) : false;
}

int Document::GetCurFieldName(int option, char16_t* fieldname, int fieldnamesize) {
    auto* impl = AsDocImpl(m_pImpl);
    return impl ? impl->GetCurFieldName(option, fieldname, fieldnamesize) : 0;
}

bool Document::SetCurFieldName(const char16_t* fieldname, int option, const char16_t* direction, const char16_t* memo) {
    auto* impl = AsDocImpl(m_pImpl);
    return impl ? impl->SetCurFieldName(Utf16ToUtf8(fieldname), option, Utf16ToUtf8(direction), Utf16ToUtf8(memo)) : false;
}

int Document::GetCurFieldState() {
    auto* impl = AsDocImpl(m_pImpl);
    return impl ? impl->GetCurFieldState() : 0;
}

int Document::ModifyFieldProperties(const char16_t* field, int remove, int add) {
    auto* impl = AsDocImpl(m_pImpl);
    return impl ? impl->ModifyFieldProperties(Utf16ToUtf8(field), remove, add) : 0;
}

int Document::SetFieldViewOption(int option) {
    auto* impl = AsDocImpl(m_pImpl);
    return impl ? impl->SetFieldViewOption(option) : 0;
}

int Document::GetMetatagList(int number, int option, char16_t* metataglist, int metataglistsize) {
    auto* impl = AsDocImpl(m_pImpl);
    return impl ? impl->GetMetatagList(number, option, metataglist, metataglistsize) : 0;
}

bool Document::MetatagExist(const char16_t* tag) {
    auto* impl = AsDocImpl(m_pImpl);
    return impl ? impl->MetatagExist(Utf16ToUtf8(tag)) : false;
}

bool Document::MoveToMetatag(const char16_t* tag, bool text, bool start, bool select) {
    auto* impl = AsDocImpl(m_pImpl);
    return impl ? impl->MoveToMetatag(Utf16ToUtf8(tag), text, start, select) : false;
}

int Document::GetMetatagNameText(const char16_t* tag, char16_t* metatagnametext, int metatagnametextsize) {
    auto* impl = AsDocImpl(m_pImpl);
    return impl ? impl->GetMetatagNameText(Utf16ToUtf8(tag), metatagnametext, metatagnametextsize) : 0;
}

bool Document::PutMetatagNameText(const char16_t* tag, const char16_t* text) {
    auto* impl = AsDocImpl(m_pImpl);
    return impl ? impl->PutMetatagNameText(Utf16ToUtf8(tag), Utf16ToUtf8(text)) : false;
}

bool Document::SetCurMetatagName(const char16_t* tag) {
    auto* impl = AsDocImpl(m_pImpl);
    return impl ? impl->SetCurMetatagName(Utf16ToUtf8(tag)) : false;
}

int Document::GetCurMetatagName(char16_t* metatagname, int metatagnamesize) {
    auto* impl = AsDocImpl(m_pImpl);
    return impl ? impl->GetCurMetatagName(metatagname, metatagnamesize) : 0;
}

bool Document::RenameMetatag(const char16_t* oldtag, const char16_t* newtag) {
    auto* impl = AsDocImpl(m_pImpl);
    return impl ? impl->RenameMetatag(Utf16ToUtf8(oldtag), Utf16ToUtf8(newtag)) : false;
}

int Document::ModifyMetatagProperties(const char16_t* tag, int remove, int add) {
    auto* impl = AsDocImpl(m_pImpl);
    return impl ? impl->ModifyMetatagProperties(Utf16ToUtf8(tag), remove, add) : 0;
}

int Document::GetCurMetatagState() {
    auto* impl = AsDocImpl(m_pImpl);
    return impl ? impl->GetCurMetatagState() : 0;
}

int Document::GetUserProperty(const char16_t* name, int option, char16_t* userproperty, int userpropertysize) {
    auto* impl = AsDocImpl(m_pImpl);
    return impl ? impl->GetUserProperty(Utf16ToUtf8(name), option, userproperty, userpropertysize) : 0;
}

int Document::SetUserProperty(const char16_t* name, int format, const char16_t* value, int option) {
    auto* impl = AsDocImpl(m_pImpl);
    return impl ? impl->SetUserProperty(Utf16ToUtf8(name), format, Utf16ToUtf8(value), option) : 0;
}

int Document::GetUserPropertyNameList(int option, char16_t* userpropertynamelist, int userpropertynamelistsize) {
    auto* impl = AsDocImpl(m_pImpl);
    return impl ? impl->GetUserPropertyNameList(option, userpropertynamelist, userpropertynamelistsize) : 0;
}

bool Document::MovePos(int moveID, int para, int pos) {
    auto* impl = AsDocImpl(m_pImpl);
    return impl ? impl->MovePos(moveID, para, pos) : false;
}

void Document::GetPos(int FAR* list, int FAR* para, int FAR* pos) {
    auto* impl = AsDocImpl(m_pImpl);
    if (impl) impl->GetPos(list, para, pos);
}

bool Document::SetPos(int list, int para, int pos) {
    auto* impl = AsDocImpl(m_pImpl);
    return impl ? impl->SetPos(list, para, pos) : false;
}

ParameterSet* Document::GetPosBySet() {
    auto* impl = AsDocImpl(m_pImpl);
    if (!impl) return nullptr;
    auto s = impl->GetPosBySet();
    impl->WrapSet(s);
    return new ParameterSet(s.get());
}

bool Document::SetPosBySet(ParameterSet* pos) {
    auto* impl = AsDocImpl(m_pImpl);
    return (impl && pos) ? impl->SetPosBySet(AsSetImpl(pos->GetImpl())) : false;
}

bool Document::SelectText(int spara, int spos, int epara, int epos) {
    auto* impl = AsDocImpl(m_pImpl);
    return impl ? impl->SelectText(spara, spos, epara, epos) : false;
}

int Document::GetSelectText(char16_t* text, int textSize) {
    auto* impl = AsDocImpl(m_pImpl);
    return impl ? impl->GetSelectText(text, textSize) : 0;
}

bool Document::GetSelectedPos(int* slist, int* spara, int* spos, int* elist, int* epara, int* epos) {
    auto* impl = AsDocImpl(m_pImpl);
    return impl ? impl->GetSelectedPos(slist, spara, spos, elist, epara, epos) : false;
}

bool Document::GetSelectedPosBySet(ParameterSet* sset, ParameterSet* eset) {
    auto* impl = AsDocImpl(m_pImpl);
    return impl ? impl->GetSelectedPosBySet(
                      sset ? AsSetImpl(sset->GetImpl()) : nullptr,
                      eset ? AsSetImpl(eset->GetImpl()) : nullptr)
                : false;
}

void Document::ReleaseScan() {
    auto* impl = AsDocImpl(m_pImpl);
    if (impl) impl->ReleaseScan();
}

bool Document::InitScan(int option, int range, int spara, int spos, int epara, int epos) {
    auto* impl = AsDocImpl(m_pImpl);
    return impl ? impl->InitScan(option, range, spara, spos, epara, epos) : false;
}

bool Document::Insert(const char16_t* path, const char16_t* format, const char16_t* arg) {
    auto* impl = AsDocImpl(m_pImpl);
    return impl ? impl->Insert(Utf16ToPath(path), Utf16ToUtf8(format), Utf16ToUtf8(arg)) : false;
}

CtrlCode* Document::InsertCtrl(const char16_t* ctrlid, ParameterSet* pset) {
    auto* impl = AsDocImpl(m_pImpl);
    if (!impl) return nullptr;
    auto c = impl->InsertCtrl(Utf16ToUtf8(ctrlid), pset ? AsSetImpl(pset->GetImpl()) : nullptr);
    return c ? new CtrlCode(c.get()) : nullptr;
}

CtrlCode* Document::InsertExtraCtrl(const char16_t* dataurl, const char16_t* extractrlid, const char16_t* extractrldata) {
    auto* impl = AsDocImpl(m_pImpl);
    if (!impl) return nullptr;
    auto c = impl->InsertExtraCtrl(Utf16ToUtf8(dataurl), Utf16ToUtf8(extractrlid), Utf16ToUtf8(extractrldata));
    return c ? new CtrlCode(c.get()) : nullptr;
}

bool Document::DeleteCtrl(CtrlCode* ctrl) {
    auto* impl = AsDocImpl(m_pImpl);
    return (impl && ctrl) ? impl->DeleteCtrl(AsCtrlImpl(ctrl->GetImpl())) : false;
}

bool Document::SelectCtrl(const char16_t* ctrllist, int option) {
    auto* impl = AsDocImpl(m_pImpl);
    return impl ? impl->SelectCtrl(Utf16ToUtf8(ctrllist), option) : false;
}

bool Document::UnSelectCtrl() {
    auto* impl = AsDocImpl(m_pImpl);
    return impl ? impl->UnSelectCtrl() : false;
}

CtrlCode* Document::InsertPicture(
    const char16_t* path,
    bool embedded,
    unsigned short sizeoption,
    bool reverse,
    bool watermark,
    unsigned short effect,
    int width,
    int height) {
    auto* impl = AsDocImpl(m_pImpl);
    if (!impl) return nullptr;
    auto c = impl->InsertPicture(Utf16ToPath(path), embedded, sizeoption, reverse, watermark, effect, width, height);
    return c ? new CtrlCode(c.get()) : nullptr;
}

bool Document::InsertBackgroundPicture(
    const char16_t* bordertype,
    const char16_t* path,
    bool embedded,
    unsigned short filloption,
    bool watermark,
    unsigned short effect,
    unsigned short brightness,
    unsigned short contrast) {
    auto* impl = AsDocImpl(m_pImpl);
    return impl ? impl->InsertBackgroundPicture(
                      Utf16ToUtf8(bordertype),
                      Utf16ToPath(path),
                      embedded,
                      filloption,
                      watermark,
                      effect,
                      brightness,
                      contrast)
                : false;
}

int Document::SetTextFile(const char16_t* data, const char16_t* format, const char16_t* option) {
    auto* impl = AsDocImpl(m_pImpl);
    return impl ? impl->SetTextFile(Utf16ToUtf8(data), Utf16ToUtf8(format), Utf16ToUtf8(option)) : 0;
}

int Document::GetText(char16_t* text, int textsize) {
    auto* impl = AsDocImpl(m_pImpl);
    return impl ? impl->GetText(text, textsize) : 0;
}

int Document::GetTextBySet(ParameterSet* text) {
    auto* impl = AsDocImpl(m_pImpl);
    return (impl && text) ? impl->GetTextBySet(AsSetImpl(text->GetImpl())) : 102;
}

bool Document::GetBinDataPath(int binid, char16_t* path, int pathsize) {
    auto* impl = AsDocImpl(m_pImpl);
    return impl ? impl->GetBinDataPath(binid, path, pathsize) : false;
}

ParameterSet* Document::GetFormObjectAttr(const char16_t* formname) {
    auto* impl = AsDocImpl(m_pImpl);
    if (!impl) return nullptr;
    auto s = impl->GetFormObjectAttr(Utf16ToUtf8(formname));
    impl->WrapSet(s);
    return new ParameterSet(s.get());
}

int Document::GetPageText(int page, int option, char16_t* text, int textSize) {
    auto* impl = AsDocImpl(m_pImpl);
    return impl ? impl->GetPageText(page, option, text, textSize) : -1;
}

bool Document::SetBarCodeImage(const char16_t* path, int pgno, int index, int x, int y, int width, int height) {
    auto* impl = AsDocImpl(m_pImpl);
    return impl ? impl->SetBarCodeImage(Utf16ToPath(path), pgno, index, x, y, width, height) : false;
}

bool Document::SetFormObjectAttr(const char16_t* formname, ParameterSet* pset) {
    auto* impl = AsDocImpl(m_pImpl);
    return (impl && pset) ? impl->SetFormObjectAttr(Utf16ToUtf8(formname), AsSetImpl(pset->GetImpl())) : false;
}

bool Document::CreatePageImage(
    const char16_t* path,
    int pgno,
    short resolution,
    short depth,
    const char16_t* format,
    unsigned int options) {
    auto* impl = AsDocImpl(m_pImpl);
    return impl ? impl->CreatePageImage(Utf16ToPath(path), pgno, resolution, depth, Utf16ToUtf8(format), options) : false;
}

bool Document::PrintDocument() {
    auto* impl = AsDocImpl(m_pImpl);
    return impl ? impl->PrintDocument() : false;
}

int Document::GetFilterList(const unsigned int flags, char16_t* filterlist, int filterlistsize) {
    auto* impl = AsDocImpl(m_pImpl);
    return impl ? impl->GetFilterList(flags, filterlist, filterlistsize) : 0;
}

int Document::GetTextFile(const char16_t* format, const char16_t* option, char16_t* textfile, int textfilesize) {
    auto* impl = AsDocImpl(m_pImpl);
    return impl ? impl->GetTextFile(Utf16ToUtf8(format), Utf16ToUtf8(option), textfile, textfilesize) : 0;
}

ParameterSet* Document::GetFileInfo(const char16_t* FileName) {
    auto* impl = AsDocImpl(m_pImpl);
    if (!impl) return nullptr;
    auto s = impl->GetFileInfo(Utf16ToPath(FileName));
    impl->WrapSet(s);
    return new ParameterSet(s.get());
}

int Document::GetTableCellAddr(int type) {
    auto* impl = AsDocImpl(m_pImpl);
    return impl ? impl->GetTableCellAddr(type) : -1;
}

bool Document::VersionSave(
    const char16_t* filepath,
    bool overwrite,
    bool infolock,
    const char16_t* writer,
    const char16_t* description) {
    auto* impl = AsDocImpl(m_pImpl);
    return impl ? impl->VersionSave(
                      Utf16ToPath(filepath),
                      overwrite,
                      infolock,
                      Utf16ToUtf8(writer),
                      Utf16ToUtf8(description))
                : false;
}

bool Document::VersionDelete(int index) {
    auto* impl = AsDocImpl(m_pImpl);
    return impl ? impl->VersionDelete(index) : false;
}

ParameterSet* Document::MakeVersionDiffAll(const char16_t* filepath) {
    auto* impl = AsDocImpl(m_pImpl);
    if (!impl) return nullptr;
    auto s = impl->MakeVersionDiffAll(Utf16ToPath(filepath));
    impl->WrapSet(s);
    return new ParameterSet(s.get());
}

ParameterSet* Document::GetVersionInfo(int index) {
    auto* impl = AsDocImpl(m_pImpl);
    if (!impl) return nullptr;
    auto s = impl->GetVersionInfo(index);
    impl->WrapSet(s);
    return new ParameterSet(s.get());
}

int Document::GetVersionHistoryCount() {
    auto* impl = AsDocImpl(m_pImpl);
    return impl ? impl->GetVersionHistoryCount() : 0;
}

ParameterSet* Document::GetStyleList(const char16_t* filePath) {
    auto* impl = AsDocImpl(m_pImpl);
    if (!impl) return nullptr;
    auto s = impl->GetStyleList(Utf16ToPath(filePath));
    impl->WrapSet(s);
    return new ParameterSet(s.get());
}

bool Document::ExportStyle(ParameterSet* styleset) {
    auto* impl = AsDocImpl(m_pImpl);
    return (impl && styleset) ? impl->ExportStyle(AsSetImpl(styleset->GetImpl())) : false;
}

bool Document::ImportStyle(ParameterSet* styleset) {
    auto* impl = AsDocImpl(m_pImpl);
    return (impl && styleset) ? impl->ImportStyle(AsSetImpl(styleset->GetImpl())) : false;
}

ParameterSet* Document::GetActionCmdUIStatus(const char16_t* actid, int withkey) {
    auto* impl = AsDocImpl(m_pImpl);
    if (!impl) return nullptr;
    auto s = impl->GetActionCmdUIStatus(Utf16ToUtf8(actid), withkey);
    impl->WrapSet(s);
    return new ParameterSet(s.get());
}

int Document::GetHeadingString(char16_t* headingstring, int headingstringsize) {
    auto* impl = AsDocImpl(m_pImpl);
    return impl ? impl->GetHeadingString(headingstring, headingstringsize) : 0;
}

int Document::GetCtrlHorizontalOffset(CtrlCode* pCtrl, int relTo) {
    auto* impl = AsDocImpl(m_pImpl);
    return (impl && pCtrl) ? impl->GetCtrlHorizontalOffset(AsCtrlImpl(pCtrl->GetImpl()), relTo) : 0;
}

int Document::GetCtrlVerticalOffset(CtrlCode* pCtrl, int relTo) {
    auto* impl = AsDocImpl(m_pImpl);
    return (impl && pCtrl) ? impl->GetCtrlVerticalOffset(AsCtrlImpl(pCtrl->GetImpl()), relTo) : 0;
}

int Document::GetMemoString(CtrlCode* ctrl, char16_t* memostring, int memostringsize) {
    auto* impl = AsDocImpl(m_pImpl);
    return (impl && ctrl) ? impl->GetMemoString(AsCtrlImpl(ctrl->GetImpl()), memostring, memostringsize) : 0;
}

int Document::GetFontList(const int langid, char16_t* fontlist, int fontlistsize) {
    auto* impl = AsDocImpl(m_pImpl);
    return impl ? impl->GetFontList(langid, fontlist, fontlistsize) : 0;
}

bool Document::ReplaceFont(int langid, const char16_t* desFontName, int desFontType, const char16_t* newFontName, int newFontType) {
    auto* impl = AsDocImpl(m_pImpl);
    return impl ? impl->ReplaceFont(langid, Utf16ToUtf8(desFontName), desFontType, Utf16ToUtf8(newFontName), newFontType) : false;
}

bool Document::ScanFont() {
    auto* impl = AsDocImpl(m_pImpl);
    return impl ? impl->ScanFont() : false;
}

bool Document::GetCtrlToPicture(char16_t* path, int format, bool fullsave) {
    auto* impl = AsDocImpl(m_pImpl);
    return impl ? impl->GetCtrlToPicture(Utf16ToPath(path), format, fullsave) : false;
}

int Document::InsertMemo(const char16_t* Content, const char16_t* Author, const char16_t* DateTime) {
    auto* impl = AsDocImpl(m_pImpl);
    return impl ? impl->InsertMemo(Utf16ToUtf8(Content), Utf16ToUtf8(Author), Utf16ToUtf8(DateTime)) : 0;
}

bool Document::SelectMemo(int nMemoInstID) {
    auto* impl = AsDocImpl(m_pImpl);
    return impl ? impl->SelectMemo(nMemoInstID) : false;
}

bool Document::ModifyMemo(const char16_t* Content, const char16_t* Author, const char16_t* DateTime) {
    auto* impl = AsDocImpl(m_pImpl);
    return impl ? impl->ModifyMemo(Utf16ToUtf8(Content), Utf16ToUtf8(Author), Utf16ToUtf8(DateTime)) : false;
}

bool Document::DeleteMemo(int nMemoInstID) {
    auto* impl = AsDocImpl(m_pImpl);
    return impl ? impl->DeleteMemo(nMemoInstID) : false;
}

int Document::GetMemoContent(char16_t* content, int contentsize) {
    auto* impl = AsDocImpl(m_pImpl);
    return impl ? impl->GetMemoContent(content, contentsize) : 0;
}

int Document::GetMemoAuthor(char16_t* author, int authorsize) {
    auto* impl = AsDocImpl(m_pImpl);
    return impl ? impl->GetMemoAuthor(author, authorsize) : 0;
}

int Document::GetMemoDateTime(char16_t* datetime, int datetimesize) {
    auto* impl = AsDocImpl(m_pImpl);
    return impl ? impl->GetMemoDateTime(datetime, datetimesize) : 0;
}

SHDK::IObject* Document::GetImpl() {
    return m_pImpl;
}

} // namespace HWPSDK
