#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "IOCP_ZQQServer.h"

#define CMD_PORT 8888
#define FILE_PORT 8889

#define MAX_ACCPPTPOST 8
#define MAX_FILEACCPPTPOST 2

HANDLE g_session_mutex;
extern HANDLE g_fileInformation_mutex;


int main()
{
	//��ȡ�û���Ϣ
	if (OPEN_USER_DB_FAIL == loadUserDB("c:/users.txt"))
	{
		MYERROR("fopen");
		exit(0);
	}

	g_session_mutex = (HANDLE)CreateMutex(NULL, FALSE, NULL);
	if (g_session_mutex == NULL)
	{
		MYERROR("CreateMutex");
		exit(0);
	}
	g_fileInformation_mutex = (HANDLE)CreateMutex(NULL, FALSE, NULL);
	if (g_fileInformation_mutex == NULL)
	{
		MYERROR("CreateMutex");
		exit(0);
	}

	//step1:��ʼ��Windows Socket 2.2��
	WSADATA              wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		MYERROR("WSAStartup");
		exit(0);
	}

	//����IOCP
	HANDLE completionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (NULL == completionPort) {   // ����IO�ں˶���ʧ��
		MYERROR("CreateIOCP");
		exit(0);
	}

	// ȷ���������ĺ�������
	SYSTEM_INFO mySysInfo;
	GetSystemInfo(&mySysInfo);

	int threadCount = mySysInfo.dwNumberOfProcessors * 2;
	HANDLE *threadHandle = (HANDLE *)GlobalAlloc(GPTR, sizeof(HANDLE) * threadCount);

	// ���ڴ������ĺ������������߳�
	for (int i = 0; i < threadCount; ++i)
	{
		// �����������������̣߳�������ɶ˿ڴ��ݵ����߳�
		threadHandle[i] = CreateThread(NULL, 0, ServerWorkThread, completionPort, 0, NULL);//��һNULL����Ĭ�ϰ�ȫѡ���һ��0�������߳�ռ����Դ��С���ڶ���0�������̴߳���������ִ��
		if (NULL == threadHandle[i]) {
			MYERROR("CreateThread");
			exit(0);
		}
	}

	//�����׽���
	int ret = CreateServerSocket(NULL, CMD_PORT, FILE_PORT, completionPort);
	if (-1 == ret)
	{
		MYERROR("CreateServerSocket");
		exit(0);
	}

	//Ͷ��AccpetEx
	for (int i = 0;i < MAX_ACCPPTPOST;i++)
	{
		LPPER_IO_DATA lpPerIOData = (LPPER_IO_DATA)GlobalAlloc(GPTR, sizeof(PER_IO_DATA));
		PostAcceptEx(CMD_PORT, lpPerIOData);
	}
	for (int i = 0;i < MAX_FILEACCPPTPOST;i++)
	{
		LPPER_IO_DATA lpPerIOData = (LPPER_IO_DATA)GlobalAlloc(GPTR, sizeof(PER_IO_DATA));
		PostAcceptEx(FILE_PORT, lpPerIOData);
	}
	printf("��ʼ����...\n");
	printf("�ر�������exit\n");

	char str[100];
	while (1)
	{
		fgets(str, 100, stdin);
		if (0 == strcmp(str, "exit\n"))
			break;
	}

	//���̷߳���Ϣ
	for (int i = 0; i < threadCount; ++i)
	{
		PostQueuedCompletionStatus(completionPort, 0, 0, NULL);
	}
	WaitForMultipleObjects(threadCount, threadHandle, 1, INFINITE);
	for (int i = 0; i < threadCount; ++i)
	{
		CloseHandle(threadHandle[i]);
	}

	//step8:�ر��׽���
	closesocket(g_cmdListenSocket);
	closesocket(g_fileListenSocket);
	//�ر���ɶ˿�
	CloseHandle(completionPort);
	//�رջ�����
	CloseHandle(g_session_mutex);
	CloseHandle(g_fileInformation_mutex);
	//step9���ͷ�Windows Socket DLL�������Դ
	WSACleanup();

	return 0;
}

