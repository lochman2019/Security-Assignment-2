// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include <iostream>
#include <Windows.h>
#include <shared_mutex>
#include "IVEngineServer.h"
#include "IVEngineClient.h"
#include "globalValues.h"
#include "GetEntityList.h"
#include <vector>
using namespace std;

HackVariables* hackVars = new HackVariables();
SRWLOCK srwlock = SRWLOCK_INIT;
WindowVairables* windowVars = new WindowVairables();
Player* player = new Player();
Camera* camera = new Camera();
PortalGun* portalGun = new PortalGun();
IVEngineClient* engineServer;
IClientEntityList* clientEntityList;
SetAbsOriginType SetAbsOriginFunc;
SetNetOriginType SetNetOriginFunc;
SetAbsOriginType SetLocalOriginFunc;
SetAbsOriginType SetServerOriginFunc;
GetModelType GetModelFunc;

float basePos[3] = { 0, 0, 0 };

uint8_t* PatternScan(void* module, const char* signature);  
uint8_t* SecondPattern(void* module, const char* signature);

bool grid[5][5] = {};

//void CBaseEntity::SetAbsOriginFunc(float* origin) {
//    using SetAbsOriginType = void(__thiscall*)(void*, float* origin);
//    static SetAbsOriginType SetAbsOrigin = (SetAbsOriginType)PatternScan()
//}

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

    // Is player on wall
    uintptr_t t1 = *(uintptr_t*)(PhysicsPtr + 0x00D492C);
    uintptr_t t2 = *(uintptr_t*)(t1 + 0x0);
    uintptr_t t3 = *(uintptr_t*)(t2 + 0xB0);
    uintptr_t t4 = *(uintptr_t*)(t3 + 0x4);

    player->onWall = (int*)(t4 + 0x104);


    
    std::cout << "Initialised\n";
}

void SetEntityPos(float x, float y, float z, void* ent) {
    float* x_coord = (float*)ent + 0x97;
    float* y_coord = (float*)ent + 0x97 + 0x1;
    float* z_coord = (float*)ent + 0x97 + 0x2; // Positions of the MODEL (not the actual object)
    float* roll = (float*)ent + 0x97 + 0x3;
    float* pitch = (float*)ent + 0x97 + 0x4;
    float* yaw = (float*)ent + 0x97 + 0x5;
    float origin[3] = { x, y, z };
    float angle[3] = { 0, 0, 0 };
    float vel[3] = { 0, 0, 0 };
    //SetAbsOriginFunc((int*)ent, origin);
    SetNetOriginFunc((int*)ent, origin);
    SetAbsOriginFunc((int*)ent, origin);
    *roll = 0;
    *pitch = 0;
    *yaw = 0;
}

vector<void*> GetAllEntities() { // Gets all entities of the weighted box type in the world
    int h_i = clientEntityList->GetHighestEntityIndex(); // Get the highest index currently in use
    vector<void*> entities = {};
    int i = 0;
    void* current = 0x0;
    int number_blocks = 0;
    bool isFirst = true;
    while (i <= h_i) {
        if ((current = clientEntityList->GetClientEntity(i)) != 0x0) { // Iterate through the entity list, filter out null
            int* e_id = (int*)(((int*)current) + 0x1D);

            //auto name = GetModelFunc((int*)current);
            auto name = " ";
            //std::cout << "index: " << i << " | address: " << current << " | id : " << e_id << " | id_a : " << *e_id << " | name: " << name << "\n";
            if (*e_id == 257) { // Check if the entity is a weighted box
                int a1 = *(((float*)current) + 0x68) * 100;
                int a2 = *(((float*)current) + 0x69) * 100;
                int a3 = *(((float*)current) + 0x6A) * 100;
                int a4 = *(((float*)current) + 0x6B) * 100;
                int a5 = *(((float*)current) + 0x6C) * 100;
                int a6 = *(((float*)current) + 0x6D) * 100;
                int v1 = -2025;
                int v2 = 2025;
                if (a1 == v1 && a2 == v1 && a3 == v1 && a4 == v2 && a5 == v2 && a6 == v2) {
                    entities.push_back(current);
                }
            }
        }
        i++;

    }
    //std::cout << "count " << c << "\n";
   // std::cout << "sioze " << entities.size() << "\n";
    return entities;
}

