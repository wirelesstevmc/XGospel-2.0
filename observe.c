#include <string.h>
#if !STDC_HEADERS && HAVE_MEMORY_H
# include <memory.h>
#endif /* not STDC_HEADERS and HAVE_MEMORY_H */
#include <stddef.h>
#include <stdlib.h>

#include <X11/StringDefs.h>
#include <X11/Intrinsic.h>
#include <X11/Shell.h>
#include <X11/Xaw/AsciiText.h>
extern unsigned long FMT8BIT;   /* Seems to be missing on CONVEX */
#include <X11/Xaw/Toggle.h>
#include <X11/Xaw/Scrollbar.h>

#include <except.h>
#include <mymalloc.h>
#include <myxlib.h>

#include "analyze.h"
#include "connect.h"
#include "observe.h"
#include "games.h"
#include "gospel.h"
#include "stats.h"
#include "tell.h"
#include "utils.h"
#include "xgospel.h"

#ifdef    HAVE_NO_MEMCHR_PROTO
extern void *memchr(const void *s, int c, size_t n);
#endif /* HAVE_NO_MEMCHR_PROTO */

struct _Observe {
    struct _Observe *Next;
    Game	   *Game;
    size_t          Node;
    Boolean         Redraw, Scoring;
    int             ReplayRate, ReplayCount;
    char           *SgfFile, *PsFile, *KibitzFile;
    Widget          Root,  BoardWidget, InfoWidget, InputWidget, ChatterWidget;
    Widget          TimeWidget, Pass, Done, CaptureWidget, ScrollWidget;
    Widget          KomiWidget, HandicapWidget, MoveWidget, Replay, Title;
    Widget          MoveBeepWidget, MoveRaiseWidget, ErrorBeep, ErrorRaise;
    Widget          KibitzBeep, KibitzRaise, Score;
    Widget          SgfOverwrite, KibitzOverwrite, PsOverwrite;
    Widget          SgfFromStart, PsFromStart;
    int             lastX, lastY; /* x,y at last button down */
    Time            lastTime;     /* time of last button down */
};

static void SendKibitz(Widget w,  XEvent *event,String *string, Cardinal *n);
static void SendChatter(Widget w,  XEvent *event,String *string, Cardinal *n);
static void ChangeTitle(Widget w, XEvent *event,String *string, Cardinal *n);
static void ObserveGoto(Widget w, XEvent *event, String *string, Cardinal *n);

extern Observe *_FindObserveWidget(Widget w, int offset);
extern Observe *_FindBoardWidget(Widget w);

static XtActionsRec actionTable[] = {
    { (String) "kibitz",      SendKibitz  },
    { (String) "chatter",     SendChatter },
    { (String) "changetitle", ChangeTitle },
    { (String) "observegoto", ObserveGoto },
    { (String) "analyzegoto", AnalyzeGoto }
};

static RegexYesNo KibitzYesNo;

static Pixel defaultTimeBackground, defaultTimeForeground;
static Pixel defaultObserveBackground, defaultObserveForeground;

static void CallDestroyObserve(Widget w,
                               XtPointer clientdata, XtPointer calldata);
static void ButtonDown(Widget w, XtPointer clientdata, XtPointer calldata);
static void ButtonUp(Widget w, XtPointer clientdata, XtPointer calldata);

int ObserveMove(const Observe *observe)
{
    Game    *game;
    Gamelog *log;
    int      i, nodes, Now, Pos;

    game    = observe->Game;
    log     = GameGamelog(game);
    if (!log) Raise1(AssertException, "Saving observation without a game !?");
    nodes   = NumberNodes(log);
    Now     = NodeNumber(log);
    Pos     = observe->Node;
    
    if      (Pos < 1)      Pos = 1;
    else if (Pos >= nodes) Pos = nodes-1;
    
    if      (Pos > Now) for (i=Now; i<Pos; i++) DownGamelog(log);
    else if (Pos < Now) for (i=Now; i>Pos; i--) UpGamelog(log);
    return XTPOINTER_TO_INT(FindComment(log, &MoveFun));
}

/* Returns the white advantage if the observed game is at the most recent
 * position, UNKNOWN_ADVANTAGE otherwise.
 */
float GetWhiteAdvantage(const Observe *observe) {
    Game    *game = observe->Game;
    Gamelog *log = GameGamelog(game);
    int      nodes = NumberNodes(log)-1;
    float   whiteAdvantage;
    int     wCaptures, bCaptures;

    if (observe->Node == nodes) {
	sscanf(GameCaptures(game), "B:%d, W:%d", &bCaptures, &wCaptures);
	sscanf(GameKomi(game), "%f", &whiteAdvantage);
	whiteAdvantage += wCaptures - bCaptures;
	return whiteAdvantage;
    } else {
	return  UNKNOWN_ADVANTAGE;
    }
}

static void ToggleReplay(Widget w, XtPointer clientdata, XtPointer calldata)
{
    Observe *observe;
    Game    *game;
    Gamelog *log;
    int      nodes;
    
    observe = (Observe *) clientdata;
    if (False == (Boolean) XTPOINTER_TO_INT(calldata)) observe->ReplayRate = 0;
    else {
        observe->ReplayRate  = appdata.ReplayTimeout;
        game  = observe->Game;
        log   = GameGamelog(game);
        nodes = NumberNodes(log)-1;
        if (observe->Node == nodes) {
            GotoObserve(observe, 1);
            observe->ReplayCount = 0;
        } else observe->ReplayCount = observe->ReplayRate-1;
    }
}

static void ToggleScoring(Widget w, XtPointer clientdata, XtPointer calldata)
/* Refresh the board with or without marks */
{
    Observe *observe = (Observe *) clientdata;

    observe->Scoring = !observe->Scoring;
    observe->Redraw = True;
    GotoObserve(observe, observe->Node);
}

static void ChangeKibitzPass(Widget w,
                             XtPointer clientdata, XtPointer calldata)
{
    Observe *observe;

    observe = (Observe *) clientdata;
    EditStringList(observe->Root, "Edit kibitz pass pattern",
                   "Invalid pass pattern",
                   &appdata.KibitzPass, &appdata.KibitzKill,
                   &KibitzYesNo, 0);
}

static void ChangeKibitzKill(Widget w,
                             XtPointer clientdata, XtPointer calldata)
{
    Observe *observe;

    observe = (Observe *) clientdata;
    EditStringList(observe->Root, "Edit kibitz kill pattern",
                   "Invalid kill pattern",
                   &appdata.KibitzPass, &appdata.KibitzKill,
                   &KibitzYesNo, 1);
}

static void AnalyzeGame(Widget w, XtPointer clientdata, XtPointer calldata)
{
    Observe *observe;
    Game    *game;
    
    observe = (Observe *) clientdata;
    game    = observe->Game;
    OpenAnalyze(observe->BoardWidget, GameDescription(game),
                0, GameXSize(game), ObserveMove(observe),
                GameAllowSuicide(game), GetWhiteAdvantage(observe));
}

