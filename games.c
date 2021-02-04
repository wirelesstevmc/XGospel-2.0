/**********************************************************/
/* games.c: The part of xgospel keeping the list of games */
/*          up to date and to enter/remove data in the    */
/*          game record (which itself is kept in gospel.c */
/*                                                        */
/* Author:  Ton Hospel                                    */
/*          ton@linux.cc.kuleuven.ac.be                   */
/*          (1993, 1994, 1995)                            */
/*                                                        */
/* Copyright: GNU copyleft                                */
/**********************************************************/

#include <X11/StringDefs.h>
#include <X11/Intrinsic.h>
#include <X11/Shell.h>
#include <X11/Xaw/Paned.h>
#include <X11/Xaw/Toggle.h>
#include <X11/Xaw/StripChart.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <stddef.h>

#include <mymalloc.h>
#include <except.h>
#include <myxlib.h>
#include <YShell.h>

#include "analyze.h"
#include "games.h"
#include "utils.h"
#include "xgospel.h"
#include "gamesP.h"

extern struct tm UniversalTime;

#define PRTPTR    "#%p"        /* Nice outputformat for a pointer          */

#define MOVES_PER_BYO 25

Widget GamesButton, gameinfo;
int    nrgames;
int    minImportantRating = 50;

#ifndef   HAVE_NO_STDARG_H
static void RemoveGame(Game *game, const char *Comment, ...);
#else  /* HAVE_NO_STDARG_H */
static void RemoveGame();
#endif /* HAVE_NO_STDARG_H */
static void DoGame(Widget w, XEvent *evnt, String *str, Cardinal *n);
static XtActionsRec GamesActionTable[] = {
    { (String) "dogame", DoGame },
};

static Game GameBase   = { &GameBase,   &GameBase };

static ConvertToWidget GameToWidget;
extern ConvertToWidget PlayerToWidget;
static Game           *MyGame;
static WidgetPlan      GamesPlan;
static Widget          RaiseOnImportant, BeepOnImportant;
static int             ForceNew = -1;
static int             WantGames = 0;
static int             Trailing = -1; /* server id of last trailed game */

HashEntry gameHash[HASH_SIZE], playerHash[HASH_SIZE];

/* Current move number (-1 on first node, 0 on initial position) */
CommentFun MoveFun = {
    NULL,
    NULL,
};

/* Encoded last move (undo corrected, undefined on first node,
   (-1,-1) for any setup position (like the initial),
   (-1, 0) for pass, (0, -handicap) for handicap) */
CommentFun LastMoveFun = {
    NULL,
    NULL,
};

/* Number of next node  (undefined on last node) */
CommentFun NextMoveFun = {
    NULL,
    NULL,
};

/* Number of previous node (undefined on first node) */
CommentFun PrevMoveFun = {
    NULL,
    NULL,
};

static void  ShowGame(Game *game);
static void  SetTime(Game *game);
static Game *CompareMoves(Game *game, NameVal *moves);

static void SetNrGames(Widget w, XtPointer client_data, XtPointer value)
{
    *(double *) value = ((double) nrgames) / appdata.GamesScale;
}

/*****************************************************/
/* Applying regular expressions to widget contents   */
/*****************************************************/

/* This function assumes there is something to free */
void FreeToWidget(ConvertToWidget *Convert)
{
    int i;
    ToWidget *Here;

    Here = Convert->Convert;
    for (i=Convert->NrConvert+1; i>0; i--, Here++)
        REGPREFIX(free)(&Here->Pattern);
    myfree(Convert->Convert);
    Convert->Convert   = NULL;
    Convert->NrConvert = -1;
}

const char *CompileToWidget(struct _StringPairList *Base,
                            ConvertToWidget *Convert)
{
    int i, Nr, Length, error;
    size_t  size;
    char  Buffer[100], *Pat, *Ptr;
    ToWidget *Here;
    static char Error[300];

    if (Convert->Convert) FreeToWidget(Convert);
    if (!Base) return NULL;

    Nr   = Base->Nr;
    Length = Nr*3;
    Here = Convert->Convert = mynews(ToWidget, Nr+1);
    for (i = 0; i<Nr; i++, Here++) {
        Here->Name       = Base->String2[i];
        Here->NameLength = Base->Length2[i];
        error = REGPREFIX(comp)(&Here->Pattern, Base->String1[i],
                                REG_EXTENDED | REG_NEWLINE | REG_NOSUB);
        if (error) {
            size = REGPREFIX(error)(error, &Here->Pattern,
                                    Buffer, sizeof(Buffer));
            sprintf(Error, "Regular expression '%.100s': %.180s%s",
                    Base->String1[i], Buffer,
                    size > sizeof(Buffer) ? "..." : "");
            Convert->NrConvert = i-1;
            FreeToWidget(Convert);
            return Error;
        }
        Length += Base->Length1[i];
    }
    Pat = Ptr = mynews(char, Length);
    for (i = 0; i<Nr; i++) {
        *Ptr++ = '(';
        memcpy(Ptr, Base->String1[i], Base->Length1[i]);
        Ptr += Base->Length1[i];
        *Ptr++ = ')';
        *Ptr++ = '|';
    }
    Ptr[-1] = 0;
    error = REGPREFIX(comp)(&Here->Pattern, Pat,
                            REG_EXTENDED | REG_NEWLINE | REG_NOSUB);
    Convert->NrConvert = Nr;
    if (error) {
        size = REGPREFIX(error)(error, &Here->Pattern, Buffer, sizeof(Buffer));
        sprintf(Error, "Combined regular expression '%.100s': %.180s%s",
                Pat, Buffer, size > sizeof(Buffer) ? "..." : "");
        Convert->NrConvert--;
        FreeToWidget(Convert);
        myfree(Pat);
        return Error;
    }
    myfree(Pat);
    return NULL;
}

void InitHash()
{
    const char *Error;

    memset(gameHash, sizeof(gameHash), 0);
    memset(playerHash, sizeof(playerHash), 0);

    /* If simpleNames we initialize once for all the player hash table,
     * which remains constant until simpleNames is changed.
     */
    if (appdata.SimpleNames) {
        Error = CompileToHash(appdata.PlayerToWidget, playerHash,
			      "playerEntry", &PlayerToWidget);
	if (Error) {
	    Warning("InitHash: %s\n", Error);
	    appdata.SimpleNames = False;
	}
    }
}

unsigned hash_x33(unsigned char *s)
{
    /* Adapted from a hashing package by Seltzer & Yigit, where the `times-33'
       idea was attributed to Chris Torek.
     */
    unsigned h = 0;
    while (*s) {
        h = (h << 5) + h + *s++;
    }
    return h & (HASH_SIZE - 1);
}

const char *CompileToHash(struct _StringPairList *Base,
			  HashEntry *hashTable,
			  const char *Prefix, ConvertToWidget *Convert)
{
    int i, Nr;
    static char Error[300];
    unsigned h;
    char *name;

    if (!Base || !hashTable) return NULL;

    memset(hashTable, 0, sizeof(HashEntry)*HASH_SIZE);

    Nr   = Base->Nr;
    for (i = 0; i<Nr; i++) {

	h = hash_x33((unsigned char*)Base->String1[i]);

	if (hashTable[h].description) {
	    sprintf(Error, "Hash conflict: '%s' and '%s'",
		    hashTable[h].description, Base->String1[i]);
	    return Error;
	}
	name = (char*)RegexGetType(Prefix, Convert, Base->String1[i]);
	hashTable[h].description = mystrdup(Base->String1[i]);
	hashTable[h].content = mystrdup(name);
    }
    return NULL;
}

const char *RegexGetGameType(const char *Description)
/* This function is called only when *simpleNames is false.
   Description is the complete line from the Games window, but
   we keep only the part with the two players names and strengths
   to optimize the hash table.
 */
{
    char buf[256];
    char *name;
    unsigned h;

    /* skip game number and spaces */
    while (*Description && *Description++ != ')') ;
    while (*Description && *Description == ' ') Description++;
    strcpy(buf, Description);

    /* skip move number and rest */
    name = buf;
    while (*name && *name != '(') name++;
    if (*name == '(') *name = ' ';
    while (name != buf && *name == ' ') name--;
    name[1] = '\0';

    h = hash_x33((unsigned char*)buf);

    if (gameHash[h].description &&
	!strcmp(gameHash[h].description, buf)) {
        return gameHash[h].content;
    }
    name = (char*)RegexGetType("gameEntry", &GameToWidget, buf);
    myfree(gameHash[h].description);
    myfree(gameHash[h].content);
    gameHash[h].description = mystrdup(buf);
    gameHash[h].content = mystrdup(name);
    return name;
}

const char *RegexGetPlayerType(const char *Description)
{
    char buf[256];
    char *name;
    unsigned h;

    /* skip spaces */
    while (*Description && *Description == ' ') Description++;
    strcpy(buf, Description);

    h = hash_x33((unsigned char*)buf);

    if (playerHash[h].description &&
	!strcmp(playerHash[h].description, buf)) {
        return playerHash[h].content;
    }
    if (appdata.SimpleNames) return NULL;

    name = (char*)RegexGetType("playerEntry", &PlayerToWidget, buf);
    myfree(playerHash[h].description);
    myfree(playerHash[h].content);
    playerHash[h].description = mystrdup(buf);
    playerHash[h].content = mystrdup(name);
    return name;
}

const char *RegexGetType(const char *Prefix, ConvertToWidget *Convert,
                         const char *Description)
{
    int error, i, ch;
    size_t size;
    char *ptr;
    static char Buffer[100];

    if (!Convert->Convert) return NULL;

    error = REGPREFIX(exec)(&Convert->Convert[Convert->NrConvert].Pattern,
                            Description, 0, NULL, 0);
    if (error == REG_NOMATCH) return NULL;
    else if (error) {
        size = REGPREFIX(error)(error,
                                &Convert->Convert[Convert->NrConvert].Pattern,
                                Buffer, sizeof(Buffer));
        Warning("ToWidget: Test string '%.100s': %s%s\n",
                Description, Buffer, size > sizeof(Buffer) ? "..." : "");
    } else {
        for (i=0; i<Convert->NrConvert; i++) {
            error = REGPREFIX(exec)(&Convert->Convert[i].Pattern,
                                    Description, 0, NULL, 0);
            if (error == REG_NOMATCH) continue;
            else if (error) {
                size = REGPREFIX(error)(error, &Convert->Convert[i].Pattern,
                                        Buffer, sizeof(Buffer));
                Warning("ToWidget: Sub test string '%.100s': %s%s\n",
                        Description, Buffer,
                        size > sizeof(Buffer) ? "..." : "");
            } else {
                sprintf(Buffer, "%.*s%.*s", (int) sizeof(Buffer)/2, Prefix,
                        (int) sizeof(Buffer)/2-1, Convert->Convert[i].Name);
                for (ptr = Buffer; (ch = *ptr) != 0; ptr++)
                    if (!isalnum(ch) && ch != '-') *ptr = '_';
                return Buffer;
            }
        }
        Raise2(AssertException, ExceptionCopy(Description),
               "matches general pattern but not any specific one");
    }
    return NULL;
}

/**********************************************/
/* Batch update of a widgets children         */
/**********************************************/

struct _WidgetPlan {
    WidgetPlan next, previous;

    /* Base of doubly lined circular list to consider */
    char  *Base;

    /* Offset of several fields in an entry */
    size_t Next, WidgetPlan, Widget, Pos;

    /* Parent of the widgets to add/delete */
    Widget Collect;

    /* Witchet containing collect. This is where we will add the new ones */
    Widget Root;

    /* Function to compare entries. (cast to (void *) via (Widget *)) */
    int  (*Compare)(const void *entry1, const void *entry2);

    /* Function to convert entry to the name its widget should get.
       Also fills in extra X resources (e.g. label) */
    const char * (*Properties)(Cardinal *i, Arg *arg,
                               void *entry, XtPointer Closure);

    /* Function to call when entry gets completely destroyed */
    void (*Destroy)(void *Entry, XtPointer Closure);

    /* Function to call when entry gets created */
    void (*Create)(void *Entry, XtPointer Closure);

    /* Function to call when update is done */
    void (*Done)(int NrEntries, XtPointer Closure);

    /* is update routine posted ? (0: no, otherwise contains routine Id */
    XtWorkProcId WidgetProc;

    /* Time to next timeout, 0 means not running */
    int Update;

    /* Where to get the update rate */
    int *Rate;

    /* Do we need an update ? */
    int  WantUpdate;

    /* Number of widgets currently displayed */
    int  NrEntries;
    /* Extra info you would like to be stored.
       Mainly meant for the Properties function */
    XtPointer Closure;
};

static struct _WidgetPlan BasePlan = { &BasePlan, &BasePlan };
static MyContext          PlanContext;
static XrmQuark           WidgetToPlanEntry;
static int                PlanPos;

int PlanInsert(Widget w)
{
    return PlanPos;
}

WidgetPlan AllocWidgetPlan(void *Base, size_t Next, size_t Pos,
                           size_t widgetPlan, size_t widget, int *Rate,
                           Widget Collect, XtPointer Closure,
                           int  (*Compare)(const void *entry1,
                                           const void *entry2),
                           const char * (*Properties)(Cardinal *i, Arg *arg,
                                                      void *entry,
                                                      XtPointer Closure),
                           void (*Create) (void *Entry, XtPointer Closure),
                           void (*Destroy)(void *Entry, XtPointer Closure),
                           void (*done)(int NrEntries, XtPointer Closure))
{
    WidgetPlan Result;

    Result = mynew(struct _WidgetPlan);
    Result->Base  = (char *) Base;
    Result->Next  = Next;
    Result->WidgetPlan   = widgetPlan;
    Result->Widget       = widget;
    Result->Pos          = Pos;
    Result->Root         = WitchetOfWidget(Collect);
    Result->Collect      = Collect;
    Result->Compare      = Compare;
    Result->Properties   = Properties;
    Result->Create       = Create;
    Result->Destroy      = Destroy;
    Result->Done         = done;
    Result->WidgetProc   = 0;
    Result->Update       = 0;
    Result->Rate         = Rate;
    Result->WantUpdate   = 0;
    Result->NrEntries    = 0;
    Result->Closure      = Closure;

    XtVaSetValues(Collect, XtNinsertPosition, (XtArgVal) PlanInsert, NULL);

    Result->next     = &BasePlan;
    Result->previous =  BasePlan.previous;
    Result->previous->next = Result->next->previous = Result;

    return Result;
}

void FreeWidgetPlan(WidgetPlan Plan)
{
    Plan->previous->next = Plan->next;
    Plan->next->previous = Plan->previous;
    if (Plan->WidgetProc) XtRemoveWorkProc(Plan->WidgetProc);
    myfree(Plan);
}

