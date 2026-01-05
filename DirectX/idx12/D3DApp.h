#pragma once

#include <string>
#include <vector> 
#include <Windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>

class D3DApp
{
	void LogOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format);
	void LogAdapterOutputs(IDXGIAdapter* adapter);
public:
	void LogAdapters();

};

#ifndef ReleaseCom
#define ReleaseCom(x) { if(x){ x->Release(); x = 0; } }
#endif