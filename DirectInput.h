#ifndef __CDIRECTINPUT__
#define __CDIRECTINPUT__

#include <windows.h>
#include <dinput.h>

class CDirectInput
{
public:
	CDirectInput(void);
	~CDirectInput(void);
	bool initialize(HWND hWnd, HINSTANCE hInstance);
	bool reacquireKeyboard(void);
	bool reacquireMouse(void);
	void unInitialize(void);
	void pollInput(void);
	bool keyCheck(UINT diKey);
	LONG getMouseXChange(void);
	LONG getMouseYChange(void);
	LONG getWheelChange(void);
	bool getMouseLClick(void);
	bool getMouseRClick(void);
	bool getMouseMClick(void);
private:
	HWND _hWnd;
	HINSTANCE _hInstance;
	bool bInitialized;
	bool bKeyboardAcquired;
	bool bMouseAcquired;
	LPDIRECTINPUT8 din;                //DirectInput Interface
	LPDIRECTINPUTDEVICE8 dinKeyboard;  // Keyboard Interface
	LPDIRECTINPUTDEVICE8 dinMouse;     // Mouse Interface
	BYTE keystate[256];
	DIMOUSESTATE mousestate;
};

#endif