#define Get(x, offname, type) (*(type *) &(x)[Plan->offname])
static Boolean WorkWidgetPlan(XtPointer Closure)
{
    int         j, ToCreate, ToDelete;
    XtPointer   Data;
    char       *Entry;
    Widget     *Create, *Cre, *Delete, *Del, *Children, Temp;
    Cardinal    i, NrChildren;
    Arg         arg[5];
    const char *Name;
    WidgetPlan  Plan;

    Plan = (WidgetPlan) Closure;

    ToCreate = ToDelete = 0;
    for (Entry = (char *) Get(Plan->Base, Next, void *); Entry != Plan->Base;
         Entry = (char *) Get(Entry, Next, void *))
        switch(Get(Entry, WidgetPlan, int)) {
           case CREATE:
             ToCreate++;
             break;
           case DELETE:
             ToDelete++;
             break;
           case CREATE | DELETE:
             ToDelete++;
             ToCreate++;
             break;
         }

    XtVaGetValues(Plan->Collect,
                  XtNchildren,    (XtArgVal) &Children,
                  XtNnumChildren, (XtArgVal) &NrChildren,
                  NULL);
    Create = mynews(Widget, ToCreate);
    WITH_UNWIND {
        Delete = mynews(Widget, ToDelete);
        WITH_UNWIND {
            Cre = Create;
            Del = Delete;
            for (Entry = (char *) Get(Plan->Base, Next, void *);
                 Entry != Plan->Base;
                 Entry = (char *) Get(Entry, Next, void *))
                switch(Get(Entry, WidgetPlan, int)) {
                   case CREATE:
                     *Cre++ = (Widget) Entry;
                     break;
                   case DELETE:
                     *Del++ = (Widget) Entry;
                     break;
                   case CREATE | DELETE:
                     *Cre++ = (Widget) Entry;
                     *Del++ = (Widget) Entry;
                     break;
            }
            if (ToCreate) {
                qsort(Create, (size_t)ToCreate, sizeof(Widget), Plan->Compare);
                j = 0;
                for (i=0; i<NrChildren; i++) {
                    if (MyFindContext(PlanContext, Children[i],
                                      WidgetToPlanEntry, &Data))
                        Raise1(AssertException,
                               "Could not find plan entry for widget");
                    Temp = (Widget) (char *) Data;
                    while (Plan->Compare(&Temp, &Create[j]) > 0) {
                        Get((char *) Create[j], Pos, int) = i+j;
                        if (++j >= ToCreate) goto done;
                    }
                }
                for (;j<ToCreate; j++)
                    Get((char *) Create[j], Pos, int) = NrChildren+j;
            }
          done:
            for (j=0; j<ToDelete; j++) {
                Entry     = (char *) Delete[j];
                Delete[j] = Get(Entry, Widget, Widget);
                MyDeleteContext(PlanContext, Get(Entry, Widget, Widget),
                                WidgetToPlanEntry);
                Plan->NrEntries--;
                Get(Entry, Widget, Widget) = 0;
                if (Get(Entry, WidgetPlan, int) == DELETE) {
                    Get(Entry, WidgetPlan, int) = 0;
                    if (Plan->Destroy) Plan->Destroy(Entry, Plan->Closure);
                } else Get(Entry, WidgetPlan, int) = 0;
            }
            for (j=0; j<ToCreate; j++) {
                Entry = (char *) Create[j];
                Get(Entry, WidgetPlan, int) = 0;
                i = 0;
                Name = Plan->Properties(&i, arg, Entry, Plan->Closure);
                XtSetArg(arg[i], (String) MyNname, (XtArgVal) Name); i++;
                PlanPos = Get(Entry, Pos, int);
                Get(Entry, Widget, Widget) = Create[j] =
                    MyCreateWidget("entry", Plan->Root, arg, i);
                Plan->NrEntries++;
                /* Maybe combine with CreateWidget */
                if (Plan->Create) Plan->Create(Entry, Plan->Closure);
                MySaveContext(PlanContext, Get(Entry, Widget, Widget),
                              WidgetToPlanEntry, (XtPointer) Entry);
            }
            SetManagementChildren(Create, ToCreate, Delete, ToDelete);
            for (j=0; j<ToDelete; j++) XtDestroyWidget(Delete[j]);
/*          for (j=0; j<ToCreate; j++) XtManageChild(Create[j]); */
        } ON_UNWIND {
            myfree(Delete);
        } END_UNWIND;
    } ON_UNWIND {
        myfree(Create);
    } END_UNWIND;
    if (Plan->Done) Plan->Done(Plan->NrEntries, Plan->Closure);
    Plan->WidgetProc = 0;
    Plan->Update     = *Plan->Rate;
    return True;
}

void WidgetPlanResort(WidgetPlan Plan)
{
    char *Entry;
    int   changes;

    changes = 0;
    for (Entry = (char *) Get(Plan->Base, Next, void *); Entry != Plan->Base;
         Entry = (char *) Get(Entry, Next, void *))
        if (Get(Entry, Widget, Widget) && Get(Entry, WidgetPlan, int) == 0) {
            Get(Entry, WidgetPlan, int) = DELETE | CREATE;
            changes = 1;
        }
    if (changes) {
        WidgetPlanRefreshNow(Plan);
    }
}

void WidgetPlanReposition(void *entry, WidgetPlan Plan)
{
    char       *Entry, *en;
    const char *Name;
    Widget      Temp1, Temp2;
    Arg         arg[5];
    Cardinal    i;

    Entry = entry;

    XtDestroyWidget(Get(Entry, Widget, Widget));
    MyDeleteContext(PlanContext, Get(Entry, Widget, Widget),
                    WidgetToPlanEntry);
    Get(Entry, Widget, Widget) = 0;
    Temp1 = (Widget) Entry;
    PlanPos = 0;
    for (en = (char *) Get(Plan->Base, Next, void *);
         en != Plan->Base;
         en = (char *) Get(en, Next, void *))
        if (Get(en, Widget, Widget)) {
            if (Get(en, Pos, int) > Get(Entry, Pos, int)) Get(en, Pos, int)--;
            Temp2 = (Widget) en;
            if (Plan->Compare(&Temp1, &Temp2) > 0) PlanPos++;
            else Get(en, Pos, int)++;
        }
    Get(Entry, Pos, int) = PlanPos;
    i = 0;
    Name = Plan->Properties(&i, arg, Entry, Plan->Closure);
    XtSetArg(arg[i], (String) MyNname, (XtArgVal) Name); i++;
    Get(Entry, Widget, Widget) =
        MyCreateManagedWidget("entry", Plan->Root, arg, i);
    if (Plan->Create) Plan->Create(Entry, Plan->Closure);
    MySaveContext(PlanContext, Get(Entry, Widget, Widget), WidgetToPlanEntry,
                  (XtPointer) Entry);
}

void WidgetPlanRefresh(WidgetPlan Plan)
{
    if (Plan->WidgetProc || Plan->WantUpdate) return;
    if (Plan->Update) Plan->WantUpdate = 1;
    else Plan->WidgetProc =
        XtAppAddWorkProc(AppContext(Plan->Collect),
                         WorkWidgetPlan, (XtPointer) Plan);
}

void WidgetPlanRefreshNow(WidgetPlan Plan)
{
    if (Plan->WidgetProc) return;
    if (Plan->Update) {
        Plan->Update = 0;
        Plan->WantUpdate = 0;
    }
    Plan->WidgetProc = XtAppAddWorkProc(AppContext(Plan->Collect),
                                        WorkWidgetPlan, (XtPointer) Plan);
}
#undef Get

static void WidgetPlanTime(unsigned long diff)
{
    WidgetPlan Here;

    for (Here = BasePlan.next; Here != &BasePlan; Here = Here->next)
        if (Here->Update) {
            Here->Update -= diff;
            if (Here->Update <= 0) {
                Here->Update = 0;
                if (Here->WantUpdate) {
                    Here->WantUpdate = 0;
                    WidgetPlanRefresh(Here);
                }
            }
        }
}

/**********************************************/
/* The games functions proper                 */
/**********************************************/

static int GameCompare(const Game *game1, const Game *game2);
static int WidgetGameCompare(const void *game1, const void *game2)
{
    /* On machines where this idiotic series of casts is necessary, the
       method to get the addresses is probably invalid anyways, but let's
       pretend... */
    return GameCompare((Game *)(void *)(char *)*(Widget *)game1,
                       (Game *)(void *)(char *)*(Widget *)game2);
}

static void ToggleObserve(Widget w, XtPointer clientdata, XtPointer calldata);
static void GameWidgetCreate(void *Entry, XtPointer Closure)
{
    Game *game;

    game = (Game *) Entry;
    XtAddCallback(game->Widget, XtNcallback, ToggleObserve,
                  (XtPointer) game);
}

static void GameWidgetDestroy(void *Entry, XtPointer Closure)
{
    Game *game;

    game = (Game *) Entry;
    if (!game->Observers) FreeGame(game);
}

static void GameWidgetDone(int NrEntries, XtPointer Closure)
{
    nrgames = NrEntries;
    ShowPlayerStats();
}

#define PutPos(pos, pat, value)         \
do {                                    \
    ptr = strchr(ptr, 0);               \
    width = pos-(ptr-Text);             \
    if (width <= 0) width = 1;          \
    sprintf(ptr, pat, width, value);    \
} while(0);

static const char *SetGameEntry(Cardinal *i, Arg *args, const Game *game)
{
    static char Text[80];
    char        Temp1[80], Temp2[80], *ptr;
    const char *BlackName, *BlackStrength, *WhiteName, *WhiteStrength;
    int         width;

    BlackName     = PlayerToName(game->Black);
    BlackStrength = PlayerToStrength(game->Black);
    WhiteName     = PlayerToName(game->White);
    WhiteStrength = PlayerToStrength(game->White);

    sprintf(Temp1, "%s %3s%1s", BlackName, BlackStrength,
	    PlayerToAutoRated(game->Black));
    sprintf(Temp2, "%3s%1s %s", WhiteStrength, PlayerToAutoRated(game->White),
	    WhiteName);
    sprintf(Text, "(%3d) %16s vs %-16s (", game->ServerId, Temp1, Temp2);
    ptr = Text;

    PutPos(47, "%*d ", game->Move);
    PutPos(50, "%*d ", game->XSize);
    PutPos(52, "%*d ", game->Handicap);
    PutPos(56, "%*s ", game->Komi);
    PutPos(60, "%*d ", game->ByoPeriod);
    PutPos(62, "%*c",  game->Mode);
    PutPos(63, "%*c) (", game->Rules);
    PutPos(69, "%*d)", game->NrObservers);

    XtSetArg(args[*i], (String) XtNlabel, (XtArgVal) Text); (*i)++;
    XtSetArg(args[*i], (String) XtNstate, (XtArgVal)
             (game->WantObserved > 0 ? True : False)); (*i)++;
    return Text;
}
#undef PutPos

static const char *GameProperties(Cardinal *i, Arg *arg,
                                  void *Entry, XtPointer Closure)
{
    Game       *game;
    const char *Name;

    game = (Game *) Entry;
    Name = SetGameEntry(i, arg, game);
    if (!appdata.SimpleNames) {
        Name = RegexGetGameType(Name);
    }
    if (appdata.SimpleNames || Name == NULL) {
        Name = RegexGetPlayerType(PlayerToName(game->Strongest));
        if (!Name) Name = RegexGetPlayerType(PlayerToName(game->Weakest));
	if (Name) {
	    /* replace playerEntryXxx with gameEntry */
	    static char nam[256];
	    strcpy(nam, "game");
	    strcpy(nam+4, Name+6);
	    return nam;
	}
    }
    if (!Name) Name = GetPlayerType(game->Strongest, "gameEntry");
    return Name;
}

void GamesResort(void)
{
    WidgetPlanResort(GamesPlan);
}

static enum _SortMethod_ {
    SORTIMPORTANCE, SORTSTRENGTH, SORTNUMBER
} SortMethod = SORTIMPORTANCE;

static void CallSort(Widget w, XtPointer clientdata, XtPointer calldata)
{
    if ((Boolean)XTPOINTER_TO_INT(calldata) != False)
        if (SortMethod != (enum _SortMethod_) XTPOINTER_TO_INT(clientdata)) {
            SortMethod = (enum _SortMethod_) XTPOINTER_TO_INT(clientdata);
            GamesResort();
        }
}

void InitGames(Widget Toplevel)
{
    Widget  GameRoot, GameCollect, GamesWidget, GameStrip;
    Widget  GameSortImportance, GameSortStrength, GameSortNumber;
    Widget  AllowResize;
    Boolean state;
    const char *Error;

    GameToWidget.Convert   = NULL;
    GameToWidget.NrConvert = -1;
    Error = CompileToWidget(appdata.GameToWidget, &GameToWidget);
    if (Error) Warning("CompileToWidget: %s\n", Error);

    nrgames    = 0;
    MyGame     = NULL;
    WantGames  = 0;

    WidgetToPlanEntry = XrmPermStringToQuark("WidgetToPlanEntry");
    PlanContext       = YShellContext(Toplevel);

    XtAppAddActions(AppContext(Toplevel), GamesActionTable,
                    XtNumber(GamesActionTable));

    GameRoot = MyVaCreateManagedWidget("games", Toplevel, NULL);

    GamesWidget      = XtNameToWidget(GameRoot, "*set");
    GameCollect      = XtNameToWidget(GameRoot, "*collect");
    gameinfo         = XtNameToWidget(GameRoot, "*info");
    GameStrip        = XtNameToWidget(GameRoot, "*strip");
    if (GameStrip) XtAddCallback(GameStrip, XtNgetValue, SetNrGames, NULL);

    if (GamesButton) {
        XtAddCallback(GamesButton, XtNcallback,        CallToggleUpDown,
                      (XtPointer) GameRoot);
        XtAddCallback(GameRoot,    XtNpopupCallback,   CallToggleOn,
                      (XtPointer) GamesButton);
        XtAddCallback(GameRoot,    XtNpopdownCallback, CallToggleOff,
                      (XtPointer) GamesButton);
    }

    GamesPlan =
        AllocWidgetPlan(&GameBase, offsetof(Game, Next), offsetof(Game, Pos),
                        offsetof(Game, WidgetPlan), offsetof(Game, Widget),
                        &appdata.GameUpdateTimeout, GamesWidget,
                        (XtPointer) "games", WidgetGameCompare,
                        GameProperties, GameWidgetCreate, GameWidgetDestroy,
                        GameWidgetDone);

    GameSortImportance = XtNameToWidget(GameRoot, "*sortImportance");
    if (GameSortImportance) {
        XtAddCallback(GameSortImportance, XtNcallback,
                      CallSort, INT_TO_XTPOINTER((int) SORTIMPORTANCE));
        XtVaGetValues(GameSortImportance, XtNstate, (XtArgVal) &state, NULL);
        if (state != False)
            CallSort(GameSortImportance, INT_TO_XTPOINTER((int)SORTIMPORTANCE),
                     INT_TO_XTPOINTER((int) state));
    }
    GameSortStrength = XtNameToWidget(GameRoot, "*sortStrength");
    if (GameSortStrength) {
        XtAddCallback(GameSortStrength, XtNcallback,
                      CallSort, INT_TO_XTPOINTER((int) SORTSTRENGTH));
        XtVaGetValues(GameSortStrength, XtNstate, (XtArgVal) &state, NULL);
        if (state != False)
            CallSort(GameSortStrength, INT_TO_XTPOINTER((int) SORTSTRENGTH),
                     INT_TO_XTPOINTER((int) state));
    }
    GameSortNumber = XtNameToWidget(GameRoot, "*sortNumber");
    if (GameSortNumber) {
        XtAddCallback(GameSortNumber, XtNcallback,
                      CallSort, INT_TO_XTPOINTER((int) SORTNUMBER));
        XtVaGetValues(GameSortNumber, XtNstate, (XtArgVal) &state, NULL);
        if (state != False)
            CallSort(GameSortNumber, INT_TO_XTPOINTER((int) SORTNUMBER),
                     INT_TO_XTPOINTER((int) state));
    }

    RaiseOnImportant = XtNameToWidget(GameRoot, "*raise");
    BeepOnImportant = XtNameToWidget(GameRoot, "*beep");
    minImportantRating = StrengthToRating(appdata.MinImportantRank);

    AllowResize = XtNameToWidget(GameRoot, "*allowResize");
    if (AllowResize) {
        XtVaGetValues(AllowResize, XtNstate, (XtArgVal) &state, NULL);
        XtVaSetValues(GameRoot, XtNallowShellResize, (XtArgVal) state, NULL);
        XtAddCallback(AllowResize, XtNcallback, CallAllowShellResize,
                      (XtPointer) GameRoot);
        if (GamesWidget) {
            XtVaSetValues(GamesWidget, XtNallowResize, (XtArgVal) state, NULL);
            XtAddCallback(AllowResize, XtNcallback, CallAllowResize,
                          (XtPointer) GamesWidget);
        }
    }

    XtRealizeWidget(GameRoot);
    XtInstallAllAccelerators(GameCollect,      GameCollect);
    DeleteProtocol(GameRoot);
    if (GamesButton) CallToggleUpDown(GamesButton, (XtPointer) GameRoot, NULL);
}

void CleanGames(void)
{
    if (GameToWidget.Convert) FreeToWidget(&GameToWidget);
    FreeWidgetPlan(GamesPlan);
}

void CheckMyKomi(const char *Komi)
{
    Gamelog *Log;

    if (MyGame) {
        Log = MyGame->Log;
        CHANGESTRING(MyGame, Komi, Komi, PROPSTRING(Komi, "Komi");
                     SetKomi(MyGame->Observers, Komi); ShowGame(MyGame));
    } else Warning("but I can't even find your game\n");
}

