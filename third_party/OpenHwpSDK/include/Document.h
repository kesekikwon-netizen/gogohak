#pragma once

namespace HWPSDK {

	class Action;
	class ParameterSet;
	class CtrlCode;
	
	/**
	 * @class Document
	 * @ingroup Apis
	 * @brief 한글 SDK의 Document 클래스를 나열합니다.
	 */
	class SDK_FE_API Document
	{
	public:
		Document(SHDK::IObject* doc);
		virtual ~Document();

		void Release();

		/**
		 * @brief 문서를 엽니다.
		 * @details
		 * @param path 문서의 경로이며 URL을 사용할 수 있습니다. (ver:0x05050111)
		 * @param format 문서 형식이며 별도 설명을 참조하세요. 빈 문자열을 지정하면 자동으로 인식되며 생략하면 빈 문자열이 지정됩니다.
		 * @param args 세부 옵션으로 format에 지정한 파일 형식에 따라 다르며 생략하면 빈 문자열이 지정됩니다. 암호가 설정되어 있는 문서의 경우 "Password:비밀번호;" 형태로 암호를 지정해야 Open이 정상적으로 수행됩니다.
		 * @return true 성공
		 * @return false 실패
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_document_Open
		 */
		bool Open(const char16_t* path, const char16_t* format = nullptr, const char16_t* args = nullptr);
		
		/**
		 * @brief 현재 편집 중인 문서의 내용을 닫고 빈 문서 편집 상태로 돌아갑니다.
		 *
		 * @return void
		 * **Examples**
		 * \snippet snippets.cpp example_document_Clear
		 */
		void Clear();

		/**
		 * @brief 현재 편집 중인 문서를 저장합니다.
		 *
		 * @param save_if_dirty true를 지정하면 문서가 변경된 경우에만 저장하며, false를 지정하면 변경 여부와 관계없이 무조건 문서를 저장합니다. 생략하면 true가 지정됩니다.
		 * @return true
		 * @return false
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_document_Save
		 */
		bool Save(bool save_if_dirty = true);

		/**
		 * @brief 현재 편집 중인 문서를 지정한 이름으로 저장합니다.
		 *
		 * @param path 문서의 경로입니다.
		 * @param format 문서 형식이며 별도 설명을 참조하세요. 생략하면 "HWP"가 지정됩니다.
		 * @param args 세부 옵션으로 format에 지정한 파일 형식에 따라 다르며 생략하면 빈 문자열이 지정됩니다.
		 * @return true
		 * @return false
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_document_SaveAs
		 */
		bool SaveAs(const char16_t* path, const char16_t* format = nullptr, const char16_t* args = nullptr);

		/**
		 * @brief Action 객체를 생성합니다.
		 *
		 * @param actionname 액션 ID입니다.
		 * @return IAction* Action object
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_document_CreateAction
		 */
		Action* CreateAction(const char16_t* actionname);

		/**
		 * @brief ParameterSet을 생성합니다.
		 *
		 * @param setid 생성할 ParameterSet의 ID(SetIDTable.hwp)입니다.
		 * @return ParameterSet* 생성된 ParameterSet 개체
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_document_CreateSet
		 */
		ParameterSet* CreateSet(const char16_t* setid);

		// @ todo CreateArray가 없넹 .. 


		/**
		 * @brief 액션을 실행합니다.
		 *
		 * @param actid 액션 ID입니다. (ActionIDTable.hwp 참조)
		 * @return int 실패하면 -1
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_document_Run
		 */
		int Run(const char16_t* actid);

		/**
		 * @brief 특정 액션이 실행되지 않도록 잠급니다.
		 *
		 * @param actionID 액션 ID입니다. (ActionIDTable.Hwp 참조)
		 * @param lock true면 액션 실행을 잠그고, false면 액션을 실행합니다.
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_document_LockCommand
		 */
		void LockCommand(const char16_t* actionID, bool lock);

		/**
		 * @brief 해당 액션이 잠겨 있는지 확인합니다.
		 *
		 * @param actionID 액션 ID입니다. (ActionIDTable.Hwp 참조)
		 * @return true
		 * @return false
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_document_IsCommandLock
		 */
		bool IsCommandLock(const char16_t* actionID);

		// Document API+
		/**
		 * @brief 빈 문서인지 여부를 나타냅니다. (읽기 전용)
		 *
		 * @return true
		 * @return false
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_document_IsEmpty
		 */
		bool IsEmpty();

		/**
		 * @brief 문서가 변경되었는지 나타냅니다. (읽기 전용) @n
		 * 0 | 변경되지 않은 상태 @n
		 * 1 | 변경된 상태 @n
		 * 2 | 변경되었으나 자동 저장된 상태 @n
		 * @return true
		 * @return false
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_document_IsModified
		 */
		int IsModified();

		/**
		 * @brief 문서의 변경 상태를 설정합니다.
		 *
		 * @param modified 문서의 변경 상태를 설정합니다.
		 * 
		 * \snippet snippets.cpp example_document_
		 */

		// @TODO 

		/**
		 * @brief 개인 정보가 보호되어 있는지 여부를 나타냅니다. (읽기 전용)
		 *
		 * @return true
		 * @return false
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_document_IsPrivateInfoProtected
		 */
		bool IsPrivateInfoProtected();

		/**

		 * @brief 변경 내용 추적이 적용되어 있는지 여부를 나타냅니다.
		 *
		 * @return true
		 * @return false
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_document_IsTrackChange
		 */
		bool IsTrackChange();

		/**
		 * @brief 변경 내용 추적을 적용합니다.
		 *
		 * @param trackchange true: 변경 내용 추적 적용, false: 변경 내용 추적 해제
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_document_SetTrackChange
		 */
		void SetTrackChange(bool trackchange);

		/**
		 * @brief 변경 내용 추적 암호가 적용되었는지 여부를 나타냅니다.
		 *
		 * @return true
		 * @return false
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_document_IsTrackChangePassword
		 */
		bool IsTrackChangePassword();

		/**
		 * @brief 맞춤법 검사가 완료되었는지 여부를 나타냅니다.
		 *
		 * @return true
		 * @return false
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_document_IsSpellCheckCompleted
		 */
		bool IsSpellCheckCompleted();

		/**
		 * @brief 문서가 읽기 전용인지 여부를 알려줍니다.
		 *
		 * @return true
		 * @return false
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_document_IsReadOnlyMode
		 */
		bool IsReadOnlyMode();

		/**
		 * @brief 현재 문서의 편집 모드 상태를 나타냅니다.
		 *
		 * @return int
		 * @brief
		 * 값|설명
		 * -|-
		 * 0  | 읽기 전용
		 * 1  | 일반 편집 모드
		 * 2  | 양식 모드(양식 사용자 모드): 셀과 누름틀 중 양식 모드에서 편집 가능한 속성을 가진 항목만 편집 가능
		 * 16  | 예약
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_document_GetEditMode
		 */
		int GetEditMode();

		/**
		* @brief 현재 문서를 편집 모드로 설정합니다.
		* @param nNewValue
		* @brief
		* 값|설명
		* -|-
		* 0 | ReadOnly
		* 1 | Normal
		* 2 | Protected: 셀과 누름틀만 편집 가능
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_document_SetEditMode
		 */
		void SetEditMode(int mode);

		/**
		 * @brief 문서에서 선택된 내용의 상태를 알려줍니다. (읽기 전용)
		 * 일반 블록이 아닌 F3키나 F4키에 의해 블록이 지정된 경우
		 * HWPSEL_STRICT_MODE ( = 0x10, 십진수 16)으로 OR 마스크되기 때문에
		 * 항상 0x0F(십진수15)로 AND 마스크한 결과로 판단해야 합니다.
		 *
		 * @return int
		 * @brief
		 * 값|설명
		 * -|-
		 * 0 | 선택 내용 없음
		 * 1 | 일반 내용 선택
		 * 2 | Column 선택
		 * 3 | Table 선택
		 * 4 | Ctrl 선택
		 * 15 | MaskMode 선택
		 * 16 | StrictMode 선택
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_document_GetSelectionMode
		 */
		int GetSelectionMode();
		
		/**
		 * @brief 문서의 경로를 가져옵니다.
		 *
		 * @param path 문서 경로 버퍼 포인터입니다.
		 * @param pathsize 문서 경로 버퍼 포인터의 크기입니다.
		 * @return int 버퍼가 지정되지 않은 경우 필요한 버퍼 크기
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_document_GetPath
		 */
		int GetPath(char16_t* path, int pathsize);

		/**
		 * @brief 문서의 경로를 설정합니다.
		 *
		 * @param path 문서 경로
		 * @return true 성공
		 * @return false 실패
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_document_SetPath
		 */
		bool SetPath(const char16_t* path);

		/**
		 * @brief 현재 선택되어 있는 표와 셀의 모양 정보를 나타냅니다.@n
		 * ParameterSet/Table로 표의 속성에 대한 기본 정보를 나타내며, "Cell"이 ParameterSet/Cell로 셀의 속성을 나타냅니다.@n
		 * 셀 블록이 지정되지 않은 경우 현재 캐럿이 위치한 셀만 대상으로 합니다. 현재 표 내부에 캐럿이 위치하지 않으면 오류가 발생합니다.@n
		 * @return IParameterSet* "Cell" ParameterSet
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_document_GetCellShape 
		 */
		ParameterSet* GetCellShape();

