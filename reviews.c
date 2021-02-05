#include <mymalloc.h>
#include <myxlib.h>

#include <stdlib.h>
#include <string.h>
#if !STDC_HEADERS && HAVE_MEMORY_H
# include <memory.h>
#endif /* not STDC_HEADERS and HAVE_MEMORY_H */
#include <ctype.h>

#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <X11/Xaw/Command.h>
#include <X11/Xmu/CharSet.h>

#include <SmeToggle.h>

#include "gamesP.h"
#include "reviews.h"
#include "xgospel.h"

#ifdef    HAVE_NO_MEMCHR_PROTO
extern void *memchr(const void *s, int c, size_t n);
#endif /* HAVE_NO_MEMCHR_PROTO */

#define PRTPTR    "#%p"        /* Nice outputformat for a pointer          */

/* Must be < 0 */
#define REVIEWNONODE  -1
#define REVIEWBADNODE -2

Widget ReviewsButton;
static void DoReview(Widget w, XEvent *evnt, String *str, Cardinal *n);
static XtActionsRec ReviewsActionTable[] = {
    { (String) "doreview",    DoReview   },
};

Game ReviewBase = { &ReviewBase, &ReviewBase };

static ConvertToWidget  ReviewToWidget;
static Widget           ReviewRoot;
static WidgetPlan       ReviewsPlan;
/* invariant: If Reviewed becomes 0, ReviewNode should become REVIEWNONODE */
static int              ReviewNode;
static Game            *Reviewed;
static NumVal          *ReviewGlobal = NULL;
static NumNameListList *ReviewLocal  = NULL;
static int              WantReviews;
static int              DropVariations, AfterStart;
static int              ReviewMessage, ReviewError;

extern struct tm UniversalTime;

static int ReviewCompare(const Game *review1, const Game *review2)
{
    if (review1->ServerId > review2->ServerId) return -1;
    if (review1->ServerId < review2->ServerId) return  1;
    return XmuCompareISOLatin1(review1->Title, review2->Title);
}

static int WidgetReviewCompare(const void *review1, const void *review2)
{
    /* On machines where this idiotic series of casts is necessary, the
       method to get the addresses is probably invalid anyways, but let's
       pretend... */
    return ReviewCompare((Game *)(void *)(char *)*(Widget *)review1,
                         (Game *)(void *)(char *)*(Widget *)review2);
}

static const char *ReviewProperties(Cardinal *i, Arg *arg,
                                    void *Entry, XtPointer Closure)
{
    Game       *review;
    const char *Name;

    review = (Game *) Entry;
    XtSetArg(arg[*i], XtNlabel, (XtArgVal) review->Title); (*i)++;

    if      (review->WantObserved)  Name = "reviewEntryPending";
    else if (review->Observed)      Name = "reviewEntryDone";
    else                            Name = "reviewEntry";

    return Name;
}

static void CallReview(Widget w, XtPointer clientdata, XtPointer calldata);
static void ReviewWidgetCreate(void *Entry, XtPointer Closure)
{
    Game *review;

    review = (Game *) Entry;
    XtAddCallback(review->Widget, XtNcallback, CallReview, (XtPointer) review);
}

void InitReviews(Widget Toplevel)
{
    Widget      ReviewsWidget, ReviewCollect, AllowResize;
    Boolean     state;
    const char *Error;

    ReviewToWidget.Convert   = NULL;
    ReviewToWidget.NrConvert = -1;
    Error = CompileToWidget(appdata.ReviewToWidget, &ReviewToWidget);
    if (Error) Warning("CompileToWidget: %s\n", Error);

    ReviewGlobal = mynew(NumVal);
    ReviewGlobal->Previous = ReviewGlobal->Next  = ReviewGlobal;
    ReviewGlobal->Num   = 0;
    ReviewGlobal->Value = NULL;

    ReviewLocal = mynew(NumNameListList);
    ReviewLocal->Previous = ReviewLocal->Next  = ReviewLocal;
    ReviewLocal->Num = 0;
    ReviewLocal->Value = NULL;

    WantReviews = 0;
    Reviewed    = NULL;
    ReviewNode  = REVIEWNONODE;

    ReviewRoot =
        MyVaCreateManagedWidget("reviews", Toplevel, NULL);

    ReviewsWidget = XtNameToWidget(ReviewRoot, "*set");
    ReviewCollect = XtNameToWidget(ReviewRoot, "*collect");

    XtAppAddActions(AppContext(Toplevel), ReviewsActionTable,
                    XtNumber(ReviewsActionTable));
    if (ReviewsButton) {
        XtAddCallback(ReviewsButton, XtNcallback,     CallToggleUpDown,
                      (XtPointer) ReviewRoot);
        XtAddCallback(ReviewRoot, XtNpopupCallback,   CallToggleOn,
                      (XtPointer) ReviewsButton);
        XtAddCallback(ReviewRoot, XtNpopdownCallback, CallToggleOff,
                      (XtPointer) ReviewsButton);
    }

    ReviewsPlan =
        AllocWidgetPlan(&ReviewBase, offsetof(Game, Next), offsetof(Game, Pos),
                        offsetof(Game, WidgetPlan), offsetof(Game, Widget),
                        &appdata.ReviewUpdateTimeout, ReviewsWidget,
                        (XtPointer) "reviews", WidgetReviewCompare,
                        ReviewProperties, ReviewWidgetCreate, 0, 0);

    AllowResize = XtNameToWidget(ReviewRoot, "*allowResize");
    if (AllowResize) {
        XtVaGetValues(AllowResize, XtNstate, (XtArgVal) &state, NULL);
        XtVaSetValues(ReviewRoot, XtNallowShellResize, (XtArgVal) state, NULL);
        XtAddCallback(AllowResize, XtNcallback, CallAllowShellResize,
                      (XtPointer) ReviewRoot);
        /*(This code will be needed as soon as reviews is not the only entry
        if (ReviewsWidget) {
            XtVaSetValues(ReviewsWidget, XtNallowResize,
                          (XtArgVal) state, NULL);
            XtAddCallback(AllowResize, XtNcallback, CallAllowResize,
                          (XtPointer) ReviewsWidget);
        }
        */
    }

    XtRealizeWidget(ReviewRoot);
    XtInstallAllAccelerators(ReviewCollect, ReviewCollect);
    DeleteProtocol(ReviewRoot);
    if (ReviewsButton)
        CallToggleUpDown(ReviewsButton, (XtPointer) ReviewRoot, NULL);
}

