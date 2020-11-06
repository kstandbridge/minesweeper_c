#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' ""version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include <windows.h>
#include <stdio.h>
#include <assert.h>
#include <commctrl.h>

#include <stdlib.h>
#include <time.h>

#include "resource.h"

#define ID_BUTTON 9000

const char g_szClassName[] = "mineSweeperClass";

int g_boardCols = 10;
int g_boardRows = 15;
int g_num_bombs = 10;
BOOL g_bFirstMove = TRUE;
int g_TilesToCheck = 0;
int g_TimerSeconds = 0;

int* g_nBoard = NULL;
char* g_nBoardGuess = NULL;

BOOL ClearBoard(HWND hwnd)
{
    for(int y = 0; y < g_boardRows; ++y)
        for(int x = 0; x < g_boardCols; ++x)
    {
        int button_id = ID_BUTTON + (y * g_boardCols + x);
        
        HWND button_hwnd = GetDlgItem(hwnd, button_id);
        assert(button_hwnd != NULL);
        
        BOOL res = DestroyWindow(button_hwnd);
        assert(res);
    }
    return TRUE;
}

BOOL IsBombOnButton(int x, int y)
{
    if(g_nBoard[y * g_boardCols + x] == -1)
    {
        return TRUE;
    }
    return FALSE;
}

