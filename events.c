/* 
   This is meant to become a programmable interface. At the moment I just use
   it to notice when a client user logs on
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <myxlib.h>
#include <mymalloc.h>
#include <except.h>

#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <X11/Xaw/Toggle.h>

#include "connect.h"
#include "events.h"
#include "utils.h"
#include "version.h"
#include "xgospel.h"

#define FREENULL        ((FreeFun *) 0)
#define LOGONWAIT         15000

#define BEGINESCAPE       "\\{"
#define ENDESCAPE         "} "
#define XGOSPEL           "xgospel"

#define XGOSPELESCAPE BEGINESCAPE XGOSPEL ENDESCAPE

extern      XtAppContext  app_context;
extern int *_PlayerXgospelUser(Player *player);

extern struct tm LocalTime;

struct _Action {
    Action          Next, Previous;
    Event           Event;
    int             RefCount;
    unsigned long   Delay;
    ActionFunction *Fun;
    XtPointer       Closure;
    FreeFun        *FreeClosure;
};

struct _Event {
    Event          Next, Previous;
    int            RefCount;
    EventFunPtr         Fun;
    XtPointer      Match;
    FreeFun       *FreeMatch;
    struct _Action Actions;
};

static struct _Event Events = {&Events, &Events};

static Widget EventsRoot, EventsOverwrite, EventsInfo;
static Widget EventsBeep, EventsRaise, EventsErrorBeep, EventsErrorRaise;
static char  *EventsFileName, *Version;

Widget EventsButton;

int maintainerRating = 26;
/* used to get my exact rating with "proba". Default is 6k 26, to allow
 * h9 with 4d and 15k.
 */

static void FreeEvent(Event event)
{
    if (--event->RefCount) return;
    if (event->FreeMatch) (*event->FreeMatch)(event->Match);
    myfree(event);
}

static void FreeAction(Action action)
{
    if (--action->RefCount) return;
    if (action->FreeClosure) (*action->FreeClosure)(action->Closure);
    FreeEvent(action->Event);
    myfree(action);
}

void DeleteAction(Action action)
{
    action->Next->Previous = action->Previous;
    action->Previous->Next = action->Next;
    FreeAction(action);
}

void DeleteEvent(Event event)
{
    event->Next->Previous = event->Previous;
    event->Previous->Next = event->Next;
    while (event->Actions.Previous != &event->Actions)
        DeleteAction(event->Actions.Previous);
    FreeEvent(event);
}

Action AddAction(Event event, unsigned long Delay, ActionFunction *Fun,
                 XtPointer Closure, FreeFun *FreeClosure)
{
    Action action;

    action = mynew(struct _Action);

    action->Next  = &event->Actions;
    action->Previous = event->Actions.Previous;
    action->Previous->Next = action->Next->Previous = action;

    action->RefCount++;
    action->Event = event;
    event->RefCount++;

    action->Delay       = Delay;
    action->Fun         = Fun;
    action->Closure     = Closure;
    action->FreeClosure = FreeClosure;
    return action;
}

Event AddEvent(EventFunPtr Fun, XtPointer Match, FreeFun *FreeMatch)
{
    Event event;

    event = mynew(struct _Event);

    event->Next = &Events;
    event->Previous = Events.Previous;
    event->Previous->Next = event->Next->Previous = event;
    event->Actions.Next = event->Actions.Previous = &event->Actions;

    event->RefCount  = 1;

    event->Fun       = Fun;
    event->Match     = Match;
    event->FreeMatch = FreeMatch;
    return event;
}

typedef struct {
    Action    Action;
    EventFunPtr    Fun;
    XtPointer Data;
} LocalData;

static void DelayedAction(XtPointer Closure, XtIntervalId *id)
{
    LocalData *localdata;
    Action     action;
    Event      event;

    localdata = (LocalData *) Closure;
    action = localdata->Action;
    event  = action->Event;
    (*action->Fun)(action, localdata->Fun, localdata->Data,
                   event->Match, action->Closure);
    FreeAction(action);
    myfree(localdata);
}

