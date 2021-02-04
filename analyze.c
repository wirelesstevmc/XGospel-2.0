#include <X11/StringDefs.h>
#include <X11/Intrinsic.h>
#include <X11/Shell.h>
#include <X11/Xaw/Scrollbar.h>
#include <X11/Xaw/Toggle.h>

#include <stdlib.h>

#include <except.h>
#include <mymalloc.h>
#include <myxlib.h>

#include "analyze.h"
#include "games.h"
#include "gospel.h"
#include "utils.h"
#include "xgospel.h"

struct _Analyze {
    struct _Analyze *Next, *Previous;
    char           *Name;
    int             ReplayRate, ReplayCount;
    char           *SgfFile, *PsFile;
    Gamelog        *Log;
    int             Level, BaseMove;
    size_t          Size;
    Widget          Model, TopWidget, BoardWidget, ScrollWidget, SgfOverwrite;
    Widget          SgfFromStart, PsOverwrite, PsFromStart, Score;
    Widget          MoveBeep, MoveRaise, ErrorBeep, ErrorRaise, Copy, Replay;
    Boolean         scoring;
    float           WhiteAdvantage;
};

static Analyze *FindAnalyzeWidget(Widget w);
static     void CallScore(Widget w, XtPointer clientdata, XtPointer calldata);
static    char *BoardScore(Gamelog *Log, float WhiteAdvantage,
			   Widget BoardWidget);

static Analyze AnalyzeBase = { &AnalyzeBase, &AnalyzeBase };

static void CallDestroyAnalyze(Widget w,
                               XtPointer clientdata, XtPointer calldata)
{
    Analyze *Ana;

    Ana = (Analyze *) clientdata;

    if (Ana->Model) {
        XtRemoveCallback(Ana->Model, XtNdestroyCallback,
                         CallDestroyWidgetReference, (XtPointer) &Ana->Model);
        XtRemoveCallback(Ana->Model, XtNdestroyCallback, CallDestroy,
                         (XtPointer) Ana->Copy);
    }
    Ana->Previous->Next = Ana->Next;
    Ana->Next->Previous = Ana->Previous;
    FreeGamelog(Ana->Log);
    myfree(Ana->SgfFile);
    myfree(Ana->PsFile);
    myfree(Ana->Name);
    myfree(Ana);
}

static void GotoAnalyze(Analyze *analyze, int Pos)
{
    Gamelog   *log;
    int        i, node, nodes;
    float      Step;
    Widget     w;

    log = analyze->Log;
    nodes  = NumberNodes(log)-1;
    node   = NodeNumber(log);

    if      (Pos < 1)     Pos = 1;
    else if (Pos > nodes) Pos = nodes;

    if      (Pos > node) for (i=node; i<Pos; i++) DownGamelog(log);
    else if (Pos < node) for (i=node; i>Pos; i--) UpGamelog(log);

    w = analyze->BoardWidget;
    if (w) {
        int x, y;

	DisplayScore(analyze->scoring, log, analyze->WhiteAdvantage,
		     w, analyze->TopWidget);

        i = XTPOINTER_TO_INT(FindComment(log, &LastMoveFun));
        if (i<0) x = y = -1;
        else {
            y = i / log->SizeX;
            x = i % log->SizeY;
        }
        LastMove(w, x, y); /* must be called after DisplayScore */
    }

    if (analyze->ScrollWidget) {
        Step = 1.0 / nodes;
        MyScrollbarSetThumb(analyze->ScrollWidget, (Pos-1)*Step, Step);
    }
}

static void ResetAnalyze(Widget w, XtPointer clientdata, XtPointer calldata)
{
    Analyze *Ana;
    Gamelog *log;
    int      i, node, nodes;

    Ana   = (Analyze *) clientdata;
    log   = Ana->Log;
    node  = NodeNumber(log)-1;
    nodes = NumberNodes(log)-2;

    for (i=node; i<nodes; i++) DownGamelog(log);
    for (i=nodes; i>0; i--) {
	int uncaptured = DeleteNode(log);
	if (Ana->WhiteAdvantage != UNKNOWN_ADVANTAGE) {
	    Ana->WhiteAdvantage -= uncaptured;
	}
    }
    if (Ana->scoring && Ana->Score) {
	CallScore(NULL, (XtPointer)Ana, NULL); /* toggle scoring */
    }
    GotoAnalyze(Ana, 1);
}

