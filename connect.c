/*******************************************************/
/* connect.c: The part of xgospel responsible for      */
/*            connecting to the server and sending and */
/*            receiving IGS commands                   */
/*                                                     */
/* Author: Ton Hospel                                  */
/*         ton@linux.cc.kuleuven.ac.be                 */
/*         (1994, 1995)                                */
/*                                                     */
/* Copyright: GNU copyleft                             */
/*******************************************************/

#ifdef HAVE_TERMNET
# include <termnet.h>
#endif /* HAVE_TERMNET */

#ifdef HAVE_SOCKS
# define connect	Rconnect
# define getsockname	Rgetsockname
# define bind		Rbind
# define accept		Raccept
# define listen		Rlisten
# define select		Rselect
#endif /* HAVE_SOCKS */

/* COMMUNICATION part of the program */
#include "games.h"
#include "messages.h"
#include "reviews.h"
#include "utils.h"
#include "xgospel.h"

#ifdef HAVE_TERM
# include <client.h>
#endif /* HAVE_TERM */

#include <stdio.h>
#include <stdlib.h>
#ifndef   HAVE_NO_STDARG_H
# include <stdarg.h>
#else  /* HAVE_NO_STDARG_H */
# include <varargs.h>
#endif /* HAVE_NO_STDARG_H */
#include <string.h>
#if !STDC_HEADERS && HAVE_MEMORY_H
# include <memory.h>
#endif /* not STDC_HEADERS and HAVE_MEMORY_H */
#include <ctype.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>

#include <except.h>
#include <mymalloc.h>
#include <myxlib.h>

#ifdef index
# undef index
#endif /* index */

#ifdef rindex
# undef rindex
#endif /* rindex */

#include <X11/Xos.h>
#include <X11/StringDefs.h>

#ifndef   XtNstate
# define XtNstate "state"
#endif /* XtNstate */

#define PermStringToQuark       XrmPermStringToQuark
#define StringToQuark           XrmStringToQuark
#define QuarkToString           XrmQuarkToString
#define UniqueQuark             XrmUniqueQuark
typedef XrmQuark Quark;

/* Let's try to keep conflicts with system declarations to a minimum */
extern int _XGetHostname(char *buf, int maxlen);

#ifndef HAVE_NO_CUSERID
extern char *cuserid(/* char *Buffer */);
# define USERID()	cuserid(NULL)
#else
# ifndef HAVE_NO_GETLOGIN
extern char *getlogin(/* void */);
#  define USERID()	getlogin()
# else
#  define USERID()	"????????"
# endif
#endif

#ifndef _POSIX_SOURCE
extern int read( /* int fd,       char *buf, unsigned int n */);
extern int write(/* int fd, const char *buf, unsigned int n */);
#endif /* _POSIX_SOURCE */
extern int close(/* int fd */);

#ifdef    HAVE_NO_MEMMOVE
void bcopy(/* char *source, char *target, int n */);
# define memmove(to, from, n)   bcopy(from, to, n)
#endif /* HAVE_NO_MEMMOVE */

#ifdef    HAVE_NO_MEMCHR_PROTO
extern void *memchr(const void *s, int c, size_t n);
#endif /* HAVE_NO_MEMCHR_PROTO */

/* Constants for the calculation of average roundtrip time */
#define MIX                        20
#define RESOLUTION                100
#define INITIALAVGROUNDTRIP        10
#define MOVEWAIT                    2

#define USER    1
#define GAMES   2
#define FIRST   4
#define FORCE   8
#define SHARE  16
#define MOVE   32
#define LAST   64

#define BUFSIZE 512

typedef struct commands
{
    struct commands *Next, *Previous;
    char            *Command;
    XtPointer        Closure;
    int              Flags;
} CommandEntry;

struct _Connection {
    Connection    Next, Previous;
    Quark         Site;
    Widget        IsConnected, WantConnect;
    const char   *Name;                 /* Site to which we connect */
    char         *SiteName;             /* Site on which we arrived */
    int           Port;
    int           Socket;
    char         *Buffer;
    int           Request, killnl, CommandAllowed, TryNum;
    size_t        Sent, Used;
    long          WhoTimeOut, GamesTimeOut, ReviewsTimeOut, TimeOut;
    long          QuitTimeOut, ReconnectTimeOut, InactiveTimeOut;
    long          RoundTripTime, LastRoundTripTime, AvgRoundTripTime;
    CommandEntry *LastCommand;          /* Command pending on server        */
    CommandEntry  CommandBase;          /* List of pending commands         */
    char          Parsing[BUFSIZE+1];   /* Last read buffer (errormessages) */
    char          Line[BUFSIZE];        /* current parse buffer             */
    XtInputId     Id, WriteId, SendId;
};

static struct _Connection Connections;
static Widget  ConnectWidget, HasConnectWidget, WantConnectWidget, QuitButton;

#define EEA ErrnoExceptionAction
static Exception SockError    = {"Could not create socket",  NULL, EEA };
#undef EEA
static Exception WriteError   = {"Write failed"};
static Exception NoConnection = {"No connection available"};
#define MAX_WRITE_ERRORS 20

extern XtAppContext  app_context;

static void TryCommand(Connection conn);
Connection Conn;
int        RealQuit;

extern int h_errno;
static char NoError[]      = "No error";
static char InvalidError[] = "Unknown error number to strherror()";

#ifdef HAVE_H_ERRLIST
extern char *h_errlist[];
extern int   h_nerr;
#else  /* HAVE_H_ERRLIST */
static const char *h_errlist[] = {
    NoError,
    "Authoritive Answer Host not found",
    "Non-Authoritive Host not found, or SERVERFAIL",
    "Non recoverable errors, FORMERR, REFUSED, NOTIMP",
    "Valid name but no data record of requested type, look for MX record"
};
static int h_nerr = sizeof(h_errlist)/sizeof(*h_errlist);
#endif /* HAVE_H_ERRLIST */

const char *strherror(int nr)
{
    if (0 < nr && nr < h_nerr) return h_errlist[nr];
    else if (nr == 0)          return NoError;
    else                       return InvalidError;
}

const char *strherrno(void)
{
    if (0 < h_errno && h_errno < h_nerr) return h_errlist[h_errno];
    else if (h_errno == 0)               return NoError;
    else                                 return InvalidError;
}

const char *GetHostname(void)
{
    static char     Buffer[80];
    const char    **hosts, *host;
    struct hostent *hp;
    size_t          Length;

    _XGetHostname(Buffer, sizeof(Buffer));
    hp = gethostbyname(Buffer);
    if (hp) {
        Length = strlen(hp->h_name);
        if (!strchr(hp->h_name, '.')) {
            for (hosts = (const char **) hp->h_aliases;
                 (host = *hosts) != NULL; hosts++)
                if (strncmp(host, hp->h_name, Length) == 0 &&
                    host[Length] == '.') {
                    Length = strlen(host);
                    goto found;
                }
        }
        host = hp->h_name;
      found:
        Length++;
        if (Length > sizeof(Buffer)) Length = sizeof(Buffer);
        memcpy(Buffer, host, Length);
    }
    return Buffer;
}

