#define WIN32_LEAN_AND_MEAN

#include "socket.h"
#include "parser.h"
#include "validate.h"
#include "imager.h"
#include <sstream>

using namespace std;

#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")

#define YMOVER 40
#define XMOVER 40

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK EditSubProc1(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);
int setResponse(HWND hWnd, HDC hdc, Socket socket, int i);
int getCmdFromHtmlTag(string tag);
int executeCmd(HWND hWnd, HDC hdc, Socket socket, string tag, string content, ImgInfo imgInfo, InputInfo inputInfo);
int errorEventHandler(HWND hWnd, int n);
vector<string> split(string str, char delimiter);

HINSTANCE g_hinst;
LPCTSTR lpszClass = TEXT("Web Browser");
HWND hEdit_ip;
WNDPROC OldEditProc_ip;
HWND parent_hWnd;

vector<HWND> v_inputEdit;
vector<InputInfo> v_inputInfo;
Socket socketObj;

int x;
int y;
int g_index;
int input_id;
int submitId;
bool decrese;
bool increse;
bool attr_BOLD;
bool disableWMPaint;

string action;
string method;

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPreInstance, LPSTR lpszCmdParam, int nCmdShow)
{
	/*  GDI+ startup token */
	ULONG_PTR gdiplusStartupToken;
	Gdiplus::GdiplusStartupInput gdiInput;
	Gdiplus::GdiplusStartup(&gdiplusStartupToken, &gdiInput, NULL);
	g_index = -1;
	input_id = 40000;
	decrese = false;
	increse = false;
	disableWMPaint = false;
	submitId = -1;

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

	parent_hWnd = CreateWindow(lpszClass, lpszClass, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, (HMENU)NULL, hInstance, NULL);
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

	switch (iMessage)
	{
		case WM_CREATE:
		{
			//http://52.192.132.151:8888/example0.html
			hEdit_ip = CreateWindowEx(
				WS_EX_WINDOWEDGE, "EDIT", "http://52.192.132.151:8888/example0.html", WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER | ES_LEFT, 80, 10, 600, 30, hWnd, (HMENU)(101), g_hinst, NULL);
			CreateWindow("button", "<-", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 10, 10, 30, 30, hWnd, (HMENU)0, g_hinst, NULL);
			CreateWindow("button", "->", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 40, 10, 30, 30, hWnd, (HMENU)1, g_hinst, NULL);
			
			//서브클래싱
			OldEditProc_ip = (WNDPROC)SetWindowLongPtr(hEdit_ip, GWLP_WNDPROC, (LONG_PTR)EditSubProc1);
			return 0;
		}
		case WM_COMMAND:
		{	
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
				case 40003:
				{
					// cgi 요청
					char newUrl[BUFSIZE];
					socketObj.ipAddrs.append(":"); socketObj.ipAddrs.append(socketObj.port); socketObj.ipAddrs.append("/");
					strcpy(newUrl, "http://");
					strcat(newUrl, socketObj.ipAddrs.c_str());
					strcat(newUrl, action.c_str());
					int i = 0;
					string param = "";
					for (i = 0; i < v_inputInfo.size(); i++)
					{
						param.append(v_inputInfo[i].name + "=");
						char value[BUFSIZE];
						GetWindowText(v_inputEdit[i], value, BUFSIZE);
						if (strcmp(value, "") == 0)
							param.append(v_inputInfo[i].value);
						else
							param.append(value);
						if (i+1 < v_inputInfo.size())
							param.append("&");
					}

					int iResult = socketObj.ClientToServer(newUrl, hWnd, method, param, false);
					errorEventHandler(hWnd, iResult);
					disableWMPaint = false;
					
					int size = input_id - 40000;
					for (i = 0; i < size; i++)
					{
						//DestroyWindow(v_input[i]);
						ShowWindow(v_inputEdit[i], SW_HIDE);
					}
					InvalidateRect(hWnd, NULL, TRUE);
				}
			}

			return 0;
			//MessageBox(hWnd, "Second Button Clicked", "Button", MB_OK);
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
					else if ((g_index + 1) < socketObj.v_htmlBuf.size())
					{
						//g_index++;
						g_index = socketObj.v_htmlBuf.size()-1;
					}
					setResponse(hWnd, hdc, socketObj, g_index);
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
	char url[BUFSIZE];
	int n;

	switch (iMessage)
	{
		case WM_KEYDOWN:
		{
			if (wParam == VK_RETURN)
			{
				GetWindowText(hEdit_ip, url, strlen(url));
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

int setResponse(HWND hWnd, HDC hdc, Socket socket, int i)
{
	Parser parser;
	ImgInfo imgInfo;
	Imager imager;
	PAINTSTRUCT ps;
	InputInfo inputInfo;

	string htmlTag;
	string htmlContent;
	string split;

	int iResult = 0;
	int offset = 0;
	int foundS = 0;
	int foundE = 0;
	bool opening = false;
	bool closing = false;
	x = 20; y = 40;
	
	socket.v_htmlBuf[i].erase(remove(socket.v_htmlBuf[i].begin(), socket.v_htmlBuf[i].end(), '\r'), socket.v_htmlBuf[i].end());

	while (socket.v_htmlBuf[i].length() > 0)
	{
		htmlTag = ""; htmlContent = ""; attr_BOLD = false;
		if ((foundS = socket.v_htmlBuf[i].find("<")) >= 0)
		{
			opening = true;
		}
		if (opening && (foundE = socket.v_htmlBuf[i].find(">")) >= 0)
		{
			closing = true;
		}

		if (foundS != 0)
		{
			htmlTag = "txt";
			htmlContent = socket.v_htmlBuf[i].substr(0, foundS);
			socket.v_htmlBuf[i].erase(0, foundS);
		}
		else if (opening && closing)
		{
			htmlTag = parser.getHTMLTag(socket.v_htmlBuf[i]);

			if (htmlTag == "form")
			{
				action = "";  method = "";
				foundS = socket.v_htmlBuf[i].find("<");
				foundE = socket.v_htmlBuf[i].find(">");

				// < 와 > 를 포함한 substring
				htmlContent = socket.v_htmlBuf[i].substr(foundS, foundE - foundS + 1);
				action = parser.getFormAction(htmlContent);
				method = parser.getFormMethod(htmlContent);

				socket.v_htmlBuf[i].erase(foundS, foundE - foundS + 1);
				foundE = socket.v_htmlBuf[i].find("</form>");
				socket.v_htmlBuf[i].erase(foundE, 7);
				htmlTag = ""; htmlContent = "";

			}
			else if (htmlTag == "input")
			{
				foundS = socket.v_htmlBuf[i].find("<");
				foundE = socket.v_htmlBuf[i].find(">");

				// < 와 > 를 포함한 substring
				htmlContent = socket.v_htmlBuf[i].substr(foundS, foundE - foundS + 1);
				inputInfo.value = parser.getInputValue(htmlContent);
				inputInfo.type = parser.getInputType(htmlContent);
				inputInfo.name = parser.getInputName(htmlContent);
				if (inputInfo.type == "text")
					v_inputInfo.push_back(inputInfo);
				socket.v_htmlBuf[i].erase(foundS, foundE - foundS + 1);
				htmlContent = "";		
			}
			else if (htmlTag == "img")
			{
				foundS = socket.v_htmlBuf[i].find("<");
				foundE = socket.v_htmlBuf[i].find(">");
				string temp = socket.v_htmlBuf[i].substr(foundS, foundE - foundS + 1);
				imgInfo = parser.getImgInfo(temp);
				if (imgInfo.height == "")
					imgInfo.height = "300";
				if (imgInfo.width == "")
					imgInfo.width = "300";
				socket.v_htmlBuf[i].erase(0, foundE + 1);

				// 이미지 요청
				char newUrl[BUFSIZE];
				socket.ipAddrs.append(":"); socket.ipAddrs.append(socket.port); socket.ipAddrs.append("/");
				strcpy(newUrl, "http://");
				strcat(newUrl, socket.ipAddrs.c_str());
				strcat(newUrl, imgInfo.src.c_str());
				iResult = socket.ClientToServer(newUrl, hWnd, "GET", "", true);
				errorEventHandler(hWnd, iResult);
				htmlTag = "img";
			}
			else if (htmlTag == "br/" || htmlTag == "br" || htmlTag == "BR/" || htmlTag == "BR")
			{
				htmlTag = "br"; htmlContent = "";
				socket.v_htmlBuf[i].erase(foundS, foundE - foundS + 1);
			}
			else if (htmlTag == "hr")
			{
				htmlTag = "hr"; htmlContent = "";
				foundE = socket.v_htmlBuf[i].find("<hr>");
				socket.v_htmlBuf[i].erase(foundE, 4);
			}
			else
			{
				string temp = "</"; temp.append(htmlTag); temp.append(">");
				foundS = foundE;
				if ((foundE = socket.v_htmlBuf[i].find(temp)) >= 0)
				{
					htmlContent = socket.v_htmlBuf[i].substr(foundS + 1, foundE - (foundS + 1));
					//만약 태그 안에 또 태그가 있다면. 한계(두번째까지)

					string innerTag = parser.getHTMLTag(htmlContent);			
					if (innerTag == "b")
					{
						attr_BOLD = true;
						htmlContent = parser.removeHTMLTag(htmlContent);
					}
				}
				offset = htmlTag.length() + 3;
				socket.v_htmlBuf[i].erase(0, foundE + offset);
			}
			opening = false;
			closing = false;
		}	
		
		if (htmlTag != "pre")
		{
			htmlContent.erase(remove(htmlContent.begin(), htmlContent.end(), '\t'), htmlContent.end());
			htmlContent.erase(remove(htmlContent.begin(), htmlContent.end(), '\n'), htmlContent.end());
		}
		iResult = executeCmd(hWnd, hdc, socket, htmlTag, htmlContent, imgInfo, inputInfo);
	}

	return iResult;
}

int executeCmd(HWND hWnd, HDC hdc, Socket socket, string tag, string content, ImgInfo imgInfo, InputInfo oneInputInfo)
{
	enum htmlTag { a = 0, h1, h2, h3, h4, h5, h6, hr, br = 10, div = 11, address, span,  p = 20, b, pre, ul = 30, ol, input = 40, center = 100, table = 200, img = 1000 };
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
			font = CreateFont(40, 0, 0, 0, 20, FALSE, 0, 0, 0, 0, 0, 0, 0, "Times New Roman");
			oldfont = (HFONT)SelectObject(hdc, font);		
			TextOut(hdc, x, y, content.c_str(), strlen(content.c_str()));
			y += YMOVER;
			SelectObject(hdc, oldfont);
			DeleteObject(font);
			return 0;
		}
		case h2:
		{
			font = CreateFont(35, 0, 0, 0, 20, FALSE, 0, 0, 0, 0, 0, 0, 0, "Times New Roman");
			oldfont = (HFONT)SelectObject(hdc, font);	
			TextOut(hdc, x, y, content.c_str(), strlen(content.c_str()));
			y += YMOVER;
			SelectObject(hdc, oldfont);
			DeleteObject(font);
			return 0;
		}
		case h3:
		{
			font = CreateFont(30, 0, 0, 0, 20, FALSE, 0, 0, 0, 0, 0, 0, 0, "Times New Roman");
			oldfont = (HFONT)SelectObject(hdc, font);			
			TextOut(hdc, x, y, content.c_str(), strlen(content.c_str()));
			y += YMOVER;
			SelectObject(hdc, oldfont);
			DeleteObject(font);
			return 0;
		}
		case h4:
		{
			font = CreateFont(25, 0, 0, 0, 20, FALSE, 0, 0, 0, 0, 0, 0, 0, "Times New Roman");
			oldfont = (HFONT)SelectObject(hdc, font);	
			TextOut(hdc, x, y, content.c_str(), strlen(content.c_str()));
			y += YMOVER;
			SelectObject(hdc, oldfont);
			DeleteObject(font);
			return 0;
		}
		case h5:
		{
			font = CreateFont(20, 0, 0, 0, 20, FALSE, 0, 0, 0, 0, 0, 0, 0, "Times New Roman");
			oldfont = (HFONT)SelectObject(hdc, font);
			TextOut(hdc, x, y, content.c_str(), strlen(content.c_str()));
			y += YMOVER;
			SelectObject(hdc, oldfont);
			DeleteObject(font);
			return 0;
		}
		case h6:
		{
			font = CreateFont(15, 0, 0, 0, 20, FALSE, 0, 0, 0, 0, 0, 0, 0, "Times New Roman");
			oldfont = (HFONT)SelectObject(hdc, font);
			TextOut(hdc, x, y, content.c_str(), strlen(content.c_str()));
			y += YMOVER;
			SelectObject(hdc, oldfont);
			DeleteObject(font);
			return 0;
		}
		case hr:
		{
			Graphics graphics(hdc);
			Pen pen(Color(0, 0, 0));
			graphics.DrawLine(&pen, 0, y, 8000, y);
			//y += YMOVER;
		}
		case div:
		{
			return 0;
		}
		case address:
		{
			return 0;
		}
		case span:
		{
			return 0;
		}
		case b:
		{
			font = CreateFont(25, 0, 0, 0, FW_BOLD, FALSE, 0, 0, 0, 0, 0, 0, 0, "Times New Roman");
			oldfont = (HFONT)SelectObject(hdc, font);
			TextOut(hdc, x, y, content.c_str(), strlen(content.c_str()));
			y += YMOVER;
			SelectObject(hdc, oldfont);
			DeleteObject(font);
			return 0;
		}
		case p:
		{
			if (attr_BOLD)
				font = CreateFont(20, 0, 0, 0, FW_BOLD, FALSE, 0, 0, 0, 0, 0, 0, 0, "Times New Roman");
			else
				font = CreateFont(20, 0, 0, 0, 20, FALSE, 0, 0, 0, 0, 0, 0, 0, "Times New Roman");
			oldfont = (HFONT)SelectObject(hdc, font);
			TextOut(hdc, x, y, content.c_str(), strlen(content.c_str()));
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
			int i;
			vector<string> contents = split(content, '\n');
			font = CreateFont(20, 0, 0, 0, 20, FALSE, 0, 0, 0, 0, 0, 0, 0, "Times New Roman");
			for (i = 0; i < contents.size(); i++)
			{
				y += YMOVER;
				if (contents[i].empty())
					continue;
				TextOut(hdc, x, y, contents[i].c_str(), strlen(contents[i].c_str()));
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
				string subcontent = "";
				if ((foundS = content.find("<li>")) < 0) break;
				if ((foundE = content.find("</li>")) < 0) break;
				subcontent = content.substr(foundS + 4, foundE - (foundS + 4));
				string combined = "@. ";
				combined.append(subcontent);
				TextOut(hdc, x, y, combined.c_str(), strlen(combined.c_str()));
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
				string subcontent = "";
				if ((foundS = content.find("<li>")) < 0) break;
				if ((foundE = content.find("</li>")) < 0) break;
				subcontent = content.substr(foundS + 4, foundE - (foundS + 4));
				string combined = std::to_string(i);
				combined.append(". ");
				combined.append(subcontent);
				TextOut(hdc, x, y, combined.c_str(), strlen(combined.c_str()));
				content.erase(0, foundE + 5);
				y += YMOVER;
				i++;
			}
			x -= XMOVER;
			return 0;
		}
		case input:
		{
			x += XMOVER * 2;
			if (oneInputInfo.value == "submit")
			{
				v_inputEdit.push_back(CreateWindow("BUTTON", "Submit", WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER | ES_LEFT, x, y, 100, 30, hWnd, (HMENU)(input_id), g_hinst, NULL));
				submitId = input_id;
			}
			else
				v_inputEdit.push_back(CreateWindow("EDIT", oneInputInfo.value.c_str(), WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER | ES_LEFT, x, y, 200, 30, hWnd, (HMENU)(input_id), g_hinst, NULL));
			disableWMPaint = true;
			input_id++;
			x -= XMOVER * 2;
		}
		case center:
		{
			HANDLE screen = GetStdHandle(STD_OUTPUT_HANDLE);
			COORD max_size = GetLargestConsoleWindowSize(screen);
			COORD pos;
			pos.X = (max_size.X - sizeof(content)) / 2;		
			TextOut(hdc, pos.X, y, content.c_str(), strlen(content.c_str()));
			y += YMOVER;
			return 0;
		}
		case table:
		{
			int S = 0;
			int E = 0;
			int foundS = 0;
			int foundE = 0;
			int offset = 10;
			Graphics graphics(hdc);
			Pen pen(Color(0, 0, 0));
			int sub_x = x;
			int sub_y = y+30;

			while (1)
			{
				if ((foundS = content.find("<tr>")) < 0) break;
				if ((foundE = content.find("</tr>")) < 0) break;
				S = foundS; E = foundE;
				string subcontent = content.substr(foundS + 4, foundE - (foundS + 4));
				
				while (1)
				{
					string subsubcontent = "";
					if ((foundS = subcontent.find("<th>")) >= 0)
					{
						foundE = subcontent.find("</th>");
						subsubcontent = subcontent.substr(foundS + 4, foundE - (foundS + 4));					
					}
					else if ((foundS = subcontent.find("<td>")) >= 0)
					{
						foundE = subcontent.find("</td>");
						subsubcontent = subcontent.substr(foundS + 4, foundE - (foundS + 4));
					}
					else
					{
						break;
					}	

					//offset = strlen(subsubcontent.c_str()) * 10;
					offset = 80;
					//1글자 크기당  x값을 10으로 주자
					Rectangle(hdc, sub_x - 5, sub_y - 20, sub_x + offset, sub_y + 20);
					
					TextOut(hdc, sub_x, sub_y, subsubcontent.c_str(), strlen(subsubcontent.c_str()));
					subcontent.erase(foundS, foundE - foundS + 5);

					sub_x += offset;
				}

				//세로로 이동
				content.erase(S, E - S + 5);
				sub_y = sub_y + 40;
				sub_x = x;
			}
			return 0;
		}
		case img:
		{
			Imager imager;
			string imgPath = imager.GetExeFileName(imgInfo.src);
			wstring wimgPath = wstring(imgPath.begin(), imgPath.end());
			int width = atoi(imgInfo.width.c_str());
			int height = atoi(imgInfo.height.c_str());
			//y += YMOVER;
			x += width;
			imager.setImage(hWnd, hdc, wimgPath.c_str(), x, y, width, height);
			return 0;
		}
		default:
		{
			if (content.empty())
				break;
			TextOut(hdc, x, y, content.c_str(), strlen(content.c_str()));
			//y += YMOVER;
			return 0;
		}
	}
}

int getCmdFromHtmlTag(string tag)
{
	if (strcmp(tag.c_str(), "a") == 0) return 0;
	else if (strcmp(tag.c_str(), "h1") == 0) return 1;
	else if (strcmp(tag.c_str(), "h2") == 0) return 2;
	else if (strcmp(tag.c_str(), "h3") == 0) return 3;
	else if (strcmp(tag.c_str(), "h4") == 0) return 4;
	else if (strcmp(tag.c_str(), "h5") == 0) return 5;
	else if (strcmp(tag.c_str(), "h6") == 0) return 6;
	else if (strcmp(tag.c_str(), "hr") == 0) return 7;
	else if (strcmp(tag.c_str(), "br") == 0) return 10;
	else if (strcmp(tag.c_str(), "div") == 0) return 11;
	else if (strcmp(tag.c_str(), "address") == 0) return 12;
	else if (strcmp(tag.c_str(), "span") == 0) return 13;
	else if (strcmp(tag.c_str(), "p") == 0) return 20;
	else if (strcmp(tag.c_str(), "b") == 0) return 21;
	else if (strcmp(tag.c_str(), "pre") == 0) return 22;
	else if (strcmp(tag.c_str(), "ul") == 0) return 30;
	else if (strcmp(tag.c_str(), "ol") == 0) return 31;
	else if (strcmp(tag.c_str(), "input") == 0) return 40;
	else if (strcmp(tag.c_str(), "center") == 0) return 100;
	else if (strcmp(tag.c_str(), "table") == 0) return 200;
	else if (strcmp(tag.c_str(), "img") == 0) return 1000;
	else return -1;
}

int errorEventHandler(HWND hWnd, int n)
{
	if (n == 4001)
		MessageBox(hWnd, "Please, input a valid protocol (only http://)", "Button", MB_OK);
	else if (n == -1)
		MessageBox(hWnd, "A issue has occured in the program", "Button", MB_OK);
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