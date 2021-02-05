#include <X11/StringDefs.h>
#include <X11/Intrinsic.h>
#include <X11/Shell.h>
#include <X11/Xaw/AsciiText.h>
#include <X11/Xaw/Toggle.h>

#include <myxlib.h>
#include <mymalloc.h>
#include <except.h>

#include <ctype.h>

#include "events.h"
#include "stats.h"
#include "gointer.h"
#include "xgospel.h"

static void SetInfo(Widget w,  XEvent *event,String *string, Cardinal *n);
static XtActionsRec actionTable[] = {
    { (String) "setinfo",     SetInfo  },
};

struct _Stats {
    Player          *Player;
    char            *Name, *Idle;
    Widget           Root;
/*    struct _NameVal  */
};

void InitStats(Widget Toplevel)
{
    XtAppAddActions(XtWidgetToApplicationContext(Toplevel),
                    actionTable, XtNumber(actionTable));
}

static void UnSpace(char *Value)
{
    char ch, *ptr;

    ptr = Value;
    while (*ptr == ' ') ptr++;
    while ((ch = *ptr++) != 0) *Value++ = ch;
    *Value = 0;
}

static void WrapLine(char *Value, int MaxLength)
{
    char *ptr;
    int   Length;

    Length = strlen(Value);
    if (Length > MaxLength-6) { /* strlen("Info: ") = 6 */
        *Value = '\n';
        while (Length > MaxLength)
            if ((ptr = strchr(Value+MaxLength, ' ')) != NULL) {
                *ptr++  = '\n';
                Length -= ptr-Value;
                Value   = ptr;
            } else break;
    }
}

extern Stats **_PlayerStatsAddress(Player *player);
static void CallDestroyStats(Widget w,
                             XtPointer clientdata, XtPointer calldata)
{
    Stats  *stats, **Ref;
    Player *player;

    player = (Player *) clientdata;
    Ref    =  _PlayerStatsAddress(player);
    stats  = *Ref;

    myfree(stats);
   *Ref = NULL;
}

void SetStatsDescription(Stats *stats)
{
    SetPlayerTitles(stats->Root, stats->Player);
}

/* Fields seem to get compressed if created after realize, when window not on
   screen yet. Should try to find out what causes this --Ton */
static Stats *FindStats(Player *player)
{
    Widget Root, tell, playerObserve;
    Stats **Ref, *stats;

    Ref    = _PlayerStatsAddress(player);
    if (*Ref) return *Ref;

    stats = mynew(Stats);
    stats->Player = player;
    WITH_HANDLING {
        Root = MyVaCreateManagedWidget("stats", toplevel, NULL);
        XtAddCallback(Root, XtNdestroyCallback, CallDestroyStats,
                      (XtPointer) player);
        stats->Root = Root;
	tell = XtNameToWidget(Root, "*getTell");
	if (tell) {
	  CoupleTell(player, tell);
	}
        playerObserve  = XtNameToWidget(Root, "*playerObserve");
        if (playerObserve) {
	  XtAddCallback(playerObserve, XtNcallback,
			CallPlayerObserve, (XtPointer) player);
	}
        SetStatsDescription(stats);
    } ON_EXCEPTION {
        CallDestroyStats(0, (XtPointer) player, NULL);
    } END_HANDLING;
    *Ref = stats;

    return stats;
}

/* Valid until the next call */
/* Convert text to valid widgetname. Just truncate if Name too long */
static const char *NameAsWidget(const char *Prefix, const char *Name)
{
    static char Work[80];
    char       *ptr, ch;
    const char *End;
    int         UpNext;

    if (Prefix) {
        size_t Len;

        Len = strlen(Prefix);
        if (Len >= sizeof(Work)) Len = sizeof(Work)-1;
        memcpy(Work, Prefix, Len);
        ptr = Work+Len;
    } else ptr = Work;

    UpNext = ptr != Work;
    End = Work+sizeof(Work)-1;
    while (ptr<End) {
        ch = *Name++;
        if (ch == 0) break;
        else if (isupper(ch) || ch == '_' || ch == '-') *ptr = ch;
        else if (islower(ch))
            if (UpNext) *ptr = toupper(ch);
            else        *ptr = ch;
        else if (isspace(ch)) {
            UpNext = 1;
            continue;
        } else *ptr = '_';
        UpNext = 0;
        ptr++;
    }
    *ptr = 0;
    if (isupper(Work[0])) Work[0] = tolower(Work[0]);
    return Work;
}

