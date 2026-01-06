#include "stdafx.h"
#include "D3D12HelloWindow.h"

_Use_decl_annotations_
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int CmdShow)
{
	D3D12HelloWindow sample(1280, 720, L"Hello Window");
	return Win32App::Run(&sample, hInstance, CmdShow);
}