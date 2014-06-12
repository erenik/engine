
#ifndef WNDPROC_H
#define WNDPROC_H

#include "OS/OS.h"

#ifdef WINDOWS

#include <Windows.h>
#include <WindowsX.h>
#include <tchar.h>

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

#endif // WINDOWS

#endif
