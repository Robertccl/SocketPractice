#ifndef __CHATSERVER_TCP__H
#define __CHATSERVER_TCP__H

#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <winsock2.h>
#include <string>
#include <iostream>
#include <process.h>
#include <thread>
#include <mutex>

#include "ChatServer.h"
#include "TcpServer.h"
#include "MySession.h"
#include "Users.h"
#include "Protocol_TCP.h"
#include "MyConst.h"



class TcpChatServer : public ChatServer
{
public:
	TcpChatServer(const char* ip, int tempPort, TcpServer *ss);
	TcpChatServer(const char* ip, int tempPort);
	~TcpChatServer();

public:
	void Run();

	//void processThread(SOCKET client);
	friend void processThread(void* lpParam);
	int ProcessPkt_tcp(const char* pkt, Session &session);

	int ProcessLoginPkt_tcp(const char* pkt, Session session);
	int ProcessLogoutPkt_tcp(const char* pkt, Session session);
	int ProcessPublicChatPkt_tcp(const char* pkt, Session session);
	int ProcessPrivateChatPkt_tcp(const char* pkt, Session session);
	int BroadcastPkt_tcp(Protocol::PublicChatMsg* publicChatMsg);

public:
	mutex mutex_server;

private:
	TcpServer *server;
	UserManager &userManager = UserManager::GetInstance();
	SessionManager &sessionManager = SessionManager::GetInstance();
};

typedef struct threadNode
{
	TcpChatServer *tcpChatServer;
	TcpServer *server;
	SOCKET	sNewConnection;
}threadNode;


#endif
