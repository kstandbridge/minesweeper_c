#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' ""version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include <windows.h>
#include <stdio.h>
#include <assert.h>
#include <commctrl.h>

#include "resource.h"

#define ID_BUTTON 9000

const char g_szClassName[] = "mineSweeperClass";

int g_boardCols = 10;
int g_boardRows = 15;
int g_num_bombs = 10;

int* g_pBombs = NULL;

BOOL ClearBoard(HWND hwnd)
{
    for(int y = 0; y < g_boardRows; ++y)
        for(int x = 0; x < g_boardCols; ++x)
    {
        int button_id = g_boardCols * y + x + ID_BUTTON + 1;
        
        HWND button_hwnd = GetDlgItem(hwnd, button_id);
        assert(button_hwnd != NULL);
        
        BOOL res = DestroyWindow(button_hwnd);
        assert(res);
    }
    return TRUE;
}

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
        
        MoveWindow(button_hwnd, button_left, button_top, button_width, button_height, FALSE);
    }
    InvalidateRect(hwnd, NULL, TRUE);
    UpdateWindow(hwnd);
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

void CreateNewGame(HWND hwnd, BOOL clear_board)
{
    if(clear_board == TRUE)
    {
        ClearBoard(hwnd);
    }
    InitalizeButtons(hwnd);
    ToggleBombVisibility(hwnd, FALSE);
    PositionButtons(hwnd);
}

BOOL HandleButtonClick(HWND hwnd, int button_id, BOOL recursive)
{
    HWND hButton = GetDlgItem(hwnd, button_id);
    if(hButton == NULL)
    {
        return FALSE;
    }
    assert(button_id > 9000);
    
    if(!IsWindowEnabled(hButton))
    {
        return TRUE;
    }
    
    if(IsBombOnButton(button_id))
    {
        MessageBox(hwnd, "Bomb! Game Over...", "BOOM", MB_OK | MB_ICONWARNING);
        CreateNewGame(hwnd, TRUE);
        return TRUE;
    }
    
    int bomb_count = 0;
    if(IsBombOnButton(button_id - 1)) bomb_count++;               // Left
    if(IsBombOnButton(button_id - 1 - g_boardCols)) bomb_count++; // TopLeft
    if(IsBombOnButton(button_id - g_boardCols)) bomb_count++;     // Top
    if(IsBombOnButton(button_id + 1 - g_boardCols)) bomb_count++; // TopRight
    if(IsBombOnButton(button_id + 1)) bomb_count++;               // Right
    if(IsBombOnButton(button_id + 1 + g_boardCols)) bomb_count++; // BottomRight
    if(IsBombOnButton(button_id + g_boardCols)) bomb_count++;     // Bottom
    if(IsBombOnButton(button_id - 1 + g_boardCols)) bomb_count++; // BottomLeft
    
    if(bomb_count > 0 && recursive == TRUE)
    {
        return TRUE;
    }
    
    if(bomb_count > 0)
    {
        char buf[3];
        
        sprintf(buf, "%d", bomb_count);
        
        SendMessage(hButton, WM_SETTEXT, 0, (LPARAM)buf);
    }
    
    EnableWindow(hButton, FALSE);
    
    if(bomb_count == 0)
    {
        HandleButtonClick(hwnd, button_id-1, TRUE);             // Left
        HandleButtonClick(hwnd, button_id-1-g_boardCols, TRUE); // TopLeft
        HandleButtonClick(hwnd, button_id-g_boardCols, TRUE);   // Top
        HandleButtonClick(hwnd, button_id+1-g_boardCols, TRUE); // TopRight
        HandleButtonClick(hwnd, button_id+1, TRUE);             // Right
        HandleButtonClick(hwnd, button_id+1+g_boardCols, TRUE); // BottomRight
        HandleButtonClick(hwnd, button_id+g_boardCols, TRUE);   // Bottom
        HandleButtonClick(hwnd, button_id-1+g_boardCols, TRUE); // BottomLeft
    }
    return TRUE;
}

