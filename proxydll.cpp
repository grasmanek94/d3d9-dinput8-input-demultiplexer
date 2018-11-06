#include <iostream>

// proxydll.cpp
#include "proxydll.h"

// global variables
#pragma data_seg (".d3d9_shared")
myIDirect3DSwapChain9*  gl_pmyIDirect3DSwapChain9;
myIDirect3DDevice9*		gl_pmyIDirect3DDevice9;
myIDirect3D9*			gl_pmyIDirect3D9;
HINSTANCE				gl_hOriginalDll;
HINSTANCE				gl_hThisInstance;
#pragma data_seg ()

HINSTANCE				gl_dinput8_hOriginalDll;

WNDPROC		orig_wndproc;
HWND			orig_wnd = 0;
D3DPRESENT_PARAMETERS	pPresentParam;

BOOL APIENTRY DllMain( HANDLE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	// to avoid compiler lvl4 warnings 
    LPVOID lpDummy = lpReserved;
    lpDummy = NULL;
    
    switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH: InitInstance(hModule); break;
	    case DLL_PROCESS_DETACH: ExitInstance(); break;
        
        case DLL_THREAD_ATTACH:  break;
	    case DLL_THREAD_DETACH:  break;
	}
    return TRUE;
}

VOID (WINAPI *pSleep)(_In_ DWORD dwMilliseconds) = Sleep;
VOID WINAPI MySleep(_In_ DWORD dwMilliseconds){}

void HooksInstall()
{
	LONG error = 0;
	Sleep(0);

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&(PVOID&)pSleep, MySleep);
	error = DetourTransactionCommit();

	if (error != NO_ERROR)
	{
		OutputDebugString(L"PROXYDLL: Sleep detour failed\r\n");
		::ExitProcess(0);
	}
}

static LRESULT CALLBACK wnd_proc(HWND wnd, UINT umsg, WPARAM wparam, LPARAM lparam)
{

	switch (umsg)
	{
	case WM_KILLFOCUS:
		return 0;
	}

	return CallWindowProc(orig_wndproc, wnd, umsg, wparam, lparam);
}

void wndprochook_uninstall(void)
{
	if (orig_wnd != NULL)
	{
		SetWindowLong(orig_wnd, GWL_WNDPROC, (LONG)(UINT_PTR)orig_wndproc);
		orig_wnd = NULL;
	}
}

void wndprochook_maybe_install(HWND wnd)
{
	if (orig_wndproc == NULL || wnd != orig_wnd)
	{
		wndprochook_uninstall();

		orig_wndproc = (WNDPROC)(UINT_PTR)SetWindowLong(wnd, GWL_WNDPROC, (LONG)(UINT_PTR)wnd_proc);
		orig_wnd = wnd;
	}
}

void LoadOriginalDll(const wchar_t* dll_filename, HINSTANCE& original)
{
	wchar_t buffer[MAX_PATH];

	// Getting path to system dir and to d3d8.dll
	::GetSystemDirectory(buffer, MAX_PATH);

	// Append dll name
	wcscat_s(buffer, MAX_PATH, dll_filename);

	// try to load the system's d3d9.dll, if pointer empty
	if (!original)
	{
		original = ::LoadLibrary(buffer);
	}

	// Debug
	if (!original)
	{
		OutputDebugString(L"PROXYDLL: Original DLL not loaded ERROR: ");
		OutputDebugString(dll_filename);
		OutputDebugString(L"\r\n");
		::ExitProcess(0); // exit the hard way
	}
}

// Exported function (faking d3d9.dll's one-and-only export)
IDirect3D9* WINAPI Direct3DCreate9(UINT SDKVersion)
{
	if (!gl_hOriginalDll)
	{
		LoadOriginalDll(L"\\d3d9.dll", gl_hOriginalDll); // looking for the "right d3d9.dll"
		LoadOriginalDll(L"\\dinput8.dll", gl_dinput8_hOriginalDll);
	}
	
	// Hooking IDirect3D Object from Original Library
	typedef IDirect3D9 *(WINAPI* D3D9_Type)(UINT SDKVersion);
	D3D9_Type D3DCreate9_fn = (D3D9_Type) GetProcAddress( gl_hOriginalDll, "Direct3DCreate9");
    
    // Debug
	if (!D3DCreate9_fn) 
    {
        OutputDebugString(L"PROXYDLL: Pointer to original D3DCreate9 function not received ERROR ****\r\n");
        ::ExitProcess(0);
    }

	// Request pointer from Original Dll. 
	IDirect3D9 *pIDirect3D9_orig = D3DCreate9_fn(SDKVersion);
	
	// Create my IDirect3D8 object and store pointer to original object there.
	// note: the object will delete itself once Ref count is zero (similar to COM objects)
	gl_pmyIDirect3D9 = new myIDirect3D9(pIDirect3D9_orig);
	
	HooksInstall();

	AllocConsole();
	BindCrtHandlesToStdHandles(true, true, true);
	std::cout << "Focus keyboards here" << std::endl;

	DInput8HookAttach();

	// Return pointer to hooking Object instead of "real one"
	return (gl_pmyIDirect3D9);
}

void InitInstance(HANDLE hModule) 
{
	OutputDebugString(L"PROXYDLL: InitInstance called.\r\n");
	
	// Initialisation
	gl_hOriginalDll				= NULL;
	gl_hThisInstance			= NULL;
	gl_pmyIDirect3D9			= NULL;
	gl_pmyIDirect3DDevice9		= NULL;
	gl_pmyIDirect3DSwapChain9	= NULL;

	gl_dinput8_hOriginalDll = NULL;

	// Storing Instance handle into global var
	gl_hThisInstance = (HINSTANCE)  hModule;
}

void ExitInstance() 
{    
    OutputDebugString(L"PROXYDLL: ExitInstance called.\r\n");
	
	// Release the system's d3d9.dll
	if (gl_hOriginalDll)
	{
		::FreeLibrary(gl_hOriginalDll);
	    gl_hOriginalDll = NULL;  
	}

	if (gl_dinput8_hOriginalDll)
	{
		::FreeLibrary(gl_dinput8_hOriginalDll);
		gl_dinput8_hOriginalDll = NULL;
	}
}

int WINAPI D3DPERF_BeginEvent(DWORD col, LPCWSTR wszName)
{
	return 0;
}

int WINAPI D3DPERF_EndEvent()
{
	return 0;
}

void WINAPI D3DPERF_SetMarker()
{
	//MessageBox(NULL, "D3DPERF_SetMarker", "D3D9Wrapper", MB_OK);
}

void WINAPI D3DPERF_SetRegion()
{
	//MessageBox(NULL, "D3DPERF_SetRegion", "D3D9Wrapper", MB_OK);
}

void WINAPI D3DPERF_QueryRepeatFrame()
{
	//MessageBox(NULL, "D3DPERF_QueryRepeatFrame", "D3D9Wrapper", MB_OK);
}

void WINAPI D3DPERF_SetOptions(DWORD options)
{
	//MessageBox(NULL, "D3DPERF_SetOptions", "D3D9Wrapper", MB_OK);
}

void WINAPI D3DPERF_GetStatus()
{
	//MessageBox(NULL, "D3DPERF_GetStatus", "D3D9Wrapper", MB_OK);
}