static int MatchEvent(EventFunPtr Fun, int (*MatchFun)(XtPointer, XtPointer),
                      XtPointer Data, size_t Size)
{
    Event      event, next;
    Action     action, nextAction;
    LocalData *localdata;
    int        Found;

    Found = 0;
    for (event = Events.Next; event != &Events; event = next) {
        next = event->Next; /* In case a selfremove happens */
        if (event->Fun == Fun && (*MatchFun)(event->Match, Data)) {
            Found = 1;
            event->RefCount++; /* To delay selfremove */
            for (action = event->Actions.Next;
                 action != &event->Actions;
                 action = nextAction) {
                nextAction = action->Next;
                if (action->Delay) {
                    localdata = (LocalData *) mymalloc(sizeof(LocalData)+Size);
                    localdata->Action = action;
                    action->RefCount++;
                    localdata->Fun    = Fun;
                    localdata->Data = (XtPointer) &localdata[1];
                    memcpy(localdata->Data, Data, Size);
                    XtAppAddTimeOut(app_context, action->Delay, DelayedAction,
                                    (XtPointer) localdata);
                } else (*action->Fun)(action, Fun, Data,
                                      event->Match, action->Closure);
            }
            FreeEvent(event);
        }
    }
    return Found;
}
    
static int MatchPlayer(XtPointer Pattern, XtPointer player)
{
    const char *Name, *Pat;

    Name = PlayerToName(*(Player **)player);
    Pat  = (const char *) Pattern;
    return (Pat[0] == '*' && Pat[1] == 0) || 0 == strcmp(Pat, Name);
}

int Logon(const Player *player)
{
    return MatchEvent(Logon, MatchPlayer, (XtPointer) &player, sizeof(player));
}

int Logoff(const Player *player)
{
    return MatchEvent(Logoff,MatchPlayer, (XtPointer) &player, sizeof(player));
}

typedef struct {
    Player *player;
    size_t  Length;
    char    Message[1];
} PlayerMessage;

static int MatchTell(XtPointer Pattern, XtPointer Comm)
{
    PlayerMessage *Communication;

    Communication = (PlayerMessage *) Comm;
    return 0 == memcmp(Communication->Message, BEGINESCAPE, 2);
}

int GotTell(Player *player, const char *Message)
{
    PlayerMessage *Communication;
    int            rc;
    size_t         Size, Length;

    Length = strlen(Message);
    Size   = sizeof(PlayerMessage)+Length;
    Communication = (PlayerMessage *) mymalloc(Size);
    WITH_UNWIND {
        Communication->player  = player;
        Communication->Length  = Length;
        memcpy(Communication->Message, Message, Length+1);
        rc = MatchEvent(GotTell, MatchTell, (XtPointer) Communication, Size);
    } ON_UNWIND {
        myfree(Communication);
    } END_UNWIND;
    return rc;
}

int GotNoTell(Player *player, const char *Message)
{
    PlayerMessage *Communication;
    int            rc;
    size_t         Size, Length;

    Length = strlen(Message);
    Size   = sizeof(PlayerMessage)+Length;
    Communication = (PlayerMessage *) mymalloc(Size);
    WITH_UNWIND {
        Communication->player  = player;
        Communication->Length  = Length;
        memcpy(Communication->Message, Message, Length+1);
        rc = MatchEvent(GotNoTell, MatchTell, (XtPointer) Communication, Size);
    } ON_UNWIND {
        myfree(Communication);
    } END_UNWIND;
    return rc;
}

typedef struct {
    const Player  *player;
    NameVal       *stats;
} PlayerStats;

static int MatchStats(XtPointer Pattern, XtPointer stats)
{
    PlayerStats *Stats;

    Stats = (PlayerStats *) stats;
    return 0 == strcmp((char *) Pattern, PlayerToName(Stats->player));
}

