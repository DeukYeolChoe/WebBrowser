#define WIN32_LEAN_AND_MEAN

#include "socket.h"
#include "parser.h"
#include "validate.h"

using namespace std;
#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")

#define SIZE 2048

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK EditSubProc1(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);

HINSTANCE g_hinst;
LPCTSTR lpszClass = TEXT("Client");
HWND hEdit_ip;
WNDPROC OldEditProc_ip;
HFONT hFont, hTmp;

char g_buf[SIZE];

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPreInstance, LPSTR lpszCmdParam, int nCmdShow)
{
	MSG Message;
	WNDCLASS WndClass;
	g_hinst = hInstance;

	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	WndClass.hInstance = hInstance;
	WndClass.lpfnWndProc = WndProc;
	WndClass.lpszClassName = lpszClass;
	WndClass.lpszMenuName = NULL;
	WndClass.style = CS_HREDRAW | CS_VREDRAW;
	RegisterClass(&WndClass);

	HWND hWnd = CreateWindow(lpszClass, lpszClass, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, (HMENU)NULL, hInstance, NULL);
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	while (GetMessage(&Message, NULL, 0, 0))
	{
		TranslateMessage(&Message);
		DispatchMessage(&Message);
	}
	return (int)Message.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	PAINTSTRUCT ps;
	int len, n, x, y;
	char str[SIZE];
	char* split;

	switch (iMessage)
	{
	case WM_CREATE:
		//www.naver.com
		hEdit_ip = CreateWindowEx(
			WS_EX_WINDOWEDGE, "EDIT", "http://52.192.132.151:8888/index.html", WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER | ES_LEFT, 10, 10, 400, 30, hWnd, (HMENU)(101), g_hinst, NULL);
		SetFocus(hEdit_ip);
		//¼­ºêÅ¬·¡½Ì
		OldEditProc_ip = (WNDPROC)SetWindowLongPtr(hEdit_ip, GWLP_WNDPROC, (LONG_PTR)EditSubProc1);
		return 0;
	case WM_PAINT:
		x = 40;
		y = 40;
		hdc = BeginPaint(hWnd, &ps);
		if (g_buf != NULL && g_buf != "" && strlen(g_buf) > 0)
		{
			split = strtok(g_buf, ";");
			while (split != NULL)
			{
				TextOut(hdc, x, y, split, strlen(split));
				split = strtok(NULL, ";");
				y += 30;
			}
		}
		EndPaint(hWnd, &ps);
		return 0;
	case WM_DESTROY:
		SetWindowLongPtr(hEdit_ip, GWLP_WNDPROC, (LONG_PTR)EditSubProc1);
		DeleteObject(hFont);
		PostQuitMessage(0);
		return 0;
	}
	return (DefWindowProc(hWnd, iMessage, wParam, lParam));
}

LRESULT CALLBACK EditSubProc1(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	char url[SIZE];
	int n;
	Socket socket;
	Parser parser;
	string temp;

	switch (iMessage)
	{
	case WM_KEYDOWN:
		if (wParam == VK_RETURN)
		{
			GetWindowText(hEdit_ip, url, strlen(url));
			temp = parser.getValueFromHTMLTag("asfsdf");
			n = socket.ClientToServer(url, hWnd);
			SetFocus(hEdit_ip);
		}
		break;
	}
	return CallWindowProc(OldEditProc_ip, hWnd, iMessage, wParam, lParam);
}