// socket-mid.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#pragma warning(disable:4996)

#include "driver.h"
#include <iostream>
#include <TlHelp32.h>
#include <string>
#include "eftstructs.h"
#include <Dwmapi.h> 
#include <d3dtypes.h>
#include <d3d9.h>
#include <d3dx9.h>
#include "visuals.h"
#include "overlay.h"

using namespace std;
#pragma comment(lib,"Dwmapi.lib")  
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib,"d3dx9.lib")  
#pragma comment(lib,"dwmapi.lib")  

//defining data
int s_width = 2560;
int s_height = 1440;
LPDIRECT3D9 d3d;
LPDIRECT3DDEVICE9 d3ddev;
LPDIRECT3DVERTEXBUFFER9 g_pVB;
HWND hWnd;
HWND twnd;
MARGINS  margin = { 0,0,s_width,s_height };
ID3DXLine* d3dLine;


auto pRender = PRENDER::Instance();
auto gameData = EFTData::Instance();
//render function
void render_scene()
{
	// write
	d3ddev->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);

	d3ddev->BeginScene();    // begins the 3D scene

	static char cTitle[256];
	sprintf_s(cTitle, "Knove | EFT 0.12 ");

	String(42, 72, D3DCOLOR_RGBA(255, 255, 255, 255), true, cTitle);
	//calculate and and draw esp stuff
	pRender->Render();

	if (!gameData->Read())
	{
		gameData->InitOffsets();
		Sleep(4000);
	}

	//draw_string(10, 10, color, pFont, "output");

	d3ddev->EndScene();    // ends the 3D scene

	d3ddev->Present(NULL, NULL, NULL, NULL);   // displays the created frame on the screen
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_PAINT:
		render_scene();
		break;
	case WM_CREATE:
		DwmExtendFrameIntoClientArea(hWnd, &margin);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

string wstring2string(wstring wstr)
{
	string result;
	//获取缓冲区大小，并申请空间，缓冲区大小事按字节计算的  
	int len = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), wstr.size(), NULL, 0, NULL, NULL);
	char* buffer = new char[len + 1];
	//宽字节编码转换成多字节编码  
	WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), wstr.size(), buffer, len, NULL, NULL);
	buffer[len] = '\0';
	//删除缓冲区并返回值  
	result.append(buffer);
	delete[] buffer;
	return result;
}


std::uint32_t find_process_by_id(const std::string& name)
{
	const auto snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (snap == INVALID_HANDLE_VALUE) {
		return 0;
	}

	PROCESSENTRY32 proc_entry{};
	proc_entry.dwSize = sizeof proc_entry;

	auto found_process = false;
	if (!!Process32First(snap, &proc_entry)) {
		do {

			std::wstring s(proc_entry.szExeFile);
			if (name == wstring2string(s)) {
				found_process = true;
				break;
			}
		} while (!!Process32Next(snap, &proc_entry));
	}

	CloseHandle(snap);
	return found_process
		? proc_entry.th32ProcessID
		: 0;
}

void initD3D()
{
	cout << "initD3D";
	d3d = Direct3DCreate9(D3D_SDK_VERSION);    // create the Direct3D interface

	D3DPRESENT_PARAMETERS d3dpp;    // create a struct to hold various device information

	ZeroMemory(&d3dpp, sizeof(d3dpp));    // clear out the struct for use
	d3dpp.Windowed = TRUE;    // program windowed, not fullscreen
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;    // discard old frames
	d3dpp.hDeviceWindow = hWnd;    // set the window to be used by Direct3D
	d3dpp.BackBufferFormat = D3DFMT_A8R8G8B8;     // set the back buffer format to 32-bit
	d3dpp.BackBufferWidth = s_width;    // set the width of the buffer
	d3dpp.BackBufferHeight = s_height;    // set the height of the buffer

	d3dpp.EnableAutoDepthStencil = TRUE;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D16;

	// create a device class using this information and the info from the d3dpp stuct
	d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &d3ddev);

	D3DXCreateLine(d3ddev, &d3dLine);
}

uint32_t EntryMode()
{
	MSG msg;
	//RECT rc;

	while (TRUE)
	{
		ZeroMemory(&msg, sizeof(MSG));
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if (msg.message == WM_QUIT)
			exit(0);


		SetWindowPos(twnd, hWnd, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
		UpdateWindow(hWnd);

		//render your esp
		render_scene();

		Sleep(5);
	}
	return 0;
}



int main()
{
	uint32_t pid = find_process_by_id("EscapeFromTarkov.exe");
	//uint32_t pid = find_process_by_id("readm.exe");

	cout << "成功寻找到目标进程！ PID: " << pid << "\n";

	if (!pid) return 0;

	driver::initialize();
	if (!driver::cache(pid)) return 0;


	// FreeConsole();

	RECT rc;
	twnd = NULL;

	int s_width = 0;
	int s_height = 0;
	while (s_width < 1000) {
		twnd = FindWindow(L"UnityWndClass", 0);
		GetWindowRect(twnd, &rc);
		s_width = rc.right - rc.left;
		s_height = rc.bottom - rc.top;
		cout << s_width << "~" << s_height << "~" << endl;
		Sleep(1000);
	}


	if (twnd != NULL)
	{
		WNDCLASSEX wndclass;
		ZeroMemory(&wndclass, sizeof(WNDCLASSEX)); // Initialises, sets all bits to 0 to remove garbage data
		wndclass.cbClsExtra = NULL;
		wndclass.cbWndExtra = NULL;
		wndclass.cbSize = sizeof(WNDCLASSEX);
		wndclass.style = CS_HREDRAW | CS_VREDRAW;
		wndclass.lpfnWndProc = WindowProc; // Function that will be executed when the window receives a "message" (input). Required! (crashes if set to NULL)
		wndclass.hInstance = NULL;
		wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
		wndclass.hIcon = LoadIcon(0, IDI_APPLICATION);
		wndclass.hIconSm = LoadIcon(0, IDI_APPLICATION);
		wndclass.hbrBackground = (HBRUSH)RGB(0, 0, 0);
		wndclass.lpszClassName = L"Class_RiotWnd";
		RegisterClassEx(&wndclass);

		hWnd = CreateWindowEx(WS_EX_LAYERED | WS_EX_TRANSPARENT, wndclass.lpszClassName, L"", WS_POPUP, rc.left, rc.top, s_width, s_height, NULL, NULL, wndclass.hInstance, NULL);

		// Activate transparency on color black.
		SetLayeredWindowAttributes(hWnd, RGB(0, 0, 0), 0, ULW_COLORKEY);
		SetLayeredWindowAttributes(hWnd, 0, 255, LWA_ALPHA);
		//	SetLayeredWindowAttributes(hWnd, 0, 255, LWA_ALPHA);

		ShowWindow(hWnd, SW_SHOW);
		initD3D();
		Init(d3ddev, g_pVB, d3dLine);

		EntryMode();
	}



	//while (TRUE)
	//{
	//	if (!gameData->Read())
	//	{
	//		gameData->InitOffsets();
	//		Sleep(4000);
	//	}

	//	//Sleep(2);
	//}


	driver::deinitialize();
	/*
	*/
}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单
