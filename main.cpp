#include <Windows.h>
#include "resource.h"
#include <CommCtrl.h>
#include <string>
#include <Shlwapi.h>
#include "CryptoProvider.h"

#pragma comment(lib, "shlwapi.lib")

#define MAX_STRING_LEN 255

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
void EncryptAndCreateFile();
HWND window, statusBar, buttonFile, buttonEncrypt, encryptFile;
wchar_t file[MAX_PATH] = L"";

//XTEA 128-Bit Key
uint32_t key[4] = { 0x4c257364, 0x7e32236c, 0x2c263339, 0x645b4548 };

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, char* cmdArgs, int cmdShow)
{
	WCHAR szWindowClass[MAX_STRING_LEN] = L"";
	WCHAR szWindowTitle[MAX_STRING_LEN] = L"";

	LoadString(hInst, IDS_WNDCLASS, szWindowClass, MAX_STRING_LEN);
	LoadString(hInst, IDS_WNDTITLE, szWindowTitle, MAX_STRING_LEN);

	WNDCLASSEX wcx;
	wcx.cbClsExtra = 0;
	wcx.cbWndExtra = 0;
	wcx.cbSize = sizeof(WNDCLASSEX);
	wcx.hbrBackground = (HBRUSH)(COLOR_MENU + 1);
	wcx.hCursor = LoadCursor(hInst, IDC_ARROW);
	wcx.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON));
	wcx.hIconSm = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON));
	wcx.hInstance = hInst;
	wcx.lpfnWndProc = WndProc;
	wcx.lpszClassName = szWindowClass;
	wcx.lpszMenuName = 0;
	wcx.style = CS_HREDRAW | CS_VREDRAW;

	RegisterClassEx(&wcx);

	window = CreateWindow(szWindowClass, szWindowTitle, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU, CW_USEDEFAULT, CW_USEDEFAULT, 300, 100, NULL, NULL, hInst, NULL);

	ShowWindow(window, cmdShow);
	UpdateWindow(window);

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{

	switch (msg)
	{
	case WM_CREATE:
	{
		statusBar = CreateWindowEx(0, STATUSCLASSNAME, NULL, WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, hwnd, NULL, GetModuleHandle(NULL), NULL);
		SetWindowText(statusBar, L"Ready!");

		buttonFile = CreateWindowEx(0, L"button", L"Choose File", WS_CHILD | WS_VISIBLE, 50, 10, 90, 25, hwnd, (HMENU)IDB_CHOOSEFILE, GetModuleHandle(NULL), NULL);
		encryptFile = CreateWindowEx(0, L"button", L"Encrypt", WS_CHILD | WS_VISIBLE, 150, 10, 90, 25, hwnd, (HMENU)IDB_ENCRYPTFILE, GetModuleHandle(NULL), NULL);
		EnableWindow(encryptFile, false);
		break;
	}
	case WM_COMMAND:
		switch (LOWORD(wparam))
		{
		case IDB_CHOOSEFILE:
			if (HIWORD(wparam) == BN_CLICKED)
			{
				OPENFILENAME of;
				memset(&of, 0, sizeof(of));
				of.lStructSize = sizeof(of);
				of.hwndOwner = hwnd;
				of.lpstrFile = file;
				of.lpstrFilter = L"All files (*.*)\0*.*\0";
				of.nMaxFile = MAX_PATH;
				of.nFilterIndex = 0;
				of.lpstrFileTitle = NULL;
				of.nMaxFileTitle = 0;
				of.lpstrInitialDir = NULL;
				of.lpstrDefExt = L"";
				of.Flags = OFN_HIDEREADONLY | OFN_EXPLORER;
				of.lpstrTitle = NULL;

				BOOL rc = GetOpenFileName(&of);
				if (rc)
				{
					if (PathFileExists(file))
					{
						EnableWindow(encryptFile, true);
						SetWindowText(statusBar, (std::wstring(L"File: ")+PathFindFileName(file)).c_str());
					}
					else
						SetWindowText(statusBar, L"Invalid file!");
				}
			}
			break;
		case IDB_ENCRYPTFILE:
			if (HIWORD(wparam) == BN_CLICKED)
			{
				if (!PathFileExists(file))
				{
					EnableWindow(encryptFile, false);
					SetWindowText(statusBar, L"Invalid file!");
				}
				else
				{
					EnableWindow(buttonFile, false);
					SetWindowText(statusBar, L"Encrypting...");
					CreateThread(0, 0, (LPTHREAD_START_ROUTINE)EncryptAndCreateFile, 0, 0, 0);
				}
			}
			break;
		}
		break;
	case WM_PAINT:
		break;
	case WM_CLOSE:
		DestroyWindow(hwnd);
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hwnd, msg, wparam, lparam);
	}

	return 0;
}

void EncryptAndCreateFile()
{
	std::wstring imFile = file;

	HANDLE hfile = CreateFile(imFile.c_str(), GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	DWORD size;
	if (GetFileAttributes(imFile.c_str()) &FILE_ATTRIBUTE_COMPRESSED)
		size = GetCompressedFileSize(imFile.c_str(), 0);
	else
		size = GetFileSize(hfile, 0);

	BYTE* buffer = new BYTE[size];
	DWORD bytesread;
	ReadFile(hfile, buffer, size, &bytesread, 0);
	unsigned int outsize;
	BYTE* encryptedFile = Encrypt(buffer, size, key, outsize);
	std::wstring extension = PathFindExtension(imFile.c_str());
	PathRemoveExtension((wchar_t*)imFile.c_str());
	std::wstring fileName = PathFindFileName(imFile.c_str());
	PathRemoveFileSpec((wchar_t*)imFile.c_str());
	std::wstring newfile = std::wstring(imFile.c_str()) + L"\\" + fileName + L"_enc" + extension;
	HANDLE hnewfile = CreateFile(newfile.c_str(), GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	WriteFile(hnewfile, encryptedFile, outsize, &bytesread, 0);

	CloseHandle(hfile);
	CloseHandle(hnewfile);
	delete encryptedFile;

	SetWindowText(statusBar, L"Ready!");
	EnableWindow(buttonFile, true);
}