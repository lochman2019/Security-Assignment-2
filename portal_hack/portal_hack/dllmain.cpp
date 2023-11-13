// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include <iostream>
#include <Windows.h>
#include <shared_mutex>
#include "IVEngineServer.h"
#include "IVEngineClient.h"
#include "globalValues.h"

HackVariables* hackVars = new HackVariables();
SRWLOCK srwlock = SRWLOCK_INIT;
WindowVairables* windowVars = new WindowVairables();
Player* player = new Player();
Camera* camera = new Camera();
PortalGun* portalGun = new PortalGun();
IVEngineClient* engineServer;

void* FindInterface(HMODULE dll, const char* name) {
    auto a = GetProcAddress(dll, "CreateInterface");
    if (a == NULL) {
        std::cout << "IT'S NULL\n";
    }
    CreateInterfaceType CreateInterfaceFunction = (CreateInterfaceType)a;
    std::cout << "Got factory\n";

    int returnCode = 0;
    void* interface = CreateInterfaceFunction(name, &returnCode);

    return interface;
}

// Changes the state of the necessary label 
void ChangeOnOff(HWND* wnd, bool state) { 
    if (state) {
        SetWindowText(*wnd, L"Off");
    }
    else {
        SetWindowText(*wnd, L"On");
    }
    ShowWindow(*wnd, SW_HIDE);
    ShowWindow(*wnd, SW_SHOW);
}


LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_CREATE:
        // Creates the labels for the buttons
        windowVars->on_off1 = CreateWindow(L"Static", L"Off", WS_CHILD | WS_VISIBLE, 125, 10, 175, 30, hwnd, 0, windowVars->global_hInstance, 0);
        windowVars->on_off2 = CreateWindow(L"Static", L"Off", WS_CHILD | WS_VISIBLE, 125, 50, 175, 30, hwnd, 0, windowVars->global_hInstance, 0);
        windowVars->label1 = CreateWindow(L"Static", L"| Jump hack - Press F for up and G for down", WS_CHILD | WS_VISIBLE, 300, 10 , 400, 30, hwnd, 0, windowVars->global_hInstance, 0);
        windowVars->label2 = CreateWindow(L"Static", L"| Portal hack - Press 1-9 for different Portal pairs", WS_CHILD | WS_VISIBLE, 300, 50, 400, 30, hwnd, 0, windowVars->global_hInstance, 0);

    case WM_COMMAND:
        std::cout << "command called\n";
        std::cout << wParam;
        if (LOWORD(wParam) == LOWORD(L"Button1")) {
            // perform actions here
            std::cout << "Button1 pressed\n";
            AcquireSRWLockExclusive(&srwlock);
            ChangeOnOff(&windowVars->on_off1, hackVars->jumpHack);
            hackVars->jumpHack = !(hackVars->jumpHack);
            ReleaseSRWLockExclusive(&srwlock);
        }
        else if (LOWORD(wParam) == LOWORD(L"Button2")) {
            std::cout << "Button2 pressed\n";
            AcquireSRWLockExclusive(&srwlock);
            ChangeOnOff(&windowVars->on_off2, hackVars->multiplePortalsHack);
            hackVars->multiplePortalsHack = !(hackVars->multiplePortalsHack);
            ReleaseSRWLockExclusive(&srwlock);
        }
        break;
    case WM_CLOSE:
        DestroyWindow(hwnd);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

void CreateNewWindow(HINSTANCE hInstance) {
    // Register the window class
    const wchar_t CLASS_NAME[] = L"PortalHackWindowClass";

    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    // Create the window
    HWND hwnd = CreateWindowEx(
        0,                              // Optional window styles
        CLASS_NAME,                     // Window class
        L"Portal Hack Controller",                   // Window text
        WS_OVERLAPPEDWINDOW,            // Window style

        // Size and position
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,

        NULL,       // Parent window
        NULL,       // Menu
        hInstance,  // Instance handle
        NULL        // Additional application data
    ); // Creates a new window

    if (hwnd == NULL) {
        MessageBox(NULL, L"Window creation failed", L"Error", MB_ICONERROR);
        return;
    }

    HWND button1 = CreateWindow(
        L"BUTTON",
        L"Toggle",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        10,
        10,
        100,
        30,
        hwnd,
        (HMENU)L"Button1",
        hInstance,
        NULL
    ); // Creates a new button

    HWND button2 = CreateWindow(
        L"BUTTON",
        L"Toggle",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        10,
        50,
        100,
        30,
        hwnd,
        (HMENU)L"Button2",
        hInstance,
        NULL
    );

    ShowWindow(hwnd, SW_SHOWDEFAULT);

    // Enter the message loop
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}


