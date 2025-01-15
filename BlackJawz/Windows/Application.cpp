#include "Application.h"

// Windows Version
#include <SDKDDKVer.h>

// Define static members
HWND BlackJawz::Application::Application::_hWnd = nullptr;
UINT BlackJawz::Application::Application::_WindowWidth = 1920;
UINT BlackJawz::Application::Application::_WindowHeight = 1080;

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

	// Create window
	RECT rc = { 0, 0, _WindowWidth, _WindowHeight };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
	_hWnd = CreateWindow(L"BlackJawz Engine", L"BlackJawz Engine", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInstance,
		nullptr);
	if (!_hWnd)
		return E_FAIL;

	ShowWindow(_hWnd, nCmdShow);

	return S_OK;
}