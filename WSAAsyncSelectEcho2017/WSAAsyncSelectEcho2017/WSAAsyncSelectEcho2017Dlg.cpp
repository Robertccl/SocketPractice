// WSAAsyncSelectEcho2017Dlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "WSAAsyncSelectEcho2017.h"
#include "WSAAsyncSelectEcho2017Dlg.h"
#include "NetworkTool.h"


#define WM_SOCKET WM_USER+80

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// �Ի�������
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CWSAAsyncSelectEcho2017Dlg �Ի���


CWSAAsyncSelectEcho2017Dlg::CWSAAsyncSelectEcho2017Dlg(CWnd* pParent /*=NULL*/)
	: CDialog(CWSAAsyncSelectEcho2017Dlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CWSAAsyncSelectEcho2017Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CWSAAsyncSelectEcho2017Dlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BTN_START_SERVER, &CWSAAsyncSelectEcho2017Dlg::OnStartServer)
	ON_MESSAGE(WM_SOCKET, OnSocket)
END_MESSAGE_MAP()


// CWSAAsyncSelectEcho2017Dlg ��Ϣ�������
BOOL CWSAAsyncSelectEcho2017Dlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������

	

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void CWSAAsyncSelectEcho2017Dlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CWSAAsyncSelectEcho2017Dlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CWSAAsyncSelectEcho2017Dlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CWSAAsyncSelectEcho2017Dlg::OnStartServer()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	//m_sListen = CreateServerSocket(NULL, 8888);
	server = new TcpServer("127.0.0.1", 5150);
	tcpChatServer = new TcpChatServer("127.0.0.1", 5150, server);
	m_sListen = server->GetListen();
	WSAAsyncSelect(m_sListen, this->m_hWnd, WM_SOCKET, FD_ACCEPT | FD_CLOSE);
	
	_beginthread(::processThread, 0, (void*)tcpChatServer);
}

void processThread(void* lpParam)
{
	TcpChatServer* tcpChatServer = (TcpChatServer*)lpParam;
	tcpChatServer->Run();
}


LRESULT CWSAAsyncSelectEcho2017Dlg::OnSocket(WPARAM  wParam, LPARAM lParam)
{
	if(WSAGETSELECTERROR(lParam))    //����������
	{
		AfxMessageBox("socket����");
		return -1;
	}
	else
	{	
		//��������¼�
		switch(WSAGETSELECTEVENT(lParam)){
			case FD_ACCEPT:
				{
					Session s;
					//��������
					s.clientSock = accept(wParam, NULL, NULL);
				
					if(INVALID_SOCKET == s.clientSock)
					{
						MessageBox("��������ʧ��!");
						break;
					}
					WSAAsyncSelect(s.clientSock, this->m_hWnd,
						WM_SOCKET, FD_READ|FD_CLOSE);
					
					s.state = MyConst::STATE_UNLOGIN;
					SessionManager &sessionManager = SessionManager::GetInstance();
					sessionManager.AddSession(s);
					break;
				}
			case FD_READ:
				{
					SessionManager &sessionManager = SessionManager::GetInstance();
					vector<Session>::iterator it = sessionManager.GetSessionBySocket(wParam);
					//��������
					char buff[1024];
					memset(buff, 1024, '\0');
					int headerByte = server->Receive_Header_PKG(wParam, buff);
					int len = ((Protocol::Header*)buff)->length;
					int type = ((Protocol::Header*)buff)->type;
					//string temp = buff;
					int recvBytes = server->Receive_PKG(wParam, buff + headerByte, len);
					if (recvBytes <= 0) //����ʱ�ر�
					{
						//sConnection[i] = INVALID_SOCKET;
						sessionManager.RemoveSession(*it);
					}
					int length = headerByte + recvBytes;
					//string temp = buff;
					char *pkt = new  char[length + 10];
					memset(pkt, '\0', length + 10);

					Protocol::msgNode tempNode;
					tempNode.msgLength = length;
					tempNode.message = pkt;

					memcpy(pkt, buff, len + 20);

					it->messagePkt.push(tempNode);
					if (length == (8 + len))
						it->messageState = MyConst::MESSAGE_GET;
				
					if (it != sessionManager.GetEndIndex())
						it++;
					
					/*char buff[2048];
					int recvBytes = recv(wParam, buff, sizeof(buff), 0);
					if(recvBytes <= 0)
					{
						closesocket(wParam);
						break;
					}
					int sendBytes = send(wParam, buff, recvBytes, 0);*/
					break;
				}
			case FD_CLOSE:
			{
				closesocket(wParam);
				SessionManager &sessionManager = SessionManager::GetInstance();
				vector<Session>::iterator it = sessionManager.GetSessionBySocket(wParam);
				sessionManager.RemoveSession(*it);
				break;
			
			}
			default:
				break;
				
		}
	}
}