static void DestroyReview(Game *Entry);
void CleanReviews(void)
{
    if (ReviewToWidget.Convert) FreeToWidget(&ReviewToWidget);

    if (ReviewGlobal) {
        FreeNumValList(ReviewGlobal);
        ReviewGlobal = NULL;
    }
    if (ReviewLocal) {
        FreeNumNameListList(ReviewLocal);
        ReviewLocal = NULL;
    }
    while (ReviewBase.Next != &ReviewBase) {
        ReviewBase.Next->Widget = NULL;
        DestroyReview(ReviewBase.Next);
    }
    XtDestroyWidget(ReviewRoot);
    Reviewed   = NULL;
    ReviewNode = REVIEWNONODE;
}

static Game *MakeReview(const char *Title, int TitleLength, int Type)
{
    Game *Ptr;

    if (TitleLength < 0) TitleLength = strlen(Title);

    if (DebugFun) {
        printf("MakeReview(%.*s, %d)\n", TitleLength, Title, Type);
        fflush(stdout);
    }

    Ptr = mynew(Game);
    Ptr->Black         = NULL;
    Ptr->White         = NULL;
    Ptr->Log           = NULL;
    Ptr->Observers     = NULL;
    Ptr->ServerId      = Type;
    Ptr->Komi          = NULL;
    Ptr->Move          = 0;
    Ptr->Handicap      = 0;
    Ptr->Title         = mystrndup(Title, (size_t) TitleLength);
    Ptr->TitleLength   = TitleLength;
    Ptr->Observed      = 0;
    Ptr->WantObserved  = 0;
    Ptr->NrObservers   = 0;
    Ptr->XSize         = 0;/* These are the special numbers for unknown size */
    Ptr->YSize         = 0;
    Ptr->UniversalTime = UniversalTime;
    Ptr->Widget        = 0;
    Ptr->WidgetPlan    = 0;
    Ptr->Pos           = 0;
    Ptr->Mode          = 'U';
    Ptr->Rules         = 'U';
    Ptr->Finished      = OVER;
    Ptr->WhiteCaptures = Ptr->BlackCaptures = 0;
    Ptr->Found         = NEW;
    Ptr->WhiteTime = Ptr->BlackTime = 0;
    Ptr->ByoPeriod     = 0;
    Ptr->BlackByo      = Ptr->WhiteByo = NOBYO;
    Ptr->ToMove        = Empty;
    /* The next lines assume Empty = 0, Black = 1, White = 2 --Ton */
    Ptr->Color = Empty;

    Ptr->Next      = &ReviewBase;
    Ptr->Previous  =  ReviewBase.Previous;
    Ptr->Previous->Next = Ptr->Next->Previous = Ptr;

    return Ptr;
}

static void DestroyReview(Review *Entry)
{
    if (Entry->Widget) {
        XtDestroyWidget(Entry->Widget);
        Entry->Widget = 0;
    }
    if (Entry->White) {
        FreeDummyPlayer(Entry->White);
        Entry->White = NULL;
    }
    if (Entry->Black) {
        FreeDummyPlayer(Entry->Black);
        Entry->Black = NULL;
    }
    FreeGame(Entry);
}

/* Called when the last observer on a review disappears */
void TestDeleteReview(Review *review)
{
    /* We should check wether it's still in the list, and quit if not */
}

