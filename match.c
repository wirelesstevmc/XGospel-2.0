#include <string.h>
#include <stdlib.h>

#include <myxlib.h>
#include <mymalloc.h>
#include <except.h>
#include <YShell.h>

#include <X11/StringDefs.h>
#include <X11/Xaw/Toggle.h>
#include <X11/Xaw/AsciiText.h>

#include "match.h"
#include "players.h"
#include "stats.h"
#include "tell.h"
#include "gospel.h"
#include "utils.h"
#include "xgospel.h"

extern Match **_PlayerMatchAddress(Player *player);

enum _dummy {
    CHALLENGER, CHALLENGEE, DISPUTE, DECLINED,
    COLORWHITE, COLORBLACK,
    RULESIGS,   RULESGOE,
    TOURNAMENTYES, TOURNAMENTNO,
    NRCHOICES
};

struct _Match {
    Player *Player;
    Widget  Root, Size, Tim, ByoYomi, Tell, State;
    Widget  Choice[NRCHOICES];
    /* Currently never gets reset to 0. Must study IGS match exchange --Ton */
    int     Challenged;  /* CHALLENGER, CHALLENGEE or DISPUTE */
    int     Automatch;
};

static MyContext Context;
static XrmQuark  MatchQuark;

static void MayDecline(Widget w, XEvent *event, String *string, Cardinal *n);
static XtActionsRec actionTable[] = {
    { (String) "maydecline", MayDecline },
};

void AutoMatchDispute(const char *FirstLine, const NameList *Lines)
{
    const NameList *Line;

    Outputf("%s\n", FirstLine);
    for (Line = Lines->Next; Line != Lines; Line = Line->Next)
        Outputf("%s\n", Line->Name);
}

static int    DefaultRules      = RULESIGS;
static int    DefaultTournament = TOURNAMENTNO;
/* static int    DefaultByoStones  = 25; */
static size_t DefaultSize       = 19;

static void SetState(Match *Exchange, int Value)
{
    int i;

    Exchange->Challenged = Value;
    VaSetManagementChildren
        (Exchange->Choice[CHALLENGER], Value == CHALLENGER,
         Exchange->Choice[CHALLENGEE], Value == CHALLENGEE,
         Exchange->Choice[DISPUTE],    Value == DISPUTE,
         Exchange->Choice[DECLINED],   Value == DECLINED,
         (Widget) 0);
    for (i=0; i<NRCHOICES; i++) XtSetSensitive(Exchange->Choice[i], True);
}

static void MayDecline(Widget w, XEvent *event, String *string, Cardinal *n)
{
    Widget        Root;
    XtPointer     Data;
    Match        *Exchange;
    const Player *player;

    Root = w;
    while (XtIsShell(Root) == False) Root = XtParent(Root);
    if (MyFindContext(Context, Root, MatchQuark, &Data))
        WidgetWarning(w, "maydecline() could not get context");
    else {
        player   = (const Player *) Data;
        Exchange = *_PlayerMatchAddress((Player *) player);
        if (Exchange->Root != Root)
            Raise1(AssertException, "invalid widget back reference");
        if (Exchange->Challenged == CHALLENGEE) {
            SendCommand(NULL, NULL, "decline %s", PlayerToName(player));
            SetState(Exchange, CHALLENGER);
        }
    }
}

