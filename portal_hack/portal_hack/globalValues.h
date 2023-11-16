#pragma once
#include "pch.h"
#include <iostream>
#include <Windows.h>
#include <shared_mutex>
#include "IVEngineServer.h"
#include "IVEngineClient.h"

// Structure used to store the current state of different hacks
// Used to allow the hacks to be controlled by the Window and the updates
// to be recieved by the 
typedef struct HackVariables {
    bool jumpHack = false;
    bool multiplePortalsHack = false;
};

// Used as the signature for the exported CreateInterface function 
typedef void* (__cdecl* CreateInterfaceType)(const char* name, int* returnCode);

// Variables used to control various parts of the Window
typedef struct WindowVairables {
    HINSTANCE global_hInstance;
    HWND on_off1;
    HWND on_off2;
    HWND label1;
    HWND label2;
};

// Modules to different DLLs
HMODULE ClientModule;
HMODULE ServerModule;
HMODULE EngineModule;
HMODULE PhysicsModule;

// Pointers for the DLL modules
uintptr_t ClientPtr;
uintptr_t ServerPtr;
uintptr_t EnginePtr;
uintptr_t PhysicsPtr;

typedef struct Player {
    float* X;
    float* Y;
    float* Z;
    int* onWall;
};

typedef struct Camera {
    float* X;
    float* Y;
    float* Z;
};

typedef struct PortalGun {
    int* linkID;
};