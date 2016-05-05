#include "DebugWindow.h"

CDebugWindow * CDebugWindow::_class = NULL;

CDebugWindow::CDebugWindow(HWND hWnd)
{
	if (!_class) {
		hDlg = CreateDialogParam(GetModuleHandle("CVEngine"),MAKEINTRESOURCE(IDD_DEBUG),hWnd,reinterpret_cast<DLGPROC>(DebugBoxProc),reinterpret_cast<LPARAM>(this));
		log("CVEngine Debug Window Instantiated, Copyright Craig Vella 2009");
	}
}

CDebugWindow::~CDebugWindow(void)
{
}

void CDebugWindow::log(const char * LogMsg) {
	SYSTEMTIME st = {};
	stringstream stringBuf;
	char * buffer = NULL;

	GetSystemTime(&st); // get the system time
	
	int bufSize = Edit_GetTextLength(GetDlgItem(hDlg,IDC_DEBUGTEXT)); // Get the Buffer Window Size
	buffer = (char *) malloc((bufSize + 2) * sizeof(char)); // set the Buffer Large Enough to hold the text
	
	Edit_GetText(GetDlgItem(hDlg,IDC_DEBUGTEXT),buffer, bufSize + 1); // get the text

	stringBuf << buffer  << "[" << st.wHour << ":" << st.wMinute << ":"
		<< ":" << st.wSecond << ":" << st.wMilliseconds << "] - " << LogMsg << "\r\n"; // Append The String Buf with the loginfo & systime

	Edit_SetText(GetDlgItem(hDlg,IDC_DEBUGTEXT),stringBuf.str().c_str()); // Set the String in

	Edit_Scroll(GetDlgItem(hDlg,IDC_DEBUGTEXT),Edit_GetLineCount(GetDlgItem(hDlg,IDC_DEBUGTEXT)),0); // Scroll to the last row
	
}

int CALLBACK CDebugWindow::DebugBoxProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	
	switch (uMsg) {
		case WM_INITDIALOG:
			_class = reinterpret_cast<CDebugWindow *>(lParam);
			ShowWindow(hDlg,SW_SHOW);
			break;
	}

	return false;
}