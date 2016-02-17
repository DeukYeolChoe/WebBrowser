#ifndef PARSER_H
#define PARSER_H

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <regex>

#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")
#pragma warning(disable:4996)
using namespace std;

#define SIZE 2048

class Parser
{
private:

public:
	Parser();
	string getValueFromHTMLTag(string content);
	char* Parser::removeHTMLTag(char* content);
};

#endif