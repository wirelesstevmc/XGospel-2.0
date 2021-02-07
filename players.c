/*****************************************************/
/* players.c: The part of xgospel responsible for    */
/*            keeping the list of players and player */
/*            data up to date.                       */
/*                                                   */
/* Author:  Ton Hospel                               */
/*          ton@linux.cc.kuleuven.ac.be              */
/*          (19993,1994, 1995)                       */
/*                                                   */
/* Copyright: GNU copyleft                           */
/*****************************************************/

#include <X11/StringDefs.h>
#include <X11/Intrinsic.h>
#include <X11/Shell.h>
#include <X11/Xaw/Label.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/Paned.h>
#include <X11/Xaw/StripChart.h>
#include <X11/Xmu/CharSet.h>
#include "SmeToggle.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include <mymalloc.h>
#include <except.h>
#include <myxlib.h>
#include <YShell.h>

#include "connect.h"
#include "events.h"
#include "gamesP.h"
#include "match.h"
#include "stats.h"
#include "tell.h"
#include "utils.h"
#include "xgospel.h"

#define PRTPTR    "#%p"        /* Nice outputformat for a pointer          */
#define NULLNAME  "(null)"     /* How to print the null pointer            */

#define BEGINESCAPE       "\\{"
#define ENDESCAPE         "} "
#define XGOSPEL           "xgospel"

#define XGOSPELESCAPE BEGINESCAPE XGOSPEL ENDESCAPE

#define CREATE    1
#define DELETE    2
#define PlayerWidget(player) (((player)->Widget != 0) +                 \
                              ((player)->WidgetPlan & CREATE != 0) -    \
                              ((player)->WidgetPlan & DELETE != 0))

#define AppContext(w)   XtWidgetToApplicationContext(w)

struct _Player {
    struct _Player *Next, *Previous;
    char           *Name;
    size_t          Length;
    char           *Strength;
    int             Rating;     /* IGS rating */
    int             Importance; /* rating by default */
    int             Stored;     /* number of stored games */
    float           ExactRating;/* my rating computed with "proba" */
    int             AutoRated, Won, Lost, FirstObserving, Playing;
    Boolean         IsMarked;
    char           *State;
    char           *Idle;
    char           *Info, *Country, *Language;
    NameList       *Results;
    int             Pos, OnServer, XgospelUser;
    State           Found;
    Tell           *Tell;       /* Talk session with this user       */
    Match          *Match;      /* Challenge exchange with this user */
    Stats          *Stats;      /* Stats of this user                */
    Game           *Game;       /* Game this user is involved in     */
    int             WidgetPlan;
    Widget          Widget;
};

Widget  PlayersButton;
int     nrplayers, maxplayers;
int     minProRating;
int     minHiDanRating;
Player *Me;

extern int nrgames;

static void DoPlayer(Widget w, XEvent *evnt, String *str, Cardinal *n);
static XtActionsRec actionTable[] = {
    { (String) "doplayer", DoPlayer },
};

static Player PlayerBase      = { &PlayerBase,       &PlayerBase };
static Player DummyPlayerBase = { &DummyPlayerBase,  &DummyPlayerBase };

ConvertToWidget PlayerToWidget;
static WidgetPlan      PlayersPlan;
static const char      StrengthOrder[] = "p DdkR?";
static Widget          playerinfo, players, playerstats;
static Widget          RaiseOnImportant, BeepOnImportant;
static int             WantWho;

static Player *MakePlayer(const char *Name, int Length);
static void    ChangeStrength(Player *player);
static void    ShowPlayer(Player *player, int newStatus);
static void    DeletePlayer(Player *player);

static const char *NameStrength(const char *Name, const char *Strength,
                                int AutoRated)
{
    static char Text[80];

    sprintf(Text, "%s[%3s]%c", Name, Strength, AutoRated);
    return Text;
}

const char *PlayerString(const Player *player)
{
    int AutoRated;

    switch(player->AutoRated) {
      case 0:  AutoRated = ' '; break;
      case 1:  AutoRated = '*'; break;
      default: AutoRated = '?'; break;
    }

    return NameStrength(player->Name, player->Strength, AutoRated);
}

const char *PlayerNameToString(const char *Name)
{
    return PlayerString(NameToPlayer(Name));
}

double PlayerExactRating(const Player *player)
{
    return player == NULL ? -1. : player->ExactRating;
}

int PlayerRating(const Player *player)
{
    return player == NULL ? -1 : player->Rating;
}

char *PlayerTemplateDescription(const Player *player, const char *Template)
{
    const char *AutoRated, *autorated;

    switch(player->AutoRated) {
      case 0:  AutoRated =      autorated = " "; break;
      case 1:  AutoRated =      autorated = "*"; break;
      default: AutoRated = "?"; autorated = ""; break;
    }

    return StringToFilename(Template,
                            (int) 'N', player->Name,
                            (int) 'n', player->Strength,
                            (int) 'A', AutoRated,
                            (int) 'a', autorated,
                            0);
}

static char *PlayerTitle(const char *Pattern, XtPointer Closure)
{
    return PlayerTemplateDescription((const Player *) Closure, Pattern);
}

void SetPlayerTitles(Widget w, const Player *player)
{
    SetWidgetTitles(w, PlayerTitle, (XtPointer) player);
}

/*****************************************************************************/

static void SetNrPlayers(Widget w, XtPointer client_data, XtPointer value)
{
    *(double *) value = ((double) nrplayers) / appdata.PlayersScale;
}

 static int WidgetPlayerCompare(const void *player1, const void *player2)
{
    /* On machines where this idiotic series of casts is necessary, the
       method to get the addresses is probably invalid anyways, but let's
       pretend... */
    return PlayerCompare((Player *)(void *)(char *)*(Widget *)player1,
                       (Player *)(void *)(char *)*(Widget *)player2);
}

static void SelectPlayer(Widget w, XtPointer clientdata, XtPointer calldata);
static void PlayerWidgetCreate(void *Entry, XtPointer Closure)
{
    Player *player;

    player = (Player *) Entry;
    XtAddCallback(player->Widget, XtNcallback, SelectPlayer,
                  (XtPointer) player);
}

