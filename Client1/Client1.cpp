#include "framework.h"
#include "resource.h"
#include <stdio.h>

// жорстко задаємо назви, щоб вікно гарантовано створилося
LPCWSTR szTitle = L"Клієнт 1 (Варіант 3)";
LPCWSTR szWindowClass = L"Client1FileMapClass";

static HWND hwndEdit;
static HANDLE hMapFile;
LPVOID pBuf;
char szBuf[512];
char mess[2048] = "";
char* m_mess = mess;

// Оголошення функцій
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    MyRegisterClass(hInstance);
    if (!InitInstance(hInstance, nCmdShow)) return FALSE;


    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_CLIENT1));

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


    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_CLIENT1);

    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
    return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, 400, 300, nullptr, nullptr, hInstance, nullptr);

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
    { // Дужка для ізоляції змінних WM_CREATE
        hwndEdit = CreateWindowA("EDIT", "", WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL, 10, 10, 360, 200, hWnd, NULL, ((LPCREATESTRUCT)lParam)->hInstance, NULL);

        // Формування повідомлення варіанту 3
        int screenWidth = GetSystemMetrics(SM_CXSCREEN);
        int screenHeight = GetSystemMetrics(SM_CYSCREEN);
        int printer = 1; // Заглушка
        HDC hdc = GetDC(hWnd);
        int colorDepth = GetDeviceCaps(hdc, BITSPIXEL);
        ReleaseDC(hWnd, hdc);

        sprintf_s(mess, "Дані від Клієнта #1 (Варіант 3):\r\n- Роздільна здатність: %d x %d\r\n- Наявність принтера: %d\r\n- Глибина кольору: %d біт\r\n", screenWidth, screenHeight, printer, colorDepth);
        SendMessageA(hwndEdit, WM_SETTEXT, 0, (LPARAM)m_mess);
    }
    break;

    case WM_COMMAND:
    { 
        int wmId = LOWORD(wParam);
        switch (wmId)
        {  
        case ID_FILE_SEND:
        { 
            DWORD cbWritten;
            cbWritten = (DWORD)SendMessageA(hwndEdit, WM_GETTEXTLENGTH, 0, 0);
            SendMessageA(hwndEdit, WM_GETTEXT, (WPARAM)cbWritten + 1, (LPARAM)szBuf);

            // Відкриття FileMapping
            HANDLE hMapFile = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, "myFileMapping");
            if (hMapFile == NULL) {
                MessageBoxA(hWnd, "Не можливо відкрити відображений у пам'яті об'єкт (Сервер не запущено?)", "Map", MB_OK);
                break;
            }
            LPVOID pBuf = MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, 512);
            if (pBuf != NULL) {
                CopyMemory((PVOID)pBuf, szBuf, strlen(szBuf) + 1);
                UnmapViewOfFile(pBuf);
            }
            CloseHandle(hMapFile);

            // Сигналізуємо серверу про нові дані (завдання з зірочкою)
            HANDLE hEvent = OpenEventA(EVENT_MODIFY_STATE, FALSE, "myFileMappingEvent");
            if (hEvent != NULL) {
                SetEvent(hEvent);
                CloseHandle(hEvent);
            }

            sprintf_s(mess, "%s\r\nДані успішно відправлено у FileMapping!\r\n", m_mess);
            SendMessageA(hwndEdit, WM_SETTEXT, 0, (LPARAM)m_mess);
            break;
        }
        }
    }
    break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}