// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include <iostream>
#include <Windows.h>
#include <shared_mutex>
#include "IVEngineServer.h"
#include "IVEngineClient.h"
#include "dllmain.h"
#include "GetEntityList.h"
#include <vector>

using namespace std;

/**
* Represents a 3D vector
*/
class Vector3 {
public:
    float* x;
    float* y;
    float* z;
};

/**
* A wrapper for pointers to various fields on the player object.
*/
class Player {
    HMODULE serverModule;
public:
    Vector3* position;
    Vector3* velocity;
    bool flying;

    Player(HMODULE serverModule) {
        position = new Vector3();
        velocity = new Vector3();
        flying = false;
        this->serverModule = serverModule;
        this->ReloadPointers();
    }

    /**
    * Gets pointers to necessary playervalues from the Portal process
    */
    void ReloadPointers() {
        uintptr_t server = (uintptr_t) this->serverModule;

        // A pointer to a memory region containing numerous player object fields
        uintptr_t playerRegion = *(uintptr_t*)(server + 0x006E4E94);

        // Wrap fields in our player class
        this->velocity->x = (float*)(playerRegion + 0x214);
        this->velocity->y = (float*)(playerRegion + 0x218);
        this->velocity->z = (float*)(playerRegion + 0x21C);

        this->position->x = (float*)(playerRegion + 0x304);
        this->position->y = (float*)(playerRegion + 0x308);
        this->position->z = (float*)(playerRegion + 0x30C);

    }
};

/**
* A wrapper for pointers to various fields on the portal gun object.
*/
class PortalGun {
    HMODULE serverModule;
public:
    int* linkID;

    PortalGun(HMODULE serverModule) {
        this->serverModule = serverModule;
        this->ReloadPointers();
    }

    /**
    * Gets pointers to necessary portal gun values from the Portal process
    */
    void ReloadPointers() {
        uintptr_t server = (uintptr_t) this->serverModule;

        // Get a pointer to the PortalGun ID
        uintptr_t s1 = *(uintptr_t*)(server + 0x006DD15C);
        uintptr_t s2 = *(uintptr_t*)(s1 + 0X0);
        uintptr_t s3 = *(uintptr_t*)(s2 + 0x50);
        uintptr_t s4 = *(uintptr_t*)(s3 + 0x13C);
        uintptr_t s5 = *(uintptr_t*)(s4 + 0x14);
        uintptr_t s6 = *(uintptr_t*)(s5 + 0x18);
        uintptr_t s7 = *(uintptr_t*)(s6 + 0xC);

        this->linkID = (int*)(s7 + 0x598);
    }
};

/**
* State for the hack controller window.
*/
struct WindowState {
    HINSTANCE global_hInstance;

    HWND superJumpEnabled;
    HWND multiPortalEnabled;
    HWND weightBoxSpawnEnabled;

    HWND superJumpLabel;
    HWND multiPortalLabel;
    HWND weightBoxSpawnLabel;
};

/**
* Stores the current state of different hacks.
* Used to allow the hacks to be controlled by the window
*/
struct HackState {
    bool superJump = false;
    bool multiPortals = false;
    bool weightBoxSpawn = false;
};

HackState* hackState = new HackState();
SRWLOCK srwlock = SRWLOCK_INIT;

WindowState* windowState = new WindowState();


//Sets up a wrapper class for the player and portal gun that will load pointers to the necessary memory locations.
Player* player = new Player(GetModuleHandle(L"server.dll"));
PortalGun* portalGun = new PortalGun(GetModuleHandle(L"server.dll"));

// Various game engine functions
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

void* FindInterface(HMODULE dll, const char* name) {
    auto a = GetProcAddress(dll, "CreateInterface");
    if (a == NULL) {
        std::cout << "Couldn't load interface" << name << "\n";
    }
    CreateInterfaceType CreateInterfaceFunction = (CreateInterfaceType)a;

    int returnCode = 0;
    void* interface = CreateInterfaceFunction(name, &returnCode);

    return interface;
}

