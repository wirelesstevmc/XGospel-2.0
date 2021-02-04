#include <X11/StringDefs.h>
#include <X11/Intrinsic.h>
#include <X11/Shell.h>
#include <X11/Xaw/AsciiText.h>
#include <X11/Xaw/Toggle.h>
#include <X11/Xmu/CharSet.h>

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#if !STDC_HEADERS && HAVE_MEMORY_H
# include <memory.h>
#endif /* not STDC_HEADERS and HAVE_MEMORY_H */

#include <except.h>
#include <myxlib.h>
#include <mymalloc.h>

#include "broadcast.h"
#include "connect.h"
#include "players.h"
#include "utils.h"
#include "xgospel.h"

#ifdef    HAVE_NO_MEMCHR_PROTO
extern void *memchr(const void *s, int c, size_t n);
#endif /* HAVE_NO_MEMCHR_PROTO */

#define MINCHLENGTH     2

Widget BroadcastButton, YellButton;

static void Broadcast(Widget w, XEvent *event, String *string, Cardinal *n);
static void Yell(Widget w, XEvent *event, String *string, Cardinal *n);
static void ChangeChannel(Widget w, XEvent *event,
                          String *string, Cardinal *n);
static void ChangeChannelTitle(Widget w, XEvent *event,
                               String *string, Cardinal *n);

static XtActionsRec actionTable[] = {
    { (String) "broadcast",              Broadcast  },
    { (String) "yell",                   Yell },
    { (String) "changechannel",          ChangeChannel },
    { (String) "changechanneltitle",     ChangeChannelTitle },
};

struct ChannelData {
    ChannelData *Next, *Previous;
    char        *Name, *Title;
    char        *State;
    char        *Moderator;  /* Maybe make Player * from this ? */
    NameList    *Names;
};

static RegexYesNo BroadcastYesNo, YellYesNo;
static ChannelData ChannelBase = { &ChannelBase, &ChannelBase };
static int    MyChannel;
static size_t NoChannelLength;
static char  *NoChannel;
static char  *BroadcastFileName, *YellFileName;
static Widget broadcastinfo, broadcastinput, broadcastbeep, broadcastraise;
static Widget broadcastoverwrite;
static Widget BroadcastErrorBeep, BroadcastErrorRaise;
static Widget yellinfo, yellinput, YellBeep, YellRaise, yellchannel;
static Widget yelloverwrite, YellErrorBeep, YellErrorRaise, YellTitle;
static Widget YellModerator, YellState;

static void ChangeBroadcastFilename(Widget w,
                                    XtPointer clientdata, XtPointer calldata)
{
    char **Filename;
    Widget Root;

    Filename = (char **) clientdata;
    Root = ChangeFilename(toplevel, "broadcastFilename",
                          "Enter name of broadcast file",
                          Filename, "filename", *Filename, NULL);
    MyDependsOn(Root, w);
}

static void SaveBroadcast(Widget w, XtPointer clientdata, XtPointer calldata)
{
    Boolean                   Overwrite;
    static char const * const ErrorType = "Broadcast save error";

    if (broadcastinfo) {
        if (broadcastoverwrite) XtVaGetValues(broadcastoverwrite, XtNstate,
                                              (XtArgVal) &Overwrite, NULL);
        else Overwrite = False;

        SaveWrite(BroadcastFileName, Overwrite,
                  BroadcastErrorBeep, BroadcastErrorRaise, ErrorType,
                  SaveTextFun, (XtPointer) broadcastinfo);
    } else {
        IfBell(BroadcastErrorBeep);
        IfRaise(BroadcastErrorRaise, BroadcastErrorRaise);
        PopMessage(ErrorType, "Broadcasts were ignored so there is "
                   "nothing to be saved");
    }
}