static void UndoAnalyze(Widget w, XtPointer clientdata, XtPointer calldata)
{
    Analyze   *Ana;
    Gamelog   *log;
    int        i, node, nodes;
    int uncaptured;

    Ana   = (Analyze *) clientdata;
    log   = Ana->Log;
    node  = NodeNumber(log)-1;
    nodes = NumberNodes(log)-2;

    if   (nodes == 0) {
        IfBell(Ana->ErrorBeep);
        IfRaise(Ana->ErrorRaise, Ana->ErrorRaise);
    } else {
        for (i=node; i<nodes; i++) DownGamelog(log);
	uncaptured = DeleteNode(log);
	if (Ana->WhiteAdvantage != UNKNOWN_ADVANTAGE) {
	    Ana->WhiteAdvantage -= uncaptured;
	}
        GotoAnalyze(Ana, nodes);
    }
}

static void ToggleReplay(Widget w, XtPointer clientdata, XtPointer calldata)
{
    Analyze *analyze;
    Gamelog *log;
    int      nodes;

    analyze = (Analyze *) clientdata;
    if (False == (Boolean) XTPOINTER_TO_INT(calldata)) analyze->ReplayRate = 0;
    else {
        analyze->ReplayRate  = appdata.ReplayTimeout;
        log   = analyze->Log;
        nodes = NumberNodes(log)-1;
        if (NodeNumber(log) == nodes) {
            GotoAnalyze(analyze, 1);
            analyze->ReplayCount = 0;
        } else analyze->ReplayCount = analyze->ReplayRate-1;
    }
}

static void AnalyzeAnalyze(Widget w, XtPointer clientdata, XtPointer calldata)
{
    Analyze *Ana;
    Gamelog *log;
    int      node;

    Ana = (Analyze *) clientdata;
    log = Ana->Log;
    node  = NodeNumber(log)-1;
    OpenAnalyze(Ana->BoardWidget, Ana->Name, Ana->Level+1, Ana->Size,
                Ana->BaseMove+node, log->AllowSuicide, Ana->WhiteAdvantage);
}

static Exception NotEmpty = { "Position is not empty" };

static void AnalyzeButton(Widget w, XtPointer clientdata, XtPointer calldata)
{
    Analyze   *Ana;
    Gamelog   *log;
    int        i, node, nodes, move;
    float      Step;
    GoButton  *Button;
    char     **Board;
    StoneList  Stone;
    Boolean    Beep;

    Button = (GoButton *) calldata;
    Ana = (Analyze *)     clientdata;
    log = Ana->Log;
    node  = NodeNumber(log)-1;
    nodes = NumberNodes(log)-2;
    for (i=node; i<nodes; i++) DownGamelog(log);

    w   = Ana->BoardWidget;
    if (Button->params != 1) {
	Warning("Invalid parameters to Button action\n");
	return;
    }
    Board = GamelogToBoard(log);
    if (Button->x >= 0 && Button->x < log->SizeX &&
	Button->y >= 0 && Button->y < log->SizeY) {

        WITH_HANDLING {
	    int buttonNb = atoi(Button->str[0]);
	    int captures = 0; /* number of captured black stones */
	    BWPiece oldColor = (BWPiece)Board[Button->y][Button->x];
            move = Ana->BaseMove+nodes;
            switch(buttonNb) {
              case 0:
		if (oldColor == Empty) {
		    /* add a stone */
		    captures = DoMove(log, Button->x, Button->y,
				      move & 1 ? White : Black);
		    LastMove(w, Button->x, Button->y);
		    i = Button->y * log->SizeX + Button->x;
		    AddComment(log, &LastMoveFun, INT_TO_XTPOINTER(i));

		    if (Ana->MoveBeep) {
			XtVaGetValues(Ana->MoveBeep,
				      XtNstate, (XtArgVal) &Beep, NULL);
			if (Beep != False) StoneSound(Ana->BoardWidget);
		    }
		} else if (!Ana->scoring) {
		    /* remove a single stone */
		    captures = (oldColor == Black ? 1 : -1);
		    Stone.x     = Button->x;
		    Stone.y     = Button->y;
		    Stone.Color = Empty;
		    Stone.Next  = NULL;
		    SetStones(log, &Stone);
		    LastMove(w, -1, -1);
		    AddComment(log, &LastMoveFun, INT_TO_XTPOINTER(-1));
		} else {
		    /* in scoring mode, remove a stone group */
		    captures = RemoveGroupFromStone(log, Button->x, Button->y);
		    LastMove(w, -1, -1);
		    AddComment(log, &LastMoveFun, INT_TO_XTPOINTER(-1));
		}
		if (Ana->WhiteAdvantage != UNKNOWN_ADVANTAGE) {
		    Ana->WhiteAdvantage += captures;
		}
                break;

              case 1: /* set black stone */
              case 2: /* set white stone */
		Stone.Color = (buttonNb == 1 ? Black : White);
		if (Stone.Color != oldColor) {
		    Stone.x     = Button->x;
		    Stone.y     = Button->y;
		    Stone.Next  = NULL;
		    SetStones(log, &Stone);
		    /* WhiteAdvantage unchanged */
		    LastMove(w, -1, -1);
		    AddComment(log, &LastMoveFun, INT_TO_XTPOINTER(-1));
		}
                break;

              default:
                Warning("Invalid parameter '%s' to Button action\n",
                        Button->str[0]);
                break;
	    }
            IfRaise(Ana->MoveRaise, Ana->MoveRaise);

        } ON_EXCEPTION {
            IfBell(Ana->ErrorBeep);
            IfRaise(Ana->ErrorRaise, Ana->ErrorRaise);
/*          Warning(ExceptionName()); */
            ClearException();
        } END_HANDLING;
    }
    GotoAnalyze(Ana, NumberNodes(log)-1); /* display new board and score */
}

