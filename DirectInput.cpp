#include "DirectInput.h"

CDirectInput::CDirectInput(void)
{
	bInitialized = false;
	dinKeyboard = NULL;
	dinMouse = NULL;
	din = NULL;
	bKeyboardAcquired = false;
	ZeroMemory(keystate,256);
	ZeroMemory(&mousestate,sizeof(DIMOUSESTATE));
	bMouseAcquired = false;
}

CDirectInput::~CDirectInput(void)
{
	unInitialize();
	din->Release(); // Do NOT release till Program Terminates
}

void CDirectInput::unInitialize() {
	if (bKeyboardAcquired) {
		dinKeyboard->Unacquire();
		dinKeyboard = NULL;
		bKeyboardAcquired = false;
	}
	if (bMouseAcquired) {
		dinMouse->Unacquire();
		dinMouse = NULL;
		bMouseAcquired = false;
	}
}

bool CDirectInput::initialize(HWND hWnd, HINSTANCE hInstance) {

	_hInstance = hInstance;
	_hWnd = hWnd;

	// Create Direct Input Device
	if (DirectInput8Create(_hInstance,DIRECTINPUT_VERSION,IID_IDirectInput8,reinterpret_cast<void **>(&din),NULL) == DI_OK)
		bInitialized = true;
	else
		bInitialized = false;

	// Create Keyboard Device
	if (din->CreateDevice(GUID_SysKeyboard,&dinKeyboard,NULL) == DI_OK) {
		dinKeyboard->SetDataFormat(&c_dfDIKeyboard);
		if (dinKeyboard->SetCooperativeLevel(_hWnd, DISCL_NONEXCLUSIVE | DISCL_BACKGROUND) == DI_OK) {
			dinKeyboard->Acquire();
			bKeyboardAcquired = true;
		}
	} else 
		bInitialized = false;

	if (din->CreateDevice(GUID_SysMouse,&dinMouse,NULL) == DI_OK) {
		dinMouse->SetDataFormat(&c_dfDIMouse);
		if (dinMouse->SetCooperativeLevel(_hWnd, DISCL_NONEXCLUSIVE | DISCL_BACKGROUND) == DI_OK) {
			dinMouse->Acquire();
			bMouseAcquired = true;
		}
	} else
		bInitialized = false;

	return bInitialized;
}

bool CDirectInput::reacquireKeyboard(void) {
	if (bKeyboardAcquired) return false;
	
	if (din->CreateDevice(GUID_SysKeyboard,&dinKeyboard,NULL) != DI_OK)
		return false;
	
	dinKeyboard->SetDataFormat(&c_dfDIKeyboard);
	
	if (dinKeyboard->SetCooperativeLevel(_hWnd, DISCL_NONEXCLUSIVE | DISCL_BACKGROUND) != DI_OK)
		return false;

	return true;
}

bool CDirectInput::reacquireMouse(void) {
	if (bMouseAcquired) return false;
	
	if (din->CreateDevice(GUID_SysMouse,&dinMouse,NULL) != DI_OK)
		return false;
	
	dinMouse->SetDataFormat(&c_dfDIMouse);
	
	if (dinMouse->SetCooperativeLevel(_hWnd, DISCL_NONEXCLUSIVE | DISCL_BACKGROUND) != DI_OK)
		return false;

	dinMouse->Acquire();

	return true;
}

void CDirectInput::pollInput() {
	if (!bInitialized) return;

	if (bKeyboardAcquired) {
		dinKeyboard->Acquire();
		dinKeyboard->GetDeviceState(256,reinterpret_cast<LPVOID>(keystate));
	}

	if (bMouseAcquired) {
		dinMouse->Acquire();
		dinMouse->GetDeviceState(sizeof(DIMOUSESTATE), reinterpret_cast<LPVOID>(&mousestate));
	}
}

bool CDirectInput::keyCheck(UINT diKey){
	if (diKey > 255) return false;
	if (keystate[diKey] & 0x80) return true;
	return false;
}

LONG CDirectInput::getMouseXChange(void) {
	return mousestate.lX;
}

LONG CDirectInput::getMouseYChange(void) {
	return mousestate.lY;	
}

LONG CDirectInput::getWheelChange(void) {
	return mousestate.lZ;
}

bool CDirectInput::getMouseLClick(void) {
	return mousestate.rgbButtons[0] & 0x80;
}

bool CDirectInput::getMouseRClick(void) {
	return mousestate.rgbButtons[1] & 0x80;
}

bool CDirectInput::getMouseMClick(void) {
	return mousestate.rgbButtons[2] & 0x80;
}