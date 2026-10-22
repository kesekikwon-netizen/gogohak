#pragma once
namespace HWPSDK {

	class ParameterArray;

	/**
	 * @class ParameterSet
	 * @ingroup Apis
	 * @brief 액션 실행에 필요한 ParameterSet 인터페이스입니다.
	 */
	class SDK_FE_API ParameterSet{
	public:
		ParameterSet(SHDK::IObject* pSet);
		virtual ~ParameterSet();

	public:
		// IParameterSet Implementation
		void Release();

		/**
		 * @brief 하위 항목의 개수를 가져옵니다.
		 *
		 * @return int
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_ParameterSet_GetCount
		 */
		int GetCount();

		/**
		 * @brief PameterSet의 ID를 가져옵니다.
		 *
		 * @return
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_ParameterSet_GetSetID
		 */
		const char16_t* GetSetID();

		/**
		 * @brief @ref IParameterArray를 항목으로 생성합니다.
		 *
		 * @param itemid 항목 ID입니다.
		 * @param count 생성할 배열의 초기 크기입니다.
		 * @return
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_ParameterSet_CreateItemArray
		 */
		ParameterArray* CreateItemArray(const char16_t* itemid, int count);

		/**
		 * @brief @ref IParameterSet을 항목으로 생성합니다.
		 *
		 * @param itemid 항목 ID입니다.
		 * @param setid 생성할 세트 ID입니다.
		 * @return
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_ParameterSet_CreateItemSet
		 */
		ParameterSet* CreateItemSet(const char16_t* itemid, const char16_t* setid);

		/**
		 * @brief ParameterSet의 itemid 항목이 등록되어 있는지 확인합니다.
		 *
		 * @param itemid 항목 ID입니다.
		 * @return
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_ParameterSet_ItemExist
		 */
		bool ItemExist(const char16_t* itemid);

		/**
		 * @brief srcset과 item을 합칩니다.
		 *
		 * @param srcset 대상 ParameterSet입니다.
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_ParameterSet_Merge
		 */
		void Merge(ParameterSet* srcset);

		/**
		 * @brief parameter set을 초기화합니다.
		 *
		 * @param setid 새로 적용할 set id입니다.
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_ParameterSet_RemoveAll
		 */
		void RemoveAll(const char16_t* setid);

		/**
		 * @brief 지정한 Item을 삭제합니다.
		 *
		 * @param itemid 항목 ID입니다.
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_ParameterSet_RemoveItem
		 */
		void RemoveItem(const char16_t* itemid);

		/**
		 * @brief itemid의 item String 값을 가져옵니다.
		 *
		 * @param itemid 항목 ID입니다.
		 * @param value 문자열 버퍼입니다.
		 * @param valuesize 버퍼 크기입니다.
		 * @return
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_ParameterSet_GetString
		 */
		int GetString(const char16_t* itemid, char16_t* value, int valuesize);

		/**
		 * @brief itemid의 item Int 값을 가져옵니다.
		 *
		 * @param itemid 항목 ID입니다.
		 * @return
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_ParameterSet_GetInt
		 */
		int GetInt(const char16_t* itemid);

		/**
		 * @brief itemid의 item UInt 값을 가져옵니다.
		 *
		 * @param itemid 항목 ID입니다.
		 * @return
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_ParameterSet_GetUInt
		 */
		unsigned int GetUInt(const char16_t* itemid);

		/**
		 * @brief itemid의 item ParameterSet 값을 가져옵니다.
		 *
		 * @param itemid 항목 ID입니다.
		 * @return
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_ParameterSet_GetItemSet
		 */
		ParameterSet* GetItemSet(const char16_t* itemid);

		/**
		 * @brief itemid의 item ParameterArray 값을 가져옵니다.
		 *
		 * @param itemid 항목 ID입니다.
		 * @return
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_ParameterSet_GetItemArray
		 */
		ParameterArray* GetItemArray(const char16_t* itemid);

		/**
		 * @brief itemid의 item String 값을 설정합니다.
		 *
		 * @param itemid 항목 ID입니다.
		 * @return
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_ParameterSet_SetString
		 */
		void SetString(const char16_t* itemid, const char16_t* value);

		/**
		 * @brief itemid의 item Int 값을 설정합니다.
		 *
		 * @param itemid 항목 ID입니다.
		 * @param value int 값입니다.
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_ParameterSet_SetInt
		 */
		void SetInt(const char16_t* itemid, int value);

		/**
		 * @brief itemid의 item UInt 값을 설정합니다.
		 *
		 * @param itemid 항목 ID입니다.
		 * @param value unsigned int 값입니다.
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_ParameterSet_SetUInt
		 */
		void SetUInt(const char16_t* itemid, unsigned int value);

		/**
		 * @brief itemid의 item ParameterSet 값을 설정합니다.
		 *
		 * @param itemid 항목 ID입니다.
		 * @param value ParameterSet입니다.
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_ParameterSet_SetItemSet
		 */
		void SetItemSet(const char16_t* itemid, ParameterSet* value);

		/**
		 * @brief itemid의 item ParameterArray 값을 설정합니다.
		 *
		 * @param itemid 항목 ID입니다.
		 * @param value ParameterArray입니다.
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_ParameterSet_SetItemArray
		 */
		void SetItemArray(const char16_t* itemid, ParameterArray* value);

		/**
		 * @brief itemid의 item Type을 가져옵니다.
		 *
		 * @param itemid 항목 ID입니다.
		 * 
		 * @return int
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_ParameterSet_GetItemType
		 */
		unsigned int GetItemType(const char16_t* itemid);
	public:
		SHDK::IObject* GetImpl();
	private:
		SHDK::IObject* m_pImpl;
	};

	/**
	* @}
	*/
}

