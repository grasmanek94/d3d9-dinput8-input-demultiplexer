#include <iostream>
#include <stdexcept>
#include <vector>

#include "proxydll.h"
#include "IDirectInput8Hook.h"

std::vector<GUID> keyboard_guids;
GUID system_keyboard;

BOOL CALLBACK staticEnumerateKeyboards(LPCDIDEVICEINSTANCE devInst, LPVOID pvRef)
{
	if (pvRef == NULL)
	{
		system_keyboard = devInst->guidInstance;
	}
	else
	{
		if (!IsEqualGUID(devInst->guidInstance, system_keyboard))
		{
			keyboard_guids.push_back(devInst->guidInstance);
		}
	}
	return DIENUM_CONTINUE;
}

IDirectInput8Hook::IDirectInput8Hook(IDirectInput8 * dinput)
{
	m_dinput = dinput;
	HRESULT result = m_dinput->EnumDevices(DI8DEVCLASS_KEYBOARD, &staticEnumerateKeyboards, 0, DIEDFL_ATTACHEDONLY);
	if (FAILED(result))
	{
		OutputDebugString(L"1 Critical error: Unable to enumerate input devices!\r\n");
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

	result = m_dinput->EnumDevices(DI8DEVCLASS_KEYBOARD, &staticEnumerateKeyboards, this, DIEDFL_INCLUDEALIASES);
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