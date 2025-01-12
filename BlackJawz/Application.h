#pragma once

namespace BlackJawz::Application
{
	class Application
	{
	public:
		Application();
		~Application();

		HRESULT Initialise(HINSTANCE hInstance, int nCmdShow);
	private:
		HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow);

		// Initialise + Device
		HWND                    _hWnd;
		UINT _WindowWidth = 1920;
		UINT _WindowHeight = 1080;
	};
}