		/**
		 * @brief 현재 선택된 표와 셀의 모양 정보를 설정합니다.
		 *
		 * @param newvalue "Cell" ParameterSet
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_document_SetCellShape
		 */
		void SetCellShape(ParameterSet* newvalue);

		/**
		 * @brief 현재 선택된 내용의 글자 모양을 나타냅니다.
		 *
		 * @return IParameterSet* "CharShape" ParamterSet
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_document_GetCharShape 
		 */
		ParameterSet* GetCharShape();

		/**
		 * @brief 현재 선택된 내용의 글자 모양을 설정합니다.
		 *
		 * @param newvalue "CharShape" ParamterSet
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_document_SetCharShape
		 */
		void SetCharShape(ParameterSet* newvalue);

		/**
		 * @brief 현재 선택된 내용의 문단 모양을 나타냅니다.
		 *
		 * @return IParameterSet* "ParaShape" ParameterSet
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_document_GetParaShape 
		 */
		ParameterSet* GetParaShape();

		/**
		 * @brief 현재 선택된 내용의 문단 모양을 설정합니다.
		 *
		 * @param newvalue "ParaShape" ParameterSet
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_document_SetParaShape
		 */
		void SetParaShape(ParameterSet* newvalue);

		/**
		 * @brief 현재 선택된 컨트롤 객체입니다. (읽기 전용) @n
		 * 마우스 커서로 선택된 컨트롤 객체가 있거나 컨트롤 객체 앞에 캐럿이 있는 경우 해당 컨트롤 객체에 대한 정보를 가져옵니다. @n
		 * CtrlCode 객체가 반환되면 CtrlCode 객체의 속성과 함수를 사용할 수 있습니다. @n
		 *
		 * @return ICtrlCode*
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_document_GetSelectedCtrl 
		 */
		CtrlCode* GetSelectedCtrl();

		/**
		 * @brief 문서 중 첫 번째 컨트롤입니다. (읽기 전용) @n
		 * 문서의 모든 컨트롤(표, 그림 등의 특수 문자)은 Linked List로 서로 연결되어 있으며 그 List의 시작 컨트롤을 나타냅니다. @n
		 * 이 컨트롤부터 시작하여 Ctrl.Next로 순방향 탐색을 수행할 수 있습니다. @n
		 *
		 * @return ICtrlCode*
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_document_GetHeadCtrl 
		 */
		CtrlCode* GetHeadCtrl();

		/**
		 * @brief 문서 중 마지막 컨트롤입니다. (읽기 전용) @n
		 * 문서의 모든 컨트롤(표, 그림 등의 특수 문자)은 linked list로 서로 연결되어 있으며 그 list의 마지막 컨트롤을 나타냅니다. @n
		 * 이 컨트롤부터 시작하여 Ctrl.Prev로 역방향 탐색을 수행할 수 있습니다. @n
		 *
		 * @return ICtrlCode*
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_document_GetLastCtrl 
		 */
		CtrlCode* GetLastCtrl();

		/**
		 * @brief 현재 캐럿 위치의 상위 컨트롤입니다. (읽기 전용) @n
		 * 상위 컨트롤은 현재 캐럿이 위치한 리스트를 보유한 컨트롤입니다. @n
		 * 예를 들어 셀 내부에 위치하면 표, 각주 내용에 위치하면 각주, 바탕쪽이면 구역 컨트롤이 상위 컨트롤입니다. @n
		 * 현재 캐럿이 본문에 위치해 상위 컨트롤이 없을 때는 NULL이 반환됩니다. @n
		 * @return ICtrlCode*
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_document_GetParentCtrl 
		 */
		CtrlCode* GetParentCtrl();

		// Field API

		/**
		 * @brief 문서에 존재하는 필드의 목록을 가져옵니다.
		 *
		 * @param filelist 필드 목록 버퍼입니다.
		 * @param filelistsize 필드 목록 길이입니다.
		 * @param number 문서 내에서 동일한 이름의 필드가 여러 개 존재할 경우 이를 구별하기 위한 식별 방법을 지정합니다. 생략하면 hwpFieldPlain이 지정됩니다.
		 * ID|값|설명
		 * -|-|-
		 * hwpFieldPlain|0|아무 기호 없이 순서대로 필드의 이름을 나열합니다.
		 * hwpFieldNumber|1|필드 이름 뒤에 {{#}} 형식의 일련 번호를 추가합니다.
		 * hwpFieldCount|2|필드 이름 뒤에 {{#}} 형식의 필드 개수를 추가합니다.
		 * @param option 다음과 같은 옵션을 조합할 수 있습니다. 0을 지정하면 어떤 옵션도 적용되지 않으며 생략하면 0이 지정됩니다.
		 * ID|값|설명
		 * -|-|-
		 * hwpFieldCell|0x01|셀에 부여된 필드 목록만 가져옵니다. hwpFieldClickHere와 함께 지정할 수 없습니다.
		 * hwpFieldClickHere|0x02|누름틀에 부여된 필드 목록만 가져옵니다. hwpFieldCell과 함께 지정할 수 없습니다.
		 * HwpFieldSelection|0x04|선택된 내용 안에 존재하는 필드 목록을 가져옵니다.
		 *
		 * @return int 필드 목록의 길이
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_document_GetFieldList
		 */
		int GetFieldList(char16_t* filelist, int filelistsize, int number = 0, int option = 0);

		/**
		* @brief 문서에 지정된 데이터 필드가 존재하는지 검사합니다.
		*
		* @param field 필드 이름입니다.
		* @return true
		* @return false
		* 
		* **Examples**
		 * \snippet snippets.cpp example_document_FieldExist
		 */
		bool FieldExist(const char16_t* field);

		/**
		 * @brief 지정한 필드로 캐럿을 이동합니다.
		 *
		 * @param field 필드 이름입니다. GetFieldText()/PutFieldText()와 같은 형식으로 이름 뒤에 ‘{{#}}’로 번호를 지정할 수 있습니다.
		 * @param text 필드가 누름틀일 경우 누름틀 내부의 텍스트로 이동할지(True) 누름틀 코드로 이동할지(False) 지정합니다. 누름틀이 아닌 필드일 경우 무시되며 생략하면 True가 지정됩니다.
		 * @param start 필드의 처음으로 이동할지(True) 끝으로 이동할지(False) 지정합니다. select를 True로 지정하면 무시되며 생략하면 True가 지정됩니다.
		 * @param select 필드 내용을 블록으로 선택할지(True) 캐럿만 이동할지(False) 지정합니다. 생략하면 False가 지정됩니다.
		 * @return true
		 * @return false
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_document_MoveToField
		 */
		bool MoveToField(const char16_t* field, bool text = true, bool start = true, bool select = false);
		
		/**
		 * @brief 지정한 필드에서 문자열을 가져옵니다.
		 *
		 * @param fieldlist 텍스트를 가져올 필드 이름의 목록입니다. 다음과 같이 필드 사이를 문자 코드 0x02로 구분하여 한 번에 여러 개의 필드를 지정할 수 있습니다. @n
		 * [“필드이름#1\x2필드이름#2\x2...필드이름#n"] @n
		 * 문서에서 지정한 필드 이름이 두 개 이상 있을 때 표현 방식은 다음과 같습니다. @n
		 * 항목|설명
		 * -|-
		 * 필드 이름|지정한 이름의 필드 중 첫 번째
		 * 필드 이름{{n}}|지정한 이름의 필드 중 n 번째
		 * 예를 들어 "제목{{1}}\x2본문\x2이름{{0}}"과 같이 지정하면 ‘제목’이라는 이름의 필드 중 두 번째, ‘본문’이라는 이름의 필드 중 첫 번째, ‘이름’이라는 이름의 필드 중 첫 번째를 각각 지정합니다. 즉, ‘필드이름’과 ‘필드이름0 ’은 동일한 의미로 해석됩니다.
		 * @param fieldtext
		 * @param fieldtextsize
		 * @return int
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_document_GetFieldText
		 */
		int	GetFieldText(const char16_t* fieldlist, char16_t* fieldtext, int fieldtextsize);

		/**
		 * @brief 지정한 필드의 내용을 채웁니다.
		 *
		 * @param fieldlist 내용을 채울 필드 이름의 목록입니다. 한 번에 여러 개의 필드를 지정할 수 있으며 형식은 GetFieldText와 동일합니다. 다만 필드 이름 뒤에 ‘# ’로 번호를 지정하지 않으면 해당 이름을 가진 모든 필드에 동일한 텍스트가 추가됩니다. 즉, PutFieldText에서는 ‘필드이름’과 ‘필드이름0 ’의 의미가 다릅니다.
		 * @param textlist 필드에 채워 넣을 문자열의 목록입니다. 형식은 필드 목록과 동일하며 필드 개수 만큼 0x02로 구분하여 텍스트를 지정합니다.
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_document_PutFieldText
		 */
		void PutFieldText(const char16_t* fieldlist, const char16_t* textlist);

		/**
		 * @brief 지정한 필드의 이름을 바꿉니다.
		 *
		 * @param oldname 이름을 바꿀 필드의 목록이며 형식은 PutFieldText와 동일합니다.
		 * @param newname 새로운 필드 이름의 목록입니다. oldname과 동일한 개수의 필드 이름을 0x02로 구분하여 지정합니다.
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_document_RenameField
		 */
		void RenameField(const char16_t* oldname, const char16_t* newname);