static void ChangeBroadcastPass(Widget w,
                                XtPointer clientdata, XtPointer calldata)
{
    EditStringList((Widget) clientdata, "Edit broadcast pass pattern",
                   "Invalid pass pattern",
                   &appdata.BroadcastPass, &appdata.BroadcastKill,
                   &BroadcastYesNo, 0);
}

static void ChangeBroadcastKill(Widget w,
                                XtPointer clientdata, XtPointer calldata)
{
    EditStringList((Widget) clientdata, "Edit broadcast kill pattern",
                   "Invalid kill pattern",
                   &appdata.BroadcastPass, &appdata.BroadcastKill,
                   &BroadcastYesNo, 1);
}

void InitBroadcast(Widget Toplevel)
{
    Widget      BroadcastRoot, BroadcastSave, BroadcastFile;
    Widget      BroadcastCollect, Pass, Kill;
    const char *Error;

    EmptyRegexYesNo(&BroadcastYesNo);
    Error = CompileRegexYesNo(appdata.BroadcastPass, appdata.BroadcastKill,
                              &BroadcastYesNo);
    if (Error) Warning("failed to compile broadcast regex pattern: %s\n", 
                       Error);

    XtAppAddActions(XtWidgetToApplicationContext(Toplevel),
                    actionTable, XtNumber(actionTable));
    BroadcastRoot = MyVaCreateManagedWidget("broadcasts", Toplevel, NULL);

    BroadcastFileName = StringToFilename(appdata.BroadcastFilename,
                                         (int) 'T', "Broadcasts",
                                         (int) 't', "_", 0);
    XtAddCallback(BroadcastRoot, XtNdestroyCallback,
                  CallFree, (XtPointer) BroadcastFileName);
    broadcastbeep       = XtNameToWidget(BroadcastRoot, "*beep");
    broadcastraise      = XtNameToWidget(BroadcastRoot, "*raise");
    broadcastinfo       = XtNameToWidget(BroadcastRoot, "*info");
    broadcastinput      = XtNameToWidget(BroadcastRoot, "*input");
    broadcastoverwrite  = XtNameToWidget(BroadcastRoot, "*overwrite");
    BroadcastErrorBeep  = XtNameToWidget(BroadcastRoot, "*errorBeep");
    BroadcastErrorRaise = XtNameToWidget(BroadcastRoot, "*errorRaise");

    Pass = XtNameToWidget(BroadcastRoot, "*passPattern");
    if (Pass) XtAddCallback(Pass, XtNcallback, ChangeBroadcastPass,
                            (XtPointer) BroadcastRoot);
    Kill = XtNameToWidget(BroadcastRoot, "*killPattern");
    if (Kill) XtAddCallback(Kill, XtNcallback, ChangeBroadcastKill,
                            (XtPointer) BroadcastRoot);

    BroadcastSave       = XtNameToWidget(BroadcastRoot, "*save");
    if (BroadcastSave) XtAddCallback(BroadcastSave, XtNcallback, SaveBroadcast,
                                     NULL);
    BroadcastFile       = XtNameToWidget(BroadcastRoot, "*file");
    if (BroadcastFile)
        XtAddCallback(BroadcastFile, XtNcallback, ChangeBroadcastFilename,
                      &BroadcastFileName);
    if (BroadcastButton) {
        XtAddCallback(BroadcastButton,   XtNcallback, CallToggleUpDown,
                      (XtPointer) BroadcastRoot);
        XtAddCallback(BroadcastRoot,    XtNpopupCallback, CallToggleOn,
                      (XtPointer) BroadcastButton);
        XtAddCallback(BroadcastRoot,    XtNpopdownCallback, CallToggleOff,
                      (XtPointer) BroadcastButton);
        CallToggleUpDown(BroadcastButton, (XtPointer) BroadcastRoot, NULL);
    }

    BroadcastCollect    = XtNameToWidget(BroadcastRoot, "*collect");
    if (BroadcastCollect && broadcastinput)
        XtSetKeyboardFocus(BroadcastCollect, broadcastinput);

    XtRealizeWidget(BroadcastRoot);
    if (BroadcastCollect)
        XtInstallAllAccelerators(BroadcastCollect, BroadcastCollect);
    DeleteProtocol(BroadcastRoot);
}