static void PlayerWidgetDone(int NrEntries, XtPointer Closure)
{
    ShowPlayerStats();
}

static const char *SetPlayerEntry(Cardinal *i, Arg *args, const Player *player)
{
    static char Text[80];

    sprintf(Text, "%16s %.2s%4d %4d", PlayerString(player), player->State,
            player->FirstObserving, player->Playing);
    switch(player->FirstObserving) {
      case -2:
        Text[21] = Text[22] = Text[23] = '?';
        break;
      case -1:
        Text[21] = Text[22] = Text[23] = '-';
        break;
    }
    switch(player->Playing) {
        break;
      case -2:
        Text[25] = Text[26] = Text[27] = '?';
        break;
      case -1:
        Text[25] = Text[26] = Text[27] = '-';
        break;
    }
    if (args) {
        XtSetArg(args[*i], XtNlabel, Text); (*i)++;
    }
    return Text;
}

/* Result is usable until the next call */
const char *GetPlayerType(const Player *player, const char *Prefix)
{
    static char Name[80];
    char   ch, *ptr;

    ptr = player->Strength;
    while (isdigit(*ptr)) ptr++;
    sprintf(Name, "%.*s%.*s",
            (int) sizeof(Name)/2, Prefix, (int) sizeof(Name)/2-1, ptr);
    /* Convert 5 and 6 dans to HiDans */
    if (player->Rating >= minHiDanRating && player->AutoRated == 1) {
        Name[strlen(Name)-1] = 'D';
    }
    /* Convert high dans to pros */
    if (player->Rating >= minProRating && player->AutoRated == 1) {
        Name[strlen(Name)-1] = 'p';
    }
    for (ptr = Name; (ch = *ptr) != 0; ptr++)
        if (!isalnum(ch) && ch != '-') *ptr = '_';
    return Name;
}

int StrengthToRating(const char *Strength)
/* NR = 0, 30k = 2, 1k = 31, 1d = 32, 5d=36, 9d = 40, 1p = 41 */
{
    int  level = atoi(Strength);

    switch (Strength[strlen(Strength)-1]) {
        case 'p': return 40 + level;
        case 'D': return 35 + level;
        case 'd': return 31 + level;
        case 'k': return 32 - level;
        default:  return 0;
    }
}

const char *RatingToStength(int rating)
/* NR = 0, 30k = 2, 1k = 31, 1d = 32, 9d = 40, 1p = 41 */
{
    static char buffer[20];

    if (rating <= 1) {
	return "NR";
    } else if (rating <= 31) {
	sprintf(buffer, "%dk", 32 - rating);
    } else if (rating <= 40) {
	sprintf(buffer, "%dd", rating - 31);
    } else {
	sprintf(buffer, "%dp", rating - 40);
    }
    return buffer;
}

static const char *PlayerProperties(Cardinal *i, Arg *arg,
                                  void *Entry, XtPointer Closure)
{
    Player       *player;
    const char *Name;
    int j;

    player = (Player *) Entry;
    Name = SetPlayerEntry(i, arg, player);
    if (player->IsMarked) return "playerEntryMarked";

    if (appdata.SimpleNames) {
        Name = RegexGetPlayerType(PlayerToName(player));
    } else {
        Name = RegexGetPlayerType(Name);
    }
    if (!Name) Name = GetPlayerType(player, "playerEntry");
    return Name;
}

void PlayersResort(void)
{
    WidgetPlanResort(PlayersPlan);
}

static enum _SortMethod_ {
    SORTIMPORTANCE, SORTSTRENGTH, SORTNAME
} SortMethod = SORTIMPORTANCE;

static void CallSort(Widget w, XtPointer clientdata, XtPointer calldata)
{
    if ((Boolean)XTPOINTER_TO_INT(calldata) != False)
        if (SortMethod != (enum _SortMethod_) XTPOINTER_TO_INT(clientdata)) {
            SortMethod = (enum _SortMethod_) XTPOINTER_TO_INT(clientdata);
            PlayersResort();
        }
}

