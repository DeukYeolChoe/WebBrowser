#ifndef VALIDATE_H
#define VALIDATE_H

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <algorithm>

using namespace std;

#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")
#pragma warning(disable:4996)

#define SIZE 1024

class Validate
{
	private:
	public:
		Validate();
		int isURLValid(string content);
		int isHTMLValid(string content);
};

#endif