static void CallStatsToggle(Widget w, XtPointer clientdata, XtPointer calldata)
{
    String Text, Ptr;
    XrmValue src, dst;

    if ((Boolean) XTPOINTER_TO_INT(calldata) != False) {
        XtVaGetValues(w, XtNlabel, (XtArgVal) &Text, NULL);
        Ptr = strchr(Text, ':');
        if (Ptr) {
            src.size = strlen(Ptr+2)+1;
            src.addr = (XPointer) Ptr+2;
            dst.size = 0;
            dst.addr = NULL;
            if (XtConvertAndStore(w, XtRString, &src, XtRBoolean, &dst) != 
                False)
                SendCommand(NULL, NULL, "toggle %c%.*s %s",
                            tolower(Text[0]), (int) (Ptr-Text)-1, Text+1,
                            *(Boolean *) dst.addr != False ? "off" : "on");
            else WidgetWarning(w, "Could not convert %s to a boolean", Ptr);
        } else WidgetWarning(w, "Cannot determine current state for toggle");
    }
}

static void SetInfo(Widget w,  XEvent *event,String *string, Cardinal *n)
{
    String Text, Ptr;

    XtVaGetValues(w, XtNstring, (XtArgVal) &Text, NULL);
    if (memcmp(Text, "Info:", 5) == 0) {
        Text += 5;
        while (*Text == ' ' || *Text == '\n' || *Text == '\r') Text++;
    }
    Ptr = Text;
    while ((Ptr = strchr(Ptr, ':')) != NULL) *Ptr++ = ';';
    /* We should only send if changed --Ton */
    SendCommand(NULL, NULL, "info %s", Text);
}