void InitPlayers(Widget Toplevel)
{
    Widget PlayerRoot, PlayerCollect, PlayerStrip;
    Widget PlayerSortName, PlayerSortStrength, PlayerSortImportance;
    Widget AllowResize;
    Boolean state;
    const char *Error;

    minProRating = StrengthToRating(appdata.MinProRank);
    minHiDanRating = StrengthToRating(appdata.MinHiDanRank);

    PlayerToWidget.Convert   = NULL;
    PlayerToWidget.NrConvert = -1;
    Error = CompileToWidget(appdata.PlayerToWidget, &PlayerToWidget);
    if (Error) Warning("CompileToWidget: %s\n", Error);

    InitHash();

    Me             = NULL;
    nrplayers      = maxplayers = 0;
    WantWho        = 0;

    XtAppAddActions(AppContext(Toplevel), actionTable, XtNumber(actionTable));

    PlayerRoot = MyVaCreateManagedWidget("players", Toplevel, NULL);

    players       = XtNameToWidget(PlayerRoot, "*set");
    PlayerCollect = XtNameToWidget(PlayerRoot, "*collect");
    playerstats   = XtNameToWidget(PlayerRoot, "*stats");
    PlayerStrip   = XtNameToWidget(PlayerRoot, "*strip");
    if (PlayerStrip)
        XtAddCallback(PlayerStrip, XtNgetValue, SetNrPlayers, NULL);
    playerinfo    = XtNameToWidget(PlayerRoot, "*info");

    if (PlayersButton) {
        XtAddCallback(PlayersButton, XtNcallback,        CallToggleUpDown,
                      (XtPointer) PlayerRoot);
        XtAddCallback(PlayerRoot,    XtNpopupCallback,   CallToggleOn,
                      (XtPointer) PlayersButton);
        XtAddCallback(PlayerRoot,    XtNpopdownCallback, CallToggleOff,
                      (XtPointer) PlayersButton);
    }

    PlayersPlan =
        AllocWidgetPlan(&PlayerBase, offsetof(Player, Next),
                        offsetof(Player, Pos), offsetof(Player, WidgetPlan),
                        offsetof(Player, Widget), &appdata.PlayerUpdateTimeout,
                        players, (XtPointer) "players", WidgetPlayerCompare,
                        PlayerProperties, PlayerWidgetCreate, 0,
                        PlayerWidgetDone);
    PlayerSortStrength = XtNameToWidget(PlayerRoot, "*sortStrength");
    if (PlayerSortStrength) {
        XtAddCallback(PlayerSortStrength, XtNcallback,
                      CallSort, INT_TO_XTPOINTER((int) SORTSTRENGTH));
        XtVaGetValues(PlayerSortStrength, XtNstate, (XtArgVal) &state, NULL);
        if (state != False)
            CallSort(PlayerSortStrength, INT_TO_XTPOINTER((int) SORTSTRENGTH),
                     INT_TO_XTPOINTER((int) state));
    }
    PlayerSortImportance = XtNameToWidget(PlayerRoot, "*sortImportance");
    if (PlayerSortImportance) {
        XtAddCallback(PlayerSortImportance, XtNcallback,
                      CallSort, INT_TO_XTPOINTER((int) SORTIMPORTANCE));
        XtVaGetValues(PlayerSortImportance, XtNstate, (XtArgVal) &state, NULL);
        if (state != False)
            CallSort(PlayerSortImportance,
		     INT_TO_XTPOINTER((int) SORTIMPORTANCE),
                     INT_TO_XTPOINTER((int) state));
    }
    PlayerSortName = XtNameToWidget(PlayerRoot, "*sortName");
    if (PlayerSortName) {
        XtAddCallback(PlayerSortName, XtNcallback,
                      CallSort, INT_TO_XTPOINTER((int) SORTNAME));
        XtVaGetValues(PlayerSortName, XtNstate, (XtArgVal) &state, NULL);
        if (state != False)
            CallSort(PlayerSortName, INT_TO_XTPOINTER((int) SORTNAME),
                     INT_TO_XTPOINTER((int) state));
    }

    RaiseOnImportant = XtNameToWidget(PlayerRoot, "*raise");
    BeepOnImportant = XtNameToWidget(PlayerRoot, "*beep");

    AllowResize   = XtNameToWidget(PlayerRoot, "*allowResize");
    if (AllowResize) {
        XtVaGetValues(AllowResize, XtNstate, (XtArgVal) &state, NULL);
	XtVaSetValues(PlayerRoot, XtNallowShellResize, (XtArgVal) state, NULL);
        XtAddCallback(AllowResize, XtNcallback, CallAllowShellResize,
                      (XtPointer) PlayerRoot);
        if (players) {
            XtVaSetValues(players, XtNallowResize, (XtArgVal) state, NULL);
            XtAddCallback(AllowResize, XtNcallback, CallAllowResize,
                          (XtPointer) players);
        }
    }
    XtRealizeWidget(PlayerRoot);
    if (PlayerCollect) XtInstallAllAccelerators(PlayerCollect, PlayerCollect);
    DeleteProtocol(PlayerRoot);
    if (PlayersButton)
        CallToggleUpDown(PlayersButton, (XtPointer) PlayerRoot, NULL);
}

void CleanPlayers(void)
{
    if (PlayerToWidget.Convert) FreeToWidget(&PlayerToWidget);
    FreeWidgetPlan(PlayersPlan);
}

/*****************************************************************************/

static int PlayerImportance(Player *player)
/* Return the player importance if the player matches playerToWidget
 * and the corresponding player entry matches playerImportance, otherwise
 * return the player rating.
 */
{
    const char *entry;
    int  i;

    if (!appdata.PlayerImportance) return player->Rating;

    /* Check first whether the player matches in playerToWidget */
    if (appdata.SimpleNames) {
        entry = RegexGetPlayerType(PlayerToName(player));
    } else {
        entry = RegexGetPlayerType(SetPlayerEntry(NULL, NULL, player));
    }
    if (!entry) return player->Rating;

    if (strncmp(entry, "playerEntry", 11)) {
	Raise3(AssertException, "Player", PlayerString(player),
	       "has bad entry");
    }
    entry += 11; /* skip "playerEntry" */

    /* Check whether the player entry matches in playerImportance */
    for (i = 0; i < appdata.PlayerImportance->Nr; i++) {

	if (!strcmp(entry, appdata.PlayerImportance->String1[i])) {
	    return StrengthToRating(appdata.PlayerImportance->String2[i]);
	}
    }
    return player->Rating;
}

void CheckPlayerStrength(Player *player, const char *Strength)
{
    char   *str;
    size_t  Length;
    int     AutoRated;
    int     newStatus = (player->Found != UNCHANGED);

    Length    = strlen(Strength)-1;
    AutoRated = Strength[Length] == '*';
    if (!AutoRated) Length++;
    str = mystrndup(Strength, Length);
    WITH_UNWIND {
        CHANGEINT   (player, AutoRated, AutoRated, player->Found = CHANGED);
        CHANGESTRING(player, Strength, str, 
		     ChangeStrength(player); player->Found = CHANGED);
    } ON_UNWIND {
        myfree(str);
    } END_UNWIND;
    if (player->Found != UNCHANGED) {
	ShowPlayer(player, newStatus);
    }
}

static void NewPlayer(Player *player)
{
    if (playerinfo)
        AddText(playerinfo, "%16s connected\n", PlayerString(player));
    if (player->Tell) TellMessage(player->Tell, "player connected");
    nrplayers++;

    if (player->Importance >= minImportantRating) {
	IfBell(BeepOnImportant);
	IfRaise(RaiseOnImportant, RaiseOnImportant);
    }

    Logon(player);
}

Player *PlayerConnect(const char *Name, const char *Strength)
{
    Player *player;
    int     OldOn;

    player = PlayerFromName(Name);
    OldOn  = player->OnServer;
    player->OnServer = 1;
    /* Force recomputing the importance if simpleNames was changed between
     * player disconnected and player reconnected:
     */
    player->Importance = -1;
    player->Found    = CHANGED;
    CheckPlayerStrength(player, Strength);
    if (!OldOn) NewPlayer(player);
    return player;
}

void PlayerDisconnect(const char *Name)
{
    Player *player;

    player = NameToPlayer(Name);
    if (player->OnServer) DeletePlayer(player);
}