void SetGameTitle(Game *game, const char *Title)
{
    /* CHANGESTRING(game, Title, Title, PROPSTRING(Title, "Name")); */
    char    *NewTitle;
    Gamelog *Log;
    size_t   Length;

    if ( game->Title &&  Title && strcmp(Title, game->Title) == 0) return;
    if (!game->Title && !Title) return;

    if (Title) {
        Length   = strlen(Title);
        NewTitle = mystrndup(Title, Length);
    } else {
        NewTitle = NULL;
        Length   = 0;
    }
    WITH_HANDLING {
        Log = game->Log;
        if (Log) {
#define TITLE "Name"
            if (game->Title) DeleteGlobalProperty(Log, TITLE);
            if (NewTitle)    AddGlobalProperty(Log, TITLE, NewTitle, -1);
#undef TITLE
        }
    } ON_EXCEPTION {
        myfree(NewTitle);
    } END_HANDLING;
    myfree(game->Title);
    game->Title       = NewTitle;
    game->TitleLength = Length;
    SetTitle(game->Observers, NewTitle);
}

void SetMyGameTitle(const char *title)
{
    if (TeachingP(MyGame)) {
	SetGameTitle(MyGame, title);
    } else {
	Warning("Title set to '%s', "
		"but you are not playing a teaching game\n", title);
    }
}

void StopMyGameForward(const char *message)
{
    if (TeachingP(MyGame)) {
	StopForward(MyGame->Observers, NumberNodes(MyGame->Log)-1);
    } else {
	Warning("Received '%s' but you are not playing a teaching game\n",
		message);
    }
}

/* Called with game->Log == 0 */
static void InitGamelog(Game *game)
{
    Gamelog *log;
    char     Buffer[2048];
    const char *Host;

    log = AllocGamelog(game->XSize, game->YSize, GameAllowSuicide(game));
    AddComment(log, &MoveFun,     INT_TO_XTPOINTER(-1));
    AddComment(log, &NextMoveFun, INT_TO_XTPOINTER(1));
    SetStones(log, NULL);
    AddComment(log, &LastMoveFun, LastFromXY(game, -1, -1));
    AddComment(log, &PrevMoveFun, INT_TO_XTPOINTER(0));
    AddComment(log, &MoveFun,     INT_TO_XTPOINTER(0));
    game->Log = log;

#define AddProp(Name, Value) AddGlobalProperty(log, Name, Value, -1)
    AddProp("BlackName",     PlayerToName(game->Black));
    AddProp("WhiteName",     PlayerToName(game->White));
    AddProp("BlackStrength", PlayerToStrength(game->Black));
    AddProp("WhiteStrength", PlayerToStrength(game->White));
    AddProp("Komi",          game->Komi);
    if (game->Handicap) {
        sprintf(Buffer, "%d", game->Handicap);
        AddProp("Handicap", Buffer);
    }
    if (game->Title) AddProp("Name", game->Title);
    AddProp("EnteredBy",     UserId);
    Host = LogonSite(NULL);
    if (Host) {
        sprintf(Buffer, "IGS(%.*s)", (int) sizeof(Buffer)-10, Host);
        AddProp("Place", Buffer);
    } else AddProp("Place", "IGS");
    strftime(Buffer, sizeof(Buffer), SgfDateFormat,
             &game->UniversalTime);
    Buffer[sizeof(Buffer)-1] = 0;
    AddProp("Date", Buffer);
#undef AddProp
}

static void FoundGame(Game *game, int Id, int MyPlay,
                      Player *BLack, Player *WHite, int Move,
                      int Handicap, const char *Komi,
                      int ByoPeriod, int Mode, int Rules, int NrObservers)
{
    Gamelog    *Log;
    char        Buffer[256];
    const char *bStrength, *wStrength;
    int         DoCompareEmpty;

    bStrength = PlayerToStrength(BLack);
    wStrength = PlayerToStrength(WHite);

    DoCompareEmpty = 0;
    game->Found    = UNCHANGED;
    if (MyPlay) {
        MyGame = game;
        game->Observed = game->WantObserved = 1;
    }
    if (game->ServerId < 0) {
        game->Found = NEW;
        if (game->WantObserved > 0) {
            if (game == MyGame)
                if (Move) AutoCommand(NULL, "moves %d", Id);
                else DoCompareEmpty = 1;
            else AutoCommand(NULL, "observe %d", Id);
            UserActive(NULL);
        }
        goto AsChange;
    } else if (game->Black != BLack || game->White != WHite) {
        game->Found = CHANGED;
	if (appdata.WantVerbose) {
            Warning("Game %d, players renamed\n", Id);
	}
	game->Black = BLack;
	game->White = WHite;
	ChangeGameDescription(game);
        goto AsChange;
    } else if (game->ServerId != Id) {
        game->Found = CHANGED;
        if (appdata.WantVerbose) {
            Warning("Game moved from %d to %d\n", game->ServerId, Id);
        }
      AsChange:
        game->ServerId = Id;
        PlayerInGame(BLack, game, Id);
        if (BLack != WHite) PlayerInGame(WHite, game, Id);
        if (game->Black2) PlayerInGame(game->Black2, game, Id);
        if (game->White2) PlayerInGame(game->White2, game, Id);
    }
    if (DoCompareEmpty) game = CompareMoves(game, NULL);
    Log = game->Log;
    if (Log) DeleteGlobalProperty(Log, "Result");
    CHANGEINT(game, Move,        Move,;);
    CHANGEINT(game, Handicap,    Handicap, PROPHANDICAP(Handicap);
              SetHandicap(game->Observers, Handicap));
    CHANGEINT(game, NrObservers, NrObservers,;);
    CHANGEINT(game, Mode,        Mode,;);
    CHANGEINT(game, ByoPeriod,   ByoPeriod,;);
    CHANGESTRING(game, Komi, Komi, PROPSTRING(Komi, "Komi");
                 SetKomi(game->Observers, Komi));
    PROPSTRING(bStrength, "BlackStrength");
    PROPSTRING(wStrength, "WhiteStrength");
    if ((game->Finished & ~SCORING) == ADJOURNED)
        GameMessage(game, "----------", "Game assumed "
                    "resumed at move %d", Move);
    game->Finished &= ~OVER;     /* Changes ADJOURNED to FINISHED */
}

static void AssumeAdjourn(Game *game)
{
    RemoveGame(game, "assumed adjourned");
    game->Finished |= OVER; /* Changes BUSY to ADJOURNED */
}

Game *FindGame(int Id, Player *BLack, Player *WHite, int Move,
               size_t XSize, size_t YSize, int Handicap, const char *Komi,
               int ByoPeriod, int Mode, int Rules, int NrObservers)
{
    int         MyPlay;
    const char *white, *black;
    Game       *Ptr, *BestPtr, *prefPtr = NULL;

    black     = PlayerToName(BLack);
    white     = PlayerToName(WHite);

    if (DebugFun) {
        printf(
          "FindGame(%d %s-%s, mv %d, %dx%d, h%d, k%s, b%d, r%d, ob%d, fn%d)\n",
	  Id, black, white, Move, (int) XSize, (int) YSize, Handicap, Komi,
	  ByoPeriod, Rules, NrObservers, ForceNew);
        fflush(stdout);
    }

    MyPlay = (BLack == Me || WHite == Me);

    BestPtr = NULL;
    /* Still force incorrect extra game if force happens to be first in an
       incorrectly matched games command -- Ton */
    if (ForceNew >= 0 && Id != ForceNew) ForceNew = -1;
    if (ForceNew < 0) {
        for (Ptr = GameBase.Next; Ptr != &GameBase; Ptr = Ptr->Next) {
	    if (Ptr->ServerId == Id) prefPtr = Ptr;
            if ((Ptr->Black == BLack || Ptr->Black2 == BLack) &&
               (Ptr->White == WHite || Ptr->White2 == WHite) &&
                Ptr->XSize == XSize && Ptr->YSize == YSize &&
                Ptr->Rules == Rules &&
                Ptr->Move-5 <= Move && /* stops unhandled matches, remove when
                                          match matching is improved --Ton */
	        (Ptr->Finished & BUSY) /* Finished is BUSY or ADJOURNED */) {
                if (Ptr->ServerId == Id) {
		    MyPlay |= (Ptr->Black2 == Me || Ptr->White2 == Me);
                    FoundGame(Ptr, Id, MyPlay, BLack, WHite, Move, Handicap,
                              Komi, ByoPeriod, Mode, Rules, NrObservers);
                    return Ptr;
                } else if (BestPtr) {
                    if (Ptr->Move >= Move) {
                        if (BestPtr->Move < Move || BestPtr->Move > Ptr->Move)
                            BestPtr = Ptr;
                    } else if (BestPtr->Move < Ptr->Move) {
			BestPtr = Ptr;
		    }
                } else {
		    BestPtr = Ptr;
		}
	    }
        }
        if (BestPtr) {
	    MyPlay |= (BestPtr->Black2 == Me || BestPtr->White2 == Me);
            FoundGame(BestPtr, Id, MyPlay, BLack, WHite, Move, Handicap, Komi,
                      ByoPeriod, Mode, Rules, NrObservers);
            return BestPtr;
        }
	/* deal with teaching games renamed with the "name" command: */
	if (prefPtr != NULL && prefPtr->Teacher != NULL) {
	    if (BLack == WHite) {
		BLack = prefPtr->Black;
		WHite = prefPtr->White;
	    }
            FoundGame(prefPtr, Id, MyPlay, BLack, WHite, Move, Handicap, Komi,
                      ByoPeriod, Mode, Rules, NrObservers);
	    return prefPtr;
	}
    }

    if (DebugFun) {
        printf("FindGame failed, creating game entry\n");
        fflush(stdout);
    }

    Ptr = ServerIdToGame(Id);
    if (Ptr) AssumeAdjourn(Ptr);

    Ptr = mynew(Game);
    Ptr->Black     = BLack;
    Ptr->White     = WHite;
    Ptr->Black2    = 0;
    Ptr->White2    = 0;
    Ptr->Teacher   = BLack == WHite ? BLack : 0;
    if (StrengthCompare(BLack, WHite) < 0) {
        Ptr->Strongest = BLack;
        Ptr->Weakest   = WHite;
    } else {
        Ptr->Strongest = WHite;
        Ptr->Weakest   = BLack;
    }
    PlayerInGame(BLack, Ptr, Id);
    if (BLack != WHite) PlayerInGame(WHite, Ptr, Id);
    Ptr->Log       = NULL;
    Ptr->Observers = NULL;
    Ptr->ServerId  = Id;
    Ptr->Next      = GameBase.Next;
    Ptr->Previous  = &GameBase;
    Ptr->Previous->Next = Ptr->Next->Previous = Ptr;
    Ptr->Komi          = mystrdup(Komi);
    Ptr->Move          = Move;
    Ptr->Handicap      = Handicap;
    Ptr->Title         = 0;
    Ptr->TitleLength   = 0;
    if (NrObservers > 0 && Id == Trailing) {
        /* Match observed before being announced: */
        Ptr->Observed = Ptr->WantObserved  = 1;
	if (DebugFun) {
            printf("new trailed game %d %s %s\n", Id, black, white);
	    fflush(stdout);
	}
	Trailing = -1;
    } else {
        Ptr->Observed = Ptr->WantObserved  = 0;
    }        
    Ptr->NrObservers   = NrObservers;
    Ptr->XSize         = XSize;
    Ptr->YSize         = YSize;
    Ptr->UniversalTime = UniversalTime;
    Ptr->Widget        = 0;
    Ptr->WidgetPlan    = 0;
    Ptr->Pos           = 0;
    Ptr->Mode          = Mode;
    Ptr->Rules         = Rules;
    Ptr->Finished      = BUSY;
    Ptr->WhiteCaptures = Ptr->BlackCaptures = 0;
    Ptr->oldWhiteCaptures = Ptr->oldBlackCaptures = 0;
    Ptr->Found         = NEW;
    Ptr->WhiteTime = Ptr->BlackTime = 90*60;
    Ptr->ByoPeriod     = ByoPeriod;
    Ptr->BlackByo      = Ptr->WhiteByo = NOBYO;
    Ptr->ToMove        = Empty;
#ifndef NO_CLIENT_TIME
    Ptr->MoveTime      = -1;
#endif
    /* The next lines assume Empty = 0, Black = 1, White = 2 --Ton */
    Ptr->Color = Empty;
    if (BLack == Me) Ptr->Color |= Black;
    if (WHite == Me) Ptr->Color |= White;
    if (Ptr->Color != Empty) {
        MyGame = Ptr;
        Ptr->Observed = Ptr->WantObserved = 1;
        if (Move) AutoCommand(NULL, "moves %d", Id);
        else {
#ifndef NO_CLIENT_TIME
	  if (BLack == Me) Ptr->MoveTime = time(NULL);
#endif
	  Ptr->ToMove = Black;
       }
    }
    if (Ptr->Observed) {
        InitGamelog(Ptr);
	if (Move == 0) ShowObserve(Ptr); /* otherwise wait for "moves" */
        SendCommand(NULL, NULL, "time %d", Id);

    } else if (PlayerToImportance(BLack) >= minImportantRating ||
	       PlayerToImportance(WHite) >= minImportantRating) {
	IfBell(BeepOnImportant);
	IfRaise(RaiseOnImportant, RaiseOnImportant);
    }
    return Ptr;
}

void TeamGame(int Id, const char *black, const char *white,
	      const char *black2, const char *white2, int move)
{
    Game       *Ptr;

    if (DebugFun) {
        printf("TeamGame(%d, %s, %s, %s, %s) move %d\n",
               Id, black, white, black2, white2, move);
        fflush(stdout);
    }

    Ptr = ServerIdToGame(Id);
    if (Ptr) AssumeAdjourn(Ptr);

    Ptr = mynew(Game);
    Ptr->Black     = PlayerFromName(black);
    Ptr->White     = PlayerFromName(white);
    Ptr->Black2    = PlayerFromName(black2);
    Ptr->White2    = PlayerFromName(white2);
    Ptr->Teacher   = 0;

    if (StrengthCompare(Ptr->Black, Ptr->White) < 0) {
        Ptr->Strongest = Ptr->Black;
        Ptr->Weakest   = Ptr->White;
    } else {
        Ptr->Strongest = Ptr->White;
        Ptr->Weakest   = Ptr->Black;
    }
    PlayerInGame(Ptr->Black, Ptr, Id);
    PlayerInGame(Ptr->White, Ptr, Id);
    PlayerInGame(Ptr->Black2, Ptr, Id);
    PlayerInGame(Ptr->White2, Ptr, Id);
    Ptr->Log       = NULL;
    Ptr->Observers = NULL;
    Ptr->ServerId  = Id;
    Ptr->Next      = GameBase.Next;
    Ptr->Previous  = &GameBase;
    Ptr->Previous->Next = Ptr->Next->Previous = Ptr;
    Ptr->Komi          = mystrdup("5.5");
    Ptr->Move          = move;
    Ptr->Handicap      = 0;
    Ptr->Title         = 0;
    Ptr->TitleLength   = 0;
    Ptr->Observed      = 0;
    Ptr->WantObserved  = 0;
    Ptr->NrObservers   = 0;
    Ptr->XSize         = 19;
    Ptr->YSize         = 19;
    Ptr->UniversalTime = UniversalTime;
    Ptr->Widget        = 0;
    Ptr->WidgetPlan    = 0;
    Ptr->Pos           = 0;
    Ptr->Mode          = ' ';
    Ptr->Rules         = 'I';
    Ptr->Finished      = BUSY;
    Ptr->WhiteCaptures = Ptr->BlackCaptures = 0;
    Ptr->oldWhiteCaptures = Ptr->oldBlackCaptures = 0;
    Ptr->Found         = NEW;
    Ptr->WhiteTime = Ptr->BlackTime = 90*60;
    Ptr->ByoPeriod     = 15;
    Ptr->BlackByo      = Ptr->WhiteByo = NOBYO;
    Ptr->ToMove        = Empty;
#ifndef NO_CLIENT_TIME
    Ptr->MoveTime      = -1;
#endif
    /* The next lines assume Empty = 0, Black = 1, White = 2 --Ton */
    Ptr->Color = Empty;
    if (Ptr->Black == Me || Ptr->Black2 == Me) Ptr->Color |= Black;
    if (Ptr->White == Me || Ptr->White2 == Me) Ptr->Color |= White;
    if (Ptr->Color != Empty) {
        MyGame = Ptr;
        InitGamelog(Ptr);
        Ptr->Observed = Ptr->WantObserved = 1;
        if (Ptr->Move) AutoCommand(NULL, "moves %d", Id);
        else {
#ifndef NO_CLIENT_TIME
	  if (Ptr->Black == Me) Ptr->MoveTime = time(NULL);
#endif
	  Ptr->ToMove = Black;
	  SendCommand(NULL, NULL, "time %d", Id);
	  ShowObserve(Ptr);
       }
    }
    if (!Ptr->Observed &&
	(PlayerToImportance(Ptr->White) >= minImportantRating  ||
	 PlayerToImportance(Ptr->Black) >= minImportantRating  ||
         PlayerToImportance(Ptr->White2) >= minImportantRating ||
	 PlayerToImportance(Ptr->Black2) >= minImportantRating)) {
        IfBell(BeepOnImportant);
        IfRaise(RaiseOnImportant, RaiseOnImportant);
    }
}