void CleanBroadcast(void)
{
    FreeRegexYesNo(&BroadcastYesNo);
}

static void RealSendBroadcast(Widget w, const char *Name,
                              const char *Text, int length)
{
    int         i;
    const char *Ptr;

    for (i=length, Ptr = Text; i>0; i--, Ptr++) if (!isspace(*Ptr)) break;
    if (i) {
        UserSendCommand(NULL, NULL, "shout %.*s", length, Text);
        if (w) BatchAddText(w, "%16s: %.*s\n", Name, length, Text);
        else        Outputf("%16s: %.*s\n", Name, length, Text);
    } else if (w) BatchAddText(w, ".....       Empty broadcast not sent\n");
    else                Output(".....       Empty broadcast not sent\n");
}

static void Broadcast(Widget w, XEvent *event, String *string, Cardinal *n)
{
    String      Buffer;
    const char *From, *To, *End, *Name;

    if (Me) {
        XtVaGetValues(w, XtNstring, &Buffer, NULL);
        Name = PlayerString(Me);
        End = strchr(Buffer, 0);
        for (From = Buffer;
             (To = memchr((char *)From, '\n', (size_t)(End-From))) != NULL;
             From = To+1)
            RealSendBroadcast(broadcastinfo, Name, From, To-From);
        RealSendBroadcast(broadcastinfo, Name, From, End-From);
        XtVaSetValues(w, XtNstring, "", NULL);
    } else if (broadcastinfo)
        /* Not yet logged in. Leave the text since the user probably wants to
           send it as soon as he does get logged in */
        BatchAddText(broadcastinfo,
                     ".....             You don't exist. Go away\n");
}

void ShowBroadcast(const Player *player, const char *separator,
		   const char *What)
{
    char Buffer[500];
    const char *Error;
    int         Yes, No;
    static char dittobuffer[500] = "empty";
    static int dittolast=0;

    sprintf(Buffer, "%16s%s %s", PlayerString(player), separator, What);
    Error = TestRegexYesNo(Buffer, &BroadcastYesNo, &Yes, &No);
    if (Error) {
        if (broadcastinfo)
            BatchAddText(broadcastinfo, ".....           : %s\n", Error);
        IfBell(BroadcastErrorBeep);
        IfRaise(BroadcastErrorRaise, BroadcastErrorRaise);
    } else if (Yes && !No) {
        if (broadcastinfo) {
  	    if (!strcmp(Buffer, dittobuffer)) {
		dittolast++;
	        BatchAddText(broadcastinfo, "%s", "+");
	    } else if (dittolast > 0) {
		dittolast = 0;
   	        BatchAddText(broadcastinfo, "\n%s\n", Buffer);
	    } else {
   	        BatchAddText(broadcastinfo, "%s\n", Buffer);
	    }
	    strcpy(dittobuffer, Buffer);
	}
        IfBell(broadcastbeep);
        IfRaise(broadcastraise, broadcastraise);
    }
}

static void ChangeYellFilename(Widget w,
                                    XtPointer clientdata, XtPointer calldata)
{
    char **Filename;
    Widget Root;

    Filename = (char **) clientdata;
    Root = ChangeFilename(toplevel, "yellFilename",
                          "Enter name of channel file",
                          Filename, "filename", *Filename, NULL);
    MyDependsOn(Root, w);
}