static void DisplayReview(Game *Entry)
{
    if (Entry->Found == UNCHANGED) return;
    if (Entry->Found == DELETED) {
        if (!Entry->Widget) {
            Entry->Found = UNCHANGED;
            return;
        }
        Entry->WidgetPlan = DELETE;
    } else if (Entry->Widget) Entry->WidgetPlan = DELETE | CREATE;
    else                      Entry->WidgetPlan = CREATE;
    Entry->Found = UNCHANGED;
    WidgetPlanRefresh(ReviewsPlan);
}

static void TryReview(void)
{
    Game *review;

    if (DebugFun) {
        printf("TryReview() (ReviewNode = %d)\n", ReviewNode);
        fflush(stdout);
    }

    if (ReviewNode == REVIEWNONODE)
        for (review = ReviewBase.Next; review != &ReviewBase;
             review = review->Next)
            if (review->WantObserved) {
                ReviewNode = 0;
                Reviewed = review;
                SendCommand(NULL, (XtPointer) 1, "review %s", review->Title);
                break;
            }
}

static void WantReview(Game *review)
{
    if (DebugFun) {
        printf("WantReview(\"%s\")\n", review->Title);
        fflush(stdout);
    }

    if (review->Observed || review->WantObserved) return;
    review->WantObserved = 1;
    TryReview();
    review->Found = CHANGED;
    DisplayReview(review);
}

static void WantNoReview(Game *review)
{
    if (DebugFun) {
        printf("WantNoReview(%s)\n", review->Title);
        fflush(stdout);
    }

    if (review->Observed || !review->WantObserved) return;
    review->WantObserved = 0;
    review->Found = CHANGED;
    DisplayReview(review);
}

/* Check if name is in list. If not so make entry. Return entry */
/* If you type a name by hand longer than an existing one and it fails,
   you will not be able to use the old name again --Ton */
static Game *TestReview(const char *Name, int Type)
{
    Game       *REntry;
    const char *End;
    int         Kludge;
    size_t      Length;

    if (DebugFun) {
        printf("TestReview(%s, %d)\n", Name, Type);
        fflush(stdout);
    }

    End = strchr(Name, 0);

    Kludge = 0;
    if (Type == REVIEWGAME) { /* Kludge to recognize reviewed games that are
                                 really sgf games --Ton */
        const char *ptr;
        int ch, hasdash;
        
        hasdash = 0;
        for (ptr = Name; (ch = *ptr) != 0; ptr++)
            if (ch == ' ' || (ch == '-' && ++hasdash > 1)) goto fail;
            else if (ch == '(')
                if (strncmp(ptr+1, "B) IGS", (size_t) (End-ptr-1))) goto fail;
                else break;
        if (ptr-Name == 9 || hasdash == 1) {
            End = ptr;
            Kludge = 1;
            Type = SGFGAME;
        }
      fail:;
    }

    Length = End-Name;
    for (REntry = ReviewBase.Next; REntry != &ReviewBase;
         REntry = REntry->Next)
        if (REntry->ServerId == Type) {
            if (strncmp(REntry->Title, Name,
                        (REntry->TitleLength < Length ?
                         REntry->TitleLength : Length)) == 0) {
                if (Length > REntry->TitleLength) {
                    char *NewTitle;

                    NewTitle = mystrndup(Name, Length);
                    myfree(REntry->Title);
                    REntry->Title = NewTitle;
                    REntry->TitleLength = Length;
                    REntry->Found = CHANGED;
                }
                return REntry;
            }
        }
    if (Kludge) SharedLastCommand(NULL, "%%sgf sgf %.*s", End-Name, Name);
    return MakeReview(Name, End-Name, Type);
}

void SgfList(const NameList *Sgfs)
{
    NameList *Sgf;
    Game     *REntry;
    int       i, user;

    user = UserCommandP(NULL);
    i = 0;
    for (Sgf = Sgfs->Next; Sgf != Sgfs; Sgf = Sgf->Next, i++) {
        REntry = TestReview(Sgf->Name, SGFGAME);
        if (REntry->Found != UNCHANGED) DisplayReview(REntry);
        if (user) {
            Outputf("%30s", REntry->Title);
            if (i&1) Output("\n");
            else     Output("    ");
        }
    }
    if (user && (i&1)) Output("\n");
}

void ReviewList(const NameList *Reviews)
{
    NameList *review;
    Game     *REntry, *Next;

    if (DebugFun) {
        printf("ReviewList(" PRTPTR ")\n", Reviews);
        fflush(stdout);
    }

    WantReviews = 0;
    for (REntry = ReviewBase.Next; REntry != &ReviewBase;
         REntry = REntry->Next)
        if (REntry->ServerId == REVIEWGAME) REntry->Found = DELETED;

    for (review = Reviews->Next; review != Reviews; review = review->Next) {
        REntry = TestReview(review->Name, REVIEWGAME);
        if (REntry->Found == DELETED) REntry->Found = UNCHANGED;
    }

    for (REntry = ReviewBase.Next; REntry != &ReviewBase; REntry = Next) {
        Next = REntry->Next;
        switch(REntry->Found) {
          case DELETED:
            DestroyReview(REntry);
            break;
          case NEW:
            DisplayReview(REntry);
            break;
          default:
            break;
        }
    }
    TryReview();
}

