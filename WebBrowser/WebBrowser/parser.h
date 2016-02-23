#ifndef PARSER_H
#define PARSER_H

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <regex>
#include <algorithm>
#include <codecvt>
#include "tree.h"

#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")
#pragma warning(disable:4996)

using namespace std;

#define BUFSIZE 1024

class Parser
{
	private:
	protected:
	public:
		Parser();
		string getIPaddrsFromURL(string url);
		string getPortFromURL(string url);
		string getPathFromURL(string url);
		wstring getHTMLTag(wstring content);
		wstring removeHTMLTag(wstring content);
		ImgAttr getImgInfo(wstring content);
		wstring getFormAction(wstring content);
		wstring getFormMethod(wstring content);
		wstring getInputType(wstring content);
		wstring getInputValue(wstring content);
		wstring getInputName(wstring content);
		wstring getSpanstyle(wstring content);
		wstring getHyperRef(wstring content);
		Attribute getAttributes(wstring tag, wstring content);
		bool isEnglishLetter(wstring letters);
};

#endif