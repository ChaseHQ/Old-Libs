#include "IRCHandler.h"

CIRCHandler::CIRCHandler(char * cpNickname, char * cpRealName, char * cpServer, UINT uiPort )
{
	srand((UINT)time(NULL));
	ConnectionSetup(cpNickname,cpRealName,cpServer,uiPort);
}

CIRCHandler::CIRCHandler(void)
{
	srand((UINT)time(NULL));
}

int CIRCHandler::RecieveSetup(IRCH_RECIEVE_SETUP_STRUCT * ptIrss)
{
	sIRSSSetup = *ptIrss;
	if (sIRSSSetup.hHandleToMainWindow == NULL && sIRSSSetup.bUseWindowMessages)
		return IRCH_ERROR;

	bRecSetup = true;

	return IRCH_SUCCESS;
}

int CIRCHandler::ConnectionSetup(char * cpNickname, char * cpRealName, char * cpServer, UINT uiPort )
{
	if (bConSetup)
		return IRCH_ERROR;

	ResetValues();
	
	if ((strlen(cpNickname) > 15) || (strlen(cpRealName) > 255) || (strlen(cpServer) > 255))
		return IRCH_ERROR;

	strcpy_s(cNickname, cpNickname);
	strcpy_s(cRealName, cpRealName);
	strcpy_s(cServer,  cpServer);
	this->uiPort = uiPort;

	
	if (int err = SockSetup() < IRCH_SUCCESS)
		return err;

	bConSetup = true;
	return IRCH_SUCCESS;
}

void CIRCHandler::ResetValues(void)
{
	ZeroMemory (cServer , 256);
	uiPort = 6667;
	ZeroMemory (cNickname, 16);
	ZeroMemory (cRealName, 256);
	bConnected = false;
	bConSetup = false;
	bRecSetup = false;
	bNegSent = false;
	bAutoNegotionComplete = false;
	qRawQueue.SetOptions(true,4086);
	qComplQueue.SetOptions(sIRSSSetup.bUseStackOverflowProtection,sIRSSSetup.StackOverflowProtectionMaxData);
	bHoldingFragments = false;
}

CIRCHandler::~CIRCHandler(void)
{
	closesocket(mainSocket);
	WSACleanup();
}

int CIRCHandler::SockSetup(void)
{
	mainSocket = NULL;

	WSAData wsData;

	if (WSAStartup(MAKEWORD(2,2),&wsData))
		return IRCH_WINSOCKSETUPERROR;

	return IRCH_SUCCESS;
}

