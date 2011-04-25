// PhotoMeasurer.cpp : ����Ӧ�ó��������Ϊ��
//

#include "stdafx.h"
#include "PhotoMeasurer.h"
#include "PhotoMeasurerDlg.h"

#include "..\Console\CharUtils.h"
#include "..\Console\Console.h"

#include <iostream>
#include <string>
#include <fstream>
#include <stdlib.h>
#include <stdio.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CPhotoMeasurerApp

BEGIN_MESSAGE_MAP(CPhotoMeasurerApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CPhotoMeasurerApp ����

CPhotoMeasurerApp::CPhotoMeasurerApp()
{
	// TODO: �ڴ˴���ӹ�����룬
	// ��������Ҫ�ĳ�ʼ�������� InitInstance ��
}


// Ψһ��һ�� CPhotoMeasurerApp ����

CPhotoMeasurerApp theApp;


// CPhotoMeasurerApp ��ʼ��

BOOL CPhotoMeasurerApp::InitInstance()
{
	// ���һ�������� Windows XP �ϵ�Ӧ�ó����嵥ָ��Ҫ
	// ʹ�� ComCtl32.dll �汾 6 ����߰汾�����ÿ��ӻ���ʽ��
	//����Ҫ InitCommonControlsEx()�����򣬽��޷��������ڡ�
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// ��������Ϊ��������Ҫ��Ӧ�ó�����ʹ�õ�
	// �����ؼ��ࡣ
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

	AfxEnableControlContainer();

	// �������ڴ洢���õ�ע�����
	SetRegistryKey(_T("WHUSGG_Simbaforrest"));

	Console::Instance()->ShowConsole(true);
	::SetConsoleTitle(::MCBS_2_LOCAL("PhotoMeasurer Console"));

	CFileDialog fdlg(TRUE, 0, 0
		, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT
		, _T("�����ļ�(*.*)|*.*||")
		, 0);
	fdlg.m_ofn.lpstrTitle = _T("ѡ��ͼ���ļ�");
	if(fdlg.DoModal()!=IDOK)
		return FALSE;

	CString path = fdlg.GetPathName();

	CPhotoMeasurerDlg dlg(path);
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: �ڴ˴����ô����ʱ�á�ȷ�������ر�
		//  �Ի���Ĵ���
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: �ڴ˷��ô����ʱ�á�ȡ�������ر�
		//  �Ի���Ĵ���
	}

	// ���ڶԻ����ѹرգ����Խ����� FALSE �Ա��˳�Ӧ�ó���
	//  ����������Ӧ�ó������Ϣ�á�
	return FALSE;
}
