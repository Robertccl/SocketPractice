// WSAAsyncSelectEcho2017.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CWSAAsyncSelectEcho2017App:
// �йش����ʵ�֣������ WSAAsyncSelectEcho2017.cpp
//

class CWSAAsyncSelectEcho2017App : public CWinApp
{
public:
	CWSAAsyncSelectEcho2017App();

// ��д
	public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CWSAAsyncSelectEcho2017App theApp;