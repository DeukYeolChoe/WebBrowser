#define WIN32_LEAN_AND_MEAN

#include "socket.h"
#include "parser.h"

using namespace std;

Socket::Socket()
{
}

int Socket::ClientToServer(char* url, HWND hWnd)
{
	WSADATA wsaData;
	Parser parser;
	SOCKET ConnectSocket = INVALID_SOCKET;
	HDC hdc;

	struct addrinfo *result = NULL, *ptr = NULL, hints;
	struct hostent *remoteHost;
	struct in_addr addr;

	char sendbuf[SIZE];
	char recvbuf[SIZE];
	char* split;
	char* ipAddrs;
	char* port;
	char* path;
	char temp[SIZE];
	string text;
	char* text_c;
	//char **pAlias;

	int iResult;
	int recvbuflen = SIZE;
	int dwError;
	int n;

	strcpy(temp, url);
	split = strtok(temp, "://");
	split = strtok(NULL, "://");
	split = strtok(split, ":");
	ipAddrs = strdup(split);

	split = strtok(url, ":");
	split = strtok(NULL, ":");
	split = strtok(NULL, ":");
	split = strtok(split, "/");
	port = strdup(split);

	split = strtok(NULL, "/");
	if (split != NULL)
	{
		path = strdup(split);
		n = sprintf(sendbuf, "GET %s HTTP/1.1\r\n\r\n", path);
	}
	else
	{
		strcpy(sendbuf, "GET HTTP/1.1\r\n\r\n");
	}

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
		return 1;
	}

	remoteHost = gethostbyname(ipAddrs);
	if (remoteHost == NULL) {
		dwError = WSAGetLastError();
		if (dwError != 0) {
			if (dwError == WSAHOST_NOT_FOUND) {
				printf("Host not found\n");
				return 1;
			}
			else if (dwError == WSANO_DATA) {
				printf("No data record found\n");
				return 1;
			}
			else {
				printf("Function failed with error: %ld\n", dwError);
				return 1;
			}
		}
	}
	else {
		printf("Function returned:\n");
		printf("\tOfficial name: %s\n", remoteHost->h_name);
		printf("\tAddress type: ");
		switch (remoteHost->h_addrtype) {
		case AF_INET:
			printf("AF_INET\n");
			break;
		case AF_NETBIOS:
			printf("AF_NETBIOS\n");
			break;
		default:
			printf("%d\n", remoteHost->h_addrtype);
			break;
		}
		printf("\tAddress length: %d\n", remoteHost->h_length);

		int i = 0;
		if (remoteHost->h_addrtype == AF_INET)
		{
			while (remoteHost->h_addr_list[i] != 0) {
				addr.s_addr = *(u_long *)remoteHost->h_addr_list[i++];
				printf("\tIP Address #%d: %s\n", i, inet_ntoa(addr));
				ipAddrs = inet_ntoa(addr);
			}
		}
		else if (remoteHost->h_addrtype == AF_NETBIOS)
		{
			printf("NETBIOS address was returned\n");
		}
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// Resolve the server address and port0
	if (port == NULL)
		port = "80";
	iResult = getaddrinfo(ipAddrs, port, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		return 1;
	}

	// Attempt to connect to an address until one succeeds
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {
		// Create a SOCKET for connecting to server
		ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
		if (ConnectSocket == INVALID_SOCKET) {
			printf("socket failed with error: %ld\n", WSAGetLastError());
			WSACleanup();
			return 1;
		}

		// Connect to server
		iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			closesocket(ConnectSocket);
			ConnectSocket = INVALID_SOCKET;
			continue;
		}
		break;
	}

	freeaddrinfo(result);

	if (ConnectSocket == INVALID_SOCKET) {
		printf("Unable to connect to server!\n");
		WSACleanup();
		return 1;
	}

	// Send an initial buffer
	iResult = send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);
	if (iResult == SOCKET_ERROR) {
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return 1;
	}

	printf("Bytes Sent: %ld\n", iResult);

	// shutdown the connection since no more data will be sent
	iResult = shutdown(ConnectSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed with error: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return 1;
	}

	// Receive until the peer closes the connection
	int x = 40;
	int y = 40;
	memset(&g_buf, '\0', sizeof(g_buf));
	do {
		memset(&recvbuf, '\0', sizeof(recvbuf));
		iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0)
		{
			printf("Bytes received: %d\n", iResult);

			hdc = GetDC(hWnd);
			split = strtok(recvbuf, "\r\n");
			while (split != NULL)
			{
				strcpy(temp, split);
				//text = parser.getValueFromHTMLTag(temp);
				//if (!text.empty())
				//{
				//	text_c = text.c_str();
				//	TextOut(hdc, x, y, text_c, strlen(text_c));
				//}
				//else
				//{
				text_c = parser.removeHTMLTag(temp);
					TextOut(hdc, x, y, text_c, strlen(text_c));
				//}
				
				strcat(g_buf, temp);
				strcat(g_buf, ";");
				split = strtok(NULL, "\r\n");
				y += 30;
			}
			ReleaseDC(hWnd, hdc);
		}
		else if (iResult == 0)
			printf("Connection closed\n");
		else
			printf("recv failed with error: %d\n", WSAGetLastError());
	} while (iResult > 0);

	// cleanup
	closesocket(ConnectSocket);
	WSACleanup();

	return 0;
}

