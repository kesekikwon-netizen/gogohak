#pragma once
namespace HWPSDK {
	class Document;

	/**
	 * @class HWindow
	 * @ingroup Apis
	 * @brief 창(window)을 생성하고 제어합니다. 
	 */

	class SDK_FE_API HWindow{
	public:
		HWindow(SHDK::IObject* pObj);
		virtual ~HWindow();

	public:
		void Release();

		/**
		 * @brief window에 연결된 Document API를 가져옵니다.
		 *
		 * **Examples**
		 * \snippet snippets.cpp example_HWindow_GetDocument
		 */
		Document* GetDocument();


		/**
		 * @brief window의 뷰 속성 Properties를 가져옵니다.
		 *
		 * **Examples**
		 * \snippet snippets.cpp example_HWindow_GetProperties
		 */
		ParameterSet* GetProperties();


		/**
		 * @brief window의 뷰 속성 Properties를 설정합니다.
		 *
		 * @param newvalue
		 *
		 * **Examples**
		 * \snippet snippets.cpp example_HWindow_SetProperties
		 */
		void SetProperties(ParameterSet* newvalue);
		

		/**
		 * @brief 현재 window에 설정된 핸들을 가져옵니다. 
		 *
		 * **Examples**
		 * \snippet snippets.cpp example_HWindow_GetWindowHandle
		 */
		HWND GetWindowHandle();


		/**
		 * @brief 크기를 조절합니다.
		 *
		 * @param x x 좌표
		 * @param y y 좌표
		 * @param width 넓이
		 * @param height 높이
		 *
		 * **Examples**
		 * \snippet snippets.cpp example_HWindow_Resize
		 */
		void Resize(int x, int y, int width, int height);
		

		/**
		 * @brief 현재 창에 포커스를 둡니다.
		 *
		 * **Examples**
		 * \snippet snippets.cpp example_HWindow_SetFocus
		 */
		void SetFocus();
		

		/**
		 * @brief 창을 표시하거나 숨깁니다.
		 *
		 * @param bVisible
		 *
		 * **Examples**
		 * \snippet snippets.cpp example_HWindow_SetVisible
		 */
		void SetVisible(bool bVisible);


		/**
		 * @brief 현재 페이지의 번호를 가져옵니다. 
		 *
		 *
		 * **Examples**
		 * \snippet snippets.cpp example_HWindow_GetCurPageNumber
		 */
		int GetCurPageNumber();


		/**
		 * @brief 현재 줄의 번호를 가져옵니다.
		 *
		 *
		 * **Examples**
		 * \snippet snippets.cpp example_HWindow_GetCurLineNumber
		 */
		int GetCurLineNumber();


		/**
		 * @brief 현재 pos를 가져옵니다.
		 *
		 *
		 * **Examples**
		 * \snippet snippets.cpp example_HWindow_GetCurPosNumber
		 */
		int GetCurPosNumber();

	public:
		SHDK::IObject* GetImpl();
	private:
		Document* m_pDocument;
		SHDK::IObject* m_pImpl;
	};

	/**
	* @}
	*/
}

