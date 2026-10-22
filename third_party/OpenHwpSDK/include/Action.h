#pragma once
namespace HWPSDK {

	class ParameterSet;

	/**
	 * @class Action
	 * @ingroup Apis
	 * @brief Document 클래스의 CreateAction API를 이용해 액션 API를 실행할 수 있는 액션 인터페이스입니다.
	 */
	class SDK_FE_API Action {
	public:
		Action(SHDK::IObject* obj);
		virtual ~Action();

	public:
		void Release();

		/**
		 * @brief 액션에서 사용하는 ParameterSet을 생성합니다.
		 *
		 * @return @ref IParameterSet
		 *
		 * **Examples**
		 * \snippet snippets.cpp example_action_CreateParameterSet
		 */
		ParameterSet* CreateParameterSet();

		/**
		 * @brief 액션에서 사용하는 ParameterSet의 기본 값을 가져옵니다.
		 *
		 * @param set
		 * @return
		 *
		 * **Examples**
		 * \snippet snippets.cpp example_action_GetDefault
		 */
		int GetDefault(ParameterSet* set);

		/**
		 * @brief ParameterSet을 이용하여 액션을 실행합니다. 
		 * 
		 * @param set
		 * @return 
		 *
		 * **Examples**
		 * \snippet snippets.cpp example_action_Execute
		 */
		int Execute(ParameterSet* set);

	public:
		SHDK::IObject* GetImpl();
	private:
		SHDK::IObject* m_pImpl;
	};

	/**
	* @}
	*/
}