static void CallSendChallenge(Widget w,
                              XtPointer clientdata, XtPointer calldata)
{
    Match *Exchange;
    int    Color, Rules, Tournament, Tim, ByoYomi;
    size_t Size;
    String Value;
    const char *Prefix;
 
    Exchange = (Match *) clientdata;

    if (Exchange->Size) {
        XtVaGetValues(Exchange->Size, XtNstring, (XtArgVal) &Value, NULL);
        Size = 0;
        ConvertSize("Board size error", &Size, (XtPointer) &Value, 0);
    } else Size = DefaultSize;

    if (Exchange->Tim) {
        XtVaGetValues(Exchange->Tim, XtNstring, (XtArgVal) &Value, NULL);
        Tim = -1;
        ConvertPositive("Game time error", &Tim, (XtPointer) &Value, 0);
    } else Tim = appdata.DefaultTime;

    if (Exchange->ByoYomi) {
        XtVaGetValues(Exchange->ByoYomi, XtNstring, (XtArgVal) &Value, NULL);
        ByoYomi = -1;
        ConvertNatural("Byo yomi error", &ByoYomi, (XtPointer) &Value, 0);
    } else ByoYomi = appdata.DefaultByoYomi;

    Color = StrengthCompare(Exchange->Player, Me) <= 0 ? Black : White;
    if (Exchange->Choice[COLORBLACK] &&
        XtIsManaged(Exchange->Choice[COLORBLACK]) != False)
        Color = Black;
    if (Exchange->Choice[COLORWHITE] &&
        XtIsManaged(Exchange->Choice[COLORWHITE]) != False)
        Color = White;

    Rules = DefaultRules;
    if (Exchange->Choice[RULESIGS] &&
        XtIsManaged(Exchange->Choice[RULESIGS]) != False)
        Rules = RULESIGS;
    if (Exchange->Choice[RULESGOE] &&
        XtIsManaged(Exchange->Choice[RULESGOE]) != False)
        Rules = RULESGOE;

    Tournament = DefaultTournament;
    if (Exchange->Choice[TOURNAMENTYES] &&
        XtIsManaged(Exchange->Choice[TOURNAMENTYES]) != False)
        Tournament = TOURNAMENTYES;
    if (Exchange->Choice[TOURNAMENTNO] &&
        XtIsManaged(Exchange->Choice[TOURNAMENTNO]) != False)
        Tournament = TOURNAMENTNO;

    if (Rules == RULESIGS)
        if (Tournament == TOURNAMENTYES) Prefix = "t";
        else                             Prefix = "";
    else
        if (Tournament == TOURNAMENTYES) Prefix = "tg";
        else                             Prefix = "goe";

    if (Size != 0 && Tim != -1 && ByoYomi != -1) {
        if (Exchange->Automatch) {
	    SendCommand(NULL, NULL, "automatch %s",
                    PlayerToName(Exchange->Player));
	} else {
	    SendCommand(NULL, NULL, "%smatch %s %c %d %d %d",
			Prefix, PlayerToName(Exchange->Player),
			Color == Black ? 'B':'W', Size, Tim, ByoYomi);
	}
        XtPopdown(Exchange->Root);
        SetState(Exchange, CHALLENGER);
    }
}
    
void SetMatchDescription(Match *match)
{
    SetPlayerTitles(match->Root, match->Player);
}

static void EnterExchange(Player *player, Match *Exchange,
                          int Color, int Rules, int Tournament,
                          size_t Size, int Tim, int ByoYomi)
{
    char    Work[80];

    if (Size) {
        sprintf(Work, "%d", (int) Size);
        XtVaSetValues(Exchange->Size, XtNstring, (XtArgVal) Work, NULL);
    }

    sprintf(Work, "%d", Tim);
    XtVaSetValues(Exchange->Tim, XtNstring, (XtArgVal) Work, NULL);

    sprintf(Work, "%d", ByoYomi);
    XtVaSetValues(Exchange->ByoYomi, XtNstring, (XtArgVal) Work, NULL);

    if (Exchange->Automatch) {
        XtSetSensitive(Exchange->Choice[COLORBLACK], False);
        XtSetSensitive(Exchange->Choice[COLORWHITE], False);
    } else if (Color != Empty) {
        VaSetManagementChildren
            (Exchange->Choice[COLORBLACK],    Color == Black,
             Exchange->Choice[COLORWHITE],    Color == White,
             (Widget) 0);
    }
    if (Rules && !Exchange->Automatch) {
        VaSetManagementChildren
            (Exchange->Choice[RULESIGS],      Rules == RULESIGS,
             Exchange->Choice[RULESGOE],      Rules == RULESGOE,
             (Widget) 0);
    } else {
        XtSetSensitive(Exchange->Choice[RULESIGS], False);
        XtSetSensitive(Exchange->Choice[RULESGOE], False);
    }

    if (Tournament && !Exchange->Automatch) {
        VaSetManagementChildren
            (Exchange->Choice[TOURNAMENTYES], Tournament == TOURNAMENTYES,
             Exchange->Choice[TOURNAMENTNO],  Tournament == TOURNAMENTNO,
             (Widget) 0);
    } else {
        XtSetSensitive(Exchange->Choice[TOURNAMENTYES], False);
        XtSetSensitive(Exchange->Choice[TOURNAMENTNO],  False);
    }
}

