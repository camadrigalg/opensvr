// PhotoMeasurer.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CPhotoMeasurerApp:
// �йش����ʵ�֣������ PhotoMeasurer.cpp
//

class CPhotoMeasurerApp : public CWinApp
{
public:
	CPhotoMeasurerApp();

// ��д
	public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CPhotoMeasurerApp theApp;