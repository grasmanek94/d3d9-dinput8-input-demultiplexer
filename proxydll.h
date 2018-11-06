// proxydll.h
#pragma once

#define WIN32_LEAN_AND_MEAN		
#include <windows.h>

#include "d3d9.h"
#include "myIDirect3D9.h"
#include "myIDirect3DDevice9.h"
#include "myIDirect3DSwapChain9.h"
#include "dxgihook.hxx"
#include "detours.h"

// Exported function
IDirect3D9* WINAPI Direct3DCreate9 (UINT SDKVersion);

// regular functions
void InitInstance(HANDLE hModule);
void ExitInstance(void);
void LoadOriginalDll(void);

void wndprochook_maybe_install(HWND wnd);
extern D3DPRESENT_PARAMETERS pPresentParam;