static void ReviewCleanup(void)
{
    NumVal          *Temp1;
    NumNameListList *Temp2;

    if (DebugFun) {
        printf("ReviewCleanup()\n");
        fflush(stdout);
    }

    if (!Reviewed)
        Raise1(AssertException, "Stop review while not reviewing !?");

    ReviewNode = REVIEWNONODE;
    Reviewed   = NULL;

    if (ReviewGlobal && (Temp1 = ReviewGlobal->Next) != ReviewGlobal) {
        Temp1->Previous = ReviewGlobal->Previous;
        Temp1->Previous->Next = Temp1;
        FreeNumValList(Temp1);
        ReviewGlobal->Previous = ReviewGlobal->Next = ReviewGlobal;
    }
    if (ReviewLocal && (Temp2 = ReviewLocal->Next) != ReviewLocal) {
        Temp2->Previous = ReviewLocal->Previous;
        Temp2->Previous->Next = Temp2;
        FreeNumNameListList(Temp2);
        ReviewLocal->Previous = ReviewLocal->Next = ReviewLocal;
    }
}

void UnReview(Connection conn)
{
    if (DebugFun) {
        printf("UnReview(" PRTPTR ")\n", conn);
        fflush(stdout);
    }

    if (Reviewed) {
        DestroyObservers(&Reviewed->Observers);
        if (Reviewed->Log) {
	    FreeGamelog(Reviewed->Log);
            Reviewed->Log = NULL;
        }
        ReviewCleanup();
    }
}

void ReviewNotFound(void)
{
    Game *KillIt;

    KillIt = Reviewed;
    if (Reviewed) {
        if (Reviewed->ServerId == REVIEWGAME)
            UserSendCommand(NULL, NULL, "revie");
        if (Reviewed->WantObserved) {
            Reviewed->WantObserved = 0;
            Reviewed->Found = CHANGED;
            DisplayReview(Reviewed);
        }
    }
    UnReview(NULL);
    if (KillIt) DestroyReview(KillIt);
}

void ReviewStart(const char *Name)
{
    size_t   Length;
    char    *NewTitle;

    if (DebugFun) {
        printf("ReviewStart(%s)\n", Name);
        fflush(stdout);
    }

    if (!Reviewed) {
        /*Raise1(AssertException, "start of review while not reviewing !?");*/
        /* Allow this for relog */
        Reviewed = TestReview(Name, REVIEWGAME);
        if (!Reviewed->Observed) Reviewed->WantObserved = 1;
    }

    ReviewNode = 0;
    DropVariations = 0;
    ReviewMessage = ReviewError = 0;

    if (Reviewed->WantObserved) LastCommand(NULL, "forward");
    else                        UserSendCommand(NULL, NULL, "revie");

    Length = strlen(Reviewed->Title);
    if (strncmp(Name, Reviewed->Title, Length) == 0 && Name[Length]) {
        Length   = strlen(Name);
        NewTitle = mystrndup(Name, Length);
        myfree(Reviewed->Title);
        Reviewed->Title       = NewTitle;
        Reviewed->TitleLength = Length;
        Reviewed->Found = CHANGED;
        DisplayReview(Reviewed);
    }
}

static void SpuriousReview(void)
{
    Warning("Received review information but as far as I know you are "
            "not reviewing a game. Trying to stop spurious review\n");
    ReviewEnd(2);
}

void ReviewEntryBegin(int Nr)
{
    if (ReviewNode >= 0) {
        Nr++;
        if (Nr >= ReviewNode) {
            ReviewNode = Nr+1;
            ReviewLocal->Num = ReviewNode-2;
        } else ReviewEnd(2);
    }
}

