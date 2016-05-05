// ------------------------------------------------
// CIRCHandler Class By Craig Vella
// Used to Handle Connections and Communications
// To IRC Servers as well As Message Queues
// :IRCHandlerDef.h - This is the Defines File
// 6 - 22 - 06
// ------------------------------------------------

#ifndef __IRCHANDLERDEF__
#define __IRCHANDLERDEF__

#define IRCH_CONNECTIONTIMEOUT -5
#define IRCH_CONNECTIONREQREFUSED -4
#define IRCH_INTERNALWINSOCKERROR -3
#define IRCH_WINSOCKSETUPERROR -2
#define IRCH_ERROR -1
#define IRCH_SUCCESS 0
#define IRCH_CONNECTIONINUSE 1
#define IRCH_HOSTNOTFOUND 2
#define IRCH_WOULDBLOCKWAIT 3

// id like to reserve the whole 100 - series for future expandability
#define IRCH_WM_CONNECTED   (WM_USER + 100)
#define IRCH_WM_DISCONNECT  (WM_USER + 101)
#define IRCH_WM_RECIEVE     (WM_USER + 102)
#define IRCH_WM_AUTOPONG     (WM_USER + 103)


#endif