// dinput8.cpp : Defines the exported functions for the DLL application.
//

#include "proxydll.h"
#include "IDirectInput8Hook.h"

// Real function pointer
HRESULT(WINAPI *RealDirectInput8Create)(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID *ppvOut, LPUNKNOWN punkOuter) = NULL;

// DirectInput8Create
extern "C" HRESULT WINAPI MyDirectInput8Create(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID *ppvOut, LPUNKNOWN punkOuter)
{
	// Log call
	OutputDebugString(L"DirectInput8Create hook has been called!\n");

	// Create DirectInput8 instance
	HRESULT hr = RealDirectInput8Create(hinst, dwVersion, riidltf, ppvOut, punkOuter);

	// Fetch real DirectInput8 instance
	IDirectInput8 * realDirectInput8Interface = (IDirectInput8 *)*ppvOut;

	// Wrap it in our proxy class
	*ppvOut = new IDirectInput8Hook(realDirectInput8Interface);

	// Return creation result
	return hr;
}

// Entry point
void DInput8HookAttach()
{
	// Fetch the real function pointer
	RealDirectInput8Create = (HRESULT(WINAPI *)(HINSTANCE, DWORD, REFIID, LPVOID*, LPUNKNOWN))GetProcAddress(gl_dinput8_hOriginalDll, "DirectInput8Create");

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&(PVOID&)RealDirectInput8Create, MyDirectInput8Create);
	if(DetourTransactionCommit() != NO_ERROR)
	{
		OutputDebugString(L"PROXYDLL: DInputCreate detour failed\r\n");
		::ExitProcess(0);
	}

	// Log initialization
	OutputDebugString(L"Initialized dinput8.dll proxy module!\n");
}
