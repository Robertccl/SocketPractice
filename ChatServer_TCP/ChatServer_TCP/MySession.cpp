#include "MySession.h"

Session::Session()
{
	
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
void SessionManager::RemoveSession(Session s)  
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

vector<Session>::iterator SessionManager::GetBeginIndex()
{
	return sessions.begin();
}


int SessionManager::OnlineNumber()
{
	return sessions.size();
}