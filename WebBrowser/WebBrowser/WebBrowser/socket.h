#ifndef SOCKET_H
#define SOCKET_H

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")
#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")
#pragma warning(disable:4996)

#define SIZE 2048

class Socket
{
	private:
		char g_buf[SIZE];
	public:
		Socket();
		int ClientToServer(char* url, HWND hWnd);
};

#endif