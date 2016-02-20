#define WIN32_LEAN_AND_MEAN

#include "socket.h"
#include "parser.h"
#include "validate.h"
#include "imager.h"
#include "tree.h"
#include <sstream>
#include <codecvt>
#include <shellapi.h>

using namespace std;

#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")
//하이퍼링크
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64 
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#pragma endregion 


#define YMOVER 40
#define XMOVER 40

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK EditSubProc1(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);
//int setResponse(HWND hWnd, HDC hdc, Socket socket, int i);
int getCmdFromHtmlTag(wstring tag);
void traveralExecute(HWND hWnd, HDC hdc, Node root, Socket socket);
//int executeCmd(HWND hWnd, HDC hdc, Socket socket, wstring tag, wstring content, ImgInfo imgInfo, InputInfo inputInfo);
int executeCmd(HWND hWnd, HDC hdc, Socket socket, wstring tag, wstring content, Attribute attrs);
int errorEventHandler(HWND hWnd, int n);
vector<string> split(string str, char delimiter);
HWND CreateSysLink(HWND hwnd);

HINSTANCE g_hinst;
LPCTSTR lpszClass = TEXT("Web Browser");
HWND hEdit_ip;
WNDPROC OldEditProc_ip;
HWND parent_hWnd;

vector<Node> roots;
vector<HWND> v_inputEdit;
vector<InputAttr> v_inputInfo;
Socket socketObj;
wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;

int x; int y; int g_index; 
int input_id; int submitId;
int xPos; int yPos; int xMax; int yMax;
int xCur; int yCur;
int colormsg;
bool decrese; bool increse;
bool disableWMPaint;
INITCOMMONCONTROLSEX iccex;
HFONT myfont;

