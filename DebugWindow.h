#pragma once

#include <windows.h>
#include <windowsx.h>
#include <iostream>
#include <sstream>

#include "resource.h"

using namespace std;

class CDebugWindow
{
public:
	CDebugWindow(HWND);
	~CDebugWindow(void);

	void log(const char * LogMsg);
	
	// Static
	static int CALLBACK DebugBoxProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static CDebugWindow * _class;
private:
	HWND hDlg;
};