void ShowStats(NameVal *nameval, NameVal *extstats)
{
    const char     *Strength, *Name;
    char           *title, *ptr, Buffer[1500];
    Widget          Root, Text, Collect, Info;
    NameVal        *Nameval;
    const NameList *Line, *Lines;
    Player         *Person;
    size_t          Length;
    Stats          *stats;
    int             SeenExt;
    int             trueRating = -1;

    Strength = Name = NULL;

    for (Nameval=nameval->Next; Nameval != nameval; Nameval=Nameval->Next) {
        if      (strcmp(Nameval->Name, "Info") == 0)
            WrapLine(Nameval->Value, 75);
        else {
            UnSpace(Nameval->Value);
            if      (strcmp(Nameval->Name, "Rank") == 0 && !Strength)
                Strength = Nameval->Value;
            else if (strcmp(Nameval->Name, "Player") == 0 && !Name)
                Name = Nameval->Value;
            else if (strcmp(Nameval->Name, "Rating") == 0)
                Strength = Nameval->Value;
        }
    }
    if (!Name || !*Name) {
        Name = "unknown person";
        Warning("Receiving stats for an unknown person. "
                "Probably out of sync\n");
        Person = PlayerFromName(Name);
    } else Person = PlayerFromName(Name);

    if (Strength) {
        title = strchr(Strength, ' ');
        if (title) {
	    trueRating = atoi(title);
	    *title = 0;
	}
        CheckPlayerStrength(Person, Strength);
	GetExactRating(Person);
        if (title) *title = ' ';
    }
    SendCommand(NULL, NULL, "%%stored%s stored -%s", Name, Name);

    /* Popup a stats window except for the first implicit "stats maintainer" */
    if (!GotStats(Person, nameval)) {
        stats = FindStats(Person);
        Root = stats->Root;

        SeenExt = 0;
        for (Nameval=nameval->Next; Nameval != nameval;
             Nameval=Nameval->Next) {
            const char *LocalName;

            if (Person == Me &&
                (Nameval->Name[0] == 'I' || Nameval->Name[0] == 'i') &&
                Nameval->Name[1] == 'n' && Nameval->Name[2] == 'f' &&
                Nameval->Name[3] == 'o' && Nameval->Name[4] == 0) {
                LocalName = NameAsWidget("*stats", "MyInfo");
            } else {
                LocalName = NameAsWidget("*stats", Nameval->Name);
	    }
   	    if (!strcmp(Nameval->Name, "Rating")) {
		if (*PlayerToAutoRated(Person) == ' ' && trueRating > 0) {
		    /* ignore the rank, display true rating: */
		    sprintf(Buffer, "%.20s: %s  %d", Nameval->Name,
			    RatingToStength(trueRating), trueRating);
		} else if (PlayerExactRating(Person) > 0) {
		     sprintf(Buffer, "%.20s: %.270s (%.2f)", Nameval->Name,
			     Nameval->Value, PlayerExactRating(Person));
		} else {
		    sprintf(Buffer,"%.20s: %.270s", Nameval->Name,
			    Nameval->Value);
		}
	    } else {
		sprintf(Buffer,"%.20s: %.270s", Nameval->Name, Nameval->Value);
	    }
            Text = XtNameToWidget(Root, LocalName);
            if (Nameval == extstats && Person == Me) SeenExt = 1;
            if (SeenExt) {
                if (Text) {
		    XtVaSetValues(Text,
				  XtNlabel, (XtArgVal) Buffer,
				  XtNstate, (XtArgVal) False,
				  NULL);
                } else {
                    Text =
                        MyVaCreateManagedWidget("toggle",Root,
                                                "text",(XtArgVal) Buffer,
                                                "name",(XtArgVal)&LocalName[1],
                                                NULL);
                    XtAddCallback(Text, XtNcallback, CallStatsToggle, NULL);
                }
            } else {
                /* Here we assume Text is a text widget --Ton */
                if (Text) XtVaSetValues(Text, XtNstring,
                                        (XtArgVal) Buffer, NULL);
                else Text =
                    MyVaCreateManagedWidget("info",Root,
                                            "text",(XtArgVal) Buffer,
                                            "name",(XtArgVal)&LocalName[1],
                                            NULL);
                RelaxText(Text);
            }
	    /* Insert a "stored games" widget after "Reg date": */
   	    if (!strcmp(Nameval->Name, "Reg date")) {
                LocalName = NameAsWidget("*stats", "Stored");
		Text = XtNameToWidget(Root, LocalName);
		if (PlayerToStored(Person) >= 0) {
		    sprintf(Buffer, "Stored: %d", PlayerToStored(Person));
		} else {
		    strcpy(Buffer, "Stored: ?");
		}
                if (Text) {
		    XtVaSetValues(Text, XtNstring,
				  (XtArgVal) Buffer, NULL);
                } else {
		    Text = MyVaCreateManagedWidget
			      ("info",Root,
			       "text",(XtArgVal) Buffer,
			       "name",(XtArgVal)&LocalName[1],
			       NULL);
		}
		RelaxText(Text);
	    }
        }
        Lines = PlayerToResults(Person);
        if (Lines && Lines != Lines->Next) {
            const char *LocalName;

            LocalName = NameAsWidget("*stats", "Results");
            Text = XtNameToWidget(Root, LocalName);
            if (Text) XtVaSetValues(Text, XtNstring, (XtArgVal) "", NULL);
            else
                Text = MyVaCreateManagedWidget("results", Root,
                                               "text", (XtArgVal) "",
                                               "name", (XtArgVal)&LocalName[1],
                                               NULL);
            ptr = Buffer;
            for (Line = Lines->Next; Line != Lines; Line = Line->Next) {
                /* We know this new length is far less than sizeof(Buffer)/3 */
                Length = strlen(Line->Name);
                memcpy(ptr, Line->Name, Length);
                ptr += Length;
                if (ptr-Buffer > sizeof(Buffer)*2/3) {
                    *ptr = 0;
                    AddText(Text, Buffer);
                    ptr = Buffer;
                }
                *ptr++ = '\n';
            }
            *--ptr = 0;
            AddText(Text, Buffer);
            RelaxText(Text);
        }
        Collect = XtNameToWidget(Root, "*collect");
        Info    = XtNameToWidget(Root, "*statsInfo");
        if (Collect && Info) XtSetKeyboardFocus(Collect, Info);
        MyRealizeWidget(Root);
    }
}

void RefreshRating(const Player *player)
/* Refresh the stats window if the exact rating has changed */
{
    Stats *stats = *_PlayerStatsAddress((Player *)player);
    const char     *LocalName;
    const char     *name = PlayerToName(player);
    Widget          Text;
    char            Buffer[1500];
    String          oldRating = NULL;
    double          exactRating = PlayerExactRating(player);
    char            *p;

    if (!stats || exactRating < 0.) return;

    LocalName = NameAsWidget("*stats", "Rating");
    Text = XtNameToWidget(stats->Root, LocalName);
    if (!Text) return;

    XtVaGetValues(Text, XtNstring,
		  (XtArgVal) &oldRating, NULL);
    if (oldRating == NULL) return;

    strcpy(Buffer, oldRating);
    p = strrchr(Buffer, '(');
    if (!p) {
	p = strchr(Buffer, '\0');
	*p++ = ' ';
    }
    sprintf(p, "(%.2f)", exactRating);
    if (strcmp(oldRating, Buffer)) {
	XtVaSetValues(Text, XtNstring,
		      (XtArgVal) Buffer, NULL);
	RelaxText(Text);
    }
    /* XtPopup(stats->Root, XtGrabNone); */
}