static void GameNotFound(const char *comment)
{
    if (Entered) Warning("Games database inconsistency %s\n", comment);
    /* Try to get a more sensible database */
    if (appdata.GamesTimeout > 0) AutoCommand(NULL, "games");
}

static Game *PlayersToGame(int PreferredId,
                           const char *black, const char *white, int Finished)
{
    Game *Ptr, *BestPtr = NULL, *prefPtr = NULL;
    Player *BLack, *WHite;

    if (DebugFun) {
        printf("PlayersToGame(%d, %s, %s, %d)\n",
               PreferredId, black, white, Finished);
        fflush(stdout);
    }

    if (PreferredId) {
        BLack = NameToPlayer(black);
        WHite = NameToPlayer(white);
    } else {
        BLack = PlayerFromName(black);
        WHite = PlayerFromName(white);
    }
    for (Ptr = GameBase.Next; Ptr != &GameBase; Ptr = Ptr->Next) {
        if ((Ptr->Finished & UNSCORING) != Finished) continue;
	if (Ptr->ServerId == PreferredId) prefPtr = Ptr;
	if ((Ptr->Black == BLack || Ptr->Black2 == BLack) &&
	    (Ptr->White == WHite || Ptr->White2 == WHite)) {
            if (Ptr->ServerId == PreferredId) return Ptr;
            else if (!BestPtr) BestPtr = Ptr;
	}
    }
    /* deal with teaching games renamed with the "name" command: */
    if (BestPtr == NULL && prefPtr != NULL && prefPtr->Teacher != NULL) {
	if (prefPtr->Black != BLack || prefPtr->White != WHite) {
	    prefPtr->Black = BLack;
	    prefPtr->White = WHite;
	    ChangeGameDescription(prefPtr);
	}
	return prefPtr;
    }
    return BestPtr;
}

Game *IdPlayersToGame(int Id, const char *black, const char *white)
{
    Game *game;

    game = PlayersToGame(Id, black, white, BUSY);
    if (game) {
        if (game->ServerId != Id) {
            if (Entered && !strchr(black, '*') && appdata.WantVerbose) {
	        /* check for name* because teaching games have no match
                 * announcement
                 */
                Warning("Received information about game %d (%s vs %s)\n"
                        "But this is inconsistent with my database.\n",
                        Id, black, white);
	    }
            if (appdata.GamesTimeout > 0) AutoCommand(NULL, "games");
        }
    } else {
        if (Entered && !strchr(black, '*') && appdata.WantVerbose &&
            /* As long as teaching games have no match announcement --Ton */
            strcmp(black, white))
            Warning("Received information about game %d (%s vs %s)\n"
                    "But this game is not in my database.\n",
                    Id, black, white);
        if (appdata.GamesTimeout > 0) AutoCommand(NULL, "games");
    }
    return game;
}

Game *ServerIdToGame(int nr)
{
    Game *Ptr;

    for (Ptr = GameBase.Next; Ptr != &GameBase; Ptr = Ptr->Next)
        if (Ptr->ServerId == nr) return Ptr;
    return NULL;
}

static Game *ServerIdGame(int nr)
{
    Game *Ptr;

    Ptr = ServerIdToGame(nr);
    if (!Ptr) GameNotFound("for serverid to game");
    return Ptr;
}

int NewMatch(int Id, const Player *Player1, const Player *Player2)
{
    char  Buffer[80];

    strcpy(Buffer, PlayerString(Player1));
    if (gameinfo) AddText(gameinfo, "New game, %16s vs %16s\n",
                          Buffer, PlayerString(Player2));
    SendCommand(NULL, INT_TO_XTPOINTER(Id+1), "games %d", Id);
    /* INT_TO_XTPOINTER(Id+1) will set ForceNew = Id in AssertGamesDeleted() */
    return Id;
}

void DumpGame(const Game *game)
{
    Output ("-------Game Dump------\n");
    Outputf("Game %d (%s)\n", game->ServerId, GameLongDescription(game));
    Outputf("Me = " PRTPTR ", Next = " PRTPTR ", Previous = " PRTPTR
            ", Log = " PRTPTR ", Observers = " PRTPTR "\n",
            game, game->Next, game->Previous, game->Log, game->Observers);
    Outputf("Game title = '%s', titleLenth = %d\n",
            (game->Title ? game->Title : "(null)"), game->TitleLength);
    Outputf("Observed = %d, WantObserved = %d, NrObservers = %d, "
            "BlackByo = %d, WhiteByo = %d\n",
            game->Observed, game->WantObserved, game->NrObservers,
            game->BlackByo, game->WhiteByo);
    Outputf("BlackCaptures = %d, WhiteCaptures = %d\n",
            game->BlackCaptures, game->WhiteCaptures);
    Outputf("Player Black = " PRTPTR ", White = " PRTPTR ", Strongest = "
            PRTPTR ", Weakest = " PRTPTR "\n",
            game->Black, game->White, game->Strongest, game->Weakest);
    Outputf("BlackTime = %ld, WhiteTime = %ld, ByoPeriod = %d\n",
            game->BlackTime, game->WhiteTime, game->ByoPeriod);
    Outputf("XSize = %d, YSize = %d\n", (int) game->XSize, (int) game->YSize);
    Outputf("Komi = %s, Handicap = %d, Move = %dPos = %d\n",
            game->Komi, game->Handicap, game->Move, game->Pos);
    Outputf("Finished = %d, Mode = '%c', Rules = %c, Found = %d\n",
            game->Finished, game->Mode, game->Rules, game->Found);
    Outputf("UniversalTime = %s", asctime(&game->UniversalTime));
    Outputf("Widget = " PRTPTR ", WidgetPlan = %d, ToMove = %d, Color = %d\n",
            game->Widget, game->WidgetPlan, game->ToMove, game->Color);
}

void DumpGames(const char *args)
{
    const Game *game;

    for (game = GameBase.Next; game != &GameBase; game = game->Next)
        DumpGame(game);
}

static void DoGame(Widget w, XEvent *evnt, String *str, Cardinal *n)
{
    if (*n) XtCallCallbacks(w, XtNcallback, (XtPointer) str[0]);
    else    XtCallCallbacks(w, XtNcallback, (XtPointer) "none");
}

static void ToggleObserve(Widget w, XtPointer clientdata, XtPointer calldata)
{
    Game    *game;
    String   text;

    UserActive(NULL);
    game = (Game *) clientdata;
    text = (String) calldata;
    if (strcmp(text, GAME_OBSERVE) == 0) {
        if (game == MyGame) {
            ShowGame(game);
            if (!game->Observers) ShowObserve(game);
        } else
            if (game->WantObserved > 0) {
                if (game->Observed)
                    SendCommand(NULL, NULL, "unobserve %d", game->ServerId);
                game->WantObserved = 0;
            } else {
                if (!game->Observed)
                    SendCommand(NULL, NULL, "observe %d", game->ServerId);
                game->WantObserved = 1;
            }
    } else if (strcmp(text, GAME_STATUS) == 0)
        SendCommand(NULL, NULL, "status %d", game->ServerId);
    else if (strcmp(text, GAME_OBSERVERS) == 0)
        SendCommand(NULL, (XtPointer) 1, "all %d", game->ServerId);
    else if (strcmp(text, GAME_PLAYERS) == 0) {
        SendCommand(NULL, NULL, "stats %s", PlayerName(game->White));
        SendCommand(NULL, NULL, "stats %s", PlayerName(game->Black));
    } else if (strcmp(text, GAME_PLAYERS_MAIN) == 0) {
        SendCommand(NULL, NULL, "%%literal stats %s", PlayerName(game->White));
        SendCommand(NULL, NULL, "%%literal stats %s", PlayerName(game->Black));
    } else if (strcmp(text, DUMPGAME) == 0) DumpGame(game);
    else WidgetWarning(w, "Unknown argument to dogame");
}

void StartObserve(Game *game)
{
    if (game->ServerId >= 0 && game->Color == Empty) {
        if (game->WantObserved <= 0) game->WantObserved = 1;
        if (!game->Observed)
            SendCommand(NULL, NULL, "observe %d", game->ServerId);
    }
}

void StopObserve(Game *game)
{
    if (game->ServerId >= 0 && game->Color == Empty) {
        if (game->WantObserved > 0) game->WantObserved = 0;
        if (game->Observed)
            SendCommand(NULL, NULL, "unobserve %d", game->ServerId);
    }
}

int MyFirstObserved(void)
{
    Game *game;
    int   Result;
    
    Result = -1;
    for (game = GameBase.Next; game != &GameBase; game = game->Next)
        if (game->Observed)
            if (Result < 0) Result = game->ServerId;
    /* Maybe we can do better than this if we know how the server sorts --Ton*/
            else return -2;
    return Result;
}

static int GameCompare(const Game *game1, const Game *game2)
{
    int Rc;

    if (SortMethod == SORTIMPORTANCE) {
	int imp1 = PlayerToImportance(game1->Strongest);
	int imp2 = PlayerToImportance(game2->Strongest);

	if (imp1 < PlayerToImportance(game1->Weakest)) {
	    imp1 = PlayerToImportance(game1->Weakest);
	}
	if (imp2 < PlayerToImportance(game2->Weakest)) {
	    imp2 = PlayerToImportance(game2->Weakest);
	}
        if (imp1 != imp2) return imp2 - imp1;
    }
    if (SortMethod != SORTNUMBER) {
        Rc = StrengthCompare(game1->Strongest, game2->Strongest);
        if (Rc) return  Rc;
        Rc = StrengthCompare(game1->Weakest,   game2->Weakest);
        if (Rc) return  Rc;
        /* We don't update for change in this condition ! */
        Rc = game1->NrObservers - game2->NrObservers;
        if (Rc) return -Rc;
    }
    /* Sort by decreasing strength but increasing game number */
    return game1->ServerId - game2->ServerId;
}

void FreeGame(Game *game)
{
    game->Next->Previous = game->Previous;
    game->Previous->Next = game->Next;

    DestroyObservers(&game->Observers);
    myfree(game->Komi);
    FreeGamelog(game->Log);
    if (game->Title) myfree(game->Title);
    myfree(game);
}

static void ShowGame(Game *game)
{
    if (DebugFun) {
        printf("ShowGame(%2d (%s))\n",
               game->ServerId, GameLongDescription(game));
        fflush(stdout);
    }
    if (game->ServerId < 0) return;

    if (game->Observed != game->WantObserved && game != MyGame)
        if (game->WantObserved > 0) {
            AutoCommand(NULL, "observe %d", game->ServerId);
            UserActive(NULL);
        } else if (game->WantObserved < 0) {
            if (game->Observed) game->WantObserved  = 1;
        } else {
            AutoCommand(NULL, "unobserve %d", game->ServerId);
            UserActive(NULL);
        }

    if (game->Widget) game->WidgetPlan = CREATE | DELETE;
    else              game->WidgetPlan = CREATE;
    WidgetPlanRefresh(GamesPlan);
    game->Found = UNCHANGED;
}

static void DeleteGame(Game *game)
{
    if (DebugFun) {
        printf("DeleteGame(%2d (%s))\n",
               game->ServerId, GameLongDescription(game));
        fflush(stdout);
    }

    if (game == MyGame) MyGame = NULL;
    if (game->Widget) {
        game->WidgetPlan = DELETE;
        WidgetPlanRefresh(GamesPlan);
    } else if (!game->Observers) FreeGame(game);
}

#ifndef   HAVE_NO_STDARG_H
static void RemoveGame(Game *game, const char *Comment, ...)
#else  /* HAVE_NO_STDARG_H */
static void RemoveGame(va_alist)
va_dcl
#endif /* HAVE_NO_STDARG_H */
{
    int         Sentence, Id;
    char        Text[256], *From;
    Gamelog    *Log;
    va_list     args;

#ifndef   HAVE_NO_STDARG_H
    va_start(args, Comment);
#else  /* HAVE_NO_STDARG_H */
    Game *game;
    const char *Comment;

    va_start(args);
    game    = va_arg(args, Game *);
    Comment = va_arg(args, const char *);
#endif /* HAVE_NO_STDARG_H */

    if (DebugFun) {
        if (game) printf("RemoveGame(%2d (%s), %s, ...)\n",
                         game->ServerId, GameLongDescription(game), Comment);
        else      printf("RemoveGame(NULL, %s, ...)\n", Comment);
        fflush(stdout);
    }

    if (game && game->ServerId >= 0) {
        game->ToMove = Empty;
#ifndef NO_CLIENT_TIME
	game->MoveTime = -1;
#endif
        sprintf(Text, "Game %10s vs %-10s ", PlayerToName(game->Black),
                PlayerToName(game->White));
        From = strchr(Text, 0);
        vsprintf(From, Comment, args);
        Sentence = isupper(From[0]);
        GameMessage(game, "----------", Sentence ? "%s" : "Game %s", From);
        Log = game->Log;
        if (Log) AddGlobalProperty(Log, "Result", From, -1);
        if (gameinfo) AddText(gameinfo, "%s\n", Text);
        Id = game->ServerId;
        game->ServerId = NOTONSERVER;
        PlayerOutGame(game->Black, game, Id);
        if (game->White != game->Black) PlayerOutGame(game->White, game, Id);
        if (game->Black2) PlayerOutGame(game->Black2, game, Id);
        if (game->White2) PlayerOutGame(game->White2, game, Id);
        DeleteGame(game);
    } else GameNotFound("for removegame");
    va_end(args);
}

/*****************************************************************************/

#define SHOW(Log, Name, Text)                           \
do {                                                    \
    XtPointer *name;                                    \
                                                        \
    name = Findcomment(Log, &Name);                     \
    if (name) printf(Text ": %2ld\n", (long) *name);    \
    else      printf(Text ": ---\n");                   \
} while(0)

/* Debugging function for gamelog */
void ShowGamelogInfo(Gamelog *log)
{
    int       i, node, nodes;
    static    Boolean inShowGame = False;

    if (inShowGame) return; /* avoid infinite recursion */
    inShowGame = True;

    nodes = NumberNodes(log)-1;
    node  = NodeNumber(log);
    for (i = node; i>0; i--)  UpGamelog(log);
    for (i = 0; i<nodes; i++, DownGamelog(log)) {
        printf("--node %2d--\n", i);
        SHOW(log, NextMoveFun, "Next     move");
        SHOW(log, PrevMoveFun, "Previous move");
        SHOW(log, LastMoveFun, "Last     move");
        SHOW(log, MoveFun,     "Move number  ");
    }
    printf("--node %2d--\n", i);
    SHOW(log, NextMoveFun, "Next     move");
    SHOW(log, PrevMoveFun, "Previous move");
    SHOW(log, LastMoveFun, "Last     move");
    SHOW(log, MoveFun,     "Move number  ");
    printf("==========\n");
    fflush(stdout);
    for (i=nodes; i>node; i--) UpGamelog(log);
    inShowGame = False;
}
#undef SHOW

static int HandicapPoints[] = {
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 2,
    0, 2, 0, 3, 0,
    3, 0, 3, 0, 3,
    0, 3, 0, 0, 0,
    4
};

static int HandPos[9][2] = {
    {1, 3}, {2, 3}, {3, 3},   /* 012 */
    {1, 2}, {2, 2}, {3, 2},   /* 345 */
    {1, 1}, {2, 1}, {3, 1}    /* 678 */
};

#define P(n) HandPos[n]
static int *HandicapPattern[8][10] = {
 {P(6), P(2), NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL},
 {P(6), P(2), P(0), NULL, NULL, NULL, NULL, NULL, NULL, NULL},
 {P(6), P(8), P(2), P(0), NULL, NULL, NULL, NULL, NULL, NULL},
 {P(6), P(8), P(2), P(0), P(4), NULL, NULL, NULL, NULL, NULL},
 {P(6), P(3), P(0), P(8), P(5), P(2), NULL, NULL, NULL, NULL},
 {P(6), P(3), P(0), P(8), P(5), P(2), P(4), NULL, NULL, NULL},
 {P(6), P(3), P(0), P(7), P(1), P(2), P(5), P(8), NULL, NULL},
 {P(6), P(3), P(0), P(7), P(4), P(1), P(8), P(5), P(2), NULL}
};
#undef P

