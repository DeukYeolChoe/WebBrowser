#ifndef SOCKET_H
#define SOCKET_H

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include "parser.h"
#include "Imager.h"

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")
#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")
#pragma warning(disable:4996)

using namespace std;

#define BUFSIZE 1024

class Socket
{
	private:
	protected:
	public:	
		string ipAddrs;
		string port;
		vector<wstring> v_htmlBuf;
		char g_imgBuf[BUFSIZE];
		ImgAttr imgInfo;

		Socket();
		int ClientToServer(wchar_t* url, HWND hWnd, string method, string para, bool flag);
		int getRemoteHostInfo(hostent *remoteHost, string ipAddrs);
		bool isFileExist(string fileName);
		string createHTTPHeader(string method, string file, string param);
};

#endif