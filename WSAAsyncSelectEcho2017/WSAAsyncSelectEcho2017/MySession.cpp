#include "MySession.h"
#include "MyConst.h"

Session::Session()
{
	userName = "";
	messageState = MyConst::MESSAGE_UNGET;
	state = MyConst::STATE_UNLOGIN;
	clientAddrLen = sizeof(sockaddr);
}

Session::Session(sockaddr* addr)
{
	clientAddr = *addr;
	clientAddrLen = sizeof(sockaddr);
}

Session::Session(string name, sockaddr*addr)
{
	userName = name;
	clientAddr = *addr;
	clientAddrLen = sizeof(sockaddr);
}
Session::~Session()
{
	userName = "";
	while (messagePkt.size() > 0)
	{
		messagePkt.pop();
	}
	messageState = MyConst::MESSAGE_UNGET;
	state = MyConst::STATE_UNLOGIN;
}

void Session::SetUserName(string name)
{
	userName = name;
}
string Session::GetuserName()
{
	return userName;
}

void Session::SetClientAddr(sockaddr* addr)
{
	clientAddr = *addr;
	clientAddrLen = sizeof(sockaddr);
}

void Session::UserLogin()
{
	state = MyConst::STATE_LOGINED;

}

void Session::UserLogout()
{
	state = MyConst::STATE_UNLOGIN;
}

void Session::ClearMsg()
{
	messageState = MyConst::MESSAGE_UNGET;
	messagePkt.pop();
}

sockaddr* Session::GetSockaddr()
{
	return &clientAddr;
}

void Session::SetSecretSessionID()
{
	secretSessionID = "aaaa";
}
string Session::GetSecretSessionID()
{
	return secretSessionID;
}

Session Session::operator=(const Session s)
{
	Session s1;
	s1.SetUserName(s.userName);
	s1.messageState = s.messageState;
	s1.messagePkt = s.messagePkt;
	s1.clientSock = s.clientSock;
	return s1;
}

bool operator==(const Session &s1, const Session &s2)
{
	if(s1.userName == s2.userName)
		return true;
	else
		return false;
}

bool Session::Empty()
{
	if (userName == "" && clientSock == 0)
		return true;
	else
		return false;
}


SessionManager::SessionManager()
{

}
SessionManager::~SessionManager()
{
	sessions.clear();
}

void SessionManager::AddSession(Session s)
{
	if(!FindSession(s))
		sessions.push_back(s);
}

bool SessionManager::FindSession(Session s)
{
	vector<Session>::iterator i;
	if (sessions.begin() == sessions.end())
		return false;
	for (i = sessions.begin(); i != sessions.end(); i++)
	{
		if (*i == s)
			return true;
	}
	return false;
}
void SessionManager::RemoveSession(Session &s)  
{
	vector<Session>::iterator i;
	i = sessions.begin();
	while(i!=sessions.end())
	{
		if (*i == s)
			sessions.erase(i);
		
		i = sessions.begin();
		if (sessions.size() > 0)
			i++;
		
	}
}

vector<Session>::iterator SessionManager::getSession(vector<Session>::iterator i)
{
	return i;
}

Session SessionManager::GetSession(vector<Session>::iterator i)
{
	return *i;
}

Session SessionManager::GetSessionByName(User s)
{
	vector<Session>::iterator i;
	for (i = sessions.begin(); i != sessions.end(); i++)
	{
		if (i->GetuserName() == s.GetName())
			return *i;
	}
	return *sessions.end();
}

vector<Session>::iterator SessionManager::GetSessionBySocket(SOCKET s)
{
	vector<Session>::iterator i;
	for (i = sessions.begin(); i != sessions.end(); i++)
	{
		if (i->clientSock == s)
			return i;
	}
	return sessions.end();
}

vector<Session>::iterator SessionManager::GetBeginIndex()
{
	return sessions.begin();
}

vector<Session>::iterator SessionManager::GetEndIndex()
{
	return sessions.end();
}


int SessionManager::OnlineNumber()
{
	return sessions.size();
}