void InitMatch(Widget Toplevel)
{
    XtAppAddActions(XtWidgetToApplicationContext(Toplevel),
                    actionTable, XtNumber(actionTable));
    Context = YShellContext(Toplevel);
    MatchQuark = XrmPermStringToQuark("match");
}

static void CallDestroyChallenge(Widget w,
                                 XtPointer clientdata, XtPointer calldata)
{
    Match  *Exchange, **Ref;
    Player *player;

    player   = (Player *) clientdata;
    Ref      =  _PlayerMatchAddress(player);
    Exchange = *Ref;

    WITH_HANDLING {
        MyDeleteContext(Context, Exchange->Root, MatchQuark);
    } ON_EXCEPTION {
        ClearException();
    } END_HANDLING;
    myfree(Exchange);
   *Ref = NULL;
}

static Widget CreateChallenge(XtPointer Closure);
static Match *FindChallenge(Player *player)
{
    Match *Exchange, **Ref;
    Widget Root, Collect, Ok, stats, tell;
    int    Color;

    Ref    = _PlayerMatchAddress(player);
    if (*Ref) return *Ref;

    Exchange = mynew(Match);
    WITH_HANDLING {
        Root = MyVaCreateManagedWidget("challenge", toplevel,
                                       "stateText", (XtArgVal) "You challenge",
                                       NULL);
        Exchange->Player     = player;
        Exchange->Root       = Root;
        XtAddCallback(Root, XtNdestroyCallback,
                      CallDestroyChallenge, (XtPointer) player);
        *Ref = Exchange;
    } ON_EXCEPTION {
        myfree(Exchange);
    } END_HANDLING;
    WITH_HANDLING {
        MySaveContext(Context, Root, MatchQuark, (XtPointer) player);
        SetMatchDescription(Exchange);

        Ok      = XtNameToWidget(Root, "*ok");
        if (Ok) XtAddCallback(Ok, XtNcallback, CallSendChallenge,
                              (XtPointer) Exchange);

        stats   = XtNameToWidget(Root, "*getStats");
        if (stats) XtAddCallback(stats, XtNcallback, CallGetStats,
                                 (XtPointer) player);

        Exchange->Choice[CHALLENGER]  =XtNameToWidget(Root,"*stateChallenger");
        Exchange->Choice[CHALLENGEE]  =XtNameToWidget(Root,"*stateChallengee");
        Exchange->Choice[DISPUTE]     =XtNameToWidget(Root,"*stateDispute");
        Exchange->Choice[DECLINED]    =XtNameToWidget(Root,"*stateDecline");
        Exchange->Choice[COLORWHITE]   =XtNameToWidget(Root, "*colorWhite");
        Exchange->Choice[COLORBLACK]   =XtNameToWidget(Root, "*colorBlack");
        Exchange->Choice[RULESIGS]     =XtNameToWidget(Root, "*rulesIgs");
        Exchange->Choice[RULESGOE]     =XtNameToWidget(Root, "*rulesGoe");
        Exchange->Choice[TOURNAMENTYES]=XtNameToWidget(Root, "*tournamentYes");
        Exchange->Choice[TOURNAMENTNO] =XtNameToWidget(Root, "*tournamentNo");

        Exchange->Size    = XtNameToWidget(Root, "*size*text");
        Exchange->Tim     = XtNameToWidget(Root, "*time*text");
        Exchange->ByoYomi = XtNameToWidget(Root, "*byoYomi*text");
        XtCallActionProc(Root, (String) "nexttext", NULL, NULL, 0); 

        Exchange->State = XtNameToWidget(Root, "*state");
	Exchange->Automatch = False;

        /* Must be done here in case we get an early popup
           due to toggle coupling */
        Color = StrengthCompare(player, Me) <= 0 ? Black : White;
        SetState(Exchange, CHALLENGER);
        EnterExchange(player, Exchange, Color, DefaultRules,
                      DefaultTournament, DefaultSize, appdata.DefaultTime,
                      appdata.DefaultByoYomi);

        Exchange->Tell  = tell  = XtNameToWidget(Root, "*getTell");
        XtRealizeWidget(Root);
        Collect = XtNameToWidget(Root, "*collect");
        if (Collect) XtInstallAllAccelerators(Collect, Collect);
        DeleteProtocol(Root);
    } ON_EXCEPTION {
        XtDestroyWidget(Root);
    } END_HANDLING;

    if (tell) CoupleTell(player, tell);
    CoupleTellChallenge(player, Root, CreateChallenge);
    return Exchange;
}

