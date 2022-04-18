#pragma once


#pragma comment(lib,"d3d11.lib")
#pragma comment(lib,"DXGI.lib")
#pragma comment (lib, "D2D1.lib")
#pragma comment (lib, "DirectXTK.lib")
#include "..\\ErrorLogger.h"
#include <wrl/client.h>
#include <vector> 
#include <d3d11.h>
#include <d3dcompiler.h>
#include <dinput.h>
#include <dwrite.h>
class AdapterData
{
public:	
	AdapterData(IDXGIAdapter * pAdapter);
	IDXGIAdapter * pAdapter = nullptr;
	DXGI_ADAPTER_DESC description;
};

class AdapterReader
{
public:
	static std::vector<AdapterData> GetAdapters();
private:
	static std::vector<AdapterData> adapters;
};