/*****************************************************************************/

Player *FindPlayerByNameAndStrength(const char *Name,
                                    const char *Strength)
{
    Player *player;

    if (DebugFun) {
        printf("FindPlayerByNameAndStrength(%s, %s)\n", Name, Strength);
        fflush(stdout);
    }

    player = PlayerFromName(Name);
    CheckPlayerStrength(player, Strength);
    return player;
}

Player *PlayerFromLengthName(const char *Name, int Length)
{
    Player *player;

    if (Length < 0) Length = strlen(Name);
    for (player = PlayerBase.Next;
         player != &PlayerBase;
         player = player->Next)
        if (player->Length == Length &&
            0 == memcmp(Name, player->Name, (size_t) Length)) return player;
    return MakePlayer(Name, Length);
}

Player *LengthNameOnServer(const char *Name, int Length)
{
    Player *player;

    if (Length < 0) Length = strlen(Name);
    for (player = PlayerBase.Next;
         player != &PlayerBase;
         player = player->Next)
        if (player->Length == Length &&
            0 == memcmp(Name, player->Name, (size_t) Length)) return player;
    return NULL;
}

static int GuestNameP(const char *Name)
{
    const char *ptr;

    if (strncmp(Name, "guest", 5)) return 0;
    ptr=Name+5;
    while (isdigit(*ptr)) ptr++;
    return *ptr == 0 && ptr != Name+5;
}

Player *LengthNameToPlayer(const char *Name, int Length)
{
    Player *player;

    if (Length < 1) Length = strlen(Name);
    player = LengthNameOnServer(Name, Length);
    if (!player) {
        player = MakePlayer(Name, Length);
        Name = player->Name;
        if (!TersePlay(NULL) &&
            /* Remove guest clutch as soon as server gets fixed --Ton */
            !GuestNameP(Name)) {
	    if (Entered && Name[0] != '*' && appdata.WantVerbose) {
                Warning("Player %s not found, "
                        "player database corrupted ?\n", Name);
	    }
            if (appdata.WhoTimeout > 0) AutoCommand(NULL, "who");
        }
    }
    return player;
}

void PlayerInGame(Player *player, Game *game, int Id)
{
    if (Id < 0) Raise3(AssertException, "Player", PlayerString(player),
                       "getting involved in unidentified game. "
                       "Internal data corrupted ?");
    if (player) {
        player->Game = game;
        CHANGEINT(player, Playing, Id, player->Found = CHANGED);
        if (player->Found != UNCHANGED) ShowPlayer(player, True);
    }
}

void PlayerOutGame(Player *player, Game *game, int Id)
{
    int First;

    if (Id < 0) Raise3(AssertException, "Player", PlayerString(player),
                       "getting out of unidentified game."
                       " Internal data corrupted ?");
    if (player) {
        if (player->Game != game && !strchr(player->Name, '*')
	    && appdata.WantVerbose) {
            Warning("Player %s seems to be involved in some old game\n",
                    PlayerString(player));
	}
        player->Game = NULL;
        CHANGEINT(player, Playing, -1, player->Found = CHANGED);
        if (player->Found != UNCHANGED) ShowPlayer(player, True);
    }
    for (player = PlayerBase.Next;
         player != &PlayerBase; player = player->Next) {
        if (player->FirstObserving == Id) {
            if (player == Me) First = MyFirstObserved();
            else              First = -2;
            CHANGEINT(player, FirstObserving, First, player->Found = CHANGED);
            if (player->Found != UNCHANGED) ShowPlayer(player, True);
        }
    }
}

Player *FindPlayer(const char *Name,  const char *Strength,
                   const char *state, const char *Idle)
{
    Player   *player;
    int       OldOn, Work;
    char      ShortState[3];

    if (DebugFun) {
        printf("FindPlayer(%s, %s, %s)\n", Name, Strength, state);
        fflush(stdout);
    }

    player = PlayerFromName(Name);
    OldOn = player->OnServer;
    player->OnServer = 1;
    if (PlayerWidget(player)) player->Found = UNCHANGED;
    else                      player->Found = CHANGED;
    CHANGESTRING(player, Idle, Idle, /* player->Found = CHANGED */;);
    ShortState[0] = state[0];
    ShortState[1] = state[1];
    ShortState[2] = 0;
    CHANGESTRING(player, State, ShortState, player->Found = CHANGED);
    if (state[4] == '-') Work = -1;
    else Work = atoi(state+2);
    CHANGEINT(player, FirstObserving, Work, player->Found = CHANGED);
    if (state[9] == '-') Work = -1;
    else Work = atoi(state+6);
    CHANGEINT(player, Playing, Work, player->Found = CHANGED);
    CheckPlayerStrength(player, Strength);
    if (!OldOn) NewPlayer(player);
    return player;
}

static Player *RealMakePlayer(const char *Name, size_t Length)
{
    Player *player;

    if (DebugFun) {
        printf("MakePlayer(%s)\n", Name);
        fflush(stdout);
    }

    player = mynew(Player);
    player->Name = player->Strength = player->State = player->Idle = NULL;
    player->Info = player->Country  = player->Language = NULL;
    WITH_HANDLING {
        player->Name        = mystrndup(Name, Length);
        player->Length      = Length;
        player->Strength    = mystrdup("???");
	player->Rating      = player->Importance = -1;
	player->Stored      = -1;
	player->ExactRating = -1.0;
        player->State       = mystrdup("??");
        player->Idle        = mystrdup(UNKNOWN);
        player->Info        = NULL;
        player->Country     = NULL;
        player->Language    = NULL;
        player->Won         = -1;       /* Means: unknown */
        player->Lost        = -1;       /* Means: unknown */
        player->FirstObserving = -2;    /* Means: unknown, -1 means none */
        player->Playing     = -2;       /* Means: unknown, -1 means none */
        player->AutoRated   = -2;
	player->IsMarked    = False;
        player->Results     = NULL;
        player->Tell        = 0;
        player->Match       = 0;
        player->Stats       = 0;
        player->WidgetPlan  = 0;
        player->Widget      = 0;
        player->Found       = NEW;
        player->Pos         = 0;
        player->OnServer    = 0;
        player->XgospelUser = NONXGOSPELUSER;
        player->Game        = NULL;
    } ON_EXCEPTION {
        if (player->Language) myfree(player->Language);
        if (player->Country)  myfree(player->Country);
        if (player->Info)     myfree(player->Info);
        if (player->Idle)     myfree(player->Idle);
        if (player->State)    myfree(player->State);
        if (player->Strength) myfree(player->Strength);
        if (player->Name)     myfree(player->Name);
        myfree(player);
    } END_HANDLING;
    return player;
}