void FlipPixels(int i, int j) {
    int iVals[2] = { i - 1, i + 1 };
    int jVals[2] = { j - 1, j + 1 };
    int c;
    for (int a = 0; a < 2; a++) {
        c = iVals[a];
        if (c >= 0 && c <= 4) {
            grid[c][j] = !grid[c][j];
        }
        c = jVals[a];
        if (c >= 0 && c <= 4) {
            grid[i][c] = !grid[i][c];
        }
    }
    grid[i][j] = !grid[i][j];
}

int GetClosest(float x, float* in) {
    float a[5] = { };
    for (int i = 0; i < 5; i++) {
        a[i] = abs(in[i] - x);
    }
    int s_i = 0;
    float smallest = 100;
    for (int i = 0; i < 5; i++) {
        if (a[i] < smallest) {
            smallest = a[i];
            s_i = i;
        }
    }
    return s_i;
}

void NewLevel() {
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            if (rand() % 100 > 49) {
                grid[i][j] = true;
            }
            else {
                grid[i][j] = false;
            }
        }
    }
}

void DisplayGrid() {
    int width = 5;
    int height = 5;
    vector<void*> entities = GetAllEntities();
    int diff = (width * height) - entities.size();
    int size_diff = 55;
    float half = (((width-1)*size_diff) / 2);
    *(player->X) = -1400;
    *(player->Y) = -2750;
    //*(player->Z) = basePos[2] + half;
    //std::cout << "Start\n";
    if (diff > 0) {
        engineServer->ClientCmd("ent_create_portal_weight_box");
        Sleep(200);
    }
    else {
        float base_x = *(player->X) + 500;
        float base_y = *(player->Y) + half;
        float base_z = *(player->Z) + 75;
        int size_diff = 55;

        float l1i[5] = { 0.515, 0.589, 0.655, 0.720, 0.783 };
        float l2i[5] = { 0.359, 0.433, 0.504, 0.577, 0.642 };
        int li = GetClosest(*((player->X) + 0x23), l1i);
        int lj = GetClosest(*((player->X) + 0x24), l2i);


        std::cout << lj << ":" << *((player->X) + 0x24) << " | " << li << ":" << *((player->X) + 0x23) << "\n";
        

        if (GetAsyncKeyState(0x01) & 1) {
            FlipPixels(li, lj);
        }

        if (GetAsyncKeyState('N') & 1) {
            NewLevel();
        }

        int index = 0;
        while (index < (width * height)) {
            //std::cout << "Setting entity position\n";
            int i2 = div(index, 5).quot;
            int j2 = index % 5;
            float z_coord = (base_z + ((div(index, width).quot) * size_diff));
            if (!grid[i2][j2]) {
                z_coord = 5000;
            }
            SetEntityPos(base_x, (base_y - ((index % width) * size_diff)), z_coord, entities[index]);
            //std::cout << *(player->X) << " | " << *(player->Y) << " | " << *(player->Z) << "\n";
            //SetEntityPos(*(player->X), *(player->Y), *(player->Z)-100, entities[index]);
            index++;
        }
    }

}


void InitMiniGame() {
    std::cout << "start game init\n";
    basePos[0] = *(player->X);
    basePos[1] = *(player->Y);
    basePos[2] = *(player->Z);
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            grid[i][j] = true;
        }
    }
    std::cout << *((player->X) + 0x23) << " | " << *((player->X) + 0x24) << "\n";
    std::cout << basePos << "\n";
    std::cout << "finish game init\n";
}