//string action; string method;

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPreInstance, LPSTR lpszCmdParam, int nCmdShow)
{
	/*  GDI+ startup token */
	ULONG_PTR gdiplusStartupToken;
	Gdiplus::GdiplusStartupInput gdiInput;
	Gdiplus::GdiplusStartup(&gdiplusStartupToken, &gdiInput, NULL);
	input_id = 40000;
	decrese = false;
	increse = false;
	disableWMPaint = false;
	xCur = XMOVER; yCur = YMOVER;
	submitId = -1;
	g_index = -1;

	//하이퍼링크
	iccex.dwICC = ICC_LINK_CLASS;
	iccex.dwSize = sizeof(iccex);
	InitCommonControlsEx(&iccex);

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

	parent_hWnd = CreateWindow(lpszClass, lpszClass, WS_OVERLAPPEDWINDOW | WS_VSCROLL | WS_HSCROLL, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, (HMENU)NULL, hInstance, NULL);
	ShowWindow(parent_hWnd, nCmdShow);
	UpdateWindow(parent_hWnd);

	while (GetMessage(&Message, NULL, 0, 0))
	{
		TranslateMessage(&Message);
		DispatchMessage(&Message);
	}
	Gdiplus::GdiplusShutdown(gdiplusStartupToken);
	return (int)Message.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	Parser parser;
	Imager imager;
	PAINTSTRUCT ps;
	HDC hdc;
	Tree domTree;

	int xInc; int yInc;

	switch (iMessage)
	{
		case WM_CREATE:
		{
			//http://52.192.132.151:8888/example0.html
			//http://202.179.177.22/
			hEdit_ip = CreateWindowEx(
				WS_EX_WINDOWEDGE, L"EDIT", L"http://www.naver.com", WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER | ES_LEFT, 80, 10, 600, 30, hWnd, (HMENU)(101), g_hinst, NULL);

			CreateWindow(L"button", L"<-", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 10, 10, 30, 30, hWnd, (HMENU)0, g_hinst, NULL);
			CreateWindow(L"button", L"->", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 40, 10, 30, 30, hWnd, (HMENU)1, g_hinst, NULL);
			
			//스크롤 설정
			xPos = 0; yPos = 0; xMax = 1024; yMax = 768;
			SetScrollRange(hWnd, SB_VERT, 0, yMax, TRUE);
			SetScrollPos(hWnd, SB_VERT, 0, TRUE);
			SetScrollRange(hWnd, SB_HORZ, 0, xMax, TRUE);
			SetScrollPos(hWnd, SB_HORZ, 0, TRUE);

			//하이퍼
			//hwndStatic = CreateWindow("Static", "This is a hyperlink", SS_NOTIFY | WS_CHILD | WS_VISIBLE, 40, 60, 90, 20, hWnd, 0, g_hinst, 0);
			//myfont = CreateFont(12, 0, 0, 0, FW_NORMAL, 0, 1, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FF_MODERN, "MS Sans Serif");
			//hwndHyper = CreateSysLink(hWnd);

			//서브클래싱
			OldEditProc_ip = (WNDPROC)SetWindowLongPtr(hEdit_ip, GWLP_WNDPROC, (LONG_PTR)EditSubProc1);

			return 0;
		}
		case WM_COMMAND:
		{	
			//if ((HWND)lParam == hwndStatic) 
			//	ShellExecute(parent_hWnd, L"open", L"http://52.192.132.151:8888/example0.html", 0, 0, 0);

			switch (LOWORD(wParam))
			{
				case 0:
				{
					if (socketObj.v_htmlBuf.size() > 0 && (g_index - 1) >= 0)
					{
						decrese = true;
						InvalidateRect(hWnd, NULL, TRUE);
					}
					break;
				}
				case 1:
				{
					if (socketObj.v_htmlBuf.size() > 0 && (g_index + 1) < socketObj.v_htmlBuf.size())
					{
						increse = true;
						InvalidateRect(hWnd, NULL, TRUE);
					}
					break;
				}
				//case 40003:
				//{
				//	// cgi 요청
				//	char newUrl[BUFSIZE];
				//	socketObj.ipAddrs.append(":"); socketObj.ipAddrs.append(socketObj.port); socketObj.ipAddrs.append("/");
				//	strcpy(newUrl, "http://");
				//	strcat(newUrl, socketObj.ipAddrs.c_str());
				//	strcat(newUrl, action.c_str());
				//	int i = 0;
				//	string param = "";
				//	for (i = 0; i < v_inputInfo.size(); i++)
				//	{
				//		param.append(converter.to_bytes(v_inputInfo[i].name) + "=");
				//		char value[BUFSIZE];
				//		GetWindowText(v_inputEdit[i], value, BUFSIZE);
				//		if (strcmp(value, "") == 0)
				//			param.append(converter.to_bytes(v_inputInfo[i].value));
				//		else
				//			param.append(value);
				//		if (i+1 < v_inputInfo.size())
				//			param.append("&");
				//	}

				//	int iResult = socketObj.ClientToServer(newUrl, hWnd, method, param, false);
				//	errorEventHandler(hWnd, iResult);
				//	disableWMPaint = false;
				//	
				//	int size = input_id - 40000;
				//	for (i = 0; i < size; i++)
				//	{
				//		//DestroyWindow(v_input[i]);
				//		ShowWindow(v_inputEdit[i], SW_HIDE);
				//	}
				//	InvalidateRect(hWnd, NULL, TRUE);
				//}
			}

			return 0;
			//MessageBox(hWnd, "Second Button Clicked", "Button", MB_OK);
		}
		case WM_HSCROLL:
		{
			xInc = 0;
			switch (LOWORD(wParam))
			{
				case SB_LINELEFT:	{ xInc = -20; break; }
				case SB_LINERIGHT:	{ xInc = 20; break;	 }
				case SB_PAGELEFT:	{ xInc = -25; break; }
				case SB_PAGERIGHT:	{ xInc = 25; break;  }
				case SB_THUMBTRACK: { xInc = HIWORD(wParam) - xPos; break; }
			}
			if (xPos + xInc < 0) xInc = -xPos;
			if (xPos + xInc > xMax) xInc = xMax - xPos;
			xPos = xPos + xInc;
			ScrollWindow(hWnd, -xInc, 0, NULL, NULL);
			SetScrollPos(hWnd, SB_HORZ, xPos, TRUE);
			return 0;
		}
		case WM_VSCROLL:
		{
			yInc = 0;
			switch (LOWORD(wParam))
			{
				case SB_LINEUP:		{ yInc = -10; break; }
				case SB_LINEDOWN:	{ yInc = 10; break;  }
				case SB_PAGEUP:		{ yInc = -20; break; }
				case SB_PAGEDOWN:	{ yInc = 20; break;  }
				case SB_THUMBTRACK: { yInc = HIWORD(wParam) - yPos; break; }
			}
			if (yPos + yInc < 0) yInc = -yPos;
			if (yPos + yInc > yMax) yInc = yMax - yPos;
			yPos = yPos + yInc;
			ScrollWindow(hWnd, 0, -yInc, NULL, NULL);
			SetScrollPos(hWnd, SB_VERT, yPos, TRUE);
			return 0;
		}
		case WM_PAINT:
		{
			hdc = BeginPaint(hWnd, &ps);
			if (!disableWMPaint)
			{
				if (socketObj.v_htmlBuf.size() > 0)
				{
					if (increse) g_index++;
					else if (decrese) g_index--;
					else if ((g_index + 1) < socketObj.v_htmlBuf.size()){
						g_index = socketObj.v_htmlBuf.size() - 1;
					}

					// Raw데이터와 roots사이즈가 다르다면 추가 하지 않는다. (=이미 정보를 가지고있음)
					// socketObj.v_htmlBuf = Raw 데이터
					// roots = 이미 트리로 만들어진 데이터. 각각의 루트들을 가지고있다
					if (roots.size() < socketObj.v_htmlBuf.size())
					{
						Node root = domTree.createTree(socketObj.v_htmlBuf[g_index]);
						roots.push_back(root);
					}
					
					x = XMOVER - xPos;
					y = YMOVER - yPos;

					// recursive를 이용하여 실행
					traveralExecute(hWnd, hdc, roots.at(g_index), socketObj);
					increse = false; decrese = false;
				}
			}
			EndPaint(hWnd, &ps);
			return 0;
		}
		case WM_DESTROY:
		{
			SetWindowLongPtr(hEdit_ip, GWLP_WNDPROC, (LONG_PTR)EditSubProc1);
			PostQuitMessage(0);
			return 0;
		}
	}
	
	return (DefWindowProc(hWnd, iMessage, wParam, lParam));
}