static void ExecuteMove(Game *game, int Nr, const char *MoveDesc)
{
    int        x, y, xmid, ymid, xoff, yoff, Handicap, **Pattern, *Pat;
    int        SizeX, SizeY, node;
    char       Buffer[256];
    Gamelog   *Log;
    StoneList *Stones, *Stone;

    Log = game->Log;
    SizeX = game->XSize;
    SizeY = game->YSize;
    node = NodeNumber(Log);
    AddComment(Log, &NextMoveFun, INT_TO_XTPOINTER(node+1));
    if (memcmp(MoveDesc, "Handicap ", 9) == 0) {
        Handicap = atoi(MoveDesc+9);
        xmid = (SizeX-1)/2;
        ymid = (SizeY-1)/2;
        xoff = HandicapPoints[SizeX];
        yoff = HandicapPoints[SizeY];
        Stones = NULL;

        for (Pattern = HandicapPattern[Handicap-2];
             (Pat = *Pattern) != NULL; Pattern++) {
            Stone = mynew(StoneList);
            if (xoff)
                switch(Pat[0]) {
                  case 1: Stone->x = xoff;         break;
                  case 2: Stone->x = xmid;         break;
                  case 3: Stone->x = SizeX-1-xoff; break;
                  default: Raise1(AssertException, "invalid handicap pattern");
                }
            else Stone->x = 0;
            if (yoff)
                switch(Pat[1]) {
                  case 1: Stone->y = yoff;         break;
                  case 2: Stone->y = ymid;         break;
                  case 3: Stone->y = SizeY-1-yoff; break;
                  default: Raise1(AssertException, "invalid handicap pattern");
                }
            else Stone->y = 0;
            Stone->Color = Black;
            Stone->Next  = Stones;
            Stones = Stone;
        }
        SetStones(Log, Stones);
        FreeStones(Stones);
        AddComment(Log, &LastMoveFun, LastFromXY(game, 0, -Handicap));
        AddComment(Log, &PrevMoveFun, INT_TO_XTPOINTER(node));
        AddComment(Log, &MoveFun,     INT_TO_XTPOINTER(Nr));
        GameMessage(game, "..........", MoveDesc);

        CHANGEINT(game, Handicap,    Handicap, PROPHANDICAP(Handicap);
                  SetHandicap(game->Observers, Handicap));
    } else if (strcmp(MoveDesc, "Pass") == 0) {
        DoPass(Log, Nr % 2 ? Black : White);
        AddComment(Log, &LastMoveFun, LastFromXY(game, -1, 0));
        AddComment(Log, &PrevMoveFun, INT_TO_XTPOINTER(node));
        AddComment(Log, &MoveFun,     INT_TO_XTPOINTER(Nr));
        NbGameMessage(game, "..........", "%s passed",
                    Nr % 2 ? "Black" : "White");
    } else {
        GoXYFromMove(&x, &y, MoveDesc);
        DoMove(Log, x, y, Nr % 2 ? Black : White);
        AddComment(Log, &LastMoveFun, LastFromXY(game, x, y));
        AddComment(Log, &PrevMoveFun, INT_TO_XTPOINTER(node));
        AddComment(Log, &MoveFun,     INT_TO_XTPOINTER(Nr));
    }
}

static int EqualDescriptionP(const Game *game,
			     XtPointer Last, const char *Description)
{
    int x, y, x1, y1;

    XYFromLast(game, &x, &y, Last);
    if (memcmp(Description, "Handicap ", 9) == 0) {
	if (x) return 0;
	if (atoi(Description+9) != -y) return 0;
    } else if (strcmp(Description, "Pass") == 0) {
	if (x == 0 || y) return 0;
    } else {
	GoXYFromMove(&x1, &y1, Description);
	if (x1 != x || y1 != y) return 0;
    }
    return 1;
}

static Game *CompareMoves(Game *game, NameVal *moves)
{
    int        Nr, i, node, nodes;
    Gamelog   *log;
    XtPointer *Next, Last;
    NameVal   *move;
    NameVal    DummyMoves;
    Game      *newgame;

    if (DebugFun) {
        printf("CompareMoves(%s, " PRTPTR ")\n",
               GameLongDescription(game), moves);
        fflush(stdout);
    }

    if (!moves) {
        moves = &DummyMoves;
        moves->Previous = moves->Next = moves;
    }
  retry:
    log = game->Log;
    for (i= NodeNumber(log); i>0; i--) UpGamelog(log);
    node = XTPOINTER_TO_INT(FindComment(log, &NextMoveFun));
    for (i=0; i<node; i++) DownGamelog(log);
    Nr   = 0;
    Last = 0;
    for (move = moves->Next;
         move != moves;
         move = move->Next) {
        if ((Next = Findcomment(log, &NextMoveFun)) != NULL) {
            Nr++;
            for (nodes = XTPOINTER_TO_INT(*Next); node<nodes; node++)
                DownGamelog(log);
            Last = FindComment(log, &LastMoveFun);
	    if (!EqualDescriptionP(game, Last, move->Value)) break;
        } else {
            do {
                Nr++;
                ExecuteMove(game, Nr, move->Value);
                move = move->Next;
            } while (move != moves);
            if (Nr < game->Move ) {
                /* We could have a bit of undo here */
                goto error;
            }
            game->Move   = Nr;
            game->ToMove = Nr % 2 ? White : Black;
            return game;
        }
    }
    if (move != moves || Nr < game->Move) {
      error:
	if (appdata.WantVerbose) {
	    ShowGamelogInfo(log);
	    if (move != moves) {
		printf("move: %s %s(%d)\n", move->Name, move->Value,
		       XTPOINTER_TO_INT(Last));
	    }
	}
        if (game->ServerId < 0)
            Raise1(AssertException, "How did you find this game ?");
        Warning("Invalid moves for game %s. Replacing by a new one...\n",
                GameLongDescription(game));
        ForceNew = game->ServerId;
        newgame = FindGame(game->ServerId, game->Black, game->White,
                           Nr, game->XSize, game->YSize,
                           game->Handicap, game->Komi, game->ByoPeriod,
                           game->Mode, game->Rules, game->NrObservers);
	if (game->Black2) {
	    newgame->Black2 = game->Black2;
	    newgame->White2 = game->White2;
	    PlayerInGame(newgame->Black2, newgame, newgame->ServerId);
	    PlayerInGame(newgame->White2, newgame, newgame->ServerId);
	    /* For undo in team game, igs does not tell all players about
	     * the undo and we get here because of inconsistent moves.
	     * Make sure at least that xgospel knows I'm still playing:
             */
	    if (newgame->Black2 == Me || newgame->White2 == Me) {
	        MyGame = newgame;
	    }
	}
        newgame->Observed     = game->Observed;
        newgame->WantObserved = game->WantObserved;
        if (!newgame->Log) InitGamelog(newgame);
        game = newgame;
        goto retry;
/*      Raise(AssertException); */
    }
/*
        for (nodes = (int) FindComment(log, &PrevMoveFun); node>nodes; node--)
            UpGamelog(log);
        ExecuteMove(game, Nr, move->Value);
    }
    game->Move   = Nr;
    game->ToMove = Nr % 2 ? White : Black;
*/
    return game;
}

static Game *DescToGame(GameDesc *Desc, int Warn, int Add)
{
    Game       *game;
    const char *black, *white, *black2, *white2;

    Warn  = Warn && Entered;
    black = Desc->BlackName;
    white = Desc->WhiteName;
    black2 = Desc->BlackName2;
    white2 = Desc->WhiteName2;

    game = PlayersToGame(Desc->Id, black, white, BUSY);

     /* IGS can announce game Black2 vs White2 without telling us it's
      * a team game, then send Black vs. White after an observe request.
      * So we must change the previously created game Black2 vs White2
      * into a team game at this point.
      */
    if (!game && black2) {
        game = PlayersToGame(Desc->Id, black2, white2, BUSY);
	if (!game) {
	    game = PlayersToGame(Desc->Id, black2, white, BUSY);
	}
	if (!game) {
	    game = PlayersToGame(Desc->Id, black, white2, BUSY);
	}
    }
    if (game && black2) {
	game->Black = PlayerFromName(black);
	game->White = PlayerFromName(white);
	game->Black2 = PlayerFromName(black2);
	game->White2 = PlayerFromName(white2);
	PlayerInGame(game->Black, game, Desc->Id);
	PlayerInGame(game->Black2, game, Desc->Id);
	PlayerInGame(game->White, game, Desc->Id);
	PlayerInGame(game->White2, game, Desc->Id);
    }
    if (DebugFun && black2) {
        printf("DescToGame Id %d, %s+%s vs %s+%s\n",
               Desc->Id, black, black2, white, white2);
        fflush(stdout);
    }

    if (game) {
        if (game->ServerId != Desc->Id) {
            if (Warn && !strchr(black, '*') && appdata.WantVerbose) {
	        /* check for name* because teaching games have no match
                 * announcement
                 */
                Warning("Received information about game %d (%s vs %s)\n"
                        "But this is inconsistent with my database.\n",
                        Desc->Id, black, white);
	    }
            AutoCommand(NULL, "games");
	    /* Sent even if appdata.GamesTimeout == 0 otherwise xgospel
             * will be too confused.
             */
        } else {
            game->BlackCaptures = Desc->BlackCaptures;
            game->BlackByo      = Desc->BlackByo;
            game->BlackTime     = Desc->BlackTime;
            game->WhiteCaptures = Desc->WhiteCaptures;
            game->WhiteByo      = Desc->WhiteByo;
            game->WhiteTime     = Desc->WhiteTime;
            /* Also forces a gamelog */
            if (!game->Log) InitGamelog(game);
        }
    } else {
        /* Do not send "games" for a trailed game, it will be announced
         * later. Otherwise send even if appdata.GamesTimeout == 0.
         */
        if (!Add) AutoCommand(NULL, "games");

        /* Test as long as no match announce for teach --Ton */
        if (Warn && strcmp(black, white) && !strchr(black, '*') 
	    && appdata.WantVerbose) {
            Warning("Received information about game %d (%s vs %s)\n"
                    "But this game is not in my database.\n",
                    Desc->Id, black, white);
	}
    }
    return game;
}

void ShowObserve(Game *game)
{
    if (DebugFun) {
        printf("ShowObserve(%s)\n", GameLongDescription(game));
        fflush(stdout);
    }

    if (!game->Observers &&
        (game->ServerId < NOTONSERVER || game->WantObserved))
            GotoObserve(OpenObserve(game), NumberNodes(game->Log)-1);
    ShowPosition(game);
    SetTime(game);
    SetCaptures(game);
    ShowGame(game);
}

Game *AddMove(int Add, GameDesc *Desc, NameVal *moves)
{
    Game      *game;
    Gamelog   *log;
    int        i, nodes, Nr, newNr, AnyMoves;
    char       Buffer[256];

    Trailing = -1;
    AnyMoves = moves != moves->Next;
    game = DescToGame(Desc, AnyMoves, Add);
    if (game) {
#ifndef NO_CLIENT_TIME
	game->MoveTime = -1;
#endif
        /* Allow games observed automatically by the trail command, and
         * manual "observe xx":
         */
	if (Add) game->WantObserved = game->Observed = 1;
        if (AnyMoves) {
            if (game == MyGame) RemoveMoves(NULL);
            if (Add) game->NrObservers++;
            log = game->Log;
            nodes = NumberNodes(log)-1;
            for (i=NodeNumber(log); i<nodes; i++) DownGamelog(log);
            Nr = XTPOINTER_TO_INT(FindComment(log, &MoveFun));
	    newNr = atoi(moves->Previous->Name);

            if (Nr != newNr + Add) {
		/* Send "moves" except for a refresh which just repeats
                 * the previous move:
                 */
		if (Add || Nr != newNr + 1) {
		    AutoCommand(NULL, "moves %d", Desc->Id);
		}
                Nr = newNr + 1;

                game->Move   = Nr;
                game->ToMove = Nr % 2 ? White : Black;
            } else {
                if (Add) {
                    XtPointer *Last;

                    Last = Findcomment(log, &LastMoveFun);
                    if (!Last || !EqualDescriptionP(game, *Last,
                                                    moves->Previous->Value))
                        AutoCommand(NULL, "moves %d", Desc->Id);
                } else {
                    ExecuteMove(game, ++Nr, moves->Previous->Value);
                    sprintf(Buffer, "%ld", game->BlackTime);
                    AddLocalProperty(log, "TimeLeftBlack", Buffer, -1);
                    sprintf(Buffer, "%ld", game->WhiteTime);
                    AddLocalProperty(log, "TimeLeftWhite", Buffer, -1);
#ifndef NO_CLIENT_TIME
		  { Player *toMove;
  		    if (game->Black2) {
		        switch (Nr % 4) {
			  case 0: toMove = game->Black; break;
			  case 1: toMove = game->White; break;
			  case 2: toMove = game->Black2; break;
			  case 3: toMove = game->White2; break;
			}
		    } else {
  		        toMove = Nr % 2 ? game->White : game->Black;
		    }
		    game->MoveTime = toMove == Me ? time(NULL) : -1;
		  }
#endif
                }

                game->Move   = Nr;
                game->ToMove = Nr % 2 ? White : Black;
                ShowObserve(game);
            }
        } else {
            game = CompareMoves(game, moves);
            ShowObserve(game);
        }
    } else if (Add) {
        /* Trailed games are added to observation list before the game
         * announcement "21 {Match ...}". Remember that we want to observe
         * this game when the game is created in FindGame:
         */
        Trailing = Desc->Id;
    }
    return game;
}

Game *Moves(GameDesc *Desc, NameVal *moves)
{
    Game      *game;

    Trailing = -1;
    game = DescToGame(Desc, 1, 0);
    if (game) {
        game = CompareMoves(game, moves);
        ShowObserve(game);
    }
    return game;
}

Game *Undo(int Add, int Id, const char *black, const char *white,
          const char *MoveDesc)
{
    Game      *game;
    Gamelog   *Log;
    char     **Pos, **Board, **P, **B, Buffer[256];
    XtPointer *LastNode, lastMove, lastFrom;
    int        x, y, i, node, nodes, Nr, Handicap;

    game = IdPlayersToGame(Id, black, white);
    if (game) {
        if (game == MyGame) RemoveMoves(NULL);
        if (Add) game->Observed = 1;
#ifndef NO_CLIENT_TIME
	game->MoveTime = -1;
#endif
        Log = game->Log;
        if (Log) {
	    /* When wrong handicap is fixed, use EqualDescriptionP -Ton */
            WITH_HANDLING {
                GoXYFromMove(&x, &y, MoveDesc);
            } ON_EXCEPTION {
                x = y = 0;
	        if (strcmp(MoveDesc, "Pass") == 0) {
                    x = -1;
                    ClearException();
                } else if (memcmp(MoveDesc, "Handicap ", 9) == 0) {
                    y = -atoi(MoveDesc+9);
                    Handicap = 0;
                    CHANGEINT(game, Handicap, Handicap, PROPHANDICAP(Handicap);
                              SetHandicap(game->Observers, Handicap));
                    ClearException();
                } /* otherwise -> ReRaise(); */
            } END_HANDLING;

            node  = NodeNumber(Log);
            nodes = NumberNodes(Log)-1;
            for (i=node; i<nodes; i++) DownGamelog(Log);
            Nr = XTPOINTER_TO_INT(FindComment(Log, &MoveFun))-1;
            LastNode = Findcomment(Log, &PrevMoveFun);
            lastFrom = LastFromXY(game, x, y);
            lastMove = FindComment(Log, &LastMoveFun);
            if (LastNode && (lastFrom == lastMove ||
/* Fix for igs bug */        memcmp(MoveDesc, "Handicap ", 9) == 0)) {
                node = XTPOINTER_TO_INT(*LastNode);
                for (i=nodes; i>node; i--) UpGamelog(Log);
                lastMove = FindComment(Log, &LastMoveFun);
                Board = GamelogToBoard(Log);
                Pos = AllocMatrix(char, game->YSize, game->XSize);
                for (y = game->YSize, B=Board, P=Pos; y>0; y--, B++, P++)
                    memcpy(*P, *B, game->XSize);
                for (node = XTPOINTER_TO_INT(FindComment(Log, &PrevMoveFun));
                     i>node; i--) UpGamelog(Log);
                FindComment(Log, &NextMoveFun) = INT_TO_XTPOINTER(nodes+1);
                for (i=node; i<nodes; i++) DownGamelog(Log);
                PositionToNode(Log, Pos, Nr % 2 ? White : Black);
                FreeMatrix(Pos, game->YSize);
                AddComment(Log, &PrevMoveFun, INT_TO_XTPOINTER(node));
                AddComment(Log, &MoveFun,     INT_TO_XTPOINTER(Nr));
                AddComment(Log, &LastMoveFun, lastMove);

                game->Move   = Nr;
                game->ToMove = Nr % 2 ? White : Black;
                ShowObserve(game);
            } else {
	      AutoCommand(NULL, "moves %d", Id);
	    }
	    sprintf(Buffer, "%ld", game->BlackTime);
	    AddLocalProperty(Log, "TimeLeftBlack", Buffer, -1);
	    sprintf(Buffer, "%ld", game->WhiteTime);
	    AddLocalProperty(Log, "TimeLeftWhite", Buffer, -1);
        } else {
	  AutoCommand(NULL, "moves %d", Id);
	}
        NbGameMessage(game, "----------", "Undo %s", MoveDesc);
    }
    return game;
}

