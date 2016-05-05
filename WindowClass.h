#ifndef __CWINDOWCLASS__
#define __CWINDOWCLASS__

#include <windows.h>
#include <vector>
#include <iostream>

#define WINDOW_LOOPTYPE_PEEK 0
#define WINDOW_LOOPTYPE_GET  1

typedef void (*PEEKFUNCTION)();
typedef void (*KEYHANDLER)(UINT keyCode, bool keyDown);
typedef void (*ONWINDOW) ();

class CWindowClass
{
public:
	static LRESULT CALLBACK DefWindowCallBack(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	CWindowClass(void);
	~CWindowClass(void);
	bool initialize(LPSTR appName, HINSTANCE hInstance,DWORD dwClassStyles = (CS_HREDRAW|CS_VREDRAW), WNDPROC windowCallback = DefWindowCallBack);
	bool createWindow(LPSTR windowTitle, RECT windowRect, DWORD dwWindowStyle = WS_OVERLAPPEDWINDOW);
	void showWindow(bool show);
	void runLoop(void);
	void stepLoop(void);
	void setLoopMode(UINT loopMode) {uWindowLoopType = loopMode;};
	void setKeyHandler(KEYHANDLER keyHandlerFunction) {keyHandler = keyHandlerFunction;};
	void setWindowClose(ONWINDOW windowCloseFunction) {windowClose = windowCloseFunction;};
	void addPeekFunction(PEEKFUNCTION peekFunction) {peekFunctionList.push_back(peekFunction);};
	HWND getWindowHandle(void);
	bool callKeyHandler(UINT keyCode, bool keyDown);
	void callWindowClose(void);
	bool windowRunning(void) { return bRun; };
	void killWindow(void);
private:
	bool bInitialized;
	bool bWindowCreated;
	bool bRun;
	HINSTANCE _hInstance;
	HWND _hWnd;
	UINT uWindowLoopType;
	KEYHANDLER keyHandler;
	ONWINDOW windowClose;
	std::vector<PEEKFUNCTION> peekFunctionList;
	std::string strClassName;
};

#endif