const char *GetUserId(void)
{
    static char Buffer[80];
    const char *ptr;
    size_t      Length;

    ptr = USERID();
    if (!ptr) ptr = "????????";
    Length = strlen(ptr);
    if (Length >= sizeof(Buffer)) Length = sizeof(Buffer)-1;
    memcpy(Buffer, ptr, Length);
    Buffer[Length] = 0;
    return Buffer;
}

void  SiteLogon(Connection conn, const char *Host)
{
    char *Str;

    if (!conn) {
        conn = Connections.Next;
        if (conn == &Connections) Raise(NoConnection);
    }
    if (strcmp(Host, conn->Name) == 0) { /* Site name is often nonsense */
        Str = mystrdup(Host);
        if (conn->SiteName) myfree(conn->SiteName);
        conn->SiteName = Str;
    }
}

const char *LogonSite(Connection conn)
{
    if (!conn) {
        conn = Connections.Next;
        if (conn == &Connections) Raise(NoConnection);
    }
    return conn->SiteName;
}

/* Does not remove pending command,
   removes all commands that start with the given text */
static void RemoveCommands(Connection conn, const char *command)
{
    CommandEntry *Here, *Next;
    size_t        Length;

    Length = strlen(command);
    for (Here = conn->CommandBase.Next; Here != &conn->CommandBase;
         Here = Next) {
        Next = Here->Next;
        if (strncmp(Here->Command, command, Length) == 0) {
            Here->Previous->Next = Here->Next;
            Here->Next->Previous = Here->Previous;
            myfree(Here->Command);
            myfree(Here);
        }
    }
}

/* Does not remove pending command */
void RemoveMoves(Connection conn)
{
    CommandEntry *Here, *Next;

    if (!conn) {
        conn = Connections.Next;
        if (conn == &Connections) Raise(NoConnection);
    }
    for (Here = conn->CommandBase.Next; Here != &conn->CommandBase;
         Here = Next) {
        Next = Here->Next;
        if (Here->Flags & MOVE) {
            Here->Previous->Next = Here->Next;
            Here->Next->Previous = Here->Previous;
            myfree(Here->Command);
            myfree(Here);
        }
    }
}

void UserActive(Connection conn)
{
    if (!conn) {
        conn = Connections.Next;
        if (conn == &Connections) Raise(NoConnection);
    }
    if (conn->InactiveTimeOut >= 0) conn->InactiveTimeOut = 0;
}

static void AutoReconnect(Connection conn)
{
    if (Me && GuestP(Me)) conn->ReconnectTimeOut = 0;
    else                  conn->ReconnectTimeOut = appdata.ReconnectTimeout;
}

static void CloseConnection(Connection conn)
{
    CommandEntry *lastCommand;

    if (DebugFile) {
        fprintf(DebugFile, "Connection with %s %d closed\n",
                conn->Name, conn->Port);
        fflush(DebugFile);
    }
    if (conn->Id) {
        XtRemoveInput(conn->Id);
        conn->Id = 0;
    }
    if (conn->WriteId) {
        XtRemoveInput(conn->WriteId);
        conn->WriteId = 0;
    }
    if (conn->SendId) {
        XtRemoveInput(conn->SendId);
        conn->SendId = 0;
    }
    /* The connection may already be closed after explicit "quit":
     * first close in TryCommand upon first prompt after sending the
     * quit, second close in CleanConnect (xgospel.c).
     */
    if (conn->Socket != -1) {
      close(conn->Socket);
      conn->Socket = -1;
    }
    conn->TimeOut = conn->WhoTimeOut = conn->GamesTimeOut =
        conn->ReviewsTimeOut = conn->QuitTimeOut = 0;
    conn->InactiveTimeOut = -1;
    conn->RoundTripTime = -1;
    lastCommand = conn->LastCommand;
    if (lastCommand) {
        lastCommand->Next     =  conn->CommandBase.Next;
        lastCommand->Previous = &conn->CommandBase;
        lastCommand->Next->Previous = lastCommand->Previous->Next =
            lastCommand;
        conn->LastCommand = NULL;
    }
    RemoveCommands(conn, "quit");
    RemoveCommands(conn, "forward");
    RemoveCommands(conn, "who");
    RemoveCommands(conn, "games");
    RemoveCommands(conn, "review");
    if (conn->IsConnected)
        XtVaSetValues(conn->IsConnected, XtNstate, (XtArgVal) False, NULL);
    Entered       = 0;
    UnobserveGames(conn);
    UnassumePlayers(conn);
    UnReview(conn);
    conn->Request = -1;
    conn->Sent    =  0;
    conn->Used    =  0;

    Outputf("Connection with %s %d closed\n", conn->Name, conn->Port);
    AutoReconnect(conn);
}

static void QuitAll(Widget w, XtPointer clientdata, XtPointer calldata)
{
    Connection conn;
    Boolean    state;

    XtVaGetValues((Widget) clientdata, XtNstate, (XtArgVal) &state, NULL);
    conn = Connections.Next;
    if (conn == &Connections) Raise(NoConnection);
    RealQuit = False != state;
    if (RealQuit)
        if (conn->Socket < 0) {
            conn->Request = -1;
            conn->Sent    =  0;
            conn->Used    =  0;
        } else if (conn->WriteId) /* We were waiting for a connect */
            CloseConnection(conn);
        else {
            UserCommand(conn, "quit");
            conn->QuitTimeOut = appdata.QuitTimeout;
        }
    else {
        conn->QuitTimeOut = 0;
        XtVaSetValues(QuitButton, XtNlabel, "Quit", NULL);
        RemoveCommands(conn, "quit");
    }
}

void ForceQuit(void)
{
    Connection conn;
    conn = Connections.Next;

    if (conn == &Connections) Raise(NoConnection);
    RealQuit = True;

    if (conn->Socket < 0) {
	conn->Request = -1;
	conn->Sent    =  0;
	conn->Used    =  0;
    } else if (conn->WriteId) { /* We were waiting for a connect */
	CloseConnection(conn);
    } else {
	UserCommand(conn, "quit");
	conn->QuitTimeOut = appdata.QuitTimeout;
    }
}

int ConnectedP(Connection conn)
{
    if (!conn) {
        conn = Connections.Next;
        if (conn == &Connections) Raise(NoConnection);
    }
    return conn->Socket >= 0;
}