static void SaveYell(Widget w, XtPointer clientdata, XtPointer calldata)
{
    Boolean                   Overwrite;
    static char const * const ErrorType = "Yell save error";

    if (yellinfo) {
        if (yelloverwrite) XtVaGetValues(yelloverwrite, XtNstate,
                                         (XtArgVal) &Overwrite, NULL);
        else Overwrite = False;

        SaveWrite(YellFileName, Overwrite,
                  YellErrorBeep, YellErrorRaise, ErrorType,
                  SaveTextFun, (XtPointer) yellinfo);
    } else {
        IfBell(YellErrorBeep);
        IfRaise(YellErrorRaise, YellErrorRaise);
        PopMessage(ErrorType, "channels were ignored so there is "
                   "nothing to be saved");
    }
}

static void ChangeYellPass(Widget w, XtPointer clientdata, XtPointer calldata)
{
    EditStringList((Widget) clientdata, "Edit channels pass pattern",
                   "Invalid pass pattern", &appdata.YellPass,&appdata.YellKill,
                   &YellYesNo, 0);
}

static void ChangeYellKill(Widget w, XtPointer clientdata, XtPointer calldata)
{
    EditStringList((Widget) clientdata, "Edit channels kill pattern",
                   "Invalid kill pattern", &appdata.YellPass,&appdata.YellKill,
                   &YellYesNo, 1);
}

void InitYell(Widget Toplevel)
{
    Widget YellRoot;
    Widget YellCollect, YellFile, YellSave, YellChannels, Pass, Kill;
    String Name;
    const char *Error;

    EmptyRegexYesNo(&YellYesNo);
    Error = CompileRegexYesNo(appdata.YellPass, appdata.YellKill, &YellYesNo);
    if (Error) Warning("failed to compile yell regex pattern: %s\n",Error);

    MyChannel = -1;
    NoChannel = NULL;
    XtAppAddActions(XtWidgetToApplicationContext(Toplevel),
                    actionTable, XtNumber(actionTable));
    YellRoot = MyVaCreateManagedWidget("yells", Toplevel, NULL);
    YellFileName = StringToFilename(appdata.YellFilename,
                                    (int) 'T', "Channels",
                                    (int) 't', "_", 0);
    XtAddCallback(YellRoot, XtNdestroyCallback,
                  CallFree, (XtPointer) YellFileName);

    YellBeep       = XtNameToWidget(YellRoot, "*beep");
    YellRaise      = XtNameToWidget(YellRoot, "*raise");
    yellinfo       = XtNameToWidget(YellRoot, "*info");
    yellinput      = XtNameToWidget(YellRoot, "*input");
    yelloverwrite  = XtNameToWidget(YellRoot, "*overwrite");
    YellErrorBeep  = XtNameToWidget(YellRoot, "*errorBeep");
    YellErrorRaise = XtNameToWidget(YellRoot, "*errorRaise");

    Pass = XtNameToWidget(YellRoot, "*passPattern");
    if (Pass) XtAddCallback(Pass, XtNcallback, ChangeYellPass,
                            (XtPointer) YellRoot);
    Kill = XtNameToWidget(YellRoot, "*killPattern");
    if (Kill) XtAddCallback(Kill, XtNcallback, ChangeYellKill,
                            (XtPointer) YellRoot);
 
    YellSave       = XtNameToWidget(YellRoot, "*save");
    if (YellSave) XtAddCallback(YellSave, XtNcallback, SaveYell, NULL);
    YellFile       = XtNameToWidget(YellRoot, "*file");
    if (YellFile) XtAddCallback(YellFile, XtNcallback,
                                ChangeYellFilename, &YellFileName);
    
    YellChannels   = XtNameToWidget(YellRoot, "*channels");
    if (YellChannels)
        XtAddCallback(YellChannels, XtNcallback, CallSendCommand, 
                      (XtPointer) "channels");

    yellchannel = XtNameToWidget(YellRoot, "*channel");
    if (yellchannel) {
        XtVaGetValues(yellchannel, XtNstring, &Name, NULL);
        NoChannelLength = strlen(Name);
        if (NoChannelLength < MINCHLENGTH) {
            size_t Temp;

            NoChannel = mynews(char, MINCHLENGTH+1);
            Temp = MINCHLENGTH-NoChannelLength;
            memset(NoChannel, ' ', Temp);
            memcpy(NoChannel+Temp, Name, NoChannelLength+1);
            NoChannelLength = MINCHLENGTH;
            XtVaSetValues(yellchannel, XtNstring, NoChannel, NULL);
        } else NoChannel = mystrndup(Name, NoChannelLength);
    }

    if (YellButton) {
        XtAddCallback(YellButton, XtNcallback, CallToggleUpDown,
                      (XtPointer) YellRoot);
        XtAddCallback(YellRoot,   XtNpopupCallback, CallToggleOn,
                      (XtPointer) YellButton);
        XtAddCallback(YellRoot,   XtNpopdownCallback, CallToggleOff,
                      (XtPointer) YellButton);
        CallToggleUpDown(YellButton, (XtPointer) YellRoot, NULL);
    }

    YellTitle = XtNameToWidget(YellRoot, "*title");
/*  if (YellTitle) XtUnmanageChild(YellTitle); */

    YellModerator = XtNameToWidget(YellRoot, "*moderator");
    if (YellModerator) XtUnmanageChild(YellModerator);

    YellState = XtNameToWidget(YellRoot, "*state");
    if (YellState) XtUnmanageChild(YellState);

    YellCollect    = XtNameToWidget(YellRoot, "*collect");
    if (YellCollect && yellinput) XtSetKeyboardFocus(YellCollect, yellinput);

    XtRealizeWidget(YellRoot);
    if (YellCollect) XtInstallAllAccelerators(YellCollect, YellCollect);
    DeleteProtocol(YellRoot);
    if (!YellButton) XtPopup(YellRoot, XtGrabNone);
}

