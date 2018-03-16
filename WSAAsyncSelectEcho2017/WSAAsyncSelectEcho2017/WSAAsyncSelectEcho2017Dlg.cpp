// WSAAsyncSelectEcho2017Dlg.cpp : 实现文件
//

#include "stdafx.h"
#include "WSAAsyncSelectEcho2017.h"
#include "WSAAsyncSelectEcho2017Dlg.h"
#include "NetworkTool.h"


#define WM_SOCKET WM_USER+80

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
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


// CWSAAsyncSelectEcho2017Dlg 对话框


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


// CWSAAsyncSelectEcho2017Dlg 消息处理程序
BOOL CWSAAsyncSelectEcho2017Dlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
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

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码

	

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
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

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CWSAAsyncSelectEcho2017Dlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CWSAAsyncSelectEcho2017Dlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CWSAAsyncSelectEcho2017Dlg::OnStartServer()
{
	// TODO: 在此添加控件通知处理程序代码
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
	if(WSAGETSELECTERROR(lParam))    //检查网络错误
	{
		AfxMessageBox("socket错误");
		return -1;
	}
	else
	{	
		//检查网络事件
		switch(WSAGETSELECTEVENT(lParam)){
			case FD_ACCEPT:
				{
					Session s;
					//接收连接
					s.clientSock = accept(wParam, NULL, NULL);
				
					if(INVALID_SOCKET == s.clientSock)
					{
						MessageBox("接受连接失败!");
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
					//接收数据
					char buff[1024];
					memset(buff, 1024, '\0');
					int headerByte = server->Receive_Header_PKG(wParam, buff);
					int len = ((Protocol::Header*)buff)->length;
					int type = ((Protocol::Header*)buff)->type;
					//string temp = buff;
					int recvBytes = server->Receive_PKG(wParam, buff + headerByte, len);
					if (recvBytes <= 0) //出错时关闭
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