size_t GetInput(Connection conn, char *buffer, size_t size)
{
    if (!conn) {
        conn = Connections.Next;
        if (conn == &Connections) Raise(NoConnection);
    }
    if (conn->Request >= 0) {
        conn->Request = size;
        conn->Buffer  = buffer;
        while (conn->Request >= 0) XtAppProcessEvent(app_context, XtIMAll);
    }
    conn->Request = 0;
    return conn->Sent;
}

/*
static void IgsOutput(XtPointer ClientData, int *fid, XtInputId *Id)
{
    printf("I could write now, rc = %d\n", write(*Id, "", 0));
    fflush(stdout);
}
*/

static const char   CloseMessage[] = "Connection closed by foreign host.\n";
static const size_t CloseLength    = sizeof(CloseMessage)-1;

static void IgsInput(XtPointer ClientData, int *fid, XtInputId *Id)
{
    char       *From, *Ptr, *End, *Rest;
    int         rc;
    size_t      NrBytes;
    Connection  conn;

    conn = (Connection) ClientData;

    From = conn->Line+conn->Used;
    rc = read(*fid, From, sizeof(conn->Line) - conn->Used);
    if (rc <= 0) {
        if (rc < 0)
	    switch(errno) {
#ifdef ETIMEDOUT
	      case ETIMEDOUT:
#endif /* ETIMEDOUT */
#ifdef ECONNREFUSED
	      case ECONNREFUSED:
#endif /* ECONNREFUSED */
#ifdef EHOSTDOWN
	      case EHOSTDOWN:
#endif /* EHOSTDOWN */
#ifdef EHOSTUNREACH
	      case EHOSTUNREACH:
#endif /* EHOSTUNREACH */
#ifdef ECONNRESET
	      case ECONNRESET:
#endif /* ECONNRESET */
                ServerMessage("Warning: bad read error on connection with "
			      "%.200s %d: %s. Closing connection\n",
			      conn->Name, conn->Port, strerrno());
                CloseConnection(conn);
		break;
	      default:
                ServerMessage("Warning: read error on connection with %.200s %d"
                              ": %s\n", conn->Name, conn->Port, strerrno());
		break;
	    }
        else if (rc == 0) {
            ServerMessage("Connection with %.200s %d closed by foreign host\n",
                          conn->Name, conn->Port);
            CloseConnection(conn);
        }
        return;
    }
    NrBytes = rc;

    if (DebugFile) {
        fwrite(From, sizeof(char), NrBytes, DebugFile);
        fflush(DebugFile);
    }
    if (appdata.WantStdout != False)
        fwrite(From, sizeof(char), NrBytes, stdout);

/* Otherwise we don't get resynced
    if (conn->TimeOut) conn->TimeOut = appdata.ServerTimeout;
*/

    if (conn->killnl) {
        End  = From+NrBytes;
        Ptr  = From;
        while (*Ptr == '\r') if (++Ptr == End) return;
        conn->killnl = 0;
        if (*Ptr == '\n' && ++Ptr == End) return;
        NrBytes = End-Ptr;
        memmove(From, Ptr, NrBytes);
    }
    while ((Ptr = memchr(From, '\r', NrBytes)) != NULL) {
        End  = From+NrBytes;
       *Ptr++ = '\n';
        for (Rest = Ptr; Rest != End; Rest++) if (*Rest != '\r') goto nocr;
        conn->killnl = 1;
        NrBytes = Ptr-From;
        break;
      nocr:
        if (*Rest == '\n') Rest++;
        memmove(Ptr, Rest, (size_t) (End-Rest));
        NrBytes -= Rest-Ptr;
    }

    conn->Used += NrBytes;
    NrBytes = conn->Used;
    if (NrBytes >= CloseLength &&
        !memcmp(conn->Line+NrBytes-CloseLength, CloseMessage, CloseLength))
        if (NrBytes == CloseLength) {
            ServerMessage("External connection to %.200s %d got closed. "
                          "Cutting real connection\n", conn->Name, conn->Port);
            CloseConnection(conn);
            return;
        } else NrBytes -= CloseLength;

    if (NrBytes && conn->Request >= 0) {
        if (NrBytes > conn->Request) conn->Sent = conn->Request;
        else                         conn->Sent = NrBytes;
        memcpy(conn->Buffer,  conn->Line, conn->Sent);
        memcpy(conn->Parsing, conn->Line, conn->Sent);
        conn->Parsing[conn->Sent] = 0;
        conn->Used -= conn->Sent;
        memmove(conn->Line, conn->Line+conn->Sent, conn->Used);
        conn->Request = -1;
    }
}

void InitConnect(Widget topLevel)
{
    Widget  RealQ, Top;
    Boolean state;
    WidgetList Children;
    Cardinal   i, NrChildren;

    Connections.Next  = Connections.Previous = &Connections;
    ConnectWidget     = XtNameToWidget(topLevel, "*main*connect");
    HasConnectWidget  = XtNameToWidget(topLevel, "*main*hasConnect");
    WantConnectWidget = XtNameToWidget(topLevel, "*main*wantConnect");
    QuitButton = XtNameToWidget(topLevel, "*main*quit");
    if (QuitButton) {
        RealQ = 0;
        Top = XtNameToWidget(topLevel, "*quitConfirm");
        if (Top) RealQ = XtNameToWidget(Top, "*ok");
        XtVaGetValues(QuitButton, XtNstate, (XtArgVal) &state, NULL);
        if (state != False || !RealQ) {
            XtVaSetValues(QuitButton, XtNstate, (XtArgVal) False, NULL);
            XtAddCallback(QuitButton, XtNcallback, QuitAll,
                          (XtPointer) QuitButton);
        } else {
            XtAddCallback(Top, XtNpopupCallback, CallToggleOn,
                          (XtPointer) QuitButton);
            XtAddCallback(Top, XtNpopdownCallback, CallToggleOff,
                          (XtPointer) QuitButton);
            XtAddCallback(QuitButton, XtNcallback, CallToggleUpDown,
                          (XtPointer) Top);
            XtAddCallback(RealQ, XtNcallback, QuitAll, (XtPointer) QuitButton);

            XtRealizeWidget(Top);
            XtVaGetValues(Top, XtNchildren, &Children,
                          XtNnumChildren, &NrChildren, NULL);
            for (i=0; i<NrChildren; i++)
                XtInstallAllAccelerators(Children[i], Children[i]);
            DeleteProtocol(Top);
        }
    }
}

static void CleanConnection(Connection conn)
{
    Outputf("Cleaning up connection %.200s %d\n", conn->Name, conn->Port);
    CloseConnection(conn);

    conn->Previous->Next = conn->Next;
    conn->Next->Previous = conn->Previous;
    if (Conn == conn) Conn = NULL;
    if (conn->SiteName) myfree(conn->SiteName);
    myfree(conn);
}