//�����������߳�
DWORD WINAPI ServerWorkThread(LPVOID IpParam)
{
	HANDLE CompletionPort = (HANDLE)IpParam;
	DWORD BytesTransferred;
	Session *session = NULL;
	int port;
	LPPER_IO_DATA lpPerIOData = NULL;
	int ret;

	while (1)
	{
		ret = GetQueuedCompletionStatus(CompletionPort, &BytesTransferred, 
			(PULONG_PTR)&port, (LPOVERLAPPED *)&lpPerIOData, INFINITE);
		if (NULL == lpPerIOData)
		{
			//�յ����̵߳��˳���־
			break;
		}
		if (CMD_PORT == port)
		{
			if (0 == ret)
			{
				MYERROR("GetQueuedCompletionStatus");
				//���ͻ��߽��շ�������ɾ��session
				if (lpPerIOData->operationType != OPERATION_ACCEPT)
				{
					if (WAIT_OBJECT_0 == WaitForSingleObject(g_session_mutex, INFINITE))
					{
						removeSession(lpPerIOData->socket);
						ReleaseMutex(g_session_mutex);
					}
					GlobalFree(lpPerIOData);
					lpPerIOData = NULL;
				}
				//accept���׽��ַ�������ֱ�ӹر��׽���
				else
				{
					closesocket(lpPerIOData->socket);
					//Ͷ����һ��AcceptEx
					PostAcceptEx(CMD_PORT, lpPerIOData);
				}
				continue;
			}

			//����׽��ִ���AcceptEX���������ݣ�
			if (0 == BytesTransferred && lpPerIOData->operationType != OPERATION_ACCEPT)
			{
				if (WAIT_OBJECT_0 == WaitForSingleObject(g_session_mutex, INFINITE))
				{
					removeSession(lpPerIOData->socket);
					ReleaseMutex(g_session_mutex);
				}
				GlobalFree(lpPerIOData);
				lpPerIOData = NULL;
				continue;
			}

			//��������
			if (OPERATION_ACCEPT == lpPerIOData->operationType)
			{
				Session *accSession;
				SOCKADDR_IN *clientAddr = NULL;
				SOCKADDR_IN *localAddr = NULL;
				memset(&clientAddr, 0, sizeof(clientAddr));
				int clientAddrLen = sizeof(clientAddr);
				int localAddrLen = sizeof(localAddr);


				//��ʾ��ַ��Ϣ
				lpfnGetAcceptExSockAddrs(lpPerIOData->databuff.buf, 0,
					sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16,
					(sockaddr **)&localAddr, &localAddrLen, (sockaddr **)&clientAddr, &clientAddrLen);
				int clientip = clientAddr->sin_addr.s_addr;
				printf("Client ip : %d.%d.%d.%d", clientip & 255, (clientip >> 8) & 255,
					(clientip >> 16) & 255, (clientip >> 24) & 255);
				printf(" port : %d\n", ntohs(clientAddr->sin_port));

				//�ȴ��¿ͻ��˵�socket
				SOCKET clientSocket = lpPerIOData->socket;
				//Ͷ����һ��AcceptEx
				PostAcceptEx(CMD_PORT, lpPerIOData);

				//��ӵ�session��Ͷ��Recv
				if (WAIT_OBJECT_0 == WaitForSingleObject(g_session_mutex, INFINITE))
				{
					accSession = addSession(clientSocket);
					ReleaseMutex(g_session_mutex);
					if (NULL != accSession)
					{
						if (NULL == CreateIoCompletionPort((HANDLE)(clientSocket), CompletionPort, (ULONG_PTR)CMD_PORT, 0))
						{
							MYERROR("CreateIOCP");
							//accept֮���п����׽��ֻ�ر�
							if (WAIT_OBJECT_0 == WaitForSingleObject(g_session_mutex, INFINITE))
							{
								removeSession(clientSocket);
								ReleaseMutex(g_session_mutex);
							}
							continue;
						}

						LPPER_IO_DATA lpRecvIOData = (LPPER_IO_DATA)GlobalAlloc(GPTR, sizeof(PER_IO_DATA));
						ZeroMemory(&(lpRecvIOData->overlapped), sizeof(OVERLAPPED));
						lpRecvIOData->databuff.len = DATABUFFSIZE;
						ZeroMemory(lpRecvIOData->buffer, sizeof(lpRecvIOData->buffer));
						lpRecvIOData->databuff.buf = lpRecvIOData->buffer;
						lpRecvIOData->operationType = OPERATION_RECV;
						lpRecvIOData->flags = 0;
						lpRecvIOData->socket = clientSocket;

						ret = WSARecv(clientSocket, &(lpRecvIOData->databuff), 1,
							&(lpRecvIOData->bufferLen), &(lpRecvIOData->flags), &(lpRecvIOData->overlapped), NULL);
						if (-1 == ret && GetLastError() != ERROR_IO_PENDING)
						{
							MYERROR("WSARecv");
							if (WAIT_OBJECT_0 == WaitForSingleObject(g_session_mutex, INFINITE))
							{
								removeSession(clientSocket);
								ReleaseMutex(g_session_mutex);
							}
							GlobalFree(lpRecvIOData);
							lpRecvIOData = NULL;
						}
					}
					else
					{
						closesocket(clientSocket);
					}
				}
			}
			//����
			else if (OPERATION_RECV == lpPerIOData->operationType)
			{
				if (WAIT_OBJECT_0 == WaitForSingleObject(g_session_mutex, INFINITE))
				{
					session_queue_Write(lpPerIOData->socket, lpPerIOData->databuff.buf, BytesTransferred);
					processSocketData(lpPerIOData->socket);
					ReleaseMutex(g_session_mutex);
				}

				// Ϊ��һ���ص����ý�����I/O��������
				ZeroMemory(&(lpPerIOData->overlapped), sizeof(OVERLAPPED)); // ����ڴ�
				lpPerIOData->databuff.len = DATABUFFSIZE;
				ZeroMemory(lpPerIOData->buffer, sizeof(lpPerIOData->buffer));
				lpPerIOData->databuff.buf = lpPerIOData->buffer;
				lpPerIOData->operationType = OPERATION_RECV;
				lpPerIOData->flags = 0;

				ret = WSARecv(lpPerIOData->socket, &(lpPerIOData->databuff), 1,
					&(lpPerIOData->bufferLen), &(lpPerIOData->flags), &(lpPerIOData->overlapped), NULL);
				if (-1 == ret&& GetLastError() != ERROR_IO_PENDING)
				{
					MYERROR("WSARecv");
					if (WAIT_OBJECT_0 == WaitForSingleObject(g_session_mutex, INFINITE))
					{
						removeSession(lpPerIOData->socket);
						ReleaseMutex(g_session_mutex);
					}
					GlobalFree(lpPerIOData);
					lpPerIOData = NULL;
				}
			}
			//����
			else if (OPERATION_SEND == lpPerIOData->operationType)
			{
				if (lpPerIOData->bufferLen == BytesTransferred)
				{
					GlobalFree(lpPerIOData);
					lpPerIOData = NULL;
				}
				else if (lpPerIOData->bufferLen > BytesTransferred)
				{
					ZeroMemory(&(lpPerIOData->overlapped), sizeof(OVERLAPPED)); // ����ڴ�
					lpPerIOData->bufferLen -= BytesTransferred;

					ret = WSASend(lpPerIOData->socket, &(lpPerIOData->databuff) + BytesTransferred, 1,
						&(lpPerIOData->bufferLen), lpPerIOData->flags, &(lpPerIOData->overlapped), NULL);

					if (-1 == ret&& GetLastError() != ERROR_IO_PENDING)
					{
						MYERROR("WSASend");
						if (WAIT_OBJECT_0 == WaitForSingleObject(g_session_mutex, INFINITE))
						{
							removeSession(lpPerIOData->socket);
							ReleaseMutex(g_session_mutex);
						}
						GlobalFree(lpPerIOData);
						lpPerIOData = NULL;
					}
				}
			}
		}
		else if (FILE_PORT == port)
		{
			if (0 == ret)//�������
			{
				MYERROR("GetQueuedCompletionStatus");
				if (lpPerIOData->operationType != OPERATION_ACCEPT) //���ͻ��߽��շ������󣬹ر��ļ�
				{
					LPPER_FILE_DATA lpPerFileData = (LPPER_FILE_DATA)lpPerIOData;
					GlobalFree(lpPerFileData);
					lpPerFileData = NULL;
				}
				else //accept���׽��ַ�������ֱ�ӹر��׽���
				{
					closesocket(lpPerIOData->socket);
					//Ͷ����һ��AcceptEx
					PostAcceptEx(FILE_PORT, lpPerIOData);
				}
				continue;
			}
			else
			{
				ret = processDataPort(CompletionPort, BytesTransferred, lpPerIOData);
			}
		}
	}
	return 0;
}