static void DupGame(Widget w, XtPointer clientdata, XtPointer calldata)
{
    Observe *observe, *newobserve;
    
    observe = (Observe *) clientdata;
    newobserve = OpenObserve(observe->Game);
    GotoObserve(newobserve, observe->Node);
}

static void SaveGame(Widget w, XtPointer clientdata, XtPointer calldata)
{
    Observe *observe;
    Game    *game;
    Gamelog *log;
    Boolean  Overwrite, FromStart;
    int      i, Now, node;

    observe = (Observe *) clientdata;
    game    = observe->Game;
    log     = GameGamelog(game);
    if (log) {
        Now     = NodeNumber(log);

        w = observe->SgfFromStart;
        if (w) XtVaGetValues(w, XtNstate, (XtArgVal) &FromStart, NULL);
        else   FromStart = True;

        if (FromStart != False) node = 1;
        else                    node = observe->Node;

        if      (Now < node) for (i = Now; i < node; i++) DownGamelog(log);
        else if (Now > node) for (i = Now; i > node; i--) UpGamelog(log);

        w = observe->SgfOverwrite;
        if (w) XtVaGetValues(w, XtNstate, (XtArgVal) &Overwrite, NULL);
        else   Overwrite = False;
        
        SaveWrite(observe->SgfFile, Overwrite,
                  observe->ErrorBeep, observe->ErrorRaise,
                  "Sgf save error", WriteSgfFun, (XtPointer) log);
    } else Raise1(AssertException, "Saving observation without a game !?");
}

static void SavePs(Widget w, XtPointer clientdata, XtPointer calldata)
{
    Observe *observe;
    Game    *game;
    Gamelog *log;
    Boolean  Overwrite, FromStart;
    int      i, Now, node;

    observe = (Observe *) clientdata;
    game    = observe->Game;
    log     = GameGamelog(game);
    if (log) {
        Now     = NodeNumber(log);

        w = observe->PsFromStart;
        if (w) XtVaGetValues(w, XtNstate, (XtArgVal) &FromStart, NULL);
        else   FromStart = True;

        if (FromStart != False) node = 1;
        else                    node = observe->Node;

        if      (Now < node) for (i = Now; i < node; i++) DownGamelog(log);
        else if (Now > node) for (i = Now; i > node; i--) UpGamelog(log);

        w = observe->PsOverwrite;
        if (w) XtVaGetValues(w, XtNstate, (XtArgVal) &Overwrite, NULL);
        else   Overwrite = False;
        
        SaveWrite(observe->PsFile, Overwrite,
                  observe->ErrorBeep, observe->ErrorRaise,
                  "Postscript save error", WritePsFun, (XtPointer) log);
    } else Raise1(AssertException, "Saving observation without a game !?");
}

static void SaveKibitz(Widget w, XtPointer clientdata, XtPointer calldata)
{
    Widget     kibitz;
    Observe   *observe;
    Boolean    Overwrite;

    observe = (Observe *) clientdata;
    kibitz = observe->InfoWidget;
    if (kibitz) {
        w = observe->KibitzOverwrite;
        if (w) XtVaGetValues(w, XtNstate, (XtArgVal) &Overwrite, NULL);
        else   Overwrite = False;

        SaveWrite(observe->KibitzFile, Overwrite,
                  observe->ErrorBeep, observe->ErrorRaise,
                  "Kibitz save error", SaveTextFun, (XtPointer) kibitz);
    } else {
        IfBell(observe->ErrorBeep);
        IfRaise(observe->ErrorRaise, observe->ErrorRaise);
        PopMessage("Kibitz save error", "Kibitzes were ignored so there is "
                   "nothing to be saved");
    }
}

void GotoObserve(Observe *observe, size_t Pos)
{
    Game    *game;
    Gamelog *log;
    int      i, x, y, Now, nodes, move;
    float    Step;
    Widget   w;
    char     Text[10];

    game    = observe->Game;
    log     = GameGamelog(game);
    if (log) {
        nodes   = NumberNodes(log)-1;
        Now     = NodeNumber(log);
        Step    = 1.0 / nodes;

        if      (Pos < 1)     Pos = 1;
        else if (Pos > nodes) Pos = nodes;

        if      (Pos > Now) for (i=Now; i<Pos; i++) DownGamelog(log);
        else if (Pos < Now) for (i=Now; i>Pos; i--) UpGamelog(log);

        w = observe->BoardWidget;
        if (w) {
            if (Pos != observe->Node || observe->Redraw) {
		float   whiteAdvantage;
		int     wCaptures, bCaptures;

		sscanf(GameCaptures(game), "B:%d, W:%d",
		       &bCaptures, &wCaptures);
		sscanf(GameKomi(game), "%f", &whiteAdvantage);
		whiteAdvantage += wCaptures - bCaptures;

		DisplayScore(observe->Scoring, log, whiteAdvantage,
			     w, observe->Root);
            }
            /* These are outside the previous if so that a non accepted move's
               blinking cross gets reset */
            XYFromLast(game, &x, &y, FindComment(log, &LastMoveFun));
            LastMove(w, x, y);
        }

        observe->Node = Pos;
	observe->Redraw = False;

        if (observe->ScrollWidget)
            MyScrollbarSetThumb(observe->ScrollWidget, (Pos-1)*Step, Step);

        if (observe->MoveWidget) {
            move = XTPOINTER_TO_INT(FindComment(log, &MoveFun));
            sprintf(Text, "%3d/%-3d", move, GameMove(game));
            XtVaSetValues(observe->MoveWidget,
                          XtNlabel, (XtArgVal) Text, NULL);
        }
    } else Raise1(AssertException, "Scrolling in a window without game !?");
}

static void ScrollObserve(Widget scrollbar, XtPointer client_data,
                          XtPointer pos)
{
    Observe   *observe;
    int        Pos, Pixels;
    Dimension  Length;

    observe = (Observe *) client_data;
    Pos = XTPOINTER_TO_INT(pos);

    XtVaGetValues(scrollbar, XtNlength, (XtArgVal) &Length, NULL);

    Pixels = Length / (NumberNodes(GameGamelog(observe->Game))-1);
    if (Pixels < appdata.ScrollUnit) Pixels = appdata.ScrollUnit;
    if   (Pos < 0) Pos = Pos/Pixels-1;
    else           Pos = Pos/Pixels+1;
    GotoObserve(observe, observe->Node+Pos);
}

static void JumpObserve(Widget scrollbar, XtPointer client_data, XtPointer per)
{
    Observe *observe;
    size_t   Pos;

    observe = (Observe *) client_data;
    Pos     = * (float *) per * (NumberNodes(GameGamelog(observe->Game))-1);

    GotoObserve(observe, Pos+1);
}

static void ObserveGoto(Widget w, XEvent *event, String *string, Cardinal *n)
/* used to scroll the board by forward (string > 0) or backward (string < 0).
 */
{
    Observe *observe = _FindObserveWidget(w, offsetof(Observe, ChatterWidget));

    if (observe == NULL) {
	observe = _FindObserveWidget(w, offsetof(Observe, InputWidget));
    }
    if (observe) {
	int move = (int)(observe->Node) + ResourceStringToInt(string[0]);
	GotoObserve(observe, (move <= 0 ? 1 : (size_t)move));
    } else {
	printf("ObserveGoto: observe widget not found\n");
	fflush(stdout);
    }
}