		/**
		 * @brief 캐럿의 현재 위치에 누름틀을 생성합니다.
		 *
		 * @param direction 내용이 입력되지 않은 누름틀에 표시할 안내문입니다.
		 * @param memo 누름틀에 대한 설명/도움말입니다.
		 * @param name 누름틀에 대한 필드 이름입니다.
		 * @return true
		 * @return false
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_document_CreateField
		 */
		bool CreateField(const char16_t* direction, const char16_t* memo = nullptr, const char16_t* name = nullptr);

		/**
		 * @brief 현재 캐럿이 위치하는 곳의 필드 이름을 가져옵니다.
		 *
		 * @param option 다음과 같은 옵션을 지정할 수 있습니다. 0을 지정하면 어떤 옵션도 적용되지 않으며 생략하면 0이 지정됩니다.
		 * ID|값|설명
		 * -|-|-
		 * hwpFieldCell|1|셀에 부여된 필드 목록만 가져옵니다. hwpFieldClickHere와 함께 지정할 수 없습니다.
		 * hwpFieldClickHere|2|누름틀에 부여된 필드 목록만 가져옵니다. hwpFieldCell과 함께 지정할 수 없습니다.
		 * @param fieldname
		 * @param fieldnamesize
		 * @return int
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_document_GetCurFieldName
		 */
		int GetCurFieldName(int option, char16_t* fieldname, int fieldnamesize);

		/**
		 * @brief 현재 캐럿이 위치하는 곳의 필드 이름을 설정합니다.
		 *
		 * @param fieldname 데이터 필드 이름입니다.
		 * @param option 다음과 같은 옵션을 지정할 수 있습니다. 0을 지정하면 어떤 옵션도 적용되지 않으며 생략하면 0이 지정됩니다.
		 * ID|값|설명
		 * -|-|-
		 * hwpFieldCell|1|셀에 부여된 필드 목록만 가져옵니다. hwpFieldClickHere와 함께 지정할 수 없습니다.
		 * hwpFieldClickHere|2|누름틀에 부여된 필드 목록만 가져옵니다. hwpFieldCell과 함께 지정할 수 없습니다.
		 * @param direction 누름틀 필드의 안내문이며 누름틀 필드일 때만 유효합니다.
		 * @param memo 누름틀 필드의 메모이며 누름틀 필드일 때만 유효합니다.
		 * @return true
		 * @return false
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_document_SetCurFieldName
		 */
		bool SetCurFieldName(const char16_t* fieldname, int option = 0, const char16_t* direction = nullptr, const char16_t* memo = nullptr);
		
		/**
		 * @brief 캐럿이 위치한 필드의 상태 정보를 가져옵니다. (읽기 전용)
		 *
		 * @return int 반환되는 내용은 일종의 flag 값으로 비트가 Set되어 있으면 각 값의 의미는 다음과 같습니다.
		 * 값|설명
		 * -|-
		 * BIT 0|현재 필드는 셀
		 * BIT 1|현재 필드는 누름틀
		 * BIT 4|필드명이 존재
		 * 나머지|예약
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_document_GetCurFieldState
		 */
		int GetCurFieldState();

		/**
		 * @brief 지정한 필드의 속성을 변경합니다.
		 *
		 * @param field 속성을 변경할 필드 이름의 목록이며 형식은 PutFieldText와 동일합니다.
		 * @param remove 제거할 속성입니다.
		 * @param add 추가할 속성입니다.
		 * @return int
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_document_ModifyFieldProperties
		 */
		int ModifyFieldProperties(const char16_t* field, int remove, int add);

		/**
		 * @brief 양식 모드와 읽기 전용 모드일 때 현재 열린 문서에 포함된 필드의 보기 속성(『』표시)을 변경합니다.
		 *
		 * @param option 보기 속성 bit
		 * option|누름틀|개인 정보/문서 요약/날짜 시간/파일 경로|비고
		 * -|-|-|-
		 * 1|『』을 표시하지 않음|『』을 표시하지 않음|
		 * 2|『』을 빨간색으로 표시|『』을 흰색으로 표시|설정하지 않았을 때 기본 값
		 * 3|『』을 흰색으로 표시|『』을 흰색으로 표시|
		 * @return int
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_document_SetFieldViewOption
		 */
		int SetFieldViewOption(int option);

		// Metatag API

		/**
		 * @brief 문서의 메타 태그 목록을 가져오며 각 태그는 0x02를 사용하여 구분합니다.
		 *
		 * @param metataglist 문자열 버퍼입니다.
		 * @param metataglistsize 버퍼 크기입니다.
		 * @param number 문서 내에서 동일한 이름의 필드가 여러 개 존재할 경우 이를 구별하기 위한 식별 방법을 지정합니다. 생략하면 hwpFieldPlain이 지정됩니다.
		 * ID|값|설명
		 * -|-|-
		 * hwpFieldPlain|0|아무 기호 없이 순서대로 필드의 이름을 나열합니다.
		 * hwpFieldNumber|1|필드 이름 뒤에 일련 번호가 {{#}}과 같은 형식으로 추가됩니다.
		 * hwpFieldCount|2|필드 이름 뒤에 그 필드의 개수가 {{#}}과 같은 형식으로 추가됩니다.
		 * @param option 다음과 같은 옵션을 조합할 수 있습니다. 0을 지정하면 어떤 옵션도 적용하지 않으며 생략하면 0이 지정됩니다.
		 * ID|값|설명
		 * -|-|-
		 * hwpFieldCell|0x01|셀에 부여된 필드 목록만 가져옵니다. hwpFieldClickHere과 함께 지정할 수 없습니다.
		 * hwpFieldClickHere|0x02|누름틀에 부여된 필드 목록만 가져옵니다. hwpFieldCell과 함께 지정할 수 없습니다.
		 * HwpFieldSelection|0x04|선택된 내용 안에 존재하는 필드 목록을 가져옵니다.
		 * @return int
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_document_GetMetatagList
		 */
		int GetMetatagList(int number, int option, char16_t* metataglist, int metataglistsize);

		/**
		 * @brief 메타 태그가 등록되어 있는지 확인합니다.
		 *
		 * @param tag 확인하려는 메타 태그 이름입니다.
		 * @return TRUE : 성공 / FALSE : 실패
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_document_MetatagExist
		 */
		bool MetatagExist(const char16_t* tag);

		/**
		 * @brief 메타 태그로 이동합니다.
		 *
		 * @param tag 메타 태그 이름입니다.
		 * @param text TRUE/FALSE : 메타 태그 내부에 텍스트가 있는 경우 텍스트 안으로 이동할지 여부를 지정합니다.
		 * @param start TRUE/FALSE : 시작 메타 태그로 이동할지 마지막 메타 태그로 이동할지 여부를 지정합니다.
		 * @param select TRUE/FALSE  : 메타 태그를 선택한 상태로 이동할지 여부를 지정합니다.
		 * @return TRUE : 성공 / FALSE : 실패
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_document_MoveToMetatag
		 */
		bool MoveToMetatag(const char16_t* tag, bool text = true, bool start = true, bool select = false);

		/**
		 * @brief 입력한 이름의 메타 태그의 Text를 가져옵니다.
		 *
		 * @param tag 메타 태그 이름입니다.
		 * @param metatagnametext 가져올 Text입니다.
		 * @param metatagnametextsize 가져올 TextSize입니다.
		 * @return int
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_document_GetMetatagNameText
		 */
		int GetMetatagNameText(const char16_t* tag, char16_t* metatagnametext, int metatagnametextsize);

		/**
		 * @brief 입력한 이름의 메타 태그의 Text를 입력합니다.
		 *
		 * @param tag 메타 태그 이름입니다.
		 * @param text 메타 태그에 입력하려는 Text입니다.
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_document_PutMetatagNameText
		 */
		bool PutMetatagNameText(const char16_t* tag, const char16_t* text);

		/**
		 * @brief 현재 메타 태그의 이름을 설정합니다.
		 *
		 * @param tag 변경할 이름입니다.
		 * @return true
		 * @return false
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_document_SetCurMetatagName
		 */
		bool SetCurMetatagName(const char16_t* tag);

		/**
		 * @brief 현재 메타 태그의 이름을 가져옵니다.
		 *
		 * @param metatagname 텍스트 버퍼입니다.
		 * @param metatagnamesize 텍스트 버퍼 크기입니다.
		 * @return int
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_document_GetCurMetatagName
		 */
		int GetCurMetatagName(char16_t* metatagname, int metatagnamesize);

		/**
		 * @brief 메타 태그 이름을 변경합니다.
		 *
		 * @param oldtag 기존 메타 태그 이름입니다.
		 * @param newtag 변경 메타 태그 이름입니다.
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_document_RenameMetatag
		 */
		bool RenameMetatag(const char16_t* oldtag, const char16_t* newtag);

		/**
		 * @brief 메타 태그의 속성을 변경합니다.
		 *
		 * @param tag 메타 태그 이름입니다.
		 * @param remove 제거할 속성입니다.
		 * @param add 추가할 속성입니다.
		 * @return int
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_document_ModifyMetatagProperties
		 */
		int ModifyMetatagProperties(const char16_t* tag, int remove, int add);

		/**
		 * @brief 현재 메타 태그의 상태를 가져옵니다.
		 *
		 * @return int
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_document_GetCurMetatagState
		 */
		int GetCurMetatagState();

		/**
		 * @brief 사용자 지정 목록을 가져옵니다.
		 *
		 * @param option 옵션입니다.
		 * @param userpropertynamelist 텍스트 버퍼입니다.
		 * @param userpropertynamelistsize 버퍼 크기입니다.
		 * @return int 필요 버퍼 크기
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_document_GetUserProperty
		 */
		int GetUserProperty(const char16_t* name, int option, char16_t* userproperty, int userpropertysize);