DWORD WINAPI Test(HMODULE module) {
    while (true) {
        //*(camera->X) = 850;
        //*(camera->Y) = 500;
        *(camera->Z) = 500;
    }
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
    PhysicsModule = GetModuleHandle(L"vphysics.dll");

    ClientPtr = (uintptr_t)ClientModule;
    ServerPtr = (uintptr_t)ServerModule;
    EnginePtr = (uintptr_t)EngineModule;
    PhysicsPtr = (uintptr_t)PhysicsModule;

    // Interface for client engine
    engineServer = (IVEngineClient*)FindInterface(EngineModule, "VEngineClient014");
    clientEntityList = (IClientEntityList*)FindInterface(ClientModule, "VClientEntityList003");

    const char* networkPosPattern = "55 8B EC 8B 45 08 D9 00 D9 99 34 03 00 00 D9 40 04 D9 99 38 03 00 00 D9 40 08 D9 99 3C 03 00 00 5D C2 04 00";
    std::uint8_t* ps = PatternScan(GetModuleHandleW(L"client.dll"), networkPosPattern);

    const char* setPosPattern = "55 8B EC 56 57 8B F1 E8 ? ? ? ? 8B 7D 08 F3 0F 10 07 0F 2E 86 ? ? ? ?";
    SetAbsOriginFunc = (SetAbsOriginType)PatternScan(GetModuleHandleW(L"client.dll"), setPosPattern);

    const char* setLocalPattern = "55 8B EC 56 57 8B 7D 08 8B F1 F3 0F 10 07 0F 2E 86 8C 02 00 00 9F";
    SetLocalOriginFunc = (SetAbsOriginType)PatternScan(GetModuleHandleW(L"client.dll"), setLocalPattern);

    const char* serverLocalPattern = "55 8B EC F3 0F 10 ? ? ? ? ? 83 EC 10 0F 28 C1 0F 57 05 ? ? ? ? 56 8B 75 08 57 8B F9 F3 0F 10 16";
    auto sp = SecondPattern(GetModuleHandleW(L"server.dll"), serverLocalPattern);
    SetServerOriginFunc = (SetAbsOriginType)sp;

    std::cout << "start model nbame\n";

    const char* modelNamePattern = "8B 81 94 01 00 00 C3";
    //auto mp = PatternScan(GetModuleHandleW(L"client.dll"), modelNamePattern);
    //GetModelFunc = (GetModelType)mp;

    //std::cout << "funish model name" << mp << "\n";


    std::cout << "Pattern scan: " << (int*)sp << "\n";

    SetNetOriginFunc = (SetNetOriginType)ps;

    bool onWall = false;
    bool isLoading = false;
    bool miniGame = false;
    bool gameIsInit = false;
    InitialisePointers();

    // Main loop
    while (!(GetAsyncKeyState('9') & 1)) {
        if (miniGame && !isLoading && !gameIsInit) {
            DisplayGrid();
        }
        if (miniGame && gameIsInit && !isLoading) {
            InitMiniGame();
            gameIsInit = false;
        }
        if (GetAsyncKeyState('T') & 1) {
            miniGame = !miniGame;
            if (miniGame && !gameIsInit) {
                gameIsInit = true;
                isLoading = true;
                engineServer->ClientCmd("map testchmb_a_10");
                Sleep(10);
            }
        }
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
            Sleep(3000);
            InitialisePointers();
        }
        if (GetAsyncKeyState('K') & 1) { // Execute the get entities function
            std::cout << "K pressed\n";
            GetAllEntities();
        }
        if (GetAsyncKeyState('J') & 1) { // Execute the get entities function
            std::cout << "x coord " << player->X << "\n";
        }
       /* if (*(player->onWall) == 1 && !onWall) {
            onWall = true;
            std::cout << "Player is on wall\n";
        }
        else if (*(player->onWall) != 1 && onWall) {
            std::cout << *(player->onWall);
            std::cout << "\n";
            onWall = false;
            std::cout << "Player is off wall\n";
        }*/
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
            engineServer->ClientCmd("ent_create_portal_weight_box"); // Executes a game console command
            while (GetAsyncKeyState('C')) {
                Sleep(1);
            }
        }
        if (GetAsyncKeyState('Z')) {
            std::cout << "Pressed Z";
            engineServer->ClientCmd("developer 1"); // Executes a game console command
            while (GetAsyncKeyState('C')) {
                Sleep(1);
            }
        }
        if (GetAsyncKeyState('V')) {
            std::cout << "Pressed V";
            engineServer->ClientCmd("ent_text"); // Executes a game console command
            while (GetAsyncKeyState('C')) {
                Sleep(1);
            }
        }

        
        ReleaseSRWLockShared(&srwlock);
    }
    if (f) fclose(f);
    FreeConsole();  
    FreeLibraryAndExitThread(module, 0);
    return 0;
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