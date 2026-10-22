#pragma once
namespace HWPSDK {

	class ParameterSet;
	
	/**
	 * @class ParameterArray
	 * @ingroup Apis
	 * @brief 액션 실행에 필요한 ParameterArray 인터페이스입니다.
	 */
	class SDK_FE_API ParameterArray
	{
	public:
		ParameterArray(SHDK::IObject* obj);
		~ParameterArray();

	public:
		void Release();

		/**
		 * @brief ParameterArray의 배열 크기를 가져옵니다.
		 * @return int
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_ParameterArray_GetCount
		 */
		int GetCount();

		/**
		 * @brief ParameterArray의 배열 크기를 설정합니다.
		 *
		 * @param count 배열의 크기입니다.
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_ParameterArray_SetCount
		 */
		void SetCount(int count);

		/**
		 * @brief ParameterArray를 복사합니다.
		 *
		 * @param srcarray 복사 대상 ParameterArray입니다.
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_ParameterArray_Copy
		 */
		void Copy(ParameterArray* srcarray);
		
		/**
		 * @brief index의 문자열을 가져옵니다.
		 *
		 * @param index 인덱스이며 0부터 시작합니다.
		 * @param value 텍스트 버퍼입니다.
		 * @param valuesize 버퍼 크기입니다.
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_ParameterArray_GetString
		 */
		int GetString(int index, char16_t* value, int valuesize);
		
		/**
		 * @brief index의 UInt 값을 가져옵니다. 
		 *
		 * @param index
		 * 
		 * @return int
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_ParameterArray_GetInt
		 */
		int GetInt(int index);
		
		/**
		 * @brief index의 UInt 값을 가져옵니다. 
		 *
		 * @param index 인덱스이며 0부터 시작합니다.
		 * 
		 * @return unsigned int 
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_ParameterArray_GetUInt
		 */
		unsigned int GetUInt(int index);
		
		/**
		 * @brief index의 ItemSet을 가져옵니다.
		 *
		 * @param index
		 * 
		 * @return ParameterSet
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_ParameterArray_GetItemSet
		 */
		ParameterSet* GetItemSet(int index);

		/**
		 * @brief index에 문자열을 설정합니다.
		 *
		 * @param index 인덱스이며 0부터 시작합니다.
		 * @param value 값입니다.
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_ParameterArray_SetString
		 */
		void SetString(int index, const char16_t* value);

		/**
		 * @brief index에 int 값을 설정합니다.
		 *
		 * @param index 인덱스이며 0부터 시작합니다.
		 * @param value 값입니다.
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_ParameterArray_SetInt
		 */
		void SetInt(int index, int value);
		
		/**
		 * @brief index에 unsigned int 값을 설정합니다.
		 *
		 * @param index 인덱스이며 0부터 시작합니다.
		 * @param value 값
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_ParameterArray_SetUInt
		 */
		void SetUInt(int index, unsigned int value);
		
		/**
		 * @brief index에 ParameterSet을 설정합니다.
		 *
		 * @param index 인덱스이며 0부터 시작합니다.
		 * @param value
		 * 
		 * **Examples**
		 * \snippet snippets.cpp example_ParameterArray_SetItemSet
		 */
		void SetItemSet(int index, ParameterSet* value);
		int GetType();

	public:
		SHDK::IObject* GetImpl();

	private:
		SHDK::IObject* m_pImpl;
	};

	/**
	* @}
	*/
}