		/**
		 * @brief 문서 정보의 사용자 지정 값을 입력한다.
		 *
		 * @param name 이름입니다.
		 * @param format 형식입니다.
		 * @param value 값입니다.
		 * @param option 옵션입니다.
		 * @return int
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_document_SetUserProperty
		 */
		int SetUserProperty(const char16_t* name, int format, const char16_t* value, int option);

		/**
		 * @brief 사용자 지정 목록을 가져옵니다.
		 *
		 * @param option 옵션입니다.
		 * @param userpropertynamelist 텍스트 버퍼입니다.
		 * @param userpropertynamelistsize 버퍼 크기입니다.
		 * @return int 필요 버퍼 크기
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_document_GetUserPropertyNameList
		 */
		int GetUserPropertyNameList(int option, char16_t* userpropertynamelist, int userpropertynamelistsize);

		// Document Position API

		/**
		 * @brief 캐럿의 위치를 이동합니다.
		 *
		 * @param moveID 다음과 같은 값을 지정할 수 있습니다. 생략하면 moveCurList가 지정됩니다.
		 * ID|값|설명
		 * -|-|-
		 * moveMain|0|루트 목록의 특정 위치(para pos로 위치 지정)
		 * moveCurList|1|현재 목록의 특정 위치(para pos로 위치 지정)
		 * moveTopOfFile|2|문서의 시작 지점으로 이동
		 * moveBottomOfFile|3|문서의 끝 지점으로 이동
		 * moveTopOfList|4|현재 목록의 시작 지점으로 이동
		 * moveBottomOfList|5|현재 목록의 끝 지점으로 이동
		 * moveStartOfPara|6|현재 위치한 문단의 시작 지점으로 이동
		 * moveEndOfPara|7|현재 위치한 문단의 끝 지점으로 이동
		 * moveStartOfWord|8|현재 위치한 단어의 시작 지점으로 이동(현재 목록만 대상으로 동작)
		 * moveEndOfWord|9|현재 위치한 단어의 끝 지점으로 이동(현재 목록만 대상으로 동작)
		 * moveNextPara|10|다음 문단의 시작 지점으로 이동(현재 목록만 대상으로 동작)
		 * movePrevPara|11|앞 문단의 끝 지점으로 이동(현재 목록만 대상으로 동작)
		 * moveNextPos|12|한 글자 뒤로 이동(하위 목록으로 이동 가능)
		 * movePrevPos|13|한 글자 앞으로 이동(하위 목록으로 이동 가능)
		 * moveNextPosEx|14|한 글자 뒤로 이동(하위 목록으로 이동 가능하며 머리말/꼬리말, 각주/미주, 글상자 포함)
		 * movePrevPosEx|15|한 글자 앞으로 이동(하위 목록으로 이동 가능하며 머리말/꼬리말, 각주/미주, 글상자 포함)
		 * moveNextChar|16|한 글자 뒤로 이동(현재 목록만 대상으로 동작)
		 * movePrevChar|17|한 글자 앞으로 이동(현재 목록만 대상으로 동작)
		 * moveNextWord|18|한 단어 뒤로 이동(현재 목록만 대상으로 동작)
		 * movePrevWord|19|한 단어 앞으로 이동(현재 목록만 대상으로 동작)
		 * moveNextLine|20|한 줄 아래로 이동
		 * movePrevLine|21|한 줄 위로 이동
		 * moveStartOfLine|22|현재 위치한 줄의 시작 지점으로 이동
		 * moveEndOfLine|23|현재 위치한 줄의 끝 지점으로 이동
		 * moveParentList|24|한 수준 위로 이동
		 * moveTopLevelList|25|최상위 목록으로 이동
		 * moveRootList|26|루트 목록으로 이동
		 * 현재 루트 목록에 위치해 있어 더 이상 상위 목록이 없을 때는 위치 이동 없이 값을 반환합니다. 이동한 후 위치는 상위 목록에서 하위 목록이 속한 컨트롤 코드가 위치한 곳입니다. 위치 이동 시 셀렉션은 무조건 해제됩니다."
		 * moveCurrentCaret|27|현재 캐럿이 위치한 곳으로 이동(캐럿 위치가 뷰의 맨 위쪽으로 올라감)
		 * ver:0x0507013B, 0x06000105"
		 * moveLeftOfCell|100|현재 캐럿이 위치한 셀의 왼쪽
		 * moveRightOfCell|101|현재 캐럿이 위치한 셀의 오른쪽
		 * moveUpOfCell|102|현재 캐럿이 위치한 셀의 위쪽
		 * moveDownOfCell|103|현재 캐럿이 위치한 셀의 아래쪽
		 * moveStartOfCell|104|현재 캐럿이 위치한 셀에서 행(row)의 시작
		 * moveEndOfCell|105|현재 캐럿이 위치한 셀에서 행(row)의 끝
		 * moveTopOfCell|106|현재 캐럿이 위치한 셀에서 열(column)의 시작
		 * moveBottomOfCell|107|현재 캐럿이 위치한 셀에서 열(column)의 끝
		 * moveScrPos|200|한/글 문서 창에서 스크린 좌표로서 위치를 설정
		 * moveScanPos|201|GetText() 실행 후 위치로 이동
		 * @param para 이동할 문단의 번호입니다. @n
		 * moveMain 또는 moveCurList가 지정되었을 때만 사용됩니다. moveScrPos가 지정되었을 때는 문단 번호가 아닌 스크린 좌표로 해석됩니다. @n
		 * (스크린 좌표 : LOWORD = x 좌표, HIWORD = y 좌표) @n
		 * @param pos 이동할 문단 내 문자의 위치입니다. @n
		 * moveMain 또는 moveCurList가 지정되었을 때만 사용됩니다.
		 * @return true
		 * @return false
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_document_MovePos
		 */
		bool MovePos(int moveID = moveCurList, int para = 0, int pos = 0);
		
		/**
		 * @brief 캐럿의 위치를 가져옵니다.
		 *
		 * @param list 캐럿이 위치한 문서 내 list ID입니다.
		 * @param para 캐럿이 위치한 문단 ID입니다.
		 * @param pos 캐럿이 위치한 문단 내 글자의 위치입니다.
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_document_GetPos
		 */
		void GetPos(int FAR* list, int FAR* para, int FAR* pos);

		/**
		 * @brief 캐럿을 문서 내 특정 위치로 이동합니다.
		 *
		 * @param list 캐럿이 위치한 문서 내 list ID입니다.
		 * @param para 캐럿이 위치한 문단 ID입니다.
		 * @param pos 캐럿이 위치한 문단 내 글자의 위치입니다.
		 * @return true
		 * @return false
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_document_SetPos
		 */
		bool SetPos(int list, int para, int pos);

		/**
		 * @brief 현재 캐럿의 위치 정보를 ParameterSet으로 가져옵니다.
		 *
		 * @return IParameterSet* 캐럿 위치에 대한 ParameterSet
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_document_GetPosBySet 
		 */
		ParameterSet* GetPosBySet();

		/**
		 * @brief ParameterSet이 가져오는 위치로 캐럿을 이동합니다.
		 *
		 * @param pos 캐럿을 이동할 위치에 대한 ParameterSet 정보입니다.
		 * @return true
		 * @return false
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_document_SetPosBySet
		 */
		bool SetPosBySet(ParameterSet* pos);

		/**
		 * @brief 블록을 설정합니다.
		 *
		 * @param spara 블록 시작 지점의 문단 번호입니다.
		 * @param spos 블록 시작 지점의 문단 내 문자의 위치입니다.
		 * @param epara 블록 끝 지점의 문단 번호입니다.
		 * @param epos 블록 끝 지점의 문단 내 문자의 위치입니다.
		 * @return true
		 * @return false
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_document_SelectText
		 */
		bool SelectText(int spara, int spos, int epara, int epos);

		/**
		 * @brief 블록의 텍스트를 가져옵니다.
		 *
		 * @param text
		 * @param textSize
		 * @return
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_document_GetSelectText
		 */
		int GetSelectText(char16_t* text, int textSize);

		/**
		 * @brief 현재 설정된 블록의 위치 정보를 가져옵니다.
		 *
		 * @param slist 설정된 블록의 시작 목록 아이디입니다.
		 * @param spara 설정된 블록의 시작 문단 아이디입니다.
		 * @param spos 설정된 블록의 문단 내 시작 글자의 위치를 나타내는 숫자입니다.
		 * @param elist 설정된 블록의 끝 목록 아이디입니다.
		 * @param epara 설정된 블록의 끝 문단 아이디입니다.
		 * @param epos 설정된 블록의 문단 내 끝 글자의 위치를 나타내는 숫자입니다.
		 * @return true
		 * @return false
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_document_GetSelectedPos
		 */
		bool GetSelectedPos(int* slist, int* spara, int* spos, int* elist, int* epara, int* epos);

		/**
		 * @brief 현재 설정된 블록의 위치 정보를 가져옵니다. (GetSelectedPos의 ParameterSet 버전)
		 *
		 * @param sset 설정된 블록의 시작 매개 변수 집합(ListParaPos)
		 * @param eset 설정된 블록의 끝 매개 변수 집합(ListParaPos)
		 * @return true
		 * @return false
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_document_GetSelectedPosBySet
		 */
		bool GetSelectedPosBySet(ParameterSet* sset, ParameterSet* eset);