BOOL SettingsDlgEnableControls(HWND hwnd, BOOL bEnable)
{
    HWND hRows = GetDlgItem(hwnd, IDC_SETTINGS_COLS);
    assert(hRows != NULL);
    EnableWindow(hRows, bEnable);
    
    HWND hCols = GetDlgItem(hwnd, IDC_SETTINGS_ROWS);
    assert(hCols != NULL);
    EnableWindow(hCols, bEnable);
    
    HWND hBombs = GetDlgItem(hwnd, IDC_SETTINGS_BOMBS);
    assert(hBombs != NULL);
    EnableWindow(hBombs, bEnable);
    
    return TRUE;
}

LRESULT CALLBACK SettingsDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
        case WM_INITDIALOG:
        {
            HWND slider = GetDlgItem(hwnd, IDC_SETTINGS_BOMBS);
            assert(slider != NULL);
            SendMessage(slider, TBM_SETRANGE, (WPARAM)TRUE, (LPARAM)MAKELONG(20, 80));
            SendMessage(slider, TBM_SETPAGESIZE, 0, (LPARAM)10);
            SendMessage(slider, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)50);
            
            return TRUE;
        } break;
        case WM_COMMAND:
        {
            switch(LOWORD(wParam))
            {
                case IDC_SETTINGS_EASY:
                {
                    switch(HIWORD(wParam))
                    {
                        case BN_CLICKED:
                        {
                            if(SendDlgItemMessage(hwnd, IDC_SETTINGS_EASY, BM_GETCHECK, 0, 0) == 0)
                            {
                                SettingsDlgEnableControls(hwnd, FALSE);
                                
                                SendDlgItemMessage(hwnd, IDC_SETTINGS_EASY, BM_SETCHECK, 1, 0);
                                SendDlgItemMessage(hwnd, IDC_SETTINGS_NORMAL, BM_SETCHECK, 0, 0);
                                SendDlgItemMessage(hwnd, IDC_SETTINGS_HARD, BM_SETCHECK, 0, 0);
                                SendDlgItemMessage(hwnd, IDC_SETTINGS_CUSTOM, BM_SETCHECK, 0, 0);
                                
                                SendDlgItemMessage(hwnd, IDC_SETTINGS_COLS, WM_SETTEXT, 0, (LPARAM)"10");
                                SendDlgItemMessage(hwnd, IDC_SETTINGS_ROWS, WM_SETTEXT, 0, (LPARAM)"15");
                                
                                HWND slider = GetDlgItem(hwnd, IDC_SETTINGS_BOMBS);
                                assert(slider != NULL);
                                SendMessage(slider, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)20);
                            }
                        } break;
                    }
                } break;
                
                case IDC_SETTINGS_NORMAL:
                {
                    switch(HIWORD(wParam))
                    {
                        case BN_CLICKED:
                        {
                            if(SendDlgItemMessage(hwnd, IDC_SETTINGS_NORMAL, BM_GETCHECK, 0, 0) == 0)
                            {
                                SettingsDlgEnableControls(hwnd, FALSE);
                                
                                SendDlgItemMessage(hwnd, IDC_SETTINGS_EASY, BM_SETCHECK, 0, 0);
                                SendDlgItemMessage(hwnd, IDC_SETTINGS_NORMAL, BM_SETCHECK, 1, 0);
                                SendDlgItemMessage(hwnd, IDC_SETTINGS_HARD, BM_SETCHECK, 0, 0);
                                SendDlgItemMessage(hwnd, IDC_SETTINGS_CUSTOM, BM_SETCHECK, 0, 0);
                                
                                SendDlgItemMessage(hwnd, IDC_SETTINGS_COLS, WM_SETTEXT, 0, (LPARAM)"15");
                                SendDlgItemMessage(hwnd, IDC_SETTINGS_ROWS, WM_SETTEXT, 0, (LPARAM)"20");
                                
                                HWND slider = GetDlgItem(hwnd, IDC_SETTINGS_BOMBS);
                                assert(slider != NULL);
                                SendMessage(slider, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)30);
                            }
                        } break;
                    }
                } break;
                
                case IDC_SETTINGS_HARD:
                {
                    switch(HIWORD(wParam))
                    {
                        case BN_CLICKED:
                        {
                            if(SendDlgItemMessage(hwnd, IDC_SETTINGS_HARD, BM_GETCHECK, 0, 0) == 0)
                            {
                                SettingsDlgEnableControls(hwnd, FALSE);
                                
                                SendDlgItemMessage(hwnd, IDC_SETTINGS_EASY, BM_SETCHECK, 0, 0);
                                SendDlgItemMessage(hwnd, IDC_SETTINGS_NORMAL, BM_SETCHECK, 0, 0);
                                SendDlgItemMessage(hwnd, IDC_SETTINGS_HARD, BM_SETCHECK, 1, 0);
                                SendDlgItemMessage(hwnd, IDC_SETTINGS_CUSTOM, BM_SETCHECK, 0, 0);
                                
                                SendDlgItemMessage(hwnd, IDC_SETTINGS_COLS, WM_SETTEXT, 0, (LPARAM)"20");
                                SendDlgItemMessage(hwnd, IDC_SETTINGS_ROWS, WM_SETTEXT, 0, (LPARAM)"30");
                                
                                HWND slider = GetDlgItem(hwnd, IDC_SETTINGS_BOMBS);
                                assert(slider != NULL);
                                SendMessage(slider, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)40);
                            }
                        } break;
                    }
                } break;
                
                case IDC_SETTINGS_CUSTOM:
                {
                    switch(HIWORD(wParam))
                    {
                        case BN_CLICKED:
                        {
                            if(SendDlgItemMessage(hwnd, IDC_SETTINGS_CUSTOM, BM_GETCHECK, 0, 0) == 0)
                            {
                                SettingsDlgEnableControls(hwnd, TRUE);
                                
                                SendDlgItemMessage(hwnd, IDC_SETTINGS_EASY, BM_SETCHECK, 0, 0);
                                SendDlgItemMessage(hwnd, IDC_SETTINGS_NORMAL, BM_SETCHECK, 0, 0);
                                SendDlgItemMessage(hwnd, IDC_SETTINGS_HARD, BM_SETCHECK, 0, 0);
                                SendDlgItemMessage(hwnd, IDC_SETTINGS_CUSTOM, BM_SETCHECK, 1, 0);
                            }
                        } break;
                    }
                } break;
                
                case IDOK:
                {
                    HWND hWindow = GetParent(hwnd);
                    assert(hWindow != NULL);
                    ClearBoard(hWindow);
                    
                    UINT cols = GetDlgItemInt(hwnd, IDC_SETTINGS_COLS, NULL, FALSE);
                    UINT rows = GetDlgItemInt(hwnd, IDC_SETTINGS_ROWS, NULL, FALSE);
                    
                    HWND hBombs = GetDlgItem(hwnd, IDC_SETTINGS_BOMBS);
                    assert(hBombs != NULL);
                    UINT bombs = SendMessage(hBombs, TBM_GETPOS, 0, 0);
                    assert(bombs > 0);
                    
                    
                    g_boardCols = cols;
                    g_boardRows = rows;
                    g_num_bombs = (cols * rows) * bombs / 100;
                    assert(g_num_bombs > 0);
                    
                    EndDialog(hwnd, IDOK);
                } break;
                case IDCANCEL:
                {
                    EndDialog(hwnd, IDCANCEL);
                } break;
            }
        } break;
        default:
        {
            return FALSE;
        } break;
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
                case IDD_GAME_NEW:
                {
                    CreateNewGame(hwnd, TRUE);
                } break;
                case IDD_GAME_SETTINGS:
                {
                    int res = DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_SETTINGS), hwnd, SettingsDlgProc);
                    if(res == IDOK)
                    {
                        CreateNewGame(hwnd, FALSE);
                    }
                } break;
                case IDD_GAME_EXIT:
                {
                    PostMessage(hwnd, WM_CLOSE, 0, 0);
                } break;
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
                    if(!HandleButtonClick(hwnd, button_id, FALSE))
                    {
                        MessageBox(hwnd, "Failed to handle button click", "Error", MB_OK | MB_ICONERROR);
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
    wc.lpszMenuName  = MAKEINTRESOURCE(IDR_MENU);
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