void CallGetChallenge(Widget w, XtPointer clientdata, XtPointer calldata)
{
    XtPopup(FindChallenge((Player *) clientdata)->Root, XtGrabNone);
}

/* Opponent challenges us */
static void MatchExchange(Player *player,
                          int Color, int Rules, int Tournament,
                          size_t Size, int Tim, int ByoYomi, int Automatch)
{
    Match *Exchange;

    if (appdata.AutoReply) {
        char msg[256];
	sprintf(msg, "challenged you. To accept type: match %s %c %d %d %d\n",
		PlayerName(player), (Color == Black ? 'B' : 'W'), Size,
		Tim, ByoYomi);
	ReceivedTell(player, msg);
	return;
    }

    Exchange = FindChallenge(player);
    Exchange->Automatch = Automatch;
    SetState(Exchange, CHALLENGEE);
    EnterExchange(player, Exchange,
                  Color, Rules, Tournament, Size, Tim, ByoYomi);
    XtPopup(Exchange->Root, XtGrabNone);
}

static Widget CreateChallenge(XtPointer Closure)
{
    Match *match;

    match = FindChallenge((Player *) Closure);
    return match->Root;
}

void CoupleChallenge(Player *player, Widget Toggle)
{
    Match *match;
    Widget Root;

    match = *_PlayerMatchAddress(player);
    if (match) {
        Root = match->Root;
        XtVaSetValues(Toggle, XtNstate, (XtArgVal) MyIsPopped(Root), NULL);
    } else Root = NULL;
    CoupleToggleWidget(Root, Toggle, CreateChallenge, (XtPointer) player);
}

void CoupleChallengeTell(Player *player, Widget Root,
                         Widget (*InitFun)(XtPointer Closure))
{
    Match *match;

    match = *_PlayerMatchAddress(player);
    if (match && match->Tell)
        CoupleToggleWidget(Root, match->Tell, InitFun, (XtPointer) player);
}

void MatchRequest(int rules, const NameList *names)
{
    const char *Name;
    int         Color, Tim, ByoYomi, Tournament, Rules;
    size_t      Size;

    names   = names->Next;
    Name    = names->Name;
    names   = names->Next;
    Color   = names->Name[0] == 'B' ? Black : White;
    names   = names->Next;
    Size    = atoi(names->Name);
    names   = names->Next;
    Tim     = atoi(names->Name); 
    names   = names->Next;
    ByoYomi = atoi(names->Name); 

    switch(rules) {
      case 'I':
        Rules      = RULESIGS;
        Tournament = TOURNAMENTNO;
        break;
      case 'i':
        Rules      = RULESIGS;
        Tournament = TOURNAMENTYES;
        break;
      case 'G':
        Rules      = RULESGOE;
        Tournament = TOURNAMENTNO;
        break;
      case 'g':
        Rules      = RULESGOE;
        Tournament = TOURNAMENTYES;
        break;
      default:
        Raise1(AssertException, "Impossible match type");
        return;
    }

    MatchExchange(NameToPlayer(Name), Color, Rules, Tournament,
                  Size, Tim, ByoYomi, False);
}

