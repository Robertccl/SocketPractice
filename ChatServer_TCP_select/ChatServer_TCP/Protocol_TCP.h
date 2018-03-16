#ifndef __CQ_PROTOCOLTCP__H
#define __CQ_PROTOCOLTCP__H

class Protocol
{
public:
	typedef struct node {
		int msgLength;
		char *message;

	}msgNode;
	typedef msgNode* msgPkt;



public:

	typedef struct Header
	{
		int length;
		int type;
	}Header;

	typedef struct LoginPkt
	{
		Header header;
		char   userName[20];
		char   password[20];
	}LoginPkt;

	typedef struct LogoutPkt
	{
		Header header;
		char   userName[20];
	}LogoutPkt;

	typedef struct LoginRelpyPkt
	{
		Header header;
		int		retCode;
	}LoginRelpyPkt;

	typedef struct PublicChatMsg
	{
		Header header;
		char   from[20];
		char   message[512];
	}PublicChatMsg;

	typedef struct PrivateChatMsg
	{
		Header header;
		char   from[20];
		char   to[20];
		int		retCode;
		char   message[512];
	}PrivateChatMsg;

};
#endif //__CQ_PROTOCOLUDP__H