void CleanConnect(void)
{
    Connection Here, Next;

    for (Here = Connections.Next; Here != &Connections; Here = Next) {
        Next = Here->Next;
        CleanConnection(Here);
    }
}

static void IgsConnect(XtPointer ClientData, int *fid, XtInputId *Id)
{
    Connection conn;

    conn = (Connection) ClientData;
    XtRemoveInput(*Id);
    if (conn->WriteId != *Id)
        Raise1(AssertException, "igs connection from unknown source");
    conn->WriteId = 0;
/*  fcntl(conn->Socket, F_SETFL, 0);
    if (-1 == write(conn->Socket, "", 0)) {
        ServerMessage("Attempt to connect to %.200s %d failed: %s\n",
                      conn->Name, conn->Port, strerrno());
        CleanConnection(conn);
    } else
*/
    Outputf("Connected to %.200s %d\n", conn->Name, conn->Port);
}

void ReConnect(Connection conn)
{
    struct sockaddr_in server;
    struct hostent    *hp;
    unsigned long      addr;
    int                sock, j;
    XtAppContext       context;

    if (!conn) {
        conn = Connections.Next;
        if (conn == &Connections) Raise(NoConnection);
    }
    if (conn->Socket >=0) {
        /* Maybe should make this fatal. It shouldn't happen -Ton */
        ServerMessage("Attempt to reconnect to %.200s %d, but you are "
                      "already connected\n", conn->Name, conn->Port);
        return;
    }
    if (conn->Id) Raise1(AssertException,
                         "attempt to add second connection");
    if (conn->WriteId) Raise1(AssertException,
                              "unexpected pending connect");
    conn->ReconnectTimeOut = 0;
    context = XtWidgetToApplicationContext(toplevel);
#ifdef HAVE_TERM
    if (appdata.UseTerm != False) {
        if ((sock = connect_server(0)) >= 0) {
            send_command(sock, C_PORT, 0, "%s:%d", conn->Name, conn->Port);
            send_command(sock, C_DUMB, 1, 0);
            goto connected;
        } else ServerMessage("Connecting to %.200s %d through term: %s\n",
                             conn->Name, conn->Port, command_result);
    } else
#endif /* HAVE_TERM */
    {
        sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) Raise(SockError);

        Outputf("Attempting to connect to %.200s %d\n",
                conn->Name, conn->Port);

        addr = inet_addr(conn->Name);
        /* We really expect (unsigned long) -1, but alpha returns only part.
           So we should use INADDR_NONE, but often excluded from netinet/in.h
           so just use INADDR_BROADCAST which is the same */

        if (addr == (unsigned long) INADDR_BROADCAST) {
            hp = gethostbyname(conn->Name);
            if (!hp) {
                ServerMessage("Host %.200s not found: %s\n",
                              conn->Name, strherrno());
                goto fail;
            }
            memcpy(&server.sin_addr, hp->h_addr, (size_t) hp->h_length);
            /* Portability trick for alpha (long = 8 bytes)
               Hope it doesn't break on other long=8 machines */
            addr = (unsigned long) server.sin_addr.s_addr;
        }
        memcpy(&server.sin_addr, &addr, sizeof(addr));
        server.sin_family = AF_INET;
        server.sin_port = htons((unsigned short) conn->Port);

#ifndef HAVE_NO_NONBLOCK
        /* Should we turn this of eventually ? -- Ton */
        if (fcntl(sock, F_SETFL, O_NONBLOCK) == -1)
            ServerMessage("Could not execute fcntl on socket for %.200s: "
                          "%s\n", conn->Name, strerrno());
        else
#endif /* HAVE_NO_NONBLOCK */
        {
            j = connect(sock, (struct sockaddr *) &server, sizeof(server));
            if (j >= 0 || errno == EINPROGRESS) {
                if (j < 0) conn->WriteId =
                    XtAppAddInput(context, sock, (XtPointer) XtInputWriteMask,
                                  IgsConnect, (XtPointer) conn);
                else {
#ifdef HAVE_TERM
                  connected:
#endif /* HAVE_TERM */
                    Outputf("Connected to %.200s %d\n",
                            conn->Name, conn->Port);
                }
                conn->Id =
                    XtAppAddInput(context, sock, (XtPointer) XtInputReadMask,
                                  IgsInput, (XtPointer) conn);
                if (conn->IsConnected)
                    XtVaSetValues(conn->IsConnected,
                                  XtNstate, (XtArgVal) True, NULL);
                conn->Socket      = sock;
                conn->QuitTimeOut = conn->InactiveTimeOut = 0;
                return;
            } else ServerMessage("Connecting to %.200s %d: %s\n",
                                 conn->Name, conn->Port, strerrno());
        }
    }
  fail:
    close(sock);
    AutoReconnect(conn);
    if (conn->IsConnected)
        XtVaSetValues(conn->IsConnected, XtNstate, (XtArgVal) False, NULL);
}

static void CallIsConnect(Widget w, XtPointer clientdata, XtPointer calldata)
{
    Connection conn;

    conn = (Connection) clientdata;
    if (False == (Boolean) XTPOINTER_TO_INT(calldata)) CloseConnection(conn);
    else ReConnect(conn);
}

static void CallWantConnect(Widget w, XtPointer clientdata, XtPointer calldata)
{
    Connection conn;

    conn = (Connection) clientdata;
    if (False == (Boolean) XTPOINTER_TO_INT(calldata)) {
        if (conn->Socket >= 0) {
            UserCommand(conn, "quit");
            conn->QuitTimeOut = appdata.QuitTimeout;
        }
        conn->ReconnectTimeOut = 0;
    } else if (conn->Socket < 0) ReConnect(conn);
}

static char *ConnectionNameFun(const char *Pattern, XtPointer Closure)
{
    Connection conn;
    char       Port[80];

    conn = (Connection) Closure;
    sprintf(Port, "%d", conn->Port);
    return StringToFilename(Pattern,
			    (int) 'S', conn->Name,
			    (int) 's', conn->Name,
			    (int) 'P', Port,
			    (int) 'p', Port,
			    NULL);
}