/**
*Changes the text of a passed label based on some state
*/
void ChangeOnOff(HWND* wnd, bool state) { 
    if (state) {
        SetWindowText(*wnd, L"On");
    }
    else {
        SetWindowText(*wnd, L"Off");
    }
    UpdateWindow(*wnd);

}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_CREATE:
        // Creates the labels for the buttons
        windowState->superJumpEnabled = CreateWindow(
            L"Static",
            L"Off",
            WS_CHILD | WS_VISIBLE,
            220,
            10,
            50,
            30,
            hwnd,
            0,
            windowState->global_hInstance, 
            0
        );

        windowState->multiPortalEnabled = CreateWindow(
            L"Static",
            L"Off",
            WS_CHILD | WS_VISIBLE,
            220,
            50,
            50,
            30,
            hwnd,
            0,
            windowState->global_hInstance,
            0
        );

        windowState->weightBoxSpawnEnabled = CreateWindow(
            L"Static",
            L"Off",
            WS_CHILD | WS_VISIBLE,
            220,
            90,
            50,
            30,
            hwnd,
            0,
            windowState->global_hInstance,
            0
        );

        windowState->superJumpLabel = CreateWindow(
            L"Static",
            L"Press Space to super jump",
            WS_CHILD | WS_VISIBLE,
            280,
            10,
            290,
            30,
            hwnd,
            0, windowState->global_hInstance, 
            0
        );

        windowState->multiPortalLabel = CreateWindow(
            L"Static",
            L"Press 1-9 to use extra portals",
            WS_CHILD | WS_VISIBLE,
            280,
            50,
            290,
            30,
            hwnd,
            0,
            windowState->global_hInstance, 
            0
        );

        windowState->weightBoxSpawnLabel = CreateWindow(
            L"Static",
            L"Press C to create a weighted box",
            WS_CHILD | WS_VISIBLE,
            280,
            90,
            290,
            30,
            hwnd,
            0,
            windowState->global_hInstance, 
            0
        );

    case WM_COMMAND:
        if (LOWORD(wParam) == LOWORD(L"SuperJumpEnabled")) {
            AcquireSRWLockExclusive(&srwlock);
            hackState->superJump = !(hackState->superJump);
            ChangeOnOff(&windowState->superJumpEnabled, hackState->superJump);
            ReleaseSRWLockExclusive(&srwlock);
        }
        else if (LOWORD(wParam) == LOWORD(L"PortalsEnabled")) {
            AcquireSRWLockExclusive(&srwlock);
            hackState->multiPortals = !(hackState->multiPortals);
            ChangeOnOff(&windowState->multiPortalEnabled, hackState->multiPortals);
            ReleaseSRWLockExclusive(&srwlock);
        }
        else if (LOWORD(wParam) == LOWORD(L"CubesEnabled")) {
            AcquireSRWLockExclusive(&srwlock);
            hackState->weightBoxSpawn = !(hackState->weightBoxSpawn);
            ChangeOnOff(&windowState->weightBoxSpawnEnabled, hackState->weightBoxSpawn);
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
        0,                              
        CLASS_NAME,                     
        L"Portal Hack Controller",             
        WS_OVERLAPPEDWINDOW,          

        // Window position
        CW_USEDEFAULT, 
        CW_USEDEFAULT,

        // Window size
        600, 
        250,

        NULL,       
        NULL,       
        hInstance,  
        NULL        
    ); 

    if (hwnd == NULL) {
        MessageBox(NULL, L"Window creation failed", L"Error", MB_ICONERROR);
        return;
    }

    // Create hack config buttons
    HWND superJumpButton = CreateWindow(
        L"BUTTON",
        L"Toggle Super Jump",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        10,
        10,
        200,
        30,
        hwnd,
        (HMENU)L"SuperJumpEnabled",
        hInstance,
        NULL
    ); 

    HWND multiPortalButton = CreateWindow(
        L"BUTTON",
        L"Toggle Multi-Portals",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        10,
        50,
        200,
        30,
        hwnd,
        (HMENU)L"PortalsEnabled",
        hInstance,
        NULL
    );

    HWND weightBoxButton = CreateWindow(
        L"BUTTON",
        L"Toggle Cube Spawning",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        10,
        90,
        200,
        30,
        hwnd,
        (HMENU)L"CubesEnabled",
        hInstance,
        NULL
    );

    CreateWindow(
        L"Static",
        L"Press T to enter the 'Lights Out' minigame.",
        WS_CHILD | WS_VISIBLE,
        10,
        130,
        450,
        30,
        hwnd,
        0,
        hInstance,
        0
    );

    ShowWindow(hwnd, SW_SHOWDEFAULT);

    // Enter the message loop
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

/**
* Switches the portal gun's linkage ID based on user input.
*/ 
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
    SetNetOriginFunc((int*) ent, origin);
    SetAbsOriginFunc((int*) ent, origin);
    *roll = 0;
    *pitch = 0;
    *yaw = 0;
}

