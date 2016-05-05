#include "WindowClass.h"

CWindowClass::CWindowClass(void)
{
	bInitialized = false;
	bWindowCreated = false;
	bRun = false;
	uWindowLoopType = WINDOW_LOOPTYPE_GET;
	keyHandler = NULL;
	windowClose = NULL;
}

CWindowClass::~CWindowClass(void)
{
	if (bInitialized)
		UnregisterClass(strClassName.c_str(),_hInstance);
}

bool CWindowClass::initialize(LPSTR appName, HINSTANCE hInstance,DWORD dwClassStyles, WNDPROC windowCallback){
	WNDCLASS wnd = {};
	wnd.hbrBackground = reinterpret_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
	wnd.hCursor = LoadCursor(NULL,IDC_ARROW);
	wnd.hInstance = hInstance;
	wnd.lpfnWndProc = windowCallback;
	wnd.lpszClassName = appName;
	wnd.style = dwClassStyles;

	strClassName = appName;

	_hInstance = hInstance;

	if (!RegisterClass(&wnd))
		return false;

	bInitialized = true;
	return true;
}

bool CWindowClass::createWindow(LPSTR windowTitle, RECT windowRect, DWORD dwWindowStyle){
	if (!bInitialized) return false;

	_hWnd = CreateWindowA(strClassName.c_str(),windowTitle,dwWindowStyle,windowRect.top,windowRect.left,windowRect.right,windowRect.bottom,NULL,NULL,_hInstance,NULL);
	
	if (!_hWnd)
		return false;

	LONG test = SetWindowLongPtr(_hWnd,GWLP_USERDATA,reinterpret_cast<LONG>(this));

	bWindowCreated = true;
	bRun = true;

	return true;
}

void CWindowClass::stepLoop() {
	if (!bWindowCreated) return;

	MSG msg;

	if (uWindowLoopType == WINDOW_LOOPTYPE_GET) {
		if (GetMessage(&msg,NULL,NULL,NULL)){
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		} else {
			bRun = false;
			bWindowCreated = false;
		}
	}
	if (uWindowLoopType == WINDOW_LOOPTYPE_PEEK) {
		if (PeekMessage(&msg,NULL,NULL,NULL, PM_REMOVE)) {
			if (msg.message == WM_QUIT) {
				bRun = false;
				bWindowCreated = false;
			}
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		std::vector<PEEKFUNCTION>::iterator iter = peekFunctionList.begin();
		while (iter != peekFunctionList.end()) {
			PEEKFUNCTION callMe = *iter;
			callMe();
			++iter;
		}
	}
}

void CWindowClass::runLoop() {
	if (!bWindowCreated) return;

	while (bRun) {
		stepLoop();
	}
}

void CWindowClass::showWindow(bool show){
	if (!bWindowCreated) return;
	ShowWindow(_hWnd,show ? SW_SHOW : SW_HIDE);
}

HWND CWindowClass::getWindowHandle() {
	if (!bWindowCreated) return NULL;
	return _hWnd;
}

bool CWindowClass::callKeyHandler(UINT keyCode, bool keyDown) {
	if (!keyHandler) return false;
	keyHandler(keyCode,keyDown);
	return true;
}

void CWindowClass::callWindowClose() {
	if (!windowClose) return;
	windowClose();
}

void CWindowClass::killWindow() {
	PostMessage(_hWnd,WM_CLOSE,NULL,NULL);
}

LRESULT CALLBACK CWindowClass::DefWindowCallBack(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam){
	CWindowClass * refWindow = reinterpret_cast<CWindowClass *> (GetWindowLongPtr(hWnd,GWLP_USERDATA));
	switch (uMsg) {
		case WM_CLOSE:
			refWindow->callWindowClose();
			PostQuitMessage(0);
			break;
		case WM_KEYDOWN:
			if (refWindow->callKeyHandler(static_cast<UINT>(wParam),true))
				return 0;
			break;
		case WM_KEYUP:
			if (refWindow->callKeyHandler(static_cast<UINT>(wParam),false))
				return 0;
			break;
	}
	return DefWindowProc(hWnd,uMsg,wParam,lParam);
}
