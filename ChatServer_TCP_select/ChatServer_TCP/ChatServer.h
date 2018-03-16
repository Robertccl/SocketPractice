#ifndef __CHATSERVER__H
#define __CHATSERVER__H

#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <winsock2.h>
#include <string>
#include <iostream>

#include "MySession.h"
#include "Users.h"
#include "Protocol_TCP.h"

using namespace std;



class ChatServer
{
public:
	ChatServer();
	~ChatServer();

public:
	virtual void Run() = 0;

	//virtual void processThread(SOCKET client) = 0;

	virtual int ProcessPkt_tcp(const char* pkt, Session &session) = 0;

	virtual int ProcessLoginPkt_tcp(const char* pkt, Session session) = 0;
	virtual int ProcessLogoutPkt_tcp(const char* pkt, Session session) = 0;
	virtual int ProcessPublicChatPkt_tcp(const char* pkt, Session session) = 0;
	virtual int ProcessPrivateChatPkt_tcp(const char* pkt, Session session) = 0;
	virtual int BroadcastPkt_tcp(Protocol::PublicChatMsg* publicChatMsg) = 0;
};




#endif


