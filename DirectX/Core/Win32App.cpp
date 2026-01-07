#include "Win32App.h"
#include "DXSample.h"

HWND Win32App::hwnd = nullptr;

int Win32App::Run(DXSample* sample, HINSTANCE hinstance, int cmdshow)
{
	// Parse the Command Line Parameters
	int argc;
	LPWSTR* argv{ CommandLineToArgvW(GetCommandLineW(), &argc) };
	sample->ParseCommandLineArgs(argv, argc);
	LocalFree(argv);

	WNDCLASSEX windowClass{ 0 };
	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.style = CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc = WindowProc;
	windowClass.hInstance = hinstance;
	windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	windowClass.lpszClassName = L"DXSampleClass";
	RegisterClassEx(&windowClass);

	RECT windowRect{
		0, 0,
		static_cast<LONG>(sample->GetWidth()),
		static_cast<LONG>(sample->GetHeight())
	};
	AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

	hwnd = CreateWindow(
		windowClass.lpszClassName,
		sample->GetTitle(),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		windowRect.right - windowRect.left,
		windowRect.bottom - windowRect.top,
		nullptr,
		nullptr,
		hinstance,
		sample
	);
	
	sample->OnInit();

	ShowWindow(hwnd, cmdshow);

	MSG msg{};
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	sample->OnDestroy();

	return static_cast<char>(msg.wParam);
}

LRESULT CALLBACK Win32App::WindowProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	DXSample* sample = reinterpret_cast<DXSample*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
	
	switch (message)
	{
	case WM_CREATE:
	{
		LPCREATESTRUCT pCreateStruct{ reinterpret_cast<LPCREATESTRUCT>(lparam) };
		SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCreateStruct->lpCreateParams));
	}
	return 0;

	case WM_KEYDOWN:
		if (sample)
			sample->OnKeyDown(static_cast<UINT8>(wparam));
		return 0;

	case WM_KEYUP:
		if (sample)
			sample->OnKeyUp(static_cast<UINT8>(wparam));
		return 0;

	case WM_PAINT:
		if (sample)
		{
			sample->OnUpdate();
			sample->OnRender();
		}
		return 0;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hwnd, message, wparam, lparam);
}