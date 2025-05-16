#include "pch.h"
#include <iostream>

int main()
{
	SocketUtils::Init();

	this_thread::sleep_for(1s);

	// 블로킹 소켓
	// accept -> 접속한 클라가 있을 때
	// connect -> 서버 접속 성공했을 때
	// send -> 요청한 데이터를 송신 버퍼에 복사했을 때
	// recv -> 수신 버퍼에 도착한 데이터가 있고, 이를 유저레벨 버퍼에 복사했을 때

	SOCKET clientSocket = ::socket(AF_INET, SOCK_STREAM, 0);
	if (clientSocket == INVALID_SOCKET)
		return 0;

	// 논블로킹 소켓으로
	u_long on = 1;
	if (::ioctlsocket(clientSocket, FIONBIO, &on) == INVALID_SOCKET)
		return 0;

	SOCKADDR_IN serverAddr;
	::memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	//serverAddr.sin_addr.s_addr = ::inet_addr("127.0.0.1"); << deprecated
	::inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);
	serverAddr.sin_port = ::htons(7777); // 80 : HTTP

	// Connect
	while (true)
	{
		if (::connect(clientSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
		{
			// 원래 블록했어야 했는데 ... 너가 논블로킹 하라며?
			if (::WSAGetLastError() == WSAEWOULDBLOCK)
				continue;

			// 이미 연결된 상태라면 break;
			if (::WSAGetLastError() == WSAEISCONN)
				break;
		}
	}

	// Send
	while (true)
	{
		char sendBuffer[100] = "Hello I am Client!";
		int32 sendLen = sizeof(sendBuffer);

		if (::send(clientSocket, sendBuffer, sendLen, 0) == SOCKET_ERROR)
		{
			// 원래 블록했어야 했는데 ... 너가 논블로킹 하라며?
			if (::WSAGetLastError() == WSAEWOULDBLOCK)
				continue;

			cout << "Send Data ! Len = " << sendLen << endl;			
		}

		this_thread::sleep_for(1s);
	}


	SocketUtils::Close(clientSocket);
}