static void ScrollAnalyze(Widget scrollbar, XtPointer client_data,
                          XtPointer pos)
{
    Analyze   *Ana;
    int        Pos, Pixels;
    Dimension  Length;

    Ana = (Analyze *) client_data;
    Pos = XTPOINTER_TO_INT(pos);

    if (scrollbar)
        XtVaGetValues(scrollbar, XtNlength, (XtArgVal) &Length, NULL);

    Pixels = (int) Length / (NumberNodes(Ana->Log)-1);
    if (Pixels < appdata.ScrollUnit) Pixels = appdata.ScrollUnit;
    if   (Pos < 0) Pos = Pos/Pixels-1;
    else           Pos = Pos/Pixels+1;
    GotoAnalyze(Ana, NodeNumber(Ana->Log)+Pos);
}

static void JumpAnalyze(Widget scrollbar, XtPointer client_data, XtPointer per)
{
    Analyze *analyze;
    int      Pos;

    analyze = (Analyze *) client_data;
    Pos     = * (float *) per * (NumberNodes(analyze->Log)-1);

    GotoAnalyze(analyze, Pos+1);
}

void AnalyzeGoto(Widget w, XEvent *event, String *string, Cardinal *n)
/* used to scroll the board by forward (string > 0) or backward (string < 0).
 */
{
    Analyze *analyze = FindAnalyzeWidget(w);

    if (analyze) {
	int move = NodeNumber(analyze->Log) + ResourceStringToInt(string[0]);
	GotoAnalyze(analyze, move);
    } else {
	printf("AnalyzeGoto: analyze widget not found\n");
	fflush(stdout);
    }
}

static void ToggleAllowSuicide(Widget w,
                               XtPointer clientdata, XtPointer calldata)
{
    Analyze *analyze;

    analyze = (Analyze *) clientdata;
    analyze->Log->AllowSuicide = False != (Boolean) XTPOINTER_TO_INT(calldata);
}

/* Toggle equal time blink */
void ToggleBlink(Widget w, XtPointer clientdata, XtPointer calldata)
{
    Widget board;
    int    timeout;

    board = (Widget) clientdata;
    if (False == (Boolean) XTPOINTER_TO_INT(calldata))
        XtVaSetValues(board, XtNoffTimeout, (XtArgVal) 0, NULL);
    else {
        XtVaGetValues(board, XtNonTimeout,  (XtArgVal) &timeout, NULL);
        XtVaSetValues(board, XtNoffTimeout, (XtArgVal)  timeout, NULL);
    }
}

