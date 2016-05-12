#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#pragma comment(lib, "shlwapi")

#include <windows.h>
#include <shlwapi.h>

#include <iostream>
#include <string>
#include <regex>

TCHAR szClassName[] = TEXT("Window");

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static HWND hEdit1, hEdit2, hButton;
	static HWND hStatic1, hStatic2;
	static HWND hCheck;
	switch (msg)
	{
	case WM_CREATE:
		hStatic1 = CreateWindow(TEXT("Static"), TEXT("対象リスト"), WS_VISIBLE | WS_CHILD, 0, 0, 0, 0, hWnd, 0, ((LPCREATESTRUCT)lParam)->hInstance, 0);
		hEdit1 = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("EDIT"), TEXT(""), WS_VISIBLE | WS_CHILD | WS_TABSTOP | WS_VSCROLL | WS_HSCROLL | ES_AUTOHSCROLL | ES_AUTOVSCROLL | ES_MULTILINE | ES_WANTRETURN, 0, 0, 0, 0, hWnd, 0, ((LPCREATESTRUCT)lParam)->hInstance, 0);
		SendMessage(hEdit1, EM_LIMITTEXT, 0, 0);
		hCheck = CreateWindow(TEXT("Button"), TEXT("大小区別"), WS_VISIBLE | WS_CHILD | WS_TABSTOP | BS_AUTOCHECKBOX, 0, 0, 0, 0, hWnd, 0, ((LPCREATESTRUCT)lParam)->hInstance, 0);
		hButton = CreateWindow(TEXT("Button"), TEXT(">重複削除>"), WS_VISIBLE | WS_CHILD | WS_TABSTOP | BS_DEFPUSHBUTTON, 0, 0, 0, 0, hWnd, (HMENU)IDOK, ((LPCREATESTRUCT)lParam)->hInstance, 0);
		hStatic2 = CreateWindow(TEXT("Static"), TEXT("結果リスト"), WS_VISIBLE | WS_CHILD, 0, 0, 0, 0, hWnd, 0, ((LPCREATESTRUCT)lParam)->hInstance, 0);
		hEdit2 = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("EDIT"), TEXT(""), WS_VISIBLE | WS_CHILD | WS_TABSTOP | WS_VSCROLL | WS_HSCROLL | ES_AUTOHSCROLL | ES_AUTOVSCROLL | ES_MULTILINE | ES_READONLY, 0, 0, 0, 0, hWnd, 0, ((LPCREATESTRUCT)lParam)->hInstance, 0);
		SendMessage(hEdit2, EM_LIMITTEXT, 0, 0);
		DragAcceptFiles(hWnd, TRUE);
		break;
	case WM_DROPFILES:
		{
			const HDROP hDrop = (HDROP)wParam;
			const UINT nFiles = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);
			if (nFiles == 1)
			{
				TCHAR szFileName[MAX_PATH];
				DragQueryFile(hDrop, 0, szFileName, sizeof(szFileName));
				const HANDLE hFile = CreateFile(szFileName, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
				if (hFile != INVALID_HANDLE_VALUE)
				{
					const DWORD dwFileSize = GetFileSize(hFile, 0);
					DWORD dwReadSize;
					LPSTR lpszText = (LPSTR)GlobalAlloc(0, dwFileSize + 1);
					ReadFile(hFile, lpszText, dwFileSize, &dwReadSize, 0);
					lpszText[dwReadSize] = 0;
					SetWindowTextA(hEdit1, lpszText);
					GlobalFree(lpszText);
					CloseHandle(hFile);
				}
			}
			DragFinish(hDrop);
		}
		break;
	case WM_SIZE:
		{
			const int button_width = 128;
			const int width = (LOWORD(lParam) - button_width - 20) / 2;
			MoveWindow(hStatic1, 0, 0, width, 32, TRUE);
			MoveWindow(hEdit1, 0, 32, width, HIWORD(lParam) - 32, TRUE);
			MoveWindow(hCheck, width + 10, HIWORD(lParam) / 2 - 5 - 32, button_width, 32, TRUE);
			MoveWindow(hButton, width + 10, HIWORD(lParam) / 2 + 5, button_width, 32, TRUE);
			MoveWindow(hStatic2, width + button_width + 20, 0, width, 32, TRUE);
			MoveWindow(hEdit2, width + button_width + 20, 32, width, HIWORD(lParam) - 32, TRUE);
		}
		break;
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK)
		{
			EnableWindow(hButton, FALSE);
			SetWindowTextA(hEdit2, 0);
			const INT_PTR nEditLineCount1 = SendMessage(hEdit1, EM_GETLINECOUNT, 0, 0);
			const INT_PTR bCaseSensitive = SendMessage(hCheck, BM_GETCHECK, 0, 0);
			BOOL bIsFound;
			std::vector<std::wstring> list;
			for (INT_PTR i = 0; i < nEditLineCount1; ++i)
			{
				bIsFound = FALSE;
				const INT_PTR nEditLineIndex1 = SendMessage(hEdit1, EM_LINEINDEX, i, 0);
				const INT_PTR nEditLineLength1 = SendMessage(hEdit1, EM_LINELENGTH, nEditLineIndex1, 0);
				if (!nEditLineLength1) continue;
				LPTSTR lpszEditLine1 = (LPTSTR)GlobalAlloc(0, sizeof(TCHAR)* (nEditLineLength1 + 1 + 2)); // 改行文字2文字分追加
				*(WORD *)lpszEditLine1 = (WORD)nEditLineLength1;
				SendMessage(hEdit1, EM_GETLINE, i, (LPARAM)lpszEditLine1);
				lpszEditLine1[nEditLineLength1] = 0;
				for (auto v : list)
				{
					if ((bCaseSensitive ? lstrcmp(v.c_str(), lpszEditLine1) : lstrcmpi(v.c_str(), lpszEditLine1)) == 0)
					{
						bIsFound = TRUE;
					}
				}
				if (!bIsFound)
				{
					list.push_back(lpszEditLine1);

					lstrcat(lpszEditLine1, TEXT("\r\n"));
					INT_PTR dwEditTextLength = SendMessageA(hEdit2, WM_GETTEXTLENGTH, 0, 0);
					SendMessage(hEdit2, EM_SETSEL, dwEditTextLength, dwEditTextLength);
					SendMessage(hEdit2, EM_REPLACESEL, 0, (LPARAM)lpszEditLine1);
				}
				GlobalFree(lpszEditLine1);
			}
			SendMessage(hEdit2, EM_SETSEL, 0, -1);
			SetFocus(hEdit2);
			EnableWindow(hButton, TRUE);
		}
		break;
	case WM_CLOSE:
		DestroyWindow(hWnd);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefDlgProc(hWnd, msg, wParam, lParam);
	}
	return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPreInst, LPSTR pCmdLine, int nCmdShow)
{
	MSG msg = { 0 };
	const WNDCLASS wndclass = {
		CS_HREDRAW | CS_VREDRAW,
		WndProc,
		0,
		DLGWINDOWEXTRA,
		hInstance,
		0,
		LoadCursor(0,IDC_ARROW),
		0,
		0,
		szClassName
	};
	RegisterClass(&wndclass);
	const HWND hWnd = CreateWindow(
		szClassName,
		TEXT("入力されたテキストの重複行を削除"),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		0,
		CW_USEDEFAULT,
		0,
		0,
		0,
		hInstance,
		0
	);
	ShowWindow(hWnd, SW_SHOWDEFAULT);
	UpdateWindow(hWnd);
	while (GetMessage(&msg, 0, 0, 0))
	{
		if (!IsDialogMessage(hWnd, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	return (int)msg.wParam;
}