static void RefreshGame(Widget w, XtPointer clientdata, XtPointer calldata)
{
    Observe *observe;
    int      Id;

    observe = (Observe *) clientdata;
    Id = GameServerId(observe->Game);
    if (Id >= 0) {
        if (ScoringP(observe->Game)) {
	    SendCommand(NULL, NULL, "status %d", Id);
        } else if (MyTurnP(observe->Game)) {
	    /* Do not send "refresh" if I could lose on time */
	    SendCommand(NULL, NULL, "time %d", Id);
	    SendCommand(NULL, NULL, "moves %d", Id);
	} else {
	    SendCommand(NULL, NULL, "refresh %d", Id);
	    SendCommand(NULL, NULL, "moves %d", Id);
	}
    } else if (observe->InfoWidget) {
        BatchAddText(observe->InfoWidget, ".....            "
                     "Refresh not sent, game is gone\n");
    } else Output("Refresh not sent, game is gone\n");
}

static void ScoreGame(Widget w, XtPointer clientdata, XtPointer calldata)
{
    Observe *observe;
    int      Id;

    observe = (Observe *) clientdata;
    Id = GameServerId(observe->Game);
    if (Id >= 0) SendCommand(NULL, NULL, "score %d", Id);
    else if (observe->InfoWidget)
        BatchAddText(observe->InfoWidget, ".....            "
                     "Score not sent, game is gone\n");
    else Output("Score not sent, game is gone\n");
}

static void GetBets(Widget w, XtPointer clientdata, XtPointer calldata)
{
    Game    *game;
    Observe *observe;

    SendCommand(NULL, NULL, "%%bet bet");
    game = ObservedGame();
    if (!game) {
        
        observe = (Observe *) clientdata;
        if (observe->InfoWidget)
            BatchAddText(observe->InfoWidget, ".....            "
                         "Bet command sent, but it will probably fail since "
                         "you seem to be observing more than one game\n");
        else Output("Bet command sent, but it will probably fail since you "
                    "seem to be observing more than one game\n");

    }
}

static void SendMove(Widget w, XtPointer clientdata, const char *Text)
{
    Observe *observe;
    Game    *game;
    int      Id;

    observe = (Observe *) clientdata;
    game    = observe->Game;

    Id = GameServerId(game);
    /* replace pass with forward for teaching games: */
    if (Id >= 0) {
      if (TeachingP(game) && !strcmp(Text, "pass")) {
	MoveCommand(NULL, "forw");
      } else {
	MoveCommand(NULL, Text);
      }
    } else if (observe->InfoWidget)
        BatchAddText(observe->InfoWidget, ".....            "
                     "%s not sent, game is gone\n", Text);
    else Outputf("%s not sent, game is gone\n", Text);
}

static void SendUndo(Widget w, XtPointer clientdata, XtPointer calldata)
{
    SendMove(w, clientdata, "undo");
}

static void SendPass(Widget w, XtPointer clientdata, XtPointer calldata)
{
    SendMove(w, clientdata, "pass");
}

static void SendDone(Widget w, XtPointer clientdata, XtPointer calldata)
{
    SendMove(w, clientdata, "done");
}

static void CallResume(Widget w, XtPointer clientdata, XtPointer calldata)
{
    const Game *game;
    const Player *PlayerBlack, *PlayerWhite;

    game = (Game *) clientdata;
    PlayerBlack = GameBlack(game);
    PlayerWhite = GameWhite(game);
    SendCommand(NULL, NULL, "load %s-%s",
                PlayerToName(PlayerWhite), PlayerToName(PlayerBlack));
}

static void CallObservers(Widget w, XtPointer clientdata, XtPointer calldata)
{
    const Game    *game;
    const Observe *observe;
    int            Id;

    observe = (const Observe *) clientdata;
    game = observe->Game;
    Id = GameServerId(game);
    if (Id < 0) {
        if (observe->InfoWidget)
            BatchAddText(observe->InfoWidget, ".....            "
                         "Observers not sent, game is gone\n");
    } else SendCommand(NULL, (XtPointer) 2, "all %d", Id);
}

static void SetObserveDescription(Observe *observe, const Game *game)
{
    char          *Name;

    SetGameHeaders(observe->Root, game);

    Name = GameName(appdata.SgfFilename, SGFEXTENSION, game);
    myfree(observe->SgfFile);
    observe->SgfFile = Name;

    Name = GameName(appdata.PsFilename, PSEXTENSION, game);
    myfree(observe->PsFile);
    observe->PsFile = Name;

    Name = GameName(appdata.KibitzFilename, "kibitz", game);
    myfree(observe->KibitzFile);
    observe->KibitzFile = Name;
}

void SetObserveDescriptions(Observe *observers, const Game *game)
{
    Observe *observe;

    for (observe = observers; observe; observe = observe->Next)
        SetObserveDescription(observe, game);
}