int processDataPort(HANDLE completionPort, DWORD BytesTransferred, LPPER_IO_DATA lpPerIOData)
{
	int ret;
	//����׽��ִ���AcceptEX���������ݣ�
	if (0 == BytesTransferred && lpPerIOData->operationType != OPERATION_ACCEPT)
	{
		LPPER_FILE_DATA lpPerFileData = (LPPER_FILE_DATA)lpPerIOData;
		GlobalFree(lpPerFileData);
		lpPerFileData = NULL;
		return -1;
	}

	if (OPERATION_ACCEPT == lpPerIOData->operationType)
	{
		SOCKADDR_IN *clientAddr = NULL;
		SOCKADDR_IN *localAddr = NULL;
		memset(&clientAddr, 0, sizeof(clientAddr));
		int clientAddrLen = sizeof(clientAddr);
		int localAddrLen = sizeof(localAddr);


		//��ʾ��ַ��Ϣ
		lpfnGetAcceptExSockAddrs(lpPerIOData->databuff.buf, 0,
			sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16,
			(sockaddr **)&localAddr, &localAddrLen, (sockaddr **)&clientAddr, &clientAddrLen);
		int clientip = clientAddr->sin_addr.s_addr;
		printf("file Client ip : %d.%d.%d.%d", clientip & 255, (clientip >> 8) & 255,
		(clientip >> 16) & 255, (clientip >> 24) & 255);
		printf(" port : %d\n", ntohs(clientAddr->sin_port));

		//�ȴ��¿ͻ��˵�socket
		SOCKET clientSocket = lpPerIOData->socket;
		//Ͷ����һ��AcceptEx
		PostAcceptEx(FILE_PORT, lpPerIOData);

		if (NULL == CreateIoCompletionPort((HANDLE)(clientSocket), completionPort, (ULONG_PTR)FILE_PORT, 0))
		{
			MYERROR("CreateIOCP");
			//accept֮���п����׽��ֻ�ر�
			closesocket(clientSocket);
		}

		LPPER_FILE_DATA lpRecvFileData = (LPPER_FILE_DATA)GlobalAlloc(GPTR, sizeof(PER_FILE_DATA));
		ZeroMemory(&(lpRecvFileData->overlapped), sizeof(OVERLAPPED));
		lpRecvFileData->databuff.len = sizeof(FileHeader);
		ZeroMemory(lpRecvFileData->buffer, sizeof(lpRecvFileData->buffer));
		lpRecvFileData->databuff.buf = lpRecvFileData->buffer;
		lpRecvFileData->operationType = OPERATION_RECV;
		lpRecvFileData->flags = 0;
		lpRecvFileData->socket = clientSocket;
		lpRecvFileData->state = STATE_WAITHEAD;
		lpRecvFileData->recvSize = 0;

		ret = WSARecv(clientSocket, &(lpRecvFileData->databuff), 1,
			&(lpRecvFileData->bufferLen), &(lpRecvFileData->flags), &(lpRecvFileData->overlapped), NULL);
		if (-1 == ret && GetLastError() != ERROR_IO_PENDING)
		{
			MYERROR("WSARecv");
			closesocket(clientSocket);
			GlobalFree(lpRecvFileData);
			lpRecvFileData = NULL;
			return -1;
		}

	}
	else if (OPERATION_RECV == lpPerIOData->operationType)
	{
		LPPER_FILE_DATA lpPerFileData = (LPPER_FILE_DATA)lpPerIOData;
		if (STATE_WAITHEAD == lpPerFileData->state)
		{
			lpPerFileData->recvSize += BytesTransferred;
			if (lpPerFileData->recvSize >= sizeof(FileHeader))//�յ��ļ������ͷ��
			{
				FileHeader *fileHeader = (FileHeader *)lpPerFileData->buffer;
				fileHeader->cmd = htonl(fileHeader->cmd);
				lpPerFileData->fileSize = ntohl(fileHeader->fileSize);
				lpPerFileData->recvSize -= sizeof(FileHeader);
				ZeroMemory(lpPerFileData->md5, sizeof(lpPerFileData->md5));
				memcpy(lpPerFileData->md5, fileHeader->md5, sizeof(fileHeader->md5));
				if (-1 == processFileHeader(lpPerFileData->socket, &(lpPerFileData->fsockto), *fileHeader))
				{
					closesocket(lpPerFileData->socket);
					if(lpPerFileData->fsockto != INVALID_SOCKET)//������ն��Ѿ�����  �͹ر�
						closesocket(lpPerFileData->fsockto);
					GlobalFree(lpPerFileData);
					lpPerFileData = NULL;
					return -1;
				}
				else
				{
					if (FILE_RECV == fileHeader->cmd)//���ն�����
					{
						GlobalFree(lpPerFileData);
						lpPerFileData = NULL;
					}
					else//���Ͷ�����
					{
						lpPerFileData->state = STATE_TRANS;
						lpPerFileData->fsockfrom = lpPerFileData->socket;
						ZeroMemory(lpPerFileData->buffer, sizeof(lpPerFileData->buffer));
						lpPerFileData->databuff.buf = lpPerFileData->buffer;
						lpPerFileData->databuff.len = FILEBUFFSIZE;

						ret = WSARecv(lpPerFileData->socket, &(lpPerFileData->databuff), 1,
							&(lpPerFileData->bufferLen), &(lpPerFileData->flags), &(lpPerFileData->overlapped), NULL);
						if (-1 == ret&& GetLastError() != ERROR_IO_PENDING)
						{
							MYERROR("WSARecv");
							closesocket(lpPerFileData->fsockfrom);
							closesocket(lpPerFileData->fsockto);
							GlobalFree(lpPerFileData);
							lpPerFileData = NULL;
							return -1;
						}
					}
				}
			}
			else//ͷ������ȫ
			{
				// Ϊ��һ���ص����ý�����I/O��������
				ZeroMemory(&(lpPerFileData->overlapped), sizeof(OVERLAPPED)); // ����ڴ�
				lpPerFileData->databuff.len = DATABUFFSIZE - lpPerFileData->recvSize;
				lpPerFileData->databuff.buf = lpPerFileData->buffer + lpPerFileData->recvSize;
				lpPerFileData->operationType = OPERATION_RECV;
				lpPerFileData->flags = 0;

				ret = WSARecv(lpPerFileData->socket, &(lpPerFileData->databuff), 1,
					&(lpPerFileData->bufferLen), &(lpPerFileData->flags), &(lpPerFileData->overlapped), NULL);
				if (-1 == ret&& GetLastError() != ERROR_IO_PENDING)
				{
					MYERROR("WSARecv");
					closesocket(lpPerFileData->socket);
					GlobalFree(lpPerFileData);
					lpPerFileData = NULL;
					return -1;
				}
			}
		}
		else if (STATE_TRANS == lpPerFileData->state)
		{
			lpPerFileData->recvSize += BytesTransferred;

			//if (lpPerFileData->recvSize >= lpPerFileData->fileSize)//�ļ�������
			//{
			//	ret = transmitEnd(lpPerFileData);
			//	closesocket(lpPerFileData->socket);
			//	GlobalFree(lpPerFileData);
			//	lpPerFileData = NULL;
			//	return ret;
			//}
			
				// Ϊ��һ���ص����ý�����I/O��������
				ZeroMemory(&(lpPerFileData->overlapped), sizeof(OVERLAPPED)); // ����ڴ�
				lpPerFileData->databuff.len = BytesTransferred;
				//ZeroMemory(lpPerFileData->buffer, sizeof(lpPerFileData->buffer));
				//lpPerFileData->databuff.buf = lpPerFileData->buffer;
				lpPerFileData->operationType = OPERATION_SEND;
				lpPerFileData->flags = 0;
				//�յ�֮�󷢳�ȥ
				ret = WSASend(lpPerFileData->fsockto, &(lpPerFileData->databuff), 1,
					&(lpPerFileData->bufferLen), (lpPerFileData->flags), &(lpPerFileData->overlapped), NULL);
				if (-1 == ret&& GetLastError() != ERROR_IO_PENDING)
				{
					MYERROR("WSARecv");
					closesocket(lpPerFileData->fsockfrom);
					closesocket(lpPerFileData->fsockto);
					GlobalFree(lpPerFileData);
					lpPerFileData = NULL;
					return -1;

				}
		}
	}
	else if (OPERATION_SEND == lpPerIOData->operationType)
	{
		LPPER_FILE_DATA lpPerFileData = (LPPER_FILE_DATA)lpPerIOData;
		if (BytesTransferred == lpPerFileData->databuff.len)
		{
			if (lpPerFileData->recvSize >= lpPerFileData->fileSize)//�ļ�������
			{
				ret = transmitEnd(lpPerFileData);
				closesocket(lpPerFileData->fsockfrom);
				closesocket(lpPerFileData->fsockto);
				GlobalFree(lpPerFileData);
				lpPerFileData = NULL;
				return ret;
			}
			else
			{
				// Ϊ��һ���ص����ý�����I/O��������
				ZeroMemory(&(lpPerFileData->overlapped), sizeof(OVERLAPPED)); // ����ڴ�
				lpPerFileData->databuff.len = DATABUFFSIZE;
				ZeroMemory(lpPerFileData->buffer, sizeof(lpPerFileData->buffer));
				lpPerFileData->databuff.buf = lpPerFileData->buffer;
				lpPerFileData->operationType = OPERATION_RECV;
				lpPerFileData->flags = 0;
				//����ȥ֮�������
				ret = WSARecv(lpPerFileData->fsockfrom, &(lpPerFileData->databuff), 1,
					&(lpPerFileData->bufferLen), &(lpPerFileData->flags), &(lpPerFileData->overlapped), NULL);
				if (-1 == ret&& GetLastError() != ERROR_IO_PENDING)
				{
					MYERROR("WSARecv");
					closesocket(lpPerFileData->fsockfrom);
					closesocket(lpPerFileData->fsockto);
					GlobalFree(lpPerFileData);
					lpPerFileData = NULL;
					return -1;
				}
			}
		}
		else//WSASendû����
		{
			ZeroMemory(&(lpPerFileData->overlapped), sizeof(OVERLAPPED)); // ����ڴ�
			lpPerFileData->databuff.len -= BytesTransferred;
			//ZeroMemory(lpPerFileData->buffer, sizeof(lpPerFileData->buffer));
			lpPerFileData->databuff.buf = lpPerFileData->buffer + BytesTransferred;
			lpPerFileData->operationType = OPERATION_SEND;
			lpPerFileData->flags = 0;

			ret = WSASend(lpPerFileData->fsockto, &(lpPerFileData->databuff), 1,
				&(lpPerFileData->bufferLen), (lpPerFileData->flags), &(lpPerFileData->overlapped), NULL);
			if (-1 == ret&& GetLastError() != ERROR_IO_PENDING)
			{
				MYERROR("WSARecv");
				closesocket(lpPerFileData->fsockfrom);
				closesocket(lpPerFileData->fsockto);
				GlobalFree(lpPerFileData);
				lpPerFileData = NULL;
				return -1;

			}
		}
	}
	return 0;
}

