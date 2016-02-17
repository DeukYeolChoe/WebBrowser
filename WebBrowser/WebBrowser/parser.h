#ifndef PARSER_H
#define PARSER_H

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <regex>
#include <algorithm>

#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")
#pragma warning(disable:4996)

using namespace std;

#define BUFSIZE 1024

struct ImgInfo {
	string src;
	string width;
	string height;
};

struct HtmlInfo {
	string status;
	string head;
	string body;
};

struct InputInfo {
	string type;
	string name;
	string value;
};

class Parser
{
	private:
	protected:
	public:
		HtmlInfo htmlInfo;

		Parser();
		string getIPaddrsFromURL(char* url);
		string getPortFromURL(char* url);
		string getPathFromURL(char* url);
		string getHTMLTag(string content);
		string removeHTMLTag(string content);
		ImgInfo getImgInfo(string content);
		HtmlInfo getHttpHeadBody(string content);
		string getFormAction(string content);
		string getFormMethod(string content);
		string getInputType(string content);
		string getInputValue(string content);
		string getInputName(string content);
};

#endif