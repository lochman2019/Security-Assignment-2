// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include <iostream>
#include <Windows.h>

typedef void (WINAPIV* LPFN_DISPLAYHELLO) ();

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
    float* sixthStep = (float*)(fifthStep + 0x33C);

    uintptr_t ServerPtr = (uintptr_t)GetModuleHandle(L"server.dll");
    HINSTANCE ServerDLL = LoadLibraryA("server.dll");
    LPFN_DISPLAYHELLO fnDisplayName = (LPFN_DISPLAYHELLO)GetProcAddress(ServerDLL, "UTIL_GetLocalPlayer()");

    std::cout << fnDisplayName();
    FreeLibrary(ServerDLL);

    std::cout << sixthStep;
    float z_coord = *sixthStep;
    std::cout << z_coord;

    while (true) {
        *sixthStep = 500;
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


