#define WIN32_LEAN_AND_MEAN

#include "socket.h"
#include "parser.h"
#include "validate.h"

using namespace std;

Socket::Socket()
{
}

int Socket::ClientToServer(wchar_t* url, HWND hWnd, string method, string param, bool flag)
{
	WSADATA wsaData;
	SOCKET ConnectSocket = INVALID_SOCKET;
	Parser parser;
	Validate valid;
	Imager imager;
	FILE *wfp;

	struct addrinfo *result = NULL, *ptr = NULL, hints;
	struct hostent *remoteHost;

	char sendbuf[BUFSIZE];
	char recvbuf[BUFSIZE];
	char* split;

	string path;	
	bool imgCreated = false;
	int iResult;
	int found = 0;

	wstring ws(url);
	string strurl(ws.begin(), ws.end());

	if (valid.isURLValid(strurl) == -1) return 4001;
	ipAddrs = parser.getIPaddrsFromURL(strurl);
	port = parser.getPortFromURL(strurl);
	path = parser.getPathFromURL(strurl);

	// 파일 열기
	string requestFile = "";
	if (flag)
	{
		requestFile = path;
		while ((found = requestFile.find("/")) >= 0)
		{
			requestFile = requestFile.erase(0, found + 1);
		}

		// 만약 존재한다면 받지 않는다.
		if (isFileExist(requestFile))
			return 0;

		string imgPath = imager.GetExeFileName(requestFile);
		wfp = fopen(imgPath.c_str(), "wb");	
	}

	strcpy(sendbuf, createHTTPHeader(method, ipAddrs, path, param).c_str());

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
		return -1;
	}

	remoteHost = gethostbyname(ipAddrs.c_str());
	if (getRemoteHostInfo(remoteHost, ipAddrs) == -1) return -1;

	std::memset(&hints, '\0', sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// Resolve the server address and port0
	iResult = getaddrinfo(ipAddrs.c_str(), port.c_str(), &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		return -1;
	}

	// Attempt to connect to an address until one succeeds
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {
		// Create a SOCKET for connecting to server
		ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
		if (ConnectSocket == INVALID_SOCKET) {
			printf("socket failed with error: %ld\n", WSAGetLastError());
			WSACleanup();
			return -1;
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
		return -1;
	}

	// Send an initial buffer
	iResult = send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);
	if (iResult == SOCKET_ERROR) {
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return -1;
	}
	std::memset(&sendbuf, '\0', sizeof(sendbuf));
	
	printf("Bytes Sent: %ld\n", iResult);

	// Shutdown the connection since no more data will be sent
	iResult = shutdown(ConnectSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed with error: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return -1;
	}

	// Receive until the peer closes the connection
	std::memset(&recvbuf, '\0', sizeof(recvbuf));
	string repository = "";
	int totalBytes = 0;
	wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;

	while ((iResult = recv(ConnectSocket, recvbuf, BUFSIZE, 0)) > 0)
	{
		if (!flag)
		{
			repository.append(recvbuf, iResult);
			totalBytes += iResult;
		}
		//if (((found = path.find(".bmp")) >= 0 || (found = path.find(".jpg")) >= 0 || (found = path.find(".png") >= 0)) && (flag && strstr(recvbuf, "HTTP/1.") == 0))
		if (flag && strstr(recvbuf, "HTTP/1.") == NULL)
		{
			fwrite(recvbuf, iResult, 1, wfp);
		}
		else if (iResult == 0) printf("Connection closed\n");
		else printf("recv failed with error: %d\n", WSAGetLastError());

		std::memset(&recvbuf, '\0', sizeof(recvbuf));	
	}

	int nLen = MultiByteToWideChar(CP_UTF8, 0, repository.c_str(), totalBytes, NULL, 0);
	wstring wstr(nLen, 0);
	MultiByteToWideChar(CP_UTF8, 0, repository.c_str(), -1, &wstr[0], nLen);
	wcout << wstr << endl;

	v_htmlBuf.push_back(wstr);

	// cleanup
	closesocket(ConnectSocket);
	WSACleanup();
	
	if (flag) fclose(wfp);
	return 0;
}

int Socket::getRemoteHostInfo(hostent *remoteHost, string ipAddrs)
{
	int dwError;
	struct in_addr addr;
	if (remoteHost == NULL) {
		dwError = WSAGetLastError();
		if (dwError != 0) {
			if (dwError == WSAHOST_NOT_FOUND) {
				printf("Host not found\n");
				return -1;
			}
			else if (dwError == WSANO_DATA) {
				printf("No data record found\n");
				return -1;
			}
			else {
				printf("Function failed with error: %ld\n", dwError);
				return -1;
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
	return 1;
}

bool Socket::isFileExist(string fileName)
{
	Imager imager;
	string imgPath = imager.GetExeFileName(fileName);
	std::ifstream infile(imgPath);
	return infile.good();
}

string Socket::createHTTPHeader(string method, string ipAddrs ,string file, string param)
{
	string header = "";
	header.append(method + " " + file + " ");
	//버전
	header.append("HTTP/1.1\r\n\r\n");
	//호스트
	header.append("Host: " + ipAddrs + "\r\n");
	//컨넥션
	header.append("Connection: keep-alive\r\n");
	if (method == "POST")
	{
		header.append("Content-Length: ");
		int length = param.length();
		header.append(to_string(length) + " \r\n");
	}
	//캐쉬
	header.append("Cache-Control: max-age=0 \r\n");
	//액셉트
	header.append("Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8 \r\n");
	//header.append("Origin: \r\n");
	header.append("Upgrade-Insecure-Requests: 1 \r\n");
	header.append("User-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/48.0.2564.109 Safari/537.36 \r\n");
	header.append("Content-Type: application/x-www-form-urlencoded \r\n");
	//header.append("Referer: \r\n");
	//엔코딩
	header.append("Accept-Encoding: gzip, deflate, sdch \r\n");
	header.append("Accept-Language: ko-KR,ko;q=0.8,en-US;q=0.6,en;q=0.4 \r\n\r\n");
	header.append(param);
	return header;
}