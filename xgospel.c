#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <X11/Xaw/AsciiText.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include <mymalloc.h>
#include <except.h>
#include <myxlib.h>

#include "GoBoard.h"
#include "SmeBell.h"

#include "analyze.h"
#include "broadcast.h"
#include "connect.h"
#include "events.h"
#include "games.h"
#include "gointer.h"
#include "gospel.h"
#include "match.h"
#include "messages.h"
#include "observe.h"
#include "players.h"
#include "reviews.h"
#include "stats.h"
#include "tell.h"
#include "utils.h"
#include "version.h"
#include "xgospel.h"

#define DEBUGFUN              1
#define DEBUGPENDING          2
#define DEBUGABORTONEXCEPTION 4
#define DEBUGROUNDTRIP        8
#define DEBUGBISON        0x100
#define DEBUGFLEX         0x200

#define MIX        100
#define RESOLUTION 10000

/*
#define VERSIONMESSAGE                                                       \
"\nThe latest version of the program should be available for anonymous ftp " \
"on cc1.kuleuven.ac.be in the directory ANONYMOU.200 as the file "           \
"XGOSPEL.TARZ. Just get the file by name, you don't want to do a dir in "    \
"this place (in case you ever wondered why people invented directories, "    \
"DO try dir :-) ). Don't forget to set the transfermode to binary (cc1 is "  \
"an EBCDIC system). Once you have the file, rename it to xgospel.tar.Z and " \
"proceed as usual.\n"                                                        \
"\nIf you don't want this message anymore, set the X resource "              \
"xgospel*version: %s\n"
*/

#define VERSIONMESSAGE                                          \
"\nThe latest version of this program is in "                   \
"http://www.teaser.fr/~jlgailly/go.html\n"                   \
"\nIf you don't want this message anymore, set the X resource " \
"xgospel*version: %s\n"

/* #define SITE    "igs.joyjoy.net" */
#define SITE "210.134.78.91"

/* We measure the roundtrip time (to the X server) every RETRIP seconds */
#define RETRIP    5
#define NEWOUTPUT 3
/*****************************************************************************/

static Widget Info, Input, Stdout, Localtime, Universaltime, Servertime;
static Widget Verbose, ShortHelp, HelpButton, AutoReply;
static Widget ErrorBeep, ErrorRaise, OutputBeep, OutputRaise, Overwrite, Main;
static char  *MainFileName, *AutoReplyMessage;
static String DateFormat;
static unsigned long ClockTime, TripTime, LastTrip, RoundTrip, LastRoundTrip;
static unsigned long OutputTime, CurTime;
static time_t        BaseTime;

static Atom   TripAtom;
static int    DebugRoundTrip;
char         *SgfDateFormat, *CompileTime;
static void IgsCommand(    Widget w,XEvent *event,String *string, Cardinal *n);
static void IgsUserCommand(Widget w,XEvent *event,String *string, Cardinal *n);
static void IgsSendCommand(Widget w,XEvent *event,String *string, Cardinal *n);
static void HelpUp        (Widget w,XEvent *event,String *string, Cardinal *n);
static void HelpDown      (Widget w,XEvent *event,String *string, Cardinal *n);
static void TripMessage(   Widget w,XEvent *event,String *string, Cardinal *n);
static void DoOutput(      Widget w,XEvent *event,String *string, Cardinal *n);

static XtActionsRec actionTable[] = {
    { (String) "igscommand",  IgsCommand     },
    { (String) "usercommand", IgsUserCommand },
    { (String) "sendcommand", IgsSendCommand },
    { (String) "helpup",      HelpUp         },
    { (String) "helpdown",    HelpDown       },
    { (String) "tripmessage", TripMessage    },
    { (String) "output",      DoOutput       },
};

AppData         appdata;
XtAppContext	app_context;

FILE               *DebugFile;
int                 DebugFun, DebugPending;
Widget              toplevel;
static int	    global_argc;
static char       **global_argv;
char               *MyPassword, *MyName, *UserId, *ServerName;
int     	    ServerVersion, ServerSubVersion;
ServerTYPE          ServerType;
int                 SetServerTime, Entered;
Boolean             WantStdout;
struct tm           LocalTime, UniversalTime, ServerTime;

/*****************************************************************************/

extern String fallback_resources[];
extern int    RealQuit;
extern int    IgsYYdebug, IgsYY_flex_debug;

extern void   _IgsRestartParse(void);

void IgsPrompt(void)
{
    Output("> ");
}

static void DebugFunCommand(const char *args)
{
    char *ptr;
    int   value;

    while (*args == ' ') args++;
    value = strtol(args, &ptr, 0);
    if (ptr == args) Outputf("DebugFun remains 0x%02x\n", appdata.DebugFun);
    else {
        Outputf("DebugFun changed from 0x%02x to 0x%02x\n",
                appdata.DebugFun, value);
        appdata.DebugFun = value;
        DebugFun         = (value & DEBUGFUN)       != 0;
        DebugPending     = (value & DEBUGPENDING)   != 0;
        AbortOnException = (value & DEBUGABORTONEXCEPTION) ? -1 : 0;
        DebugRoundTrip   = (value & DEBUGROUNDTRIP) != 0;
        IgsYYdebug       = (value & DEBUGBISON)     != 0;
        IgsYY_flex_debug = (value & DEBUGFLEX)      != 0;
    }
}

static void ShowCounters(const char *args)
{
    Outputf("Current time:                                 %4lu\n",CurTime);
    Outputf("Time for next triptest:                       %4lu\n",TripTime);
    Outputf("Time for next counters update:                %4lu\n",ClockTime);
    Outputf("Timer for how soon to bell/raise main widget: %4lu\n",OutputTime);
    Outputf("Last X server round trip:                     %4lu\n",LastRoundTrip);
    Outputf("Moving average X server round trip:           %4lu.%02lu\n",
            RoundTrip/RESOLUTION,
            (100*RoundTrip)/RESOLUTION-100*(RoundTrip/RESOLUTION));
    ShowConnectCounters(NULL);
}

static void ShowCommands(const char *Options)
{
    Output("Xgospel extended commands:\n");
    Outputf("  showcommands\n");
    Outputf("  dumpgames\n");
    Outputf("  dumpplayers\n");
    Outputf("  dumpreviews\n");
    Outputf("  debugfun\n");
    Outputf("  showcounters\n");
    Outputf("  xgospeltell\n");
}

static void IgsCommand(Widget w, XEvent *event, String *string, Cardinal *n)
{
    String          Buffer;
    int             Prompt;

    XtVaGetValues(w, XtNstring, &Buffer, NULL);
/*  BatchAppendText(Info, Buffer, strlen(Buffer), 0); */
    Prompt = 1;
    while (isspace(*Buffer)) Buffer++;
    if      (!strncmp(Buffer, "xgospelcommands",15)) ShowCommands(Buffer+15);
    else if (!strncmp(Buffer, "dumpgames"   , 9)) DumpGames(Buffer+9);
    else if (!strncmp(Buffer, "dumpplayers" ,11)) DumpPlayers(Buffer+11);
    else if (!strncmp(Buffer, "dumpreviews" ,11)) DumpReviews(Buffer+11);
    else if (!strncmp(Buffer, "debugfun"    , 8)) DebugFunCommand(Buffer+8);
    else if (!strncmp(Buffer, "showcounters",12)) ShowCounters(Buffer+12);
    else if (!strncmp(Buffer, "xgospeltell" ,11))
        if (Buffer[11]) TellXgospelUsers(NULL, Buffer+12);
        else TellXgospelUsers(NULL, "");
    else {
        Prompt = 0;
        if      (!strncmp(Buffer, "review", 6))
            ReviewWanted(NULL, Buffer+6, 2);
        else if (!strncmp(Buffer, "sgf ", 4))
            UserCommand(NULL, "%%sgf %s", Buffer);
        else if (!strncmp(Buffer, "refresh", 7)) {
            UserCommand(NULL, "%s", Buffer);
	    /* refresh only does not always pop up board, send moves too: */
            AutoCommand(NULL, "moves%s", Buffer+7);
        } else if (!strncmp(Buffer, "bet ", 4))
            UserCommand(NULL, "%%bet %s", Buffer);
        else if (!strncmp(Buffer, "literal ", 8))
            UserCommand(NULL, "%%literal %s", Buffer+8);
	else if (!strncmp(Buffer, "tell ", 5)) ExplicitTell(Buffer+5);
	else if (!strncmp(Buffer, "stats ", 6)) {
	    Player *player = PlayerFromName(Buffer+6);
	    CallGetStats(NULL, (XtPointer)player, NULL);
        } else if (Buffer[0] == '\0') {
	    char *line = TextWidgetCursorLine(Info);
	    char *cmd = line;

	    while (cmd[0] == '>' || cmd[0] == ' ') cmd++;
	    UserCommand(NULL, "%s", cmd);
	    myfree(line);
	} else {
	    UserCommand(NULL, "%s", Buffer);
	}
    }

    if (Prompt) IgsPrompt();
    XtVaSetValues(Input, XtNstring, "", NULL);
}

