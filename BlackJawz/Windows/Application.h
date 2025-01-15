#pragma once
#include "../pch.h"

namespace BlackJawz::Application
{
	class Application
	{
	public:
		Application();
		~Application();

		HRESULT Initialise(HINSTANCE hInstance, int nCmdShow);

		static HWND GetWindow() { return _hWnd; }
		static UINT GetWindowWidth() { return _WindowWidth; }
		static UINT GetWindowHeight() { return _WindowHeight; }

	private:
		HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow);

		static HWND _hWnd;
		static UINT _WindowWidth;
		static UINT _WindowHeight;
	};
}