LRESULT CALLBACK EditSubProc1(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	wchar_t url[BUFSIZE];
	int n;
	memset(url, '\0', BUFSIZ);
	switch (iMessage)
	{
		case WM_KEYDOWN:
		{
			if (wParam == VK_RETURN)
			{
				GetWindowText(hEdit_ip, url, size(url));
				// 서버랑 연결해서 모든 정보를 받아온다
				n = socketObj.ClientToServer(url, hWnd, "GET", "", false);
				errorEventHandler(hWnd, n);

				//WM_PAINT를 호출한다.
				RECT rect;
				GetClientRect(parent_hWnd, &rect);
				InvalidateRect(parent_hWnd, &rect, TRUE);
			}
			break;
		}
	}
	return CallWindowProc(OldEditProc_ip, hWnd, iMessage, wParam, lParam);
}

void traveralExecute(HWND hWnd, HDC hdc, Node root, Socket socket)
{
	if (root.children.size() == 0)
	{
		wcout << root.tag << "=" << root.content << endl;
		executeCmd(hWnd, hdc, socket, root.tag, root.content, root.attributes);
		return;
	}

	for (int i = 0; i < root.children.size(); i++)
	{
		traveralExecute(hWnd, hdc, root.children.at(i), socket);		
	}
}

int executeCmd(HWND hWnd, HDC hdc, Socket socket, wstring tag, wstring content, Attribute attrs)
{
	enum htmlTag { a = 0, h1, h2, h3, h4, h5, h6, hr, br = 10, div = 11, address, span,  
					p = 20, b, pre, ul = 30, ol, input = 40, center = 100, table = 200, img = 1000, form };

	int cmd;
	HFONT font, oldfont;
	Parser parser;
	if (tag.empty())
		return 0;
	
	cmd = getCmdFromHtmlTag(tag);

	switch (cmd)
	{
		case a:
		{
			return 0;
		}
		case h1:
		{
			font = CreateFont(40, 0, 0, 0, 20, FALSE, 0, 0, 0, 0, 0, 0, 0, L"Times New Roman");
			oldfont = (HFONT)SelectObject(hdc, font);
			//x = XMOVER;
			//y = yCur;
			TextOutW(hdc, x, y, content.c_str(), content.length());	
			//yCur = y + 40;
			y += YMOVER;
			SelectObject(hdc, oldfont);
			DeleteObject(font);
			return 0;
		}
		case h2:
		{
			font = CreateFont(35, 0, 0, 0, 20, FALSE, 0, 0, 0, 0, 0, 0, 0, L"Times New Roman");
			oldfont = (HFONT)SelectObject(hdc, font);
			//x = XMOVER;
			//y = yCur;
			TextOutW(hdc, x, y, content.c_str(), content.length());
			//yCur = y + 35;
			y += YMOVER;
			SelectObject(hdc, oldfont);
			DeleteObject(font);
			return 0;
		}
		case h3:
		{
			font = CreateFont(30, 0, 0, 0, 20, FALSE, 0, 0, 0, 0, 0, 0, 0, L"Times New Roman");
			oldfont = (HFONT)SelectObject(hdc, font);
			//x = XMOVER;
			//y = yCur;
			TextOutW(hdc, x, y, content.c_str(), content.length());
			//yCur = y + 30;
			y += YMOVER;
			SelectObject(hdc, oldfont);
			DeleteObject(font);
			return 0;
		}
		case h4:
		{
			font = CreateFont(25, 0, 0, 0, 20, FALSE, 0, 0, 0, 0, 0, 0, 0, L"Times New Roman");
			oldfont = (HFONT)SelectObject(hdc, font);
			//x = XMOVER;
			//y = yCur;
			TextOutW(hdc, x, y, content.c_str(), content.length());
			y += YMOVER;
			//yCur = y + 25;
			SelectObject(hdc, oldfont);
			DeleteObject(font);
			return 0;
		}
		case h5:
		{
			font = CreateFont(20, 0, 0, 0, 20, FALSE, 0, 0, 0, 0, 0, 0, 0, L"Times New Roman");
			oldfont = (HFONT)SelectObject(hdc, font);
			//x = XMOVER;
			//y = yCur;
			TextOutW(hdc, x, y, content.c_str(), content.length());
			//yCur = y + 20;
			y += YMOVER;
			SelectObject(hdc, oldfont);
			DeleteObject(font);
			return 0;
		}
		case h6:
		{
			font = CreateFont(15, 0, 0, 0, 20, FALSE, 0, 0, 0, 0, 0, 0, 0, L"Times New Roman");
			oldfont = (HFONT)SelectObject(hdc, font);
			//x = XMOVER;
			//y = yCur;
			TextOutW(hdc, x, y, content.c_str(), content.length());
			//yCur = y + 15;
			y += YMOVER;
			SelectObject(hdc, oldfont);
			DeleteObject(font);
			return 0;
		}
		case hr:
		{
			Graphics graphics(hdc);
			Pen pen(Color(0, 0, 0));
			//y = yCur;
			graphics.DrawLine(&pen, 0, y, 8000, y);
			y += YMOVER;
		}
		case div:
		{
			return 0;
		}
		case address:
		{
			font = CreateFont(25, 0, 0, 0, 20, TRUE, 0, 0, 0, 0, 0, 0, 0, L"Times New Roman");
			oldfont = (HFONT)SelectObject(hdc, font);
			//x = XMOVER;
			//y = yCur;
			TextOutW(hdc, x, y, content.c_str(), content.length());
			//yCur = y + 25;
			y += YMOVER;
			SelectObject(hdc, oldfont);
			DeleteObject(font);
			return 0;
		}
		case span:
		{
			font = CreateFont(25, 0, 0, 0, 20, FALSE, 0, 0, 0, 0, 0, 0, 0, L"Times New Roman");
			oldfont = (HFONT)SelectObject(hdc, font);

			wstring style = attrs.font.style;
			int foundS = style.find(L"color:");
			if (foundS >= 0)
			{
				style = style.erase(foundS, strlen("color:"));
				if (style == L"blue") SetTextColor(hdc, RGB(0, 0, 255));
				else if (style == L"red") SetTextColor(hdc, RGB(255, 0, 0));
				else if (style == L"yellow") SetTextColor(hdc, RGB(255, 255, 0));
				else if (style == L"green") SetTextColor(hdc, RGB(0, 255, 0));
			}
			//x = xCur;
			//y = yCur;
			TextOutW(hdc, x, y, content.c_str(), content.length());
			//xCur = x + content.length();
			y += YMOVER;
			SetTextColor(hdc, RGB(0, 0, 0));
			SelectObject(hdc, oldfont);
			DeleteObject(font);
			return 0;
		}
		case b:
		{
			font = CreateFont(25, 0, 0, 0, FW_BOLD, FALSE, 0, 0, 0, 0, 0, 0, 0, L"Times New Roman");
			oldfont = (HFONT)SelectObject(hdc, font);
			//x = xCur;
			//y = yCur;
			TextOutW(hdc, x, y, content.c_str(), content.length());
			//xCur = x + content.length();
			//yCur = y + 25; // 일단은 newline 으로 넘기자
			y += YMOVER;
			SelectObject(hdc, oldfont);
			DeleteObject(font);
			return 0;
		}
		case p:
		{
			font = CreateFont(20, 0, 0, 0, 20, FALSE, 0, 0, 0, 0, 0, 0, 0, L"Times New Roman");
			oldfont = (HFONT)SelectObject(hdc, font);
			//x = XMOVER;
			//y = yCur;
			TextOutW(hdc, x, y, content.c_str(), content.length());
			//yCur = y + 20;
			y += YMOVER;
			SelectObject(hdc, oldfont);
			DeleteObject(font);
			return 0;
		}
		case br:
		{
			y += YMOVER;
			return 0;
		}
		case pre:
		{
			//\t랑 \n 적용되어야한다
			x = XMOVER;
			
			int i;
			vector<string> contents = split(converter.to_bytes(content), '\n');
			font = CreateFont(20, 0, 0, 0, 20, FALSE, 0, 0, 0, 0, 0, 0, 0, L"Times New Roman");
			for (i = 0; i < contents.size(); i++)
			{
				//y = yCur;
				if (contents[i].empty())
					continue;
				TextOut(hdc, x, y, (LPWSTR)contents[i].c_str(), strlen(contents[i].c_str()));
				//yCur = y + 20;
				y += YMOVER;
				oldfont = (HFONT)SelectObject(hdc, font);
				SelectObject(hdc, oldfont);
				DeleteObject(font);
			}		
			return 0;
		}
		case ul:
		{
			int foundS = 0;
			int foundE = 0;
			x += XMOVER;
			while (1)
			{
				wstring subcontent = L"";
				if ((foundS = content.find(L"<li>")) < 0) break;
				if ((foundE = content.find(L"</li>")) < 0) break;
				subcontent = content.substr(foundS + 4, foundE - (foundS + 4));
				wstring combined = L"@. ";
				combined.append(subcontent);
				TextOutW(hdc, x, y, combined.c_str(), combined.length());
				content.erase(0, foundE + 5);
				y += YMOVER;
			}
			x -= XMOVER;
			return 0;
		}
		case ol:
		{
			int foundS = 0;
			int foundE = 0;
			int i = 1;
			x += XMOVER;
			while (1)
			{
				wstring subcontent = L"";
				if ((foundS = content.find(L"<li>")) < 0) break;
				if ((foundE = content.find(L"</li>")) < 0) break;
				subcontent = content.substr(foundS + 4, foundE - (foundS + 4));
				wstring combined = std::to_wstring(i);
				combined.append(L". ");
				combined.append(subcontent);
				TextOutW(hdc, x, y, combined.c_str(), combined.length());
				content.erase(0, foundE + 5);
				y += YMOVER;
				i++;
			}
			x -= XMOVER;
			return 0;
		}
		case input:
		{
			//			else if (htmlTag == L"input")
			//			{
			//				foundS = socket.v_htmlBuf[page].find(L"<");
			//				foundE = socket.v_htmlBuf[page].find(L">");
			//
			//				// < 와 > 를 포함한 substring
			//				htmlContent = socket.v_htmlBuf[page].substr(foundS, foundE - foundS + 1);
			//				inputInfo.value = parser.getInputValue(htmlContent);
			//				inputInfo.type = parser.getInputType(htmlContent);
			//				inputInfo.name = parser.getInputName(htmlContent);
			//				if (inputInfo.type == L"text")
			//					v_inputInfo.push_back(inputInfo);
			//				socket.v_htmlBuf[page].erase(foundS, foundE - foundS + 1);
			//				htmlContent = L"";
			//			}

			//x += XMOVER * 2;
			//if (oneInputInfo.value == L"submit")
			//{
			//	v_inputEdit.push_back(CreateWindow("BUTTON", "Submit", WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER | ES_LEFT, x, y, 100, 30, hWnd, (HMENU)(input_id), g_hinst, NULL));
			//	submitId = input_id;
			//}
			//else
			//	v_inputEdit.push_back(CreateWindow("EDIT", converter.to_bytes(oneInputInfo.value).c_str(), WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER | ES_LEFT, x, y, 200, 30, hWnd, (HMENU)(input_id), g_hinst, NULL));
			//disableWMPaint = true;
			//input_id++;
			//x -= XMOVER * 2;
			return 0;
		}
		case center:
		{
			HANDLE screen = GetStdHandle(STD_OUTPUT_HANDLE);
			COORD max_size = GetLargestConsoleWindowSize(screen);
			COORD pos;
			pos.X = (max_size.X - sizeof(content)) / 2;		
			TextOutW(hdc, pos.X, y, content.c_str(), content.length());
			y += YMOVER;
			return 0;
		}
		case table:
		{
			//int S = 0;
			//int E = 0;
			//int foundS = 0;
			//int foundE = 0;
			//int offset = 10;
			//Graphics graphics(hdc);
			//Pen pen(Color(0, 0, 0));
			//int sub_x = x;
			//int sub_y = y+30;

			//while (1)
			//{
			//	if ((foundS = content.find(L"<tr>")) < 0) break;
			//	if ((foundE = content.find(L"</tr>")) < 0) break;
			//	S = foundS; E = foundE;
			//	wstring subcontent = content.substr(foundS + 4, foundE - (foundS + 4));
			//	
			//	while (1)
			//	{
			//		wstring subsubcontent = L"";
			//		if ((foundS = subcontent.find(L"<th>")) >= 0)
			//		{
			//			foundE = subcontent.find(L"</th>");
			//			subsubcontent = subcontent.substr(foundS + 4, foundE - (foundS + 4));					
			//		}
			//		else if ((foundS = subcontent.find(L"<td>")) >= 0)
			//		{
			//			foundE = subcontent.find(L"</td>");
			//			subsubcontent = subcontent.substr(foundS + 4, foundE - (foundS + 4));
			//		}
			//		else
			//		{
			//			break;
			//		}	

			//		//offset = strlen(subsubcontent.c_str()) * 10;
			//		offset = 80;
			//		//1글자 크기당  x값을 10으로 주자
			//		Rectangle(hdc, sub_x - 5, sub_y - 20, sub_x + offset, sub_y + 20);
			//		
			//		TextOutW(hdc, sub_x, sub_y, subsubcontent.c_str(), subsubcontent.length());
			//		subcontent.erase(foundS, foundE - foundS + 5);

			//		sub_x += offset;
			//	}

			//	//세로로 이동
			//	content.erase(S, E - S + 5);
			//	sub_y = sub_y + 40;
			//	sub_x = x;
			//}
			return 0;
		}
		case img:
		{
			//사진을 서버에게 요청한다.
			if (attrs.img.height == L"")
				attrs.img.height = L"300";
			if (attrs.img.width == L"")
				attrs.img.width = L"300";

			//이미지 요청
			wchar_t newUrl[BUFSIZE];
			int found = 0;
			if ((found = attrs.img.src.find(L"http://")) < 0)
			{
				//파일
				socket.ipAddrs.append(":"); socket.ipAddrs.append(socket.port); socket.ipAddrs.append("/");
				wcscpy(newUrl, L"http://");
				wstring wst(socket.ipAddrs.begin(), socket.ipAddrs.end());
				wcscat(newUrl, wst.c_str());
				wcscat(newUrl, attrs.img.src.c_str());
			}
			else
			{
				//URL로 파일 전송하는 경우
				wcscpy(newUrl, attrs.img.src.c_str());
			}

			int iResult = socket.ClientToServer(newUrl, hWnd, "GET", "", true);
			errorEventHandler(hWnd, iResult);
			if (iResult == -1)
				return 0;

			//받은 이미지 보여주기			
			Imager imager;
			string imgPath = imager.GetExeFileName(converter.to_bytes(attrs.img.src));
			wstring wimgPath = wstring(imgPath.begin(), imgPath.end());
			int width = atoi(converter.to_bytes(attrs.img.width).c_str());
			int height = atoi(converter.to_bytes(attrs.img.height).c_str());
			imager.setImage(hWnd, hdc, wimgPath.c_str(), x, y, width, height);
			xCur = x + width;
			yCur = y + height;
			x += width;
			return 0;
		}
		case form:
		{
			//			if (htmlTag == L"form")
			//			{
			//				action = "";  method = "";
			//				foundS = socket.v_htmlBuf[page].find(L"<");
			//				foundE = socket.v_htmlBuf[page].find(L">");
			//
			//				// < 와 > 를 포함한 substring
			//				htmlContent = socket.v_htmlBuf[page].substr(foundS, foundE - foundS + 1);
			//				action = parser.getFormAction(htmlContent);
			//				method = parser.getFormMethod(htmlContent);
			//
			//				socket.v_htmlBuf[page].erase(foundS, foundE - foundS + 1);
			//				foundE = socket.v_htmlBuf[page].find(L"</form>");
			//				socket.v_htmlBuf[page].erase(foundE, 7);
			//				htmlTag = L""; htmlContent = L"";
			//
			//			}
			return 0;
		}
		default:
		{
			if (content.empty() || tag == L"title" || tag == L"meta")
				break;
			TextOutW(hdc, x, y, content.c_str(), content.length());
			y += YMOVER;
			return 0;
		}
	}
}

