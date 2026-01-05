#pragma once

#include "DXSample.h"
#include "stdafx.h"

class DXSample;

class Win32App
{
	static HWND hwnd;

public:
	static int Run(DXSample* sample, HINSTANCE hinstance, int cmdshow);
	static HWND GetHwnd() { return hwnd; };

protected:
	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
};