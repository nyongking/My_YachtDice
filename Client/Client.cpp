// Client.cpp : 애플리케이션에 대한 진입점을 정의합니다.
#include "ClientPch.h"
#include "Client.h"
#include "MainApp.h"
#include "InputManager.h"
#include "imgui_impl_win32.h"
#include "SocketUtils.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);

#define MAX_LOADSTRING 100

// 이 코드 모듈에 포함된 함수의 선언을 전달합니다:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int, uint32, uint32);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

HINSTANCE inst;
HWND      wnd;

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    // Set working directory to project root (2 levels up from Bin/Debug/)
    // so that relative paths like "Bin/Resource/..." resolve correctly
    // when the exe is launched directly instead of from Visual Studio.
    {
        WCHAR exePath[MAX_PATH];
        GetModuleFileNameW(nullptr, exePath, MAX_PATH);
        // strip: Client.exe  -> Bin/Debug
        WCHAR* p = wcsrchr(exePath, L'\\'); if (p) *p = L'\0';
        // strip: Bin/Debug   -> Bin
        p = wcsrchr(exePath, L'\\'); if (p) *p = L'\0';
        // strip: Bin         -> Client (project root)
        p = wcsrchr(exePath, L'\\'); if (p) *p = L'\0';
        SetCurrentDirectoryW(exePath);
    }

    // 전역 문자열을 초기화합니다.
    MyRegisterClass(hInstance);

    // 애플리케이션 초기화를 수행합니다:
    uint32 sizeX = 1280;
    uint32 sizeY = 720;
    
    if (!InitInstance(hInstance, nCmdShow, sizeX, sizeY))
    {
        return FALSE;
    }

    InitCore();
    Render::InitRender(true, sizeX, sizeY, wnd);
    InitServerCore();
    SocketUtils::Init();

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_CLIENT));

    MSG msg;
    MainApp mainApp;


    mainApp.Init();

    // 기본 메시지 루프입니다:
    while (GRunning)
    {
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            if (WM_QUIT == msg.message)
            {
                GRunning = false;
                break;
            }

            if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }

        if (!GRunning)
            break;

        mainApp.Loop();
    }

    return (int) msg.wParam;
}



//
//  함수: MyRegisterClass()
//
//  용도: 창 클래스를 등록합니다.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CLIENT));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName = nullptr;//MAKEINTRESOURCEW(IDC_CLIENT);
    wcex.lpszClassName  = L"Client";
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   함수: InitInstance(HINSTANCE, int)
//
//   용도: 인스턴스 핸들을 저장하고 주 창을 만듭니다.
//
//   주석:
//
//        이 함수를 통해 인스턴스 핸들을 전역 변수에 저장하고
//        주 프로그램 창을 만든 다음 표시합니다.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow, uint32 sizeX, uint32 sizeY)
{
   inst = hInstance; // 인스턴스 핸들을 전역 변수에 저장합니다.

   RECT rect{ 0, 0, static_cast<LONG>(sizeX), static_cast<LONG>(sizeY) };
   ::AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

  HWND hWnd = CreateWindowW(L"Client", L"Client", WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, rect.right - rect.left, rect.bottom - rect.top, nullptr, nullptr, hInstance, nullptr);
   //HWND hWnd = CreateWindowW(L"Client", L"Client", WS_OVERLAPPEDWINDOW,
   //    0, 0, static_cast<int>(sizeX), static_cast<int>(sizeY), nullptr, nullptr, hInstance, nullptr);

  /*HWND hWnd = CreateWindowW(L"Client", L"Client", WS_POPUP,
      0, 0, static_cast<int>(sizeX), static_cast<int>(sizeY), nullptr, nullptr, hInstance, nullptr);*/

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   wnd = hWnd;

   return TRUE;
}

//
//  함수: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  용도: 주 창의 메시지를 처리합니다.
//
//  WM_COMMAND  - 애플리케이션 메뉴를 처리합니다.
//  WM_PAINT    - 주 창을 그립니다.
//  WM_DESTROY  - 종료 메시지를 게시하고 반환합니다.
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
        return true;

    GameEngine::InputManager::GetInstance().ProcessMessage(message, wParam, lParam);

    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // 메뉴 선택을 구문 분석합니다:
            switch (wmId)
            {
            case IDM_ABOUT:
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: 여기에 hdc를 사용하는 그리기 코드를 추가합니다...
            EndPaint(hWnd, &ps);
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