		// Document Find API

		/**
		 * @brief InitScan()으로 설정된 초기화 정보를 해제합니다.
		 *
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_document_ReleaseScan
		 */
		void ReleaseScan();

		/**
		 * @brief 문서의 내용을 검색하기 위해 초기 설정을 수행합니다.
		 *
		 * @param option 찾을 대상을 다음과 같은 옵션을 조합하여 지정할 수 있습니다. 생략하면 모든 컨트롤을 찾을 대상으로 합니다.
		 * ID			|값		|설명
		 * -------------|-------|------
		 * maskNormal	|0x00	|본문을 대상으로 검색(하위 목록은 검색하지 않음)
		 * maskChar		|0x01	|char 타입 컨트롤 마스크를 대상으로 검색(강제 줄 나눔, 문단 끝, 하이픈, 묶움 빈칸, 고정 폭 빈칸 등)
		 * maskInline	|0x02	|inline 타입 컨트롤 마스크를 대상으로 검색(누름틀 필드 끝 등)
		 * maskCtrl		|0x04	|extende 타입 컨트롤 마스크를 대상으로 검색(바탕쪽, 프레젠테이션, 다단, 누름틀 필드 시작, Shape Object, 머리말, 꼬리말, 각주, 미주, 번호 관련 컨트롤, 새 번호 관련 컨트롤, 감추기, 찾아보기, 글자 겹침 등)
		 * @param range 검색의 범위를 다음과 같은 옵션을 조합하여 지정할 수 있습니다. @n
		 * 생략하면 “문서 시작부터 - 문서의 끝까지” 검색 범위가 지정됩니다.
		 * ID|값|설명
		 * -|-|-
		 * scanSposCurrent|0x0000|캐럿 위치부터(시작 위치)
		 * scanSposSpecified|0x0010|특정 위치부터(시작 위치)
		 * scanSposLine|0x0020|줄의 시작부터(시작 위치)
		 * scanSposParagraph|0x0030|문단의 시작부터(시작 위치)
		 * scanSposSection|0x0040|구역의 시작부터(시작 위치)
		 * scanSposList|0x0050|리스트의 시작부터(시작 위치)
		 * scanSposControl|0x0060|컨트롤의 시작부터(시작 위치)
		 * scanSposDocument|0x0070|문서의 시작부터(시작 위치)
		 * scanEposCurrent|0x0000|캐럿 위치까지(끝 위치)
		 * scanEposSpecified|0x0001|특정 위치까지(끝 위치)
		 * scanEposLine|0x0002|줄의 끝까지(끝 위치)
		 * scanEposParagraph|0x0003|문단의 끝까지(끝 위치)
		 * scanEposSection|0x0004|구역의 끝까지(끝 위치)
		 * scanEposList|0x0005|리스트의 끝까지(끝 위치)
		 * scanEposControl|0x0006|컨트롤의 끝까지(끝 위치)
		 * scanEposDocument|0x0007|문서의 끝까지(끝 위치)
		 * scanWithinSelection|0x00ff|검색의 범위를 블록으로 제한
		 * scanForward|0x0000|정뱡향(검색 방향)
		 * scanBackward|0x0100|역방향(검색 방향)
		 * @param spara 검색 시작 지점의 문단 번호입니다. scanSposSpecified 옵션이 지정되었을 때만 유효합니다.
		 * @param spos 검색 시작 지점의 문단 내 문자의 위치입니다. scanSposSpecified 옵션이 지정되었을 때만 유효합니다.
		 * @param epara 검색 끝 지점의 문단 번호입니다. scanEposSpecified 옵션이 지정되었을 때만 유효합니다.
		 * @param epos 검색 끝 지점의 문단 내 문자의 위치입니다. scanEposSpecified 옵션이 지정되었을 때만 유효합니다.
		 * @return true
		 * @return false
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_document_InitScan
		 */
		bool InitScan(int option = 0, int range = 0x00000070 | 0x00000007, int spara = 0, int spos = 0, int epara = 0, int epos = 0);

		// Document Content API 
		
		/**
		 * @brief 현재 캐럿 위치에 문서를 삽입합니다.
		 *
		 * @param path 문서의 경로이며 URL을 사용할 수 있습니다. (ver:0x05050111)
		 * @param format 문서 형식이며 별도 설명 참조하세요. 빈 문자열을 지정하면 자동으로 선택되며 생략하면 빈 문자열이 지정됩니다.
		 * @param arg 세부 옵션으로 format에 지정한 파일 형식에 따라 다릅니다. 생략하면 빈 문자열이 지정됩니다.
		 * @return true
		 * @return false
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_document_Insert
		 */
		bool Insert(const char16_t* path, const char16_t* format = nullptr, const char16_t* arg = nullptr);
		
		/**
		 * @brief 현재 캐럿 위치에 컨트롤을 삽입합니다. @n
		 * @n
		 * ctrlid에 지정할 수 있는 컨트롤 ID는 HwpCtrl.CtrlID가 반환하는 ID와 동일합니다. 자세한 내용은 Ctrl 오브젝트 Properties인 CtrlID를 참조하세요. @n
		 * initparam에는 컨트롤의 초기 속성을 지정합니다. 대부분의 컨트롤은 Ctrl.Properties와 동일한 형식의 parameter set을 사용하지만, 컨트롤 생성 시에는 다른 형식을 사용하는 경우도 있습니다. 예를 들어 표의 경우 Ctrl.Properties에는 "Table" 세트를 사용하지만, 생성 시 initparam에 지정하는 값은 "TableCreation" 세트입니다.
		 * @param ctrlid 삽입할 컨트롤 ID입니다.
		 * @param set 컨트롤 초기 속성이며 생략하면 default 속성으로 생성합니다.
		 * @return ICtrlCode* 생성된 컨트롤 object
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_document_GetPosBySet 
		 */
		CtrlCode* InsertCtrl(const char16_t* ctrlid, ParameterSet* pset = nullptr);
		
		/**
		 * @brief 현재 캐럿 위치에 컨트롤을 삽입합니다.
		 *
		 * @param dataurl 웹 Data-Url 형식 또는 base64 인코딩 데이터입니다.
		 * @param extractrlid 사용자 정의 컨트롤 id입니다. (meta tag에 삽입)
		 * @param extractrldata 사용자 정의 컨트롤의 data입니다. (meta tag에 삽입)
		 * @param width 삽입할 개체의 너비입니다.
		 * @param height 삽입할 개체의 높이입니다.
		 * @return ICtrlCode* 생성된 컨트롤 object
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_document_InsertExtraCtrl 
		 */
		CtrlCode* InsertExtraCtrl(const char16_t* dataurl, const char16_t* extractrlid, const char16_t* extractrldata);
		
		/**
		 * @brief 문서 내 컨트롤을 삭제합니다.
		 *
		 * @param ctrl 삭제할 문서 내 컨트롤입니다.
		 * @return true 성공
		 * @return false 실패
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_document_DeleteCtrl
		 */
		bool DeleteCtrl(CtrlCode* ctrl);

		/**
		 * @brief 컨트롤을 선택합니다.
		 *
		 * @param ctrllist 선택하려는 컨트롤의 instid입니다.
		 * @param option  1: 기존 선택된 컨트롤을 취소하지 않습니다.
		 * @return true
		 * @return false
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_document_SelectCtrl
		 */
		bool SelectCtrl(const char16_t* ctrllist, int option);
		
		/**
		 * @brief 선택된 컨트롤을 취소합니다.
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_document_UnSelectCtrl
		 */
		bool UnSelectCtrl();
		
		//@todo MoveToCtrlList


		/**
		 * @brief 현재 캐럿의 위치에 그림을 삽입합니다.
		 *
		 * @param path 삽입할 이미지 파일이며 URL을 사용할 수 있습니다. (ver:0x05050111)
		 * @param embedded 이미지 파일을 문서 내에 포함할지 여부(True/False)를 지정합니다. 생략하면 True가 지정됩니다.
		 * @param sizeoption 삽입할 그림의 크기를 지정하는 옵션입니다.
		 *  ID|값|설명
		 * -|-|-
		 * realSize|0|이미지 원래의 크기로 삽입합니다. width와 height를 지정하지 않아도 됩니다.
		 * specificSize|1|width와 height에 지정한 크기로 그림을 삽입합니다.
		 * cellSize|2|"현재 캐럿이 표의 셀 안에 있을 경우 셀의 크기에 맞게 자동 조절하여 삽입합니다.
		 * width는 셀의 width만큼, height는 셀의 height만큼 확대 또는 축소됩니다.
		 * 캐럿이 셀 안에 있지 않으면 이미지의 원래 크기대로 삽입됩니다."
		 * cellSizeWithSameRatio|3|현재 캐럿이 표의 셀 안에 있을 경우 셀의 크기에 맞게 원본 이미지의 가로 세로의 비율을 확대 또는 축소하여 삽입합니다.
		 * @param reverse 이미지의 반전 여부를 나타냅니다. (True/False)
		 * @param watermark watermark 효과 적용 여부를 나타냅니다. (True/False)
		 * @param effect 그림 효과입니다.
		 * ID|값|설명
		 * -|-|-
		 * RealPicture|0|컬러
		 * GrayScale|1|회색조
		 * BlackWhite|2|흑백
		 * @param width 그림의 가로 크기를 지정하며 단위는 mm입니다.
		 * @param height 그림의 높이 크기를 지정하며 단위는 mm입니다.
		 * @return ICtrlCode* 생성된 컨트롤 object(ver:0x05050102) @n 성공하면 TRUE, 실패하면 FALSE(ver:0x05050100)
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_document_InsertPicture 
		 */
		CtrlCode* InsertPicture(const char16_t* path, bool embedded = true, unsigned short sizeoption = 0, bool reverse = false, bool watermark = false, unsigned short effect = 0, int width = -1, int height = -1);
		