static Player *MakePlayer(const char *Name, int Length)
{
    Player *player;

    player = RealMakePlayer(Name, (size_t) Length);
    player->Previous       =  PlayerBase.Previous;
    player->Next           = &PlayerBase;
    player->Next->Previous =  player->Previous->Next = player;
    return player;
}

Player *MakeDummyPlayer(void)
{
    Player *player;

    player = RealMakePlayer("??????", 6);
    player->Previous       =  DummyPlayerBase.Previous;
    player->Next           = &DummyPlayerBase;
    player->Next->Previous =  player->Previous->Next = player;
    return player;
}

void RenameDummyPlayer(Player *player, const char *Name)
{
    CHANGESTRING(player, Name, Name,/* Here we should change game entries */;);
}

void FreeDummyPlayer(Player *player)
{
    player->Previous->Next = player->Next;
    player->Next->Previous = player->Previous;

    myfree(player->Name);
    myfree(player->Strength);
    myfree(player->State);
    myfree(player->Idle);
    myfree(player->Info);
    myfree(player->Country);
    myfree(player->Language);
    myfree(player);
}

void AddResults(const char *Name, NameList *Results)
{
    Player     *player;
    NameList   *Line, *Next;
    const char *Ptr;
    char        Name1[20], Name2[20];

    for (Line = Results->Next; Line != Results; Line = Next) {
        Next = Line->Next;
        Ptr = Line->Name;
        *Name1 = *Name2 = 0;
        sscanf(Ptr, "%19s", Name1);
        Name1[sizeof(Name1)-1] = 0;
        Ptr = strchr(Ptr, ':');
        if (Ptr) {
            sscanf(Ptr+1, "%19s", Name2);
            Name2[sizeof(Name1)-1] = 0;
            if (!*Name1 || !*Name2)
                Warning("Invalid results line deleted: %s\n", Line->Name);
            else if (!strcmp(Name, Name1) || !strcmp(Name, Name2)) continue;
        } else Warning("Invalid results line deleted: %s\n", Line->Name);
        Line->Previous->Next = Next;
        Next->Previous = Line->Previous;
        myfree(Line->Name);
        myfree(Line);
    }
    player = PlayerFromName(Name);
    if (player->Results) FreeNameList(player->Results);
    player->Results = Results;
}

void CallPlayerObserve(Widget w, XtPointer clientdata, XtPointer calldata)
{
    const Player *player = (const Player *)clientdata;
    char Buffer[200];
 
    if (player->Game && GameServerId(player->Game) >= 0) {
      if (player->Tell) {
	sprintf(Buffer, "observing game %d:  %s",
		GameServerId(player->Game),
		GameLongDescription(player->Game));
	TellMessage(player->Tell, Buffer);
      }
      StartObserve(player->Game);
    } else if (player->Tell) {
      sprintf(Buffer, "%s is not playing currently", player->Name);
      TellMessage(player->Tell, Buffer);
    } else {
      Warning("%s is not playing currently\n", player->Name);
    }
}

static void TogglePlayerMark(Player *player)
/* Mark or unmark the player. For a marked player, PlayerProperties returns
 * "playerEntryMarked" and the resources playerEntryMarked.background and
 * playerEntryMarked.foreground are used to change the widget color.
 */
{
    player->IsMarked = !player->IsMarked;

    WidgetPlanReposition(player, PlayersPlan);
}

void MyLoseProbas(Player *player, int myRating, int theirRating,
		  const char *handicap,
		  const char *probaLoseAsWhite, const char *probaLoseAsBlack)
/* player is NULL when the proba is known to be against a non-* player
 * (the xgospel maintainer) to allow setting my own exact rating.
 * Otherwise we don't know yet whether the player is *'ed or not,
 * but we should know my exact rating.
 * myRating may be different from Me->Rating. For example a rating of 33.6 is
 * 2d* 34, so Me->Rating is 33 but myRating is 34. We use myRating only
 * when I am not autorated (no *) so in this case myRating == Me->Rating.
 * Similarly, player->Rating may be different from theirRating if the player
 * is not connected or has a rating with a fractional part in 0.5 .. 0.8
 */
{
    double hand = atof(handicap);
    double Badvantage; /* hand - Wrating + Brating */
    double probLoseAsW = 0.01*atof(probaLoseAsWhite);
    double probLoseAsB = 0.01*atof(probaLoseAsBlack);
    Boolean useBlack = fabs(probLoseAsW - 0.5) > fabs(probLoseAsB - 0.5);
    double probLose = useBlack ? probLoseAsB : probLoseAsW;
    double exactRating;

    if (DebugFun) {
	printf("myRating %d, theirRating %d, hand %f\n",
	       myRating, theirRating, hand);
	printf("probLoseAsW %f, probLoseAsB %f\n",
	       probLoseAsW, probLoseAsB);
	fflush(stdout);
    }

    /* Fix the rating if the player is not connected: */
    if (player != NULL && player->Rating < 0) {
	player->Rating = theirRating;
	if (abs(theirRating - Me->Rating) > 1) {
	    GetExactRating(player); /* try again with correct handicap */
	    return;
	}
    }

    /* Check that the handicap was correctly chosen: */
    if (fabs(probLose - 0.5) > 0.4) return;

    if (probLose < 0.5) {
	/* probLose = 0.5*(0.75 ^ 2.*Badvantage);
	 * log(probLose) = log(0.5) + 2.*Badvantage * log(0.75)
	 */
	Badvantage = (log(probLose) - log(0.5))/(2.*log(0.75));
    } else {
	/* probLose = 1.0 - 0.5*(0.75 ^ 2.*(-Badvantage));
	 * log(1.0 - probLose) = log(0.5) - 2.*Badvantage * log(0.75)
	 */
	Badvantage = (log(0.5) - log(1.0 - probLose))/(2.*log(0.75));
    }
    if (useBlack) hand = -hand;

    if (player == NULL) {
	/* theirRating is exact, set my own: */
	exactRating = (double)theirRating + Badvantage + hand; 
	player = Me;

    } else if (Me->ExactRating > 0.) {
	exactRating = Me->ExactRating - Badvantage - hand;

    } else if (!Me->AutoRated) {
	if (myRating != Me->Rating) {
	    printf("myRating %d, Me->Rating %d\n", myRating, Me->Rating);
	    fflush(stdout);
	    return;
	}
	exactRating = (double)myRating - Badvantage - hand;
    } else {
	return;
    }

    if (DebugFun) {
	printf("Badvantage %f, exactRating %f\n", Badvantage, exactRating);
	fflush(stdout);
    }
    if (fabs(exactRating - player->Rating) < 1.0) {
	player->ExactRating = exactRating;
	RefreshRating(player);
    } else {
	printf("exactRating %f, player->Rating %d\n",
	       exactRating, player->Rating);
	fflush(stdout);
    }
}

