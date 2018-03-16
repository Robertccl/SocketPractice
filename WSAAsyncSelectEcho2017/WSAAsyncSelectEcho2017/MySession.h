#ifndef __MYSESSION__H
#define __MYSESSION__H

#include <string>
#include <map>
#include <vector>
#include <set>
#include <queue>
#include <winsock2.h>
#include "Users.h"
#include "Protocol_TCP.h"

using namespace std;

/*
3.16早上debug最后问题发生在session的构造问题上，还有就是长度，必须是sizeof（sockaddr）,取变量的长度是不行的，

*/

class Session
{


public:
	Session();
	Session(sockaddr* addr);
	Session(string name, sockaddr* addr);
	~Session();

	void SetUserName(string name);
	string GetuserName();

	void SetClientAddr(sockaddr* addr);
	sockaddr* GetSockaddr();

	void SetSecretSessionID();
	string GetSecretSessionID();

	void UserLogin();
	void UserLogout();

	void ClearMsg();


	Session operator=(const Session s);

	bool friend operator==(const Session &s1, const Session &s2);

	bool Empty();

public:
	string userName;
	SOCKET clientSock;
	sockaddr clientAddr;
	int clientAddrLen;

	int state;

	int messageState;

	queue<Protocol::msgNode> messagePkt;



private:
	//map<string, string> someAttribute;
	string secretSessionID;
};


class SessionManager
{
private:
	SessionManager();
	~SessionManager();

public:
	static SessionManager& GetInstance()
	{
		static SessionManager theSessionManager;

		return theSessionManager;
	}
	void AddSession(Session s);
	void RemoveSession(Session &s);
	bool FindSession(Session s);

	vector<Session>::iterator getSession(vector<Session>::iterator i);
	Session GetSession(vector<Session>::iterator i);
	vector<Session>::iterator GetBeginIndex();
	vector<Session>::iterator GetEndIndex();

	Session GetSessionByName(User s);

	vector<Session>::iterator GetSessionBySocket(SOCKET s);

	int OnlineNumber();
	

private:
	vector<Session> sessions;
	Session session;
};


#endif