Observe *OpenObserve(Game *game)
{
    Observe    *observe;
    int         sizex, sizey, Playing, Reviewing, move, hand;
    char       *title, HandicapString[10], MoveText[10];
    const char *gameTitle;
    Boolean     state;
    const Player *PlayerBlack, *PlayerWhite;
    Widget      Root, Collect, Com, Input, Chatter, PassPat, Kill;
    Widget      Refresh, analyze, Tim, undo, Pass, Scroll, Dup;
    Widget      ServerScore, Score, Bets, SgfSave, PsSave, KibitzSave;
    Widget      SgfFile, PsFile, KibitzFile;
    Widget      StatsWhite, StatsBlack;
    Widget      Blink, done, TalkBlack, TalkWhite, resume;
    Widget      adjourn, declineAdjourn, Observers, replay, Title;
    Widget      komi0, komi5, handicap;

    if (DebugFun) {
        printf("OpenObserve(%s(%d))\n",
               GameDescription(game), GameServerId(game));
        fflush(stdout);
    }

    sizex   = GameXSize(game);
    sizey   = GameYSize(game);
    Playing = PlayingP(game);
    if (Playing) Reviewing = 0;
    else Reviewing = ReviewP(game); 

    /* initialization */
    sprintf(HandicapString, "%d", GameHandicap(game));
    move = GameMove(game);
    sprintf(MoveText, "%3d/%-3d", move, move);
    if (Playing) {
	Root = MyVaCreateManagedWidget("play",
                                   toplevel,
                                   XtNboardSize, (XtArgVal) sizex,
                                   "captures",   (XtArgVal) GameCaptures(game),
                                   "move",       (XtArgVal) MoveText,
                                   "title",      (XtArgVal) "",
                                   NULL);
    } else {
	Root = MyVaCreateManagedWidget(Reviewing ? "review" : "observe",
                                   toplevel,
                                   XtNboardSize, (XtArgVal) sizex,
                                   "captures",   (XtArgVal) GameCaptures(game),
                                   "move",       (XtArgVal) MoveText,
                                   "komi",       (XtArgVal) GameKomi(game),
                                   "handicap",   (XtArgVal) HandicapString,
                                   "title",      (XtArgVal) "",
                                   NULL);
    }
    Observers       = XtNameToWidget(Root, "*observers");
    StatsBlack      = XtNameToWidget(Root, "*statsBlack");
    StatsWhite      = XtNameToWidget(Root, "*statsWhite");
    Blink           = XtNameToWidget(Root, "*blink");
    replay          = XtNameToWidget(Root, "*replay");
    TalkBlack       = XtNameToWidget(Root, "*talkBlack");
    TalkWhite       = XtNameToWidget(Root, "*talkWhite");

    PlayerBlack = GameBlack(game);
    PlayerWhite = GameWhite(game);
    if (PlayerBlack == PlayerWhite) {
        if (StatsBlack && StatsWhite) {
            XtDestroyWidget(StatsWhite);
            StatsWhite = 0;
        }
        if (TalkBlack && TalkWhite) {
            XtDestroyWidget(TalkWhite);
            TalkWhite = 0;
        }
    }

    if (Playing) {
        Title   = XtNameToWidget(Root, "*titlePlay");
        undo    = XtNameToWidget(Root, "*undo");
        Pass    = XtNameToWidget(Root, "*pass");
        done    = XtNameToWidget(Root, "*done");
        resume  = XtNameToWidget(Root, "*resume");
        adjourn = XtNameToWidget(Root, "*adjourn");
        declineAdjourn = XtNameToWidget(Root, "*declineAdjourn");
    } else Title = XtNameToWidget(Root, "*titleObserve");

    Scroll  = XtNameToWidget(Root, "*scroll");
    Input   = XtNameToWidget(Root, "*input");
    Chatter = XtNameToWidget(Root, "*chatter");
    Com     = XtNameToWidget(Root, "*board");
    Tim     = XtNameToWidget(Root, "*time");
    if (Tim) AddText(Tim, GetTime(game));

    observe = NULL; /* Dead code to please compilers */
    WITH_HANDLING {
        observe = mynew(Observe);
        observe->SgfFile = observe->PsFile = observe->KibitzFile = NULL;
        observe->Root            = Root;
        SetObserveDescription(observe, game);
        observe->Game            = game;
        observe->Node            = 0;
        observe->Redraw          = False;
        observe->Scoring         = False;
        observe->InfoWidget      = XtNameToWidget(Root, "*info");
        observe->Title           = Title;
        observe->ScrollWidget    = Scroll;
        observe->BoardWidget     = Com;
        observe->InputWidget     = Input;
        observe->ChatterWidget   = Chatter;
        observe->TimeWidget      = Tim;
        observe->MoveBeepWidget  = XtNameToWidget(Root, "*moveBeep");
        observe->MoveRaiseWidget = XtNameToWidget(Root, "*moveRaise");
        observe->KibitzBeep      = XtNameToWidget(Root, "*kibitzBeep");
        observe->KibitzRaise     = XtNameToWidget(Root, "*kibitzRaise");
        observe->ErrorBeep       = XtNameToWidget(Root, "*errorBeep");
        observe->ErrorRaise      = XtNameToWidget(Root, "*errorRaise");
        observe->KomiWidget      = XtNameToWidget(Root,
					          Playing ? "*kCom" : "*komi");
        observe->HandicapWidget  = XtNameToWidget(Root,
					      Playing ? "*hCom" : "*handicap");
        observe->CaptureWidget   = XtNameToWidget(Root, "*captures");
        observe->MoveWidget      = XtNameToWidget(Root, "*move");
        observe->SgfOverwrite    = XtNameToWidget(Root, "*sgfOverwrite");
        observe->PsOverwrite     = XtNameToWidget(Root, "*psOverwrite");
        observe->KibitzOverwrite = XtNameToWidget(Root, "*kibitzOverwrite");
        observe->SgfFromStart    = XtNameToWidget(Root, "*sgfFromStart");
        observe->PsFromStart     = XtNameToWidget(Root, "*psFromStart");
        observe->Score           = XtNameToWidget(Root, "*score");
        observe->Replay          = replay;
        observe->Next            = GameToObservers(game);
        GameToObservers(game)    = observe;

        XtAddCallback(Root,    XtNdestroyCallback, 
                      CallDestroyObserve, (XtPointer) observe);
 
        PassPat = XtNameToWidget(Root, "*passPattern");
        if (PassPat) XtAddCallback(PassPat, XtNcallback, ChangeKibitzPass,
                                   (XtPointer) observe);
        Kill = XtNameToWidget(Root, "*killPattern");
        if (Kill) XtAddCallback(Kill, XtNcallback, ChangeKibitzKill,
                                (XtPointer) observe);

        Refresh = XtNameToWidget(Root, "*refresh");
        if (Refresh) XtAddCallback(Refresh, XtNcallback, RefreshGame,
                                   (XtPointer) observe);
        ServerScore = XtNameToWidget(Root, "*serverScore");
        if (ServerScore) XtAddCallback(ServerScore, XtNcallback, ScoreGame, 
                                   (XtPointer) observe);
        Bets = XtNameToWidget(Root, "*bets");
        if (Bets)    XtAddCallback(Bets,    XtNcallback, GetBets, 
                                   (XtPointer) observe);
        analyze = XtNameToWidget(Root, "*analyze");
        if (analyze) XtAddCallback(analyze, XtNcallback, AnalyzeGame,
                                   (XtPointer) observe);
        Dup = XtNameToWidget(Root, "*dup");
        if (Dup)     XtAddCallback(Dup,     XtNcallback, DupGame, 
                                   (XtPointer) observe);
        if (Com) {
	    XtAddCallback(Com,     XtNbuttonDown, ButtonDown,
			  (XtPointer) observe);
	    XtAddCallback(Com,     XtNbuttonUp, ButtonUp,
			  (XtPointer) observe);
	}
        if (Scroll) {
            XtAddCallback(Scroll,  XtNjumpProc, JumpObserve,
                          (XtPointer) observe);
            XtAddCallback(Scroll,  XtNscrollProc, ScrollObserve,
                          (XtPointer) observe);
        }

        if (Playing) {
            if (undo) XtAddCallback(undo, XtNcallback, SendUndo,
                                    (XtPointer) observe);
            if (Pass) {
	        XtAddCallback(Pass, XtNcallback, SendPass,
			      (XtPointer) observe);
		if (TeachingP(game)) {
		  XtVaSetValues(Pass, XtNlabel, (XtArgVal) "Forw", NULL); 
		}
	    }
            if (done) {
                XtAddCallback(done, XtNcallback, SendDone,
                              (XtPointer) observe);
                XtUnmanageChild(done);
            }
            if (resume) XtAddCallback(resume, XtNcallback, CallResume,
                                      (XtPointer) game);
            if (adjourn) XtAddCallback(adjourn, XtNcallback, CallSendCommand,
                                       (XtPointer) "adjourn");
            if (declineAdjourn) {
                XtAddCallback(declineAdjourn, XtNcallback, CallSendCommand,
                              (XtPointer) "decline adjourn");
	    }
            if (observe->KomiWidget) {
		XtVaSetValues(observe->KomiWidget,
			      XtNlabel, GameKomi(game), NULL);
	    }
	    komi0 = XtNameToWidget(Root, "*k0");
	    if (komi0) {
                XtAddCallback(komi0, XtNcallback, CallSendCommand,
                              (XtPointer) "komi 0.5");
	    }
	    komi5 = XtNameToWidget(Root, "*k5");
	    if (komi5) {
                XtAddCallback(komi5, XtNcallback, CallSendCommand,
                              (XtPointer) "komi 5.5");
	    }
	    if (observe->HandicapWidget) {
		XtVaSetValues(observe->HandicapWidget,
			      XtNlabel, HandicapString, NULL);
	    }
	    for (hand = 2; hand <= 9; hand++) {
		char handName[20];
		static char handCommand[10][11];

		sprintf(handName, "*h%d", hand);
		sprintf(&handCommand[hand][0], "handicap %d", hand);
		handicap = XtNameToWidget(Root, handName);
		if (handicap) {
		    XtAddCallback(handicap, XtNcallback, CallSendCommand,
                              (XtPointer) &handCommand[hand][0]);
		}
	    }
            observe->Pass = Pass;
            observe->Done = done;
        }
        if (Observers) XtAddCallback(Observers, XtNcallback, CallObservers,
                                     (XtPointer) observe);

        SgfSave = XtNameToWidget(Root, "*sgfSave");
        if (SgfSave) XtAddCallback(SgfSave, XtNcallback, SaveGame,
                                   (XtPointer) observe);
        SgfFile = XtNameToWidget(Root, "*sgfFile");
        if (SgfFile) XtAddCallback(SgfFile, XtNcallback, ChangeSgfFilename,
                                   (XtPointer) &observe->SgfFile);

        PsSave = XtNameToWidget(Root, "*psSave");
        if (PsSave) XtAddCallback(PsSave, XtNcallback, SavePs,
                                  (XtPointer) observe);

        PsFile = XtNameToWidget(Root, "*psFile");
        if (PsFile) XtAddCallback(PsFile, XtNcallback, ChangePsFilename,
                                  (XtPointer) &observe->PsFile);

        KibitzSave = XtNameToWidget(Root, "*kibitzSave");
        if (KibitzSave) XtAddCallback(KibitzSave, XtNcallback, SaveKibitz,
                                      (XtPointer) observe);

        KibitzFile = XtNameToWidget(Root, "*kibitzFile");
        if (KibitzFile) XtAddCallback(KibitzFile, XtNcallback,
                                      ChangeKibitzFilename,
                                      (XtPointer) &observe->KibitzFile);

        if (TalkBlack) {
            XtAddCallback(TalkBlack, XtNcallback, CallGetTell,
                          (XtPointer) PlayerBlack);
            XtVaGetValues(TalkBlack, XtNlabel, (XtArgVal) &title, NULL);
            title = GameTemplateDescription(game, title);
            XtVaSetValues(TalkBlack, XtNlabel, (XtArgVal) title, NULL); 
            myfree(title);
        }

        if (TalkWhite) {
            XtAddCallback(TalkWhite, XtNcallback, CallGetTell,
                          (XtPointer) PlayerWhite);
            XtVaGetValues(TalkWhite, XtNlabel, (XtArgVal) &title, NULL);
            title = GameTemplateDescription(game, title);
            XtVaSetValues(TalkWhite, XtNlabel, (XtArgVal) title, NULL); 
            myfree(title);
        }

        if (StatsBlack) {
            XtAddCallback(StatsBlack, XtNcallback, CallGetStats,
                          (XtPointer) PlayerBlack);
            XtVaGetValues(StatsBlack, XtNlabel, (XtArgVal) &title, NULL);
            title = GameTemplateDescription(game, title);
            XtVaSetValues(StatsBlack, XtNlabel, (XtArgVal) title, NULL); 
            myfree(title);
        }

        if (StatsWhite) {
            XtAddCallback(StatsWhite, XtNcallback, CallGetStats,
                          (XtPointer) PlayerWhite);
            XtVaGetValues(StatsWhite, XtNlabel, (XtArgVal) &title, NULL);
            title = GameTemplateDescription(game, title);
            XtVaSetValues(StatsWhite, XtNlabel, (XtArgVal) title, NULL); 
            myfree(title);
        }

        if (Blink && Com) {
            XtAddCallback(Blink, XtNcallback, ToggleBlink, (XtPointer) Com);
            XtVaGetValues(Blink, XtNstate, (XtArgVal) &state, NULL);
            if (state == False)
                XtVaSetValues(Com, XtNoffTimeout, (XtArgVal) 0, NULL);
        }
        if (replay) {
            XtAddCallback(replay, XtNcallback, ToggleReplay,
                          (XtPointer) observe);
            XtVaGetValues(replay, XtNstate, (XtArgVal) &state, NULL);
            ToggleReplay(0, (XtPointer) observe,
                         INT_TO_XTPOINTER((int) state));
        }
        if (observe->Score) {
            XtVaGetValues(observe->Score, XtNstate,
			  (XtArgVal) &observe->Scoring, NULL);
            XtAddCallback(observe->Score, XtNcallback, ToggleScoring,
                          (XtPointer) observe);
        }
        if (Title) {
            if (TeachingP(game) || Reviewing || RequestP(game)) {
                gameTitle = GameTitle(game);
                if (gameTitle) AddText(Title, gameTitle);
            } else {
                XtDestroyWidget(Title);
                observe->Title = 0;
            }
	}
        if (Tim) {
	    XtVaGetValues(Tim,
			  XtNbackground, (XtArgVal)&defaultTimeBackground,
			  XtNforeground, (XtArgVal)&defaultTimeForeground,
                          NULL);
	}
	if (observe->InputWidget) {
	    XtVaGetValues(observe->InfoWidget,
			  XtNbackground, (XtArgVal)&defaultObserveBackground,
			  XtNforeground, (XtArgVal)&defaultObserveForeground,
			  NULL);
	}
        Collect = XtNameToWidget(Root, "*collect");
        XtCallActionProc(Root, (String) "nexttext", NULL, NULL, 0); 
        MyRealizeWidget(Root);
    } ON_EXCEPTION {
        XtDestroyWidget(Root);
    } END_HANDLING;
    return observe;
}

