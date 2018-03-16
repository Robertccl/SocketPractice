#pragma comment(lib, "ws2_32.lib")

#include <iostream>
#include "ChatServer_TCP.h"
#include "MySession.h"
#include "Protocol_TCP.h"
#include "Users.h"

using namespace std;

void processThread(void* lpParam);


int main ()
{
	TcpServer *server = new TcpServer("127.0.0.1", 5150);
	TcpChatServer *tcpChatServer =new TcpChatServer("127.0.0.1", 5150, server);
	tcpChatServer->Run();
	
	while (1)
	{
		SOCKET	sNewConnection=0;
		SOCKADDR_IN clientAddr;
		memset(&clientAddr, 0, sizeof(clientAddr));
		int clientAdrSize = sizeof(clientAddr);
		
		sNewConnection = server->AcceptConn(clientAddr);
		if (sNewConnection != 0)
		{
			threadNode tnode;
			tnode.tcpChatServer = tcpChatServer;
			tnode.server = server;
			tnode.sNewConnection = sNewConnection;
			cout << "连接成功...." << endl;
			_beginthread(processThread, 0, (void *)&tnode);
			
		}
	}
	return 0;
}

void processThread(void* lpParam)
{
	threadNode *tnode = (threadNode*)lpParam;
	SOCKET client = tnode->sNewConnection;
	TcpServer * server = tnode->server;
	TcpChatServer *tcpChatServer = tnode->tcpChatServer;
	while (1)
	{
		char buff[255] = { 0 };
		server->Receive_Header_PKG(client, buff);
		Session s;
		s.clientSock = client;
		tcpChatServer->ProcessPkt_tcp(buff, s);
		//memset(buff, '\0', 255);
	}

}