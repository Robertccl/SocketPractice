#ifndef __TCPSERVER__H
#define __TCPSERVER__H

#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <winsock2.h>
#include <string>
#include <iostream>

#include "MySession.h"
#include "Users.h"
#include "Protocol_TCP.h"
#include "MyUtil.h"

using namespace std;

class  TcpServer
{
public:
	TcpServer(const char* ip, int tempPort);
	~TcpServer();

public:
	/*static TcpServer& GetInstance()
	{
		static TcpServer tcpServer("127.0.0.1", 5150);
		return tcpServer;
	}*/
	void setIpServer(const char* ip, int tempPort);
	SOCKET GetListen();
	void CloseClient();
	void Close();

public:
	SOCKET AcceptConn(SOCKADDR_IN clientAddr);
	int Receive_Header_PKG(SOCKET clientSock, char* buff);
	int Receive_PKG(SOCKET clientSock, char* buff, int len);
	int Send_PKG(SOCKET clientSock, char *buff, int len);


public:
	WSADATA wsaData;
	SOCKET serverSock;
	SOCKADDR_IN serverAddr;

	Util& u = Util::GetInstance();;

};


#endif

