#ifndef IMAGER_H
#define IMAGER_H

//#define _WIN32_WINNT 0x0501
//#define WINVER 0x0501

#include <Unknwn.h>
#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <gdiplus.h>
#include <fstream>
//#include <tchar.h>
#include <string>
#include <vector>

#define _UNICODE
#define UNICODE

using namespace Gdiplus;
#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "Ole32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "comctl32.Lib")
#pragma comment(lib, "gdi32.Lib")

#define BUFSIZE 1024

class Imager
{
	private:
	protected:
	public:
		Imager();
		int setImage(HWND hWnd, HDC hdc, const wchar_t* filePath, int x, int y, int width, int height);
		std::string GetExeFileName(std::string fileName);
};

#endif