// WSAAsyncSelectEcho2017Dlg.h : ͷ�ļ�
//

#pragma once
#include "ChatServer_TCP.h"
#include "Protocol_TCP.h"
#include "MyConst.h"


void processThread(void* lpParam);

// CWSAAsyncSelectEcho2017Dlg �Ի���
class CWSAAsyncSelectEcho2017Dlg : public CDialog
{
// ����
public:
	CWSAAsyncSelectEcho2017Dlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_WSAASYNCSELECTECHO2017_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
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