/* Notice: We cannot currently handle delays !! -Ton */
/* As soon as we copy, don't cast away const !! -Ton */
int GotStats(const Player *player, const NameVal *stats)
{
    PlayerStats Stats;

    Stats.player = player;
    Stats.stats  = (NameVal *) stats;
    return MatchEvent(GotStats, MatchStats, (XtPointer) &Stats, sizeof(Stats));
}

int EnterServer(const Player *player)
{
    return MatchEvent(EnterServer, MatchPlayer,
                      (XtPointer) &player, sizeof(player)); 
}

void CleanEvents(void)
{
    while (Events.Previous != &Events) DeleteEvent(Events.Previous);
    myfree(Version);
}

static void SelfRemove(Action action, EventFunPtr Fun, XtPointer FunArgs,
                       XtPointer Match, XtPointer Closure)
{
    DeleteEvent(action->Event);
}

static void XgospelStats(Action action, EventFunPtr Fun, XtPointer FunArgs,
                         XtPointer Match, XtPointer Closure)
{
    PlayerStats *Stats;
    NameVal     *Vals, *Val;
    const char  *Ptr, *Last;
    char        *Tmp;

    Stats = (PlayerStats *) FunArgs;
    Vals  = Stats->stats;
    for (Val = Vals->Next; Val != Vals; Val = Val->Next) {
        if (!strcmp(Val->Name, "Rank")) {
	    Ptr = strrchr(Val->Value, ' ');
	    if (Ptr) {
		maintainerRating = atoi(Ptr+1);
	    }
	}
        if (!strcmp(Val->Name, "Info")) {
            Last = 0;
            for (Ptr = Val->Value; (Ptr = strchr(Ptr, 'x')) != NULL; Ptr++)
                if (0 == memcmp(Ptr, "xgospel: ", 9) ||
                    /* ; kludge as long as long as info can't take : -Ton */
                    0 == memcmp(Ptr, "xgospel; ", 9)) Last = Ptr+9;
            if (Last && EventsInfo) {
                if (strcmp(Version, Last)) {
		    AddText(EventsInfo, "xgospel: your version is %s, "
                        "the most recent official version is %s\n",
			 VERSION, Last);
		    if (strncmp(Version, Last, 5)) {
			AddText(EventsInfo, appdata.VersionMessage,Last, Last);
			XtPopup(EventsRoot, XtGrabNone);
		    } else {
			IfRaise(EventsRaise, EventsRaise);
		    }
                    Tmp = mystrdup(Last);
                    myfree(Version);
                    Version = Tmp;
                } else {
		    AddText(EventsInfo,
			    "xgospel: your version is the latest version %s\n",
			    Last);
		    IfRaise(EventsRaise, EventsRaise);
		}
                IfBell(EventsBeep);
            }
            break;
        }
    }
}

static void XgospelCheck(Action action, EventFunPtr Fun, XtPointer FunArgs,
                         XtPointer Match, XtPointer Closure)
{
    const char   *name;
    Event         event;

    name   = appdata.Maintainer;

    if (name && *name) {
        SharedLastCommand(NULL, "stats %s", name);
        event = AddEvent(GotStats, (XtPointer) name, FREENULL);
        AddAction(event, 0, XgospelStats, NULL, FREENULL);
        AddAction(event, 0, SelfRemove, NULL, FREENULL);
    }
}

static void XgospelReport(Action action, EventFunPtr Fun, XtPointer FunArgs,
                          XtPointer Match, XtPointer Closure)
{
    Player *player;
    const char *name;

    player = *(Player **) FunArgs;
    name = PlayerToName(player);

    if (OnServerP(player))
        SharedLastCommand(NULL, "tell %s %sVERSION %-5s %s (%s)",
                          name, XGOSPELESCAPE, VERSION, CompileTime, UserId);
}

