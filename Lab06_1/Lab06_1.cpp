#include "framework.h"
#include "resource.h"
#include <stdio.h>

// Жорстко задаємо назви, щоб вікно гарантовано створилося
LPCWSTR szTitle = L"Сервер FileMapping";
LPCWSTR szWindowClass = L"ServerFileMapClass";

static HWND hwndEdit;
static HANDLE hMapFile = NULL;
static HANDLE hEvent = NULL;
LPVOID pBuf;
char mess[2048] = "Лабораторна робота - Відображення файлу в пам'ять.\r\n";
char* m_mess = mess;
#define IDT_TIMER 1001

// Оголошення функцій
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    MyRegisterClass(hInstance);
    if (!InitInstance(hInstance, nCmdShow)) return FALSE;

   
    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_LAB061));

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) {
            TranslateMessage(&msg); DispatchMessage(&msg);
        }
    }
    return (int)msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex = { 0 };
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);


    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_LAB061);

    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
    return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, 450, 400, nullptr, nullptr, hInstance, nullptr);

    if (!hWnd) {
        MessageBoxA(NULL, "Помилка створення вікна! Перевір назву меню у MyRegisterClass.", "Помилка", MB_OK | MB_ICONERROR);
        return FALSE;
    }
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);
    return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
        hwndEdit = CreateWindowA("EDIT", mess, WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL, 10, 10, 400, 300, hWnd, NULL, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
        break;

    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        switch (wmId)
        {
        case ID_FILEMAPPING_START: 
            hMapFile = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, 512, "myFileMapping");
            if (hMapFile == NULL || hMapFile == INVALID_HANDLE_VALUE) {
                MessageBoxA(hWnd, "Помилка створення FileMapping", "Map", MB_OK);
                break;
            }
            hEvent = CreateEventA(NULL, FALSE, FALSE, "myFileMappingEvent");
            SetTimer(hWnd, IDT_TIMER, 100, NULL);
            sprintf_s(mess, "%sСервер FileMapping запущено\r\n", m_mess);
            SendMessageA(hwndEdit, WM_SETTEXT, 0, (LPARAM)m_mess);
            break;

        case ID_FILEMAPPING_STOP: 
            KillTimer(hWnd, IDT_TIMER);
            if (hMapFile) CloseHandle(hMapFile);
            if (hEvent) CloseHandle(hEvent);
            hMapFile = NULL;
            hEvent = NULL;
            sprintf_s(mess, "%sСервер FileMapping зупинено\r\n", m_mess);
            SendMessageA(hwndEdit, WM_SETTEXT, 0, (LPARAM)m_mess);
            break;

        case ID_FILEMAPPING_READ: 
            if (hMapFile == NULL) {
                MessageBoxA(hWnd, "Сервер не запущено!", "Помилка", MB_OK);
                break;
            }
            pBuf = MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, 512);
            if (pBuf != NULL) {
                sprintf_s(mess, "%s\r\n[Ручне читання]:\r\n%s\r\n", m_mess, (LPCSTR)pBuf);
                SendMessageA(hwndEdit, WM_SETTEXT, 0, (LPARAM)m_mess);
                UnmapViewOfFile(pBuf);
            }
            break;

        case IDM_EXIT: DestroyWindow(hWnd); break;
        default: return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
    break;

    case WM_TIMER: // Автоматичне читання (Завдання з зірочкою)
        if (wParam == IDT_TIMER && hEvent != NULL && hMapFile != NULL) {
            if (WaitForSingleObject(hEvent, 0) == WAIT_OBJECT_0) {
                pBuf = MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, 512);
                if (pBuf != NULL) {
                    sprintf_s(mess, "%s\r\n%s\r\n", m_mess, (LPCSTR)pBuf);
                    SendMessageA(hwndEdit, WM_SETTEXT, 0, (LPARAM)m_mess);
                    UnmapViewOfFile(pBuf);
                }
            }
        }
        break;

    case WM_DESTROY:
        if (hMapFile) CloseHandle(hMapFile);
        if (hEvent) CloseHandle(hEvent);
        PostQuitMessage(0);
        break;
    default: return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}