extern Observe *_FindBoardWidget(Widget w);
static void CallCopy(Widget w, XtPointer clientdata, XtPointer calldata)
{
    Widget   board;
    Analyze *Ana, *Here;
    Gamelog *Log;
    char   **Board;
    int      move, AllowSuicide;
    size_t   Size;
    Observe *observe;

    ResetAnalyze(w, clientdata, calldata);

    Ana = (Analyze *) clientdata;
    board = Ana->BoardWidget;
    if (board) {
        AllowSuicide = Ana->Log->AllowSuicide;
        FreeGamelog(Ana->Log);
        Size = Ana->Size;
        Ana->Log  = Log = AllocGamelog(Size, Size, AllowSuicide);
        AddComment(Log, &LastMoveFun, INT_TO_XTPOINTER(Size-1));
        w = Ana->Model;
        if (w) {
            observe = _FindBoardWidget(w);
            if (observe) {
		move = ObserveMove(observe);
		Ana->WhiteAdvantage = GetWhiteAdvantage(observe);
	    } else {
                for (Here = AnalyzeBase.Next;
                     Here != &AnalyzeBase;
                     Here = Here->Next)
                    if (Here->BoardWidget == w) {
                        move = Here->BaseMove + NumberNodes(Here->Log)-2;
			Ana->WhiteAdvantage = Here->WhiteAdvantage;
                        goto found;
                    }
                Raise1(AssertException, "Could not find widget to copy from");
            }
          found:
	    if (Ana->WhiteAdvantage == UNKNOWN_ADVANTAGE) {
		XtUnmanageChild(Ana->Score);
		Ana->scoring = False;
	    } else {
		XtManageChild(Ana->Score);
	    }
            Board = AllocBoard(Size, Size);
            WITH_UNWIND {
                GetBoard(w, Board);
                PositionToNode(Log, Board, Empty);
            } ON_UNWIND {
                FreeBoard(Board, Size);
            } END_UNWIND;
        } else {
            SetStones(Log, NULL);
            move = 0;
        }
        AddComment(Log, &LastMoveFun, INT_TO_XTPOINTER(-Size-1));
	ResetAnalyze(NULL, (XtPointer)Ana, NULL);
        Ana->BaseMove = move;
    }
}

static void CallScore(Widget w, XtPointer clientdata, XtPointer calldata)
/* IN assertion: Ana->Score != NULL (the Score/Done button exists) */
{
    Analyze *Ana = (Analyze *) clientdata;

    Ana->scoring = !Ana->scoring;
    if (Ana->scoring) {
	XtVaSetValues(Ana->Score, XtNlabel, (XtArgVal) "Done", NULL);
    } else {
	XtVaSetValues(Ana->Score, XtNlabel, (XtArgVal) "Score", NULL); 
    }
    DisplayScore(Ana->scoring, Ana->Log, Ana->WhiteAdvantage,
		 Ana->BoardWidget, Ana->TopWidget);
}

/* Analyze should have pos in structure (share with observe ?) -Ton */
static void SaveGame(Widget w, XtPointer clientdata, XtPointer calldata)
{
    Analyze *Ana;
    Gamelog *log;
    Boolean  Overwrite, FromStart;
    int      i, Now, node;

    Ana   = (Analyze *) clientdata;
    log   = Ana->Log;
    Now   = NodeNumber(log)-1;

    w = Ana->SgfFromStart;
    if (w) XtVaGetValues(w, XtNstate, (XtArgVal) &FromStart, NULL);
    else   FromStart = True;

    if (FromStart != False) {
        node = 0;
        if (Now > node) for (i = Now; i > node; i--) UpGamelog(log);
    } else node = Now;

    WITH_UNWIND {
        w = Ana->SgfOverwrite;
        if (w) XtVaGetValues(w, XtNstate, (XtArgVal) &Overwrite, NULL);
        else   Overwrite = False;

        SaveWrite(Ana->SgfFile, Overwrite, Ana->ErrorBeep, Ana->ErrorRaise,
                  "Sgf save error", WriteSgfFun, (XtPointer) log);
    } ON_UNWIND {
        if (Now > node) for (i = node; i < Now; i++) DownGamelog(log);
    } END_UNWIND;
}

static void SavePs(Widget w, XtPointer clientdata, XtPointer calldata)
{
    Analyze *Ana;
    Gamelog *log;
    Boolean  Overwrite, FromStart;
    int      i, Now, node;

    Ana   = (Analyze *) clientdata;
    log   = Ana->Log;
    Now   = NodeNumber(log)-1;

    w = Ana->PsFromStart;
    if (w) XtVaGetValues(w, XtNstate, (XtArgVal) &FromStart, NULL);
    else   FromStart = True;

    if (FromStart != False) {
        node = 0;
        if (Now > node) for (i = Now; i > node; i--) UpGamelog(log);
    } else node = Now;

    WITH_UNWIND {
        w = Ana->PsOverwrite;
        if (w) XtVaGetValues(w, XtNstate, (XtArgVal) &Overwrite, NULL);
        else   Overwrite = False;

        SaveWrite(Ana->PsFile, Overwrite, Ana->ErrorBeep, Ana->ErrorRaise,
		  "Postscript save error", WritePsFun, (XtPointer) log);
    } ON_UNWIND {
        if (Now > node) for (i = node; i < Now; i++) DownGamelog(log);
    } END_UNWIND;
}

