#include "framework.h"
#include "Application.h"

#include <BlackJawz.h>

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // Initialize the application
    BlackJawz::Engine* engine = new  BlackJawz::Engine();

    engine->Setup(hInstance, nCmdShow);

    //BlackJawz::Engine engine;

    MSG msg = { 0 };

    // Main message and game loop
    while (true)
    {
        // Process all pending messages
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                break; // Exit the loop if the quit message is received
            }

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        // If we received a quit message, break the main loop
        if (msg.message == WM_QUIT)
        {
            break;
        }

        // App Code here
        engine->Run();
    }

    // Cleanup
    engine->Cleanup();
    delete engine;
    return (int)msg.wParam;
}