int CIRCHandler::ConnectToIRC(void)
{
	if (bConnected)
		return IRCH_CONNECTIONINUSE;
	if (!bConSetup || !bRecSetup)
		return IRCH_ERROR;

	if ((mainSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET)
		return IRCH_INTERNALWINSOCKERROR;

	sockaddr_in clientService;
	ZeroMemory(&clientService, sizeof(clientService));

	hostent* pHost = gethostbyname(cServer);

	if (!pHost)
		return IRCH_HOSTNOTFOUND;

	clientService.sin_family = AF_INET;
	clientService.sin_port = htons(uiPort);
	clientService.sin_addr.S_un.S_addr = inet_addr(inet_ntoa(*(struct in_addr *)*pHost->h_addr_list));

	int iResult = connect(mainSocket, (sockaddr*)&clientService,sizeof(clientService));

	if (iResult == SOCKET_ERROR)
	{
		int iExtError = WSAGetLastError();
		switch(iExtError)
		{
		case WSAEISCONN:
			return IRCH_CONNECTIONINUSE;
			break;
		case WSAEADDRNOTAVAIL:
			return IRCH_HOSTNOTFOUND;
			break;
		case WSAECONNREFUSED:
			return IRCH_CONNECTIONREQREFUSED;
			break;
		case WSAETIMEDOUT:
			return IRCH_CONNECTIONTIMEOUT;
			break;
		case WSAEWOULDBLOCK:
			return IRCH_WOULDBLOCKWAIT;
			break;
		default:
			return IRCH_ERROR;
			break;
		}
	}

	if ((eCallback = WSACreateEvent()) == WSA_INVALID_EVENT)
		return IRCH_ERROR;
	if (WSAEventSelect(mainSocket,eCallback, FD_CONNECT | FD_CLOSE | FD_READ) == SOCKET_ERROR)
		return IRCH_ERROR;
	hThread = CreateThread(NULL,NULL, _ThreadFunc, (LPVOID) this, NULL, &dwThreadID);
	
	bConnected = true;

	return IRCH_SUCCESS;
}

void CIRCHandler::ThreadHandler(void)
{
	DWORD dwEventTrig = 0;
	while (1)
	{
		dwEventTrig = WSAWaitForMultipleEvents(1,&eCallback,false,WSA_INFINITE,false);
		if (dwEventTrig == WSA_WAIT_EVENT_0)
		{
			WSANETWORKEVENTS sEvents;
			WSAEnumNetworkEvents(mainSocket,eCallback, &sEvents);
			if (sEvents.lNetworkEvents & FD_READ)
				ReadIncomingData();
			if (sEvents.lNetworkEvents & FD_CONNECT)
				ReadIncomingConnect();
			if (sEvents.lNetworkEvents & FD_CLOSE)
				ReadIncomingClose();

		}
	}
}

void CIRCHandler::PumpRawData(void)
{
	// If ever winsock class was made off this idea, this function is the heart
	// it takes the Raw Incoming Data using thte CrLf as a delimeter off the raw Stack
	// and whether it comes in completely or not will continue to put it on the
	// Complete Data Stack with full completeness so all you have to do is worry about
	// data on that stack, the raw data stack should never be directly touched by anything
	// but this one function and the Winsock Thread, it's important that it remains that way
	// because it is an Async. write to that data stack, as for the complete data stack
	// do with it as you wish, that is not async data and can be manipulated at will...
	// thank you... - Craig Vella
	bool bEOTNotFound = false;
	bool bMoreDataLeft = false;
	UINT uiEOT = 0;
	IRCH_RAW_DATA rGet;
	IRCH_COMPL_DATA sCom;
	sCom.cData = NULL;
	char * buffer = NULL;
	qRawQueue.PopTailData(rGet);
	// Check to See if this Has EOT character (CrLf)
	for (long lCount = 0; lCount < (long)rGet.uiBytes; ++lCount)
	{
		if (rGet.cData[lCount] == 0x0A)
		{
			// Line Feed Found - This is the End of The Data
			uiEOT = lCount;
			// EOT Is Found we revert this to false
			bEOTNotFound = false;
			// Now Check to see if more data is left
			if (rGet.cData[uiEOT+1] != 0)
				bMoreDataLeft = true;
			// Check to see if we have any fragments
			if (bHoldingFragments)
			{
				// complete this fragment
				// Create Space for String
				sCom.cData = new char[uiEOT + sFragment.uiBytes + 1];
				ZeroMemory(sCom.cData, uiEOT + sFragment.uiBytes + 1);
				// Copy it over
				memcpy(sCom.cData, sFragment.cData, sFragment.uiBytes);
				for (UINT x = 0; x < uiEOT; ++x)
					sCom.cData[sFragment.uiBytes + x] = rGet.cData[x];
				sCom.tTimestamp = sFragment.tTimestamp;
				sCom.uiBytes = uiEOT + sFragment.uiBytes;
				// Push it on the Q
				qComplQueue.PushData(sCom);
				// Check to See if we Are AutoNegotiating Or AutoPonging and if so process packet
				if (sIRSSSetup.bAutoNegotiation || sIRSSSetup.bAutoPongResponse)
					AutoPacketHandler(sCom);
				// Free and Reset Fragment
				free(sFragment.cData);
				ZeroMemory(&sFragment, sizeof(sFragment));
				bHoldingFragments = false;
			}else{// Not Holding Fragments and this contains an EOT, FULL MSG!
				// Create Space for String
				sCom.cData = new char[uiEOT+1];
				ZeroMemory(sCom.cData,uiEOT+1);
				// copy it over
				memcpy(sCom.cData,rGet.cData,uiEOT);
				sCom.uiBytes = uiEOT;
				sCom.tTimestamp = rGet.tTimestamp;
				// push it on complete queue
				qComplQueue.PushData(sCom);
				// Check to See if we Are AutoNegotiating Or AutoPonging and if so process packet
				if (sIRSSSetup.bAutoNegotiation || sIRSSSetup.bAutoPongResponse)
					AutoPacketHandler(sCom);
			} // end of full fragments
			// if there's more data left do this
			if (bMoreDataLeft)
			{
				int iCounter = 0;
				// allocate buffer
				buffer = new char[(rGet.uiBytes - uiEOT)];
				ZeroMemory(buffer,(rGet.uiBytes - uiEOT));
				// copy it over to buffer
				for (UINT x = uiEOT + 1; x < rGet.uiBytes; ++x)
					buffer[iCounter++] = rGet.cData[x];
				// delete old data
				delete rGet.cData;
				// allocate room for new data
				rGet.cData = new char[(rGet.uiBytes - uiEOT)];
				ZeroMemory(rGet.cData, (rGet.uiBytes - uiEOT));
				// copy it over
				memcpy(rGet.cData, buffer, (rGet.uiBytes - (uiEOT + 1)));
				// Reset Bytes
				rGet.uiBytes = rGet.uiBytes - (uiEOT+1);
				lCount = -1; // Toggle Count Reset
				bMoreDataLeft = false; // Set MoreDataLeft Back
				// We go back to Top of loop and Inspect this data again to see if it contains another Message
			} // End of Moredata left
		} else { // else on EOT, EOT Not found!
			bEOTNotFound = true;
		}// end of EOT Check
	} // end of for loop, if there was No EOT Found thru the entire MSG it's Fragmented and bEOTNotFound should be true

	// Fragment Keeping
	if (!bHoldingFragments && bEOTNotFound)
	{
		sFragment.tTimestamp = rGet.tTimestamp;
		sFragment.uiBytes = rGet.uiBytes;
		sFragment.cData = (char*) malloc(rGet.uiBytes);
		memcpy(sFragment.cData, rGet.cData, rGet.uiBytes);
		bHoldingFragments = true;
	}else if(bHoldingFragments && bEOTNotFound){ // else on Holding Fragments
		int iTotalBytes = rGet.uiBytes + sFragment.uiBytes;
		sFragment.cData = (char*) realloc(sFragment.cData,iTotalBytes);
		for (UINT x = 0; x < rGet.uiBytes; ++x)
			sFragment.cData[sFragment.uiBytes + x] = rGet.cData[x];
		sFragment.uiBytes = iTotalBytes;
	} // end of fragments	
}

void CIRCHandler::ProcessLoop(void)
{
	if (qRawQueue.DataInQueue())
		PumpRawData();
}

void CIRCHandler::ReadIncomingData(void)
{
	// Recieve at most Sections of 1024, With our Smart Data Fragmenting We can Set this To anything and all packets will be complete
	IRCH_RAW_DATA rd;
	char cRaw[1024] = {};
	int brec = recv(mainSocket,cRaw,1024,0);
	rd.tTimestamp = time(NULL);
	rd.cData = new char[(brec + 1)];
	rd.uiBytes = brec;
	ZeroMemory(rd.cData, brec + 1);
	memcpy(rd.cData,cRaw,brec);
	qRawQueue.PushData(rd);
	if (sIRSSSetup.bUseWindowMessages && sIRSSSetup.bWindowMessageOnRecieve)
		SendMessage(sIRSSSetup.hHandleToMainWindow, IRCH_WM_RECIEVE, 0,0);
}

void CIRCHandler::ReadIncomingClose(void)
{
	
	closesocket(mainSocket);
	bConnected = false;
	bAutoNegotionComplete = false;
	bNegSent = false;
	PostServerData(" -- Connection Closed -- ");
	if (sIRSSSetup.bUseWindowMessages && sIRSSSetup.bWindowMessageOnDisconnect)
		SendMessage(sIRSSSetup.hHandleToMainWindow, IRCH_WM_DISCONNECT, 0,0);
	ExitThread(0);
}

void CIRCHandler::ReadIncomingConnect(void)
{
	if (sIRSSSetup.bUseWindowMessages && sIRSSSetup.bWindowMessageOnConnect)
		SendMessage(sIRSSSetup.hHandleToMainWindow, IRCH_WM_CONNECTED, 0,0);
}

bool CIRCHandler::GrabData(IRCH_COMPL_DATA& data)
{ 
	if (qComplQueue.DataInQueue()) 
	{
		ZeroMemory(&data,sizeof(IRCH_COMPL_DATA));
		qComplQueue.PopTailData(data);
		return true;
	}
	return false;
}

void CIRCHandler::PostServerData(LPSTR ServerData)
{
	IRCH_COMPL_DATA cSData;
	cSData.cData = new char[strlen(ServerData) + 1];
	ZeroMemory(cSData.cData,strlen(ServerData) + 1);
	cSData.uiBytes = (UINT)strlen(ServerData) + 1;
	memcpy(cSData.cData,ServerData,strlen(ServerData));
	cSData.tTimestamp = time(NULL);
	qComplQueue.PushData(cSData);
}

void CIRCHandler::AutoPacketHandler(IRCH_COMPL_DATA data)
{
	if (!bConnected) return;
	if ((!bNegSent && sIRSSSetup.bAutoNegotiation) && !bAutoNegotionComplete)
	{
		char buffer[300] = {};
		ZeroMemory(buffer,300);
		strcpy(buffer,"NICK ");
		strcat(buffer,cNickname);
		SendRawServerStringInternal(buffer);
		ZeroMemory(buffer,300);
		strcpy(buffer,"USER ");
		strcat(buffer,cNickname);
		strcat(buffer," * 0: ");
		strcat(buffer,cRealName);
		SendRawServerStringInternal(buffer);
		bNegSent = true;
	}
	if ((bNegSent && sIRSSSetup.bAutoNegotiation) && !bAutoNegotionComplete)
	{
		// Fix this damnit
		if (data.uiBytes <= 3) return;
		IRCH_STRUCTURED_MSG sm(data.uiBytes + 1);
		StructureMessage(data,&sm);
		if (strcmp("433",sm.pcCode) == 0)
		{
			// Nickname is no good add a random number to end and try again...
			int irand = rand() % 9;
			char crand[2] = {};
			_itoa(irand,crand,10);
			strcat(cNickname,crand);
			char buffer[300] = {};
			ZeroMemory(buffer,300);
			strcpy(buffer,"NICK ");
			strcat(buffer,cNickname);
			SendRawServerStringInternal(buffer);
			ZeroMemory(buffer,300);
			strcpy(buffer,"USER ");
			strcat(buffer,cNickname);
			strcat(buffer," * 0: ");
			strcat(buffer,cRealName);
			SendRawServerStringInternal(buffer);
		}
		if (strcmp("001",sm.pcCode) == 0)
			bAutoNegotionComplete = true;
	}
	if (sIRSSSetup.bAutoPongResponse)
	{
		if (data.uiBytes <= 5) return;
		char checkbuffer[5] = {};
		memcpy(checkbuffer,data.cData,4);
		if (strcmp(checkbuffer,"PING") == 0)
		{
			data.cData[1] = 'O';
			SendRawServerStringInternal(data.cData);
			data.cData[1] = 'I';
			if (sIRSSSetup.bUseWindowMessages && sIRSSSetup.bWindowMessageOnAutoPong)
				SendMessage(sIRSSSetup.hHandleToMainWindow, IRCH_WM_AUTOPONG, 0,0);
		}
	}
}

void CIRCHandler::SendRawServerStringInternal(LPSTR ServerString)
{
	if (sIRSSSetup.bRecordSentMessagesToStack)
		PostServerData(ServerString);
	SendRawServerString(ServerString);
}

bool CIRCHandler::SendRawServerString(LPSTR ServerString)
{
	if (!bConnected) return false;
	// Check if String already has \r\n tagged on
	if (ServerString[strlen(ServerString) - 1] != 0x0A)
	{
		// append the server string with a \r\n so it completes
		char * buffer = new char[strlen(ServerString) + 2];
		ZeroMemory (buffer, strlen(ServerString) + 2);
		strcpy(buffer,ServerString);
		buffer[strlen(ServerString)] = 0x0A;
		send(mainSocket,buffer,(int)strlen(buffer),0);
		delete buffer;
		return true;
	} else {
		// string is good to go!
		send (mainSocket,ServerString,(int)strlen(ServerString),0);
		return true;
	}

	return true;
}

bool CIRCHandler::StructureMessage(IRCH_COMPL_DATA& data, IRCH_STRUCTURED_MSG * structMsg)
{
	if (structMsg->uiTSize <= data.uiBytes)
		return false;
	if (data.cData[0] != ':')
		return false;
	// Actually is a structurable message ---
	// Copy to new string
	char * str = new char[data.uiBytes + 1];
	ZeroMemory(str, data.uiBytes + 1);
	strcpy(str,data.cData);
	char * tok;
	// Split String into two parts, Info and Message, and copy message over
	tok = strtok(str,":");
	if (tok == NULL) 
	{
		delete str;
		return false;
	}
	char * firstpart = new char[strlen(tok) + 1];
	ZeroMemory(firstpart, strlen(tok) + 1);
	strcpy(firstpart,tok);
	tok = strtok(NULL,":");
	if (tok == NULL) 
	{
		delete str;
		delete firstpart;
		return false;
	}
	strcpy(structMsg->pcMsg, tok);
	// Deal with info
	tok = strtok(firstpart," "); // Sender
	if (tok == NULL) 
	{
		delete str;
		delete firstpart;
		return false;
	}
	strcpy(structMsg->pcSender, tok);
	tok = strtok(NULL," "); // Code
	if (tok == NULL) 
	{
		delete str;
		delete firstpart;
		return false;
	}
	strcpy(structMsg->pcCode, tok);
	tok = strtok(NULL," "); // To
	if (tok == NULL) 
	{
		delete str;
		delete firstpart;
		return false;
	}
	strcpy(structMsg->pcTo, tok);
	// Try to find Host
	char * gethost = new char[strlen(structMsg->pcSender) + 1];
	ZeroMemory(gethost, strlen(structMsg->pcSender) + 1);
	strcpy(gethost,structMsg->pcSender);
	if (strstr(gethost,"!") != NULL)
	{
		tok = strtok(gethost, "!");
		ZeroMemory(structMsg->pcSender, structMsg->uiTSize);
		strcpy(structMsg->pcSender,tok);
		tok = strtok(NULL, "!");
		strcpy(structMsg->pcHost, tok);
	}
	// Done cleanup
	delete gethost;
	delete str;
	delete firstpart;
	return true;
}

IRCH_STRUCTURED_MSG::IRCH_STRUCTURED_MSG(UINT uiSize)
{
	pcSender = new char[uiSize];
	ZeroMemory(pcSender, uiSize);
	pcTo = new char[uiSize];
	ZeroMemory(pcTo, uiSize);
	pcCode = new char[uiSize];
	ZeroMemory(pcCode, uiSize);
	pcMsg = new char[uiSize];
	ZeroMemory(pcMsg, uiSize);
	pcHost = new char[uiSize];
	ZeroMemory(pcHost, uiSize);
	uiTSize = uiSize;
}

IRCH_STRUCTURED_MSG::~IRCH_STRUCTURED_MSG(void) 
{
	delete pcSender;
	delete pcTo;
	delete pcCode;
	delete pcMsg;
	delete pcHost;
}

DWORD WINAPI _ThreadFunc(LPVOID  pvThread)
{
	CIRCHandler * tObj = (CIRCHandler *)pvThread;
	tObj->ThreadHandler();
	return 0;
}
