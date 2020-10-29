#include <windows.h>
#include <stdio.h>
#include <assert.h>

#include "resource.h"

#define ID_BUTTON 9000

const char g_szClassName[] = "mineSweeperClass";

int g_boardCols = 10;
int g_boardRows = 15;
int g_num_bombs = 10;

int* g_pBombs = NULL;

BOOL IsBombOnButton(int button_id)
{
    BOOL found = FALSE;
    for(int i = 0; i < g_num_bombs; i++)
    {
        if(g_pBombs[i] == button_id)
        {
            found = TRUE;
            break;
        }
    }
    return found;
}

BOOL ToggleBombVisibility(HWND hwnd, BOOL show_bombs)
{
    for(int y = 0; y < g_boardRows; ++y)
        for(int x = 0; x < g_boardCols; ++x)
    {
        int button_id = g_boardCols * y + x + ID_BUTTON + 1;
        HWND button_hwnd = GetDlgItem(hwnd, button_id);
        assert(button_hwnd != NULL);
        SendMessage(button_hwnd, WM_SETTEXT, 0, (LPARAM)"");
        if(show_bombs)
        {
            for(int i = 0; i < g_num_bombs; i++)
            {
                if(g_pBombs[i] == button_id)
                {
                    SendMessage(button_hwnd, WM_SETTEXT, 0, (LPARAM)"B");
                    break;
                }
            }
        }
    }
    return TRUE;
}

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
    
    if(g_pBombs)
    {
        GlobalFree(g_pBombs);
    }
    g_pBombs = GlobalAlloc(GPTR, sizeof(int) * g_num_bombs);
    
    for(int i = 0; i < g_num_bombs; i++)
    {
        int random_number;
        int button_id;
        do
        {
            random_number = rand() % (g_boardRows * g_boardCols);
            button_id = random_number + ID_BUTTON + 1;
            
            BOOL found = FALSE;
            for(int x = 0; x < g_num_bombs; x++)
            {
                if(g_pBombs[x] == button_id)
                {
                    found = TRUE;
                }
            }
            
            if(!found)
            {
                g_pBombs[i] = button_id;
            }
            
        } while(g_pBombs[i] == 0);
    }
    
    for(int y = 0; y < g_boardRows; ++y)
        for(int x = 0; x < g_boardCols; ++x)
    {
        int button_id = g_boardCols * y + x + ID_BUTTON + 1;
        
        HWND button_hwnd = CreateWindow("BUTTON", "", 
                                        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                                        (1+x)*32, (1+y)*32, 32, 32,
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
            switch(button_id)
            {
                case IDD_DEBUG_SHOW_BOMBS:
                {
                    HMENU hMenu = GetMenu(hwnd);
                    assert(hMenu != NULL);
                    DWORD res = GetMenuState(hMenu, IDD_DEBUG_SHOW_BOMBS, MF_BYCOMMAND);
                    switch(res)
                    {
                        case MF_CHECKED:
                        {
                            DWORD res = CheckMenuItem(hMenu, IDD_DEBUG_SHOW_BOMBS, MF_BYCOMMAND | MF_UNCHECKED);
                            assert(res == MF_CHECKED);
                            ToggleBombVisibility(hwnd, FALSE);
                        } break;
                        case MF_UNCHECKED:
                        {
                            DWORD res = CheckMenuItem(hMenu, IDD_DEBUG_SHOW_BOMBS, MF_BYCOMMAND | MF_CHECKED);
                            assert(res == MF_UNCHECKED);
                            ToggleBombVisibility(hwnd, TRUE);
                        } break;
                        default:
                        {
                            assert(res != -1);
                        } break;
                    }
                } break;
                default: 
                {
                    HWND hButton = GetDlgItem(hwnd, button_id);
                    assert(button_id > 9000);
                    
                    if(hButton != NULL)
                    {
                        int bomb_count = 0;
                        if(IsBombOnButton(button_id - 1)) bomb_count++; // Left
                        if(IsBombOnButton(button_id - 1 - g_boardCols)) bomb_count++; // TopLeft
                        if(IsBombOnButton(button_id - g_boardCols)) bomb_count++; // Top
                        if(IsBombOnButton(button_id + 1 - g_boardCols)) bomb_count++; // TopRight
                        if(IsBombOnButton(button_id + 1)) bomb_count++; // Right
                        if(IsBombOnButton(button_id + 1 + g_boardCols)) bomb_count++; // BottomRight
                        if(IsBombOnButton(button_id + g_boardCols)) bomb_count++; // Bottom
                        if(IsBombOnButton(button_id - 1 + g_boardCols)) bomb_count++; // BottomLeft
                        
                        char buf[3];
                        
                        sprintf(buf, "%d", bomb_count);
                        
                        SendMessage(hButton, WM_SETTEXT, 0, (LPARAM)buf);
                        
                        EnableWindow(hButton, FALSE);
                    }
                } break;
            }
        } break;
        case WM_CREATE:
        {
            srand(time(NULL));
            
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
            if(g_pBombs)
            {
                GlobalFree(g_pBombs);
            }
            PostQuitMessage(0);
        } break;
        default:
        {
            return DefWindowProc(hwnd, msg, wParam, lParam);
        } break;
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
    wc.lpszMenuName  = MAKEINTRESOURCE(IDR_DEBUG);
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