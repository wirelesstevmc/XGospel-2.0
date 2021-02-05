#include <X11/StringDefs.h>
#include <X11/Intrinsic.h>
#include <X11/Xaw/AsciiText.h>
#include <X11/Xaw/Toggle.h>

#include <string.h>
#include <time.h>
#if !STDC_HEADERS && HAVE_MEMORY_H
# include <memory.h>
#endif /* not STDC_HEADERS and HAVE_MEMORY_H */

#include <YShell.h>
#include <mymalloc.h>
#include <except.h>
#include <myxlib.h>

#include "connect.h"
#include "events.h"
#include "games.h"
#include "gointer.h"
#include "match.h"
#include "players.h"
#include "stats.h"
#include "tell.h"
#include "utils.h"
#include "xgospel.h"

#ifdef    HAVE_NO_MEMCHR_PROTO
extern void *memchr(const void *s, int c, size_t n);
#endif /* HAVE_NO_MEMCHR_PROTO */

extern Tell   **_PlayerTellAddress(Player *player);

struct _Tell {
    char         *FileName;
    Player       *Player;
    Widget        Root, Info, Input, MessageBeep, BeepBeep, ErrorBeep;
    Widget        MessageRaise, BeepRaise, ErrorRaise;
    Widget        Overwrite, Challenge;
};

static RegexYesNo TellYesNo;
static MyContext  Context;
static XrmQuark   TellQuark;

static void SendTell(Widget w, XEvent *event, String *string, Cardinal *n);
static XtActionsRec actionTable[] = {
    { (String) "tell",        SendTell   },
};

static void ChangeTellFilename(Widget w,
                               XtPointer clientdata, XtPointer calldata)
{
    char **Filename;
    Widget Root;

    Filename = (char **) clientdata;
    Root = ChangeFilename(toplevel, "tellFilename",
                          "Enter name of tell file",
                          Filename, "filename", *Filename, NULL);
    MyDependsOn(Root, w);
}

static void SaveTell(Widget w, XtPointer clientdata, XtPointer calldata)
{
    Boolean                   Overwrite;
    static char const * const ErrorType = "Tell save error";

    Tell *tell;

    tell = (Tell *) clientdata;
    if (tell->Info) {
        w = tell->Overwrite;
        if (w) XtVaGetValues(w, XtNstate, (XtArgVal) &Overwrite, NULL);
        else   Overwrite = False;

        SaveWrite(tell->FileName, Overwrite,
                  tell->ErrorBeep, tell->ErrorRaise, ErrorType,
                  SaveTextFun, (XtPointer) tell->Info);
    } else {
        IfBell(tell->ErrorBeep);
        IfRaise(tell->ErrorRaise, tell->ErrorRaise);
        PopMessage(ErrorType, "Messages were ignored so there is "
                   "nothing to be saved");
    }
}

void InitTell(Widget Toplevel)
{
    const char *Error;

    XtAppAddActions(XtWidgetToApplicationContext(Toplevel),
                    actionTable, XtNumber(actionTable));
    Context = YShellContext(Toplevel);
    TellQuark = XrmPermStringToQuark("tell");

    EmptyRegexYesNo(&TellYesNo);
    Error = CompileRegexYesNo(appdata.TellPass, appdata.TellKill, &TellYesNo);
    if (Error)
        Warning("failed to compile tell regex pattern: %s\n", Error);
}

void CleanTell(void)
{
    FreeRegexYesNo(&TellYesNo);
}

static void ChangeTellPass(Widget w, XtPointer clientdata, XtPointer calldata)
{
    Tell *tell;

    tell = (Tell *) clientdata;
    EditStringList(tell->Root, "Edit tell pass pattern",
                   "Invalid pass pattern", &appdata.TellPass,&appdata.TellKill,
                   &TellYesNo, 0);
}

static void ChangeTellKill(Widget w, XtPointer clientdata, XtPointer calldata)
{
    Tell *tell;

    tell = (Tell *) clientdata;
    EditStringList(tell->Root, "Edit tell kill pattern",
                   "Invalid kill pattern", &appdata.TellPass,&appdata.TellKill,
                   &TellYesNo, 1);
}

