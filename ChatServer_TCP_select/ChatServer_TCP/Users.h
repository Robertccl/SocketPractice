#ifndef __USERS__H
#define __USERS__H

#include <string>
#include <map>
#include <vector>
#include <set>
#include <winsock2.h>

using namespace std;

class User
{
public:
	User();
	~User();

	User(string name1, string password1);
	User(string name1, string password1, string other1);

	void SetName(string name1);
	string GetName();
	void SetPassword(string psw);
	string GetPassword();

	User operator=(const User &u1);
	bool friend operator==(const User &u1, const User &u2);

	bool Empty(const User &u);

private:
	string name;
	string password;
	string other;

};


class UserManager
{
private:
	UserManager();
	~UserManager();

public:
	static UserManager& GetInstance()
	{
		static UserManager theUserManager;
		return theUserManager;
	}
	bool LoadUserDB(string path);
	bool WriteToDB(string path);

	void AddUser(User u);
	void RemoveUser(User u);
	bool CheckUser(User u);
	void TraversalUsers();

private:
	vector<User> users;
	User user();
};


#endif
