#pragma once

namespace HWPSDK {

	class Document;
	class HWindow;
	/**
	* @class HwpObject
	* @ingroup Apis
	* @brief 한글 SDK의 HwpObject 클래스를 나열합니다.
	*/
	class SDK_FE_API HwpObject
	{
	public:
		HwpObject(SHDK::IObject* pObj);
		~HwpObject();

	public:
		void Release();

		// HwpObject에서 Document를 관리해야 하기 때문에 Create/Release를 HwpObject에서 해야한다는 가이드가 필요함
		/**
		* @brief Document를 생성합니다.
		*
		* @return
		* 
		* **Examples**
		* \snippet snippets.cpp example_HwpObject_CreateDocument
		*/
		Document* CreateDocument();

		/**
		* @brief Document를 종료합니다.
		*
		* @param document 종료하려는 Document입니다.
		* 
		* **Examples**
		* \snippet snippets.cpp example_HwpObject_ReleaseDocument
		*/
		void ReleaseDocument(Document* document);
		
		// !! 윈도우 함수 이름과 충돌로 'H' Prefix 붙임

		/**
		* @brief Window를 생성합니다.
		*
		* @param x x 좌표입니다.
		* @param y y 좌표입니다.
		* @param width 넓이 값입니다.
		* @param height 높이 값입니다.
		* @param style 윈도우 창의 스타일 속성 값입니다. 
		* @param hwnd 부모의 핸들 값입니다.
		* @return
		* 
		* **Examples**
		* \snippet snippets.cpp example_HwpObject_CreateHWindow
		*/
		HWindow* CreateHWindow(int x, int y, int width, int height, unsigned long style, HWND hwnd);

		/**
		* @brief Window를 종료합니다.
		*
		* @param window 종료하려는 Window입니다.
		* 
		* **Examples**
		* \snippet snippets.cpp example_HwpObject_DestoryHWindow
		*/
		void DestroyHWindow(HWindow* window);

		/**
		* @brief 마지막 Error를 가져옵니다.
		*
		* @return @ref ErrorCode
		* 
		* **Examples**
		* \snippet snippets.cpp example_HwpObject_GetLastError
		*/
		int GetLastErrorCode();

		SHDK::IObject* GetImpl();
	private:
		SHDK::IObject* m_pImpl;
	};

	/**
	* @}
	*/
}

