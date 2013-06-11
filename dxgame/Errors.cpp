#include "StdAfx.h"
#include "Errors.h"

#include <windows.h>
#include <iostream>
#include <sstream>


namespace Errors
{

HWND window;

void Cry(wchar_t *msg)
{
    MessageBoxW(window, msg, L"Error!", MB_OK);
    std::wcerr << msg << std::endl;

    throw(Fatal());
}

void Cry(char *msg)
{
    MessageBoxA(window, msg, "Error!", MB_OK);
    std::cerr << msg << std::endl;

    throw(Fatal());
}


void Cry(char *msg, char *moreMsg)
{
    std::stringstream ss;

    ss << msg << moreMsg;
    Errors::Cry((char*)ss.str().c_str());

    throw(Fatal());
}

}