void CleanYell(void)
{
    myfree(NoChannel);
    FreeRegexYesNo(&YellYesNo);
}

static void RealSendYell(Widget w, int channel, const char *Name,
                         const char *Text, int length)
{
    int         i;
    const char *Ptr;

    for (i=length, Ptr = Text; i>0; i--, Ptr++) if (!isspace(*Ptr)) break;
    if (i) {
        UserSendCommand(NULL, NULL, "yell \\%d %.*s", channel, length, Text);
        if (w) BatchAddText(w, "%16s: %.*s\n", Name, length, Text);
        else Outputf("%16s: %.*s\n", Name, length, Text);
    } else if (w) BatchAddText(w, ".....       Empty message not sent\n");
    else Output(".....       Empty message not sent\n");
}

static void Yell(Widget w, XEvent *event, String *string, Cardinal *n)
{
    String      Buffer;
    const char *From, *To, *End, *Name;

    XtVaGetValues(w, XtNstring, &Buffer, NULL);
    if (Me) {
        Name = PlayerString(Me);
        End = strchr(Buffer, 0);
        for (From = Buffer;
             (To = memchr((char *)From, '\n', (size_t) (End-From))) != NULL;
             From = To+1)
            RealSendYell(yellinfo, MyChannel, Name, From, To-From);
        RealSendYell(yellinfo, MyChannel, Name, From, End-From);
        XtVaSetValues(w, XtNstring, "", NULL);
    } else if (yellinfo)
        /* Not yet logged in. Leave the text since the user probably wants to
           send it as soon as he does get logged in */
        BatchAddText(yellinfo, ".....       You don't exist. Go away\n");
}

static ChannelData *FindChannel(int num)
{
    ChannelData *Data;

    for (Data = ChannelBase.Next; Data != &ChannelBase; Data = Data->Next)
        if (num == atoi(Data->Name)) return Data;
    return NULL;
}