//Ͷ��AcceptEx
//�ɹ�����1��ʧ�ܷ���0
int PostAcceptEx(short int port, LPPER_IO_DATA lpAcceptIOData)
{
	DWORD dwBytes = 0;

	//��ʼ���ϴ��ù��Ľṹ��
	ZeroMemory(&(lpAcceptIOData->overlapped), sizeof(OVERLAPPED));
	lpAcceptIOData->databuff.len = 0;
	ZeroMemory(&(lpAcceptIOData->buffer), sizeof(lpAcceptIOData->buffer));
	lpAcceptIOData->databuff.buf = lpAcceptIOData->buffer;
	lpAcceptIOData->operationType = OPERATION_ACCEPT;

	//�����׽���
	lpAcceptIOData->socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (INVALID_SOCKET == lpAcceptIOData->socket)
	{
		MYERROR("WSASocket");
		printf("");
		return 0;
	}

	if (CMD_PORT == port)
	{
		//��MSDN˵dwRemoteAddressLength����ȵ�ַ���ȶ�16�ֽ�
		if (0 == lpfnAcceptEx(g_cmdListenSocket, lpAcceptIOData->socket, lpAcceptIOData->databuff.buf,
			0, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, &dwBytes, &lpAcceptIOData->overlapped))
		{
			if (WSA_IO_PENDING != WSAGetLastError())
			{
				MYERROR("AcceptEx");
				return 0;
			}
		}
	}
	else if (FILE_PORT == port)
	{
		if (0 == lpfnAcceptEx(g_fileListenSocket, lpAcceptIOData->socket, lpAcceptIOData->databuff.buf,
			0, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, &dwBytes, &lpAcceptIOData->overlapped))
		{
			if (WSA_IO_PENDING != WSAGetLastError())
			{
				MYERROR("AcceptEx");
				return 0;
			}
		}
	}
	return 1;
}

