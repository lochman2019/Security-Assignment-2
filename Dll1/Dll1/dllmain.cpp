/**
* This file is the entry point for the Portal hack DLL.
*/
#pragma once
#include "pch.h"
#include <iostream>
#include <Windows.h>
#include <shared_mutex>
#include "IVEngineServer.h"
#include "IVEngineClient.h"

// Used as the signature for the exported CreateInterface function 
typedef void* (__cdecl* CreateInterfaceType)(const char* name, int* returnCode);


IVEngineClient* engineClient;
IVEngineServer* engineServer;

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
public:
    Vector3* position;
    Vector3* velocity;
    int* onWall;
    bool flying;
    Player() {
        position = new Vector3();
        velocity = new Vector3();
        flying = false;
    }
};

/**
* A wrapper for pointers to various fields on the portal gun object.
*/
class PortalGun {
public:
    int* linkID;
};

// Switches the portal linkage ID
void switchPortalID(int* portalId) {
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

void* FindInterface(HMODULE dll, const char* name) {
    auto a = GetProcAddress(dll, "CreateInterface");
    if (a == NULL) {
        std::cout << "Interface does not exist.\n";
    }
    CreateInterfaceType CreateInterfaceFunction = (CreateInterfaceType)a;
    std::cout << "Got factory.\n";

    int returnCode = 0;
    void* interface = CreateInterfaceFunction(name, &returnCode);

    return interface;
}

DWORD WINAPI MyThread(HMODULE module) {
    AllocConsole();
    FILE* f = new FILE;
    freopen_s(&f, "CONOUT$", "w", stdout);

    std::cout << "Injection successful!\n";


    // Get pointers to the physics.dll and server.dll modules
    uintptr_t physics = (uintptr_t)GetModuleHandle(L"vphysics.dll");
    uintptr_t server = (uintptr_t)GetModuleHandle(L"server.dll");

    engineClient = (IVEngineClient*)FindInterface(GetModuleHandle(L"engine.dll"), "VEngineClient014");
    engineServer = (IVEngineServer*)FindInterface(GetModuleHandle(L"engine.dll"), "VEngineServer021");

    // A pointer to a memory region containing numerous player object fields
    uintptr_t playerRegion = *(uintptr_t*)(server + 0x006E4E94);
   
    Player* player = new Player();

    // Wrap fields in our player class
    player->velocity->x = (float*)(playerRegion + 0x214);
    player->velocity->y = (float*)(playerRegion + 0x218);
    player->velocity->z = (float*)(playerRegion + 0x21C);
    
    player->position->x = (float*)(playerRegion + 0x304);
    player->position->y = (float*)(playerRegion + 0x308);
    player->position->z = (float*)(playerRegion + 0x30C);

    // Pointer to the portal gun's ID
    uintptr_t s1 = *(uintptr_t*)(server + 0x006DD15C);
    uintptr_t s2 = *(uintptr_t*)(s1 + 0X0);
    uintptr_t s3 = *(uintptr_t*)(s2 + 0x50);
    uintptr_t s4 = *(uintptr_t*)(s3 + 0x13C);
    uintptr_t s5 = *(uintptr_t*)(s4 + 0x14);
    uintptr_t s6 = *(uintptr_t*)(s5 + 0x18);
    uintptr_t s7 = *(uintptr_t*)(s6 + 0xC);
    
    PortalGun* portalGun = new PortalGun();
    portalGun->linkID = (int*)(s7 + 0x598);

    // Is player on wall
    uintptr_t t1 = *(uintptr_t*)(physics + 0x00D492C);
    uintptr_t t2 = *(uintptr_t*)(t1 + 0x0);
    uintptr_t t3 = *(uintptr_t*)(t2 + 0xB0);
    uintptr_t t4 = *(uintptr_t*)(t3 + 0x4);

    player->onWall = (int*)(t4 + 0x104);

    float targetZ = 0.0f;

    // mod portal hack to autoincrement id
    // check cheatengine for wall flag
    while (true) {
        switchPortalID(portalGun->linkID);

        if (player->flying) {
            // If the player is below the target height, increase their vertical velocity
            if (*player->position->z < targetZ) {
                *player->velocity->z = 200.0f;
            }
            else {
                *player->velocity->z = 0.0f;
            }
        }
        
        // Super jump if e pressed
        if (GetAsyncKeyState(VK_SPACE) & 1) {    
            std::cout << "used super jump\n";
            *player->velocity->z = 500.0f;
        }

        if (GetAsyncKeyState('C')) {
            std::cout << "Pressed C";
            engineClient->ClientCmd("ent_create_portal_weight_box"); // Executes a game console command
            while (GetAsyncKeyState('C')) {
                Sleep(1);
            }
        }

        if (GetAsyncKeyState('L')) {
            const char* lvlName = engineClient->GetLevelName();
            std::cout << lvlName;
            std::cout << *lvlName
            //engineServer->ChangeLevel('1', '1');
        }

        // Levitate if c pressed
        if (GetAsyncKeyState('F') & 1) { 
            player->flying = !player->flying;
            targetZ = *player->position->z + 30.0f;

            std::cout << "flying: ";
            std::cout << player->flying;
            std::cout << "\n";
        }
    }

}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    {
        CloseHandle(CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)MyThread, hModule, 0, nullptr));
    }
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}


