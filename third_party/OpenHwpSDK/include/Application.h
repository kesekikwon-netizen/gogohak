#pragma once
namespace HWPSDK {

	class HwpObject;

	class SDK_FE_API Application
	{
	public:
		Application();
		virtual ~Application();

	public:
		int InitInstance();
		void ExitInstance();
		int GetVersion();
		void SetLicensePath(const char* path);   // InitInstance 전 호출. NULL/"" 이면 엔진이 DLL→EXE 폴더에서 자동 탐색

		HwpObject* GetHwpObject();
	private:
		HwpObject* m_pObject;
	};
}