void StoredNum(Player *player, int stored)
{
    player->Stored = stored;
    RefreshStored(player);
}

/*****************************************************************************/

int StrengthCompare(const Player *player1, const Player *player2)
/* Returns < 0 if player1 is stronger to ensure sort by decreasing strength */
{
#ifdef OLD
    const char *Strength1, *Strength2, *ptr1, *ptr2;
    int   temp;

    Strength1 = player1->Strength;
    Strength2 = player2->Strength;

    temp = Strength1[strlen(Strength1)-1];
    ptr1 = strchr(StrengthOrder, temp);
    if (!ptr1) ptr1 = StrengthOrder+1;

    temp = Strength2[strlen(Strength2)-1];
    ptr2 = strchr(StrengthOrder, temp);
    if (!ptr2) ptr2 = StrengthOrder+1;

    if (ptr1 < ptr2) return -1;
    if (ptr1 > ptr2) return  1;
    if (*ptr1 == 'p' || *ptr1 == 'd') {
        temp = atoi(Strength1) - atoi(Strength2);
        if (temp < 0) return  1;
        if (temp > 0) return -1;
    } else if (*ptr1 == 'k') {
        temp = atoi(Strength1) - atoi(Strength2);
        if (temp < 0) return -1;
        if (temp > 0) return  1;
    }
    return 0;
#else
    if (player1->Rating != player2->Rating) {
	return player2->Rating - player1->Rating;
    }
    return player2->AutoRated - player1->AutoRated;
#endif
}

int PlayerCompare(const Player *player1, const Player *player2)
/* Compares by importance, strength or name. For the first two,
 * returns < 0 if player1 is stronger to ensure sort by decreasing strength,
 * and ties are broken by name
 */
{
#ifdef OLD
    const char *Strength1, *Strength2, *ptr1, *ptr2;
    int   temp;

    if (SortMethod == SORTSTRENGTH) {
        /* Difference with StrengthCompare(...) : Tie is broken by name */
        Strength1 = player1->Strength;
        Strength2 = player2->Strength;

        temp = Strength1[strlen(Strength1)-1];
        ptr1 = strchr(StrengthOrder, temp);
        if (!ptr1) ptr1 = StrengthOrder+1;

        temp = Strength2[strlen(Strength2)-1];
        ptr2 = strchr(StrengthOrder, temp);
        if (!ptr2) ptr2 = StrengthOrder+1;

        if (ptr1 < ptr2) return -1;
        if (ptr1 > ptr2) return  1;
        if (*ptr1 == 'p' || *ptr1 == 'D'|| *ptr1 == 'd') {
            temp = atoi(Strength1) - atoi(Strength2);
            if (temp < 0) return  1;
            if (temp > 0) return -1;
        } else if (*ptr1 == 'k') {
            temp = atoi(Strength1) - atoi(Strength2);
            if (temp < 0) return -1;
            if (temp > 0) return  1;
        }
    }
    return XmuCompareISOLatin1(player1->Name, player2->Name);
#else
    if (SortMethod == SORTIMPORTANCE) {
	if (player1->Importance != player2->Importance) {
	    return player2->Importance - player1->Importance;
	}
    }
    if (SortMethod != SORTNAME) {
	if (player1->Rating != player2->Rating) {
	    return player2->Rating - player1->Rating;
	}
	if (player1->AutoRated != player2->AutoRated) {
	    return player2->AutoRated - player1->AutoRated;
	}
    }
    /* Sort in decreasing importance but increasing names */
    return XmuCompareISOLatin1(player1->Name, player2->Name);
#endif
}

/* to be used in qsorts */
int PlayersCompare(const void *player1, const void *player2)
{
    return PlayerCompare(*(const Player **) player1,
                         *(const Player **) player2);
}

static void DumpPlayer(const Player *player)
{
    Output ("-------Player Dump------\n");
    Outputf("Player %s\n", PlayerString(player));
    Outputf("Me = " PRTPTR ", Next = " PRTPTR ", Previous = " PRTPTR
            ", Results = " PRTPTR "\n",
            player, player->Next, player->Previous, player->Results);
    Outputf("Name = %s, Strength = %s, AutoRated = %d\n",
            player->Name, player->Strength, player->AutoRated);
    Outputf("Rating = %d, Importance = %d, ExactRating = %f\n",
            player->Rating, player->Importance, player->ExactRating);
    Outputf("Idle = %s, Info = %s, Country = %s, Won = %d, Lost = %d\n",
            player->Idle, player->Info, player->Country,
            player->Won, player->Lost);
    Outputf("Language = %s, First observed = %d, Playing = %d\n",
            player->Language, player->FirstObserving, player->Playing);
    Outputf("Pos = %d, OnServer = %d, XgospelUser = %d, Found = %d\n",
            player->Pos, player->OnServer, player->XgospelUser, player->Found);
    Outputf("Game = %s, Tell = " PRTPTR ", Match = " PRTPTR "\n",
            player->Game ? GameLongDescription(player->Game) : NULLNAME,
            player->Tell, player->Match);
    Outputf("Widget = " PRTPTR ", WidgetPlan = %d\n",
            player->Widget, player->WidgetPlan);
}