void AutoMatchRequest(const NameList *names)
/* When typing automatch, IGS says that I want a match with myself!
 *   (1) automatch jl
 *   36 xgospel wants a match with you:
 *   36 xgospel wants 19x19 in 5 minutes with 5 byo-yomi and 25 byo-stones
 * So the challenge must be created only when receiving it.
 */
{
    int         Color, Tim, ByoYomi;
    size_t      Size;
    Player      *player;

    names   = names->Next;
    player  = NameToPlayer(names->Name);
    if (player == Me) return;

    names   = names->Next;
    if (strcmp(names->Name, "wants")) {
        Raise2(AssertException, "Automatch", names->Name);
    }
    names   = names->Next;
    Size    = atoi(names->Name);
    names   = names->Next;
    if (strcmp(names->Name, "in")) {
        Raise2(AssertException, "Automatch", names->Name);
    }
    names   = names->Next;
    Tim     = atoi(names->Name); 
    names   = names->Next;
    if (strcmp(names->Name, "minutes")) {
        Raise2(AssertException, "Automatch", names->Name);
    }
    names   = names->Next;
    if (strcmp(names->Name, "with")) {
        Raise2(AssertException, "Automatch", names->Name);
    }
    names   = names->Next;
    ByoYomi = atoi(names->Name); 

    Color   = StrengthCompare(player, Me) < 0 ? Black : White;

    MatchExchange(player, Color, RULESIGS, TOURNAMENTNO,
                  Size, Tim, ByoYomi, True);
}

void Dispute(DisputeDesc *Lines, int FromOther)
{
    const DisputeDesc *Line;
    Match *Exchange;

    if (FromOther)
        for (Line = Lines->Next; Line != Lines; Line = Line->Next)
            /* Here I assume you can't play yourself. Currently true --Ton */
            if (Line->Player != Me) {
                Exchange = FindChallenge(Line->Player);
		Exchange->Automatch = False;
                SetState(Exchange, DISPUTE);
                EnterExchange(Line->Player, Exchange,
                              Line->Color == Black ? White : Black,
                              /* Stupid server handles rules and
                                 tournament differently */
                              0, 0,
                              Line->SizeX, (Line->Tim+30)/60, Line->ByoYomi);
                XtPopup(Exchange->Root, XtGrabNone);
            }
}

void Decline(Player *player)
{
    Match *Exchange;

    Exchange = FindChallenge(player);
    SetState(Exchange, DECLINED);
    XtPopup(Exchange->Root, XtGrabNone);
}

void WantMatchType(Player *player, int type)
{
    Match *Exchange;

    Exchange = FindChallenge(player);
    if (type & (TOURNAMENTTYPE|NOTOURNAMENTTYPE)) {
        VaSetManagementChildren
            (Exchange->Choice[TOURNAMENTYES], (type &   TOURNAMENTTYPE) ==   TOURNAMENTTYPE,
             Exchange->Choice[TOURNAMENTNO],  (type & NOTOURNAMENTTYPE) == NOTOURNAMENTTYPE,
             (Widget) 0);
        XtSetSensitive(Exchange->Choice[TOURNAMENTYES], False);
        XtSetSensitive(Exchange->Choice[TOURNAMENTNO],  False);
    }
    if (type & (IGSTYPE|GOETYPE)) {
        VaSetManagementChildren
            (Exchange->Choice[RULESIGS], (type & IGSTYPE) == IGSTYPE,
             Exchange->Choice[RULESGOE], (type & GOETYPE) == GOETYPE,
             (Widget) 0);
        XtSetSensitive(Exchange->Choice[RULESIGS], False);
        XtSetSensitive(Exchange->Choice[RULESGOE],  False);
    }
    SetState(Exchange, DISPUTE);
    XtPopup(Exchange->Root, XtGrabNone);
}