void PassToDone(const Game *game, int Pass, int done)
{
    Observe *observe;
    Widget   w;

    if (DebugFun) {
        if (game) printf("PassToDone(%s, %d, %d)\n",
                         GameDescription(game), Pass, done);
        else      printf("PassToDone(NULL, %d, %d)\n", Pass, done);
        fflush(stdout);
    }

    if (game) {
        for (observe = GameToObservers(game); observe;
	     observe = observe->Next) {
            if (!observe->Pass || !observe->Done) {
                Warning("Called PassToDone on an incomplete observe\n");
            } else {
		VaSetManagementChildren(observe->Pass, Pass, observe->Done,
					done, (Widget) 0);
	    }
	    w = observe->Score;
	    if (w) {
		/* Set the board in scoring mode or restore it to normal: */
		observe->Scoring = done & appdata.AutoScore;
		XtVaSetValues(w, XtNstate, (XtArgVal)observe->Scoring, NULL);

		observe->Redraw = True; /* force updating the marks */
		GotoObserve(observe, observe->Node);
	    }
	}
    } else Warning("Called PassToDone(NULL)\n");
    /* Should be impossible, but who knows */
}

static void ButtonDown(Widget w, XtPointer clientdata, XtPointer calldata)
{
    Observe  *observe = (Observe*)clientdata;
    GoButton *Button = (GoButton*)calldata;

    observe->lastX = Button->x;
    observe->lastY = Button->y;
    observe->lastTime = Button->time;
}

