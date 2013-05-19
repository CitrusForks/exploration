#include "StdAfx.h"
#include "Errors.h"

#include <windows.h>
#include <iostream>


namespace Errors
{

HWND window;

void Cry(wchar_t *msg)
{
    MessageBoxW(window, msg, L"Error!", MB_OK);
    std::wcerr << msg << std::endl;
}

void Cry(char *msg)
{
    MessageBoxA(window, msg, "Error!", MB_OK);
    std::cerr << msg << std::endl;
}


}