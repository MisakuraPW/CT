
// 大作业.h: 大作业 应用程序的主头文件
//
#pragma once

#ifndef __AFXWIN_H__
	#error "在包含此文件之前包含 'pch.h' 以生成 PCH"
#endif

#include "resource.h"       // 主符号


// C大作业App:
// 有关此类的实现，请参阅 大作业.cpp
//

class C大作业App : public CWinAppEx
{
public:
	C大作业App() noexcept;


// 重写
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

// 实现
	UINT  m_nAppLook;
	BOOL  m_bHiColorIcons;

	virtual void PreLoadState();
	virtual void LoadCustomState();
	virtual void SaveCustomState();

	afx_msg void OnAppAbout();
	afx_msg void OnFileOpen();
	afx_msg void OnDatasetScanTasks();
	DECLARE_MESSAGE_MAP()
};

extern C大作业App theApp;