static Analyze *RealOpenAnalyze(Analyze *Ana)
{
    const char *Name;
    char     title[80], icon[80];
    int      Level, Size;
    Boolean  state;
    Widget   Root, Reset, Analyse, board, undo, Scroll, SgfSave, PsSave;
    Widget   SgfFile, PsFile, AllowSuicide, Blink, Copy, Replay, Score;

    Level = Ana->Level;
    Name  = Ana->Name;
    Size  = Ana->Size;

    if (Level > 0) {
        sprintf(title, "%d", Ana->Level+1);
        Ana->SgfFile   = StringToFilename(appdata.AnalyzeFilename,
                                          (int) 'T', SGFEXTENSION,
                                          (int) 't', ".",
                                          (int) 'N', "Analyzing",
                                          (int) 'n', "_",
                                          (int) 'B', "", (int) 'b', "",
                                          (int) 'W', "", (int) 'w', "",
                                          (int) 'U', "", (int) 'V', "",
                                          (int) 'L', title, (int) 'l', "_", 0);
        Ana->PsFile    = StringToFilename(appdata.AnalyzeFilename,
                                          (int) 'T', PSEXTENSION,
                                          (int) 't', ".",
                                          (int) 'N', "Analyzing",
                                          (int) 'n', "_",
                                          (int) 'B', "", (int) 'b', "",
                                          (int) 'W', "", (int) 'w', "",
                                          (int) 'U', "", (int) 'V', "",
                                          (int) 'L', title, (int) 'l', "_", 0);
        sprintf(title, "Analyzing**%d %s", Ana->Level+1, Name);
        sprintf(icon,  "Analyzing**%d %s", Ana->Level+1, Name);
    } else {
        Ana->SgfFile   = StringToFilename(appdata.AnalyzeFilename,
                                          (int) 'T', SGFEXTENSION,
                                          (int) 't', ".",
                                          (int) 'N', "Analyzing",
                                          (int) 'n', "_",
                                          (int) 'B', "", (int) 'b', "",
                                          (int) 'W', "", (int) 'w', "",
                                          (int) 'U', "",
                                          (int) 'V', "",
                                          (int) 'L', "", (int) 'l', "", 0);
        Ana->PsFile    = StringToFilename(appdata.AnalyzeFilename,
                                          (int) 'T', PSEXTENSION,
                                          (int) 't', ".",
                                          (int) 'N', "Analyzing",
                                          (int) 'n', "_",
                                          (int) 'B', "", (int) 'b', "",
                                          (int) 'W', "", (int) 'w', "",
                                          (int) 'U', "",
                                          (int) 'V', "",
                                          (int) 'L', "", (int) 'l', "", 0);
	if (strncmp(Name, "status ", 7)) {
	    sprintf(title, "Analyzing %s", Name);
	    sprintf(icon,  "Analyzing %s", Name);
	} else {
	    strcpy(title, Name);
	    strcpy(icon,  Name);
	}
    }


    Root = MyVaCreateManagedWidget("analyzer", toplevel,
                                   XtNtitle,       (XtArgVal) title,
                                   XtNiconName,    (XtArgVal) icon,
                                   XtNboardSize,   (XtArgVal) Size,
                                   NULL);

    XtAddCallback(Root, XtNdestroyCallback,
                  CallDestroyAnalyze, (XtPointer) Ana);
    Reset = XtNameToWidget(Root, "*reset");
    if (Reset) XtAddCallback(Reset, XtNcallback, ResetAnalyze,(XtPointer) Ana);
    undo = XtNameToWidget(Root, "*undo");
    if (undo) XtAddCallback(undo, XtNcallback, UndoAnalyze,   (XtPointer) Ana);
    Analyse = XtNameToWidget(Root, "*analyze");
    if (Analyse) XtAddCallback(Analyse, XtNcallback,
                               AnalyzeAnalyze, (XtPointer) Ana);
    AllowSuicide = XtNameToWidget(Root, "*allowSuicide");
    if (AllowSuicide) {
        XtVaSetValues(AllowSuicide, XtNstate,
                      (XtArgVal) (Ana->Log->AllowSuicide ? True : False),
                      NULL);
        XtAddCallback(AllowSuicide, XtNcallback, ToggleAllowSuicide,
                      (XtPointer) Ana);
    }
    board = XtNameToWidget(Root, "*board");
    Blink = XtNameToWidget(Root, "*blink");
    if (Blink && board) {
        XtAddCallback(Blink, XtNcallback, ToggleBlink, (XtPointer) board);
        XtVaGetValues(Blink, XtNstate, (XtArgVal) &state, NULL);
        if (state == False)
            XtVaSetValues(board, XtNoffTimeout, (XtArgVal) 0, NULL);
    }
    Scroll = XtNameToWidget(Root, "*scroll");
    if (Scroll) {
        MyScrollbarSetThumb(Scroll, 0.0, 1.0);
        XtAddCallback(Scroll,  XtNjumpProc,   JumpAnalyze,   (XtPointer) Ana);
        XtAddCallback(Scroll,  XtNscrollProc, ScrollAnalyze, (XtPointer) Ana);
        XtAddCallback(board,   XtNbuttonUp,   AnalyzeButton, (XtPointer) Ana);
    }
    SgfSave = XtNameToWidget(Root, "*sgfSave");
    if (SgfSave) XtAddCallback(SgfSave, XtNcallback, SaveGame,
                               (XtPointer) Ana);
    SgfFile = XtNameToWidget(Root, "*sgfFile");
    if (SgfFile) XtAddCallback(SgfFile, XtNcallback, ChangeSgfFilename,
                               (XtPointer) &Ana->SgfFile);
    PsSave = XtNameToWidget(Root, "*psSave");
    if (PsSave)  XtAddCallback(PsSave,  XtNcallback, SavePs,
                               (XtPointer) Ana);
    PsFile = XtNameToWidget(Root, "*psFile");
    if (PsFile)  XtAddCallback(PsFile, XtNcallback, ChangePsFilename,
                               (XtPointer) &Ana->PsFile);
    Copy = XtNameToWidget(Root, "*copy");
    if (Copy) XtAddCallback(Copy, XtNcallback, CallCopy, (XtPointer) Ana);

    Score = XtNameToWidget(Root, "*score");
    if (Score) XtAddCallback(Score, XtNcallback, CallScore, (XtPointer) Ana);

    Replay = XtNameToWidget(Root, "*replay");
    if (Replay) {
        XtAddCallback(Replay, XtNcallback, ToggleReplay, (XtPointer) Ana);
        XtVaGetValues(Replay, XtNstate, (XtArgVal) &state, NULL);
        ToggleReplay(0, (XtPointer) Ana, INT_TO_XTPOINTER((int) state));
    }

    MyRealizeWidget(Root);

    OpenBoard(board);
    SetBoard(board, GamelogToBoard(Ana->Log));
    CloseBoard(board);
    LastMove(board, -1, -1);

    Ana->TopWidget       = Root;
    Ana->ScrollWidget    = Scroll;
    Ana->BoardWidget     = board;
    Ana->SgfOverwrite    = XtNameToWidget(Root, "*sgfOverwrite");
    Ana->SgfFromStart    = XtNameToWidget(Root, "*sgfFromStart");
    Ana->PsOverwrite     = XtNameToWidget(Root, "*psOverwrite");
    Ana->PsFromStart     = XtNameToWidget(Root, "*psFromStart");
    Ana->Replay          = Replay;
    Ana->MoveBeep        = XtNameToWidget(Root, "*moveBeep");
    Ana->MoveRaise       = XtNameToWidget(Root, "*moveRaise");
    Ana->ErrorBeep       = XtNameToWidget(Root, "*errorBeep");
    Ana->ErrorRaise      = XtNameToWidget(Root, "*errorRaise");
    Ana->Copy            = Copy;
    Ana->Score           = Score;

    return Ana;
}

