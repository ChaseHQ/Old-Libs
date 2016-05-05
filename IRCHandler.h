// ------------------------------------------------
// CIRCHandler Class By Craig Vella
// Used to Handle Connections and Communications
// To IRC Servers as well As Message Queues
// 6 - 22 - 06
// ------------------------------------------------

#ifndef __CIRCHANDLER__
#define __CIRCHANDLER__

#define _CRT_SECURE_NO_DEPRECATE

#include "TQueue.h"
#include "IRCHandlerDef.h"
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>

DWORD WINAPI _ThreadFunc(LPVOID  pvThread);

struct IRCH_RECIEVE_SETUP_STRUCT
{
	bool bAutoNegotiation;
	bool bAutoPongResponse;
	bool bUseWindowMessages;
	HWND hHandleToMainWindow;
	bool bWindowMessageOnAutoPong;
	bool bWindowMessageOnDisconnect;
	bool bWindowMessageOnConnect;
	bool bWindowMessageOnRecieve;
	bool bUseStackOverflowProtection;
	bool bRecordSentMessagesToStack;
	unsigned int StackOverflowProtectionMaxData;
};

// I kept the following data structures seperate just so functions
// could only accept one type of data even tho they are complete
// replica's of each other

struct IRCH_RAW_DATA
{
	time_t tTimestamp;
	UINT uiBytes;
	char * cData;
};

struct IRCH_COMPL_DATA
{
	time_t tTimestamp;
	UINT uiBytes;
	char * cData;
};

class IRCH_STRUCTURED_MSG
{
	friend class CIRCHandler;
	private:
	UINT uiTSize;
public:
	IRCH_STRUCTURED_MSG(UINT uiSize);
	~IRCH_STRUCTURED_MSG(void);
	char * pcSender;
	char * pcHost;
	char * pcTo;
	char * pcCode;
	char * pcMsg;
};

class CIRCHandler
{
private:
	SOCKET mainSocket;

	char cServer[256];
	UINT uiPort;

	char cNickname[16];
	char cRealName[256];

	bool bConnected;
	bool bConSetup;
	bool bRecSetup;

	IRCH_RECIEVE_SETUP_STRUCT sIRSSSetup;
	WSAEVENT eCallback;
	HANDLE hThread;
	DWORD dwThreadID;

	bool bHoldingFragments;
	bool bAutoNegotionComplete;
	bool bNegSent;

	IRCH_RAW_DATA sFragment;

	int SockSetup(void);
	void ResetValues(void);
	void ReadIncomingData(void);
	void ReadIncomingClose(void);
	void ReadIncomingConnect(void);
	void PumpRawData(void);
	void AutoPacketHandler(IRCH_COMPL_DATA data);
	void PostServerData(LPSTR ServerData);
	CTQueue<IRCH_RAW_DATA> qRawQueue;
	CTQueue<IRCH_COMPL_DATA> qComplQueue;
	void SendRawServerStringInternal(LPSTR ServerString);
public:
	CIRCHandler(void);
	CIRCHandler(char * cpNickname, char * cpRealName, char * cpServer, UINT uiPort = 6667 );
	virtual ~CIRCHandler(void);

	void ThreadHandler(void);

	int RecieveSetup(IRCH_RECIEVE_SETUP_STRUCT * ptIrss);
	int ConnectionSetup(char * cpNickname, char * cpRealName, char * cpServer, UINT uiPort = 6667 );
	int ConnectToIRC(void);
	bool IsConnected(void) { return bConnected; };
	void ProcessLoop(void);
	bool DataAvailable(void){ return qComplQueue.DataInQueue(); };
	bool GrabData(IRCH_COMPL_DATA& data);
	bool SendRawServerString(LPSTR ServerString);
	bool IsNegotianComplete(void) {return bAutoNegotionComplete;};
	bool StructureMessage(IRCH_COMPL_DATA& data, IRCH_STRUCTURED_MSG * structMsg);
};

#endif