void DumpPlayers(const char *args)
{
    const Player *player;

    for (player = PlayerBase.Next; player != &PlayerBase;
         player = player->Next) DumpPlayer(player);
}

static void SelectPlayer(Widget w, XtPointer clientdata, XtPointer calldata)
{
    Player *player;
    String  text;

    if (!calldata) Raise1(AssertException, "You shouldn't have a normal "
                          "notify() in the player widget translations");
    text   = (String)   calldata;
    player = (Player *) clientdata;

    if      (strcmp(text, TELLPLAYER) == 0) {
        CallGetTell(0, (XtPointer) player, NULL);
    } else if (strcmp(text, CHALLENGEPLAYER) == 0) {
        CallGetChallenge(w, (XtPointer) player, NULL);
    } else if (strcmp(text, STATSPLAYER) == 0) {
        CallGetStats(w, (XtPointer) player, NULL);
    } else if (strcmp(text, OBSERVEPLAYER) == 0) {
        CallPlayerObserve(w, (XtPointer) player, NULL);
    } else if (strcmp(text, MARKPLAYER) == 0) {
	TogglePlayerMark(player);
    } else if (strcmp(text, DUMPPLAYER) == 0) { /* DumpPlayer(player) buggy */;
    } else {
	Warning("Unknown argument to DoPlayer\n");
    }
}

void UnassumePlayers(Connection conn)
{
    Player *player;

    for (player = PlayerBase.Next; player != &PlayerBase;
         player = player->Next) if (player->XgospelUser == XGOSPELUSER)
             player->XgospelUser = EXXGOSPELUSER;
}

void TellXgospelUsers(Connection conn, const char *message)
{
    Player *player;
    
    for (player = PlayerBase.Next; player != &PlayerBase;
         player = player->Next)
        if (player->XgospelUser == XGOSPELUSER && player != Me)
            LastCommand(conn, "tell %s %s", player->Name, message);
}

void NewXgospelUser(Connection conn, const Player *user)
{
/*
    Player *player;

    if (user == Me) return;
    for (player = PlayerBase.Next; player != &PlayerBase;
         player = player->Next)
        if (player->XgospelUser == XGOSPELUSER && player != user)
            UserSendCommand(conn, NULL, "tell %s " XGOSPELESCAPE
                        "USER %s VERSION ????", user->Name, player->Name);
*/
}

void ShowPlayerStats(void)
{
    char Text[80];

    if (nrplayers > maxplayers) maxplayers = nrplayers;
    sprintf(Text, "%d player%s (max %d), %d game%s\n",
            nrplayers, nrplayers == 1 ? "" : "s", maxplayers,
            nrgames, nrgames == 1 ? "" : "s");
    XtVaSetValues(playerstats, XtNlabel, (XtArgVal) Text, NULL);
}

static void ChangeStrength(Player *player)
{
    if (DebugFun) {
        printf("ChangeStrength(%s)\n", PlayerString(player));
        fflush(stdout);
    }
    player->Rating = StrengthToRating(player->Strength);
    player->Importance = PlayerImportance(player);

    /* Must be repositioned now to keep the rest sorted */
    if (player->Widget) WidgetPlanReposition(player, PlayersPlan);
    if (player->Tell)  SetTellDescription( player->Tell);
    if (player->Match) SetMatchDescription(player->Match);
    if (player->Stats) SetStatsDescription(player->Stats);
    if (player->Game)  ChangeGameDescription(player->Game);
}

void ResetPlayersImportance(void)
{
    Player *player;

    for (player = PlayerBase.Next;
         player != &PlayerBase;
         player = player->Next) {
	player->Importance = -1;      /* force recomputing */
	ShowPlayer(player, True);
    }
}

static void ShowPlayer(Player *player, int newStatus)
{
    int oldImportance = player->Importance;

    if (DebugFun) {
        printf("ShowPlayer(%s) importance %d\n", PlayerString(player),
	       oldImportance);
        fflush(stdout);
    }
    if (!player->OnServer) return;

    /* oldImportance < 0 for new or reconnected player, or when simpleNames
     * was changed. Force recomputing the Importance in this case, or
     * when simpleNames is false and the status changed:
     */
    if (oldImportance < 0 || (newStatus && !appdata.SimpleNames)) {

	player->Rating = StrengthToRating(player->Strength);
	player->Importance = PlayerImportance(player);

	if (DebugFun && oldImportance != player->Importance) {
	    printf("%s: new importance %d\n", PlayerString(player),
		   player->Importance);
	    fflush(stdout);
	}
#if 0 /* sort at any change too slow */
	if (player->Widget && player->Importance != oldImportance
	    && SortMethod == SORTIMPORTANCE) {
	    WidgetPlanReposition(player, PlayersPlan);
	}
        /* if (player->Game)  ChangeGameDescription(player->Game);
	 * game not yet initialized for a new game
	 */
#endif
    }

    if (player->Widget) player->WidgetPlan = CREATE | DELETE;
    else                player->WidgetPlan = CREATE;
    WidgetPlanRefresh(PlayersPlan);
    player->Found = UNCHANGED;
}

static void DeletePlayer(Player *player)
{
    if (DebugFun) {
        if (player) printf("DeletePlayer(%s)\n", PlayerString(player));
        else printf("DeletePlayer(NULL)\n");
        fflush(stdout);
    }
    if (player) {
        if (player->Widget) {
            player->WidgetPlan = DELETE;
            WidgetPlanRefresh(PlayersPlan);
        } else player->WidgetPlan = 0;
        if (playerinfo) AddText(playerinfo, "%16s disconnected\n",
                                PlayerString(player));
        if (player->Tell) TellMessage(player->Tell, "player disconnected");
        Logoff(player);
        nrplayers--;
        player->OnServer = 0;
    }
}

static void DoPlayer(Widget w, XEvent *evnt, String *str, Cardinal *n)
{
    if (*n) XtCallCallbacks(w, XtNcallback, (XtPointer) str[0]);
    else    XtCallCallbacks(w, XtNcallback, (XtPointer) "none");
}