Connection Connect(const char *Site, int Port)
{
    Connection  conn;
    const char *Name;
    Quark       QSite;
    Boolean     Connected;
    Widget      Parent;

    QSite = StringToQuark((char *) Site);
    Name  = QuarkToString(QSite);
    conn  = mynew(struct _Connection);
    WITH_HANDLING {
        conn->Previous    =  Connections.Previous;
        conn->Next        = &Connections;
        conn->Previous->Next = conn->Next->Previous = conn;

        conn->CommandBase.Next = conn->CommandBase.Previous =
            &conn->CommandBase;
        conn->CommandBase.Flags = 0;

        conn->Site        = QSite;
        conn->Name        = Name;
        conn->SiteName    = NULL;
        conn->Port        = Port;
        conn->Id          = 0;
        conn->WriteId     = 0;
        conn->SendId      = 0;
        conn->Socket      = -1;
        conn->Request     = 0;
        conn->IsConnected = HasConnectWidget;
        conn->WantConnect = WantConnectWidget;
        conn->killnl      = 0;
        conn->Used        = 0;
        conn->LastCommand = NULL;
        conn->GamesTimeOut = conn->WhoTimeOut = conn->ReviewsTimeOut =
            conn->QuitTimeOut = conn->ReconnectTimeOut = conn->TimeOut = 0;
        conn->AvgRoundTripTime  = INITIALAVGROUNDTRIP * RESOLUTION;
        conn->LastRoundTripTime = conn->RoundTripTime = -1;
        if (ConnectWidget) {
            SetWidgetProperty(ConnectWidget, XtNlabel, XtCLabel,
                              ConnectionNameFun, (XtPointer) conn);
	    for (Parent = ConnectWidget; Parent; Parent = XtParent(Parent))
		if (XtIsShell(Parent) != False) {
		    XtUnrealizeWidget(Parent);
		    XtRealizeWidget(Parent);
		    break;
		}
        }
        if (conn->IsConnected) {
            XtAddCallback(conn->IsConnected, XtNdestroyCallback,
                          CallDestroyWidgetReference,
                          (XtPointer) &conn->IsConnected);
            XtAddCallback(conn->IsConnected, XtNcallback,
                          CallIsConnect, (XtPointer) conn);
            XtVaSetValues(conn->IsConnected, XtNstate, (XtArgVal) False, NULL);
        }
        if (conn->WantConnect) {
            XtAddCallback(conn->WantConnect, XtNdestroyCallback,
                          CallDestroyWidgetReference,
                          (XtPointer) &conn->WantConnect);
            XtAddCallback(conn->WantConnect, XtNcallback,
                          CallWantConnect, (XtPointer) conn);
            XtVaGetValues(conn->WantConnect, XtNstate,
                          (XtArgVal) &Connected, NULL);
            if (Connected != False) ReConnect(conn);
        } else ReConnect(conn);
    } ON_EXCEPTION {
        CleanConnection(conn);
    } END_HANDLING;
    return conn;
}

const char *Parsing(Connection conn)
{
    if (!conn) {
        conn = Connections.Next;
        if (conn == &Connections) Raise(NoConnection);
    }
    return conn->Parsing;
}

void SetCommand(Connection conn, int value)
{
    if (!conn) {
        conn = Connections.Next;
        if (conn == &Connections) Raise(NoConnection);
    }

    if (DebugPending) {
        if (DebugFile) {
            fprintf(DebugFile, "Set from %d->%d (SetCommand)\n",
                    conn->CommandAllowed, value);
            fflush(DebugFile);
        }
        if (appdata.WantStdout != False)
            printf("Set from %d->%d (SetCommand)\n",
                   conn->CommandAllowed, value);
    }
    conn->CommandAllowed = value;
    TryCommand(conn);
}

void ChangeCommand(Connection conn, int value)
{
    if (!conn) {
        conn = Connections.Next;
        if (conn == &Connections) Raise(NoConnection);
    }

    if (DebugPending) {
        if (DebugFile) {
            fprintf(DebugFile, "Change from %d->%d (ChangeCommand)\n",
                    conn->CommandAllowed, conn->CommandAllowed+value);
            fflush(DebugFile);
        }
        if (appdata.WantStdout != False)
            printf("Change from %d->%d (ChangeCommand) \n",
                   conn->CommandAllowed, conn->CommandAllowed+value);
    }
    conn->CommandAllowed += value;
}

void ResyncCommand(Connection conn)
{
    if (!conn) {
        conn = Connections.Next;
        if (conn == &Connections) Raise(NoConnection);
    }
    TryCommand(conn);
/*
    if (!conn->LastCommand && CommandAllowed <=0)
        Warning("Nothing allowed and nothing pending\n");
*/
}

static void SendWorkProc(XtPointer ClientData, int *fid, XtInputId *Id)
{
    Connection    conn;
    CommandEntry *command, *move;
    int           rc, err;
    size_t        length;
    char          Buffer[1], *Send;
    static        int writeErrors = 0;

    conn = (Connection) ClientData;
    if (conn->SendId != *Id)
        Raise1(AssertException, "unexpected write available");

    command = conn->LastCommand;
    Send    = command->Command;
    if (conn->TryNum) {
        Outputf("----Server not responding to '%.200s'. Trying to resync "
                "(attempt %d)----\n", Send, conn->TryNum);
        Buffer[0] = 0;
        Send = Buffer;
        for (move = conn->CommandBase.Next;
             move != &conn->CommandBase;
             move = move->Next)
            if (move->Flags & MOVE) {
                Send = move->Command;
                break;
            }
    }
    length = strlen(Send);

    Send[length] = '\n';
    rc = write(conn->Socket, Send, length+1);
    err = errno;
    Send[length] = 0;

    if (DebugFile) {
        if (conn->TryNum) fprintf(DebugFile, "* ( ) %s\n", Send);
        else fprintf(DebugFile, "* (%d) %s\n",
                     conn->CommandAllowed, Send);
        fflush(DebugFile);
    }
    if (appdata.WantStdout != False) printf("> %s\n", Send);

    if (length+1 != rc)
        if (rc < 0 && ++writeErrors < MAX_WRITE_ERRORS) {
            ServerMessage("Warning: write error on connection with %.200s %d: "
                          "%s\n", conn->Name, conn->Port, strerror(err));
            return;
        } else Raise(WriteError);

    if (!conn->TryNum) {
        conn->CommandAllowed--;
        if (DebugPending) {
            if (DebugFile) {
                fprintf(DebugFile, "Change from %d->%d (SendWorkProc)\n",
                        conn->CommandAllowed+1, conn->CommandAllowed);
                fflush(DebugFile);
            }
            if (appdata.WantStdout != False)
                printf("Change from %d->%d (SendWorkProc)\n",
                       conn->CommandAllowed+1, conn->CommandAllowed);
        }
        conn->RoundTripTime = 0;
        if (Send[0] == '%') {
            Send++;
            while (*Send && *Send != ' ') Send++;
            while (*Send == ' ') Send++;
        }
        if (command->Flags & USER) {
            /* Kludge to stop bell/raise at userinput --Ton */
            int OldEntered;

            OldEntered = Entered;
            Entered = 0;
            Outputf("%s\n", Send);
            Entered = OldEntered;
            UserActive(conn);
        }

        if      (0 == strcmp(Send, "who"))
            conn->WhoTimeOut = appdata.WhoTimeout;
        else if (0 == strcmp(Send, "games"))
            conn->GamesTimeOut = appdata.GamesTimeout;
        else if (0 == strcmp(Send, "review"))
            conn->ReviewsTimeOut = appdata.ReviewsTimeout;
    }
    conn->TryNum++;
    XtRemoveInput(*Id);
    conn->SendId = 0;
    conn->TimeOut = appdata.ServerTimeout;
}