void RefreshStored(const Player *player)
/* Refresh the stats window if the number of stored games has changed */
{
    Stats *stats = *_PlayerStatsAddress((Player *)player);
    const char     *LocalName;
    Widget          Text;
    char            Buffer[1500];
    String          oldStored = NULL;
    char            *p;

    if (!stats || PlayerToStored(player) < 0) return;

    LocalName = NameAsWidget("*stats", "Stored");
    Text = XtNameToWidget(stats->Root, LocalName);
    if (!Text) return;

    XtVaGetValues(Text, XtNstring,
		  (XtArgVal) &oldStored, NULL);
    if (oldStored == NULL) return;

    strcpy(Buffer, oldStored);
    p = strchr(Buffer, ':');
    if (!p) return;
    sprintf(p, ": %d", PlayerToStored(player));
    if (strcmp(oldStored, Buffer)) {
	XtVaSetValues(Text, XtNstring,
		      (XtArgVal) Buffer, NULL);
	RelaxText(Text);
    }
    /* XtPopup(stats->Root, XtGrabNone); */
}

void GetExactRating(const Player *player)
/* Use the proba command to get exact ratings. Must use a komi
 * of exactly 5.5 or -5.5 because of IGS bug which gives widely
 * different answers for komi of 5.0 and 5.01. Note that to get my
 * own rating, the maintainer must not be autorated (must not have a *).
 */
{
    int myRating = PlayerRating(Me);
    int theirRating = PlayerRating(player);

    /* proba command unusable if I don't have a rating, or if I'm sure
     * that other player does not have a *-rating. If the other player is
     * not connected, we try twice to get first the approximate rating
     * then the exact rating.
     */
    
    if (DebugFun) {
        printf("GetExactRating(%s), myRating %d, theirRating %d%c)\n",
	       PlayerName(player), myRating, theirRating,
	       *PlayerToAutoRated(player));
        fflush(stdout);
    }
    if (myRating <= 0 || *PlayerToAutoRated(player) == ' ') return;

    if (player != Me) {
	if (myRating == theirRating || theirRating <= 0) {
	    SendCommand(NULL, NULL, "%%proba%s proba %s 0 5.5",
			PlayerName(player), PlayerName(player));
	} else {
	    SendCommand(NULL, NULL, "%%proba%s proba %s %d -5.5",
			PlayerName(player), PlayerName(player),
			abs(myRating - theirRating));
	}
    } else if (maintainerRating > 0) {
	if (myRating == maintainerRating) {
	    SendCommand(NULL, NULL, "%%setproba proba %s 0 5.5",
			appdata.Maintainer);
	} else {
	    SendCommand(NULL, NULL, "%%setproba proba %s %d -5.5",
			appdata.Maintainer,
			abs(myRating - maintainerRating));
	}
    }
}

void CallGetStats(Widget w, XtPointer clientdata, XtPointer calldata)
{
    const Player *player;
    const Stats  *stats;
    char Name[80];

    player = (const Player *) clientdata;
    strcpy(Name, PlayerName(player));

/* Idle time now available in stats */
/*    SendCommand(NULL, NULL, "who"); */

    SendCommand(NULL, NULL, "results -%s", Name);
      /* "results -%s' to avoid matches with other names */

    if (w) {
	SendCommand(NULL, NULL, "stats %s", Name);
    } else {
	/* command typed manually, force echo: */
	UserCommand(NULL, "stats %s", Name);
    }
    stats  = *_PlayerStatsAddress((Player *) player);
    if (stats && stats->Root) XtPopup(stats->Root, XtGrabNone);
}

void SetStat(const char *Name, int value)
{
    Stats      *stats;
    Widget      Toggle;
    const char *LocalName;
    char        Buffer[80];

    stats = *_PlayerStatsAddress(Me);
    if (stats) {
        LocalName = NameAsWidget("*stats", Name);
        Toggle = XtNameToWidget(stats->Root, LocalName);
        if (Toggle) {
            sprintf(Buffer, "%s: %s", LocalName+6, value ? "On" : "Off");
            XtVaSetValues(Toggle,
                          XtNstate, (XtArgVal) False,
                          XtNlabel, (XtArgVal) Buffer,
                          NULL);
        } else Warning("Could not found stats widget %s, even though the"
                       "root widget exists\n", LocalName+1);
    }
    /* We should also check that Name is what we toggled and that we toggled
       it to value, but I don't feel particularly paranoid today --Ton */
    if (ArgsCommand(NULL, "toggle") && UserCommandP(NULL)) 
        Outputf("Set %s to be %s\n", Name, value ? "True" : "False");
}