static void CallDestroyTell(Widget w, XtPointer clientdata, XtPointer calldata)
{
    Tell *tell, **Ref;
    Player *player;

    player = (Player *) clientdata;
    Ref    =  _PlayerTellAddress(player);
    tell   = *Ref;

    if (tell->Input) MyDeleteContext(Context, tell->Input, TellQuark);
    myfree(tell->FileName);
    myfree(tell);
   *Ref = NULL;
}

static void SendBeep(Widget w, XtPointer clientdata, XtPointer calldata)
{
    const Player *player;

    player = (const Player *) clientdata;
    SendCommand(NULL, NULL, "beep %s", PlayerToName(player));
}

void SetTellDescription(Tell *tell)
{
    char *Name;

    SetPlayerTitles(tell->Root, tell->Player);
    Name = StringToFilename(appdata.TellFilename,
                            (int) 'T', "tell",
                            (int) 't', "_",
                            (int) 'N', PlayerToName(tell->Player),
                            (int) 'n', PlayerToStrength(tell->Player),
                            0);
    myfree(tell->FileName);
    tell->FileName = Name;
}

void TellMessage(Tell *tell, const char *Message)
{
    if (tell->Info) BatchAddText(tell->Info, ".....     : %s\n", Message);
    IfBell(tell->MessageBeep);
    IfRaise(tell->MessageRaise, tell->MessageRaise);
}

static Widget CreateTell(XtPointer Closure);
/* Failure behaviour is incorrect -Ton */
static Tell *FindTell(Player *player)
{
    Tell       *tell, **Ref;
    Widget      Root, Collect, Input, Pass, Kill;
    Widget      Beep, File, Save, stats, playerObserve;
    WidgetList Children;
    Cardinal   i, NrChildren;

    Ref    = _PlayerTellAddress(player);
    if (*Ref) return *Ref;

    tell = mynew(Tell);
    tell->FileName = NULL;
    tell->Input    = 0;
    tell->Player   = player;

    WITH_HANDLING {
        Root = MyVaCreateManagedWidget("tell", toplevel, NULL);
        XtAddCallback(Root, XtNdestroyCallback, CallDestroyTell,
                      (XtPointer) player);
        tell->Root = Root;
        *Ref = tell;
    } ON_EXCEPTION {
        myfree(tell);
    } END_HANDLING;
    WITH_HANDLING {
        SetTellDescription(tell);
 
        Beep         = XtNameToWidget(Root, "*bug");
        if (Beep)  XtAddCallback(Beep, XtNcallback, SendBeep,
                                 (XtPointer) player);

        stats        = XtNameToWidget(Root, "*getStats");
        if (stats) XtAddCallback(stats,XtNcallback, CallGetStats,
                                 (XtPointer) player);

        playerObserve  = XtNameToWidget(Root, "*playerObserve");
        if (playerObserve) XtAddCallback(playerObserve, XtNcallback,
					 CallPlayerObserve,
					 (XtPointer) player);

        File         = XtNameToWidget(Root, "*file");
        if (File) XtAddCallback(File, XtNcallback, ChangeTellFilename,
                                (XtPointer) &tell->FileName);

        Save         = XtNameToWidget(Root, "*save");
        if (Save) XtAddCallback(Save, XtNcallback, SaveTell, (XtPointer) tell);

        Pass = XtNameToWidget(Root, "*passPattern");
        if (Pass) XtAddCallback(Pass, XtNcallback, ChangeTellPass,
                                (XtPointer) tell);
        Kill = XtNameToWidget(Root, "*killPattern");
        if (Kill) XtAddCallback(Kill, XtNcallback, ChangeTellKill,
                                   (XtPointer) tell);

        Input        = XtNameToWidget(Root, "*input");
        if (Input) {
	    /* allow finding player from the Input widget: */
            MySaveContext(Context, Input, TellQuark, (XtPointer) player);
	}
        tell->Info         = XtNameToWidget(Root, "*info");
        tell->Input        = Input;
        tell->MessageBeep  = XtNameToWidget(Root, "*beep");
        tell->MessageRaise = XtNameToWidget(Root, "*raise");
        tell->BeepBeep     = XtNameToWidget(Root, "*beepBeep");
        tell->BeepRaise    = XtNameToWidget(Root, "*beepRaise");
        tell->ErrorBeep    = XtNameToWidget(Root, "*errorBeep");
        tell->ErrorRaise   = XtNameToWidget(Root, "*errorRaise");
        tell->Overwrite    = XtNameToWidget(Root, "*overwrite");
        tell->Challenge    = XtNameToWidget(Root, "*getChallenge");

        Collect      = XtNameToWidget(Root, "*collect");
        if (Collect && Input) XtSetKeyboardFocus(Collect, Input);
        /* must not popup now, done later in CallCreateCouple */
	MyRealizeWidgetNoPopup(Root);
    } ON_EXCEPTION {
        XtDestroyWidget(Root);
    } END_HANDLING;

    if (tell->Challenge) CoupleChallenge(player, tell->Challenge);
    CoupleChallengeTell(player, tell->Root, CreateTell);
    return tell;
}