Game *MyGameUndo(const char *MoveDesc)
{
    return Undo(0, MyGame->ServerId, PlayerToName(MyGame->Black),
		PlayerToName(MyGame->White), MoveDesc);
}

void Done(void)
{
    PassToDone(MyGame, MyGame->Rules == 'G', 1);

    /* Save the prisonners count in case of future Undo */
    MyGame->oldWhiteCaptures = MyGame->WhiteCaptures;
    MyGame->oldBlackCaptures = MyGame->BlackCaptures;

    MyGame->Finished |= SCORING;
    MyGameMessage("Click on dead groups.");
    MyGameMessage("Click 'refresh' for update");
    MyGameMessage("Click 'done' when finished");
    MyGameMessage("Click 'undo' on error");
}

static XtPointer RealGameComment(const Game *game,
                                 const char *Format, va_list Args)
{
    char      Text[2048];
    Gamelog  *log;
    XtPointer Result;
    int       i, node, nodes;

    vsprintf(Text, Format, Args);
    log = game->Log;
    if (log) {
        nodes = NumberNodes(log)-1;
        node  = NodeNumber(log);
        for (i=node; i<nodes; i++) DownGamelog(log);
        Result = AddTextComment(log, Text, -1);
        for (i=nodes; i>node; i--) UpGamelog(log);
    } else Result = NULL;
    return Result;
}

#ifndef   HAVE_NO_STDARG_H
XtPointer AddGameComment(const Game *game, const char *Format, ...)
#else  /* HAVE_NO_STDARG_H */
XtPointer AddGameComment(game, va_alist)
const Game *game;
va_dcl
#endif /* HAVE_NO_STDARG_H */
{
    XtPointer Rc;
    va_list   Args;
#ifndef   HAVE_NO_STDARG_H
    va_start(Args, Format);
#else  /* HAVE_NO_STDARG_H */
    const char *Format;

    va_start(Args);
    Format = va_arg(Args, const char *);
#endif /* HAVE_NO_STDARG_H */
    Rc = RealGameComment(game, Format, Args);
    va_end(Args);
    return Rc;
}

#ifndef   HAVE_NO_STDARG_H
XtPointer AddMyGameComment(const char *Format, ...)
#else  /* HAVE_NO_STDARG_H */
XtPointer AddMyGameComment(va_alist)
va_dcl
#endif /* HAVE_NO_STDARG_H */
{
    XtPointer Rc;
    va_list   Args;
#ifndef   HAVE_NO_STDARG_H
    va_start(Args, Format);
#else  /* HAVE_NO_STDARG_H */
    const char *Format;

    va_start(Args);
    Format = va_arg(Args, const char *);
#endif /* HAVE_NO_STDARG_H */
    if (MyGame) Rc = RealGameComment(MyGame, Format, Args);
    else {
        Raise1(AssertException, "AddMyGameComment called without game");
        Rc = NULL; /* not reached, but shuts up compilers */
    }
    va_end(Args);
    return Rc;
}

void Mailed(char *Names)
{
    char *ptr1, *ptr2;
    Game *game;

    ptr1 = strchr(Names, '-');
    if (ptr1) {
        *ptr1++ = 0;
        ptr2 = strchr(ptr1, '-');
        if (ptr2) *ptr2++ = 0;
        game = PlayersToGame(0, ptr1, Names, OVER);
        if (ptr2) {
            if (game) GameMessage(game, "----------", "Game %s-%s (%s) mailed",
                                  ptr1, Names, ptr2);
            else Outputf("Game %s-%s (%s) mailed\n", ptr1, Names, ptr2);
            ptr2[-1] = '-';
        } else
            if (game) GameMessage(game, "----------", "Game %s-%s mailed",
                                  ptr1, Names);
            else Outputf("Game %s-%s mailed\n", ptr1, Names);
        ptr1[-1] = '-';
    } else Outputf("File %s mailed\n", Names);
    /* Raise2(AssertException, "Invalid name in mailed message:", Names); */
}

void RemoveGameFile(char *Names)
{
    char *ptr;
    Game *game;

    ptr = strchr(Names, '-');
    if (ptr) {
        *ptr++ = 0;
        game = PlayersToGame(0, ptr, Names, OVER);
        if (game)
            GameMessage(game, "----------",
                        "Removed game file %s-%s from database", ptr, Names);
        else Outputf("Removed game file %s-%s from database\n", ptr, Names);
        ptr[-1] = '-';
    } else Raise2(AssertException, "Invalid name in gamefile message:", Names);
}

void RemoveGroup(const char *Stone)
{
    int        x, y, node, nodes;
    Gamelog   *log;
    int        captures;

    if (MyGame) {
        log = MyGame->Log;
        GoXYFromMove(&x, &y, Stone);

        nodes = NumberNodes(log)-1;
        for (node=NodeNumber(log); node<nodes; node++) DownGamelog(log);
        AddComment(log, &NextMoveFun, INT_TO_XTPOINTER(node+1));
        captures = RemoveGroupFromStone(log, x, y);
	if (captures > 0) {
	    MyGame->WhiteCaptures += captures;
	} else {
	    MyGame->BlackCaptures -= captures;
	}
        AddComment(log, &LastMoveFun, LastFromXY(MyGame, -1, -1));
        AddComment(log, &PrevMoveFun, INT_TO_XTPOINTER(node));
        AddComment(log, &MoveFun,     INT_TO_XTPOINTER(MyGame->Move));
        ShowObserve(MyGame);
        MyGameMessage("Removing group @ %s", Stone);
    } else {
        AutoCommand(NULL, "games");
	/* Sent even if appdata.GamesTimeout == 0 otherwise xgospel
	 * will be too confused.
	 */
        Warning("Received remove @ %s, but you are not playing ?!\n", Stone);
    }
}

void RestoreScoring(void)
{
    char     **Board, **Pos, **P, **B;
    Gamelog   *log;
    int        y, node, tnode, oldnode, Nr;
    XtPointer *Next, lastMove;

    if (MyGame) {
        log = MyGame->Log;
        tnode = NumberNodes(log)-1;
        for (node = NodeNumber(log); node<tnode; node++) DownGamelog(log);
        Nr = XTPOINTER_TO_INT(FindComment(log, &MoveFun));
        do {
            for (tnode = XTPOINTER_TO_INT(FindComment(log, &PrevMoveFun));
                 node > tnode; node--) UpGamelog(log);
        } while (Nr == XTPOINTER_TO_INT(FindComment(log, &MoveFun)));

        oldnode = node;
        Next = Findcomment(log, &NextMoveFun);
        for (tnode = XTPOINTER_TO_INT(*Next); node < tnode; node++)
            DownGamelog(log);
        lastMove = FindComment(log, &LastMoveFun);
        Board = GamelogToBoard(log);
        Pos = AllocMatrix(char, MyGame->YSize, MyGame->XSize);
        for (y = MyGame->YSize, B=Board, P=Pos; y>0; y--, B++, P++)
            memcpy(*P, *B, MyGame->XSize);
        for (tnode = NumberNodes(log)-1; node < tnode; node++)
            DownGamelog(log);
        PositionToNode(log, Pos, Empty);
        FreeMatrix(Pos, MyGame->YSize);
        node++;
        *Next = INT_TO_XTPOINTER(node);
        AddComment(log, &PrevMoveFun, INT_TO_XTPOINTER(oldnode));
        AddComment(log, &MoveFun,     INT_TO_XTPOINTER(Nr));
        AddComment(log, &LastMoveFun, lastMove);
	MyGame->WhiteCaptures = MyGame->oldWhiteCaptures;
	MyGame->BlackCaptures = MyGame->oldBlackCaptures;
        ShowObserve(MyGame);
#ifndef NO_CLIENT_TIME
	MyGame->MoveTime = -1;
#endif
        MyGameMessage("Board is restored to what it was"
                      " when you started scoring");
    } else {
        AutoCommand(NULL, "games");
	/* Sent even if appdata.GamesTimeout == 0 otherwise xgospel
	 * will be too confused.
	 */
        Warning("Received restore scoring, but you are not playing ?!\n");
    }
}

void RestoreFromScoring(void)
/* The game has been resumed in normal play mode. */
{
    char     **Board, **Pos, **P, **B;
    Gamelog   *log;
    int        y, node, tnode, oldnode, Nr;
    XtPointer *Next, lastMove;

    if (MyGame) {
        log = MyGame->Log;
        tnode = NumberNodes(log)-1;
        for (node = NodeNumber(log); node<tnode; node++) DownGamelog(log);
        Nr = XTPOINTER_TO_INT(FindComment(log, &MoveFun));
        do {
            for (tnode = XTPOINTER_TO_INT(FindComment(log, &PrevMoveFun));
                 node > tnode; node--) UpGamelog(log);
        } while (Nr == XTPOINTER_TO_INT(FindComment(log, &MoveFun)));
        for (tnode = XTPOINTER_TO_INT(FindComment(log, &NextMoveFun));
             node < tnode; node++) DownGamelog(log);

        lastMove = FindComment(log, &LastMoveFun);
        Nr = XTPOINTER_TO_INT(FindComment(log, &MoveFun));
        Board = GamelogToBoard(log);
        Pos = AllocMatrix(char, MyGame->YSize, MyGame->XSize);
        for (y = MyGame->YSize, B=Board, P=Pos; y>0; y--, B++, P++)
            memcpy(*P, *B, MyGame->XSize);

        for (oldnode = XTPOINTER_TO_INT(FindComment(log, &PrevMoveFun));
             node > oldnode; node--) UpGamelog(log);

        Next = Findcomment(log, &NextMoveFun);

        for (tnode = NumberNodes(log)-1; node < tnode; node++)
            DownGamelog(log);
        PositionToNode(log, Pos, Nr % 2 ? White : Black);
        FreeMatrix(Pos, MyGame->YSize);
        node++;
        *Next = INT_TO_XTPOINTER(node);
        AddComment(log, &PrevMoveFun, INT_TO_XTPOINTER(oldnode));
        AddComment(log, &MoveFun,     INT_TO_XTPOINTER(Nr));
        AddComment(log, &LastMoveFun, lastMove);
        MyGame->Move = Nr;
        MyGame->ToMove = Nr % 2 ? White : Black;
#ifndef NO_CLIENT_TIME
	MyGame->MoveTime = -1;
#endif
        MyGame->Finished &= UNSCORING;
        PassToDone(MyGame, 1, 0);
	MyGame->WhiteCaptures = MyGame->oldWhiteCaptures;
	MyGame->BlackCaptures = MyGame->oldBlackCaptures;
        ShowObserve(MyGame);
    } else {
        AutoCommand(NULL, "games");
	/* Sent even if appdata.GamesTimeout == 0 otherwise xgospel
	 * will be too confused.
	 */
        Warning("Received restore scoring, but you are not playing ?!\n");
    }
}

int GamePosition(NameList *black, NameList *white, NumVal *Lines)
/* Display a game position after a "status", "look" or "done".
 Format:
 22 <white> <captured stones> <time left> <byoyomi stones> <T|F> komi handicap
 22 <black> <captured stones> <time left> <byoyomi stones> <T|F> komi handicap
 T is for byo yomi, F for ?

        ##: ################### (size of the board times)

        The '##:' is the line number of the board.  The rest of the numbers
        are as follows:
                0:  Black      4:  White Territory
                1:  White      5:  Black Territory
                2:  Empty      6:  Starpoint
                3:  Dame       7:  Counted
 */
{
    NumVal   *Line, *BoardLines;
    int       rc, Id, x, y;
    size_t    Size;
    Game     *game;
    char    **Board, *Ptr, *Bptr, Buffer[80];
    const char *BlackName, *BlackStrength, *WhiteName, *WhiteStrength;
    float   whiteAdvantage;
    NameList *next;
    int     bCaptures, wCaptures;

/*
    NameList *Here;

    for (Here = black->Next; Here != black; Here = Here->Next)
        printf("%10s ", Here->Name);
    putc('\n', stdout);
    for (Here = white->Next; Here != white; Here = Here->Next)
        printf("%10s ", Here->Name);
    putc('\n', stdout);
*/
    next = black->Next;
    BlackName     = next->Name;
    next = next->Next;
    BlackStrength = next->Name;
    next = next->Next;
    bCaptures = atoi(next->Name);
    next = next->Next; /* time */
    next = next->Next; /* byo stones */
    next = next->Next; /* T|F */
    next = next->Next;
    whiteAdvantage = atof(next->Name);

    next = white->Next;
    WhiteName     = next->Name;
    next = next->Next;
    WhiteStrength = next->Name;
    next = next->Next;
    wCaptures = atoi(next->Name);
    next = next->Next; /* time */
    next = next->Next; /* byo stones */
    next = next->Next; /* T|F */
    next = next->Next;
    if (whiteAdvantage != atof(next->Name)) {
	whiteAdvantage = UNKNOWN_ADVANTAGE;
    }
    whiteAdvantage += wCaptures - bCaptures;

    BoardLines = Lines->Next;
    Size       = 0;
    for (Line = BoardLines; Line != Lines; Line = Line->Next) Size++;

    /* Better get game/description from names -Ton */
    Id = WhatCommand(NULL, "status");
    if (Id >= 0) {
        game = ServerIdGame(Id);
        rc = 1;
    } else if (ArgsCommand(NULL, "look")) {
        game = NULL;
        rc = 1;
    } else {
        game = MyGame;
        rc = 0;
    }
    sprintf(Buffer, "status %s [%3s] vs [%3s] %s",
	    BlackName,     BlackStrength,
	    WhiteStrength, WhiteName);

    if (game == NULL && appdata.AutoReply) {
	Outputf("# %s\n", Buffer);
	return rc;
    }

    Board = AllocMatrix(char, Size, Size);
    WITH_UNWIND {
        for (Line = BoardLines, x=0; Line != Lines; Line = Line->Next, x++)
            for (y=Size-1, Ptr = Line->Value; y>=0; y--, Ptr++) {
                Bptr = &Board[y][x];
                switch (*Ptr) {
                  case '0': *Bptr = Black; break;
                  case '1': *Bptr = White; break;
                    /* maybe put more info in here -Ton */
                  default:  *Bptr = Empty; break;
                }
            }
        AnalyzeBoard(Board, Buffer, -1, Size, game ? game->Move : 0,
                     GameAllowSuicide(game), whiteAdvantage);
    } ON_UNWIND {
        FreeMatrix(Board, Size);
    } END_UNWIND;
    return rc;
}

static void ShowTime(const char *Color, const char *Name, int tim, int Byo)
{
    char Text[200], *ptr;

    sprintf(Text, "%10s(%s): ", Name, Color);
    ptr = strchr(Text, 0);
    if (tim < 0) {
        *ptr++ = '-';
        tim = -tim;
    } else *ptr++ = ' ';
    sprintf(ptr, "%2d:%02d", tim / 60, tim % 60);
    ptr = strchr(ptr, 0);
    if (Byo != NOBYO) {
        sprintf(ptr, "(%2d)", Byo);
        ptr = strchr(ptr, 0);
    }
    *ptr++ = '\n';
    *ptr = 0;
    Output(Text);
}