/**
* Gets all entities of the weighted box type in the world
*/
vector<void*> GetAllEntities() {
    int h_i = clientEntityList->GetHighestEntityIndex(); // Get the highest index currently in use
    vector<void*> entities = {};
    int i = 0;
    void* current = 0x0;
    int number_blocks = 0;
    bool isFirst = true;
    while (i <= h_i) {
        if ((current = clientEntityList->GetClientEntity(i)) != 0x0) { // Iterate through the entity list, filter out null
            int* e_id = (int*)(((int*)current) + 0x1D);

            auto name = " ";

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

    *(player->position->x) = -1400;
    *(player->position->y) = -2750;

        if (diff > 0) {
        engineServer->ClientCmd("ent_create_portal_weight_box");
        Sleep(200);
    }
    else {
        float base_x = *(player->position->x) + 500;
        float base_y = *(player->position->y) + half;
        float base_z = *(player->position->z) + 75;
        int size_diff = 55;

        float l1i[5] = { 0.515, 0.589, 0.655, 0.720, 0.783 };
        float l2i[5] = { 0.359, 0.433, 0.504, 0.577, 0.642 };
        int li = GetClosest(*((player->position->x) + 0x23), l1i);
        int lj = GetClosest(*((player->position->x) + 0x24), l2i);


        std::cout << lj << ":" << *((player->position->x) + 0x24) << " | " << li << ":" << *((player->position->x) + 0x23) << "\n";
        

        if (GetAsyncKeyState(0x01) & 1) {
            FlipPixels(li, lj);
        }

        if (GetAsyncKeyState('N') & 1) {
            NewLevel();
        }

        int index = 0;
        while (index < (width * height)) {
            int i2 = div(index, 5)
                .quot;
            int j2 = index % 5;
            float z_coord = (base_z + ((div(index, width).quot) * size_diff));
            if (!grid[i2][j2]) {
                z_coord = 5000;
            }
            SetEntityPos(base_x, (base_y - ((index % width) * size_diff)), z_coord, entities[index]);
           
            index++;
        }
    }

}


void InitMiniGame() {
    std::cout << "start game init\n";
    basePos[0] = *(player->position->x);
    basePos[1] = *(player->position->y);
    basePos[2] = *(player->position->z);
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            grid[i][j] = true;
        }
    }
    std::cout << *((player->position->x) + 0x23) << " | " << *((player->position->x) + 0x24) << "\n";
    std::cout << basePos << "\n";
    std::cout << "finish game init\n";
}


DWORD WINAPI MyThread(HMODULE module) {
    AllocConsole();
    FILE* f = new FILE;
    freopen_s(&f, "CONOUT$", "w", stdout);

    std::cout << "Injection active\n";

    // Load interfaces for the engine client and client entity list
    engineServer = (IVEngineClient*)FindInterface(GetModuleHandle(L"engine.dll"), "VEngineClient014");
    clientEntityList = (IClientEntityList*)FindInterface(GetModuleHandle(L"client.dll"), "VClientEntityList003");

    // Identify entity functions
    const char* networkPosPattern = "55 8B EC 8B 45 08 D9 00 D9 99 34 03 00 00 D9 40 04 D9 99 38 03 00 00 D9 40 08 D9 99 3C 03 00 00 5D C2 04 00";
    std::uint8_t* ps = PatternScan(GetModuleHandleW(L"client.dll"), networkPosPattern);

    const char* setPosPattern = "55 8B EC 56 57 8B F1 E8 ? ? ? ? 8B 7D 08 F3 0F 10 07 0F 2E 86 ? ? ? ?";
    SetAbsOriginFunc = (SetAbsOriginType)PatternScan(GetModuleHandleW(L"client.dll"), setPosPattern);

    const char* setLocalPattern = "55 8B EC 56 57 8B 7D 08 8B F1 F3 0F 10 07 0F 2E 86 8C 02 00 00 9F";
    SetLocalOriginFunc = (SetAbsOriginType)PatternScan(GetModuleHandleW(L"client.dll"), setLocalPattern);

    const char* serverLocalPattern = "55 8B EC F3 0F 10 ? ? ? ? ? 83 EC 10 0F 28 C1 0F 57 05 ? ? ? ? 56 8B 75 08 57 8B F9 F3 0F 10 16";
    auto sp = (SetAbsOriginType)SecondPattern(GetModuleHandleW(L"server.dll"), serverLocalPattern);
    SetServerOriginFunc = (SetAbsOriginType)sp;

    const char* modelNamePattern = "8B 81 94 01 00 00 C3";

    SetNetOriginFunc = (SetNetOriginType)ps;

    bool isLoading = false;
    bool miniGame = false;
    bool gameIsInit = false;

    // Main loop
    while (true) {
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
        
        if (!engineServer->IsInGame() && !isLoading) {
            isLoading = true;
        }
        else if (engineServer->IsInGame() && isLoading && !engineServer->IsDrawingLoadingImage()) {
            isLoading = false;
            Sleep(3000);
            
            // Rerun the player and portal gun pointer paths in case anything has changed
            player->ReloadPointers();
            portalGun->ReloadPointers();
        }

        // Gets a lock to read the global hack state 
        AcquireSRWLockShared(&srwlock);

        // Checks if different hacks are active
        if (hackState->superJump) {
            // Super jump when space pressed
            if (GetAsyncKeyState(VK_SPACE) & 1) {
                *player->velocity->z = 500.0f;
            }
        }

        // Allows switching of the portal gun's link ID for multiple portals.
        if (hackState->multiPortals) { 
            PortalIDSwitch(portalGun->linkID);
        }

        // Create a weight box if C pressed
        if (hackState->weightBoxSpawn) {
            if (GetAsyncKeyState('C')) {
                engineServer->ClientCmd("ent_create_portal_weight_box");
                while (GetAsyncKeyState('C')) {
                    Sleep(1);
                }
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
        windowState->global_hInstance = hModule;
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