static void IgsUserCommand(Widget w, XEvent *event,
                           String *string, Cardinal *n)
{
    if (*n == 1) {
        if (!strncmp(string[0], "review", 6)) {
            ReviewWanted(NULL, string[0]+6, 1);
        } else if (!strcmp(string[0], "quit")) {
	    ForceQuit();
        } else if (!strncmp(string[0], "autoreply", 9)) {
	    appdata.AutoReply = !appdata.AutoReply;
	    XtVaSetValues(AutoReply, XtNstate, (XtArgVal)appdata.AutoReply,
			  NULL);
	    Outputf("Set autoreply to %s\n",
		    appdata.AutoReply ? "True" : "False");
	} else {
	    UserCommand(NULL, "%s", string[0]);
	}
    } else if (*n == 2) {
	UserCommand(NULL, "%s %s", string[0], string[1]);
    } else if (*n == 3) {
	UserCommand(NULL, "%s %s %s", string[0], string[1], string[2]);
    } else {
	WidgetWarning(w, "usercommand() expects at most 3 arguments");
    }
}

static void IgsSendCommand(Widget w, XEvent *event,
                           String *string, Cardinal *n)
{
    if (*n == 1)
        if (!strncmp(string[0], "review", 6))
            ReviewWanted(NULL, string[0]+6, 1);
        else SendCommand(NULL, NULL, "%s", string[0]);
    else WidgetWarning(w, "sendcommand() expects exactly 1 argument");
}

static void HelpUp(Widget w, XEvent *event,
		    String *string, Cardinal *n)
/* helpup(long_help, short_help)
 * display the short help in *shortHelp, and popup the long_help in help mode.
 */
{
    const char *str[2];
    Boolean help;

    if (*n != 2) {
	WidgetWarning(w, "helpup() expects exactly 2 arguments");
	return;
    }
    XtVaSetValues(ShortHelp, XtNlabel, (XtArgVal)string[1], NULL);

    XtVaGetValues(HelpButton, XtNstate, (XtArgVal) &help, NULL);
    if (help) {
	str[0] = string[0]; /* name of help widget */
	str[1] = "-1";      /* help window stays present, no grab */
	XtCallActionProc(w, (String)"help", event, (String*)str, 2); 
    }
}


static void HelpDown(Widget w, XEvent *event,
		     String *string, Cardinal *n)
/* helpdown(long_help)
 * erase the short help in *shortHelp, and popdown the long_help in help mode.
 */
{
    const char *str[2];
    Boolean help;

    if (*n != 1) {
	WidgetWarning(w, "helpdown() expects exactly 1 argument");
	return;
    }
    XtVaSetValues(ShortHelp, XtNlabel, (XtArgVal)
	      "Press shift + left mouse button on windows or buttons for help",
	      NULL);

    XtVaGetValues(HelpButton, XtNstate, (XtArgVal) &help, NULL);
    if (help) {
	str[0] = string[0]; /* name of help widget */
	str[1] = "down";    /* pop down help window */
	XtCallActionProc(w, (String)"help", event, (String*)str, 2); 
    }
}

static void toggleHelp(Widget w, XtPointer clientdata, XtPointer calldata)
{
    Boolean help;
    const char *str[2];

    str[0] = "main_help";

    XtVaGetValues(HelpButton, XtNstate, (XtArgVal) &help, NULL);
    if (help) {
	str[1] = "-1";      /* help window stays present, no grab */
    } else {
	str[1] = "down";    /* pop down help window */
    }
    XtCallActionProc(w, (String)"help", NULL, (String*)str, 2); 
}

static void DoOutput(Widget w, XEvent *event, String *string, Cardinal *n)
{
    Cardinal i;

    if (*n) Output(ResourceStringToString(string[0]));
    for (i=1; i<*n; i++) {
        Output(" ");
        Output(ResourceStringToString(string[i]));
    }
    Output("\n");
}

size_t Output(const char *Text)
{
    size_t Length;

    if (Entered && !OutputTime) {
        OutputTime = NEWOUTPUT;
        IfBell(OutputBeep);
        IfRaise(OutputRaise, OutputRaise);
    }

    Length = strlen(Text);
    BatchAppendText(Info, Text, Length, 0);
    return Length;
}

#ifndef HAVE_NO_STDARG_H
size_t Outputf(const char *Comment, ...)
#else  /* HAVE_NO_STDARG_H */
size_t Outputf(va_alist)
va_dcl
#endif /* HAVE_NO_STDARG_H */
{
    char    Text[2048];
    va_list args;

#ifndef HAVE_NO_STDARG_H
    va_start(args, Comment);
#else  /* HAVE_NO_STDARG_H */
    const char *Comment;

    va_start(args);
    Comment = va_arg(args, const char *);
#endif /* HAVE_NO_STDARG_H */
    vsprintf(Text, Comment, args);
    va_end(args);
    return Output(Text);
}

static void TestBoard(Widget w, XtPointer clientdata, XtPointer calldata)
{
    OpenAnalyze(0, "", 0, (size_t) appdata.AnalyzeSize, 0,
                appdata.AllowSuicide, UNKNOWN_ADVANTAGE);
}

void UserCommands(void)
{
    char *home = getenv("HOME");

    /* Read and send to igs ~/.xgospelrc if it exists */
    if (home) {
	FILE *rcfile;
	char command_line[255];
	char rcfile_name[255];

	strcpy(rcfile_name, home);
	strcat(rcfile_name, "/.xgospelrc");
	rcfile = fopen(rcfile_name, "r");
	if (rcfile) {
	    while (fgets(command_line, sizeof(command_line), rcfile)) {
		if (command_line[0] == '>') {
		    UserCommand(NULL, "%s", command_line+1);
		} else {
		    SendCommand(NULL, NULL, command_line);
		}
	    }
	    UserCommand(NULL, ""); /* force a prompt */
	    fclose(rcfile);
	}
    }
}

static void User_Commands(Widget w, XtPointer clientdata, XtPointer calldata)
{
    UserCommands();
}

/* Very simpleminded writability test. God help you if you are root */
static Boolean TryToWrite(String Name)
{
    DebugFile = fopen(Name, "w");
    if (DebugFile) return True;
    else           return False;
}

/*****************************************************************************/

