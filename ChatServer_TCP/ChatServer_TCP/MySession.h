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
3.16����debug������ⷢ����session�Ĺ��������ϣ����о��ǳ��ȣ�������sizeof��sockaddr��,ȡ�����ĳ����ǲ��еģ�

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
