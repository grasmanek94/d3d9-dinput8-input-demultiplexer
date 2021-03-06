#include <iostream>
#include <stdexcept>
#include <vector>

#include "proxydll.h"
#include "IDirectInput8Hook.h"

#include <tlhelp32.h>

int chosen_kb_idx = -1;

int ProcessCount(const wchar_t *processName)
{
	int count = 0;
	PROCESSENTRY32 entry;
	entry.dwSize = sizeof(PROCESSENTRY32);

	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

	if (Process32First(snapshot, &entry))
		while (Process32Next(snapshot, &entry))
			if (!wcsicmp(entry.szExeFile, processName))
				++count;

	CloseHandle(snapshot);
	return count;
}

IDirectInput8Hook::IDirectInput8Hook(IDirectInput8 * dinput)
{
	m_dinput = dinput;

	TCHAR szFileName[MAX_PATH + 1];
	GetModuleFileName(NULL, szFileName, MAX_PATH + 1);
	std::wstring wsFileName(szFileName);
	wsFileName = wsFileName.substr(wsFileName.find_last_of('\\') + 1);

	chosen_kb_idx = ProcessCount(wsFileName.c_str()) - 1;
}

HRESULT STDMETHODCALLTYPE IDirectInput8Hook::QueryInterface(REFIID riid, LPVOID * ppvObj)
{
	return m_dinput->QueryInterface(riid, ppvObj);
}

ULONG STDMETHODCALLTYPE IDirectInput8Hook::AddRef()
{
	return m_dinput->AddRef();
}

ULONG STDMETHODCALLTYPE IDirectInput8Hook::Release()
{
	ULONG uRet = m_dinput->Release();

	if (uRet == 0)
		// If the reference count is 0 delete ourselves
		delete this;

	return uRet;
}

HRESULT STDMETHODCALLTYPE IDirectInput8Hook::CreateDevice(REFGUID rguid, LPDIRECTINPUTDEVICE8W * lplpDirectInputDevice, LPUNKNOWN pUknOuter)
{
	OutputDebugString(L"CreateDevice Hook called\n");

	// Create the dinput device
	HRESULT hr = m_dinput->CreateDevice(rguid, lplpDirectInputDevice, pUknOuter);

	if (SUCCEEDED(hr))
		// Create the proxy device
		*lplpDirectInputDevice = new IDirectInputDevice8Hook(this, *lplpDirectInputDevice, rguid);

	return hr;
}

HRESULT STDMETHODCALLTYPE IDirectInput8Hook::EnumDevices(DWORD dwDevType, LPDIENUMDEVICESCALLBACKW lpCallback, LPVOID pvRef, DWORD dwFlags)
{
	return m_dinput->EnumDevices(dwDevType, lpCallback, pvRef, dwFlags);
}

HRESULT STDMETHODCALLTYPE IDirectInput8Hook::GetDeviceStatus(REFGUID rguidInstance)
{
	OutputDebugString(L"GetDeviceStatus\r\n");
	return m_dinput->GetDeviceStatus(rguidInstance);
}

HRESULT STDMETHODCALLTYPE IDirectInput8Hook::RunControlPanel(HWND hwndOwner, DWORD dwFlags)
{
	return m_dinput->RunControlPanel(hwndOwner, dwFlags);
}

HRESULT STDMETHODCALLTYPE IDirectInput8Hook::Initialize(HINSTANCE hinst, DWORD dwVersion)
{
	return m_dinput->Initialize(hinst, dwVersion);
}

HRESULT STDMETHODCALLTYPE IDirectInput8Hook::FindDevice(REFGUID rguidClass, LPCWSTR ptszName, LPGUID pguidInstance)
{
	return m_dinput->FindDevice(rguidClass, ptszName, pguidInstance);
}

HRESULT STDMETHODCALLTYPE IDirectInput8Hook::EnumDevicesBySemantics(LPCWSTR ptszUserName, LPDIACTIONFORMATW lpdiActionFormat, LPDIENUMDEVICESBYSEMANTICSCBW lpCallback, LPVOID pvRef, DWORD dwFlags)
{
	return m_dinput->EnumDevicesBySemantics(ptszUserName, lpdiActionFormat, lpCallback, pvRef, dwFlags);
}

HRESULT STDMETHODCALLTYPE IDirectInput8Hook::ConfigureDevices(LPDICONFIGUREDEVICESCALLBACK lpdiCallback, LPDICONFIGUREDEVICESPARAMSW lpdiCDParams, DWORD dwFlags, LPVOID pvRefData)
{
	return m_dinput->ConfigureDevices(lpdiCallback, lpdiCDParams, dwFlags, pvRefData);
}