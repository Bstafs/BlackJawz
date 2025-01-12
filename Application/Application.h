#pragma once

#include "resource.h"

class Application
{
public:
	Application();
	~Application();

	HRESULT Initialise(HINSTANCE hInstance, int nCmdShow);
private:
	HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow);

	// Initialise + Device
	HINSTANCE               _hInst;
	HWND                    _hWnd;
	UINT _WindowWidth = 1920;
	UINT _WindowHeight = 1080;
};