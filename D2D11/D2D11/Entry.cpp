#include <windows.h>

namespace Pipeline
{ LRESULT CALLBACK Procedure(HWND const hWindow, UINT const uMessage, WPARAM const wParameter, LPARAM const lParamater); }

int APIENTRY WinMain(_In_ HINSTANCE const hInstance, _In_opt_ HINSTANCE const prevhInstance, _In_ LPSTR const lpCmdLine, _In_ int const nCmdShow)
{
    HWND hWnd = nullptr;

    {
        WNDCLASSEX wndClass = WNDCLASSEX();

        wndClass.cbSize        = sizeof(WNDCLASSEX);
        wndClass.lpfnWndProc   = Pipeline::Procedure;
        wndClass.hInstance     = hInstance;
        wndClass.hIcon         = LoadIcon(nullptr, IDI_APPLICATION);
        wndClass.hIconSm       = LoadIcon(nullptr, IDI_APPLICATION);
        wndClass.hCursor       = LoadCursor(nullptr, IDC_ARROW);
        wndClass.hbrBackground = static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH));
        wndClass.lpszClassName = "Window";

        RegisterClassEx(&wndClass);
    }
    {
        CREATESTRUCT window = CREATESTRUCT();
        window.lpszClass = "Window";
        window.lpszName  = "Game";
        window.style     = WS_CAPTION | WS_SYSMENU;
        window.cx        = 500;
        window.cy        = 500;
        window.hInstance = hInstance;

        {
            RECT rect { 0, 0, window.cx, window.cy };

            AdjustWindowRectEx(&rect, window.style, NULL, window.dwExStyle);
            window.cx = rect.right  - rect.left;
            window.cy = rect.bottom - rect.top;

            window.x = (GetSystemMetrics(SM_CXSCREEN) - window.cx) / 2;
            window.y = (GetSystemMetrics(SM_CYSCREEN) - window.cy) / 2;
        }

        hWnd = CreateWindowEx
        (
            window.dwExStyle,
            window.lpszClass,
            window.lpszName,
            window.style,
            window.x,
            window.y,
            window.cx,
            window.cy,
            window.hwndParent,
            window.hMenu,
            window.hInstance,
            window.lpCreateParams
        );

        ShowWindow(hWnd, SW_RESTORE);
    }

    MSG msg;

    while (true)
    {
        if (PeekMessage(&msg, HWND(), WM_NULL, WM_NULL, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
                return static_cast<int>(msg.wParam);

            DispatchMessage(&msg);
        }
        else
        {
            SendMessage(hWnd, WM_APP, NULL, NULL);
        }
    }
}

