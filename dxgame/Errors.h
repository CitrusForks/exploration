#ifndef ERRORS_H
#define ERRORS_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace Errors
{
extern HWND window;

void Cry(wchar_t *msg);
void Cry(char *msg);

void Cry(char *msg, char *moreMsg);

}

#endif