BOOL ToggleBombVisibility(HWND hwnd, BOOL show_bombs)
{
    for(int x = 0; x < g_boardCols; x++)
        for(int y = 0; y < g_boardRows; y++)
    {
        int button_id = ID_BUTTON + (y * g_boardCols + x);
        HWND button_hwnd = GetDlgItem(hwnd, button_id);
        assert(button_hwnd != NULL);
        
        HICON hIcon = NULL;
        HINSTANCE hInstance = GetModuleHandle(NULL);
        char guess = g_nBoardGuess[y * g_boardCols + x];
        
        int value = g_nBoard[y * g_boardCols + x];
        if(value == -1)
        {
            if(show_bombs)
            {
                if(guess == 'B')
                {
                    hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MINE_CORRECT));
                }
                else
                {
                    hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MINE_UNEXPLODE));
                }
            }
            SendMessage(button_hwnd, WM_SETTEXT, 0, (LPARAM)"");
        }
        else
        {
            if(guess == 'B')
            {
                hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MINE_INCORRECT));
            }
            else if(value > 0)
            {
                SetDlgItemInt(hwnd, button_id, value, TRUE);
            }
            else
            {
                SendMessage(button_hwnd, WM_SETTEXT, 0, (LPARAM)"");
            }
        }
        
        SendMessage(button_hwnd, BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)hIcon);
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
    
    HWND hStatus = GetDlgItem(hwnd, IDC_STATUS);
    RECT rcStatus;
    SendMessage(hStatus, WM_SIZE, 0, 0);
    GetWindowRect(hStatus, &rcStatus);
    int iStatusHeight = rcStatus.bottom - rcStatus.top;
    window_height = window_height - iStatusHeight;
    
    int offset_left = window_width % g_boardCols / 2;
    int offset_top = window_height % g_boardRows / 2;
    
    for(int x = 0; x < g_boardCols; x++)
        for(int y = 0; y < g_boardRows; y++)
    {
        int button_id = ID_BUTTON + (y * g_boardCols + x);
        
        int button_width = window_width / g_boardCols;
        int button_height = window_height / g_boardRows;
        int button_left = (window_width / g_boardCols * x) + offset_left;
        int button_top = (window_height / g_boardRows * y) + offset_top;
        
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

LRESULT CALLBACK ButtonWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
    switch(msg)
    {
        case WM_RBUTTONUP:
        {
            char buf[255];
            int button_id = GetDlgCtrlID(hwnd);
            
            int x = (button_id - ID_BUTTON) % g_boardCols;
            int y = (button_id - ID_BUTTON) / g_boardCols;
            char currentGuess = g_nBoardGuess[y * g_boardCols + x];
            char newGuess;
            switch(currentGuess)
            {
                case 'B':
                {
                    newGuess = '?';
                } break;
                case '?':
                {
                    newGuess = ' ';
                } break;
                default:
                {
                    newGuess = 'B';
                } break;
            }
            g_nBoardGuess[y * g_boardCols + x] = newGuess;
            
            HINSTANCE hInstance = GetModuleHandle(NULL);
            HICON hIcon = NULL;
            
            switch(newGuess)
            {
                case 'B':
                {
                    hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MINE_UNEXPLODE));
                } break;
                case '?':
                {
                    hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MINE_UNKNOWN));
                } break;
                
            }
            
            if(hIcon != NULL)
            {
                SendMessage(hwnd, BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)hIcon);
            }
            else
            {
                SendMessage(hwnd, BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)0);
                SendMessage(hwnd, WM_SETTEXT, 0, (LPARAM)" ");
            }
            
            
        } break;
    }
    return DefSubclassProc(hwnd, msg, wParam, lParam);
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
    
    if(g_nBoard)
    {
        GlobalFree(g_nBoard);
    }
    g_nBoard = GlobalAlloc(GPTR, sizeof(int) * g_boardRows * g_boardCols);
    
    if(g_nBoardGuess)
    {
        GlobalFree(g_nBoardGuess);
    }
    g_nBoardGuess = GlobalAlloc(GPTR, sizeof(int) * g_boardRows * g_boardCols);
    
    
    for(int x = 0; x < g_boardCols; x++)
        for(int y = 0; y < g_boardRows; y++)
    {
        int button_id = ID_BUTTON + (y * g_boardCols + x);
        
        HWND button_hwnd = CreateWindow("BUTTON", "", 
                                        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON | BS_ICON,
                                        (1+x)*32, (1+y)*32, 32, 32,
                                        hwnd, (HMENU)button_id, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
        
        SetWindowSubclass(button_hwnd, ButtonWndProc, 0, 0);
        
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
    KillTimer(hwnd, IDT_TIMER);
    if(clear_board == TRUE)
    {
        ClearBoard(hwnd);
    }
    InitalizeButtons(hwnd);
    ToggleBombVisibility(hwnd, FALSE);
    PositionButtons(hwnd);
    g_bFirstMove = TRUE;
    HWND hStatus = GetDlgItem(hwnd, IDC_STATUS);
    SendMessage(hStatus, SB_SETTEXT, 1, (LPARAM)"New game started, good luck!");
    g_TimerSeconds = 0;
    SetTimer(hwnd, IDT_TIMER, 1000, (TIMERPROC)NULL);
}

BOOL HandleButtonClick(HWND hwnd, int button_id, BOOL recursive)
{
    HWND hButton = GetDlgItem(hwnd, button_id);
    if(hButton == NULL)
    {
        return FALSE;
    }
    assert(button_id >= ID_BUTTON);
    
    if(!IsWindowEnabled(hButton))
    {
        return TRUE;
    }
    
    int x = (button_id - ID_BUTTON) % g_boardCols;
    int y = (button_id - ID_BUTTON) / g_boardCols;
    
    if(g_bFirstMove == TRUE)
    {
        g_TilesToCheck = (g_boardCols * g_boardRows) - g_num_bombs;
        
        int mines = g_num_bombs;
        
        while(mines > 0)
        {
            int rx = rand() % g_boardCols;
            int ry = rand() % g_boardRows;
            
            if(g_nBoard[ry * g_boardCols + rx] == 0 && rx != x && ry != y)
            {
                g_nBoard[ry * g_boardCols + rx] = -1;
                mines--;
            }
        }
        
        g_bFirstMove = FALSE;
    }
    
    if(IsBombOnButton(x, y))
    {
        KillTimer(hwnd, IDT_TIMER);
        ToggleBombVisibility(hwnd, TRUE);
        HINSTANCE hInstance = GetModuleHandle(NULL);
        HICON hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MINE_EXPLODE));
        SendMessage(hButton, BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)hIcon);
        MessageBox(hwnd, "Bomb! Game Over...", "BOOM", MB_OK | MB_ICONWARNING);
        CreateNewGame(hwnd, TRUE);
        return TRUE;
    }
    
    int bomb_count = 0;
    for(int i = -1; i < 2; i++)
    {
        for(int j = -1; j < 2; j++)
        {
            if(x + i >= 0 && x + i < g_boardCols && y + j >= 0 && y + j < g_boardRows)
            {
                if(g_nBoard[(y + j) * g_boardCols + (x + i)] == -1)
                {
                    bomb_count++;
                }
            }
        }
    }
    
    if(bomb_count > 0 && recursive == TRUE)
    {
        return TRUE;
    }
    
    if(bomb_count > 0)
    {
        char buf[3];
        
        sprintf_s(buf, sizeof(buf), "%d", bomb_count);
        
        SendMessage(hButton, WM_SETTEXT, 0, (LPARAM)buf);
    }
    g_nBoard[y * g_boardCols + x] = bomb_count;
    
    EnableWindow(hButton, FALSE);
    g_TilesToCheck--;
    
    char buf[255];
    sprintf_s(buf, sizeof(buf), "Tiles to check: %d", g_TilesToCheck);
    
    HWND hStatus = GetDlgItem(hwnd, IDC_STATUS);
    SendMessage(hStatus, SB_SETTEXT, 1, (LPARAM)&buf);
    
    if(g_TilesToCheck == 0)
    {
        KillTimer(hwnd, IDT_TIMER);
        MessageBox(hwnd, "You're Winner!", "Conglaturation !!!", MB_OK | MB_ICONWARNING);
        CreateNewGame(hwnd, TRUE);
        return TRUE;
    }
    
    if(bomb_count == 0)
    {
        for(int i = -1; i < 2; i++)
        {
            for(int j = -1; j < 2; j++)
            {
                if(x + i >= 0 && x + i < g_boardCols && y + j >= 0 && y + j < g_boardRows)
                {
                    int recursive_button_id = (y + j) * g_boardCols + (x + i);
                    HandleButtonClick(hwnd, recursive_button_id + ID_BUTTON, TRUE);
                }
            }
        }
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

BOOL CALLBACK SettingsDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
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
        case WM_TIMER:
        {
            switch(wParam)
            {
                case IDT_TIMER:
                {
                    g_TimerSeconds++;
                    char buf[255];
                    sprintf_s(buf, sizeof(buf), "%d seconds.", g_TimerSeconds);
                    HWND hStatus = GetDlgItem(hwnd, IDC_STATUS);
                    assert(hStatus != NULL);
                    SendMessage(hStatus, SB_SETTEXT, 0, (LPARAM)&buf);
                } break;
            }
        } break;
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
            srand((unsigned int)time(NULL));
            HWND hStatus = CreateWindowEx(0, STATUSCLASSNAME, "",
                                          SBARS_SIZEGRIP | WS_CHILD | WS_VISIBLE,
                                          0, 0, 0, 0,
                                          hwnd, (HMENU)IDC_STATUS, GetModuleHandle(NULL), NULL);
            int status_width[] = { 100, -1 };
            SendMessage(hStatus, SB_SETPARTS, sizeof(status_width)/sizeof(int), (LPARAM)status_width);
            CreateNewGame(hwnd, FALSE);
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
            KillTimer(hwnd, IDT_TIMER);
            if(g_nBoard)
            {
                GlobalFree(g_nBoard);
            }
            if(g_nBoardGuess)
            {
                GlobalFree(g_nBoardGuess);
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
    wc.hIcon         = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MINE));
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wc.lpszMenuName  = MAKEINTRESOURCE(IDR_MENU);
    wc.lpszClassName = g_szClassName;
    wc.hIconSm       = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MINE));
    
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