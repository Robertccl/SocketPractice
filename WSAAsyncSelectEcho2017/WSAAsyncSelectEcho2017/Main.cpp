#pragma comment(lib, "ws2_32.lib")

#include <iostream>
#include <winsock2.h>
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
	

	SOCKET	sNewConnection = server->GetListen();;
	threadNode tnode;
	tnode.tcpChatServer = tcpChatServer;
	tnode.server = server;
	tnode.sNewConnection = sNewConnection;
	
	_beginthread(processThread, 0, (void *)&tnode);

	tcpChatServer->Run();
	
	/*while (1)
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
	}*/
	return 0;
}

void processThread(void* lpParam)
{
	threadNode *tnode = (threadNode*)lpParam;
	SOCKET sListen = tnode->sNewConnection;
	TcpServer *server = tnode->server;
	
	fd_set fd_read;
	SessionManager &sessionManager = SessionManager::GetInstance();
	FD_ZERO(&fd_read);
	vector<Session>::iterator it;
	int i = 0;
	while (true)
	{
		
		i = 0;
		FD_SET(sListen, &fd_read);
		int onlineNum = sessionManager.OnlineNumber();;
		it = sessionManager.GetBeginIndex();
		while (i < onlineNum)
		{
			Session &s = sessionManager.GetSession(it);
			if(s.userName != "")
				FD_SET(s.clientSock, &fd_read);
			it++;
			i++;
		}

		it = sessionManager.GetBeginIndex();

		int ret = select(0, &fd_read, NULL, NULL, NULL);
		if (SOCKET_ERROR == ret)
		{
			printf("select error\n");
			break;
		}

		if (ret > 0)
		{
			if (FD_ISSET(sListen, &fd_read))
			{
				//将连接套接字加入到sConnection中
				if (onlineNum == MyConst::MAX_USER_NUM)
				{
					printf("connection is too much!\n");

				}
				else
				{
					SOCKADDR_IN clientAddr;
					memset(&clientAddr, 0, sizeof(clientAddr));
					int clientAdrSize = sizeof(clientAddr);
					Session s;
					//接收连接
					s.clientSock = server->AcceptConn(clientAddr);

					FD_SET(s.clientSock, &fd_read);
					
					if (INVALID_SOCKET == s.clientSock)
					{
						
						break;
					}
					s.state = MyConst::STATE_UNLOGIN;
					
					sessionManager.AddSession(s);
				}
			}
		}

		onlineNum = sessionManager.OnlineNumber();
		it = sessionManager.GetBeginIndex();
		i = 0;
		while (i < onlineNum)
		{
			
			if (FD_ISSET(it->clientSock, &fd_read))
			{
				//接收数据
				char buff[1024];
				memset(buff, 1024, '\0');
				int headerByte = server->Receive_Header_PKG(it->clientSock, buff);
				int len = ((Protocol::Header*)buff)->length;
				int type = ((Protocol::Header*)buff)->type;
				//string temp = buff;
				int recvBytes = server->Receive_PKG(it->clientSock, buff+headerByte, len);
				if (recvBytes <= 0) //出错时关闭
				{
					//sConnection[i] = INVALID_SOCKET;
					sessionManager.RemoveSession(*it);
				}
				int length = headerByte + recvBytes;
				//string temp = buff;
				char *pkt = new  char[length+10];
				memset(pkt, '\0', length + 10);

				Protocol::msgNode tempNode;
				tempNode.msgLength = length;
				tempNode.message = pkt;
				
				memcpy(pkt, buff, len+20);
				
				it->messagePkt.push(tempNode);
				it->messageState = MyConst::MESSAGE_GET;
			}
			if(it!=sessionManager.GetEndIndex())
				it++;
			i++;
		}
	}
}

//void processThread(void* lpParam)
//{
//	threadNode *tnode = (threadNode*)lpParam;
//	SOCKET client = tnode->sNewConnection;
//	TcpServer *server = tnode->server;
//	TcpChatServer *tcpChatServer = tnode->tcpChatServer;
//	while (1)
//	{
//		char buff[255] = { 0 };
//		server->Receive_Header_PKG(client, buff);
//		Session s;
//		s.clientSock = client;
//		tcpChatServer->ProcessPkt_tcp(buff, s);
//		//memset(buff, '\0', 255);
//	}
//
//}