/* Maybe combine with ChannelName ? -Ton */
static void ShowChannelInfo(void)
{
    ChannelData *Data;
    String       Current;

    Data = FindChannel(MyChannel);
    if (Data) {
        if (YellTitle)
            if (Data->Title)
                XtVaSetValues(YellTitle, XtNstring, Data->Title, NULL);
            else XtVaSetValues(YellTitle, XtNstring, "", NULL);
        if (YellModerator)
            if (Data->Moderator) {
                XtVaGetValues(YellModerator, XtNstring, &Current, NULL);
                if (strcmp(Current, Data->Moderator)) {
                    XtVaSetValues(YellModerator, XtNstring, "", NULL);
                    AddText(YellModerator, Data->Moderator);
                }
                XtManageChild(YellModerator);
            } else XtUnmanageChild(YellModerator);
        if (YellState)
            if (Data->State) {
                XtVaGetValues(YellState, XtNstring, &Current, NULL);
                if (strcmp(Current, Data->State)) {
                    XtVaSetValues(YellState, XtNstring, "", NULL);
                    AddText(YellState, Data->State);
                }
                XtManageChild(YellState);
            } else XtUnmanageChild(YellState);
    } else {
/*      XtUnmanageChild(YellTitle); */
        if (YellTitle)     XtVaSetValues(YellTitle, XtNstring, "", NULL);
        if (YellModerator) XtUnmanageChild(YellModerator);
        if (YellState)     XtUnmanageChild(YellState);
    }
}

static void ChannelName(void)
{
    char   Buffer[80];
    String Buf;

    if (yellchannel) {
        switch(MyChannel) {
          case -1:
            Buf = NoChannel;
            break;
          default:
            sprintf(Buffer, "%*d", (int) NoChannelLength, MyChannel);
            Buf = Buffer;
            break;
        }
        XtVaSetValues(yellchannel, XtNstring, Buf, NULL);
    }
}

static void ChangeChannel(Widget w, XEvent *event, String *string, Cardinal *n)
{
    String Buffer, Buf;
    int    Chan;   

    if (yellchannel) {
        XtVaGetValues(yellchannel, XtNstring, &Buffer, NULL);
        while (isspace(*Buffer)) Buffer++;
        if (XmuCompareISOLatin1(NoChannel, Buffer) == 0) Chan = -1;
        else {
            Chan = strtol(Buffer, &Buf, 0);
            if (Buf == Buffer) Chan = MyChannel;
            else {
                while (isspace(*Buf)) Buf++;
                if (*Buf) Chan = MyChannel;
            }
        }
        if (MyChannel != Chan) {
            SendCommand(NULL, NULL, "; \\%d", Chan);
            /* Next line is needed as long as
               changing to -1 gives no answer -Ton */
            if (Chan == -1) SendCommand(NULL, (XtPointer) 1, "channels");
        }
        ChannelName();
    }
}

static void ChangeChannelTitle(Widget w, XEvent *event,
                               String *string, Cardinal *n)
{
    String       Buffer;
    ChannelData *Data;

    if (YellTitle) {
        XtVaGetValues(YellTitle, XtNstring, &Buffer, NULL);
        if (MyChannel >= 0 && Buffer && Buffer[0]) {
            Data = FindChannel(MyChannel);
            if (!Data || !Data->Title || strcmp(Data->Title, Buffer)) {
                SendCommand(NULL, NULL, "channel %d title %s",
                            MyChannel, Buffer);
                SendCommand(NULL, (XtPointer) 1, "channels");
            } else if (!Data) SendCommand(NULL, (XtPointer) 1, "channels");
        }
        ShowChannelInfo();
    }
}

static void InconsistentChannel(int channel)
{
    if (yellinfo) BatchAddText(yellinfo, "Somehow you seem to have moved from"
                               " channel %d to %d\n", MyChannel, channel);
    MyChannel = channel;
    ChannelName();
    ShowChannelInfo();
}

void WrongChannel(const char *Message)
{
    if (yellinfo) BatchAddText(yellinfo, "Channel must be %s\n", Message);
}

void ChannelDisallowed(void)
{
    if (yellinfo) BatchAddText(yellinfo, "Access is not allowed\n");
}