static void ReviewMove(Game *game, NumNameListList *MoveEntry)
{
    int       i, Nr, x, y, node, nodes, Color;
    Gamelog  *Log;
    char     *move;
    NameList *From, *Here;
    NumNameListList *Ptr;
    StoneList *Stones, *Stone;

    Log = game->Log;
    if (!Log) Raise1(AssertException, "Game does not have a gamelog");
    Nr  = 1+XTPOINTER_TO_INT(FindComment(Log, &MoveFun));
    node = NodeNumber(Log);
    AddComment(Log, &NextMoveFun, INT_TO_XTPOINTER(node+1));
    switch(MoveEntry->Num) {
      case retBLACK:
        Color = Black;
        goto simpleMove;
      case retWHITE:
        Color = White;
      simpleMove:
        move = MoveEntry->Value->Name;
        if (move[0] == 0 || move[1] == 0 || move[2] != 0) {
            ReviewError = 1;
            GameMessage(game, "----------", "Invalid move %s ignored", move);
            goto done;
        }
        x = move[0]-'a';
        y = move[1]-'a';
        if ((x == 19 && y == 19 && 19 >= game->XSize && 19 >= game->YSize) ||
            (x == game->XSize && y == game->YSize)) {
            DoPass(Log, (BWPiece)Color);
            AddComment(Log, &LastMoveFun, LastFromXY(game, -1, 0));
            ReviewMessage = 1;
            GameMessage(game, "..........", "%s passed",
                        Color == Black ? "Black" : "White");
        } else if (0>x || x >= game->XSize || 0>y || y >= game->YSize) {
            ReviewError = 1;
            GameMessage(game, "----------", "Invalid move %s ignored", move);
            goto done;
        } else {
            y = game->YSize-y-1;
            DoMove(Log, x, y, (BWPiece)Color);
            AddComment(Log, &LastMoveFun, LastFromXY(game, x, y));
        }
        break;
      case retBLACKSET:
      case retWHITESET:
      case retEMPTYSET:
        Stones = NULL;

        MoveEntry->Previous->Next = NULL;
        for (Ptr = MoveEntry; Ptr; Ptr = Ptr->Next) {
            switch(Ptr->Num) {
              case retBLACKSET: Color = Black; break;
              case retWHITESET: Color = White; break;
              case retEMPTYSET: Color = Empty; break;
              default:
                Raise1(AssertException, "impossibble color type");
                Color = Empty;
                break;
            }
            From = Ptr->Value;
            for (Here = From->Next; Here != From; Here = Here->Next) {
                move = Here->Name;
                if (move[0] == 0 || move[1] == 0 || move[2] != 0) {
                    Warning("Invalid stone %s ignored\n", move);
                    goto done;
                }
                x = move[0]-'a';
                y = move[1]-'a';
                if (0>x || x >= game->XSize || 0>y || y >= game->YSize) {
                    Warning("Invalid stone %s ignored\n", move);
                    goto done;
                }
                Stone = mynew(StoneList);
                Stone->x = x;
                Stone->y = game->YSize-1-y;
                Stone->Color = Color;
                Stone->Next  = Stones;
                Stones = Stone;
            }
        }
        SetStones(Log, Stones);
        FreeStones(Stones);
        AddComment(Log, &LastMoveFun, LastFromXY(game, -1, -1));
        Color = Empty;
        break;
      default:
        Raise1(AssertException, "ReviewwMove called on invalid move type");
        Color = Empty;
    }
    AddComment(Log, &PrevMoveFun, INT_TO_XTPOINTER(node));
    AddComment(Log, &MoveFun,     INT_TO_XTPOINTER(Nr));
    game->Move   = Nr;
    game->ToMove = OpponentColor(Color);
    ShowObserve(game);
    nodes = NumberNodes(Log)-1;
    for (i=NodeNumber(Log); i<nodes; i++) DownGamelog(Log);
  done:
    FreeNumNameListList(MoveEntry);
}

