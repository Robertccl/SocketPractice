// WSAAsyncSelectEcho2017Dlg.h : 头文件
//

#pragma once
#include "ChatServer_TCP.h"
#include "Protocol_TCP.h"
#include "MyConst.h"


void processThread(void* lpParam);

// CWSAAsyncSelectEcho2017Dlg 对话框
class CWSAAsyncSelectEcho2017Dlg : public CDialog
{
// 构造
public:
	CWSAAsyncSelectEcho2017Dlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_WSAASYNCSELECTECHO2017_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	LRESULT OnSocket(WPARAM  wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnStartServer();


private:
	SOCKET m_sListen;
	TcpServer *server;
	TcpChatServer *tcpChatServer;


	//SOCKET	sNewConnection;
};



