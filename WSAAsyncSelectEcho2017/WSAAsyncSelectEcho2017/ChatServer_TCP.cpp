#include "ChatServer_tcp.h"

TcpChatServer::TcpChatServer(const char* ip, int port)
{
	server= new TcpServer(ip, port);
}
TcpChatServer::TcpChatServer(const char* ip, int tempPort, TcpServer *ss)
{
	server = ss;
}
TcpChatServer::~TcpChatServer()
{
	//server.CloseClient();
	server->Close();
}


void TcpChatServer::Run()
{
	char buff[2048];
	userManager.LoadUserDB("D:/user.txt");
	while (true)
	{
		while (sessionManager.OnlineNumber() > 0)
		{
			vector<Session>::iterator it;
			int i = 0;
			int onlineNum = sessionManager.OnlineNumber();
			it = sessionManager.GetBeginIndex();
			while (i < onlineNum)
			{
				Session &s = sessionManager.GetSession(it);
				if (s.messageState == MyConst::MESSAGE_GET)
				{
					memset(buff, '\0', 2048);
					memcpy(buff,s.messagePkt.front().message, s.messagePkt.front().msgLength);
					ProcessPkt_tcp(buff, s);
				}
				if(sessionManager.OnlineNumber() != 0 )
					it++;
				i++;
			}
		}
	}
	
	
	
		//Session session;
		//char buff[1024] = { 0 };
		//cout << "开始监听：。。。。" << endl;
		//int recvBytes;//= server.Receive(buff, 1024, &(session.clientAddr), &(session.clientAddrLen));

		//if (recvBytes > 0)
		//{
		//	//ProcessPkt(buff, session);
		//}
	
}

//void TcpChatServer::processThread(SOCKET client) 


int TcpChatServer::ProcessPkt_tcp(const char* pkt, Session &session)
{
	int type = ((Protocol::Header*)pkt)->type;
	//如果是登录包，做登录处理
	if (MyConst::LOGIN_PKT == type)
	{
		return ProcessLoginPkt_tcp(pkt, session);
	}
	else if (MyConst::LOGOUT_PKT == type)//否则如果是登出包，做登出处理
	{
		return ProcessLogoutPkt_tcp(pkt, session);
	}
	else if (MyConst::PUBLIC_CHAT_PKT == type)//否则如果是公聊包，做公聊处理
	{
		return ProcessPublicChatPkt_tcp(pkt, session);
	}
	else if (MyConst::PRIVATE_CHAT_PKT == type)
	{
		return ProcessPrivateChatPkt_tcp(pkt, session);
	}
	else //否则，错误处理
	{
		return MyConst::UNKNOWN_PKT_FORMAT;
	}
}

