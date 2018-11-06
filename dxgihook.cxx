#include <dxgi.h>
#include "detours.h"
#include <cstdio>

class CDetour
{
public:
	HRESULT Mine_Target(DXGI_ADAPTER_DESC* pDesc);
	static HRESULT (CDetour::* Real_Target)(DXGI_ADAPTER_DESC* pDesc);
};

HRESULT CDetour::Mine_Target(DXGI_ADAPTER_DESC* pDesc)
{
	MessageBoxA(NULL, "Using DXGI", "HOOK", 0);
	return (this->*Real_Target)(pDesc);
}

HRESULT (CDetour::* CDetour::Real_Target)(DXGI_ADAPTER_DESC* pDesc) = (HRESULT (CDetour::*)(DXGI_ADAPTER_DESC* pDesc))&IDXGIAdapter::GetDesc;

void HookDXGI()
{
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());

	auto x = &CDetour::Mine_Target;
	DetourAttach(&(PVOID&)CDetour::Real_Target,
		*(PVOID*)(&x));

	LONG l = DetourTransactionCommit();
	printf("DetourTransactionCommit = %d\n", l);
}