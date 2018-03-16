#include "TcpServer.h"

TcpServer::TcpServer(const char* ip, int tempPort)
{
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		u.HandleError("WSAStartup failed!");
	serverSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (serverSock == INVALID_SOCKET)
		u.HandleError("Create ServerSock failed!");
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	if (NULL != ip)
		serverAddr.sin_addr.S_un.S_addr = inet_addr(ip);
	else
		serverAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	serverAddr.sin_port = htons(tempPort);

	if (bind(serverSock, (sockaddr *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
		u.HandleError("bind server failed!");
	if (listen(serverSock, 5) == SOCKET_ERROR)
		u.HandleError("listen error");

	cout << "¿ªÊ¼¼àÌý¡£¡£¡£" << endl;
}

TcpServer::~TcpServer()
{
	//CloseClient();
	Close();
}

SOCKET TcpServer::GetListen()
{
	return serverSock;
}

void TcpServer::setIpServer(const char* ip, int tempPort)
{
	
}

void TcpServer::CloseClient()
{
	//closesocket(clientSock);
}
	
void TcpServer::Close()
{
	
	//closesocket(clientSock);
	WSACleanup();
}


SOCKET TcpServer::AcceptConn(SOCKADDR_IN clientAddr)
{
	SOCKET clientSocket;
	int clientAdrSize = sizeof(clientAddr);
	clientSocket = accept(serverSock, (SOCKADDR *)&clientAddr, &clientAdrSize);
	if (clientSocket != INVALID_SOCKET)
		return clientSocket;
}

int TcpServer::Receive_Header_PKG(SOCKET clientSock, char* buff)
{
	return recv(clientSock, buff, 2*sizeof(int), 0);
}

int TcpServer::Receive_PKG(SOCKET clientSock, char* buff, int len)
{
	return recv(clientSock, buff, len, 0);
}

int TcpServer::Send_PKG(SOCKET clientSock, char *buff, int len)
{
	return send(clientSock, buff, len, 0);
}