Analyze *OpenAnalyze(Widget w, const char *Name, int Level, size_t Size,
                     int Move, int AllowSuicide, float WhiteAdvantage)
{
    Analyze *Ana;
    Gamelog *Log;
    char   **Board;

    Ana = mynew(Analyze);
    WITH_HANDLING {
        Ana->Model        = 0;
        Ana->Name         = Ana->SgfFile = Ana->PsFile = NULL;
        Ana->Log          = NULL;
        Ana->Next         =  AnalyzeBase.Next;
        Ana->Previous     = &AnalyzeBase;
        Ana->Next->Previous = Ana->Previous->Next = Ana;
        Ana->Level        = Level;
        Ana->Size         = Size;
        Ana->BaseMove     = Move;
        Ana->Name         = mystrdup(Name);
	Ana->scoring      = False;
	Ana->WhiteAdvantage = WhiteAdvantage;

        Ana->Log  = Log = AllocGamelog(Size, Size, AllowSuicide);
        AddComment(Log, &LastMoveFun, INT_TO_XTPOINTER(Size-1));
        if (w) {
            Board = AllocBoard(Size, Size);
            WITH_UNWIND {
                GetBoard(w, Board);
                PositionToNode(Log, Board, Empty);
            } ON_UNWIND {
                FreeBoard(Board, Size);
            } END_UNWIND;
        } else SetStones(Log, NULL);
        AddComment(Log, &LastMoveFun, INT_TO_XTPOINTER(-Size-1));

        RealOpenAnalyze(Ana);
        if (w) {
            XtAddCallback(w, XtNdestroyCallback, CallDestroyWidgetReference,
                          (XtPointer) &Ana->Model);
            XtAddCallback(w, XtNdestroyCallback, CallDestroy,
                          (XtPointer) Ana->Copy);
            Ana->Model = w;
        } else {
	    XtUnmanageChild(Ana->Copy);
	}
	if (WhiteAdvantage == UNKNOWN_ADVANTAGE) {
	    XtUnmanageChild(Ana->Score);
	}
    } ON_EXCEPTION {
        CallDestroyAnalyze(0, (XtPointer) Ana, 0);
    } END_HANDLING;

    return Ana;
}

