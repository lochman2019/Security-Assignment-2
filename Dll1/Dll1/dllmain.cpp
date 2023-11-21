// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include <iostream>
#include <Windows.h>
#include <cmath>
#include <thread>
#include <chrono>
#include <ctime>

const float SUPER_SPRINT_SF = 1.2f;
const double pi = 2 * acos(0.0);
bool timerecord = false;
float xg;
float yg;
float zg;


void realstatus() {
    uintptr_t ServerPtr = (uintptr_t)GetModuleHandle(L"server.dll");
    uintptr_t fS = *(uintptr_t*)(ServerPtr + 0x006E4E94);
    float* playerVelocityX = (float*)(fS + 0x214);
    float* playerVelocityY = (float*)(fS + 0x218);
    float* playerVelocityZ = (float*)(fS + 0x21C);
    float* playerX = (float*)(fS + 0x304);
    float* playerY = (float*)(fS + 0x308);
    float* playerZ = (float*)(fS + 0x30C);
    int counter = 0;
    do {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        counter++;
    } while (*playerVelocityX == 0 && *playerVelocityY == 0 && *playerVelocityZ == 0 && counter < 6);
    timerecord = false;
    if (counter >=6) {
        xg = *playerX;
        yg = *playerY;
        zg = *playerZ;
        std::cout << "recorded as: (" << xg << "," << yg << "," << zg << ")" << std::endl;
    }
    /*std::time_t curr_time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    char str[26] = {};
    ctime_s(str, 26, &curr_time);
    std::cout << "Thread end: " << str << std::endl;*/
}

void monitorVelocity() {
    uintptr_t ServerPtr = (uintptr_t)GetModuleHandle(L"server.dll");
    uintptr_t fS = *(uintptr_t*)(ServerPtr + 0x006E4E94);
    float* playerVelocityX = (float*)(fS + 0x214);
    float* playerVelocityY = (float*)(fS + 0x218);
    float* playerVelocityZ = (float*)(fS + 0x21C);

    while (true) {

        if ((*playerVelocityX == *playerVelocityY == *playerVelocityZ == 0) && !timerecord) {
            timerecord = true;
            std::thread t1(realstatus);
            /*std::cout << "thread created" << std::endl;
            std::time_t curr_time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            char str[26] = {};
            ctime_s(str, 26, &curr_time);
            std::cout << "time: " << str << std::endl;*/
            t1.join(); //ctime::ctime_s(&curr_time)
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

}

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
    float* camera_x = (float*)(fifthStep + 0x334); //adjust
    float* camera_y = (float*)(fifthStep + 0x338); //
    //float* camera_y = (float*)(ClientPtr + 0x458090);
    float* camera_z = (float*)(fifthStep + 0x33C); //

    uintptr_t ServerPtr = (uintptr_t)GetModuleHandle(L"server.dll");
    uintptr_t fS = *(uintptr_t*)(ServerPtr + 0x006E4E94);
    float* playerVelocityX = (float*)(fS + 0x214);
    float* playerVelocityY = (float*)(fS + 0x218);
    float* playerVelocityZ = (float*)(fS + 0x21C);
    float* playerX = (float*)(fS + 0x304);
    float* playerY = (float*)(fS + 0x308);
    float* playerZ = (float*)(fS + 0x30C);

    float* id = (float*)(fS + 0x104);

    float targetZ = 0.0f;
    bool levitating = false;
    bool superSprint = false;
    bool recorder = false;

    uintptr_t EnginePtr = (uintptr_t)GetModuleHandle(L"engine.dll");
    float* look_x = (float*)(EnginePtr + 0x45808C);
    float* look_y = (float*)(EnginePtr + 0x458090);
    float* look_z = (float*)(EnginePtr + 0x458094);

    //implement a timer

    //end

    std::thread mv_thread(&monitorVelocity);

    while (true) {
        if (levitating) {
            if (*playerZ < targetZ) {
                *playerVelocityZ = 200.0f;
            }
            else {
                *playerVelocityZ = 0.0f;
            }
        }


        if (superSprint) {
            *playerVelocityX *= SUPER_SPRINT_SF;
            *playerVelocityY *= SUPER_SPRINT_SF;
        }

        if (GetAsyncKeyState('G') & 1) {    // scale player velocity for sprint if g pressed
            superSprint = !superSprint;
            *playerVelocityX = 900.0f;

            std::cout << "super sprint: ";
            std::cout << superSprint;
        }

        if (GetAsyncKeyState('I')) {
            std::cout << (ServerPtr + 0x1001ACE0);
        }

        if (GetAsyncKeyState('J') & 1) {    // Super jump if j pressed
            std::cout << "super jump";
            *playerVelocityZ = 900.0f;
            //*playerVelocityY = 600.0f;
        }

        if (GetAsyncKeyState('C') & 1) { // Levitate if c pressed
            levitating = !levitating;
            targetZ = *playerZ + 30.0f;

            std::cout << "levitate: ";
            std::cout << levitating;
        }

        if (GetAsyncKeyState('R') & 1) {
            //reverse
            std::cout << "Reverse: ";
            float xdiff = *playerX - xg;
            float ydiff = *playerY - yg;
            float zdiff = *playerZ - zg;
            *playerX -= xdiff;
            *playerY -= ydiff;
            *playerZ -= zdiff;
        } //there are some bug with this, which cause the room to change bad

        if (GetAsyncKeyState('B') & 1) {
            //call a function of sub_1001ACE0+11 using the relevant address
            //104C9C40
            char text[] = "ent_create_portal_weight_box";
            //haven't figure out yet, answer is to use library function to spawn a metal block
        }

        //skip by moving steps along fixed axis
        //optional to use
        
        if (GetAsyncKeyState('2') & 1) {
            *playerY += 33.33f;
        }

        if (GetAsyncKeyState('1') & 1) {
            *playerY -= 33.33f;
        }

        if (GetAsyncKeyState('4') & 1) {
            *playerX += 33.33f;
        }

        if (GetAsyncKeyState('3') & 1) {
            *playerX -= 33.33f;
        }
        //end
        if (GetAsyncKeyState('6') & 1) { //lose height
            *playerZ += 33.33f;
        }

        if (GetAsyncKeyState('5') & 1) { //gain height
            *playerZ -= 33.33f;
        }

        if (GetAsyncKeyState('7') & 1) {
            std::cout << "Your looking direction: " << *camera_x << "," << *camera_y << "," << *camera_z << "." << std::endl;
            std::cout << "Your player direction: " << *playerX << "," << *playerY << "," << *playerZ << "." << std::endl;
            std::cout << "Data x: " << *look_x << "Data y: " << *look_y << "Data z: " << *look_z << std::endl;
        }
        //move by your camera direction
        if (GetAsyncKeyState('0') & 1) {
            //move in front of you for total of distance 33.33f
            float angle = (float) ( (*look_y) * (pi/180)); //convert angle to radians
            float x_axis = 33.33f * cos(angle);
            float y_axis = 33.33f * sin(angle);
            float anglex = (float)((*look_x) * (pi / 180));
            float z_axis = 33.33f * sin(-anglex);  //-89.00 on top; 89.00 to the floor sin(-x)
            std::cout << "y-axis face: " << y_axis << ", x-axis face: " << x_axis << std::endl;
            *playerX += x_axis;
            *playerY += y_axis;
            *playerZ += z_axis;
        }

    }

    mv_thread.join();

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
        CloseHandle(CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)MyThread, hModule, 0, nullptr));
    }
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