int getCmdFromHtmlTag(wstring tag)
{
	if (tag == L"a") return 0;
	else if (tag == L"h1") return 1;
	else if (tag == L"h2") return 2;
	else if (tag == L"h3") return 3;
	else if (tag == L"h4") return 4;
	else if (tag == L"h5") return 5;
	else if (tag == L"h6") return 6;
	else if (tag == L"hr") return 7;
	else if (tag == L"br") return 10;
	else if (tag == L"div") return 11;
	else if (tag == L"address") return 12;
	else if (tag == L"span") return 13;
	else if (tag == L"p") return 20;
	else if (tag == L"b") return 21;
	else if (tag == L"pre") return 22;
	else if (tag == L"ul") return 30;
	else if (tag == L"ol") return 31;
	else if (tag == L"input") return 40;
	else if (tag == L"center") return 100;
	else if (tag == L"table") return 200;
	else if (tag == L"img") return 1000;
	else if (tag == L"form") return 1001;
	else return -1;
}

int errorEventHandler(HWND hWnd, int n)
{
	if (n == 4001)
		MessageBox(hWnd, L"Please, input a valid protocol (only http://)", L"Button", MB_OK);
	else if (n == -1)
		MessageBox(hWnd, L"A issue has occured in the program", L"Button", MB_OK);
	return 0;
}

vector<string> split(string str, char delimiter) {
	vector<string> internal;
	stringstream ss(str); // Turn the string into a stream.
	string tok;

	while (getline(ss, tok, delimiter)) {
		internal.push_back(tok);
	}
	return internal;
}

HWND CreateSysLink(HWND hwnd) {
	HWND SysLink;
	SysLink = CreateWindowEx(0, L"SysLink", L"Produced by <A HREF=\"http://google.com\">click test</A>", WS_CHILD | WS_VISIBLE | WS_TABSTOP,
		50, 50, 100, 100, hwnd, NULL, GetModuleHandle(NULL), NULL);
	if (SysLink == NULL) {
		MessageBox(NULL, L"Error in creation of syslink.", L"Error!", MB_OK | MB_ICONEXCLAMATION);
		return 0;
	}
	else
		return SysLink;
}