static void XgospelUnmessage(Action action, EventFunPtr Fun, XtPointer FunArgs,
                             XtPointer Match, XtPointer Closure)
{
    Player *player;
    int    *User;

    player = *(Player **) FunArgs;
    User = _PlayerXgospelUser(player);
    *User = EXXGOSPELUSER;

    if (EventsInfo)
        AddText(EventsInfo, "%16s: Xgospel user gone\n", PlayerString(player));
    IfBell(EventsBeep);
    IfRaise(EventsRaise, EventsRaise);
}

static int XgospelMessage(Player *player, const char *Message)
{
    Event   event;
    int    *User;
    size_t  Length;
    char    Buffer[200];
    Player *NewUser;
    const char *ptr, *Name;

  retry:
    if        (strncmp(Message, "VERSION ", 8) == 0) {
        Message += 8;
        NewUser  = player;
    } else if (strncmp(Message, "USER ",    5) == 0) {
        /* We maybe should only accept this from the maintainer, but let's be
           trusting for the moment --Ton */
        Message += 5;
        ptr = strchr(Message, ' ');
        if (ptr) {
            Length = ptr - Message;
            if (Length >= sizeof(Buffer)) Length = sizeof(Buffer)-1;
            memcpy(Buffer, Message, Length);
            Buffer[Length] = 0;
            Message = ptr+1;
            while (*Message == ' ') Message++;
            ptr = Buffer;
        } else {
            Message = "VERSION ????";
            ptr = Message;
        }
        player = NameToPlayer(ptr);
        if (!player) return 1;
        goto retry;
    } else {
        if (EventsInfo) AddText(EventsInfo, "%16s: Xgospel VERSION %s\n",
                                PlayerString(player), Message);
        return 0;
    }
    Name = PlayerToName(NewUser);
    User = _PlayerXgospelUser(NewUser);

    if (EventsInfo) AddText(EventsInfo, "%16s: Xgospel VERSION %s\n",
                            PlayerString(NewUser), Message);
    IfBell(EventsBeep);
    IfRaise(EventsRaise, EventsRaise);

    if (appdata.Maintainer && *appdata.Maintainer &&
        strcmp(PlayerToName(Me), appdata.Maintainer) == 0) {
/*
        sprintf(Buffer, XGOSPELESCAPE "USER %s VERSION %.160s", Name, Message);
        TellXgospelUsers(NULL, Buffer);
*/
        NewXgospelUser(NULL, NewUser);
    }

    if (*User != XGOSPELUSER) {
        event = AddEvent(Logoff, (XtPointer) Name, FREENULL);
        AddAction(event, 0, XgospelUnmessage, NULL, FREENULL);
        AddAction(event, 0, SelfRemove, NULL, FREENULL);
    }

    *User = XGOSPELUSER;
    return 0;
}

static void GetEscaped(Action action, EventFunPtr Fun, XtPointer FunArgs,
                       XtPointer Match, XtPointer Closure)
{
    PlayerMessage *Communication;
    const char    *Begin, *End;
    size_t         Length, BegLen, EndLen, Len;

    Communication = (PlayerMessage *) FunArgs;

    Begin = Communication->Message;
    BegLen = strlen(BEGINESCAPE);
    if (memcmp(Begin, BEGINESCAPE, BegLen)) goto error;
    Begin += BegLen;

    End = strchr(Communication->Message, ENDESCAPE[0]);
    EndLen = strlen(ENDESCAPE);
    if (memcmp(End, ENDESCAPE, EndLen)) goto error;
    Length = End-Begin;
    End += EndLen;
    
    Len = strlen(XGOSPEL);
    if (Length == Len && 0 == memcmp(Begin, XGOSPEL, Length)) {
        if (XgospelMessage(Communication->player, End)) goto error;
    } else goto error;
    return;

  error:
    Warning("invalid protocol message from %s: %s\n",
            PlayerToName(Communication->player), Communication->Message);
}

static void ChangeEventsFilename(Widget w,
                                 XtPointer clientdata, XtPointer calldata)
{
    char **Filename;
    Widget Root;

    Filename = (char **) clientdata;
    Root = ChangeFilename(toplevel, "eventsFilename",
                          "Enter name of events file",
                          Filename, "filename", *Filename, NULL);
    MyDependsOn(Root, w);
}