/*****************************************************************************/

void  UserData(NameListList *Users)
{
    int           i, Count, Played, Work;
    NameListList *Here;
    NameList     *Name;
    Player      **All, **Ptr, *Who;
    char         *Strength, ShortState[3], *state;

    Count = 0;
    for (Here = Users->Next; Here != Users; Here = Here->Next) Count++;

    Ptr = All = mynews(Player *, Count);
    WITH_UNWIND {
        for (Here = Users->Next; Here != Users; Here = Here->Next) {
            Name = Here->Names->Next; /* Name */
            *Ptr++ = Who  = NameToPlayer(Name->Name);
            Name = Name->Next;  /* Info */
            CHANGESTRING(Who, Info, Name->Name,
                         /* player->Found = CHANGED */;);
            Name = Name->Next;  /* Country */
            CHANGESTRING(Who, Country, Name->Name,
                         /* player->Found = CHANGED */;);
            Name = Name->Next;  /* Rank */
            Strength = Name->Name;
            Name = Name->Next;  /* Won */
            Who->Won = atoi(Name->Name);
            Name = Name->Next;  /* Lost */
            Who->Lost = atoi(Name->Name);
            Name = Name->Next;  /* First game being observed */
            Work =  atoi(Name->Name);
            CHANGEINT(Who, FirstObserving, Work, Who->Found = CHANGED);
            Name = Name->Next;  /* (First) game being played */
            Work =  atoi(Name->Name);
            CHANGEINT(Who, Playing, Work, Who->Found = CHANGED);
            Name = Name->Next;  /* Idle time */
            CHANGESTRING(Who, Idle, Name->Name,
                         /* Who->Found = CHANGED */;);
            Name = Name->Next;  /* Flags */
            state = Name->Name;
            if (state[0] == '-') ShortState[0] = ' ';
            else                 ShortState[0] = state[0];
            if (state[1] == '-') ShortState[1] = ' ';
            else                 ShortState[1] = state[1];
            ShortState[2] = 0;
            CHANGESTRING(Who, State, ShortState, Who->Found = CHANGED);
            Name = Name->Next;  /* Language */
            CHANGESTRING(Who, Language,Name->Name,
                         /* Who->Found = CHANGED */;);
            CheckPlayerStrength(Who, Strength);
            if (Who->Won < 0 || Who->Lost < 0) {
                /* Remove player just logging on */
                Ptr--;
                Count--;
            }
        }

        qsort((void *) All, (size_t) Count, sizeof(*All), PlayersCompare);

        Output("     Name        Fl Obs Pl      Info      Country Won/Lost "
               "      Idle Language\n");
        Ptr = All;
        for (i=0; i<Count; i++, Ptr++) {
            Who = *Ptr;
            Outputf("  %s %-14.14s %-7s %4d/%-4d(",
                    SetPlayerEntry(NULL, NULL, Who), Who->Info, Who->Country,
                    Who->Won, Who->Lost);
            Played = Who->Won + Who->Lost;
            if (Played == 0) Output("----) ");
            else Outputf("%3d%%) ", (200*Who->Won+Played)/(2*Played));
            Outputf("%3s %s\n", Who->Idle, Who->Language);
        }
    } ON_UNWIND {
        myfree(All);
    } END_UNWIND;
}

void AssertPlayersDeleted(void)
{
    Player *player;

    for (player = PlayerBase.Next;
         player != &PlayerBase;
         player = player->Next)
        player->Found = DELETED;
}

void TestPlayersDeleted(void)
{
    Player *player, *next;

    for (player = PlayerBase.Next;
         player != &PlayerBase;
         player = next) {
        next = player->Next;
        switch(player->Found) {
          case NEW:
          case CHANGED:
            ShowPlayer(player, True);
            break;
          case DELETED:
            if (player->OnServer) DeletePlayer(player);
            break;
          case UNCHANGED: /* Can also mean: already (re)displayed */
            break;
          default:
            Raise1(AssertException, "Impossible WhoState");
            break;
        }
    }
}

void PlayerStatusLine(int Players, int MaxPlayers, int Games)
{
/*  nrplayers  = Players; */
    if (MaxPlayers > 0 && MaxPlayers > maxplayers) {
        maxplayers = MaxPlayers;
        ShowPlayerStats();
    }
    if (nrgames != Games && appdata.GamesTimeout > 0) {
        AutoCommand(NULL, "games");
    }
}

const char *PlayerToStrength(const Player *player)
{
    return player ? player->Strength : NULL;
}

int PlayerToImportance(const Player *player)
{
    return player ? player->Importance : -1;
}

int PlayerToStored(const Player *player)
{
    return player ? player->Stored : -1;
}

const char *PlayerToName(const Player *player)
{
    return player ? player->Name : NULL;
}

const char *PlayerName(const Player *player)
/* same as PlayerToName but removes final '*' */
{
    static char Name[80];
    char *p;

    if (!player) return NULL;

    strcpy(Name, player->Name);
    p = strchr(Name, '*');
    if (p) *p = '\0';
    return Name;
}

const char *PlayerToAutoRated(const Player *player)
{
    switch(player->AutoRated) {
      case 0:  return " ";
      case 1:  return "*";
    }
    return "?";
}

const char *PlayerToIdle(const Player *player)
{
    return player ? player->Idle : NULL;
}

const NameList *PlayerToResults(const Player *player)
{
    return player ? player->Results : NULL;
}

int OnServerP(const Player *player)
{
    return player->OnServer;
}

int GuestP(const Player *player)
{
    return GuestNameP(player->Name);
}

extern Tell **_PlayerTellAddress(Player *player);
Tell **_PlayerTellAddress(Player *player)
{
    return &player->Tell;
}

extern Match **_PlayerMatchAddress(Player *player);
Match **_PlayerMatchAddress(Player *player)
{
    return &player->Match;
}

extern Stats **_PlayerStatsAddress(Player *player);
Stats **_PlayerStatsAddress(Player *player)
{
    return &player->Stats;
}

extern int *_PlayerXgospelUser(Player *player);
int *_PlayerXgospelUser(Player *player)
{
    return &player->XgospelUser;
}