void ConnectTime(unsigned long Diff)
{
    Connection conn;

    for (conn = Connections.Next; conn != &Connections; conn = conn->Next) {
        if (conn->TimeOut) {
            conn->TimeOut -= Diff;
            if (conn->TimeOut <= 0) {
                conn->TimeOut = 0;
                if (!conn->TryNum) Raise1(AssertException, "Why timing out if "
                                          "no command is being sent ?");
                if (conn->SendId) Raise1(AssertException, "no send should "
                                         "be pending");
                conn->SendId =
                    XtAppAddInput(XtWidgetToApplicationContext(toplevel),
                                  conn->Socket, (XtPointer) XtInputWriteMask,
                                  SendWorkProc, (XtPointer) conn);
            }
        }
        if (conn->WhoTimeOut) {
            conn->WhoTimeOut -= Diff;
            if (conn->WhoTimeOut <= 0) {
                conn->WhoTimeOut = 0;
                if (!TersePlay(NULL)) SendCommand(conn, NULL, "who");
            }
        }
        if (conn->GamesTimeOut) {
            conn->GamesTimeOut -= Diff;
            if (conn->GamesTimeOut <= 0) {
                conn->GamesTimeOut = 0;
                if (!TersePlay(NULL)) SendCommand(conn, NULL, "games");
            }
        }
        if (conn->ReviewsTimeOut) {
            conn->ReviewsTimeOut -= Diff;
            if (conn->ReviewsTimeOut <= 0) {
                conn->ReviewsTimeOut = 0;
                if (!TersePlay(NULL)) ReviewListWanted(conn, 1);
            }
        }
        if (conn->InactiveTimeOut >= 0 && !MyOpponent(Me)) {
            conn->InactiveTimeOut += Diff;
            if (conn->InactiveTimeOut >= appdata.InactiveTimeout) {
                
                Outputf("Automatic disconnect after %ld seconds of "
                        "inactivity\n", conn->InactiveTimeOut);
                conn->InactiveTimeOut = 0;
                XtVaSetValues(conn->WantConnect, XtNstate, (XtArgVal) False,
                              NULL);
                CallWantConnect(conn->WantConnect, (XtPointer) conn,
                                INT_TO_XTPOINTER(False));
            }
        }
        if (conn->RoundTripTime >= 0) conn->RoundTripTime++;
        if (conn->ReconnectTimeOut) {
            conn->ReconnectTimeOut -= Diff;
            if (conn->ReconnectTimeOut <= 0) {
                conn->ReconnectTimeOut = 0;
                ReConnect(conn);
            }
        }
        if (conn->QuitTimeOut) {
            conn->QuitTimeOut -= Diff;
            if (conn->QuitTimeOut <= 0) {
                conn->QuitTimeOut = 0;
                ServerMessage("Quit does not seem to get through. "
                              "Cutting connection\n");
                CloseConnection(conn);
                conn->ReconnectTimeOut = 0;
            } else if (RealQuit) {
                char Buffer[80];

                sprintf(Buffer, " %2ld ", conn->QuitTimeOut);
                XtVaSetValues(QuitButton, XtNlabel, Buffer, NULL);
            }
        }
    }
}

void  ShowConnectCounters(Connection conn)
{
    if (conn) {
        Outputf("Counters on connection with %.200s %d\n",
                conn->Name, conn->Port);
        Outputf(" Time to next \"who\"     command:  %4ld\n",conn->WhoTimeOut);
        Outputf(" Time to next \"games\"   command:  %4ld\n",
                conn->GamesTimeOut);
        Outputf(" Time to next \"reviews\" command:  %4ld\n",
                conn->ReviewsTimeOut);
        if (conn->InactiveTimeOut < 0)
            Outputf("No timeout to automatic disconnect\n");
        else Outputf(" Time to automatic disconnect:    %4ld\n",
                appdata.InactiveTimeout-conn->InactiveTimeOut);
        Outputf(" Time to next forced send:        %4ld\n",conn->TimeOut);
        Outputf(" Time left before forced quit:    %4ld\n",conn->QuitTimeOut);
        Outputf(" Time left before next reconnect: %4ld\n",
                conn->ReconnectTimeOut);
        Outputf(" Time already passed in the current round trip: %4ld\n",
                conn->RoundTripTime);
        Outputf(" Full time used in the previous round trip:     %4ld\n",
                conn->LastRoundTripTime);
        Outputf(" Moving average of round trip times:            %4ld/%d\n",
                conn->AvgRoundTripTime, RESOLUTION);
    } else
        for (conn = Connections.Next; conn != &Connections; conn = conn->Next)
            ShowConnectCounters(conn);
}

static void TryCommand(Connection conn)
{
    CommandEntry *command;

    if (conn->Socket < 0 || conn->CommandAllowed <= 0 || conn->SendId) return;
    conn->TimeOut = 0;
    command = conn->LastCommand;
    if (command) {
        conn->LastRoundTripTime = conn->RoundTripTime;
        conn->AvgRoundTripTime = ((MIX-1)*conn->AvgRoundTripTime+
                                  RESOLUTION*conn->RoundTripTime)/MIX;
        conn->RoundTripTime = -1;

        if (conn->TryNum != 1) {
            Outputf("----resync succeeded. sending '%.200s' to make sure it "
                    "arrived----\n", conn->LastCommand->Command);
            goto resend;
        }
        conn->LastCommand   = NULL;
        if (command->Flags & USER) IgsPrompt();
        if (strcmp(command->Command, "quit") == 0) {
            /* "quit" sent successfully, avoid verbose IGS output now: */ 
            CloseConnection(conn);
            conn->ReconnectTimeOut = 0;
        }
        myfree(command->Command);
        myfree(command);
    }
    command = conn->CommandBase.Next;
    if (command == &conn->CommandBase) CheckReview(conn);
    else {
        command->Previous->Next = command->Next;
        command->Next->Previous = command->Previous;
        conn->LastCommand = command;
      resend:
        /* Try to recover from faulty counts */
        if (DebugPending) {
            if (DebugFile) {
                fprintf(DebugFile, "Forced from %d->1 (TryCommand)\n",
                        conn->CommandAllowed);
                fflush(DebugFile);
            }
            if (appdata.WantStdout != False)
                printf("Forced from %d->1 (TryCommand)\n",
                       conn->CommandAllowed);
        }
        conn->CommandAllowed = 1;
        conn->TryNum = 0;
        if (conn->SendId) Raise1(AssertException, ", second attempt to send");
	if (conn->Socket == -1) {
	  /* May occur if pending output after sending "quit" */
	  Outputf("Connection closed, cannot send '%.200s'\n",
		  command->Command);
	  if (DebugFile) {
	    fprintf(DebugFile, "Connection closed, cannot send '%.200s'\n",
		    command->Command);
	    fflush(DebugFile);
	  }
	} else {
	  conn->SendId = XtAppAddInput(XtWidgetToApplicationContext(toplevel),
				       conn->Socket,
				       (XtPointer)XtInputWriteMask,
				       SendWorkProc, (XtPointer) conn);
	}
    }
}