static time_t StringToTime(const char *Date, const char *Tim)
{
    static const char *Month[] = {
        "Jan", "Feb", "Mar", "Apr", "May", "Jun",
        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

    struct tm   TmTime;
    char       *Ptr;
    int         i;

    if (strlen(Tim) != 8 || Tim[2] != ':' || Tim[5] != ':') return 0;
    Ptr = strchr(Date, ' ');
    if (!Ptr || Ptr-Date != 3) return 0;
    for (i=0; i<12; i++) if (memcmp(Date, Month[i], 3) == 0) {
        TmTime.tm_mon  = i;
        TmTime.tm_mday = strtol(Ptr, &Ptr, 10);
        TmTime.tm_year = strtol(Ptr, &Ptr, 10)-1900;
        if (*Ptr) return 0;

        TmTime.tm_hour = atoi(&Tim[0]);
        TmTime.tm_min  = atoi(&Tim[3]);
        TmTime.tm_sec  = atoi(&Tim[6]);

        TmTime.tm_isdst = LocalTime.tm_isdst;

        return mktime(&TmTime);
    }
    return 0;
}

static const char *DateString(struct tm *date)
{
    static char Buffer[80];
    char       *ptr;
    size_t      Length;

    if (DateFormat) {
        if (strftime(Buffer, sizeof(Buffer), DateFormat, date))
            return Buffer;
        Warning("strftime(.., %d, %s, ..) overflows the buffer\n",
                    sizeof(Buffer), DateFormat);
        myfree(DateFormat);
        DateFormat = NULL;
    }

    /* This code can also be used if your libraries do not have strftime */
    ptr = asctime(date);
    Length = strlen(ptr)-1;
    memcpy(Buffer, ptr, Length);
    Buffer[Length] = 0;
    return Buffer;
}

/* Returns 0 on success, 1 on failure */
static int SendTripMessage(Widget w, long Message)
{
    Display *dpy;
    Window   win;
    XEvent   myEvent;

    dpy = XtDisplay(w);
    win = XtWindow(w);
    myEvent.xclient.type         = ClientMessage;
    myEvent.xclient.display      = dpy;
    myEvent.xclient.window       = win;
    myEvent.xclient.message_type = TripAtom;
    myEvent.xclient.format       = 32;
    myEvent.xclient.data.l[0]    = Message;
    return !XSendEvent(dpy, win, False, NoEventMask, &myEvent);
}

static void TripMessage(Widget w, XEvent *event, String *string, Cardinal *n)
{
    LastTrip  = event->xclient.data.l[0];
    LastRoundTrip = CurTime-LastTrip;
    RoundTrip = ((MIX-1)*RoundTrip+RESOLUTION*LastRoundTrip)/MIX;
    if (DebugRoundTrip) {
        printf("LastRoundTrip = %3lu, RoundTrip = %4lu.%02lu\n",
               LastRoundTrip, RoundTrip/RESOLUTION,
               (100*RoundTrip)/RESOLUTION-100*(RoundTrip/RESOLUTION));
        fflush(stdout);
    }
}

static void HeartBeat(XtPointer closure, XtIntervalId *id)
{
    unsigned long Diff;
    time_t        NewTime;

    if (++UniversalTime.tm_sec < 60) {
        Diff = 1;
        if (++LocalTime.tm_sec >= 60) mktime(&LocalTime);
    } else {
        time(&NewTime);
        LocalTime     = *localtime(&NewTime);
        UniversalTime = *gmtime(   &NewTime);
#ifndef HAVE_NO_DIFFTIME
        Diff = (unsigned long) difftime(NewTime, BaseTime);
#else                           /* HAVE_NO_DIFFTIME */
        Diff = (unsigned long) (NewTime - BaseTime);
#endif                          /* HAVE_NO_DIFFTIME */
        Diff -= CurTime;
    }
    if (Diff) {
        GamesTime(Diff);
        ReviewsTime(Diff);
        AnalyzeTime(Diff);
        ConnectTime(Diff);

        if (SetServerTime && (ServerTime.tm_sec += Diff) >= 60)
            mktime(&ServerTime);
        CurTime += Diff;
        if (CurTime >= TripTime) {
            TripTime = CurTime + RETRIP;
            SendTripMessage(Main, (long) CurTime);
        }
        if (CurTime >= ClockTime) {
            ClockTime = CurTime + appdata.ClockTimeout;
            if (60 % appdata.ClockTimeout == 0)
                ClockTime -= LocalTime.tm_sec % appdata.ClockTimeout;
            if (Localtime)
                XtVaSetValues(Localtime, XtNstring,
                              (XtArgVal) DateString(&LocalTime), NULL);
            if (Universaltime)
                XtVaSetValues(Universaltime, XtNstring,
                              (XtArgVal) DateString(&UniversalTime), NULL);
            if (SetServerTime && Servertime)
                XtVaSetValues(Servertime, XtNstring,
                              (XtArgVal) DateString(&ServerTime), NULL);
        }
        if (OutputTime)
            if (OutputTime > Diff) OutputTime -= Diff;
            else OutputTime = 0;
    }
    XtAppAddTimeOut((XtAppContext) closure, 1000, HeartBeat, closure);
}

static void InitHeartBeat(XtAppContext appcontext)
{
    TripAtom = GetAtom(Main, "XGOSPEL_TRIP_PROBE");
    time(&BaseTime);
    LocalTime     = *localtime(&BaseTime);
    UniversalTime = *gmtime(   &BaseTime);
    CurTime       = 0;
    SetServerTime = 0;
    RoundTrip     = 0;
    ClockTime = TripTime = LastTrip = OutputTime = 0;
    XtAppAddTimeOut(appcontext, 1000, HeartBeat, (XtPointer) appcontext);

    /* warn if connecting to new site before change at Oct 26, 1998: */
    if (!strcmp(appdata.Site, SITE) && LocalTime.tm_year <= 98 &&
	(LocalTime.tm_mon < 9 ||
 	 (LocalTime.tm_mon == 9 && LocalTime.tm_mday <= 26))) {
	ServerMessage(
         "You are connected to the new site %s\n"
	 "To connect to the old site, start xgospel as:\n"
         "  xgospel -site igs.nuri.net &\n"
         "or use the resource: xgospel*site: igs.nuri.net\n", appdata.Site);
    }
}

/*****************************************************************************/

static void ConvertAnalyzeSize(XtPointer Closure)
{
    size_t Size;

    Size = appdata.AnalyzeSize;
    if (ConvertSize("Analyze size error", &Size, Closure, 1))
        appdata.AnalyzeSize = Size;
}

static void ChangeAnalyzeSize(Widget w,
                              XtPointer clientdata, XtPointer calldata)
{
    Widget        Root;
    char          size[80];
    static char  *Result;
    XtVarArgsList Args;

    Result = NULL;
    sprintf(size, "%d", appdata.AnalyzeSize);
    Args = XtVaCreateArgsList(NULL, "size", size, NULL);
    Root = AskString(toplevel, ConvertAnalyzeSize, (XtPointer) &Result,
                     "Enter size of analyze board",
                     "analyzeSize", &Result, Args, NULL);
    XtFree(Args);
    MyDependsOn(Root, w);
}

typedef struct {
    AppDataPtr appData;
    char      *analyzeSize;
    char      *allowSuicide;
    char      *simpleNames;
    char      *numberKibitzes;
    char      *minSecPerMove;
    char      *minLagMargin;
    char      *replayTimeout;
    char      *gamesTimeout;
    char      *whoTimeout;
#ifdef DO_REVIEW
    char      *reviewsTimeout;
#endif
    char      *clockTimeout;
    char      *serverTimeout;
    char      *inactiveTimeout;
    char      *playersUpdateTimeout;
    char      *gamesUpdateTimeout;
#ifdef DO_REVIEW
    char      *reviewsUpdateTimeout;
#endif
    char      *quitTimeout;
    char      *tersePlay;
} SettingsType;

static void ConvertSettings(XtPointer Closure)
{
    SettingsType *Settings;
    int oldSimpleNames = appdata.SimpleNames;

    Settings = (SettingsType *) Closure;
    ConvertAnalyzeSize((XtPointer) &Settings->analyzeSize);
    ConvertBoolean("Allow suicide error",
                   &Settings->appData->AllowSuicide,
                   (XtPointer) &Settings->allowSuicide, 1);
    ConvertBoolean("Simple names error",
                   &Settings->appData->SimpleNames,
                   (XtPointer) &Settings->simpleNames, 1);
    if (appdata.SimpleNames != oldSimpleNames) {
	if (DebugFun) {
	    printf("simpleNames %d => %d\n",
		   oldSimpleNames, appdata.SimpleNames);
	    fflush(stdout);
	}
	InitHash();
	ResetPlayersImportance();
	PlayersResort();
	GamesResort();
	/* "who" and "games" needed to get complete update: */
	if (appdata.WhoTimeout > 0) AutoCommand(NULL, "who");
	if (appdata.GamesTimeout > 0) AutoCommand(NULL, "games");
    }
    ConvertBoolean("Number kibitzes error",
                   &Settings->appData->NumberKibitzes,
                   (XtPointer) &Settings->numberKibitzes, 1);
    ConvertNatural("Seconds per move error",
		    &Settings->appData->MinSecPerMove,
                    (XtPointer) &Settings->minSecPerMove, 1);
    ConvertNatural("Lag margin error",
		    &Settings->appData->MinLagMargin,
                    (XtPointer) &Settings->minLagMargin, 1);
    ConvertPositive("Replay rate error", &Settings->appData->ReplayTimeout,
                    (XtPointer) &Settings->replayTimeout, 1);
    ConvertNatural("Who rate error", &Settings->appData->WhoTimeout,
                    (XtPointer) &Settings->whoTimeout, 1);
    ConvertNatural("Games rate error", &Settings->appData->GamesTimeout,
                    (XtPointer) &Settings->gamesTimeout, 1);
#ifdef DO_REVIEW
    ConvertPositive("Reviews rate error", &Settings->appData->ReviewsTimeout,
                    (XtPointer) &Settings->reviewsTimeout, 1);
#endif
    if (ConvertPositive("Clock rate error", &Settings->appData->ClockTimeout,
                        (XtPointer) &Settings->clockTimeout, 1))
        ClockTime = CurTime;
    ConvertPositive("Server resend rate error",
                    &Settings->appData->ServerTimeout,
                    (XtPointer) &Settings->serverTimeout, 1);
    ConvertPositive("Disconnect rate error",
                    &Settings->appData->InactiveTimeout,
                    (XtPointer) &Settings->inactiveTimeout, 1);
    ConvertNatural ("Player window update rate error",
                    &Settings->appData->PlayerUpdateTimeout,
                    (XtPointer) &Settings->playersUpdateTimeout, 1);
    ConvertNatural ("Game window update rate error",
                    &Settings->appData->GameUpdateTimeout,
                    (XtPointer) &Settings->gamesUpdateTimeout, 1);
#ifdef DO_REVIEW
    ConvertNatural ("Review window update rate error",
                    &Settings->appData->ReviewUpdateTimeout,
                    (XtPointer) &Settings->reviewsUpdateTimeout, 1);
#endif
    ConvertPositive("quit timeout error", &Settings->appData->QuitTimeout,
                    (XtPointer) &Settings->quitTimeout, 1);
    if (ConvertBoolean("Terse play error",
                       &Settings->appData->TersePlay,
                       (XtPointer) &Settings->tersePlay, 1)) {
        /* Send games/who if was requested --Ton */ ;
    }
}

static void ChangeSettings(Widget w, XtPointer clientdata, XtPointer calldata)
{
    AppDataPtr    appData;
    Widget        Root;
    char          size[40], replayTimeout[40], whoTimeout[40], quitTimeout[40];
    char          gamesTimeout[40], reviewsTimeout[40], serverTimeout[40];
    char          playersUpdateTimeout[40], gamesUpdateTimeout[40];
    char          reviewsUpdateTimeout[40], clockTimeout[40];
    char          inactiveTimeout[40], minSecPerMove[40], minLagMargin[40];
    const char   *tersePlay, *allowSuicide, *simpleNames, *numberKibitzes;
    XtVarArgsList ArgsAna, ArgsAllowSuicide, ArgsReplay, ArgsWho, ArgsGames;
    XtVarArgsList ArgsMinSecPerMove, ArgsMinLagMargin, ArgsSimpleNames;
    XtVarArgsList ArgsNumberKibitzes;
    XtVarArgsList ArgsReviews, ArgsServer, ArgsPlayersUpdate, ArgsGamesUpdate;
    XtVarArgsList ArgsReviewsUpdate, ArgsQuit, ArgsInactive;
    XtVarArgsList ArgsTerse, ArgsClock;
    /* Allocate is better as soon as we have more contexts --Ton */
    static SettingsType  Settings;

    appData = (AppDataPtr) clientdata;
    Settings.appData        = appData;
    Settings.analyzeSize    = NULL;
    Settings.allowSuicide   = NULL;
    Settings.simpleNames    = NULL;
    Settings.numberKibitzes = NULL;
    Settings.minSecPerMove  = NULL;
    Settings.minLagMargin   = NULL;
    Settings.replayTimeout  = NULL;
    Settings.whoTimeout     = NULL;
    Settings.gamesTimeout   = NULL;
#ifdef DO_REVIEW
    Settings.reviewsTimeout = NULL;
    Settings.reviewsUpdateTimeout = NULL;
#endif
    Settings.clockTimeout   = NULL;
    Settings.serverTimeout  = NULL;
    Settings.inactiveTimeout      = NULL;
    Settings.playersUpdateTimeout = NULL;
    Settings.gamesUpdateTimeout   = NULL;
    Settings.quitTimeout    = NULL;
    Settings.tersePlay      = NULL;

    sprintf(size,            "%d", appData->AnalyzeSize);
    ArgsAna     = XtVaCreateArgsList(NULL, "size",    size, NULL);
    allowSuicide = appData->AllowSuicide != False ? "True" : "False";
    ArgsAllowSuicide = XtVaCreateArgsList(NULL, "boolean", allowSuicide, NULL);
    simpleNames = appData->SimpleNames != False ? "True" : "False";
    ArgsSimpleNames = XtVaCreateArgsList(NULL, "boolean", simpleNames, NULL);
    numberKibitzes = appData->NumberKibitzes != False ? "True" : "False";
    ArgsNumberKibitzes = XtVaCreateArgsList(NULL, "boolean", numberKibitzes, NULL);
    sprintf(replayTimeout,  "%d", appData->ReplayTimeout);
    ArgsMinSecPerMove = XtVaCreateArgsList(NULL, "timeout",
					   minSecPerMove, NULL);
    sprintf(minSecPerMove,     "%d", appData->MinSecPerMove);
    ArgsMinLagMargin = XtVaCreateArgsList(NULL, "timeout", minLagMargin, NULL);
    sprintf(minLagMargin,     "%d", appData->MinLagMargin);
    ArgsReplay  = XtVaCreateArgsList(NULL, "timeout", replayTimeout, NULL);
    sprintf(whoTimeout,     "%d", appData->WhoTimeout);
    ArgsWho     = XtVaCreateArgsList(NULL, "timeout", whoTimeout, NULL);
    sprintf(gamesTimeout,   "%d", appData->GamesTimeout);
    ArgsGames   = XtVaCreateArgsList(NULL, "timeout", gamesTimeout, NULL);
#ifdef DO_REVIEW
    sprintf(reviewsTimeout, "%d", appData->ReviewsTimeout);
    ArgsReviews = XtVaCreateArgsList(NULL, "timeout", reviewsTimeout, NULL);
#endif
    sprintf(clockTimeout, "%d", appData->ClockTimeout);
    ArgsClock   = XtVaCreateArgsList(NULL, "timeout", clockTimeout, NULL);
    sprintf(serverTimeout,  "%d", appData->ServerTimeout);
    ArgsServer  = XtVaCreateArgsList(NULL, "timeout", serverTimeout, NULL);
    sprintf(inactiveTimeout,   "%d", appData->InactiveTimeout);
    ArgsInactive = XtVaCreateArgsList(NULL, "timeout", inactiveTimeout, NULL);
    sprintf(playersUpdateTimeout, "%d", appData->PlayerUpdateTimeout);
    ArgsPlayersUpdate =
        XtVaCreateArgsList(NULL, "timeout", playersUpdateTimeout, NULL);
    sprintf(gamesUpdateTimeout,   "%d", appData->GameUpdateTimeout);
    ArgsGamesUpdate   =
        XtVaCreateArgsList(NULL, "timeout", gamesUpdateTimeout, NULL);
#ifdef DO_REVIEW
    sprintf(reviewsUpdateTimeout, "%d", appData->ReviewUpdateTimeout);
    ArgsReviewsUpdate =
        XtVaCreateArgsList(NULL, "timeout", reviewsUpdateTimeout, NULL);
#endif
    sprintf(quitTimeout,    "%d", appData->QuitTimeout);
    ArgsQuit  = XtVaCreateArgsList(NULL, "timeout", quitTimeout, NULL);
    tersePlay = appData->TersePlay != False ? "True" : "False";
    ArgsTerse = XtVaCreateArgsList(NULL, "boolean", tersePlay, NULL);

    Root = AskString(toplevel, ConvertSettings, (XtPointer) &Settings,
                     "Enter new settings",
                     "analyzeSize",    &Settings.analyzeSize,    ArgsAna,
                     "allowSuicide",   &Settings.allowSuicide,ArgsAllowSuicide,
                     "simpleNames",    &Settings.simpleNames,ArgsSimpleNames,
                     "numberKibitzes", &Settings.numberKibitzes,
		        ArgsNumberKibitzes,
		     "minSecPerMove",  &Settings.minSecPerMove,
                        ArgsMinSecPerMove,
		     "minLagMargin", &Settings.minLagMargin, ArgsMinLagMargin,
                     "replayTimeout",  &Settings.replayTimeout,  ArgsReplay,
                     "whoTimeout",     &Settings.whoTimeout,     ArgsWho,
                     "gamesTimeout",   &Settings.gamesTimeout,   ArgsGames,
#ifdef DO_REVIEW
                     "reviewsTimeout", &Settings.reviewsTimeout, ArgsReviews,
#endif
                     "clockTimeout",   &Settings.clockTimeout,   ArgsClock,
                     "inactiveTimeout",      &Settings.inactiveTimeout,
                     ArgsInactive,
                     "playersUpdateTimeout", &Settings.playersUpdateTimeout,
                     ArgsPlayersUpdate,
                     "gamesUpdateTimeout", &Settings.gamesUpdateTimeout,
                     ArgsGamesUpdate,
#ifdef DO_REVIEW
                     "reviewsUpdateTimeout", &Settings.reviewsUpdateTimeout,
                     ArgsReviewsUpdate,
#endif
                     "serverTimeout",  &Settings.serverTimeout,  ArgsServer,
                     "quitTimeout",    &Settings.quitTimeout,    ArgsQuit,
                     "tersePlay",      &Settings.tersePlay,      ArgsTerse,
                     NULL);

    XtFree(ArgsAna);
    XtFree(ArgsAllowSuicide);
    XtFree(ArgsSimpleNames);
    XtFree(ArgsNumberKibitzes);
    XtFree(ArgsMinSecPerMove);
    XtFree(ArgsMinLagMargin);
    XtFree(ArgsReplay);
    XtFree(ArgsWho);
    XtFree(ArgsGames);
#ifdef DO_REVIEW
    XtFree(ArgsReviews);
    XtFree(ArgsReviewsUpdate);
#endif
    XtFree(ArgsServer);
    XtFree(ArgsInactive);
    XtFree(ArgsPlayersUpdate);
    XtFree(ArgsGamesUpdate);
    XtFree(ArgsQuit);
    XtFree(ArgsTerse);
    XtFree(ArgsClock);
    MyDependsOn(Root, w);
}

static void ChangeMainFilename(Widget w,
                               XtPointer clientdata, XtPointer calldata)
{
    char **Filename;
    Widget Root;

    Filename = (char **) clientdata;
    Root = ChangeFilename(toplevel, "mainFilename",
                          "Enter name of main file",
                          Filename, "filename", *Filename, NULL);
    MyDependsOn(Root, w);
}

static void ChangeAutoReplyMessage(Widget w,
				   XtPointer clientdata, XtPointer calldata)
{
    char **Reply;
    Widget Root;

    Reply = (char **) clientdata;
    Root = ChangeFilename(toplevel, "autoReplyMessage",
                          "Enter auto reply message",
                          Reply, "filename", *Reply, NULL);
    MyDependsOn(Root, w);
}

static void SaveMain(Widget w, XtPointer clientdata, XtPointer calldata)
{
    Boolean                   overwrite;
    static char const * const ErrorType = "Main save error";

    if (Info) {
        if (Overwrite) CallReadToggle(Overwrite, (XtPointer) &overwrite, NULL);
        else           overwrite = False;

        SaveWrite(MainFileName, overwrite,
                  ErrorBeep, ErrorRaise, ErrorType,
                  SaveTextFun, (XtPointer) Info);
    } else {
        IfBell(ErrorBeep);
        IfRaise(ErrorRaise, ErrorRaise);
        PopMessage(ErrorType, "Main messages were ignored so there is "
                   "nothing to be saved");
    }
}

static char *MainTitle(const char *Pattern, XtPointer Closure)
{
    return StringToFilename(Pattern,
                            (int) 'N', (char *) Closure,
                            (int) 'v', VERSION,
                            (int) 'V', appdata.Version,
                            (int) 'd', DATE,
                            (int) 'D', CompileTime, /* in GMT format */
                            0);
}

static XrmOptionDescRec options[] = {
    {(char *)"-fg",	    (char *)"*foreground", XrmoptionSepArg, NULL},
    {(char *)"-foreground", (char *)"*foreground", XrmoptionSepArg, NULL},
    {(char *)"-user",       (char *)"*user",       XrmoptionSepArg, NULL},
    {(char *)"-password",   (char *)"*password",   XrmoptionSepArg, NULL},
    {(char *)"-host",       (char *)"*site",       XrmoptionSepArg, NULL},
    {(char *)"-site",       (char *)"*site",       XrmoptionSepArg, NULL},
    {(char *)"-port",       (char *)"*port",       XrmoptionSepArg, NULL},
    {(char *)"-debugFile",  (char *)"*debugFile",  XrmoptionSepArg, NULL},
    {(char *)"-debugFun",   (char *)"*debugFun",   XrmoptionSepArg, NULL},
    {(char *)"-debug",      (char *)"*debug",	   XrmoptionNoArg,
         (XPointer) "True"},
};

static const char *UseMessages[] = {
    "    -user       name         Userid on IGS",
    "    -password   secret       Password on IGS",
    "    -host       site         Connect to site",
    "    -site       site         Connect to site",
    "    -port       port         Connect to port",
    "    -debugfile  name         File to log interaction with IGS",
    "    -debugfun   int          Set program debugging flags",
    "    -debug                   Turn on internal widget debugging",
};

#define XtNuser                  "user"
#define XtCUser                  "User"
#define XtNpassword              "password"
#define XtCPassword              "Password"
#define XtNuseTerm               "useTerm"
#define XtCUseTerm               "UseTerm"
#define XtNdebugFun              "debugFun"
#define XtCDebugFun              "DebugFun"
#define XtNdebugFile             "debugFile"
#define XtCDebugFile             "DebugFile"
#define XtNkibitzSize            "kibitzSize"
#define XtCKibitzSize            "KibitzSize"
#define XtNtellsize              "tellsize"
#define XtCTellsize              "Tellsize"
#define XtNbroadcastSize         "broadcastSize"
#define XtCBroadcastSize         "BroadcastSize"
#define XtNyellsize              "yellsize"
#define XtCYellsize              "Yellsize"
#define XtCKibitzSize            "KibitzSize"
#define XtNmaintainer            "maintainer"
#define XtCMaintainer            "Maintainer"
#define XtNversion               "version"
#define XtCVersion               "Version"
#define XtNdateFormat            "dateFormat"
#define XtNsgfDateFormat         "sgfDateFormat"
#define XtCDateFormat            "DateFormat"
#define XtNminHiDanRank          "minHiDanRank"
#define XtCMinHiDanRank          "MinHiDanRank"
#define XtNminProRank            "minProRank"
#define XtCMinProRank            "MinProRank"
#define XtNminImportantRank      "minImportantRank"
#define XtCMinImportantRank      "MinImportantRank"
#define XtNreplayTimeout         "replayTimeout"
#define XtNgamesTimeout          "gamesTimeout"
#define XtNreviewsTimeout        "reviewsTimeout"
#define XtNclockTimeout          "clockTimeout"
#define XtNserverTimeout         "serverTimeout"
#define XtNinactiveTimeout       "inactiveTimeout"
#define XtNplayersUpdateTimeout  "playersUpdateTimeout"
#define XtNgamesUpdateTimeout    "gamesUpdateTimeout"
#define XtNreviewsUpdateTimeout  "reviewsUpdateTimeout"
#define XtNwhoTimeout            "whoTimeout"
#define XtNquitTimeout           "quitTimeout"
#define XtNreconnectTimeout      "reconnectTimeout"
#define XtNminButtonTime         "minButtonTime"
#define XtCTimeout               "Timeout"
#define XtNgamesScale            "gamesScale"
#define XtNplayersScale          "playersScale"
#define XTCScale                 "Scale"
#define XtNscrollUnit            "scrollUnit"
#define XtCScrollUnit            "ScrollUnit"
#define XtNsite                  "site"
#define XtCSite                  "Site"
#define XtNport                  "port"
#define XtCPort                  "Port"
#define XtNdirectory             "directory"
#define XtCDirectory             "Directory"
#define XtNanalyzeFilename       "analyzeFilename"
#define XtNsgfFilename           "sgfFilename"
#define XtNpsFilename            "psFilename"
#define XtNkibitzFilename        "kibitzFilename"
#define XtNbroadcastFilename     "broadcastFilename"
#define XtNyellFilename          "yellFilename"
#define XtNigsMessageFilename    "igsMessageFilename"
#define XtNtellFilename          "tellFilename"
#define XtNmessageFilename       "messageFilename"
#define XtNeventsFilename        "eventsFilename"
#define XtNgamesFilename         "gamesFilename"
#define XtNplayersFilename       "playersFilename"
#define XtNmainFilename          "mainFilename"
#define XtCFileFilename          "FileFilename"
#define XtNautoReplyMessage      "autoReplyMessage"
#define XtCAutoReplyMessage      "AutoReplyMessage"
#define XtNtellKill              "tellKill"
#define XtNyellKill              "yellKill"
#define XtNbroadcastKill         "broadcastKill"
#define XtNkibitzKill            "kibitzKill"
#define XtCKill                  "Kill"
#define XtNtellPass              "tellPass"
#define XtNyellPass              "yellPass"
#define XtNbroadcastPass         "broadcastPass"
#define XtNkibitzPass            "kibitzPass"
#define XtCPass                  "Pass"
#define XtNfriends               "friends"
#define XtCFriends               "Friends"
#define XtCTextToWidget          "TextToWidget"
#define XtNplayerToWidget        "playerToWidget"
#define XtNplayerImportance      "playerImportance"
#define XtNgameToWidget          "gameToWidget"
#define XtNreviewToWidget        "reviewToWidget"
#define XtNanalyzeSize           "analyzeSize"
#define XtCAnalyzeSize           "AnalyzeSize"
#define XtNallowSuicide          "allowSuicide"
#define XtCAllowSuicide          "AllowSuicide"
#define XtNversionMessage        "versionMessage"
#define XtCVersionMessage        "VersionMessage"
#define XtNtersePlay             "tersePlay"
#define XtCTersePlay             "TersePlay"
#define XtNsimpleNames           "simpleNames"
#define XtCSimpleNames           "SimpleNames"
#define XtNuseSay                "useSay"
#define XtCUseSay                "UseSay"
#define XtNmarkTerritories       "markTerritories"
#define XtCMarkTerritories       "MarkTerritories"
#define XtNmarkDame              "markDame"
#define XtCMarkDame              "MarkDame"
#define XtNautoScore             "autoScore"
#define XtCAutoScore             "AutoScore"
#define XtNnumberKibitzes        "numberKibitzes"
#define XtCNumberKibitzes        "NumberKibitzes"
#define XtNdefaultByoYomi        "defaultByoYomi"
#define XtCDefaultByoYomi        "DefaultByoYomi"
#define XtNdefaultTime           "defaultTime"
#define XtCDefaultTime           "DefaultTime"
#define XtNmyLowTimeBackground   "myLowTimeBackground"
#define XtCMyLowTimeBackground   "MyLowTimeBackground"
#define XtNmyLowTimeForeground   "myLowTimeForeground"
#define XtCMyLowTimeForeground   "MyLowTimeForeground"
#define XtNlowTimeBackground     "lowTimeBackground"
#define XtCLowTimeBackground     "LowTimeBackground"
#define XtNlowTimeForeground     "lowTimeForeground"
#define XtCLowTimeForeground     "LowTimeForeground"
#define XtNlowTimeSet            "lowTimeSet"
#define XtCLowTimeSet            "LowTimeSet"
#define XtNminSecPerMove         "minSecPerMove"
#define XtCMinSecPerMove         "MinSecPerMove"
#define XtNminLagMargin          "minLagMargin"
#define XtCMinLagMargin          "MinLagMargin"


#define offset(field) XtOffset(AppDataPtr, field)

static XtResource resources[] = {
    { (String) XtNuser, (String) XtCUser, XtRString, sizeof(String),
      offset(User), XtRString, NULL },
    { (String) XtNpassword, (String) XtCPassword, XtRString, sizeof(String),
      offset(Password), XtRString, NULL },
    { (String) XtNuseTerm, (String) XtCUseTerm, XtRBoolean, sizeof(Boolean),
      offset(UseTerm), XtRString, (XtPointer) "True" },
    { (String) XtNdebugFile, (String) XtCDebugFile, XtRString, sizeof(String),
      offset(DebugFile), XtRString, NULL },
    { (String) XtNdebugFun, (String) XtCDebugFun, XtRInt, sizeof(int),
      offset(DebugFun), XtRString, (XtPointer) "0" },
    { (String) XtNtellsize, (String) XtCTellsize, XtRInt, sizeof(int),
      offset(TellSize), XtRString, (XtPointer) "128" },
    { (String) XtNkibitzSize, (String) XtCKibitzSize, XtRInt, sizeof(int),
      offset(KibitzSize), XtRString, (XtPointer) "180" },
    { (String) XtNmaintainer, (String) XtCMaintainer, XtRString, sizeof(String),
      offset(Maintainer), XtRString, (XtPointer) "xgospel" },
    { (String) XtNversion, (String) XtCVersion, XtRString, sizeof(String),
      offset(Version), XtRString, (XtPointer) VERSION },
    { (String) XtNversionMessage, (String) XtCVersionMessage, XtRString, sizeof(String),
      offset(VersionMessage), XtRString, (XtPointer) VERSIONMESSAGE },
    { (String) XtNdateFormat, (String) XtCDateFormat, XtRString, sizeof(String),
      offset(DateFormat), XtRString, NULL },
    { (String) XtNsgfDateFormat, (String) XtCDateFormat, XtRString, sizeof(String),
      offset(SgfDateFormat), XtRString, (XtPointer) "%Y-%m-%d" },
      /*  %H:%M:%S not allowed by FF[4]
       * http://www.sbox.tu-graz.ac.at/home/h/hollosi/sgf/properties.html#DT
       */
    { (String) XtNminHiDanRank, (String) XtCMinHiDanRank, XtRString, sizeof(String),
      offset(MinHiDanRank), XtRString, (XtPointer) "5d" },
    { (String) XtNminProRank, (String) XtCMinProRank, XtRString, sizeof(String),
      offset(MinProRank), XtRString, (XtPointer) "7d" },
    { (String) XtNminImportantRank, (String) XtCMinImportantRank, XtRString, sizeof(String),
      offset(MinImportantRank), XtRString, (XtPointer) "7d" },
    { (String) XtNbroadcastSize, (String) XtCBroadcastSize, XtRInt, sizeof(int),
      offset(BroadcastSize), XtRString, (XtPointer) "128" },
    { (String) XtNyellsize, (String) XtCYellsize, XtRInt, sizeof(int),
      offset(YellSize), XtRString, (XtPointer) "128" },
    { (String) XtNtersePlay, (String) XtCTersePlay, XtRBoolean, sizeof(Boolean),
      offset(TersePlay), XtRString, (XtPointer) "True" },
    { (String) XtNplayersScale, (String) XTCScale, XtRInt, sizeof(int),
      offset(PlayersScale), XtRString, (XtPointer) "20" },
    { (String) XtNgamesScale, (String) XTCScale, XtRInt, sizeof(int),
      offset(GamesScale), XtRString, (XtPointer) "5" },
    { (String) XtNscrollUnit, (String) XtCScrollUnit, XtRInt, sizeof(int),
      offset(ScrollUnit), XtRString, (XtPointer) "20" },
    { (String) XtNserverTimeout, (String) XtCTimeout, XtRInt, sizeof(int),
      offset(ServerTimeout), XtRString, (XtPointer) "120" },
    { (String) XtNinactiveTimeout, (String) XtCTimeout, XtRInt, sizeof(int),
      offset(InactiveTimeout), XtRString, (XtPointer) "2700" },
    { (String) XtNwhoTimeout, (String) XtCTimeout, XtRInt, sizeof(int),
      offset(WhoTimeout), XtRString, (XtPointer) "300" },
    { (String) XtNplayersUpdateTimeout, (String) XtCTimeout, XtRInt, sizeof(int),
      offset(PlayerUpdateTimeout), XtRString, (XtPointer) "15" },
    { (String) XtNreplayTimeout, (String) XtCTimeout, XtRInt, sizeof(int),
      offset(ReplayTimeout), XtRString, (XtPointer) "1" },
    { (String) XtNgamesTimeout, (String) XtCTimeout, XtRInt, sizeof(int),
      offset(GamesTimeout), XtRString, (XtPointer) "310" },
    { (String) XtNgamesUpdateTimeout, (String) XtCTimeout, XtRInt, sizeof(int),
      offset(GameUpdateTimeout), XtRString, (XtPointer) "2" },
    { (String) XtNreviewsTimeout, (String) XtCTimeout, XtRInt, sizeof(int),
      offset(ReviewsTimeout), XtRString, (XtPointer) "1200" },
    { (String) XtNclockTimeout, (String) XtCTimeout, XtRInt, sizeof(int),
      offset(ClockTimeout), XtRString, (XtPointer) "1" },
    { (String) XtNreviewsUpdateTimeout, (String) XtCTimeout, XtRInt, sizeof(int),
      offset(ReviewUpdateTimeout), XtRString, (XtPointer) "2" },
    { (String) XtNquitTimeout, (String) XtCTimeout, XtRInt, sizeof(int),
      offset(QuitTimeout), XtRString, (XtPointer) "10" },
    { (String) XtNreconnectTimeout, (String) XtCTimeout, XtRInt, sizeof(int),
      offset(ReconnectTimeout), XtRString, (XtPointer) "15" },
    { (String) XtNminButtonTime, (String) XtCTimeout, XtRInt, sizeof(int),
      offset(MinButtonTime), XtRString, (XtPointer) "50" },
    { (String) XtNanalyzeSize, (String) XtCAnalyzeSize, XtRInt, sizeof(int),
      offset(AnalyzeSize), XtRString, (XtPointer) "19" },
    { (String) XtNallowSuicide, (String) XtCAllowSuicide, XtRBoolean, sizeof(Boolean),
      offset(AllowSuicide), XtRString, (XtPointer) "False" },
    { (String) XtNdirectory, (String) XtCDirectory, XtRString, sizeof(String),
      offset(Directory), XtRString, (XtPointer) "" },
    { (String) XtNanalyzeFilename, (String) XtCFileFilename, XtRString, sizeof(String),
      offset(AnalyzeFilename), XtRString, (XtPointer) "%D%N%l%L%U%B%U%b%U%V%U%w%U%W%t%T" },
    { (String) XtNsgfFilename, (String) XtCFileFilename, XtRString, sizeof(String),
      offset(SgfFilename), XtRString, (XtPointer) "%D%B%U%b%U%V%U%w%U%W%t%T" },
    { (String) XtNpsFilename, (String) XtCFileFilename, XtRString, sizeof(String),
      offset(PsFilename),  XtRString, (XtPointer) "%D%B%U%b%U%V%U%w%U%W%t%T" },
    { (String) XtNkibitzFilename, (String) XtCFileFilename, XtRString, sizeof(String),
      offset(KibitzFilename), XtRString, (XtPointer) "%D%B%U%b%U%V%U%w%U%W%t%T" },
    { (String) XtNbroadcastFilename, (String) XtCFileFilename, XtRString, sizeof(String),
      offset(BroadcastFilename), XtRString, (XtPointer) "%D%T" },
    { (String) XtNyellFilename, (String) XtCFileFilename, XtRString, sizeof(String),
      offset(YellFilename), XtRString, (XtPointer) "%D%T" },
    { (String) XtNigsMessageFilename, (String) XtCFileFilename, XtRString, sizeof(String),
      offset(IgsMessageFilename), XtRString, (XtPointer) "%D%T" },
    { (String) XtNeventsFilename, (String) XtCFileFilename, XtRString, sizeof(String),
      offset(EventsFilename), XtRString, (XtPointer) "%D%T" },
    { (String) XtNtellFilename, (String) XtCFileFilename, XtRString, sizeof(String),
      offset(TellFilename), XtRString, (XtPointer) "%D%N%t%n.%T" },
    { (String) XtNmessageFilename, (String) XtCFileFilename, XtRString, sizeof(String),
      offset(MessageFilename), XtRString, (XtPointer) "%D%N%U%n.%T" },
    { (String) XtNgamesFilename, (String) XtCFileFilename, XtRString, sizeof(String),
      offset(GamesFilename), XtRString, (XtPointer) "%D%T" },
    { (String) XtNplayersFilename, (String) XtCFileFilename, XtRString, sizeof(String),
      offset(PlayersFilename), XtRString, (XtPointer) "%D%T" },
    { (String) XtNmainFilename, (String) XtCFileFilename, XtRString, sizeof(String),
      offset(MainFilename), XtRString, (XtPointer) "%D%T" },
    { (String) XtNautoReplyMessage, (String) XtCAutoReplyMessage, XtRString, sizeof(String),
      offset(AutoReplyMessage), XtRString,
      (XtPointer) "I'm away for a moment, this is an automatic reply." },
    { (String) XtNplayerToWidget, (String) XtCTextToWidget, (String) XtRStringPairList,
      sizeof(StringPairListPtr), offset(PlayerToWidget), XtRString, NULL},
    { (String) XtNplayerImportance, (String) XtCTextToWidget, (String) XtRStringPairList,
      sizeof(StringPairListPtr), offset(PlayerImportance), XtRString, NULL},
    { (String) XtNgameToWidget, (String) XtCTextToWidget, (String) XtRStringPairList,
      sizeof(StringPairListPtr), offset(GameToWidget), XtRString, NULL},
    { (String) XtNreviewToWidget, (String) XtCTextToWidget, (String) XtRStringPairList,
      sizeof(StringPairListPtr), offset(ReviewToWidget), XtRString, NULL},
    { (String) XtNtellKill, (String) XtCKill, (String) XtRStringList,
      sizeof(StringListPtr), offset(TellKill), XtRString, NULL},
    { (String) XtNtellPass, (String) XtCPass, (String) XtRStringList,
      sizeof(StringListPtr), offset(TellPass), XtRString, NULL},
    { (String) XtNyellKill, (String) XtCKill, (String) XtRStringList,
      sizeof(StringListPtr), offset(YellKill), XtRString, NULL},
    { (String) XtNyellPass, (String) XtCPass, (String) XtRStringList,
      sizeof(StringListPtr), offset(YellPass), XtRString, NULL},
    { (String) XtNbroadcastKill, (String) XtCKill, (String) XtRStringList,
      sizeof(StringListPtr), offset(BroadcastKill), XtRString, NULL},
    { (String) XtNbroadcastPass, (String) XtCPass, (String) XtRStringList,
      sizeof(StringListPtr), offset(BroadcastPass), XtRString, NULL},
    { (String) XtNkibitzKill, (String) XtCKill, (String) XtRStringList,
      sizeof(StringListPtr), offset(KibitzKill), XtRString, NULL},
    { (String) XtNkibitzPass, (String) XtCPass, (String) XtRStringList,
      sizeof(StringListPtr), offset(KibitzPass), XtRString, NULL},
    { (String) XtNfriends, (String) XtCFriends, (String) XtRStringList,
      sizeof(StringListPtr), offset(Friends), XtRString, NULL},
    { (String) XtNsite, (String) XtCSite, XtRString, sizeof(String),
      offset(Site), XtRString, (XtPointer) SITE },
    { (String) XtNport, (String) XtCPort, XtRInt, sizeof(int),
      offset(Port), XtRString, (XtPointer) "6969" },
    { (String) XtNsimpleNames, (String) XtCSimpleNames, XtRBoolean, sizeof(Boolean),
      offset(SimpleNames), XtRString, (XtPointer) "True" },
    { (String) XtNuseSay, (String) XtCUseSay, XtRBoolean, sizeof(Boolean),
      offset(UseSay), XtRString, (XtPointer) "True" },
    { (String) XtNmarkTerritories, (String) XtCMarkTerritories, XtRBoolean, sizeof(Boolean),
      offset(MarkTerritories), XtRString, (XtPointer) "True" },
    { (String) XtNmarkDame, (String) XtCMarkDame, XtRBoolean, sizeof(Boolean),
      offset(MarkDame), XtRString, (XtPointer) "True" },
    { (String) XtNautoScore, (String) XtCAutoScore, XtRBoolean, sizeof(Boolean),
      offset(AutoScore), XtRString, (XtPointer) "True" },
    { (String) XtNnumberKibitzes, (String) XtCNumberKibitzes, XtRBoolean, sizeof(Boolean),
      offset(NumberKibitzes), XtRString, (XtPointer) "True" },
    { (String) XtNdefaultTime, (String) XtCDefaultTime, XtRInt,
       sizeof(int), offset(DefaultTime), XtRString, (XtPointer) "10" },
    { (String) XtNdefaultByoYomi, (String) XtCDefaultByoYomi, XtRInt,
       sizeof(int), offset(DefaultByoYomi), XtRString, (XtPointer) "10" },
    { (String)XtNmyLowTimeBackground, (String)XtCMyLowTimeBackground, XtRPixel,
      sizeof(Pixel), offset(MyLowTimeBackground), XtRString,(XtPointer)"red" },
    { (String)XtNmyLowTimeForeground, (String)XtCMyLowTimeForeground, XtRPixel,
      sizeof(Pixel),offset(MyLowTimeForeground),XtRString,(XtPointer)"yellow"},
    { (String)XtNlowTimeBackground, (String)XtCLowTimeBackground, XtRPixel,
      sizeof(Pixel),offset(LowTimeBackground),XtRString,(XtPointer)"cornflowerblue" },
    { (String)XtNlowTimeForeground, (String)XtCLowTimeForeground , XtRPixel,
      sizeof(Pixel), offset(LowTimeForeground),XtRString,(XtPointer)"yellow"},
    { (String) XtNminSecPerMove, (String) XtCMinSecPerMove, XtRInt,
       sizeof(int), offset(MinSecPerMove), XtRString, (XtPointer) "5" },
    { (String) XtNlowTimeSet, (String) XtCLowTimeSet, XtRInt,
       sizeof(int), offset(LowTimeSet), XtRString, (XtPointer) "3" },
    { (String) XtNminLagMargin, (String) XtCMinLagMargin, XtRInt,
       sizeof(int), offset(MinLagMargin), XtRString, (XtPointer) "15" }
};
#undef offset

int main(int argc, char **argv)
{
    char    *ptr, Name[200];
    Widget   Buttons, Collect, AnalyzeButton, UserButton, Save, File;
    Widget   ReplyMessage;
    Widget   analyzeSize, programSettings, ResTree;
    Window   GroupWindow;
    time_t   Now;
    Cardinal i;
    Arg      args[3];

    ExceptionProgram = argv[0];
    global_argc      = argc;
    global_argv      = argv;
#ifdef HAVE_SOCKS
   SOCKSinit(argv[0]);
#endif /* HAVE_SOCKS */

    Now = StringToTime(__DATE__, __TIME__);
    if (Now) {
        ptr = Name;
        strftime(Name, sizeof(Name), "%Y-%m-%d %T", gmtime(&Now));
    } else ptr = (char *) __DATE__ " " __TIME__;
    CompileTime = mystrdup(ptr);

    i = 0;
    NameClassArg(args,
                 "Board", boardWidgetClass,
                 "SmeBell", smeBellObjectClass,
                 NULL); i++;
    toplevel = MyAppInitialize(&app_context, "XGospel",
                               options, XtNumber(options),
                               &argc, argv, fallback_resources, args, i,
                               UseMessages, XtNumber(UseMessages),
                               NULL);

    Main    = XtNameToWidget(toplevel, XtName(toplevel));
    if (!Main) Raise1(FatalException, "Could not find real toplevel");

    XtAppAddActions(app_context, actionTable, XtNumber(actionTable));
    XtGetApplicationResources(toplevel, &appdata, resources,
                              XtNumber(resources), NULL, 0);

    appdata.AutoReply = False;
    appdata.WantStdout = False;
    appdata.WantVerbose = False;
    DebugFun         = (appdata.DebugFun & DEBUGFUN)       != 0;
    DebugPending     = (appdata.DebugFun & DEBUGPENDING)   != 0;
    AbortOnException = (appdata.DebugFun & DEBUGABORTONEXCEPTION) ? -1 : 0;
    DebugRoundTrip   = (appdata.DebugFun & DEBUGROUNDTRIP) != 0;
    IgsYYdebug       = (appdata.DebugFun & DEBUGBISON)     != 0;
    IgsYY_flex_debug = (appdata.DebugFun & DEBUGFLEX)      != 0;
    SgfDateFormat = DateFormat   = MyPassword = MyName = NULL;
    sprintf(Name, "%.*s@%.*s",
            (int) sizeof(Name)/2-1, GetUserId(),
            (int) sizeof(Name)/2-1, GetHostname());
    UserId = mystrdup(Name);
    ServerName = mystrdup("IGS");
    ServerType = IGS;
    ServerVersion = ServerSubVersion = 0;

    if (appdata.AutoReplyMessage)
        appdata.AutoReplyMessage = mystrdup(appdata.AutoReplyMessage);
    if (appdata.DateFormat)
        DateFormat = mystrdup(appdata.DateFormat);
    if (appdata.SgfDateFormat)
        SgfDateFormat = mystrdup(appdata.SgfDateFormat);
    if (appdata.Password && *appdata.Password) {
        MyPassword = mystrdup(appdata.Password);
        ptr = strchr(MyPassword, ' ');
        if (ptr) *ptr = 0;
    }
    if (appdata.User && *appdata.User) {
        MyName = mystrdup(appdata.User);
        ptr = strchr(MyName, ' ');
        if (ptr) *ptr = 0;
    }
    if (appdata.MinSecPerMove  <  0) appdata.MinSecPerMove = 0;
    if (appdata.MinLagMargin   <  0) appdata.MinLagMargin   =  0;
    if (appdata.ReplayTimeout  <  1) appdata.ReplayTimeout  =  1;
    if (appdata.WhoTimeout     < 10 && appdata.WhoTimeout > 0) {
        appdata.WhoTimeout     = 10;
    }
    if (appdata.GamesTimeout   < 10 && appdata.GamesTimeout > 0) {
        appdata.GamesTimeout   = 10;
    }
    if (appdata.ReviewsTimeout < 10) appdata.ReviewsTimeout = 10;
    if (appdata.ClockTimeout   <  1) appdata.ClockTimeout   =  1;
    if (appdata.InactiveTimeout<  1) appdata.InactiveTimeout=  1;

    SetWidgetTitles(Main, MainTitle, (XtPointer) XtName(toplevel));

    DebugFile = NULL;
    if (appdata.DebugFile) {
        SubstitutionRec FileSubs[5];
        String          fn;

        i = 0;
        FileSubs[i].match = 'N';
        FileSubs[i].substitution = MyName ? MyName : (char *) "NoName";
        i++;

        FileSubs[i].match = 'P';
        FileSubs[i].substitution =
            MyPassword ? MyPassword : (char *) "NoPassword";
        i++;

        FileSubs[i].match = 'H';
        FileSubs[i].substitution = (String) GetHostname();
        i++;

        FileSubs[i].match = 'h';
        FileSubs[i].substitution = appdata.Site;
        i++;

        sprintf(Name, "%d", appdata.Port);
        FileSubs[i].match = 'p';
        FileSubs[i].substitution = Name;
        i++;

        fn = XtFindFile(appdata.DebugFile, FileSubs, i, TryToWrite);
        if (fn) XtFree(fn);
    }

    time(&Now);
    Buttons = MyNameToWidget(Main, "main");
    if (!Buttons) Raise1(FatalException, "Could not find main widget");

    Collect         = XtNameToWidget(Buttons, "*collect");
    GamesButton     = XtNameToWidget(Buttons, "*gamesButton");
    PlayersButton   = XtNameToWidget(Buttons, "*playersButton");
    ReviewsButton   = 0; /* XtNameToWidget(Buttons, "*reviewsButton"); ??? */
    UserButton      = XtNameToWidget(Buttons, "*userButton");
    ServerButton    = XtNameToWidget(Buttons, "*messageButton");
    BroadcastButton = XtNameToWidget(Buttons, "*broadcastButton");
    YellButton      = XtNameToWidget(Buttons, "*yellButton");
    AnalyzeButton   = XtNameToWidget(Buttons, "*analyzeButton");
    EventsButton    = XtNameToWidget(Buttons, "*eventsButton");
    ShortHelp       = XtNameToWidget(Buttons, "*shortHelp");
    Info            = XtNameToWidget(Buttons, "*info");
    Input           = XtNameToWidget(Buttons, "*input");
    OutputBeep      = XtNameToWidget(Buttons, "*beep");
    OutputRaise     = XtNameToWidget(Buttons, "*raise");
    ErrorBeep       = XtNameToWidget(Buttons, "*errorBeep");
    ErrorRaise      = XtNameToWidget(Buttons, "*errorRaise");
    Overwrite       = XtNameToWidget(Buttons, "*overwrite");

    HelpButton      = XtNameToWidget(Buttons, "*helpButton");
    if (HelpButton) XtAddCallback(HelpButton, XtNcallback, toggleHelp, NULL);

    Save = XtNameToWidget(Buttons, "*save");
    if (Save) XtAddCallback(Save, XtNcallback, SaveMain, NULL);
    File = XtNameToWidget(Buttons, "*file");
    if (File) XtAddCallback(File, XtNcallback, ChangeMainFilename,
                            (XtPointer) &MainFileName);

    ReplyMessage = XtNameToWidget(Buttons, "*replyMessage");
    if (ReplyMessage) XtAddCallback(ReplyMessage, XtNcallback,
	       ChangeAutoReplyMessage, (XtPointer) &appdata.AutoReplyMessage);

    AutoReply = XtNameToWidget(Buttons, "*autoReply");
    if (AutoReply) {
        CallReadToggle(AutoReply, &appdata.AutoReply, NULL);
        XtAddCallback(AutoReply, XtNcallback, CallReadToggle,
                      &appdata.AutoReply);
    }
    Stdout = XtNameToWidget(Buttons, "*stdout");
    if (Stdout) {
        CallReadToggle(Stdout, &appdata.WantStdout, NULL);
        XtAddCallback(Stdout, XtNcallback, CallReadToggle,
                      &appdata.WantStdout);
    }
    Verbose = XtNameToWidget(Buttons, "*verbose");
    if (Verbose) {
        CallReadToggle(Verbose, &appdata.WantVerbose, NULL);
        XtAddCallback(Verbose, XtNcallback, CallReadToggle,
                      &appdata.WantVerbose);
    }
    analyzeSize = XtNameToWidget(Buttons, "*analyzeSize");
    if (analyzeSize)
        XtAddCallback(analyzeSize, XtNcallback, ChangeAnalyzeSize, NULL);
    programSettings = XtNameToWidget(Buttons, "*programSettings");
    if (programSettings) XtAddCallback(programSettings, XtNcallback,
                                       ChangeSettings, (XtPointer) &appdata);
    MainFileName = StringToFilename(appdata.MainFilename,
                                    (int) 'T', "IGSsession",
                                    (int) 't', "_",
                                    0);
    XtAddCallback(Buttons, XtNdestroyCallback, CallFree, MainFileName);

    Localtime     = XtNameToWidget(Buttons, "*localTime");
    if (Localtime)     AddText(Localtime, DateString(localtime(&Now)));
    Universaltime = XtNameToWidget(Buttons, "*universalTime");
    if (Universaltime) AddText(Universaltime, DateString(gmtime(&Now)));
    Servertime    = XtNameToWidget(Buttons, "*serverTime");
    if (Servertime)    AddText(Servertime, "--- --- -- --:--:-- ----");

    XtSetKeyboardFocus(Collect, Input);
    MyRealizeWidget(Main);
    GroupWindow = XtWindow(Main);
    ResTree = XtNameToWidget(toplevel, "resourceTree");
    if (ResTree)
        XtVaSetValues(ResTree, XtNwindowGroup, (XtArgVal) GroupWindow, NULL);

/*
    XtAppAddInput(app_context, 0,
                  (XtPointer) XtInputReadMask, UserThroughPut, NULL);
*/
    InitGospel();
    InitTextBatch();
    InitConnect(toplevel);
    InitEvents(toplevel);
    InitUtils(toplevel);
    InitGames(toplevel);
    InitPlayers(toplevel);
    InitReviews(toplevel);
    InitBroadcast(toplevel);
    InitYell(toplevel);
    InitMessages(toplevel);
    InitObserve(toplevel);
    InitTell(toplevel);
    InitMatch(toplevel);
    InitStats(toplevel);

    InitHeartBeat(app_context);
    if (AnalyzeButton) XtAddCallback(AnalyzeButton,XtNcallback,TestBoard,NULL);
    if (UserButton) XtAddCallback(UserButton,XtNcallback,User_Commands,NULL);

    WITH_UNWIND {
        Conn = Connect(appdata.Site, appdata.Port);
        RealQuit = 0;
        Entered  = 0;
        do {
            IgsYYparse();
	    /* Return here either for a closed connexion (IgsInput returns 0)
	     * or for a bad IGS password.
	     */
            _IgsRestartParse();
        } while (!RealQuit);
    } ON_UNWIND {
        CleanTell();
        CleanObserve();
        CleanYell();
        CleanBroadcast();
        CleanReviews();
        CleanPlayers();
        CleanGames();
        CleanEvents();
        CleanConnect();
        CleanTextBatch();
        CleanGospel();
        if (DebugFile) {
            fflush(DebugFile);
            fclose(DebugFile);
        }
        if (appdata.DebugFile != False) fflush(stdout);
        XtDestroyWidget(toplevel);
        myfree(SgfDateFormat);
        myfree(DateFormat);
        myfree(UserId);
        myfree(MyName);
        myfree(MyPassword);
        /*      XtDestroyApplicationContext(app_context); */
    } END_UNWIND;
    mallocstats();
    return 0;
}
