#include <Windows.h>
#include <shellapi.h>
#include <strsafe.h>
#include <wchar.h>
#include <Commctrl.h>
#include "resource.h"

#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#define WM_TRAYICON (WM_USER + 1)
#define HK_NEXT_SONG 100

HINSTANCE g_hInstance;
HWND g_hWnd;
NOTIFYICONDATA g_nid;

VOID Toeter()
{
	PlaySound(MAKEINTRESOURCE(IDR_TOETER), NULL, SND_ASYNC | SND_RESOURCE);
}

VOID NextSong()
{
	INPUT ip;
	ZeroMemory(&ip, sizeof(ip));
	ip.type = INPUT_KEYBOARD;
	ip.ki.wVk = VK_MEDIA_NEXT_TRACK;

	SendInput(1, &ip, sizeof(ip));

	Toeter();
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	switch (Msg) {
	case WM_COMMAND:
		if (LOWORD(wParam) == ID_TRAY_STOPPEN) {
			PostQuitMessage(0);
		}
		return 0;
	case WM_HOTKEY:
		if (wParam == HK_NEXT_SONG) {
			NextSong();
		}
		return 0;
	case WM_TRAYICON:
		if (lParam == WM_RBUTTONDOWN) {
			HMENU hMenu = LoadMenu(g_hInstance, MAKEINTRESOURCE(IDR_TRAYMENU));

			POINT point;
			GetCursorPos(&point);
			TrackPopupMenu(GetSubMenu(hMenu, 0), TPM_RIGHTALIGN | TPM_RIGHTBUTTON, point.x, point.y, 0, g_hWnd, NULL);
		}
		return 0;
	}
	return DefWindowProc(hWnd, Msg, wParam, lParam);
}

VOID CreateMessageWindow()
{
	WNDCLASSEX wc;
	
	ZeroMemory(&wc, sizeof(wc));

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.hInstance = g_hInstance;
	wc.lpfnWndProc = (WNDPROC)WindowProc;
	wc.lpszClassName = L"ScooterClass";

	RegisterClassEx(&wc);

	g_hWnd = CreateWindowEx(0, L"ScooterClass", L"", 0, 0, 0, 0, 0, HWND_MESSAGE, NULL, g_hInstance, NULL);
}

VOID CreateNotifyIcon()
{
	NOTIFYICONDATA nid;

	ZeroMemory(&nid, sizeof(nid));
	nid.cbSize = sizeof(NOTIFYICONDATA);
	nid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
	nid.hWnd = g_hWnd;
	nid.uCallbackMessage = WM_TRAYICON;

	StringCchCopy(nid.szTip, ARRAYSIZE(nid.szTip), L"Scooter");

	LoadIconMetric(g_hInstance, MAKEINTRESOURCE(IDI_TRAY), LIM_SMALL, &nid.hIcon);

	g_nid = nid;

	Shell_NotifyIcon(NIM_ADD, &nid);
}

VOID DestroyNotifyIcon()
{
	Shell_NotifyIcon(NIM_DELETE, &g_nid);
}

LRESULT CALLBACK KeyboardHook(int code, WPARAM wParam, LPARAM lParam)
{
	if (wParam == VK_MEDIA_NEXT_TRACK) {
		Toeter();
	}

	return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pCmdLine, int nCmdShow)
{
	g_hInstance = hInstance;

	HANDLE hMutex = CreateMutex(NULL, TRUE, L"ScooterMutex");
	if (GetLastError() == ERROR_ALREADY_EXISTS) {
		MessageBox(NULL, L"Ik draai al!", L"Scooter", MB_ICONINFORMATION | MB_OK);
		return 1;
	}

	CreateMessageWindow();
	CreateNotifyIcon();

	RegisterHotKey(g_hWnd, HK_NEXT_SONG, MOD_ALT, VK_RIGHT);

	SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardHook, NULL, 0);

	MSG Msg;
	while (GetMessage(&Msg, NULL, 0, 0)) {
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}

	DestroyNotifyIcon();

	return 0;
}