static Connection RealSendCommand(Connection conn, XtPointer Closure,
                                  int Flags, const char *Command,
                                  va_list args)
{
    char          Text[2048], *Ptr;
    CommandEntry *command, *move, *lastCommand;

    if (!conn) {
        conn = Connections.Next;
        if (conn == &Connections) Raise(NoConnection);
    }
    lastCommand = conn->LastCommand;

    vsprintf(Text, Command, args);
    Ptr = Text;
#if 0
    /* stopgap measure, can't handle the output of refresh (yet ?) --Ton */
    /* This problem seems fixed -- Jean-loup. */
    if (memcmp(Ptr, "refresh", 7) == 0 && (Ptr[7] == 0 || Ptr[7] == ' ')) {
        Ptr += 2;
        memcpy(Ptr, "moves", 5);
    }
#endif
    if (Flags & SHARE) {
        if (Flags & (FIRST|FORCE)) RemoveCommands(conn, Command);
        if (lastCommand && strcmp(Ptr, lastCommand->Command) == 0) return conn;
        for (command = conn->CommandBase.Next;
             command != &conn->CommandBase;
             command = command->Next)
            if (strcmp(Ptr, command->Command) == 0) return conn;
    }
    command = mynew(CommandEntry);
    command->Closure = Closure;
    command->Flags   = Flags;
    command->Command = mystrdup(Ptr);
    if (Flags & (FIRST|FORCE)) {
        command->Previous = &conn->CommandBase;
        command->Next     =  conn->CommandBase.Next;
    } else if (Flags & MOVE) {
        move = conn->CommandBase.Next;
        while (move->Flags & (FIRST | MOVE)) move = move->Next;
        command->Next     = move;
        command->Previous = move->Previous;
    } else if (Flags & LAST) {
        command->Next     = &conn->CommandBase;
        command->Previous =  conn->CommandBase.Previous;
    } else {
        move = conn->CommandBase.Previous;
        while (move->Flags & LAST) move = move->Previous;
        command->Next     = move->Next;
        command->Previous = move;
    }
    command->Previous->Next = command->Next->Previous = command;
    return conn;
}

#ifndef   HAVE_NO_STDARG_H
void SendCommand(Connection conn, XtPointer Closure, const char *Command, ...)
#else  /* HAVE_NO_STDARG_H */
void SendCommand(Connection conn, XtPointer Closure, va_alist)
va_dcl
#endif /* HAVE_NO_STDARG_H */
{
    va_list args;

#ifndef   HAVE_NO_STDARG_H
    va_start(args, Command);
#else  /* HAVE_NO_STDARG_H */
    const char *Command;

    va_start(args);
    Command = va_arg(args, const char *);
#endif /* HAVE_NO_STDARG_H */
    conn = RealSendCommand(conn, Closure, SHARE, Command, args);
    va_end(args);
    TryCommand(conn);
}
#ifndef   HAVE_NO_STDARG_H
void MoveCommand(Connection conn, const char *Command, ...)
#else  /* HAVE_NO_STDARG_H */
void MoveCommand(Connection conn, va_alist)
va_dcl
#endif /* HAVE_NO_STDARG_H */
{
    va_list args;

#ifndef   HAVE_NO_STDARG_H
    va_start(args, Command);
#else  /* HAVE_NO_STDARG_H */
    const char *Command;

    va_start(args);
    Command = va_arg(args, const char *);
#endif /* HAVE_NO_STDARG_H */
    conn = RealSendCommand(conn, NULL, MOVE, Command, args);
    va_end(args);
    if (conn->TimeOut) conn->TimeOut = MOVEWAIT;
    TryCommand(conn);
}

/* Difference with SendCommand: No Share, implies user active */
#ifndef   HAVE_NO_STDARG_H
void UserSendCommand(Connection conn, XtPointer Closure,
                     const char *Command, ...)
#else  /* HAVE_NO_STDARG_H */
void UserSendCommand(Connection conn, XtPointer Closure, va_alist)
va_dcl
#endif /* HAVE_NO_STDARG_H */
{
    va_list args;

#ifndef   HAVE_NO_STDARG_H
    va_start(args, Command);
#else  /* HAVE_NO_STDARG_H */
    const char *Command;

    va_start(args);
    Command = va_arg(args, const char *);
#endif /* HAVE_NO_STDARG_H */
    conn = RealSendCommand(conn, Closure, 0, Command, args);
    va_end(args);
    UserActive(conn);
    TryCommand(conn);
}

/* Difference with SendCommand: No TryCommand() at end. Done by prompt */
#ifndef   HAVE_NO_STDARG_H
void AutoCommand(Connection conn, const char *Command, ...)
#else  /* HAVE_NO_STDARG_H */
void AutoCommand(Connection conn, va_alist)
va_dcl
#endif /* HAVE_NO_STDARG_H */
{
    va_list args;

#ifndef   HAVE_NO_STDARG_H
    va_start(args, Command);
#else  /* HAVE_NO_STDARG_H */
    const char *Command;

    va_start(args);
    Command = va_arg(args, const char *);
#endif /* HAVE_NO_STDARG_H */
    RealSendCommand(conn, NULL, SHARE, Command, args);
    va_end(args);
}

#ifndef   HAVE_NO_STDARG_H
void LastCommand(Connection conn, const char *Command, ...)
#else  /* HAVE_NO_STDARG_H */
void LastCommand(Connection conn, va_alist)
va_dcl
#endif /* HAVE_NO_STDARG_H */
{
    va_list args;

#ifndef   HAVE_NO_STDARG_H
    va_start(args, Command);
#else  /* HAVE_NO_STDARG_H */
    const char *Command;

    va_start(args);
    Command = va_arg(args, const char *);
#endif /* HAVE_NO_STDARG_H */
    RealSendCommand(conn, NULL, LAST, Command, args);
    va_end(args);
    UserActive(conn);
}

