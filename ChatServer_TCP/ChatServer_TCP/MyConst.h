#ifndef __MYCONST__H
#define __MYCONST__H

class MyConst
{
public:
	static const int LOGIN_PKT = 0;
	static const int LOGOUT_PKT = 1;
	static const int LOGIN_REPLY_PKT = 2;
	static const int PUBLIC_CHAT_PKT = 3;
	static const int PRIVATE_CHAT_PKT = 4;
	static const int SYSTEM_PKT = 5;


	static const int LOGIN_FAIL = 0;
	static const int LOGIN_SUCCESS = 1;
	static const int USER_HAS = -1;

	static const int LOGOUT_SUCCESS = 2;



	static const int UNFOUND_SESSION = 0;
	static const int REMOVE_SESSION_SUCCESS = 1;

	static const int ERROR_S = 0;

	static const int UNKNOWN_PKT_FORMAT = -1;

	static const int PASSWORD_ERROR = -1;
	static const int USER_UNFOUND = 0;
	static const int USER_FOUND = 1;

	static const int OPEN_USER_DB_FAIL = -1;
};

#endif
