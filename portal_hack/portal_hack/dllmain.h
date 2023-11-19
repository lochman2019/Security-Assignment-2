#pragma once
#include "pch.h"
#include <iostream>
#include <Windows.h>
#include <shared_mutex>
#include "IVEngineServer.h"
#include "IVEngineClient.h"
#include <cstdint>

// Used as the signature for the exported CreateInterface function 
typedef void* (__cdecl* CreateInterfaceType)(const char* name, int* returnCode);

typedef void* (__thiscall* SetAbsOriginType)(int* ent, float* pos);
typedef void* (__thiscall* SetNetOriginType)(int* ent, float* pos);
typedef int (__thiscall* GetModelType)(int* ent);

typedef struct EntityList {
    void* head; 
    int count;
};

class CBaseEntity {
    void SetAbsOriginFunc(float* origin);
};