#include "Users.h"
#include <fstream>

User::User()
{

}

User::~User()
{
	name = "";
	password = "";
	other = "";
}

User::User(string name1, string password1)
{
	name = name1;
	password = password1;

}

User::User(string name1, string password1, string other1)
{
	name = name1;
	password = password1;
	other = other1;
}

void User::SetName(string name1)
{
	name = name1;
}

string User::GetName()
{
	return name;
}

void User::SetPassword(string psw)
{
	password = psw;

}

string User::GetPassword()
{
	return password;
}


User User::operator=(const User &u1)
{
	return User(u1.name, u1.password);
}

bool User::Empty(const User &u)
{
	if (name == u.name && u.password == password)
		return true;
	else
		return false;
}

bool operator==(const User &u1, const User &u2)
{
	if (u1.name == u2.name && u1.password == u2.password)
		return true;
	else
		return false;

}


UserManager::UserManager()
{

}

UserManager::~UserManager()
{
	users.clear();
}

bool UserManager::LoadUserDB(string path)
{
	string name, psw, other;
	if (users.size() > 0)
		users.clear();
	fstream fileIn(path, ios::in);
	if (fileIn.is_open())
	{
		while (!fileIn.eof())
		{
			fileIn >> name >> psw;
			if (name != "" && psw != "")
			{
				User u(name, psw, other);
				users.push_back(u);
				name = "";
				psw = "";
			}
		}
		fileIn.close();
		return true;
	}
	else
		return false;

}

bool UserManager::WriteToDB(string path)
{
	fstream fileIn(path, ios::out|ios::trunc);
	if (fileIn.is_open())
	{
		int i = 0;
		while (i < users.size())
		{
			fileIn << users[i].GetName() << users[i].GetPassword() << endl;
			i++;
		}
		fileIn.close();
		users.clear();
		return true;
	}
	else
		return false;
}


void UserManager::AddUser(User u)
{
	if (!u.Empty(u))
		users.push_back(u);
}

void UserManager::RemoveUser(User u)
{
	vector<User>::iterator i;
	for (i=users.begin(); i != users.end(); i++)
	{
		if (*i == u)
		{
			users.erase(i);
		}
	}
}

bool UserManager::CheckUser(User u)
{
	vector<User>::iterator i;
	for (i = users.begin(); i != users.end(); i++)
	{
		if (*i == u)
		{
			return true;
		}
	}
	return false;
}

void UserManager::TraversalUsers()
{
	for (int i = 0; i < users.size(); i++)
	{
		users[i];
	}
}
