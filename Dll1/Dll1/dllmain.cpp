// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include <iostream>
#include <Windows.h>


DWORD WINAPI MyThread(HMODULE module) {
    AllocConsole();
    FILE* f = new FILE;
    freopen_s(&f, "CONOUT$", "w", stdout);

    std::cout << "Injection active\n";

    uintptr_t ClientPtr = (uintptr_t)GetModuleHandle(L"client.dll");
    uintptr_t firstStep = *(uintptr_t*)(ClientPtr + 0x004EAAC4);
    uintptr_t secondStep = *(uintptr_t*)(firstStep + 0xC);
    uintptr_t thirdStep = *(uintptr_t*)(secondStep + 0x80);
    uintptr_t fourthStep = *(uintptr_t*)(thirdStep + 0x4);
    uintptr_t fifthStep = *(uintptr_t*)(fourthStep + 0x2A4);
    float* camera_x = (float*)(fifthStep + 0x334);
    float* camera_y = (float*)(fifthStep + 0x338);
    float* camera_z = (float*)(fifthStep + 0x33C);

    uintptr_t ServerPtr = (uintptr_t)GetModuleHandle(L"server.dll");
    uintptr_t fS = *(uintptr_t*)(ServerPtr + 0x006E4E94);
    float* playerX = (float*)(fS + 0x304);
    float* playerY = (float*)(fS + 0x308);
    float* playerZ = (float*)(fS + 0x30C);


    while (true) {
        if (GetAsyncKeyState('F') & 1) { //Press F key to go up
            *playerZ = *playerZ + 500;
        }
        if (GetAsyncKeyState('G') & 1) { //Press G key to go down
            *playerZ = *playerZ - 500;
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


