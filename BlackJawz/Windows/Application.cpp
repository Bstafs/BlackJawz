#include "Application.h"

// Windows Version
#include <SDKDDKVer.h>

// Define static members
HWND BlackJawz::Application::Application::_hWnd = nullptr;
UINT BlackJawz::Application::Application::_WindowWidth = GetSystemMetrics(SM_CXSCREEN);
UINT BlackJawz::Application::Application::_WindowHeight = GetSystemMetrics(SM_CYSCREEN);

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

BlackJawz::Application::Application::Application()
{

}

BlackJawz::Application::Application::~Application()
{

}

HRESULT BlackJawz::Application::Application::Initialise(HINSTANCE hInstance, int nCmdShow)
{
	if (FAILED(InitWindow(hInstance, nCmdShow)))
	{
		return E_FAIL;
	}

	RECT rc;
	GetClientRect(_hWnd, &rc);
	_WindowWidth = rc.right - rc.left;
	_WindowHeight = rc.bottom - rc.top;

	return S_OK;
}

HRESULT BlackJawz::Application::Application::InitWindow(HINSTANCE hInstance, int nCmdShow)
{
    // Register class
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, (LPCTSTR)IDI_APPLICATION);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = CreateSolidBrush(RGB(150, 180, 255));
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = L"BlackJawz Engine";
    wcex.hIconSm = LoadIcon(wcex.hInstance, (LPCTSTR)IDI_APPLICATION);
    if (!RegisterClassEx(&wcex))
        return E_FAIL;

    // Get the screen resolution of the primary monitor
    HMONITOR hMonitor = MonitorFromWindow(NULL, MONITOR_DEFAULTTOPRIMARY);
    MONITORINFO monitorInfo = { sizeof(MONITORINFO) };
    GetMonitorInfo(hMonitor, &monitorInfo);

    UINT screenWidth = monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left;
    UINT screenHeight = monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top;

    // Store the dimensions in the class variables
    _WindowWidth = screenWidth;
    _WindowHeight = screenHeight;

    // Create a fullscreen window (borderless)
    DWORD style = WS_POPUP; // Borderless window style
    DWORD exStyle = WS_EX_APPWINDOW | WS_EX_TOPMOST; // Keep window on top

    RECT rc = { 0, 0, static_cast<LONG>(_WindowWidth), static_cast<LONG>(_WindowHeight) };

    AdjustWindowRect(&rc, style, FALSE); // Ensure the size fits properly for fullscreen

    _hWnd = CreateWindowEx(
        exStyle,                           // Extended style
        L"BlackJawz Engine",               // Class name
        L"BlackJawz Engine",               // Window name
        style,                             // Window style
        monitorInfo.rcMonitor.left,        // X position
        monitorInfo.rcMonitor.top,         // Y position
        rc.right - rc.left,                // Width
        rc.bottom - rc.top,                // Height
        nullptr,                           // Parent window
        nullptr,                           // Menu
        hInstance,                         // Instance handle
        nullptr);                          // Additional application data

    if (!_hWnd)
        return E_FAIL;

    // Show the window
    ShowWindow(_hWnd, nCmdShow);

    // Bring the window to the foreground
    SetForegroundWindow(_hWnd);

    return S_OK;
}