void GameTime(int Id,
              const char *black, int BTime, int Bbyo,
              const char *white, int WTime, int Wbyo)
{
    Game *game;

    game = IdPlayersToGame(Id, black, white);
    if (game) {
        if (game->Observed) {
            if (!game->Observers) OpenObserve(game);
            game->BlackTime     = BTime;
            game->WhiteTime     = WTime;
            /* Test needed as long as byo yomi
               for GOE match is nonsense -- Ton */
            if (game->Rules != 'G' && game->Rules != 'g') {
                game->BlackByo      = Bbyo;
                game->WhiteByo      = Wbyo;
            }
            SetTime(game);
        }
        if (UserCommandP(NULL)) {
            ShowTime("Black", black, BTime, Bbyo);
            ShowTime("White", white, WTime, Wbyo);
        }
    }
}

void Watching(NameList *games)
{
    Game     *game;
    NameList *name, *first;
    int       Id;

    for (game = GameBase.Next; game != &GameBase; game = game->Next)
        game->Found = UNOBSERVED;

    first = games->Next;
    if (first->Name && strcmp(first->Name, "None.") == 0)
        first = first->Next;
    for (name = first; name != games; name = name->Next) {
        Id = atoi(name->Name);
        game = ServerIdGame(Id);
        if (game) {
            game->Found = OBSERVED;
            if (!game->Observed) {
                game->Observed = 1;
                ShowGame(game);
            }
        } else Warning("The server says you are observing game %d. "
                       "I think not.\n", Id);
    }
    for (game = GameBase.Next; game != &GameBase; game = game->Next)
        if (game->Found == UNOBSERVED &&
            game->ServerId >= 0 && game != MyGame) {
            game->Observed = 0;
	    /* Dropping the next lines means: If you overobserve, the game
	       will remain marked for observe, and as soon as a games command
	       is executed, the observe will be retried. So some people might
	       actually prefer that, but it is to counter intuitive for
	       the general program */
	    if (game->WantObserved) {
                game->WantObserved = -1;
                ShowGame(game);
            }
        } else game->Found = UNCHANGED;
}

void UnObserve(int Id)
{
    Game *game;

    game = ServerIdGame(Id);
    if (game) {
        game->Observed = 0;
        if (game->WantObserved) AutoCommand(NULL, "observe %d", Id);
        if (--game->NrObservers < 0) {
            game->NrObservers = 0;
            AutoCommand(NULL, "games %d", Id);
        }
        ShowGame(game);
    } else GameNotFound("for remove observe");
}

void UnobserveGames(Connection conn)
{
    Game *game;

    if (conn != Conn) Raise(AssertException);
    for (game = GameBase.Next; game != &GameBase; game = game->Next)
        game->Observed = 0;
    if (MyGame && (MyGame->Finished & ~SCORING) == BUSY) AssumeAdjourn(MyGame);
}

void ObserveWhilePlaying(void)
{
    int   Id;
    Game *game;

    Id = WhatCommand(NULL, "observe");
    if (Id < 0) AutoCommand(NULL, "watching");
    else {
        game = ServerIdGame(Id);
        if (game) {
            if (game != MyGame) {
                game->Observed = 0;
                /* This causes us to even forget the attempt --Ton */
                game->WantObserved = 0;
                ShowGame(game);
            }
        } else AutoCommand(NULL, "watching");
    }
    if (UserCommandP(NULL)) Output("You cannot observe while playing.\n");
    else if (gameinfo)
        AddText(gameinfo, "You cannot observe while playing.\n");
}

static int expectAll;

void AssertGamesDeleted()
{
    Game       *game;
    const char *Command;
    int         Id;

    expectAll = 0;
    Command = StripFirstArgCommand(NULL, "games");
    if (Command)
        if (Command[0]) {
            ForceNew = XTPOINTER_TO_INT(CommandClosure(NULL))-1;
	    /* The closure was set in NewMatch() above or in newmatch2 in
             * gointer.y
             */
	    if (ForceNew >= 0 && DebugFun) {
	        printf("AssertGamesDeleted ForceNew %d\n", ForceNew);
		fflush(stdout);
	    }
            Id = atoi(Command);
            for (game = GameBase.Next; game != &GameBase; game = game->Next)
                if (game->ServerId == Id) game->Found = DELETED;
                else                      game->Found = UNCHANGED;
        } else {
          expectAll = 1;
	  for (game = GameBase.Next; game != &GameBase;
	       game = game->Next) game->Found = DELETED;
	}
}

void TestGamesDeleted(int gamesSeen)
{
    Game *game, *next;

    if (DebugFun) {
	printf("TestGamesDeleted gamesSeen %d\n", gamesSeen);
	fflush(stdout);
    }
    for (game = GameBase.Next;
         game != &GameBase;
         game = next) {
        next = game->Next;
        switch(game->Found) {
          case DELETED:
            if (expectAll && gamesSeen <= 1) {
	        /* Assume a race condition between "games" and "games nn"
                 * instead of an empty server:
                 */ 
	        game->Found = UNCHANGED;
            } else if (game->ServerId >= 0) {
                AssumeAdjourn(game);
	    }
            break;
          case NEW:
          case CHANGED:
            ShowGame(game);
            break;
          default:              /* case UNCHANGED: */
            break;
        }
    }
    ForceNew = -1;
}

void GameInfo(int Id, const char *black, const char *white,
              NameList *Type)
{
    Game     *game;
    char      Text[200], *ptr;
    NameList *Names;
    size_t    Length;

    game = IdPlayersToGame(Id, black, white);
    if (game) {
        game->Finished = OVER;
        ptr = Text;
        for (Names = Type->Next; Names != Type; Names = Names->Next) {
            *ptr++ = ' ';
            Length = strlen(Names->Name);
            memcpy(ptr, Names->Name, Length);
            ptr += Length;
        }
        ptr[0] = 0;

        RemoveGame(game, Text+1);
    }
}

Observe **Gametoobservers(Game *game)
{
    return &game->Observers;
}

Game *Resume(int Id, const char *black, const char *white, int move)
{
    Game    *game;
    Gamelog *Log;
    int      DoCompareEmpty;

    game = PlayersToGame(Id, black, white, ADJOURNED);
    if (game) {
        DoCompareEmpty = 0;
        if (game->ServerId > 0) Raise1(AssertException, "Adjourned game "
                                       "shouldn't be in game list");
        if (game->Black == Me  || game->White == Me ||
            game->Black2 == Me || game->White2 == Me) {

	    if (game->Black2 && game->White2) {
	        if (DebugFun) {
		    printf("Resume team %s %s %s %s\n",
		    PlayerToName(game->Black), PlayerToName(game->White),
		    PlayerToName(game->Black2), PlayerToName(game->White2));
		    fflush(stdout);
		}
	        return game; /* do the job in ResumeTeam later */
	    }
	    MyGame = game;
	}
        if (game == MyGame) {
            game->Observed = game->WantObserved = 1;
            if (game->Finished & SCORING) {
                RestoreFromScoring();
                MyGameMessage("Board is restored to what it was"
                              " before you started scoring");
            }
            if (game->Move != move) {
                if (move) AutoCommand(NULL, "moves %d", Id);
                else DoCompareEmpty = 1;
	    }
        } else if (game->WantObserved) AutoCommand(NULL, "observe %d", Id);

        game->ServerId = Id;
        if (DoCompareEmpty) game = CompareMoves(game, NULL);
	/* Force updating times: */
	if (game->Move == move) {
	    game->ToMove = move % 2 ? White : Black;
	}
        game->Found = NEW;
        PlayerInGame(game->Black, game, Id);
        if (game->White != game->Black) PlayerInGame(game->White, game, Id);
        if (game->Black2) PlayerInGame(game->Black2, game, Id);
        if (game->White2) PlayerInGame(game->White2, game, Id);
        game->Finished &= ~OVER; /* Changes ADJOURNED to BUSY */
        Log = game->Log;
        if (Log) DeleteGlobalProperty(Log, "Result");
        CHANGEINT(game, Move, move,;);
        GameMessage(game, "----------", "Game has been "
                          "resumed at move %d", move);
	ChangeGameDescription(game); /* needed if game id displayed in title */
        ShowGame(game);
	if (!game->WantObserved &&
	    (PlayerToImportance(game->White) >= minImportantRating ||
	     PlayerToImportance(game->Black) >= minImportantRating)) {
	    IfBell(BeepOnImportant);
	    IfRaise(RaiseOnImportant, RaiseOnImportant);
	}
    } else {
        AutoCommand(NULL, "games %d", Id);
    }
    if (gameinfo) AddText(gameinfo, "Game %10s vs %-10s has been resumed "
                          "at move %d\n", black, white, move);
    return game;
}

Game *ResumeTeam(int Id, const char *black, const char *white,
                  const char *black2, const char *white2, int move)
{
    Game    *game;
    Gamelog *Log;
    int      DoCompareEmpty;


    if (DebugFun) {
        printf("ResumeTeam(%d, %s, %s, %s, %s) move %d\n",
               Id, black, white, black2, white2, move);
        fflush(stdout);
    }
    game = PlayersToGame(Id, black, white, ADJOURNED);
    if (game) {
        DoCompareEmpty = 0;
        if (game->ServerId > 0) Raise1(AssertException, "Adjourned game "
                                       "shouldn't be in game list");
        if (game->Black == Me  || game->White == Me ||
            game->Black2 == Me || game->White2 == Me) {
	    MyGame = game;
	}
        if (game == MyGame) {
            game->Observed = game->WantObserved = 1;
            if (game->Finished & SCORING) {
                RestoreFromScoring();
                MyGameMessage("Board is restored to what it was"
                              " before you started scoring");
            }
            if (game->Move != move)
                if (move) AutoCommand(NULL, "moves %d", Id);
                else DoCompareEmpty = 1;
        } else if (game->WantObserved) AutoCommand(NULL, "observe %d", Id);

        game->ServerId = Id;
        if (DoCompareEmpty) game = CompareMoves(game, NULL);
	/* Force updating times: */
	if (game->Move == move) {
	    game->ToMove = move % 2 ? White : Black;
	}
        game->Found = NEW;
        PlayerInGame(game->Black, game, Id);
        PlayerInGame(game->Black2, game, Id);
        PlayerInGame(game->White, game, Id);
        PlayerInGame(game->White2, game, Id);
        game->Finished &= ~OVER; /* Changes ADJOURNED to BUSY */
        Log = game->Log;
        if (Log) DeleteGlobalProperty(Log, "Result");
        CHANGEINT(game, Move, move,;);
        GameMessage(game, "----------", "Game has been "
                          "resumed at move %d", move);
	ChangeGameDescription(game); /* needed if game id displayed in title */
        ShowGame(game);
	if (!game->WantObserved &&
	    (PlayerToImportance(game->White) >= minImportantRating  ||
	     PlayerToImportance(game->Black) >= minImportantRating  ||
	     PlayerToImportance(game->White2) >= minImportantRating ||
	     PlayerToImportance(game->Black2) >= minImportantRating)) {
	    IfBell(BeepOnImportant);
	    IfRaise(RaiseOnImportant, RaiseOnImportant);
	}
    } else {
        TeamGame(Id, black, white, black2, white2, move);
        AutoCommand(NULL, "games %d", Id);
    }
    if (gameinfo) AddText(gameinfo, "Game %10s vs %-10s has been resumed "
                          "at move %d\n", black, white, move);
    return game;
}

void Adjourn(int Id, const char *black, const char *white)
{
    Game *game;

    if (strcmp(black, white) == 0 &&
        strcmp(white, PlayerToName(Me)) == 0) ChangeCommand(NULL, 1);
    game = IdPlayersToGame(Id, black, white);
    if (game) {
        RemoveGame(game, "has been adjourned");
        game->Finished |= OVER; /* Chanes BUSY to ADJOURNED */
    }
}

static char *GameHeaders(const char *Pattern, XtPointer Closure)
{
    return GameTemplateDescription((const Game *) Closure, Pattern);
}

void SetGameHeaders(Widget w, const Game *game)
{
    SetWidgetTitles(w, GameHeaders, (XtPointer) game);
}

void ChangeGameDescription(Game *game)
{
    Gamelog    *Log;

    if (DebugFun) {
        printf("ChangeGameDescription(%2d (%s))\n",
               game->ServerId, GameLongDescription(game));
        fflush(stdout);
    }

#define AddProp(Name, Value) AddGlobalProperty(Log, Name, Value, -1)
    if ((Log = game->Log) != NULL) {
        WITH_HANDLING {
            DeleteGlobalProperty(Log, "BlackStrength");
            AddProp("BlackStrength", PlayerToStrength(game->Black));
        } ON_EXCEPTION {
            ClearException();
        } END_HANDLING;

        WITH_HANDLING {
            DeleteGlobalProperty(Log, "WhiteStrength");
            AddProp("WhiteStrength", PlayerToStrength(game->White));
        } ON_EXCEPTION {
            ClearException();
        } END_HANDLING;
    }
#undef AddProp

    if (StrengthCompare(game->Black, game->White) < 0) {
        game->Strongest = game->Black;
        game->Weakest   = game->White;
    } else {
        game->Strongest = game->White;
        game->Weakest   = game->Black;
    }

    if (game->ServerId >= NOTONSERVER &&
        game->Widget && game->WidgetPlan != DELETE) {
        game->WidgetPlan = CREATE | DELETE;
        WidgetPlanRefresh(GamesPlan);
    }
    SetObserveDescriptions(game->Observers, game);
}

const char *GameDescription(const Game *game)
{
    static char Description[80];

    if (game->Black && game->White)
        sprintf(Description, "%s vs %s",
                PlayerToName(game->Black), PlayerToName(game->White));
    else strcpy(Description, "Unknown players (review)");
    return Description;
}

const char *GameLongDescription(const Game *game)
{
    static char Description[80];

    if (game->Black && game->White)
        sprintf(Description, "%s [%3s] vs [%3s] %s",
                PlayerToName(    game->Black), PlayerToStrength(game->Black),
                PlayerToStrength(game->White), PlayerToName(    game->White));
    else strcpy(Description, "Unknown players (review)");
    return Description;
}

char *GameTemplateDescription(const Game *game, const char *Template)
{
    char BlackStr[20], WhiteStr[20], MeStr[20], serverId[20];
    char black[128], white[128];

    sprintf(MeStr, "%3.*s",
            (int) sizeof(MeStr)-1, PlayerToStrength(Me));
    sprintf(serverId, "%d", game->ServerId);

    if (game->Black2 && game->White2) {
	sprintf(black, "%.*s+%.*s",
		sizeof(black)/2 - 1, PlayerToName(game->Black),
		sizeof(black)/2 - 1, PlayerToName(game->Black2));
	sprintf(white, "%.*s+%.*s",
		sizeof(white)/2 - 1, PlayerToName(game->White),
		sizeof(white)/2 - 1, PlayerToName(game->White2));
	sprintf(BlackStr, "%.*s+%.*s",
		(int) sizeof(BlackStr)/2-1, PlayerToStrength(game->Black),
		(int) sizeof(BlackStr)/2-1, PlayerToStrength(game->Black2));
	sprintf(WhiteStr, "%.*s+%.*s",
		(int) sizeof(WhiteStr)/2-1, PlayerToStrength(game->White),
		(int) sizeof(WhiteStr)/2-1, PlayerToStrength(game->White2));
    } else {
	sprintf(black, "%.*s",
		sizeof(black) - 1, PlayerToName(game->Black));
	sprintf(white, "%.*s",
		sizeof(white) - 1, PlayerToName(game->White));
	sprintf(BlackStr, "%3.*s",
		(int) sizeof(BlackStr)-1, PlayerToStrength(game->Black));
	sprintf(WhiteStr, "%3.*s",
		(int) sizeof(WhiteStr)-1, PlayerToStrength(game->White));
    }
    return StringToFilename(Template,
                            (int) 'B', black,
                            (int) 'b', BlackStr,
                            (int) 'A', PlayerToAutoRated(game->Black2 ?
					  game->Black2 : game->Black),
                            (int) 'W', white,
                            (int) 'w', WhiteStr,
                            (int) 'a', PlayerToAutoRated(game->White2 ?
					  game->White2 : game->White),
                            (int) 'N', PlayerToName(Me),
                            (int) 'n', MeStr,
                            (int) 't', game->Title ? game->Title : "",
                            (int) 'T', game->Title ? game->Title : "",
                            (int) 'G', serverId,
                            0);
}