static void SaveEvents(Widget w, XtPointer clientdata, XtPointer calldata)
{
    Boolean                   Overwrite;
    static char const * const ErrorType = "Events save error";

    if (EventsInfo) {
        if (EventsOverwrite) XtVaGetValues(EventsOverwrite, XtNstate,
                                           (XtArgVal) &Overwrite, NULL);
        else Overwrite = False;

        SaveWrite(EventsFileName, Overwrite,
                  EventsErrorBeep, EventsErrorRaise, ErrorType,
                  SaveTextFun, (XtPointer) EventsInfo);
    } else {
        IfBell(EventsErrorBeep);
        IfRaise(EventsErrorRaise, EventsErrorRaise);
        PopMessage(ErrorType, "Events were ignored so there is "
                   "nothing to be saved");
    }
}

void InitEvents(Widget TopLevel)
{
    Event event;
    Widget EventsCollect, EventsSave, EventsFile;

    CleanEvents();
    Version = mystrdup(appdata.Version);
    if (appdata.Maintainer && *appdata.Maintainer) {
        event = AddEvent(Logon, appdata.Maintainer, FREENULL);
        AddAction(event, LOGONWAIT, XgospelReport, NULL, FREENULL);
        event = AddEvent(GotTell, NULL, FREENULL);
        AddAction(event, 0, GetEscaped, NULL, FREENULL);
        event = AddEvent(EnterServer, (XtPointer) "*", FREENULL);
        AddAction(event, LOGONWAIT, XgospelCheck, NULL, FREENULL);
        AddEvent(GotNoTell, NULL, FREENULL); /* Just to stop message */

        EventsRoot = MyVaCreateManagedWidget("events", TopLevel, NULL);

        EventsCollect    = XtNameToWidget(EventsRoot, "*collect");
        EventsBeep       = XtNameToWidget(EventsRoot, "*beep");
        EventsRaise      = XtNameToWidget(EventsRoot, "*raise");
        EventsInfo       = XtNameToWidget(EventsRoot, "*info");
        EventsFile       = XtNameToWidget(EventsRoot, "*file");
        EventsSave       = XtNameToWidget(EventsRoot, "*save");
        EventsOverwrite  = XtNameToWidget(EventsRoot, "*overwrite");
        EventsErrorBeep  = XtNameToWidget(EventsRoot, "*errorBeep");
        EventsErrorRaise = XtNameToWidget(EventsRoot, "*errorRaise");

        if (EventsSave)
            XtAddCallback(EventsSave, XtNcallback, SaveEvents, NULL);
        if (EventsFile)
            XtAddCallback(EventsFile, XtNcallback, ChangeEventsFilename,
                          (XtPointer) &EventsFileName);

        EventsFileName = StringToFilename(appdata.EventsFilename,
                                          'T', "Events", 't', "_", 0);
                                      
        XtAddCallback(EventsRoot, XtNdestroyCallback, CallFree,
                      (XtPointer) EventsFileName);
        XtAddCallback(EventsButton, XtNcallback,         CallToggleUpDown,
                      (XtPointer) EventsRoot);
        XtAddCallback(EventsRoot,   XtNpopupCallback,    CallToggleOn,
                      (XtPointer) EventsButton);
        XtAddCallback(EventsRoot,   XtNpopdownCallback,  CallToggleOff,
                      (XtPointer) EventsButton);
        XtRealizeWidget(EventsRoot);
        XtInstallAllAccelerators(EventsCollect, EventsCollect);
        DeleteProtocol(EventsRoot);
        CallToggleUpDown(EventsButton, (XtPointer) EventsRoot, NULL);
    } else
        EventsRoot = EventsInfo = EventsBeep = EventsRaise =
            EventsErrorBeep = EventsErrorRaise = 0;
}