static void ButtonUp(Widget w, XtPointer clientdata, XtPointer calldata)
{
    Observe  *observe;
    Game     *game;
    Gamelog  *log;
    BWPiece   Color;
    GoButton *Button;
    char      Move[4];
    int       i, x, y;
    size_t    node, Nodes, Pos;

    Button = (GoButton *) calldata;
    x      = Button->x;
    y      = Button->y;
    observe= (Observe  *) clientdata;
    game   = observe->Game;
    log    = GameGamelog(game);
    if (!log) Raise1(AssertException, "Move in game without gamelog");
    Nodes  = NumberNodes(log)-1;

    if (MyGameP(game)) {
        GotoObserve(observe, Nodes);
	if (x == observe->lastX && y == observe->lastY &&
	    Button->time - observe->lastTime >= appdata.MinButtonTime &&
            0 <= x && x < log->SizeX && 0 <= y && y < log->SizeY) {
            GoMoveFromXY(Move, x, y);
            LastMove(w, x, y);
#ifndef NO_CLIENT_TIME
	    if (GameMoveTime(game) > 0) { /* thinking time known */
	        char timedMove[40];
		time_t delta = time(NULL) - GameMoveTime(game);
		sprintf(timedMove,"%s %ld", Move, delta);
		SendMove(w, clientdata, timedMove);
	    } else
#endif
            SendMove(w, clientdata, Move);
        } else {
            IfBell(observe->ErrorBeep);
            IfRaise(observe->ErrorRaise, observe->ErrorRaise);
        }
    } else if (0 <= x && x < log->SizeX && 0 <= y && y < log->SizeY)
        if (atoi(Button->str[0]) == 2) { /* Right mouse button -> bet */
            GotoObserve(observe, Nodes);
            GoMoveFromXY(Move, x, y);
            SendCommand(NULL, NULL, "%%bet bet %s", Move);
            if (observe->InfoWidget)
                BatchAddText(observe->InfoWidget, ".....            "
                             "I expect %s\n", Move); 
        } else {
            node = NodeNumber(log);
            Pos  = observe->Node;

            if      (Pos < 1)      Pos = 1;
            else if (Pos >= Nodes) Pos = Nodes-1;
        
            if      (Pos > node) for (i=node; i<Pos; i++) DownGamelog(log);
            else if (Pos < node) for (i=node; i>Pos; i--) UpGamelog(log);
        
            Color = (BWPiece)GetStone(log, x, y);
            while (Pos > 0) {
                UpGamelog(log);
                Pos--;
                if (Color != GetStone(log, x, y)) {
                    DownGamelog(log);
                    Pos++;
                    goto found;
                }
            }
            while (Pos < Nodes) {
                DownGamelog(log);
                Pos++;
                if (Color != GetStone(log, x, y)) goto found;
            }
          found:
            GotoObserve(observe, Pos);
        }
    else GotoObserve(observe, Nodes);
}

extern Observe *_ObserveFindWidget(Observe *observers, Widget w, int offset);
Observe *_ObserveFindWidget(Observe *observers, Widget w, int offset)
{
    Observe *observe;

    for (observe = observers; observe; observe = observe->Next)
        if (w == *(Widget *)(offset + (char *) observe)) return observe;
    return NULL;
}


/*
Observe *ObserveBoardWidget(Observe *observers, Widget w)
{
    Observe *observe;

    for (observe = observers; observe; observe = observe->Next)
        if (w == observe->BoardWidget) return observe;
    return NULL;
}
*/

Observe *_FindBoardWidget(Widget w)
{
    return _FindObserveWidget(w, offsetof(Observe, BoardWidget));
}

static void RealSendChatter(Game *game, int Id,
                            const char *Text, size_t length)
{
    UserSendCommand(NULL, NULL, "chatter %d %.*s", Id, (int) length, Text);
    Kibitz(game, Me, Text, length);
}

static void SendChatter(Widget w, XEvent *event, String *string, Cardinal *n)
{
    Observe    *observe;
    Game       *game;
    int         Id;
    String      Buffer;
    const char *From, *To, *End;

    observe = _FindObserveWidget(w, offsetof(Observe, ChatterWidget));
    if (observe) {
        game = observe->Game;
        XtVaGetValues(w, XtNstring, &Buffer, NULL);
        Id = GameServerId(game);
        if (Id >= 0) {
            End = strchr(Buffer, 0);
            for (From = Buffer;
                 (To = memchr((char *)From, '\n', (size_t)(End-From))) != NULL;
                 From = To+1)
                RealSendChatter(game, Id, From, (size_t) (To-From));
            RealSendChatter(game, Id, From, (size_t) (End-From));
        } else if (observe->InfoWidget)
            BatchAddText(observe->InfoWidget, ".....            "
                         "Chatter not sent, game is gone\n"); 
        else Output("Chatter not sent, game is gone\n");
        XtVaSetValues(w, XtNstring, "", NULL);
    } else WidgetWarning(w, "chatter() called on invalid widget");
}

static void RealSendKibitz(Game *game, int Id, const char *Text, size_t length)
{
    UserSendCommand(NULL, NULL, "kibitz %d %.*s", Id, (int) length, Text);
    Kibitz(game, Me, Text, length);
}

static void SendKibitz(Widget w, XEvent *event, String *string, Cardinal *n)
{
    Observe    *observe;
    Game       *game;
    int         Id;
    String      Buffer;
    const char *From, *To, *End;

    observe = _FindObserveWidget(w, offsetof(Observe, InputWidget));
    if (observe) {
        game = observe->Game;
        XtVaGetValues(w, XtNstring, &Buffer, NULL);
        Id = GameServerId(game);
        if (Id >= 0) {
            End = strchr(Buffer, 0);
            for (From = Buffer;
                 (To = memchr((char *)From, '\n', (size_t)(End-From))) != NULL;
                 From = To+1)
                RealSendKibitz(game, Id, From, (size_t) (To-From));
            RealSendKibitz(game, Id, From, (size_t) (End-From));
        } else if (observe->InfoWidget)
            BatchAddText(observe->InfoWidget, ".....            "
                         "Kibitz not sent, game is gone\n"); 
        else Output("Kibitz not sent, game is gone\n");
        XtVaSetValues(w, XtNstring, "", NULL);
    } else WidgetWarning(w, "kibitz() called on invalid widget");
}