#ifndef   HAVE_NO_STDARG_H
void SharedLastCommand(Connection conn, const char *Command, ...)
#else  /* HAVE_NO_STDARG_H */
void SharedLastCommand(Connection conn, va_alist)
va_dcl
#endif /* HAVE_NO_STDARG_H */
{
    va_list args;

#ifndef   HAVE_NO_STDARG_H
    va_start(args, Command);
#else  /* HAVE_NO_STDARG_H */
    const char *Command;

    va_start(args);
    Command = va_arg(args, const char *);
#endif /* HAVE_NO_STDARG_H */
    RealSendCommand(conn, NULL, LAST | SHARE, Command, args);
    va_end(args);
}

#ifndef   HAVE_NO_STDARG_H
void UserCommand(Connection conn, const char *Command, ...)
#else  /* HAVE_NO_STDARG_H */
void UserCommand(Connection conn, va_alist)
va_dcl
#endif /* HAVE_NO_STDARG_H */
{
    va_list args;

#ifndef   HAVE_NO_STDARG_H
    va_start(args, Command);
#else  /* HAVE_NO_STDARG_H */
    const char *Command;

    va_start(args);
    Command = va_arg(args, const char *);
#endif /* HAVE_NO_STDARG_H */
    conn = RealSendCommand(conn, NULL, USER | SHARE, Command, args);
    va_end(args);
    UserActive(conn);
    TryCommand(conn);
}

#ifndef   HAVE_NO_STDARG_H
void FirstCommand(Connection conn, const char *Command, ...)
#else  /* HAVE_NO_STDARG_H */
void FirstCommand(Connection conn, va_alist)
va_dcl
#endif /* HAVE_NO_STDARG_H */
{
    va_list args;

#ifndef   HAVE_NO_STDARG_H
    va_start(args, Command);
#else  /* HAVE_NO_STDARG_H */
    const char *Command;

    va_start(args);
    Command = va_arg(args, const char *);
#endif /* HAVE_NO_STDARG_H */
    RealSendCommand(conn, NULL, FIRST | SHARE, Command, args);
    va_end(args);
}

#ifndef   HAVE_NO_STDARG_H
void ForceCommand(Connection conn, const char *Command, ...)
#else  /* HAVE_NO_STDARG_H */
void ForceCommand(Connection conn, va_alist)
va_dcl
#endif /* HAVE_NO_STDARG_H */
{
    va_list args;

#ifndef   HAVE_NO_STDARG_H
    va_start(args, Command);
#else  /* HAVE_NO_STDARG_H */
    const char *Command;

    va_start(args);
    Command = va_arg(args, const char *);
#endif /* HAVE_NO_STDARG_H */
    RealSendCommand(conn, NULL, FORCE, Command, args);
    va_end(args);
    SetCommand(conn, 1);
}

int WhatCommand(Connection conn, const char *Command)
{
    size_t        length;
    CommandEntry *lastCommand;

    if (!conn) {
        conn = Connections.Next;
        if (conn == &Connections) Raise(NoConnection);
    }
    lastCommand = conn->LastCommand;
    length = strlen(Command);
    if (!lastCommand || memcmp(lastCommand->Command, Command, length) ||
        lastCommand->Command[length] != ' ') return -1;
    return atoi(lastCommand->Command+length+1);
}

const char *ArgsCommand(Connection conn, const char *Command)
{
    size_t      Length;
    const char *Text;
    CommandEntry *lastCommand;

    if (!conn) {
        conn = Connections.Next;
        if (conn == &Connections) Raise(NoConnection);
    }
    lastCommand = conn->LastCommand;
    if (!lastCommand) return NULL;
    Length = strlen(Command);
    Text = lastCommand->Command;
    if (memcmp(Text, Command, Length)) return NULL;
    Text += Length;
    if (*Text ==  0 ) return "";
    if (*Text != ' ') return NULL;
    return Text+1;
}

const char *StripArgsCommand(Connection conn, const char *Command)
{
    static char   Name[300];
    const char   *Text, *ptr;
    size_t        Length;
    CommandEntry *lastCommand;

    if (!conn) {
        conn = Connections.Next;
        if (conn == &Connections) Raise(NoConnection);
    }
    lastCommand = conn->LastCommand;

    if (!lastCommand) return NULL;
    Length = strlen(Command);
    Text = lastCommand->Command;
    if (memcmp(Text, Command, Length)) return NULL;
    Text += Length;
    if (*Text ==  0 ) return "";
    if (!isspace(*Text)) return NULL;
    Text++;
    while (isspace(*Text)) Text++;
    if (*Text == 0) return "";
    ptr = strchr(Text, 0)-1;
    while (isspace(*ptr)) ptr--;
    Length = ptr-Text+1;
    if (Length >= sizeof(Name)) Length = sizeof(Name)-1;
    memcpy(Name, Text, Length);
    Name[Length] = 0;
    return Name;
}

/* for tell: User could have used say or . -Ton */
const char *StripFirstArgCommand(Connection conn, const char *Command)
{
    static char Name[300];
    const char *Text, *ptr;
    char        ch;
    size_t      Length;
    CommandEntry *lastCommand;

    if (!conn) {
        conn = Connections.Next;
        if (conn == &Connections) Raise(NoConnection);
    }
    lastCommand = conn->LastCommand;

    if (!lastCommand) return NULL;
    Length = strlen(Command);
    Text = lastCommand->Command;
    if (memcmp(Text, Command, Length)) return NULL;
    Text += Length;
    if (*Text ==  0 ) return "";
    if (!isspace(*Text)) return NULL;
    Text++;
    while (isspace(*Text)) Text++;
    if (*Text == 0) return "";
    for (ptr = Text+1; (ch = *ptr) != 0; ptr++) if (isspace(ch)) break;
    Length = ptr-Text+1;
    if (Length >= sizeof(Name)) Length = sizeof(Name)-1;
    memcpy(Name, Text, Length);
    Name[Length] = 0;
    return Name;
}

int UserCommandP(Connection conn)
{
    CommandEntry *lastCommand;

    if (!conn) {
        conn = Connections.Next;
        if (conn == &Connections) Raise(NoConnection);
    }
    lastCommand = conn->LastCommand;
    if (!lastCommand) return 0;
    return (lastCommand->Flags & USER) != 0;
}

XtPointer CommandClosure(Connection conn)
{
    CommandEntry *lastCommand;

    if (!conn) {
        conn = Connections.Next;
        if (conn == &Connections) Raise(NoConnection);
    }
    lastCommand = conn->LastCommand;
    if (!lastCommand) return 0;
    return lastCommand->Closure;
}

void CallSendCommand(Widget w, XtPointer clientdata, XtPointer calldata)
{
    SendCommand(NULL, NULL, (const char *) clientdata);
}