void CallGetTell(Widget w, XtPointer clientdata, XtPointer calldata)
{
    XtPopup(FindTell((Player *) clientdata)->Root, XtGrabNone);
}

static Widget CreateTell(XtPointer Closure)
{
    Tell *tell;

    tell = FindTell((Player *) Closure);
    return tell->Root;
}

void CoupleTell(Player *player, Widget Toggle)
{
    Tell *tell;
    Widget Root;

    tell = *_PlayerTellAddress(player);
    if (tell) {
        Root = tell->Root;
        XtVaSetValues(Toggle, XtNstate, (XtArgVal) MyIsPopped(Root), NULL);
    } else Root = NULL;
    CoupleToggleWidget(Root, Toggle, CreateTell, (XtPointer) player);
}

void CoupleTellChallenge(Player *player, Widget Root,
                         Widget (*InitFun)(XtPointer Closure))
{
    Tell *tell;

    tell = *_PlayerTellAddress(player);
    if (tell && tell->Challenge)
        CoupleToggleWidget(Root, tell->Challenge, InitFun, (XtPointer) player);
}

static void RealSendTell(const char *Text, int Length, Widget ShowInfo,
                         const char *Name, const char *myName,
                         int UseSay, const Player *Opponent)
{
    if (UseSay) UserSendCommand(NULL, NULL, "say %.*s", Length, Text);
    else UserSendCommand(NULL, NULL, "tell %s %.*s", Name, Length, Text);
    if (Opponent) AddMyGameComment("%10s says: %.*s", myName, Length, Text);
    if (ShowInfo) BatchAddText(ShowInfo, "%10s: %.*s\n", myName, Length, Text);
}

static void SendTell(Widget w, XEvent *event, String *string, Cardinal *n)
{
    XtPointer     Data;
    Tell         *tell;
    const Player *player, *Opponent;
    String        Buffer;
    const char   *Name, *myName, *From, *To, *End;
    int           UseSay;
    Widget        ShowInfo;

    if (MyFindContext(Context, w, TellQuark, &Data))
        WidgetWarning(w, "tell() could not get context");
    else {
        player = (const Player *) Data;
        Name   = PlayerToName(player);
        myName = PlayerToName(Me);
        tell = *_PlayerTellAddress((Player *) player);
        UseSay = SayP(player);
        Opponent = MyOpponent(player);
        if (player != Me) ShowInfo = tell->Info;
        else              ShowInfo = 0;
        XtVaGetValues(w, XtNstring, &Buffer, NULL);
        End = strchr(Buffer, 0);
        for (From = Buffer;
             (To = memchr((char *)From, '\n', (size_t)(End-From))) != NULL;
             From = To+1)
            RealSendTell(From, To-From,
                         ShowInfo, Name, myName, UseSay, Opponent);
        RealSendTell(From, End-From,
                     ShowInfo, Name, myName, UseSay, Opponent);
        XtVaSetValues(w, XtNstring, "", NULL);
        return;
    }
}