		/**
		 * @brief 배경 이미지를 넣습니다.
		 *
		 * @param bordertype 배경 유형을 지정합니다.
		 *  bordertype|설명|비고
		 * -|-|-
		 * "SelectedCell"|현재 선택된 표의 셀 배경을 변경합니다.|
		 * "SelectedCellDelete"|현재 선택된 표의 셀 배경을 지웁니다.|"반드시 셀이 선택되어 있어야 합니다. @n 커서만 위치하는 경우 동작하지 않습니다."
		 * @param path
		 * @param embedded
		 * @param filloption
		 * @param watermark
		 * @param effect
		 * @param brightness
		 * @param contrast
		 * @return true
		 * @return false
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_document_InsertBackgroundPicture
		 */
		bool InsertBackgroundPicture(const char16_t* bordertype, const char16_t* path, bool embedded = true, unsigned short filloption = 5, bool watermark = false, unsigned short effect = 0, unsigned short brightness = 0, unsigned short contrast = 0);
		
		/**
		 * @brief 문서를 문자열로 지정합니다. @n
		 * @n
		 * 이 함수는 JScript나 VBScript와 같이 직접적으로 local disk에 접근하기 힘든 언어를 위한 것이므로 disk에 접근할 수 있는 언어에서는 사용하지 않는 것이 좋습니다. @n
		 * disk에 접근할 수 있다면 Open이나 Insert를 사용하세요. @n
		 * 이 함수는 내부적으로 Open이나 Insert를 호출하므로 텍스트로 저장된 파일이 메모리에서 3~4번 복사됩니다. 이에 따라 속도가 느리고 메모리가 낭비될 수 있습니다. @n
		 * @param data 문자열로 변경된 텍스트 파일입니다.
		 * @param format 파일의 형식입니다.
		 *  format|설명|비고
		 * -|-|-
		 * HWP|HWP native format|BASE64로 인코딩되어 있어야 합니다. 저장된 내용을 다른 곳에서 보여줄 필요가 없다면 이 형식을 사용할 것을 권장합니다.ver:0x0505010B
		 * HWPML2X|HWP 형식과 호환|문서의 모든 정보를 유지합니다.
		 * HTML|인터넷 문서 HTML 형식|한글 고유의 서식은 손실됩니다.
		 * UNICODE|유니코드 텍스트|서식 정보가 없는 텍스트만 저장합니다.
		 * TEXT|일반 텍스트|유니코드에만 있는 정보(한자, 고어, 특수 문자 등)는 모두 손실됩니다.
		 * @param option
		 *  option|설명|비고
		 * -|-|-
		 * insertfile|현재 커서 이후에 지정된 파일 삽입|
		 * @return bool
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_document_SetTextFile
		 */
		int SetTextFile(const char16_t* data, const char16_t* format, const char16_t* option);

		/**
		 * @brief 문서 내에서 텍스트를 가져옵니다. @n
		 * @n
		 * GetText()의 사용이 끝나면 ReleaseScan()을 반드시 호출하여 관련 정보를 초기화해야 합니다. @n
		 * 텍스트가 있는 문단으로 캐럿을 이동하려면 moveScanPos를 통해 MovePos()를 호출하면 됩니다. @n
		 * 매개 변수로 포인터를 사용하므로 포인터를 사용할 수 없는 언어에서는 사용할 수 없습니다. @n
		 * 포인터를 사용하지 않는 언어를 지원하기 위해서 ParameterSet을 사용하는 GetTextBySet()가 존재합니다. @n
		 *
		 * @param text 텍스트 데이터를 반환합니다. 텍스트에서 탭은 '\t'(0x9), 문단 바뀜은 CR/LF(0x0D/0x0A)로 표현되며, 이외의 특수 코드는 포함되지 않습니다.
		 * @param textsize
		 * @param result 값이 있으면 text scan의 결과를 반환하며 결과 값은 아래와 같습니다. @n
		 * 값|설명
		 * -|-
		 * 0|텍스트 정보 없음
		 * 1|목록의 끝
		 * 2|일반 텍스트
		 * 3|다음 문단
		 * 4|제어 문자 내부로 들어감
		 * 5|제어 문자를 빠져 나옴
		 * 101|초기화 안 됨 (InitScan() 실패 또는 InitScan()을 실행하지 않은 경우)
		 * 102|텍스트 변환 실패
		 * @return int text의 길이
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_document_GetText
		 */
		int GetText(char16_t* text, int textsize);

		/**
		 * @brief 문서 내에서 텍스트를 가져옵니다. (GetText의 ParameterSet 버전)
		 *
		 * @param text Item의 “Text"에 텍스트 데이터가 반환됩니다. 텍스트에서 탭은 '\t'(0x9), 문단 바뀜은 CR/LF(0x0D/0x0A)로 표현되며, 이외의 특수 코드는 포함되지 않습니다.
		 * @return int 다음과 같은 결과 값을 반환합니다.
		  * 값|설명
		 * -|-
		 * 0|텍스트 정보 없음
		 * 1|목록의 끝
		 * 2|일반 텍스트
		 * 3|다음 문단
		 * 4|제어 문자 내부로 들어감
		 * 5|제어 문자를 빠져 나옴
		 * 101|초기화 안 됨 (InitScan() 실패 또는 InitScan()을 실행하지 않은 경우)
		 * 102|텍스트 변환 실패
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_document_GetTextBySet
		 */
		int GetTextBySet(ParameterSet* text);

		/**
		 * @brief Binary Data의 경로를 가져옵니다.
		 *
		 * @param binid 바이너리 데이터 id 값입니다.
		 * @param path 경로 데이터를 가져올 문자열 버퍼입니다.
		 * @param pathsize 경로 데이터를 가져올 문자열 버퍼의 크기입니다.
		 * @return true 경로를 가져옴
		 * @return false
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_document_GetBinDataPath
		 */
		bool GetBinDataPath(int binid, char16_t* path, int pathsize);

		/**
		 * @brief Binary Data를 생성합니다.
		 *
		 * @param path 경로 데이터 문자열 버퍼입니다.
		 * @return bindata id
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_document_@
		 */
		// @todo CreateBinData

		/**
		 * @brief 현재 위치의 양식 개체의 특성 값을 가져옵니다.
		 *
		 * @param formname 양식 개체의 이름입니다.
		 * @return IParameterSet 양식 개체 속성 parameter set
		 * @return null 실패
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_document_GetFormObjectAttr 
		 */
		ParameterSet* GetFormObjectAttr(const char16_t* formname);

		/**
		 * @brief 페이지 단위로 텍스트를 추출합니다.
		 *
		 * @param page 추출할 페이지 번호입니다.
		 * @param option 추출 옵션을 다음과 같이 지정할 수 있습니다. @n
		 *	0x00 | 본문 텍스트를 추출(기본)
		 *	0x01 | 표 텍스트를 추출
		 *	0x02 | 글상자 텍스트를 추출
		 *	0x04 | 캡션 텍스트를 추출
		 * @param text 추출된 텍스트를 저장할 버퍼입니다.
		 * @param textSize 추출된 텍스트를 저장할 버퍼 크기입니다.
		 * @return int 추출된 텍스트 길이
		 * @return -1 실패
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_document_GetPageText
		 */
		int GetPageText(int page, int option, char16_t* text, int textSize);

		/**
		 * @brief 바코드 이미지를 삽입합니다.
		 *
		 * @param path 바코드 이미지 경로입니다.
		 * @param pgno 바코드 이미지를 삽입할 페이지 번호(0부터 시작)입니다.
		 * @param index 페이지에 삽입될 바코드 이미지의 번호(0부터 시작)입니다. @n
		 *	같은 번호를 지정하면 이전에 삽입된 바코드의 이미지가 수정됩니다. @n
		 *	바코드 이미지를 공백으로 지정하면 해당 번호의 바코드 이미지가 삭제됩니다.
		 * @param x 바코드 이미지가 삽입될 위치의 x 좌표(mm단위)입니다. @n
		 *	바코드 이미지의 좌측 상단을 기준으로 페이지(종이 기준) 좌측 상단에서의 거리입니다.
		 * @param y 바코드 이미지가 삽입 될 위치의 y 좌표(mm단위)입니다. @n
		 *	바코드 이미지의 좌측 상단 기준으로 페이지(종이 기준) 좌측 상단에서의 거리입니다.
		 * @param width 바코드 이미지의 너비입니다. @n
		 *	생략하면 원본 이미지의 너비가 적용됩니다.
		 * @param height 바코드 이미지의 높이입니다. @n
		 *	생략하면 원본 이미지의 높이가 적용됩니다.
		 * @return true 성공
		 * @return false 실패
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_document_SetBarCodeImage
		 */
		bool SetBarCodeImage(const char16_t* path, int pgno, int index, int x, int y, int width = 0, int height = 0);