void Kibitz(const Game *game, const Player *player,
            const char *kibitz, size_t length)
{
    const char *Desc;
    Observe    *observe;
    int         Failed, Yes, No;
    char        Buffer[500];
    const char *Error;

    Desc = PlayerString(player);
    if (appdata.NumberKibitzes) {
	sprintf(Buffer, "%16s:%3d: %.*s", Desc, GameMove(game), (int)length,
		kibitz);
    } else {
	sprintf(Buffer, "%16s: %.*s", Desc, (int) length, kibitz);
    }
    Error = TestRegexYesNo(Buffer, &KibitzYesNo, &Yes, &No);

    Failed = 0;
    if (Error) {
        sprintf(Buffer, ".....           : %s", Error);
        for (observe=GameToObservers(game); observe; observe = observe->Next) {
            if (observe->InfoWidget)
                BatchAddText(observe->InfoWidget, "%s\n", Buffer);
            else Failed = 1;
            IfBell(observe->ErrorBeep);
            IfRaise(observe->ErrorRaise, observe->ErrorRaise);
        }
    } else if (Yes && !No) {
        AddGameComment(game, "%s", Buffer);
        for (observe=GameToObservers(game); observe; observe = observe->Next) {
            if (observe->InfoWidget)
                BatchAddText(observe->InfoWidget, "%s\n", Buffer);
            else Failed = 1;
            IfBell(observe->KibitzBeep);
            IfRaise(observe->KibitzRaise, observe->KibitzRaise);
        }
    }
    if (Failed) Outputf("%s\n", Buffer);
}

void ObserveMessage(const Observe *observers)
{
    const Observe *observe;

    for (observe = observers; observe; observe = observe->Next) {
        IfBell(observe->KibitzBeep);
        IfRaise(observe->KibitzRaise, observe->KibitzRaise);
    }
}

void ObserveError(const Observe *observers)
{
    const Observe *observe;

    for (observe = observers; observe; observe = observe->Next) {
        IfBell(observe->ErrorBeep);
        IfRaise(observe->ErrorRaise, observe->ErrorRaise);
    }
}

void SetCaptures(const Game *game)
{
    XtArgVal Text;
    Observe *observe;

    Text = (XtArgVal) GameCaptures(game);
    for (observe = GameToObservers(game); observe; observe = observe->Next)
        if (observe->CaptureWidget)
            XtVaSetValues(observe->CaptureWidget, XtNlabel, Text, NULL);
}

void SetKomi(const Observe *observers, const char *Text)
{
    const Observe *observe;

    for (observe = observers; observe; observe = observe->Next)
        if (observe->KomiWidget)
            XtVaSetValues(observe->KomiWidget, XtNlabel, Text, NULL);
}

void SetTitle(const Observe *observers, const char *Text)
{
    const Observe *observe;

    for (observe = observers; observe; observe = observe->Next)
        if (observe->Title) {
            XtVaSetValues(observe->Title, XtNstring, (XtArgVal) "", NULL);
            if (Text) AddText(observe->Title, Text);
        }
}

static void ChangeTitle(Widget w, XEvent *event, String *string, Cardinal *n)
{
    Observe *observe;
    Game       *game;
    int         Id;
    String      Buffer;
    const char *gameTitle;

    observe = _FindObserveWidget(w, offsetof(Observe, Title));
    if (observe) {
        game = observe->Game;
        gameTitle = GameTitle(game);
        if (gameTitle == NULL) gameTitle = "";
        XtVaGetValues(w, XtNstring, &Buffer, NULL);
        Id = GameServerId(game);
        if (strcmp(gameTitle, Buffer)) {
            if (Id >= 0)
                if (Buffer[0]) UserSendCommand(NULL, NULL, "title %s", Buffer);
                else if (observe->InfoWidget)
                    BatchAddText(observe->InfoWidget, "Cannot unset title\n");
                else Outputf("Cannot unset title\n");
            else if (observe->InfoWidget)
                BatchAddText(observe->InfoWidget, ".....            "
                             "Change title not sent, game is gone\n"); 
            else Output("Change title not sent, game is gone\n");
            XtVaSetValues(w, XtNstring, "", NULL);
            if (gameTitle[0]) AddText(w, gameTitle);
        }
    } else WidgetWarning(w, "changetitle() called on invalid widget");
}

void SetHandicap(const Observe *observers, int handicap)
{
    const Observe *observe;
    char           Handicap[10];

    sprintf(Handicap, "%d", handicap);
    for (observe = observers; observe; observe = observe->Next)
        if (observe->HandicapWidget)
            XtVaSetValues(observe->HandicapWidget, XtNlabel, Handicap, NULL);
}

void SetObserveTime(const Observe *observers, const char *Text, int low)
/* Low has bit 0 set if my time is low, bit 1 set if my opponent
 * time is low, bit 2 set if one time in an observed game is low.
 */
{
    const Observe  *observe;
    Widget          Source;
    XawTextBlock    block;
    XawTextEditType type;
    Pixel background = defaultTimeBackground;
    Pixel foreground = defaultTimeForeground;
    Pixel observeBackground = defaultObserveBackground;
    Pixel observeForeground = defaultObserveForeground;

    block.firstPos = 0;
    block.length   = strlen(Text);
    block.ptr      = (char *) Text;
    block.format   = FMT8BIT;
    for (observe = observers; observe; observe = observe->Next) {
        if (observe->TimeWidget) {
            XtVaGetValues(observe->TimeWidget,
                          XtNeditType,       (XtArgVal) &type,
                          XtNtextSource,     (XtArgVal) &Source,
                          NULL);
            XtVaSetValues(observe->TimeWidget,
                          XtNeditType, (XtArgVal) XawtextEdit, NULL);
            XawTextReplace(observe->TimeWidget, 0,
                           XawTextSourceScan(Source, 0, XawstAll,
                                             XawsdRight, 1, True),
                           &block);
	    if (low & 1) {
		observeBackground = background = appdata.MyLowTimeBackground;
		observeForeground = foreground = appdata.MyLowTimeForeground;
	    } else if (low) {
		background = appdata.LowTimeBackground;
		foreground = appdata.LowTimeForeground;
	    }
	    XtVaSetValues
		(observe->TimeWidget,
		 XtNeditType,   (XtArgVal) type,
		 XtNbackground, (XtArgVal)background,
		 XtNforeground, (XtArgVal)foreground,
		 NULL);
        }
	if (observe->InputWidget) {
	    XtVaSetValues
		(observe->InputWidget,
		 XtNbackground, (XtArgVal)observeBackground,
		 XtNforeground, (XtArgVal)observeForeground,
		 NULL);
	}
    }
}