void ExplicitTell(char *tellCommand)
/* Popup a tell window if user explicitly types "tell player ..." */
{
    Player *player;
    char   *msg;
    Tell   *tell;

    while (*tellCommand == ' ') tellCommand++;
    msg = strchr(tellCommand, ' ');
    if (msg) {
        Widget ShowInfo;
	player = LengthNameToPlayer(tellCommand, msg - tellCommand);
	msg++; /* skip space */
	tell = FindTell(player);
        ShowInfo = player != Me ? tell->Info : NULL;
        RealSendTell(msg, strlen(msg), ShowInfo, PlayerToName(player),
		     PlayerToName(Me), SayP(player), MyOpponent(player));
    } else {
	player = NameToPlayer(tellCommand);
	tell = FindTell(player);
    }
    IfRaise(tell->MessageRaise, tell->MessageRaise);
}

void NoTell(void)
{
    const char *Name, *Message;
    Tell       *tell;
    Player     *player;

    Name = StripArgsCommand(NULL, "tell");
    if (Name && *Name) {
        Message = strchr(Name, ' ');
        if (Message) *(char *) Message++ = 0;
        else Message = "";
        player = PlayerFromName(Name);
        if (!GotNoTell(player, Message)) {
            tell = FindTell(player);
            if (tell->Info)
                BatchAddText(tell->Info,
                             ".....     : Tell not sent, player is gone\n");
            IfBell(tell->ErrorBeep);
            IfRaise(tell->ErrorBeep, tell->ErrorBeep);
        }
    } else Warning("Recipient of tell message is gone\n");
}

void ReceivedTell(Player *player, const char *Message)
{
    Tell   *tell;
    char    Buffer[500];
    int     Yes, No;
    const char *Error;
    char now[6];

    if (!GotTell(player, Message)) {

	if (appdata.AutoReply) {
	    tell = *_PlayerTellAddress(player);
	    if (appdata.AutoReplyMessage && *appdata.AutoReplyMessage &&
		player != Me) {
		if (SayP(player)) {
		    ForceCommand(NULL, "say %s", appdata.AutoReplyMessage);
		} else {
		    ForceCommand(NULL, "tell %s %s", PlayerToName(player),
				 appdata.AutoReplyMessage);
		}
	    }
	    strncpy(now, strchr(asctime(&LocalTime), ':')-2, 5); /* hh:mm */
	    now[5] = '\0';
	    sprintf(Buffer, "%16s(%s): %s", PlayerString(player), now,
		    Message);
	    if (!tell) {
		Outputf("#%s\n", Buffer);
		return;
	    }
	} else {
	    sprintf(Buffer, "%16s: %s", PlayerString(player), Message);
	}
        Error = TestRegexYesNo(Buffer, &TellYesNo, &Yes, &No);
        if (Error) {
            tell = FindTell(player);
            if (tell->Info) BatchAddText(tell->Info,".....     : %s\n", Error);
            IfBell(tell->ErrorBeep);
            IfRaise(tell->ErrorRaise, tell->ErrorRaise);
        } else if (Yes && !No) {
            tell = FindTell(player);
            if (tell->Info) {
		if (appdata.AutoReply) {
		    BatchAddText(tell->Info, "%10s(%s)# %s\n",
			     PlayerToName(player), now, Message);
		} else {
		    BatchAddText(tell->Info, "%10s: %s\n",
			     PlayerToName(player), Message);
		}
	    }
            IfBell(tell->MessageBeep);
            IfRaise(tell->MessageRaise, tell->MessageRaise);
        }
    }
}

void Idle(Player *player, const char *time)
{
    Tell   *tell;
    char    Buffer[500];
    const char *Error;

    sprintf(Buffer, "%s has been idle for %s", PlayerToName(player), time);

    tell = *_PlayerTellAddress(player);
    if (!tell) {
	Outputf("%s\n", Buffer);
	return;
    }
    if (tell->Info) BatchAddText(tell->Info,".....     : %s\n", Buffer);
}

void Beeping(Player *player)
{
    Tell    *tell;

    if (appdata.AutoReply) {
	ReceivedTell(player, "You are beeped");
	return;
    }
    tell = FindTell(player);
    if (tell->Info) BatchAddText(tell->Info, ".....     : You are beeped\n");
    IfBell(tell->BeepBeep);
    IfRaise(tell->BeepRaise, tell->BeepRaise);
}