		/**
		 * @brief 양식 개체의 특성 값을 설정합니다.
		 *
		 * @param formname 양식 개체 이름입니다.
		 * @param pset 설정할 양식 개체의 속성 이름과 값의 세트입니다.
		 * @return true 성공
		 * @return false 실패
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_document_SetFormObjectAttr
		 */
		bool SetFormObjectAttr(const char16_t* formname, ParameterSet* pset);

		// Application API 

		/**
		 * @brief 지정한 페이지를 이미지 파일로 생성합니다.
		 *
		 * @param path 생성할 이미지 파일의 경로입니다.
		 * @param pgno 페이지 번호이며 0부터 PageCount-1까지 지정합니다. 생략하면 0이 사용됩니다.
		 * @param resolution 이미지 해상도이며 DPI(96, 300, 1200 등)로 지정합니다. 생략하면 96이 사용됩니다.
		 * @param depth 이미지 파일의 Color Depth(1, 4, 8, 24)를 지정합니다.
		 * @param format 이미지 파일의 형식으로 “bmp", "gif", "jpeg", "png" 중 하나를 지정합니다. 생략하면 "bmp"가 사용됩니다.
		 * @return true
		 * @return false
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_document_CreatePageImage
		 */
		bool CreatePageImage(const char16_t* path, int pgno = 0, short resolution = 96, short depth = 24, const char16_t* format = nullptr, unsigned int options = 0);

		/**
		 * @brief 문서를 인쇄합니다.
		 *
		 * @return true
		 * @return false
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_document_PrintDocument
		 */
		bool PrintDocument();

		/**
		 * @brief 한글에서 사용 가능한 필터(형식)의 목록을 가져옵니다. @n
		 * BSTR 값으로 필터 목록을 가져오지만 내부적으로 Cstring 값을 사용하므로 UniCode가 아닌 ANSI Character로 반환됩니다. @n
		 * 매개 변수로 포인터를 사용하므로 스크립트 언어(javascript, visual basic)에서 사용할 수 없습니다.
		 *
		 * @param flags
		 * @param filterlist
		 * @param filterlistsize
		 * @return int
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_document_GetFilterList
		 */
		int GetFilterList(const unsigned int flags, char16_t* filterlist, int filterlistsize);

		/**
		 * @brief 현재 열린 문서를 문자열로 반환합니다. @n
		 * @n
		 * 이 함수는 JScript나 VBScript와 같이 직접적으로 local disk에 접근하기 힘든 언어를 위한 것이므로 disk를 접근할 수 있는 언어에서는 사용하지 않는 것이 좋습니다. @n
		 * disk를 접근할 수 있다면 Save나 SaveBlockAction을 사용하세요. @n
		 * 이 함수는 내부적으로 save나 SaveBlockAction을 호출하므로 텍스트로 저장된 파일이 메모리에서 3~4번 복사됩니다. 이에 따라 속도가 느리고 메모리가 낭비될 수 있습니다. @n
		 * @param format 파일의 형식입니다.
		 *  format|설명|비고
		 * -|-|-
		 * HWP|HWP native format|BASE64로 인코딩되어 있습니다. 저장된 내용을 다른 곳에서 보여줄 필요가 없다면 이 형식을 사용할 것을 권장합니다.ver:0x0505010B
		 * HWPML2X|HWP 형식과 호환|문서의 모든 정보를 유지합니다.
		 * HTML|인터넷 문서 HTML 형식|한글 고유의 서식은 손실됩니다.
		 * UNICODE|유니코드 텍스트|서식 정보가 없는 텍스트만 저장합니다.
		 * TEXT|일반 텍스트|유니코드에만 있는 정보(한자, 고어, 특수 문자 등)는 모두 손실됩니다.
		 * @param option
		 *  option|설명|비고
		 * -|-|-
		 * saveblock|선택된 블록만 저장|개체 선택 상태에서는 동작하지 않습니다.
		 * @param textfile
		 * @param textfilesize
		 * @return int 지정된 형식에 맞춰 파일을 문자열로 변환한 값을 반환합니다.
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_document_GetTextFile
		 */
		int GetTextFile(const char16_t* format, const char16_t* option, char16_t* textfile, int textfilesize);

		/**
		 * @brief 파일 정보를 가져옵니다.
		 *
		 * @param FileName 정보를 가져오려는 파일 이름입니다.
		 * @return IParameterSet*  “FileInfo" ParameterSet이 반환됩니다.
		 * ※ FileInfo
		 * Item ID|Type|설명
		 * -|-|-
		 * Format|string|"파일의 형식입니다. @n HWP: 한글 파일 @n UNKNOWN: 알 수 없음"
		 * VersionStr|string|"파일의 버전 문자열입니다. @n 예) 5.0.0.3" @n VersionNum|"unsigned
		 * int"|"파일의 버전입니다. @n 예) 0x05000003"
		 * Encrypted|int|"암호 여부 (현재는 3.0.0.0 버전 이상의 문서(한글97, 한글 워디안, 한글 2002)에 대해서만 판단합니다.) @n -1: 판단할 수 없음 @n  0: 암호가 설정되지 않음 @n 양수: 암호가 설정됨"
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_document_GetFileInfo 
		 */
		ParameterSet* GetFileInfo(const char16_t* FileName);

		/**
		 * @brief 현재 커서가 위치한 셀의 위치 정보를 가져옵니다.
		 *
		 * @param type 행 또는 열(0: 행, 1: 열)
		 * @return int 0~n까지 행 및 열의 위치 정보를 가져옵니다. @n 커서가 셀에 있지 않거나, 실패하면 -1을 반환합니다.
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_document_GetTableCellAddr
		 */
		int GetTableCellAddr(int type);

		/**
		 * @brief 현재 편집 중인 문서를 버전 정보 문서로 저장합니다.
		 *
		 * @param filepath 빈 문서일 경우 저장될 문서의 경로입니다. @n 빈 문서가 아닐 경우 현재 편집 중인 문서에 그대로 저장합니다.
		 * @param overwrite 최근 버전 정보 항목을 덮어 쓸지 여부를 결정합니다. @n 최근 버전의 지은이와 저장할 지은이가 같은 경우에만 덮어쓰고, 다른 이름이면 새 버전으로 저장합니다.
		 * @param infolock 버전 정보 내용 잠그기 여부를 결정합니다.
		 * @param writer 지은이 이름입니다.
		 * @param description 설명입니다.
		 * @return true
		 * @return false
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_document_VersionSave
		 */
		bool VersionSave(const char16_t* filepath, bool overwrite, bool infolock, const char16_t* writer = nullptr, const char16_t* description = nullptr);

		/**
		 * @brief 현재 문서의 버전 정보 항목을 지웁니다.
		 *
		 * @param index 삭제할 버전 정보 항목의 index(0~n)입니다. @n -1을 입력할 경우 현재 문서의 모든 버전 정보 항목을 삭제합니다.
		 * @return true
		 * @return false
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_document_VersionDelete
		 */
		bool VersionDelete(int index);

		/**
		 * @brief 현재 문서의 버전 비교 문서를 만듭니다.
		 *
		 * @param filepath 버전 비교 결과를 저장할 임시 파일 경로입니다. 생략할 경우 시스템의 임시 폴더에 문서가 임의로 생성됩니다.
		 * @return IParameterSet* VersionInfo ParameterSet을 반환 @n ※ VersionInfo: 버전 정보
		 *  Item ID|Type|설명
		 * -|-|-
		 * SourcePath|string|버전 비교용 원본 경로
		 * TargetPath|string|버전 비교용 대상 경로
		 * ItemStartIndex|byte|버전 비교를 보여줄 시작 히스토리 인덱스
		 * ItemEndIndex|byte|버전 비교를 보여줄 마지막 히스토리 인덱스
		 * ItemOverWrite|byte|히스토리 정보를 저장할 때 마지막 버전으로 덮어쓰는 플래그(on: 1/off: 0)
		 * ItemSaveDescription|byte|"히스토리 정보를 저장할 때 설명을 입력할 수 있는 대화 상자를 제공하는 플래그(on: 1/off: 0) @n 이전 버전과 다른 지은이 정보는 저장되지 않으므로 대화 상자를 생략할 경우 지은이 정보를 반드시 입력해야 합니다."
		 * TempFilePath|ParameterArray|버전 비교용 임시 파일 경로
		 * ItemInfoIndex|unsigned int|버전 정보 가져오기 및 삭제 시 사용할 인덱스
		 * SaveFilePath|string|버전 저장 파일 경로(OCX 컨트롤용)
		 * ItemInfoWriter|string|지은이 정보
		 * ItemInfoDescription|string|해당 버전에 대한 설명
		 * ItemInfoTimeHi|unsigned int|날짜 정보, FILETIME의 HIWORD
		 * ItemInfoTimeLo|unsigned int|날짜 정보, FILETIME의 LOWORD
		 * ItemInfoLock|byte|버전 정보 잠그기(on: 1/off:0)
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_document_MakeVersionDiffAll 
		 */
		ParameterSet* MakeVersionDiffAll(const char16_t* filepath);

