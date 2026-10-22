#pragma once
namespace HWPSDK {

	class ParameterSet;

	/**
	 * @class CtrlCode
	 * @ingroup Apis
	 * @brief 한글 SDK의 CtrlCode 클래스를 나열합니다.
	 */
	class SDK_FE_API CtrlCode{
	public:
		CtrlCode(SHDK::IObject* pCtrl);
		~CtrlCode();
		void Release();

	public:
		/**
		 * @brief 컨트롤 문자
		 * 값|설명
		 * -|-
		 * 1|예약
		 * 2|구역/단 정의
		 * 3|필드 시작
		 * 4|필드 끝
		 * 5|예약
		 * 6|예약
		 * 7|예약
		 * 8|예약
		 * 9|탭
		 * 10|강제 줄 나눔
		 * 11|그리기 개체/표
		 * 12|예약
		 * 13|문단 나누기
		 * 14|예약
		 * 15|주석
		 * 16|머리말/꼬리말
		 * 17|각주/미주
		 * 18|자동 번호
		 * 19|예약
		 * 20|예약
		 * 21|쪽 바뀜
		 * 22|책갈피/찾아보기 표시
		 * 23|덧말/글자 겹침
		 * 24|하이픈
		 * 25|예약
		 * 26|예약
		 * 27|예약
		 * 28|예약
		 * 29|예약
		 * 30|묶음 빈칸
		 * 31|고정 폭 빈칸
		 * 
		 * @return short
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_CtrlCode_GetCtrlCh
		 */
		short GetCtrlCh();
		
		/**
		 * @brief 컨트롤 ID는 컨트롤의 종류를 나타내기 위해 할당된 ID로서 최대 4개의 문자로 구성된 문자열입니다. 예를 들어 표는 "tbl", 각주는 "fn"입니다. 한글에서 현재까지 지원되는 모든 컨트롤의 ID를 확인하려면 다음 표를 참조하세요. 
		 * ID|Property Set|Initialization Set|설명
		 * -|-|-|-
		 * cold|ColDef|ColDef|단
		 * secd|SecDef|SecDef|구역
		 * fn|FootnoteShape|FootnoteShape|각주
		 * en|FootnoteShape|FootnoteShape|미주
		 * tbl|Table|TableCreation|표
		 * eqed|EqEdit|EqEdit|수식
		 * gso|ShapeObject|ShapeObject|그리기 개체
		 * atno|AutoNum|AutoNum|번호 넣기
		 * nwno|AutoNum|AutoNum|새 번호로
		 * pgct|PageNumCtrl|PageNumCtrl|페이지 번호 제어
		 * pghd|PageHiding|PageHiding|감추기
		 * pgnp|PageNumPos|PageNumPos|쪽 번호 위치
		 * head|HeaderFooter|HeaderFooter|머리말
		 * foot|HeaderFooter|HeaderFooter|꼬리말
		 * %dte|FieldCtrl|FieldCtrl|현재의 날짜/시간 필드
		 * %ddt|FieldCtrl|FieldCtrl|파일 작성 날짜/시간 필드
		 * %pat|FieldCtrl|FieldCtrl|문서 경로 필드
		 * %bmk|FieldCtrl|FieldCtrl|블록 책갈피
		 * %mmg|FieldCtrl|FieldCtrl|메일 머지
		 * %xrf|FieldCtrl|FieldCtrl|상호 참조
		 * %fmu|FieldCtrl|FieldCtrl|계산식
		 * %clk|FieldCtrl|FieldCtrl|누름틀
		 * %smr|FieldCtrl|FieldCtrl|문서 요약 정보 필드
		 * %usr|FieldCtrl|FieldCtrl|사용자 정보 필드
		 * %hlk|FieldCtrl|FieldCtrl|하이퍼링크
		 * bokm|TextCtrl|TextCtrl|책갈피
		 * idxm|IndexMark|IndexMark|찾아보기
		 * tdut|Dutmal|Dutmal|덧말
		 * tcmt|없음|없음|주석
		 *
		 * @return const char16_t* 
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_CtrlCode_GetCtrlID
		 */
		const char16_t* GetCtrlID();
		
		/**
		 * @brief 문서의 모든 컨트롤(표, 그림 등의 특수 문자)은 linked list로 서로 연결되어 있으며 list 중 다음 컨트롤을 나타냅니다.
		 *
		 * @return CtrlCode
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_CtrlCode_GetNext
		 */
		CtrlCode* GetNext();
		
		/**
		 * @brief 문서의 모든 컨트롤(표, 그림 등의 특수 문자)은 linked list로 서로 연결되어 있으며 list 중 이전 컨트롤을 나타냅니다.
		 *
		 * @return CtrlCode
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_CtrlCode_GetPrev
		 */
		CtrlCode* GetPrev();
		
		/**
		 * @brief 컨트롤에 해당하는 parameter set으로 속성을 가져옵니다.
		 *
		 * @return ParameterSet
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_CtrlCode_GetProperties
		 */
		ParameterSet* GetProperties();
		
		/**
		 * @brief 컨트롤에 해당하는 parameter set으로 속성을 설정합니다.
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_CtrlCode_SetProperties
		 */
		void SetProperties(ParameterSet* newValue);
		
		/**
		 * @brief 컨트롤의 종류를 사용자에게 보여줄 수 있는 localize된 문자열로 나타냅니다.
		 *
		 * @return int 문자열 길이
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_CtrlCode_GetUserDesc
		 */
		int GetUserDesc(char16_t* value, int valuesize);
		
		/**
		 * @brief 컨트롤의 Anchor(조판 부호)의 위치를 가져옵니다.
		 *
		 * @return ParamterSet
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_CtrlCode_GetAnchorPos
		 */
		ParameterSet* GetAnchorPos(int type);
		
		/**
		 * @brief 컨트롤의 고유한 InstId를 가져옵니다.
		 *
		 * @return int 문자열 길이
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_CtrlCode_GetCtrlInstID
		 */
		int GetCtrlInstID(char16_t* value, int valuesize);

		SHDK::IObject* GetImpl();
	private:
		SHDK::IObject* m_pImpl;
	};

	/**
	* @}
	*/
}

