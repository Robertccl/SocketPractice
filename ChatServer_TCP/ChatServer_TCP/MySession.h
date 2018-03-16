#ifndef __MYSESSION__H
#define __MYSESSION__H

#include <string>
#include <map>
#include <vector>
#include <set>
#include <winsock2.h>
#include "Users.h"

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


	Session operator=(const Session s);

	bool friend operator==(const Session &s1, const Session &s2);

	bool Empty();

public:
	string userName;
	SOCKET clientSock;
	sockaddr clientAddr;
	int clientAddrLen;

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
	void RemoveSession(Session s);
	bool FindSession(Session s);

	Session GetSession(vector<Session>::iterator i);
	vector<Session>::iterator GetBeginIndex();

	Session GetSessionByName(User s);

	int OnlineNumber();
	

private:
	vector<Session> sessions;
	Session session;
};


#endif