		/**
		 * @brief 현재 문서의 버전 정보 항목을 가져옵니다.
		 *
		 * @param index 가져올 버전 정보 항목의 index입니다.
		 * @return IParameterSet* VersionInfo의 ParameterSet을 반환 @n ※ VersionInfo: 버전 정보
		 *  Item ID|Type|설명
		 * -|-|-
		 * SourcePath|string|버전 비교용 원본 경로
		 * TargetPath|string|버전 비교용 대상 경로
		 * ItemStartIndex|byte|버전 비교를 보여줄 시작 히스토리 인덱스
		 * ItemEndIndex|byte|버전 비교를 보여줄 마지막 히스토리 인덱스
		 * ItemOverWrite|byte|히스토리 정보를 저장할 때 마지막 버전으로 덮어쓰는 플래그(on: 1/off: 0)
		 * ItemSaveDescription|byte|"히스토리 정보를 저장할 때 설명을 입력할 수 있는 대화 상자를 제공하는 플래그(on: 1/off: 0) @n 이전 버전과 다른 지은이 정보는 저장되지 않으므로 대화 상자를 생략할 경우 지은이 정보를 반드시 입력해야 합니다."
		 * TempFilePath|ParameterArray|버전 비교용 임시 파일 경로
		 * ItemInfoIndex|unsigned int|버전 정보 가져오기 및 삭제 시 사용할 인덱스
		 * SaveFilePath|string|버전 저장 파일 경로(OCX 컨트롤용)
		 * ItemInfoWriter|string|지은이 정보
		 * ItemInfoDescription|string|해당 버전에 대한 설명
		 * ItemInfoTimeHi|unsigned int|날짜 정보, FILETIME의 HIWORD
		 * ItemInfoTimeLo|unsigned int|날짜 정보, FILETIME의 LOWORD
		 * ItemInfoLock|byte|버전 정보 잠그기(on: 1/off:0)
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_document_GetVersionInfo 
		 */
		ParameterSet* GetVersionInfo(int index);

		/**
		 * @brief 현재 문서의 버전 정보 개수를 반환합니다.
		 *
		 * @return int 현재 문서의 버전 정보의 개수를 반환
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_document_GetVersionHistoryCount
		 */
		int GetVersionHistoryCount();

		/**
		 * @brief 문서의 스타일 목록을 가져옵니다.
		 *
		 * @param filePath
		 * @return styleSet
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_document_GetStyleList 
		 */
		ParameterSet* GetStyleList(const  char16_t* filePath = nullptr);

		/**
		 * @brief 현재 문서의 스타일을 내보냅니다.
		 *
		 * @param styleset
		 * @return true
		 * @return false
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_document_ExportStyle
		 */
		bool ExportStyle(ParameterSet* styleset);

		/**
		 * @brief 현재 문서의 스타일을 불러옵니다.
		 *
		 * @param styleset 불러올 스타일 세트입니다.
		 * @return true
		 * @return false
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_document_ImportStyle
		 */
		bool ImportStyle(ParameterSet* styleset);

		/**
		 * @brief Action의 UI 상태를 가져옵니다.
		 *
		 * @param actid 액션 ID입니다.
		 * @param withkey true: 메뉴 스트링 끝에 단축키를 표시합니다. @n
		 *	false: 메뉴 이름만 추가합니다.
		  * @return IParameterSet 편집, 실행 상태, 라디오 버튼, 문자열과 같은 속성 전달
		 * @return null 실패
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_document_GetActionCmdUIStatus 
		 */
		ParameterSet* GetActionCmdUIStatus(const char16_t* actid, int withkey);
		
		/**
		 * @brief 현재 위치의 글머리를 가져옵니다. headingstring은 20자를 넘지 않습니다.
		 *
		 * @param headingstring
		 * @param headingstringsize
		 *
		 * @return true
		 * @return false
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_document_GetHeadingString
		 */
		int GetHeadingString(char16_t* headingstring, int headingstringsize);

		/**
		 * @brief 개체의 가로 기준 위치에 대한 현재 위치 값을 가져옵니다.
		 *
		 * @param ctrl 개체 컨트롤입니다.
		 * @param relTo 가로 기준입니다. (종이: 0, 쪽: 1, 단: 2, 문단: 3)
		 * @return int
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_document_GetCtrlHorizontalOffset
		 */
		int GetCtrlHorizontalOffset(CtrlCode* pCtrl, int relTo);

		/**
		 * @brief 개체의 세로 기준 위치에 대한 현재 위치 값을 가져옵니다.
		 *
		 * @param ctrl 개체 컨트롤입니다.
		 * @param relTo 세로 기준입니다. (종이: 0, 쪽: 1, 문단: 2)
		 * @return int
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_document_GetCtrlVerticalOffset
		 */
		int GetCtrlVerticalOffset(CtrlCode* pCtrl, int relTo);

		/**
		 * @brief 메모의 내부 텍스트를 가져옵니다.
		 *
		 * @param ctrl 메모 컨트롤입니다.
		 * @param memostring 텍스트 버퍼입니다.
		 * @param memostringsize 텍스트 버퍼 크기입니다.
		 * @return int
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_document_GetMemoString
		 */
		int GetMemoString(CtrlCode* ctrl, char16_t* memostring, int memostringsize);

		/**
		 * @brief 문서의 글꼴 목록을 가져옵니다. 사용 전 ScanFont api를 통해 글꼴 목록을 스캔합니다.
		 *
		 * @param langid 언어입니다.
		 * 값|설명
		 * 0|한글
		 * 1|라틴
		 * 2|한자
		 * 3|일본어
		 * 4|기타
		 * 5|심볼
		 * 6|사용자
		 *
		 * @param fontlist 텍스트 버퍼입니다.
		 * @param fontlistsize 텍스트 버퍼 크기입니다.
		 * @return int
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_document_GetFontList
		 */
		int GetFontList(const int langid, char16_t* fontlist, int fontlistsize);

		/**
		 * @brief
		 *
		 * @param langid 언어
		 * 값|설명
		 * 0|한글
		 * 1|라틴
		 * 2|한자
		 * 3|일본어
		 * 4|기타
		 * 5|기호
		 * 6|사용자
		 * @param desFontName 대상 글꼴 이름
		 * @param desFontType 대상 글꼴 타입 (1: HFDT_TTF, 2: HFDT_HFT)
		 * @param newFontName 변경 글꼴 이름
		 * @param newFontType 변경 글꼴 타입 (1: HFDT_TTF, 2: HFDT_HFT)
		 * @return true
		 * @return false
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_document_ReplaceFont
		 */
		bool ReplaceFont(int langid, const char16_t* desFontName, int desFontType, const char16_t* newFontName, int newFontType);

		/**
		 * @brief 문서의 글꼴을 검색합니다. GetFontList를 통해 글꼴 목록을 가져올 수 있습니다.
		 *
		 * @return true 검색 성공
		 * @return false 검색 실패
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_document_ScanFont
		 */
		bool ScanFont();

		/**
		 * @brief 컨트롤(이미지,표,수식 등)을 이미지로 저장합니다.
		 *
		 * @param path 경로입니다.
		 * @param format 이미지 형식입니다.
		 * @param fullsave 나뉜 표의 경우 전체를 저장할지 여부를 지정합니다.
		 *
		 * @return true
		 * @return false
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_document_GetCtrlToPicture
		 */
		bool GetCtrlToPicture(char16_t* path, int format, bool fullsave);

		/**
		 * @brief 현재 위치 또는 선택 영역에 메모를 삽입합니다.
		 *
		 * @param Content 메모 내용입니다.
		 * @param Author 메모 작성자입니다.
		 * @param DateTime 메모 작성 날짜입니다.
		 *
		 * @return 메모 컨트롤의 ID
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_document_InsertMemo
		 */
		int InsertMemo(const char16_t* Content, const char16_t* Author = nullptr, const char16_t* DateTime = nullptr);

		/**
		 * @brief 메모를 선택합니다. 메모의 ID는 CtrlCode를 통해 가져오거나 InsertMemo의 반환 값으로 얻을 수 있습니다.
		 *
		 * @param 메모 컨트롤의 ID입니다.
		 *
		 * @return true
		 * @return false
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_document_SelectMemo
		 */
		bool SelectMemo(int nMemoInstID);


		/**
		 * @brief 현재 커서가 위치한 메모의 정보를 수정합니다.
		 *
		 * @param Content 메모 내용입니다.
		 * @param Author 메모 작성자입니다.
		 * @param DateTime 메모 작성 날짜입니다.
		 *
		 * @return true
		 * @return false
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_document_ModifyMemoModifyMemo
		 */
		bool ModifyMemo(const char16_t* Content, const char16_t* Author = nullptr, const char16_t* DateTime = nullptr);

		/**
		 * @brief 현재 커서가 위치한 메모를 삭제합니다.
		 *
		 * @param nMemoInstID 메모 컨트롤의 ID입니다.
		 *
		 * @return true
		 * @return false
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_document_DeleteMemoDeleteMemo
		 */
		bool DeleteMemo(int nMemoInstID);

		/**
		 * @brief 현재 위치에 있는 메모의 내용을 가져옵니다.
		 *
		 * @param content
		 * @param contentsize
		 * @return
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_document_GetMemoContent
		 */
		int GetMemoContent(char16_t* content, int contentsize);

		/**
		 * @brief 현재 위치에 있는 메모의 작성자를 가져옵니다.
		 *
		 * @param author
		 * @param authorsize
		 * @return
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_document_GetMemoAuthor
		 */
		int GetMemoAuthor(char16_t* author, int authorsize);

		/**
		 * @brief 현재 위치에 있는 메모의 작성 시간을 가져옵니다.
		 *
		 * @param datetime
		 * @param datetimesize
		 * @return
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_document_GetMemoDateTime
		 */
		int GetMemoDateTime(char16_t* datetime, int datetimesize);

		SHDK::IObject* GetImpl();

	protected:
		SHDK::IObject* m_pImpl;
	};

	/**
	* @}
	*/
}

