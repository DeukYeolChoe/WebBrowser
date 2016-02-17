#include "imager.h"

Imager::Imager()
{
}

int Imager::setImage(HWND hWnd, HDC hdc, const wchar_t* filePath, int x, int y, int width, int height)
{
	Graphics graphics(hdc);
	Image img(filePath);
	graphics.DrawImage(&img, x, y, width, height);
	return 0;
}

std::string Imager::GetExeFileName(std::string fileName)
{
	char buffer[MAX_PATH];
	GetModuleFileName(NULL, buffer, MAX_PATH);
	std::string imgPath(buffer);
	int found = imgPath.find("x64");
	imgPath = imgPath.substr(0, found);
	imgPath = imgPath.append("Images\\");
	imgPath = imgPath.append(fileName);
	return imgPath;
}