static int realGameMessage(const Game *game, const char *Text)
{
    Observe *observe;
    int      Fail, Mine;
    Gamelog  *log;
    int       i, oldnode, node;

    log = GameGamelog(game);
    if (log) oldnode = NodeNumber(log);

    AddGameComment(game, "%s", Text);
    Fail = 0;
    Mine = MyGameP(game);
    if (Mine && !GameToObservers(game)) {
	ShowObserve(game);
    }
    for (observe = GameToObservers(game); observe; observe = observe->Next) {
        if (observe->InfoWidget)
            BatchAddText(observe->InfoWidget, "%s\n", Text);
        else Fail = 1;
        if (Mine) GotoObserve(observe, observe->Node);
    }

    /* Restore game log at same position. Required at least for passes
     * which add a game message but the game log is still be constructed
     * in Moves().
     */
    if (log) {
	node = NodeNumber(log);
	for (i = node; i < oldnode; i++) DownGamelog(log);
	for (i = node; i > oldnode; i--) UpGamelog(log);
    }
    if (Fail) Outputf("%s\n", Text);
    return Fail;
}

#ifndef   HAVE_NO_STDARG_H
int GameMessage(const Game *game, const char *Prefix, 
                const char *Format, ...)
#else  /* HAVE_NO_STDARG_H */
int GameMessage(game, Prefix, va_alist)
const Game *game;
const char *Prefix;
va_dcl
#endif /* HAVE_NO_STDARG_H */
{
    char     Text[2048];
    Observe *observe;
    int      Fail, Mine;
    va_list  args;

#ifndef   HAVE_NO_STDARG_H
    va_start(args, Format);
#else  /* HAVE_NO_STDARG_H */
    const char *Format;

    va_start(args);
    Format    = va_arg(args, const char *);
#endif /* HAVE_NO_STDARG_H */
    sprintf(Text, "%-17s", Prefix);
    vsprintf(strchr(Text, 0), Format, args);
    va_end(args);

    return realGameMessage(game, Text);
}

#ifndef   HAVE_NO_STDARG_H
int NbGameMessage(const Game *game, const char *Prefix, 
                  const char *Format, ...)
#else  /* HAVE_NO_STDARG_H */
int NbGameMessage(game, Prefix, va_alist)

     const Game *game;
     const char *Prefix;
     va_dcl
#endif /* HAVE_NO_STDARG_H */
     /* Same as GameMessage but adds move number if *numberKibitzes is true */
{
    char     Text[2048];
    va_list  args;

#ifndef   HAVE_NO_STDARG_H
    va_start(args, Format);
#else  /* HAVE_NO_STDARG_H */
    const char *Format;

    va_start(args);
    Format    = va_arg(args, const char *);
#endif /* HAVE_NO_STDARG_H */
    if (appdata.NumberKibitzes) {
	sprintf(Text, "%-16s:%3d: ", Prefix, GameMove(game));
    } else {
	sprintf(Text, "%-17s", Prefix);
    }
    vsprintf(strchr(Text, 0), Format, args);
    va_end(args);
    return realGameMessage(game, Text);
}

static void CallDestroyObserve(Widget w,
                               XtPointer clientdata, XtPointer calldata)
{
    Observe *observe, *before;
    Game    *game;

    observe = (Observe *) clientdata;
    game = observe->Game;

    if (MyGameP(game) && BusyP(game)) {
	Output("You are still playing. Type 'refresh' to restore the board\n");
    }

    /* game == NULL means we are completely destroying the game and have
       been forcibly decoupled from the game data structure. Just clean up
       the pending data structure when Xt feels up to it */
    if (game) {
        /* The next line is non portable but should work 
           "almost" everywhere :-) */
        before = (Observe *)((char *) &GameToObservers(game)-
                             offsetof(Observe, Next));
        if (before->Next == observe && !observe->Next) StopObserve(game);
        else while (before->Next != observe) before = before->Next;
        before->Next = observe->Next;
    }

    myfree(observe->KibitzFile);
    myfree(observe->SgfFile);
    myfree(observe->PsFile);
    myfree(observe);
    
    if (game) TestDeleteGame(game);
}

void ShowPosition(const Game *game)
{
    Observe *observe;
    Gamelog *log;
    Boolean  Beep;
    int      me, Review;
    size_t   nodes;
    float    Step;

    log    = GameGamelog(game);
    nodes  = NumberNodes(log)-1;
    Step   = 1.0 / nodes;
    me     = MyGameP(game);
    Review = ReviewP(game);
    for (observe = GameToObservers(game); observe; observe = observe->Next)
        if (!Review && (observe->Node+1 == nodes || me)) {
            GotoObserve(observe, nodes);
            if (observe->MoveBeepWidget) {
                XtVaGetValues(observe->MoveBeepWidget,
                              XtNstate, (XtArgVal) &Beep, NULL);
                if (Beep != False) StoneSound(observe->BoardWidget);
            }
            IfRaise(observe->MoveRaiseWidget, observe->MoveRaiseWidget);
        } else GotoObserve(observe, observe->Node);
}

void InitObserve(Widget Toplevel)
{
    const char *Error;

    EmptyRegexYesNo(&KibitzYesNo);
    Error = CompileRegexYesNo(appdata.KibitzPass, appdata.KibitzKill,
                              &KibitzYesNo);
    if (Error) Warning("failed to compile kibitz regex pattern: %s\n", Error);

    XtAppAddActions(XtWidgetToApplicationContext(Toplevel),
                    actionTable, XtNumber(actionTable));
}

void CleanObserve(void)
{
    FreeRegexYesNo(&KibitzYesNo);
}

/* Notice: will not destroy game if interest drops too low ! */
void DestroyObservers(Observe **observers)
{
    Observe *next, *observe;

    for (observe = *observers; observe; observe = next) {
        next = observe->Next;
        observe->Game = NULL; /* Stops observe destroy from shooting the
                                 game from under us */
        XtDestroyWidget(observe->Root);
    }
    *observers = NULL;
}

void ReplayTime(unsigned long diff, Observe *observers, size_t nodes)
{
    Observe *observe;
    Game    *game;

    for (observe = observers; observe; observe = observe->Next) {
        if (observe->ReplayRate) {
            observe->ReplayCount += diff;
            if (observe->ReplayCount >= observe->ReplayRate) {
                observe->ReplayCount = 0;
		game    = observe->Game;
                if (observe->Node < nodes) {
		    GotoObserve(observe, observe->Node+1);
		} else if (MyGameP(game) && TeachingP(game) &&
			   GameTitle(game) != NULL) {
		    /* automatic forward */
		    MoveCommand(NULL, "forw");
                } else {
		    /* stop replay */
                    observe->ReplayRate = 0;
                    if (observe->Replay)
                        XtVaSetValues(observe->Replay, XtNstate,
                                      (XtArgVal) False, NULL);
		}
            }
        }
    }
}

void StopForward(Observe *observers, size_t nodes)
{
    Observe *observe;
    Game    *game;

    for (observe = observers; observe; observe = observe->Next) {
        if (observe->ReplayRate && observe->Node >= nodes) {
	    observe->ReplayRate = 0;
	    if (observe->Replay)
		XtVaSetValues(observe->Replay, XtNstate,
			      (XtArgVal) False, NULL);
	}
    }
}