static void FlushNode(void)
{
    Gamelog         *log;
    NumNameListList *Entry, *MoveEntry, *Next;
    NameList        *Entries, *Here;
    int              i, nodes;
    char           **Text, **Ptr;
    char            *From, *To, *End;

    if (Reviewed) {
        if (ReviewLocal->Next != ReviewLocal) {
            log = Reviewed->Log;
            if (log) {
                nodes = NumberNodes(log)-1;
                for (i=NodeNumber(log); i<nodes; i++) DownGamelog(log);
                MoveEntry = NULL;
                for (Entry = ReviewLocal->Next; Entry != ReviewLocal;
                     Entry = Next) {
                    Next = Entry->Next;
                    switch(Entry->Num) {
                      case retBLACK:
                      case retWHITE:
                        if (MoveEntry) {
                            ReviewError = 1;
                            GameMessage(Reviewed, "----------", "Spurious move"
                                        " entry %s just before node %d",
                                        Entry->Value->Name, ReviewLocal->Num);
                            ReviewMove(Reviewed, MoveEntry);
                        }
                        Entry->Previous->Next = Next;
                        Entry->Next->Previous = Entry->Previous;
                        MoveEntry = Entry;
                        MoveEntry->Previous = MoveEntry->Next = MoveEntry;
                        break;
                      case retBLACKSET:
                      case retWHITESET:
                      case retEMPTYSET:
                        if (MoveEntry &&
                            MoveEntry->Num != retBLACKSET &&
                            MoveEntry->Num != retWHITESET &&
                            MoveEntry->Num != retEMPTYSET) {
                            ReviewError = 1;
                            GameMessage(Reviewed, "----------", "Spurious move"
                                        " entry %s just before node %d",
                                        Entry->Value->Name, ReviewLocal->Num);
                            ReviewMove(Reviewed, MoveEntry);
                            MoveEntry = NULL;
                        }
                        Entry->Previous->Next = Next;
                        Entry->Next->Previous = Entry->Previous;
                        if (MoveEntry) {
                            Entry->Next = MoveEntry;
                            Entry->Previous = MoveEntry->Previous;
                            Entry->Previous->Next = Entry->Next->Previous =
                                Entry;
                        } else {
                            MoveEntry = Entry;
                            MoveEntry->Previous = MoveEntry->Next = MoveEntry;
                        }
                        break;
                    }
                }
                if (!MoveEntry && AfterStart) {
                    /* Use a fake list in the canonical processing */
                    Entries = mynew(NameList);
                    Entries->Name = NULL;
                    Entries->Next = Entries->Previous = Entries;

                    MoveEntry = mynew(NumNameListList);
                    MoveEntry->Next = MoveEntry->Previous = MoveEntry;
                    MoveEntry->Num  = retEMPTYSET;
                    MoveEntry->Value = Entries;
                }
                AfterStart = 1;
                if (MoveEntry) ReviewMove(Reviewed, MoveEntry);
                while ((Entry = ReviewLocal->Next) != ReviewLocal) {
                    switch(Entry->Num) {
                      case retCOMMENT:
                        End = strchr(Entry->Value->Name, 0);
/* Depends on what the sprintf routines in GameMessage can handle */
#define MAXBUF 1000 
                        if (End > Entry->Value->Name+MAXBUF) {
                            for (From = Entry->Value->Name;
                                 (To = memchr(From, '\n',
                                              (size_t)(End-From))) != NULL;
                                 From = To+1) {
                                *To = 0;
                                GameMessage(Reviewed, From, "");
                                *To = '\n';
                            }
                            GameMessage(Reviewed, From, "");
                        } else GameMessage(Reviewed, Entry->Value->Name, "");
                        ReviewMessage = 1;
                        break;
                      case retNODENAME:
                        AddLocalProperty(log, "NodeName",
                                         Entry->Value->Name, -1);
                        break;
                      case retBLACKTIME:
                        AddLocalProperty(log, "TimeLeftBlack",
                                         Entry->Value->Name, -1);
                        break;
                      case retWHITETIME:
                        AddLocalProperty(log, "TimeLeftWhite",
                                         Entry->Value->Name, -1);
                        break;
                      case retLETTERS:
                        Entries = Entry->Value;
                        i = 0;
                        for (Here = Entries->Next; Here != Entries;
                             Here = Here->Next) i++;
                        Ptr = Text = mynews(char *, i);
                        WITH_UNWIND {
                            for (Here = Entries->Next; Here != Entries;
                                 Here = Here->Next, Ptr++) *Ptr = Here->Name;
                            AddLocalProperties(log, "PositionLetters", i,
                                               (const char **) Text, NULL);
                        } ON_UNWIND {
                            myfree(Text);
                        } END_UNWIND;
                        break;
                    }
                    FreeNameList(Entry->Value);
                    Entry->Previous->Next = Entry->Next;
                    Entry->Next->Previous = Entry->Previous;
                    myfree(Entry);
                }
            } else {
                ReviewError = 1;
                GameMessage(Reviewed, "----------",
                            "Throwing away review entries");
                while ((Entry = ReviewLocal->Next) != ReviewLocal) {
                    FreeNameList(Entry->Value);
                    Entry->Previous->Next = Entry->Next;
                    Entry->Next->Previous = Entry->Previous;
                    myfree(Entry);
                }
            }
        }
        if (ReviewError) {
            ReviewError = 0;
            ObserveError(Reviewed->Observers);
        }
        if (ReviewMessage) {
            ReviewMessage = 0;
            ObserveMessage(Reviewed->Observers);
        }
    } else SpuriousReview();
}

void ReviewNewNode(void)
{
    if (DebugFun) {
        printf("ReviewNewNode()\n");
        fflush(stdout);
    }

    if (!DropVariations) FlushNode();
}

void ReviewOpenVariation(void)
{
    if (DebugFun) {
        printf("ReviewOpenVariation()\n");
        fflush(stdout);
    }

    /* Output("Open variation\n"); */
}

void ReviewCloseVariation(void)
{
    if (DebugFun) {
        printf("ReviewCloseVariation()\n");
        fflush(stdout);
    }

    if (!Reviewed)
        Raise1(AssertException, "close variation while not reviewing !?");
    if (!DropVariations) {
        FlushNode();
        DropVariations = 1;
        ReviewMessage = 1;
        GameMessage(Reviewed, "----------", "Ignoring variations");
        ReviewEnd(2); /* Non zero means stop reviewing */
    }
    /* Output("Close variation\n"); */
}

