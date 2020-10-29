#include <windows.h>
#include <stdio.h>

const char g_szClassName[] = "mineSweeperClass";

BOOL InitalizeButtons(HWND hwnd, int cols, int rows)
{
    RECT rect;
    if(!GetClientRect(hwnd, &rect))
    {
        MessageBox(hwnd, "Failed to get window Rect!", "Error", MB_OK | MB_ICONERROR);
    }
    
    int window_width = rect.right - rect.left;
    int window_height = rect.bottom - rect.top;
    
    for(int y = 0; y < rows; ++y)
        for(int x = 0; x < cols; ++x)
    {
        int button_width = window_width / cols;
        int button_height = window_height / rows;
        int button_left = window_width / cols * x;
        int button_top = window_height / rows * y;
        
        HWND hwndButton = CreateWindow("BUTTON", "B", 
                                       WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                                       button_left, button_top, button_width, button_height,
                                       hwnd, NULL, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
        
        HFONT hFont = CreateFont(24,0,0,0, FW_DONTCARE, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                                 CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, NULL);
        SendMessage(hwndButton, WM_SETFONT, (WPARAM)hFont, 0);
    }
    return TRUE;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
        case WM_CREATE:
        {
            if(!InitalizeButtons(hwnd, 10, 15))
            {
                MessageBox(hwnd, "Failed to create game board!", "Error", MB_OK | MB_ICONERROR);
            }
        } break;
        case WM_CLOSE:
        {
            DestroyWindow(hwnd);
        } break;
        case WM_DESTROY:
        {
            PostQuitMessage(0);
        } break;
        default:
        {
            return DefWindowProc(hwnd, msg, wParam, lParam);
        }
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    WNDCLASSEX wc;
    HWND hwnd;
    MSG Msg;
    
    wc.cbSize        = sizeof(WNDCLASSEX);
    wc.style         = 0;
    wc.lpfnWndProc   = WndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = hInstance;
    wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = g_szClassName;
    wc.hIconSm       = LoadIcon(NULL, IDI_APPLICATION);
    
    if(!RegisterClassEx(&wc))
    {
        MessageBox(NULL, "Window Registration Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }
    
    hwnd = CreateWindowEx(WS_EX_CLIENTEDGE, g_szClassName, "Minesweeper",
                          WS_OVERLAPPEDWINDOW,
                          CW_USEDEFAULT, CW_USEDEFAULT, 480, 630,
                          NULL, NULL, hInstance, NULL);
    
    if(hwnd == NULL)
    {
        MessageBox(NULL, "Window Creation Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }
    
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);
    
    while(GetMessage(&Msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }
    return Msg.wParam;
}