void JoinChannel(int channel)
{
    if (yellinfo && channel != MyChannel)
        if (channel == -1)
             BatchAddText(yellinfo, "You left all channels\n"); 
        else BatchAddText(yellinfo, "You moved to channel %d\n", channel);
    MyChannel = channel;
    ChannelName();
    ShowChannelInfo();
    MyChannel = channel;
    ChannelName();
    ShowChannelInfo();
}

void RejoinChannel(void)
{
    if (MyChannel >= 0) {
        FirstCommand(NULL, "; \\%d", MyChannel);
        MyChannel = -1;
        ChannelName();
        ShowChannelInfo();
    }
}

void ChannelJoin(int channel, const Player *Who)
{
    if (channel != MyChannel) InconsistentChannel(channel);
    if (yellinfo) BatchAddText(yellinfo, "%16s has joined channel %d\n",
                               PlayerString(Who), channel);
}

void ChannelLeave(int channel, const Player *Who)
{
    if (channel != MyChannel) InconsistentChannel(channel);
    if (yellinfo) BatchAddText(yellinfo, "%16s has left channel %d\n",
                               PlayerString(Who), channel);
}

void ChannelTitle(int channel, const Player *Who, const char *Title)
{
    ChannelData *Data;
    char        *NewTitle, *NewModerator;
    String       Current;
    const char  *WhoName;

    if (channel != MyChannel) InconsistentChannel(channel);
    if (yellinfo) BatchAddText(yellinfo, "%16s has changed the title of "
                               "channel %d to: %s\n", PlayerString(Who),
                               channel, Title);
    WhoName = PlayerToName(Who);
    if (YellModerator) {
        XtVaGetValues(YellModerator, XtNstring, &Current, NULL);
        if (strcmp(Current, WhoName)) {
            XtVaSetValues(YellModerator, XtNstring, "", NULL);
            AddText(YellModerator, WhoName);
        }
        XtManageChild(YellModerator);
    }
    Data = FindChannel(channel);
    if (Data) {
        if (YellTitle)
            if (Title)
                XtVaSetValues(YellTitle, XtNstring, Title, NULL);
            else XtVaSetValues(YellTitle, XtNstring, "", NULL);
        NewTitle = mystrdup(Title);
        myfree(Data->Title);
        Data->Title = NewTitle;
        NewModerator = mystrdup(WhoName);
        myfree(Data->Moderator);
        Data->Moderator = NewModerator;
    } else if (YellTitle)     XtVaSetValues(YellTitle, XtNstring, "", NULL);
}

void ShowYell(int channel, const Player *Who, const char *What)
{
    char        Buffer[500];
    const char *Error;
    int         Yes, No;

    if (channel != MyChannel) InconsistentChannel(channel);

    sprintf(Buffer, "%16s: %s", PlayerString(Who), What);
    Error = TestRegexYesNo(Buffer, &BroadcastYesNo, &Yes, &No);
    if (Error) {
        if (yellinfo) BatchAddText(yellinfo, ".....           : %s\n", Error);
        IfBell(YellErrorBeep);
        IfRaise(YellErrorRaise, YellErrorRaise);
    } else if (Yes && !No) {
        if (yellinfo) BatchAddText(yellinfo, "%s\n", Buffer);
        IfBell(YellBeep);
        IfRaise(YellRaise, YellRaise);
    }
}

typedef struct {
    int            channel;
    const Player  *player;
} ChannelPlayer;

static void CleanChannelData(void)
{
    ChannelData *Here, *Next;

    for (Here = ChannelBase.Next; Here != &ChannelBase; Here = Next) {
        Next = Here->Next;
        myfree(Here->Name);
        myfree(Here->Moderator);
        myfree(Here->Title);
        myfree(Here->State);
        FreeNameList(Here->Names);
        myfree(Here);
    }
    Here->Previous = Here->Next = Here;
}