Analyze *AnalyzeBoard(char **Board, const char *Name, int Level, size_t Size,
		      int Move, int AllowSuicide, float WhiteAdvantage)
{
    Analyze  *Ana;
    Gamelog  *Log;

    Ana = mynew(Analyze);
    WITH_HANDLING {
        Ana->Name         = Ana->SgfFile = Ana->PsFile = NULL;
        Ana->Log          = NULL;
        Ana->Model        = 0;
        Ana->Name         = mystrdup(Name);
        Ana->Next         =  AnalyzeBase.Next;
        Ana->Previous     = &AnalyzeBase;
        Ana->Next->Previous = Ana->Previous->Next = Ana;
        Ana->Level        = Level+1;
        Ana->Size         = Size;
        Ana->BaseMove     = Move;
	Ana->scoring      = False;
	Ana->WhiteAdvantage = WhiteAdvantage;

        Ana->Log = Log = AllocGamelog(Size, Size, AllowSuicide);
        AddComment(Log, &LastMoveFun, INT_TO_XTPOINTER(Size-1));

        PositionToNode(Log, Board, Empty);
        AddComment(Log, &LastMoveFun, INT_TO_XTPOINTER(-Size-1));

        RealOpenAnalyze(Ana);
        XtUnmanageChild(Ana->Copy);
	if (WhiteAdvantage != UNKNOWN_ADVANTAGE) {
	    CallScore(NULL, (XtPointer)Ana, NULL); /* set scoring mode */
	} else {
	    XtUnmanageChild(Ana->Score);
	}
    } ON_EXCEPTION {
        CallDestroyAnalyze(0, (XtPointer) Ana, 0);
    } END_HANDLING;
    return Ana;
}

void AnalyzeTime(unsigned long diff)
{
    Analyze *analyze;
    int      node;

    for (analyze = AnalyzeBase.Next; analyze != &AnalyzeBase;
         analyze = analyze->Next)
        if (analyze->ReplayRate) {
            analyze->ReplayCount += diff;
            if (analyze->ReplayCount >= analyze->ReplayRate) {
                analyze->ReplayCount = 0;
                node = NodeNumber(analyze->Log);
                if (node == NumberNodes(analyze->Log)-1) {
                    analyze->ReplayRate = 0;
                    if (analyze->Replay)
                        XtVaSetValues(analyze->Replay, XtNstate,
                                      (XtArgVal) False, NULL);
                } else GotoAnalyze(analyze, node+1);
            }
        }
}

static Analyze *FindAnalyzeWidget(Widget w)
{
    Analyze *analyze;

    for (analyze = AnalyzeBase.Next; analyze != &AnalyzeBase;
         analyze = analyze->Next) {
        if (analyze->TopWidget == w || analyze->BoardWidget == w) {
	    return analyze;
	}
    }
    return NULL;
}

void DisplayScore(Boolean scoring, Gamelog *Log, float WhiteAdvantage,
		  Widget BoardWidget, Widget TopWidget)
/* If scoring and positionned at last node, display the score in the title
 * and draw the board with dame highlighted. Otherwise erase the score in
 * the title and clear the dame.
 */
{
    char **Board = GamelogToBoard(Log);
    String title;
    char   newTitle[128];
    char   *p;
    int    node  = NodeNumber(Log);
    int    nodes = NumberNodes(Log)-1;
    int    x, y;

    XtVaGetValues(TopWidget,
		  XtNtitle, (XtArgVal)&title,
		  NULL);
    if (title == NULL) return;
    strncpy(newTitle, title, sizeof(newTitle)-1);
    newTitle[sizeof(newTitle)-1] = '\0';

    p = strrchr(newTitle, '+'); /* look for existing result */
    if (p && p[1] >= '0' && p[1] <= '9') {
	p -= 3; /* point at first space */
	if (p < newTitle) return; /* just for safety, should not happen */
    } else {
	p = strchr(newTitle, '\0');
    }
    if (scoring && node == nodes) {
	sprintf(p, "  %s", BoardScore(Log, WhiteAdvantage, BoardWidget));
    } else {
	*p = '\0';
	/* Clear all marks. The mark on the last move may be restored
	 * later by GotoAnalyze or GotoObserve.
         */
	for (y = 0; y < Log->SizeY; y++) {
	    for (x = 0; x < Log->SizeX; x++) {
		SetMark(BoardWidget, x, y, NoMark);
	    }
	}
    }
    if (strcmp(newTitle, title)) {
	XtVaSetValues(TopWidget,
		      XtNtitle, (XtArgVal)newTitle,
		      NULL);
    }
    OpenBoard(BoardWidget);
    SetBoard(BoardWidget, Board);
    CloseBoard(BoardWidget);
}