int TcpChatServer::ProcessLoginPkt_tcp(const char* pkt, Session session)
{
	User user;
	
	Protocol::LoginPkt* loginPkt = (Protocol::LoginPkt*)pkt;
	user.SetName(loginPkt->userName);
	user.SetPassword(loginPkt->password);
	session.SetUserName(user.GetName());
	
	if (MyConst::USER_FOUND == userManager.CheckUser(user))  //  && (!sessionManager.FindSession(session))
	{
		vector<Session>::iterator it = sessionManager.GetSessionBySocket(session.clientSock);

		it->SetUserName(user.GetName());
		it->UserLogin();
		it->ClearMsg();
		Protocol::LoginRelpyPkt replyPkt;
		memset(&replyPkt, '\0', sizeof(replyPkt));
		replyPkt.header.type = MyConst::LOGIN_REPLY_PKT;
		replyPkt.header.length = sizeof(int);
		replyPkt.retCode = MyConst::LOGIN_SUCCESS;
		server->Send_PKG(session.clientSock, (char*)&replyPkt, replyPkt.header.length+8);
		/*sendto(sock, (const char*)&replyPkt, sizeof(replyPkt),
		0, addr, sizeof(sockaddr));*/

		Protocol::PublicChatMsg publicChatMsg;
		memset(&publicChatMsg, '\0', sizeof(publicChatMsg));
		publicChatMsg.header.type = MyConst::SYSTEM_PKT;
		publicChatMsg.header.length = 0;
		strcpy(publicChatMsg.from, "system");
		sprintf(publicChatMsg.message, "%s  login", loginPkt->userName);
		

		BroadcastPkt_tcp(&publicChatMsg);

	}
	else if (sessionManager.FindSession(session))
	{
		
		Protocol::LoginRelpyPkt replyPkt;
		memset(&replyPkt, '\0', sizeof(replyPkt));
		replyPkt.header.type = MyConst::LOGIN_REPLY_PKT;
		replyPkt.header.length = sizeof(int);
		replyPkt.retCode = MyConst::USER_HAS;

		server->Send_PKG(session.clientSock, (char*)&replyPkt, replyPkt.header.length + 8);
		
	}
	return 0;
}
int TcpChatServer::ProcessLogoutPkt_tcp(const char* pkt, Session session)
{
	User user;
	
	Protocol::LogoutPkt* logoutPkt = (Protocol::LogoutPkt*)pkt;
	user.SetName(logoutPkt->userName);
	session.SetUserName(user.GetName());
	
	if (sessionManager.FindSession(session))
	{
		Protocol::PublicChatMsg publicChatMsg;
		memset(&publicChatMsg, '\0', sizeof(publicChatMsg));
		publicChatMsg.header.length = 20 + strlen(publicChatMsg.message);
		publicChatMsg.header.type = MyConst::SYSTEM_PKT;
		strcpy(publicChatMsg.from, "system");
		sprintf(publicChatMsg.message, "%s  logout", logoutPkt->userName);
		

		BroadcastPkt_tcp(&publicChatMsg);
		sessionManager.RemoveSession(session);


		/*Protocol::LoginRelpyPkt replyPkt;
		memset(&replyPkt, '\0', sizeof(replyPkt));
		replyPkt.header.type = MyConst::LOGIN_REPLY_PKT;
		replyPkt.header.length = sizeof(int);
		replyPkt.retCode = MyConst::LOGOUT_SUCCESS;
		
		server->Send_PKG(session.clientSock, (char*)&replyPkt, replyPkt.header.length + 8);*/

		
	}
	else if (!sessionManager.FindSession(session))
	{
		
		Protocol::LoginRelpyPkt replyPkt;
		memset(&replyPkt, '\0', sizeof(replyPkt));
		replyPkt.header.type = MyConst::LOGIN_REPLY_PKT;
		replyPkt.header.length = sizeof(int);
		replyPkt.retCode = MyConst::USER_UNFOUND;
		server->Send_PKG(session.clientSock, (char*)&replyPkt, replyPkt.header.length + 8);
		
	}
	
	return 0;
}
int TcpChatServer::ProcessPublicChatPkt_tcp(const char* pkt, Session session)
{
	
	//server->Receive_PKG(session.clientSock, (buff + 8), header_public.length);

	Protocol::PublicChatMsg* msg = (Protocol::PublicChatMsg*)pkt;
	msg->header.type = MyConst::PUBLIC_CHAT_PKT;
	BroadcastPkt_tcp(msg);
	vector<Session>::iterator it = sessionManager.GetSessionBySocket(session.clientSock);
	it->ClearMsg();

	return 0;
}
int TcpChatServer::ProcessPrivateChatPkt_tcp(const char* pkt, Session session)
{
	User from, to;
	
	//server->Receive_PKG(session.clientSock, (buff+8), header_privatechat->length);

	Protocol::PrivateChatMsg* privateMsg = (Protocol::PrivateChatMsg*)pkt;

	from.SetName(privateMsg->from);
	to.SetName(privateMsg->to);

	Protocol::PrivateChatMsg privateMsg_send;
	memset(&privateMsg_send, '\0', sizeof(privateMsg_send));
	privateMsg_send.header.type = MyConst::PRIVATE_CHAT_PKT;
	strcpy(privateMsg_send.from, privateMsg->from);
	strcpy(privateMsg_send.to, privateMsg->to);
	strcpy(privateMsg_send.message, privateMsg->message);
	privateMsg_send.retCode = privateMsg->retCode;
	privateMsg_send.header.length = 40 + strlen(privateMsg_send.message) + sizeof(int);
	vector<Session>::iterator it = sessionManager.GetSessionBySocket(session.clientSock);
	it->ClearMsg();
	if (from.GetName() != "" && to.GetName() != "")
	{
		Session s;
		s.SetUserName(to.GetName());
		if (sessionManager.FindSession(s))
		{
			Session s = sessionManager.GetSessionByName(to);

			server->Send_PKG(s.clientSock, (char *)&privateMsg_send, privateMsg_send.header.length + 8);
		}
		else
		{
			privateMsg->retCode = MyConst::USER_UNFOUND;
			server->Send_PKG(s.clientSock, (char *)&privateMsg, privateMsg_send.header.length + 8);
		}
	}
	
	return 0;
}
int TcpChatServer::BroadcastPkt_tcp(Protocol::PublicChatMsg* publicChatMsg)
{
	/*cout << publicChatMsg->header.type << endl;
	cout << publicChatMsg->message << endl;*/
	Protocol::PublicChatMsg p;
	memset(&p, '\0', sizeof(p));
	strcpy(p.from, publicChatMsg->from);
	strcpy(p.message, publicChatMsg->message);
	
	p.header.type = publicChatMsg->header.type;
	p.header.length = 20 + strlen(p.message);
	
	vector<Session>::iterator it;
	int i = 0;
	int onlineNum = sessionManager.OnlineNumber();
	it = sessionManager.GetBeginIndex();
	while (i < onlineNum)
	{
		Session s = sessionManager.GetSession(it);
		server->Send_PKG(s.clientSock, (char *)&p, p.header.length + 8);
		it++;
		i++;
	}
	
	return 0;
}