// portal_hack.cpp : Defines the exported functions for the DLL.
//

#include "pch.h"
#include "framework.h"
#include "portal_hack.h"


// This is an example of an exported variable
PORTALHACK_API int nportalhack=0;

// This is an example of an exported function.
PORTALHACK_API int fnportalhack(void)
{
    return 0;
}

// This is the constructor of a class that has been exported.
Cportalhack::Cportalhack()
{
    return;
}
