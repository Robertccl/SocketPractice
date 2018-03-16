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
	userManager.LoadUserDB("D:/user.txt");
	
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


int TcpChatServer::ProcessPkt_tcp(const char* pkt, Session session)
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
	Protocol::Header* header_login = (Protocol::Header*)pkt;
	char buff[512];
	memset(buff, '\0', 512);
	strcpy(buff, (char *)header_login);
	server->Receive_PKG(session.clientSock, (buff+8), 40);
	Protocol::LoginPkt* loginPkt = (Protocol::LoginPkt*)buff;
	user.SetName(loginPkt->userName);
	user.SetPassword(loginPkt->password);
	session.SetUserName(user.GetName());
	mutex_server.lock();
	if (MyConst::USER_FOUND == userManager.CheckUser(user))  //  && (!sessionManager.FindSession(session))
	{
		sessionManager.AddSession(session);
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
		mutex_server.unlock();

		BroadcastPkt_tcp(&publicChatMsg);

	}
	else if (sessionManager.FindSession(session))
	{
		mutex_server.unlock();
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
	Protocol::Header* header_logout = (Protocol::Header*)pkt;
	char buff[512];
	memset(buff, '\0', 512);
	strcpy(buff, (char *)header_logout);
	server->Receive_PKG(session.clientSock, (buff+8), header_logout->length);
	Protocol::LogoutPkt* logoutPkt = (Protocol::LogoutPkt*)buff;
	user.SetName(logoutPkt->userName);
	session.SetUserName(user.GetName());
	mutex_server.lock();
	if (sessionManager.FindSession(session))
	{
		Protocol::PublicChatMsg publicChatMsg;
		memset(&publicChatMsg, '\0', sizeof(publicChatMsg));
		publicChatMsg.header.type = MyConst::SYSTEM_PKT;
		strcpy(publicChatMsg.from, "system");
		sprintf(publicChatMsg.message, "%s  logout", logoutPkt->userName);
		mutex_server.unlock();

		BroadcastPkt_tcp(&publicChatMsg);

		Protocol::LoginRelpyPkt replyPkt;
		memset(&replyPkt, '\0', sizeof(replyPkt));
		replyPkt.header.type = MyConst::LOGIN_REPLY_PKT;
		replyPkt.header.length = sizeof(int);
		replyPkt.retCode = MyConst::LOGOUT_SUCCESS;
		
		server->Send_PKG(session.clientSock, (char*)&replyPkt, replyPkt.header.length + 8);

		mutex_server.lock();
		sessionManager.RemoveSession(session);
		mutex_server.unlock();
	}
	else if (!sessionManager.FindSession(session))
	{
		mutex_server.unlock();
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
	Protocol::Header* p = (Protocol::Header*)pkt;
	Protocol::Header header_public;
	memset(&header_public, '\0', sizeof(header_public));
	header_public.type = p->type;
	header_public.length = p->length;
	char buff[512];
	memset(buff, '\0', 512);
	strcpy(buff, (char*)&header_public);
	server->Receive_PKG(session.clientSock, (buff + 8), header_public.length);

	Protocol::PublicChatMsg* msg = (Protocol::PublicChatMsg*)buff;
	msg->header.type = MyConst::PUBLIC_CHAT_PKT;

	return BroadcastPkt_tcp(msg);
}
int TcpChatServer::ProcessPrivateChatPkt_tcp(const char* pkt, Session session)
{
	User from, to;
	Protocol::Header* header_privatechat = (Protocol::Header*)pkt;
	char buff[512];
	memset(buff, '\0', 512);
	server->Receive_PKG(session.clientSock, (buff+8), header_privatechat->length);

	Protocol::PrivateChatMsg* privateMsg = (Protocol::PrivateChatMsg*)buff;

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
	mutex_server.lock();
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
	mutex_server.unlock();
	return 0;
}
int TcpChatServer::BroadcastPkt_tcp(Protocol::PublicChatMsg* publicChatMsg)
{
	cout << publicChatMsg->header.type << endl;
	cout << publicChatMsg->message << endl;
	Protocol::PublicChatMsg p;
	memset(&p, '\0', sizeof(p));
	strcpy(p.from, publicChatMsg->from);
	strcpy(p.message, publicChatMsg->message);
	
	p.header.type = publicChatMsg->header.type;
	p.header.length = 20 + strlen(p.message);
	mutex_server.lock();
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
	mutex_server.unlock();
	return 0;
}