static char *BoardScore(Gamelog *Log, float WhiteAdvantage, Widget BoardWidget)

/* Mark territory and dame points and return the computed score as a string
   like "B+0.5".
   In this simple version, dead stones are assumed to be removed already,
   and only IGS counting rules are supported.
   IN assertion: WhiteAdvantage != UNKNOWN_ADVANTAGE
 */
{
    char **Board = GamelogToBoard(Log);
    float       score = WhiteAdvantage;
    static char result[20];
    int         x, y;
    int         size = Log->SizeX;
    Boolean     updated;

#define SEE_WHITE 0x40
#define SEE_BLACK 0x20
#define FLAGS     (SEE_WHITE | SEE_BLACK)

#ifdef DUMP
#  define DUMP_PIECE(x,y) \
     {    if ((Board[y][x] & ~FLAGS) == White) putchar('W'); \
     else if ((Board[y][x] & ~FLAGS) == Black) putchar('B'); \
     else if ((Board[y][x] & FLAGS) == SEE_WHITE) putchar('w'); \
     else if ((Board[y][x] & FLAGS) == SEE_BLACK) putchar('b'); \
     else if ((Board[y][x] & FLAGS) == FLAGS) putchar('d'); /* dame */ \
     else if ((Board[y][x] & FLAGS) == 0) putchar('.'); \
     else putchar('?'); }
#  define NEW_LINE putchar('\n')
#else
#  define DUMP_PIECE(x,y) {}
#  define NEW_LINE {}
#endif

#ifdef DUMP
    printf("Analyze before, w advantage %.1f:\n", score);
#endif
    for (y = 0; y < size; y++) {
	for (x = 0; x < size; x++) {
	    DUMP_PIECE(x,y);
	    if (Board[y][x] == White) {
		Board[y][x] |= SEE_WHITE;
	    } else if (Board[y][x] == Black) {
		Board[y][x] |= SEE_BLACK;
	    }
	}
	NEW_LINE;
    }
    /* Iterate to find the dame: empty points reaching both black and white */
    do {
	updated = FALSE;
	for (y = 0; y < size; y++) {
	    for (x = 0; x < size; x++) {
		char old = Board[y][x];
		if ((old & ~FLAGS) == Empty) {
		    if (x > 0)      Board[y][x] |= (Board[y][x-1] & FLAGS);
		    if (y > 0)      Board[y][x] |= (Board[y-1][x] & FLAGS);
		    if (x < size-1) Board[y][x] |= (Board[y][x+1] & FLAGS);
		    if (y < size-1) Board[y][x] |= (Board[y+1][x] & FLAGS);
		    updated |= (Board[y][x] != old);
		}
	    }
	}
    } while (updated);
#ifdef DUMP
    printf("Analyze after:\n");
#endif
    for (y = 0; y < size; y++) {
	for (x = 0; x < size; x++) {
	    BWMark  mark;
	    BWPiece col = (BWPiece)Board[y][x];

	    DUMP_PIECE(x,y);
	    if (col == (FLAGS | Empty)) {
		mark = (appdata.MarkDame ? DameMark : NoMark);

	    } else if (col == (SEE_WHITE | Empty)) {
		score += 1.;
		mark = (appdata.MarkTerritories ? WhiteMark : NoMark);

	    } else if (col == (SEE_BLACK | Empty)) {
		score -= 1.;
		mark = (appdata.MarkTerritories ? BlackMark : NoMark);
	    } else {
		mark = NoMark;
		/* The mark on the last move may be restored
		 * later by GotoAnalyze or GotoObserve.
		 */
	    }
	    Board[y][x] &= ~FLAGS;
	    SetMark(BoardWidget, x, y, mark);
	}
	NEW_LINE;
    }
    if (score < 0.) {
	sprintf(result, "B+%.1f", -score);
    } else {
	sprintf(result, "W+%.1f", score);
    }
#ifdef DUMP
    printf("score %s\n", result);
    fflush(stdout);
#endif
    return result;
}