ChannelData *OpenChannelData(void)
{
    CleanChannelData();
    ChannelBase.Name = NULL; /* Here we will remember our own channel */
    return &ChannelBase;
}

void AddChannelData(ChannelData *Data, char *Name, char *Moderator,
                    char *Title, char *state, NameList *Names)
{
    ChannelData *NewData;

    NewData = mynew(ChannelData);
    NewData->Name = NewData->Moderator =
        NewData->Title = NewData->State = NULL;
    WITH_HANDLING {
        NewData->Name      = mystrdup(Name);
        NewData->Moderator = mystrdup(Moderator);
        NewData->Title     = mystrdup(Title);
        NewData->State     = mystrdup(state);
        NewData->Names     = Names;
    } ON_EXCEPTION {
        if (NewData->Name)      free(NewData->Name);
        if (NewData->Moderator) free(NewData->Moderator);
        if (NewData->Title)     free(NewData->Title);
        if (NewData->State)     free(NewData->State);
    } END_HANDLING;

    NewData->Next = Data;
    NewData->Previous = Data->Previous;
    NewData->Previous->Next = NewData->Next->Previous = NewData;
}

void         CloseChannelData(ChannelData *Data)
{
    ShowChannelInfo();
}

void         ChannelList(const ChannelData *Data)
{
    ChannelData   *Here;
    int            Special, NrCols, col, NrPeople, i;
    NameList      *Now, *From;
    const char    *Text, *WasChannel;
    const Player **People, **Ptr;

    WasChannel = ArgsCommand(NULL, "channels");
    if (WasChannel && CommandClosure(NULL)) {
        Text = PlayerToName(Me);
        for (Here = Data->Next; Here != Data; Here = Here->Next) {
            From = Here->Names;
            for (Now = From->Next; Now != From; Now = Now->Next)
                if (strcmp(Text, Now->Name) == 0) {
                    ((ChannelData *)Data)->Name = Here->Name;
                    JoinChannel(atoi(Here->Name));
                    return;
                }
        }
        JoinChannel(-1);
        return;
    }

    Special = yellinfo && !(UserCommandP(NULL) && WasChannel);
    if (Special) NrCols = 3;
    else         NrCols = 4;

    for (Here = Data->Next; Here != Data; Here = Here->Next) {
        if (Special)
            BatchAddText(yellinfo, "%2s %10s -- %s -- %s", Here->Name,
                         Here->Moderator ? Here->Moderator : "",
                         Here->Title, Here->State);
        else Outputf("%2s %10s -- %s -- %s", Here->Name,
                     Here->Moderator ? Here->Moderator : "",
                     Here->Title, Here->State);
        From = Here->Names;
        col = 0;
        NrPeople = 0;
        for (Now = From->Next; Now != From; Now = Now->Next) NrPeople++;
        People = mynews(const Player *, NrPeople);
        WITH_UNWIND {
            Ptr = People;
            for (Now = From->Next; Now != From; Now = Now->Next) {
                *Ptr = PlayerFromName(Now->Name);
                if (Me == *Ptr++) ((ChannelData *)Data)->Name = Here->Name;
            }
            qsort((void *) People, (size_t) NrPeople, sizeof(*People),
                  PlayersCompare);
            for (Ptr = People, i = NrPeople; i > 0; Ptr++, i--) {
                Text = PlayerString(*Ptr);
                if (Special) BatchAddText(yellinfo, "%s%16s",
                                          col ? "  " : "\n  ", Text);
                else Outputf("%s%16s", col ? "  " : "\n  ", Text);
                if (++col == NrCols) col = 0;
            }
        } ON_UNWIND {
            myfree(People);
            if (Special) BatchAddText(yellinfo, "\n");
            else Output("\n");
        } END_UNWIND;
    }
    if (Data->Name) JoinChannel(atoi(Data->Name));
    else            JoinChannel(-1);
}