char *GameName(const char *Template, const char *Type, const Game *game)
{
    return StringToFilename(Template,
                            (int) 'T', Type,
                            (int) 't', Type && *Type ? "." : "",
                            (int) 'B', PlayerToName(    game->Black),
                            (int) 'b', PlayerToStrength(game->Black),
                            (int) 'W', PlayerToName(    game->White),
                            (int) 'w', PlayerToStrength(game->White),
                            (int) 'V', "vs",
                            (int) 'U', "_",
                            0);
}

/* Bunch of ugly object model breaking functions. We should have friends :-) */
const Player *GameBlack(const Game *game)
{
    return game->Black;
}

const Player *GameWhite(const Game *game)
{
    return game->White;
}

size_t GameXSize(const Game *game)
{
    return game->XSize;
}

size_t GameYSize(const Game *game)
{
    return game->YSize;
}

/* Currently IGS seems to allow suicide. Keep this function in case it ever
   gets rule dependent (rarely called anyways) */
int GameAllowSuicide(const Game *game)
{
    const Gamelog *log;

    if (game) {
        log = game->Log;
        if (log) return log->AllowSuicide;
        else     return 1;
    } else return 1;
}

Gamelog *GameGamelog(const Game *game)
{
    return game->Log;
}

XtPointer LastFromXY(const Game *game, int x, int y)
{
   return INT_TO_XTPOINTER(y * game->XSize + x);
}

void     XYFromLast(const Game *game, int *x, int *y, XtPointer last)
{
    int Last;

    Last = XTPOINTER_TO_INT(last);
    if (Last < 0) {
        Last = -Last;
        *y = -(Last / game->XSize);
        *x = -(Last % game->XSize);
    } else {
        *y = Last / game->XSize;
        *x = Last % game->XSize;
    }
}

int GameServerId(const Game *game)
{
    return game->ServerId;
}

const char *GameCaptures(const Game *game)
{
    static char Captures[80];

    sprintf(Captures, "B:%2d, W:%2d",
            game->BlackCaptures, game->WhiteCaptures);
    return Captures;
}

int GameMove(const Game *game)
{
    return game->Move;
}

time_t GameMoveTime(const Game *game)
{
    return game->MoveTime;
}

const char *GameKomi(const Game *game)
{
    return game->Komi;
}

int GameHandicap(const Game *game)
{
    return game->Handicap;
}

const char *GameTitle(const Game *game)
{
    return game->Title;
}

int TeachingP(const Game *game)
{
    return game && game->Teacher != NULL;
}

int RequestP(const Game *game)
{
    const char *black = PlayerToName(game->Black);
    const char *white = PlayerToName(game->White);

    return black[strlen(black)-1] == '*' && white[strlen(white)-1] == '*';
}

int PlayingP(const Game *game)
{
    return game->Color != Empty;
}

int TersePlay(const Game *game)
{
    return MyGame && MyGame != game && appdata.TersePlay != False;
}

int SayP(const Player *player)
/* Return true to replace tell with say. This must be forced if playing
 * in a team game (tell not allowed).
 */
{
    return MyGame && player != Me && (appdata.UseSay || MyGame->Black2) &&
           (MyGame->Finished == BUSY || MyGame->Finished == SCORING) &&
           (MyGame->Black == player || MyGame->White == player ||
	    MyGame->Black2 == player || MyGame->White2 == player);
}

const Player *MyOpponent(const Player *player)
{
    if (MyGame)
        if      (player == MyGame->Black) return MyGame->White;
        else if (player == MyGame->White) return MyGame->White;
        else return NULL;
    else return NULL;
}

int MyGameP(const Game *game)
{
    return game && game == MyGame;
}

int MyTurnP(const Game *game)
{
    return game && game == MyGame && (game->Color & game->ToMove) != Empty;
}

int ScoringP(const Game *game)
{
    return (game->Finished & SCORING) != 0;
}

int BusyP(const Game *game)
{
    return (game->Finished & ~SCORING) == BUSY;
}

Game *ObservedGame(void)
{
    Game *game, *result;

    result = NULL;
    for (game = GameBase.Next; game != &GameBase; game = game->Next)
        if (game->Observed)
            if (result) return NULL;
            else result = game;
    return result;
}

extern Observe *_ObserveFindWidget(Observe *observers, Widget w, int offset);
extern Observe *_FindObserveWidget(Widget w, int offset);
Observe *_FindObserveWidget(Widget w, int offset)
{
    Game *game;
    Observe *Found;

    for (game = GameBase.Next; game != &GameBase; game = game->Next)
        if ((Found = _ObserveFindWidget(game->Observers, w, offset)) != NULL)
            return Found;
    for (game = ReviewBase.Next; game != &ReviewBase; game = game->Next)
        if ((Found = _ObserveFindWidget(game->Observers, w, offset)) != NULL)
            return Found;
    return NULL;
}

static void TimeText(char *Text, int Tim, int Byo)
{
    if (Tim >= 0) {
        sprintf(Text, "%2d:%02d", Tim / 60, Tim % 60);
    } else if (Tim >= -(9*60+59)) {
        sprintf(Text, "-%1d:%02d", (-Tim) / 60, (-Tim) % 60);
    } else {
        strcpy(Text, "--:--");
    }
    if (Byo != NOBYO) sprintf(Text+strlen(Text), "(%2d)", Byo);
/*  else              strcat(Text, "    ");   */
}

const char *GetTime(const Game *game)
{
    static char Text[80];
    char white[80], black[80];

    TimeText(black, game->BlackTime, game->BlackByo);
    TimeText(white, game->WhiteTime, game->WhiteByo);
    sprintf(Text, "%s - %s", black, white);
    return Text;
}

static int LowTime(int time, int byo, int byoPeriod)
/* Returns true if time left per move is too low. time in seconds, byo is the
 * number of moves left, byoPeriod is a number of minutes for this game.
 */
{
    int minTime;

    if (byo == NOBYO) return 0;

    byoPeriod *= 60;
    if (appdata.MinSecPerMove * MOVES_PER_BYO + appdata.MinLagMargin
	> byoPeriod) {
	minTime = (byo * (byoPeriod - appdata.MinLagMargin)) / MOVES_PER_BYO;
    } else {
	minTime = byo * appdata.MinSecPerMove;
    }
    return time < minTime + appdata.MinLagMargin;
}

static void SetTime(Game *game)
{
    int low = 0;
    int blackLow = LowTime(game->BlackTime, game->BlackByo, game->ByoPeriod);
    int whiteLow = LowTime(game->WhiteTime, game->WhiteByo, game->ByoPeriod);

    if (appdata.LowTimeSet & 1) {
	if (game->Black == Me || game->Black2 == Me) {
	    low = blackLow;
	} else if (game->White == Me || game->White2 == Me) {
	    low = whiteLow;
	}
    }
    if ((appdata.LowTimeSet & 2) && game == MyGame) {
	if (game->Black == Me || game->Black2 == Me) {
	    low |= whiteLow << 1;
	} else {
	    low |= blackLow << 1;
	}
    }
    if ((appdata.LowTimeSet & 4) && game != MyGame) {
	low |= (blackLow | whiteLow) << 2;
    }
    SetObserveTime(game->Observers, GetTime(game), low);
}

void TestDeleteGame(Game *game)
{
    if (game->ServerId < NOTONSERVER) TestDeleteReview(game);
    else if (game->ServerId == NOTONSERVER && !game->Observers)
        if (game->WantObserved == 0) DeleteGame(game);
        else game->WantObserved = 1;
}

void ShowObservers(int Id, const char *black, const char *white,
                   const NameList *observers)
{
    Game           *game;
    const NameList *observer;
    const Player  **People, **Ptr;
    const char     *Name;
    char            Text[256], *From;
    int             NrCols, i, NrPeople, Type;
    volatile int    col;

    game = IdPlayersToGame(Id, black, white);
    if (game) {
        if (ArgsCommand(NULL, "all")) {
            Type = XTPOINTER_TO_INT(CommandClosure(NULL));
            if ((Type == 2 && !game->Observers) || (Type == 1 && !gameinfo))
                Type = 0;
        } else Type = 0;
        if (Type == 2) NrCols = 2;
        else           NrCols = 4;
        NrPeople = 0;
        for (observer = observers->Next; observer != observers;
             observer = observer->Next) NrPeople++;
        if (NrPeople % 2 == 1) {
            Warning("Error in observers format\n");
            return;
        }
        NrPeople /= 2;
        Ptr = People = mynews(const Player *, NrPeople);
        switch(Type) {
          case 2:
            NbGameMessage(game, "..........", "            %d observer%s%s",
                        NrPeople,
                        NrPeople == 1 ? "" : "s", NrPeople ? ":" : "");
            break;
          case 1:
            if (gameinfo) AddText(gameinfo, "Observing %2d (%s): "
                                  "(%d observer%s)\n", game->ServerId,
                                  GameLongDescription(game), NrPeople,
                                  NrPeople == 1 ? "" : "s");
            break;
          default:
            Outputf("Observing %2d (%s): (%d observer%s)\n",
                    game->ServerId, GameLongDescription(game),
                    NrPeople, NrPeople == 1 ? "" : "s");
            break;
        }
        Ptr = People = mynews(const Player *, NrPeople);
        WITH_UNWIND {
            for (observer = observers->Next; observer != observers;
                 observer = observer->Next->Next)
                *Ptr++ = FindPlayerByNameAndStrength(observer->Name,
                                                     observer->Next->Name);
            qsort((void *) People, (size_t) NrPeople,
                  sizeof(*People), PlayersCompare);
            From = Text;
            col  = 0;
            for (Ptr = People, i = NrPeople; i > 0; Ptr++, i--) {
                Name = PlayerString(*Ptr);
                sprintf(From, "%s%16s", col ? " " : "", Name);
                From = strchr(From, 0);
                if (++col == NrCols) {
                    col  = 0;
                    if (Type == 2) NbGameMessage(game, "..........", "%s", Text);
                    else {
                        *From++ = '\n';
                        *From   = 0;
                        if (Type == 1) {
                            if (gameinfo) AddText(gameinfo, "%s", Text);
                        } else Output(Text);
                    }
                    From = Text;
                }
            }
        } ON_UNWIND {
            if (col)
                if (Type == 2) NbGameMessage(game, "..........", "%s",  Text);
                else if (Type == 1) {
                    if (gameinfo) AddText(gameinfo, "%s\n", Text);
                } else Outputf("%s\n", Text);
            myfree(People);
        } END_UNWIND;
        CHANGEINT(game, NrObservers, NrPeople, ShowGame(game));
    }
}

static int BetCompare(const void *bet1, const void *bet2)
{
    BetDesc *Bet1, *Bet2;
    long     Total;

    Bet1 = *(BetDesc **) bet1;
    Bet2 = *(BetDesc **) bet2;
    Total = Bet1->Wins * Bet2->Bets - Bet2->Wins * Bet1->Bets;
    if (Total > 0) return -1;
    if (Total < 0) return  1;
    return PlayerCompare(Bet1->Who, Bet2->Who);
}

void BetResults(BetDesc *Win, BetDesc *Even, BetDesc *Loose, const char *mybet)
{
    int i, Count, User;
    BetDesc *Here, **All, **Ptr;
    Game    *game;

    Count = 0;
    for (Here = Win;   Here; Here = Here->Next) Count++;
    for (Here = Even;  Here; Here = Here->Next) Count++;
    for (Here = Loose; Here; Here = Here->Next) Count++;

    Ptr = All = mynews(BetDesc *, Count);
    for (Here = Win;   Here; Here = Here->Next) *Ptr++ = Here;
    for (Here = Even;  Here; Here = Here->Next) *Ptr++ = Here;
    for (Here = Loose; Here; Here = Here->Next) *Ptr++ = Here;
    qsort((void *) All, (size_t) Count, sizeof(*All), BetCompare);

    game = ObservedGame();
    User = UserCommandP(NULL);
    if (game && !User) {
        NbGameMessage(game, "..........", "            %d gambler%s", Count,
                    Count == 1 ? "" : "s");
        Ptr = All;
        for (i=0; i<Count; i++, Ptr++) {
            Here = *Ptr;
            if (Here->Bets)
                NbGameMessage(game, "..........", "%16s: %3ld/%-3ld (%3ld%%)",
                            PlayerString(Here->Who), Here->Wins, Here->Bets,
                            ((Here->Wins*200+Here->Bets)/(2*Here->Bets)));
            else
                NbGameMessage(game, "..........",
                            "%16s: %3ld/%-3ld (inf%%) -> ERROR",
                            PlayerString(Here->Who), Here->Wins, Here->Bets);
        }
	if (mybet) {
                NbGameMessage(game, "..........", "%s", mybet);
	}
    } else {
        if (game) Outputf("Bet results for game %d (%s)\n"
                          "            %d gambler%s",
                          game->ServerId, GameLongDescription(game),
                          Count, Count == 1 ? "" : "s");
        else Outputf("Bet results from an indeterminate game\n"
                     "            %d gambler%s", Count, Count == 1 ? "" : "s");
        Ptr = All;
        for (i=0; i<Count; i++, Ptr++) {
            Here = *Ptr;
            if (Here->Bets)
                Outputf("   %16s: %3ld/%-3ld (%3ld%%)",
                        PlayerString(Here->Who), Here->Wins, Here->Bets,
                        ((Here->Wins*200+Here->Bets)/(2*Here->Bets)));
        else
            Outputf("   %16s: %3ld/%-3ld (inf%%) -> ERROR",
                    PlayerString(Here->Who), Here->Wins, Here->Bets);
        }
    }
    myfree(All);
}

#ifndef   HAVE_NO_STDARG_H
int MyGameMessage(const char *Format, ...)
#else  /* HAVE_NO_STDARG_H */
int MyGameMessage(va_alist)
va_dcl
#endif /* HAVE_NO_STDARG_H */
{
    char     Text[512];
    va_list  args;

#ifndef   HAVE_NO_STDARG_H
    va_start(args, Format);
#else  /* HAVE_NO_STDARG_H */
    const char *Format;

    va_start(args);
    Format = va_arg(args, const char *);
#endif /* HAVE_NO_STDARG_H */
    vsprintf(Text, Format, args);
    va_end(args);
    if (MyGame) return GameMessage(MyGame, "----------", "%s", Text);
    Warning("You don't seem to be playing, but I received:\n%s\n", Text);
    return 1;
}

void GamesTime(unsigned long diff)
{
    long  old = 0;
    long new;
    int byo, refresh;
    Game *game;

    WidgetPlanTime(diff);
    for (game = GameBase.Next; game != &GameBase; game = game->Next) {
        if (game->Observed && (game->Finished & UNSCORING) == BUSY &&
            (game->Finished & SCORING) != SCORING && !TeachingP(game)) {
	    refresh = 0;
	    /* Do not ask IGS new time for "request" games: the time
	     * constantly goes back to zero (it's time per move, not
	     * remaining time in byoyomi period).
	     */
	    if (RequestP(game)) refresh = -1;

            switch(game->ToMove) {
              case Black:
                old = game->BlackTime;
		byo = game->BlackByo;
		new = old - diff;
		if (new < 0 && byo == NOBYO && refresh == 0) {
		    refresh = 1;
		    new += game->ByoPeriod*60;
		    game->BlackByo = MOVES_PER_BYO;
		}
		game->BlackTime = new;
                break;
              case White:
                old = game->WhiteTime;
		byo = game->WhiteByo;
		new = old - diff;
		if (new < 0 && byo == NOBYO && refresh == 0) {
		    refresh = 1;
		    new += game->ByoPeriod*60;
		    game->WhiteByo = MOVES_PER_BYO;
		}
		game->WhiteTime = new;
                break;
              default:
                new = old; /* No time evolution */
                break;
            }
	    if (refresh > 0 || new != old) {
		SetTime(game);
		if (refresh > 0) {
		    /* Force IGS to change from negative time to positive
		     * plus 25 stones ("time" does not do this).
		     */
		    SendCommand(NULL, NULL, "refresh %d", game->ServerId);
		} else if (refresh == 0 && new < 0 && old >= 0) {
		    /* Do not send "refresh" if I could lose on time */
		    SendCommand(NULL, NULL, "time %d", game->ServerId);
                }
	    }
	}
        if (game->Log) {
            ReplayTime(diff, game->Observers, NumberNodes(game->Log)-1);
	}
    }
}
