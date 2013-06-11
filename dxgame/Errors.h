#ifndef ERRORS_H
#define ERRORS_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <exception>

namespace Errors
{
extern HWND window;

void Cry(wchar_t *msg);
void Cry(char *msg);

void Cry(char *msg, char *moreMsg);

class Fatal : public std::exception
{
    char *what()
    {
        return "World-Ending Error. Terminating. Hasta la vista, baby.";
    }
};

}

#endif