void ReviewGlobalProperty(int Num, const char *Value)
{
    Gamelog    *Log;
    NumVal     *Entry;
    int         Temp;
    char        Buffer[80];
    char       *ptr;

    if (Reviewed) {
        Log = Reviewed->Log;
        if (Log)
#define AddProp(Name, Value) AddGlobalProperty(Log, Name, Value, -1)
            switch(Num) {
              case retGAME:
                Temp = atoi(Value);
                if (Temp != SGFGO) {
                    ReviewError = 1;
                    GameMessage(Reviewed, "----------", "This turns out not to"
                                "be a game of go, but a game of the "
                                "unsupported type %d", Temp);
                    ReviewEnd(2); /* non zero means stop reviewing */
                }
                break;
              case retKOMI:
                strtol(Value, &ptr, 10);
                if (*ptr == '.') {
                    strtol(ptr+1, &ptr, 10);
                    if (*--ptr == '0') {
                        do {
                            ptr--;
                        } while (*ptr == '0');
                        Temp = (const char *) ptr - Value + 1;
                        if (Temp < sizeof(Buffer)) {
                            sprintf(Buffer, "%.*s", Temp, Value);
                            Value = Buffer;
                        }
                    }
                }
                CHANGESTRING(Reviewed, Komi, Value,
                             AddGlobalProperty(Log, "Komi", Value, -1);
                             SetKomi(Reviewed->Observers, Value));
                break;
              case retHANDICAP:
                Temp = atoi(Value);
                CHANGEINT(Reviewed, Handicap, Temp, PROPHANDICAP(Temp);
                          SetHandicap(Reviewed->Observers, Temp));
                break;
              case retENTEREDBY:
                AddProp("EnteredBy", Value);
                break; 
              case retCOPYRIGHT:
                AddProp("Copyright", Value);
                break;
              case retPLACE:
                AddProp("Place", Value);
                break; 
              case retDATE:
                AddProp("Date", Value);
                break; 
              case retRESULT:
                AddProp("Result", Value);
                break; 
              case retTOURNAMENT:
                AddProp("Tournament", Value);
                break; 
              case retNAME:
                AddProp("Name", Value);
                break; 
              case retSIZE:
                Temp = atoi(Value);
                if (Temp != Log->SizeX || Temp != Log->SizeY) {
                    ReviewError = 1;
                    GameMessage(Reviewed, "----------", "This turns out to"
                                "be a game with inconsistent size information."
                                "Bailing out.");
                    ReviewEnd(2); /* non zero means stop reviewing */
                }
                break;
              case retWHITESTRENGTH:
                AddProp("WhiteStrength", Value);
                CheckPlayerStrength(Reviewed->White, Value);
                break; 
              case retBLACKSTRENGTH:
                AddProp("BlackStrength", Value);
                CheckPlayerStrength(Reviewed->Black, Value);
                break; 
              case retBLACKNAME:
                AddProp("BlackName", Value);
                RenameDummyPlayer(Reviewed->Black, Value);
                SetObserveDescriptions(Reviewed->Observers, Reviewed);
                break; 
              case retWHITENAME:
                AddProp("WhiteName", Value);
                RenameDummyPlayer(Reviewed->White, Value);
                SetObserveDescriptions(Reviewed->Observers, Reviewed);
                break;
              default:
                sprintf(Buffer, "Invalid global property %d", Num);
                Raise1(AssertException, ExceptionCopy(Buffer));
                break;
            }
#undef AddProp
        else {
            if (Num == retGAME) {
                Temp = atoi(Value);
                if (Temp != SGFGO) {
                    Warning("Review %s is not a game of go, but a game of the"
                            " unsupported type %d\n", Reviewed->Title, Temp);
                    ReviewEnd(2); /* non zero means stop reviewing */
                }
            } else if (Num == retSIZE) {
                Temp = atoi(Value);
                Reviewed->XSize = Reviewed->YSize = Temp;
                /* Assuming the sgf data is sensible, there will be no suicide
                   in there if it's not allowed */
                Log = AllocGamelog(Reviewed->XSize, Reviewed->YSize, 1);
                AddComment(Log, &MoveFun,     INT_TO_XTPOINTER(-1));
                AddComment(Log, &NextMoveFun, INT_TO_XTPOINTER(1));
                SetStones(Log, NULL);
                AddComment(Log, &LastMoveFun, LastFromXY(Reviewed, -1, -1));
                AddComment(Log, &PrevMoveFun, INT_TO_XTPOINTER(0));
                AddComment(Log, &MoveFun,     INT_TO_XTPOINTER(0));
                AfterStart = 0;
                Reviewed->Log = Log;
                Reviewed->White = MakeDummyPlayer();
                Reviewed->Black = MakeDummyPlayer();
                while ((Entry = ReviewGlobal->Next) != ReviewGlobal) {
                    ReviewGlobalProperty(Entry->Num, Entry->Value);
                    myfree(Entry->Value);
                    Entry->Previous->Next = Entry->Next;
                    Entry->Next->Previous = Entry->Previous;
                    myfree(Entry);
                }
                ShowObserve(Reviewed);
            } else {
                Entry = mynew(NumVal);
                Entry->Num   = Num;
                Entry->Value = mystrdup(Value);
                Entry->Next  = ReviewGlobal;
                Entry->Previous = ReviewGlobal->Previous;
                Entry->Previous->Next = Entry->Next->Previous = Entry;
            }
        }
    } else SpuriousReview();
}

void ReviewLocalProperty(int Name, const NameList *Value)
{
    NumNameListList *Entry;

    if (DropVariations) return;
    if (ReviewNode >= 0) {
        Entry = mynew(NumNameListList);
        Entry->Num   = Name;
        Entry->Value = NameListDup(Value);
        Entry->Next = ReviewLocal;
        Entry->Previous = ReviewLocal->Previous;
        Entry->Previous->Next = Entry->Next->Previous = Entry;
    }
}

