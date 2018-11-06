#include <iostream>
#include <stdexcept>
#include <vector>

#include "proxydll.h"
#include "IDirectInput8Hook.h"

#include <tlhelp32.h>

std::vector<GUID> keyboard_guids;

BOOL CALLBACK staticEnumerateKeyboards(LPCDIDEVICEINSTANCE devInst, LPVOID pvRef)
{
	if (!IsEqualGUID(devInst->guidInstance, GUID_SysKeyboard))
	{
		keyboard_guids.push_back(devInst->guidInstance);
	}
	return DIENUM_CONTINUE;
}

IDirectInput8Hook::IDirectInput8Hook(IDirectInput8 * dinput)
{
	m_dinput = dinput;
	HRESULT result = m_dinput->EnumDevices(DI8DEVCLASS_KEYBOARD, &staticEnumerateKeyboards, this, DIEDFL_INCLUDEALIASES);
	if (FAILED(result))
	{
		OutputDebugString(L"2 Critical error: Unable to enumerate input devices!\r\n");
		switch (result)
		{
		case DIERR_INVALIDPARAM:
			OutputDebugString(L"DIERR_INVALIDPARAM\r\n");
			break;
		case DIERR_NOTINITIALIZED:
			OutputDebugString(L"DIERR_NOTINITIALIZED\r\n");
			break;

		}
		::ExitProcess(0);
	}

	for (const auto& guid : keyboard_guids)
	{
		OLECHAR* guidString;
		StringFromCLSID(guid, &guidString);
		std::wcout << "IDirectInput8Hook kb device detected: " << guidString << std::endl;
		::CoTaskMemFree(guidString);
	}
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

HRESULT STDMETHODCALLTYPE IDirectInput8Hook::CreateDevice(REFGUID rguid, LPDIRECTINPUTDEVICE8W * lplpDirectInputDevice, LPUNKNOWN pUknOuter)
{
	OutputDebugString(L"CreateDevice Hook called\n");
	GUID selected_guid = rguid;
	if (IsEqualGUID(rguid, GUID_SysKeyboard))
	{
		TCHAR szFileName[MAX_PATH + 1];
		GetModuleFileName(NULL, szFileName, MAX_PATH + 1);
		std::wstring wsFileName(szFileName);
		wsFileName = wsFileName.substr(wsFileName.find_last_of('\\') + 1);

		int id = ProcessCount(wsFileName.c_str()) - 1;
		if (id >= 0 && id < keyboard_guids.size())
		{
			selected_guid = keyboard_guids[id];

			OLECHAR* guidString;
			StringFromCLSID(selected_guid, &guidString);
			std::wcout << "IDirectInput8Hook::CreateDevice selected device: " << guidString << std::endl;
			::CoTaskMemFree(guidString);
		}
		else
		{
			std::wcout << "IDirectInput8Hook::CreateDevice No suitable device for " << id << " " << wsFileName.c_str() << std::endl;
		}
	}

	// Create the dinput device
	HRESULT hr = m_dinput->CreateDevice(selected_guid, lplpDirectInputDevice, pUknOuter);

	if (SUCCEEDED(hr))
		// Create the proxy device
		*lplpDirectInputDevice = new IDirectInputDevice8Hook(this, *lplpDirectInputDevice, selected_guid);

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