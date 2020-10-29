#include <windows.h>
#include <stdio.h>

#define ID_BUTTON 9000

const char g_szClassName[] = "mineSweeperClass";

int g_boardCols = 10;
int g_boardRows = 15;

BOOL PositionButtons(HWND hwnd)
{
    RECT rect;
    if(!GetClientRect(hwnd, &rect))
    {
        MessageBox(hwnd, "Failed to get window Rect!", "Error", MB_OK | MB_ICONERROR);
    }
    
    int window_width = rect.right - rect.left;
    int window_height = rect.bottom - rect.top;
    
    for(int y = 0; y < g_boardRows; ++y)
        for(int x = 0; x < g_boardCols; ++x)
    {
        int button_width = window_width / g_boardCols;
        int button_height = window_height / g_boardRows;
        int button_left = window_width / g_boardCols * x;
        int button_top = window_height / g_boardRows * y;
        
        int button_id = g_boardCols * y + x + ID_BUTTON + 1;
        
        HWND button_hwnd = GetDlgItem(hwnd, button_id);
        
        if(button_hwnd == NULL)
        {
            MessageBox(hwnd, "Failed to get button handle!", "Error", MB_OK | MB_ICONERROR);
            return FALSE;
        }
        
        MoveWindow(button_hwnd, button_left, button_top, button_width, button_height, TRUE);
    }
    return TRUE;
}

BOOL InitalizeButtons(HWND hwnd)
{
    RECT rect;
    if(!GetClientRect(hwnd, &rect))
    {
        MessageBox(hwnd, "Failed to get window Rect!", "Error", MB_OK | MB_ICONERROR);
    }
    
    HFONT hFont = CreateFont(24,0,0,0, FW_DONTCARE, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                             CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, NULL);
    
    if(hFont == NULL)
    {
        MessageBox(hwnd, "Failed to create font!", "Error", MB_OK | MB_ICONERROR);
    }
    
    for(int y = 0; y < g_boardRows; ++y)
        for(int x = 0; x < g_boardCols; ++x)
    {
        int button_id = g_boardCols * y + x + ID_BUTTON + 1;
        
        HWND button_hwnd = CreateWindow("BUTTON", "B", 
                                        WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                                        0, 0, 32, 32,
                                        hwnd, (HMENU)button_id, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
        if(button_hwnd == NULL)
        {
            MessageBox(hwnd, "Failed to create button!", "Error", MB_OK | MB_ICONERROR);
        }
        
        SendMessage(button_hwnd, WM_SETFONT, (WPARAM)hFont, 0);
    }
    return TRUE;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
        case WM_COMMAND:
        {
            int button_id = LOWORD(wParam);
            HWND button_hwnd = GetDlgItem(hwnd, button_id);
            
            EnableWindow(button_hwnd, FALSE);
        } break;
        case WM_CREATE:
        {
            if(!InitalizeButtons(hwnd))
            {
                MessageBox(hwnd, "Failed to create game board!", "Error", MB_OK | MB_ICONERROR);
            }
        } break;
        case WM_SIZE:
        {
            if(!PositionButtons(hwnd))
            {
                MessageBox(hwnd, "Failed to position buttons!", "Error", MB_OK | MB_ICONERROR);
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