void ReviewEnd(int Last)
{
    Gamelog *log;
    int      i, Nr;
    size_t   Pos;
    char    *Ptr;

    if (DebugFun) {
        printf("ReviewEnd(%d)\n", Last);
        fflush(stdout);
    }

    if (Last) {
        FlushNode();
        ReviewNode = REVIEWBADNODE;
        if (Reviewed) {
            if (Reviewed->WantObserved) {
                Reviewed->Observed = 1;
                Reviewed->WantObserved = 0;
                Reviewed->Found = CHANGED;
                DisplayReview(Reviewed);
            }
            log = Reviewed->Log;
            if (log) {
                Nr = GetGlobalProperty(log, "Result", -1, 0, NULL, NULL, 0);
                for (i=0; i<Nr; i++) {
                    Pos = 0;
                    Ptr = NULL;
                    GetGlobalProperty(log, "Result", 0, 1, &Pos, &Ptr, 1);
                    ReviewMessage = 1;
                    GameMessage(Reviewed, "----------", "%s", Ptr);
                }
            }
        }
#ifdef DO_REVIEW
        UserSendCommand(NULL, NULL, "revie");
#endif
    } else if (ReviewNode >= 0) 
        if (Reviewed->WantObserved) LastCommand(NULL, "forward");
#ifdef DO_REVIEW
        else                        UserSendCommand(NULL, NULL, "revie");
#endif
}

void ReviewStop(void)
{
    int Temp;

    if (DebugFun) {
        printf("ReviewStop()\n");
        fflush(stdout);
    }

    if (!Reviewed)
        Raise1(AssertException, "Stop review while not reviewing !?");

    if (Reviewed->WantObserved) {
        Reviewed->WantObserved = 0;
        Reviewed->Found = CHANGED;
    }
    DisplayReview(Reviewed);
    if (Reviewed->Observed) ReviewCleanup();
    else                    UnReview(NULL);
    if (WantReviews) {
        Temp = WantReviews;
        WantReviews = 0;
        ReviewListWanted(NULL, Temp);
    }
    TryReview();
}

void ReviewListWanted(Connection conn, int Type)
{
#ifdef DO_REVIEW
    if (Reviewed) {
        if (Type > WantReviews) WantReviews = Type;
    } else if (Type > 1) UserCommand(conn, "review");
    else                 SendCommand(NULL, NULL, "review");
#endif
}

void ReviewWanted(Connection conn, const char *Name, int Type)
{
    while (isspace(*Name)) Name++;
    if (*Name) WantReview(TestReview(Name, REVIEWGAME));
    else ReviewListWanted(conn, Type);
}

void CheckReview(Connection conn)
{
    if (Reviewed && ReviewNode != REVIEWBADNODE) {
        /* There should be a review or forward command pending */
        ReviewError = 1;
        GameMessage(Reviewed, "----------",
                    "Something went wrong during review");
        ReviewEnd(2);
    }
}

static void DoReview(Widget w, XEvent *evnt, String *str, Cardinal *n)
{
    if (*n) XtCallCallbacks(w, XtNcallback, (XtPointer) str[0]);
    else    XtCallCallbacks(w, XtNcallback, (XtPointer) "none");
}

static void CallReview(Widget w, XtPointer clientdata, XtPointer calldata)
{
    Game    *review;
    Observe *observe;
    String   text;

    review = (Game *) clientdata;
    text   = (String) calldata;
    if (strcmp(text, GAME_OBSERVE) == 0 ||
        strcmp(text, GAME_STATUS) == 0)
        if (review->Log) {
            observe = OpenObserve(review);
            GotoObserve(observe, 1);
        } else WantReview(review);
    else if (strcmp(text, GAME_OBSERVERS) == 0) WantNoReview(review);
    else if (strcmp(text, DUMPGAME) == 0) DumpGame(review);
    else WidgetWarning(w, "Unknown argument to doreview");
}

void DumpReviews(const char *args)
{
    const Review *review;

    Outputf("Reviewed = " PRTPTR "(%s)\n",
            Reviewed, Reviewed ? GameLongDescription(Reviewed) : "NULL");
    Outputf("WantReviews = %d, DropVariations = %d, AfterStart = %d"
            ", ReviewNode = %d",
            WantReviews, DropVariations, AfterStart, ReviewNode);
    for (review = ReviewBase.Next; review != &ReviewBase;
         review = review->Next)
        DumpGame(review);
}

int ReviewP(const Game *game)
{
    return game->ServerId <= REVIEWGAME;
}

void ReviewsTime(unsigned long diff)
{
    Game *game;

    for (game = ReviewBase.Next; game != &ReviewBase; game = game->Next)
        if (game->Log)
            ReplayTime(diff, game->Observers, NumberNodes(game->Log)-1);
}