// Switches the portal linkage ID
void PortalIDSwitch(int* portalId) {
    if (GetAsyncKeyState('1') & 1) {
        *portalId = 0;
    } 
    else if (GetAsyncKeyState('2') & 1) {
        *portalId = 1;
    }
    else if (GetAsyncKeyState('3') & 1) {
        *portalId = 2;
    }
    else if (GetAsyncKeyState('4') & 1) {
        *portalId = 3;
    }
    else if (GetAsyncKeyState('5') & 1) {
        *portalId = 4;
    }
    else if (GetAsyncKeyState('6') & 1) {
        *portalId = 5;
    }
    else if (GetAsyncKeyState('7') & 1) {
        *portalId = 6;
    }
    else if (GetAsyncKeyState('8') & 1) {
        *portalId = 7;
    }
    else if (GetAsyncKeyState('9') & 1) {
        *portalId = 8;
    }
    else if (GetAsyncKeyState('0') & 1) {
        *portalId = 9;
    }
}

void InitialisePointers() {
    std::cout << "Start initialise\n";
    // Get pointer to Camera object
    uintptr_t firstStep = *(uintptr_t*)(ClientPtr + 0x004EAAC4);
    uintptr_t secondStep = *(uintptr_t*)(firstStep + 0xC);
    uintptr_t thirdStep = *(uintptr_t*)(secondStep + 0x80);
    uintptr_t fourthStep = *(uintptr_t*)(thirdStep + 0x4);
    uintptr_t fifthStep = *(uintptr_t*)(fourthStep + 0x2A4);
    camera->X = (float*)(fifthStep + 0x334);
    camera->Y = (float*)(fifthStep + 0x338);
    camera->Z = (float*)(fifthStep + 0x33C);
    std::cout << "Got camera\n";

    // Player coordinates
    uintptr_t fS = *(uintptr_t*)(ServerPtr + 0x006E4E94);
    player->X = (float*)(fS + 0x304);
    player->Y = (float*)(fS + 0x308);
    player->Z = (float*)(fS + 0x30C);
    std::cout << "Got player\n";

    // Pointer to PortalGun ID
    uintptr_t s1 = *(uintptr_t*)(ServerPtr + 0x006DD15C);
    uintptr_t s2 = *(uintptr_t*)(s1 + 0X0);
    uintptr_t s3 = *(uintptr_t*)(s2 + 0x50);
    uintptr_t s4 = *(uintptr_t*)(s3 + 0x13C);
    uintptr_t s5 = *(uintptr_t*)(s4 + 0x14);
    uintptr_t s6 = *(uintptr_t*)(s5 + 0x18);
    uintptr_t s7 = *(uintptr_t*)(s6 + 0xC);
    std::cout << "Starting final step\n";
    portalGun->linkID = (int*)(s7 + 0x598);
    std::cout << "Got portal gun\n";

    
    std::cout << "Initialised\n";
}

DWORD WINAPI MyThread(HMODULE module) {
    AllocConsole();
    FILE* f = new FILE;
    freopen_s(&f, "CONOUT$", "w", stdout);

    std::cout << "Injection active\n";

    // DLLs
    ClientModule = GetModuleHandle(L"client.dll");
    ServerModule = GetModuleHandle(L"server.dll");
    EngineModule = GetModuleHandle(L"engine.dll");

    ClientPtr = (uintptr_t)GetModuleHandle(L"client.dll");
    ServerPtr = (uintptr_t)GetModuleHandle(L"server.dll");
    EnginePtr = (uintptr_t)GetModuleHandle(L"engine.dll");

    // Interface for client engine
    engineServer = (IVEngineClient*)FindInterface(EngineModule, "VEngineClient014");

    bool isLoading = false;
    InitialisePointers();

    // Main loop
    while (true) { 
        if (!engineServer->IsInGame()) {
            std::cout << "Loading\n";
        }
        if (!engineServer->IsInGame() && !isLoading) {
            std::cout << "start load\n";
            isLoading = true;
        }
        else if (engineServer->IsInGame() && isLoading && !engineServer->IsDrawingLoadingImage()) {
            std::cout << "finish load\n";
            isLoading = false;
            Sleep(500);
            InitialisePointers();
        }
        // Gets a lock to read the global hackVars 
        AcquireSRWLockShared(&srwlock);
        // Checks if different hacks are active
        if (hackVars->jumpHack) {
            if (GetAsyncKeyState('F') & 1) { //Press F key to go up
                *(player->Z) = *(player->Z)+500;
            }
            if (GetAsyncKeyState('G') & 1) { //Press G key to go down
                *(player->Z) = *(player->Z)-500;
            }
        }
        if (hackVars->multiplePortalsHack) { // Performs the checks for the MultiplePortals section
            PortalIDSwitch(portalGun->linkID);
        }
        if (GetAsyncKeyState('C')) {
            std::cout << "Pressed C";
            engineServer->ClientCmd("npc_create npc_zombie"); // Executes a game console command
            while (GetAsyncKeyState('C')) {
                Sleep(1);
            }
        }
        
        ReleaseSRWLockShared(&srwlock);
    }

}

BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved

)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    {
        windowVars->global_hInstance = hModule;
        CloseHandle(CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)MyThread, hModule, 0, nullptr));
        CloseHandle(CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)CreateNewWindow, hModule, 0, nullptr));
    }
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}