//�������׽��ֳ�ʼ��
SOCKET CreateServerSocket(const char* pszIP, short int cmdPort, short int filePort,  HANDLE completionPort)
{
	// AcceptEx, GetAcceptExSockAddrs, TransmitFile ��GUID�����ڵ�������ָ��
	GUID GuidAcceptEx = WSAID_ACCEPTEX;
	GUID GuidGetAcceptExSockAddrs = WSAID_GETACCEPTEXSOCKADDRS;
	GUID GuidTransmitFile = WSAID_TRANSMITFILE;

	//SOCKET server = socket(AF_INET, SOCK_STREAM, 0);
	g_cmdListenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (g_cmdListenSocket == INVALID_SOCKET)
	{
		return -1;
	}
	g_fileListenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (g_fileListenSocket == INVALID_SOCKET)
	{
		closesocket(g_cmdListenSocket);
		return -1;
	}

	// ��ȡAcceptEx����ָ��
	DWORD dwBytes = 0;
	if (SOCKET_ERROR == WSAIoctl(
		g_cmdListenSocket,
		SIO_GET_EXTENSION_FUNCTION_POINTER,
		&GuidAcceptEx,
		sizeof(GuidAcceptEx),
		&lpfnAcceptEx,
		sizeof(lpfnAcceptEx),
		&dwBytes,
		NULL,
		NULL))
	{
		//MYERROR("WSAIoctl");
		closesocket(g_cmdListenSocket);
		closesocket(g_fileListenSocket);
		return -1;
	}

	//��ȡGetAcceptExSockAddrs����ָ��
	if (SOCKET_ERROR == WSAIoctl(
		g_cmdListenSocket,
		SIO_GET_EXTENSION_FUNCTION_POINTER,
		&GuidGetAcceptExSockAddrs,
		sizeof(GuidGetAcceptExSockAddrs),
		&lpfnGetAcceptExSockAddrs,
		sizeof(lpfnGetAcceptExSockAddrs),
		&dwBytes,
		NULL,
		NULL))
	{
		//MYERROR("WSAIoctl");
		closesocket(g_cmdListenSocket);
		closesocket(g_fileListenSocket);
		return -1;
	}

	// ��ȡTransmitFile����ָ��
	if (SOCKET_ERROR == WSAIoctl(
		g_cmdListenSocket,
		SIO_GET_EXTENSION_FUNCTION_POINTER,
		&GuidTransmitFile,
		sizeof(GuidTransmitFile),
		&lpfnTransmitFile,
		sizeof(lpfnTransmitFile),
		&dwBytes,
		NULL,
		NULL))
	{
		//MYERROR("WSAIoctl");
		closesocket(g_cmdListenSocket);
		closesocket(g_fileListenSocket);
		return -1;
	}

	//�󶨼����׽���
	if(NULL == CreateIoCompletionPort((HANDLE)g_cmdListenSocket, completionPort, (ULONG_PTR)cmdPort, 0))
	{
		//MYERROR("CreateIOCP");
		closesocket(g_cmdListenSocket);
		closesocket(g_fileListenSocket);
		return -1;
	}
	if (NULL == CreateIoCompletionPort((HANDLE)g_fileListenSocket, completionPort, (ULONG_PTR)filePort, 0))
	{
		//MYERROR("CreateIOCP");
		closesocket(g_cmdListenSocket);
		closesocket(g_fileListenSocket);
		return -1;
	}

	//��������ʵ��
	sockaddr_in local;
	memset(&local, 0, sizeof(local));
	local.sin_family = AF_INET;
	local.sin_addr.s_addr = (
		pszIP != NULL ? inet_addr(pszIP) : INADDR_ANY);
	local.sin_port = htons(cmdPort);

	//�������������׽��ֺͱ���ʵ���
	if (bind(g_cmdListenSocket, (struct sockaddr *)&local, sizeof(local)) != 0)
	{
		//MYERROR("bind");
		closesocket(g_cmdListenSocket);
		closesocket(g_fileListenSocket);
		return -1;
	}

	memset(&local, 0, sizeof(local));
	local.sin_family = AF_INET;
	local.sin_addr.s_addr = (
		pszIP != NULL ? inet_addr(pszIP) : INADDR_ANY);
	local.sin_port = htons(filePort);

	//�������������׽��ֺͱ���ʵ���
	if (bind(g_fileListenSocket, (struct sockaddr *)&local, sizeof(local)) != 0)
	{
		//MYERROR("bind");
		closesocket(g_cmdListenSocket);
		closesocket(g_fileListenSocket);
		return -1;
	}

	//listen
	listen(g_cmdListenSocket, 5);
	listen(g_fileListenSocket, 5);
	return 0;
}