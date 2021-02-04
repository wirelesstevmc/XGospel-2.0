#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include <X11/StringDefs.h>
#include <X11/IntrinsicP.h>
#include <X11/CoreP.h>
#include <X11/Xatom.h>
#include <X11/Xaw/AsciiText.h>
#include <X11/Xaw/Form.h>
#include <X11/Xaw/Paned.h>
#include <X11/Xaw/Toggle.h>
#include <X11/Xmu/CharSet.h>

#include "myxlib.h"
#include "mymalloc.h"
#include "except.h"
#include "widgettree.h"
#include "YShell.h"
#include "Tree.h"

#ifndef XtREdgeType
# define XtREdgeType    "EdgeType"
#endif /* XtREdgeType */

#define CHANGE    "change"
#define PRTPTR    "#%p"        /* Nice outputformat for a pointer          */
#define NULLNAME  "(null)"     /* How to print the null pointer            */
#define FUNPTR(x) x            /* Convert a functionpointer for printing   */

#define ClassName(x) (((CoreClassPart *)(x))->class_name)

extern void _ShowTree(Widget w);

static Exception NoCallback = { "No calback called" };

extern XtCallbackList _XtGetCallbackList(
#if NeedFunctionPrototypes
 InternalCallbackList *callbacks
#endif
);

int FindCallback(Widget w, String CallbackName, XtCallbackProc callback,
                 XtPointer *client_data)
{
    XtCallbackList        calls;
    XtCallbackProc        fun;
    InternalCallbackList *callbacks;

    callbacks = FetchInternalList(w, CallbackName);
    if (callbacks) {
        for (calls = _XtGetCallbackList(callbacks);
             (fun = calls->callback) != 0; calls++)
                if (fun == callback) {
                    if (client_data) *client_data = calls->closure;
                    return 1;
                }
    } else Raise3(NoCallback, CallbackName,
                  "exists on widget", ExceptionCopy(XtName(w)));
    return 0;
}

extern void _CallFreeTemplate(Widget w,
                              XtPointer clientdata, XtPointer calldata);
/*************/
/* Callbacks */
/*************/

void CallToggleOn(Widget w, XtPointer clientdata, XtPointer calldata)
{
    XtVaSetValues((Widget) clientdata, XtNstate, (XtArgVal) True, NULL);
}
    
void CallToggleOff(Widget w, XtPointer clientdata, XtPointer calldata)
{
    XtVaSetValues((Widget) clientdata, XtNstate, (XtArgVal) False, NULL);
}

void CallToggleUpDown(Widget w, XtPointer clientdata, XtPointer calldata)
{
    Boolean state;

    XtVaGetValues(w, XtNstate, (XtArgVal) &state, NULL);
    if (state == True) XtPopup(  (Widget) clientdata, XtGrabNone);
    else               XtPopdown((Widget) clientdata);
}

void CallReadToggle(Widget w, XtPointer clientdata, XtPointer calldata)
{
    XtVaGetValues(w, XtNstate, (XtArgVal) clientdata, NULL);
}

void CallPopup(Widget w, XtPointer clientdata, XtPointer calldata)
{
    XtPopup((Widget) clientdata, XtGrabNone);
}

void CallPopdown(Widget w, XtPointer clientdata, XtPointer calldata)
{
    XtPopdown((Widget) clientdata);
}

void CallDestroyWidgetReference(Widget w, XtPointer clientdata,
                                XtPointer calldata)
{
    *(Widget *) clientdata = 0;
}

static void CallNoDestroyWidget(Widget w,
                                XtPointer clientdata, XtPointer calldata);

void CallDestroy(Widget w, XtPointer clientdata, XtPointer calldata)
{
#ifdef BLUB
    Widget Parent;

    /* FIXME: I'm programming around a X bug here. A popup that's not
       child of a composite gets an unmanage ateempt even if not
       managed. That is not according to the Xt specs (-Ton) */

    Parent = XtParent((Widget) clientdata);
    if (Parent && XtIsComposite(Parent))
        XtUnrealizeWidget((Widget) clientdata); 
#endif /* BLUB */
    XtDestroyWidget((Widget) clientdata);
}

static void CallDestroyWidget(Widget w, XtPointer clientdata,
                              XtPointer calldata)
{
    XtRemoveCallback((Widget) clientdata, XtNdestroyCallback,
                     CallNoDestroyWidget, (XtPointer) w);
    XtUnrealizeWidget((Widget) clientdata); 
    XtDestroyWidget((Widget) clientdata);
}

static void CallNoDestroyWidget(Widget w,
                                XtPointer clientdata, XtPointer calldata)
{
    XtRemoveCallback((Widget) clientdata, XtNdestroyCallback,
                     CallDestroyWidget, (XtPointer) w);
}

/* Widget w1 should be removed if w2 goes */
void MyDependsOn(Widget w1, Widget w2)
{
    XtAddCallback(w2, XtNdestroyCallback, CallDestroyWidget,   (XtPointer) w1);
    XtAddCallback(w1, XtNdestroyCallback, CallNoDestroyWidget, (XtPointer) w2);
}

void CallFree(Widget w, XtPointer clientdata, XtPointer calldata)
{
    myfree(clientdata);
}

void CallFakeChildDestroy(Widget w, XtPointer clientdata, XtPointer calldata)
{
    XtWidgetProc delete_child;
    Widget Parent, old;

    Parent = (Widget) clientdata;
    if (Parent->core.being_destroyed != False) {
        delete_child = ((CompositeWidgetClass) Parent->core.widget_class)->
            composite_class.delete_child;
        if (delete_child) {
            old = w->core.parent;
            w->core.parent = Parent;
            delete_child(w);
            w->core.parent = old;
        } else WidgetWarning(Parent, "has nullProc as deleteChild");
    }
}

void CallPrint(Widget w, XtPointer clientdata, XtPointer calldata)
{
    fputs((char *) clientdata, stdout);
}

void CallAllowShellResize(Widget w, XtPointer clientdata, XtPointer calldata)
{
    XtVaSetValues((Widget) clientdata, XtNallowShellResize,
                  (Boolean) XTPOINTER_TO_INT(calldata), NULL);
}

void CallAllowResize(Widget w, XtPointer clientdata, XtPointer calldata)
{
    XtVaSetValues((Widget) clientdata, XtNallowResize,
                  (Boolean) XTPOINTER_TO_INT(calldata), NULL);
}

/*******************************************************/
/* Automatic popup/popdown coupled with a togglewidget */
/*******************************************************/

typedef struct {
    Widget (*InitFun)(XtPointer Closure);
    XtPointer Closure;
    Widget    Root;
} CoupleData;

static void CallCreateCouple(Widget w,
                             XtPointer clientdata, XtPointer calldata)
{
    Boolean     state;
    CoupleData *Data;
    Widget      Root;

    if ((Boolean) XTPOINTER_TO_INT(calldata) != False) {
        Data = (CoupleData *) clientdata;
        if (!Data->Root) {
            Root = (*Data->InitFun)(Data->Closure);
            if (Root) {
                CoupleToggleWidget(Root, w, Data->InitFun, Data->Closure);
                XtVaGetValues(w, XtNstate, (XtArgVal) &state, NULL);
		if (state == True) XtPopup(  Root, XtGrabNone);
		else               XtPopdown(Root);
	    }
            else XtVaSetValues(w, XtNstate, (XtArgVal) False, NULL);
        }
    }
}

static void CallNoCouple(Widget w, XtPointer clientdata, XtPointer calldata);

static void DoNoToggle(Widget Toggle, Widget Root)
{
    XtRemoveCallback(Root,XtNpopupCallback,  CallToggleOn, (XtPointer) Toggle);
    XtRemoveCallback(Root,XtNpopdownCallback,CallToggleOff,(XtPointer) Toggle);
    XtRemoveCallback(Root,XtNdestroyCallback,CallNoCouple, (XtPointer) Toggle);
}

static void CallNoToggle(Widget w, XtPointer clientdata, XtPointer calldata)
{
    CoupleData *Data;

    Data = (CoupleData *) clientdata;
    if (Data->Root) DoNoToggle(w, Data->Root);
    myfree(Data);
}

static void CallNoCouple(Widget Root, XtPointer clientdata, XtPointer calldata)
{
    Widget Toggle;
    XtPointer Temp;
    CoupleData *Data;

    Toggle = (Widget) clientdata;
    if (!FindCallback(Toggle, XtNcallback, CallCreateCouple, &Temp))
        Raise1(AssertException, "coupled forward but not backward");
    Data = (CoupleData *) Temp;
    if (Root != Data->Root) {
#ifdef OLD /* problem with Close of tell window if challenge window open */
        Raise1(AssertException,
               "coupled forward and backward but not to each other");
#else
	printf("CallNoCouple Root 0x%x, Data->Root 0x%x\n", Root, Data->Root);
	fflush(stdout);
#endif
    }
    XtRemoveCallback(Toggle, XtNcallback, CallToggleUpDown,  (XtPointer) Root);
    XtVaSetValues(Toggle, XtNstate, (XtArgVal) False, NULL);
    Data->Root   = 0;
}

/*
   Widget Toggle is a toggle widget which we want to be coupled to Root
   popup/existence
   If Root does not exist yet, InitFun called on closure will tell us how
   to make one
   It is the programmers responsibility to make sure the coupling will be done
   if Root is inited from some other place (but you can safely call
   CoupleToggle() after doing the init (but not from within InitFun !))
*/

void CoupleToggleWidget(Widget Root, Widget Toggle,
                        Widget (*InitFun)(XtPointer Clos), XtPointer Clos)
{
    Boolean     state;
    CoupleData *Data;
    XtPointer   Temp;

    Data = 0;
    WITH_HANDLING {
        if (FindCallback(Toggle, XtNcallback, CallCreateCouple, &Temp))
            Data = (CoupleData *) Temp;
    } ON_EXCEPTION {
        ClearException();
    } END_HANDLING;
    if (!Data) {
        Data = mynew(CoupleData);
        Data->Root = 0;
        XtAddCallback(Toggle,XtNcallback, CallCreateCouple,  (XtPointer) Data);
        XtAddCallback(Toggle,XtNdestroyCallback,CallNoToggle,(XtPointer) Data);
    }
    Data->InitFun = InitFun;
    Data->Closure = Clos;
    
    XtVaGetValues(Toggle, XtNstate, (XtArgVal) &state, NULL);
    if (Root) {
        /* Remove any existing coupling */
        if (Data->Root) {
            DoNoToggle(Toggle, Data->Root);
            XtRemoveCallback(Toggle, XtNcallback, CallToggleUpDown,
                             (XtPointer) Data->Root);
        }
      created:
        Data->Root   = Root;
        XtAddCallback(Toggle, XtNcallback,      CallToggleUpDown,
                      (XtPointer) Root);
        XtAddCallback(Root, XtNpopupCallback,   CallToggleOn,
                      (XtPointer) Toggle);
        XtAddCallback(Root, XtNpopdownCallback, CallToggleOff,
                      (XtPointer) Toggle);
        XtAddCallback(Root, XtNdestroyCallback, CallNoCouple,
                      (XtPointer) Toggle);
        /* Don't do this here! This routine is often called before the state
             is set correctly. MV */
	/*	if (state == True) XtPopup(  Root, XtGrabNone);
		else               XtPopdown(Root); */
    } else {
        if (state != False) {
            Root = (*InitFun)(Clos);
            if (Root) goto created;
            XtVaSetValues(Toggle, XtNstate, (XtArgVal) False, NULL);
        }
        Data->Root   = 0;
    }
}

/*****************************************************************************/
/* widget with settable help                                                 */
/*****************************************************************************/

#define XtNtext "text"
#define XtCText "Text"

typedef struct {
    String Text;
} HelpData, *HelpDataPtr;

#define offset(field) XtOffset(HelpDataPtr, field)

static XtResource resources[] = {
    { (String) XtNtext, (String) XtCText, XtRString, sizeof(String),
      offset(Text), XtRString, NULL }, 
};
#undef offset

static void ShowHelp(Widget w, const char *data)
{
    /* We could add some textformatting stuff here -Ton */
    XtVaSetValues(w,
                  XtNlabel, (XtArgVal) data,
                  NULL);
}

extern Widget _HelpOnWidget(Widget w, const char *Name);
Widget _HelpOnWidget(Widget w, const char *Name)
{
    Widget    topwidget, Root, Text;
    XtPointer data;
    HelpData  helpdata;

    topwidget = w;
    while ((Root = XtParent(topwidget)) != NULL) topwidget = Root;
    if (FindCallback(topwidget, XtNdestroyCallback, CallFakeChildDestroy,
                     &data)) topwidget = (Widget) data;

    XtGetSubresources(topwidget, (XtPointer) &helpdata, (String) Name,
                      "HelpData", resources, XtNumber(resources), NULL, 0);
    if (helpdata.Text) {
        if ((Root = XtNameToWidget(topwidget, "*widgetHelp")) != 0&&
            (Text = XtNameToWidget(Root,      "*text"))       != 0) {
            ShowHelp(Text, helpdata.Text);
            return Root;
        } else WidgetWarning(w, "help(%.80s) failed, no subwidget called text",
                             Name);
    } else WidgetWarning(w, "help(%.80s) was not found", Name);
    return 0;
}

/*****************************************************************************/
/* Interactive widget research                                               */
/* WM_PROTOCOL must be initialized !                                         */
/* todo: autmatic delete action on root                                      */
/*****************************************************************************/

static void CallSetValues(Widget w, XtPointer clientdata, XtPointer calldata)
{
    String Name, Value;
    Widget *PassWidget;

    PassWidget = (Widget *) clientdata;
    Name = Value = (char *) "Fail";
    if (PassWidget[1]) XtVaGetValues(PassWidget[1], XtNstring, &Name , NULL);
    if (PassWidget[2]) XtVaGetValues(PassWidget[2], XtNstring, &Value, NULL);
    XtVaSetValues(PassWidget[0],
                  XtVaTypedArg, Name, XtRString, Value, strlen(Value)+1,
                  NULL);
}

extern void _ChangeWidget(Widget widget);
void _ChangeWidget(Widget widget)
{
    Widget    topwidget, Root, Name, Value, Ok, Cancel, *PassWidget;
    Window    Group;
    XtPointer Work;
    char   title[80], icon[80];

    topwidget = widget;
    while ((Root = XtParent(topwidget)) != 0) topwidget = Root;
    XtVaGetValues(topwidget, XtNwindowGroup, (XtArgVal) &Group, NULL);
    if (FindCallback(topwidget, XtNdestroyCallback, CallFakeChildDestroy,
                     &Work)) topwidget = (Widget) Work;

    sprintf(title, "Change widget %s", XtName(widget));
    strcpy(icon, title);
    PassWidget = mynews(Widget, 3);

    Root = MyVaCreateManagedWidget("widgetChange", topwidget,
                                   XtNtitle,       (XtArgVal) title,
                                   XtNiconName,    (XtArgVal) icon,
                                   XtNwindowGroup, (XtArgVal) Group,
                                   XtNeditType,    (XtArgVal) XawtextEdit,
                                   NULL);

    MyDependsOn(Root, widget);
    Name   = XtNameToWidget(Root, "*name");
    Value  = XtNameToWidget(Root, "*value");

    PassWidget[0] = widget;
    PassWidget[1] = Name;
    PassWidget[2] = Value;

    Ok = XtNameToWidget(Root, "*ok");
    if (Ok) {
        XtAddCallback(Ok, XtNcallback, CallSetValues, 
                      (XtPointer) PassWidget);
        XtAddCallback(Ok, XtNdestroyCallback, CallFree,
                      (XtPointer) PassWidget);
    } else myfree(PassWidget);

    Cancel = XtNameToWidget(Root, "*cancel");
    if (Cancel)
        XtAddCallback(Cancel, XtNcallback, CallDestroy, (XtPointer) Root);

    XtCallActionProc(Root, (String) "nexttext", NULL, NULL, 0); 
    MyRealizeWidget(Root);
}

static void CallChangeWidget(Widget w,
                             XtPointer clientdata, XtPointer calldata)
{
    _ChangeWidget((Widget) clientdata);
}

static void CallWidgetAction(Widget w, XtPointer clientdata,
                                       XtPointer calldata)
{
    char  *Calldata;
    Widget Clientdata;
    
    Clientdata = (Widget) clientdata;
    Calldata   = (char *) calldata;
    if (Calldata) {
        if (XmuCompareISOLatin1(Calldata, CHANGE) == 0)
            _ChangeWidget(Clientdata);
        else InfoOfWidget(Clientdata);
    } else InfoOfWidget(Clientdata);
}

static void AddTree(Widget Tree, Widget Me, Widget Parent)
{
    Cardinal i, n;
    Widget   Brother, *Children;

    /* Hey, why isn't this an mycreate (also causes include <Command> ? -Ton */
    Brother = XtVaCreateManagedWidget("treeEntry", commandWidgetClass, Tree,
                                      XtNtreeParent, (XtArgVal) Parent,
                                      XtNlabel,      (XtArgVal) XtName(Me),
                                      NULL);
    MyDependsOn(Brother, Me);
    XtAddCallback(Brother, XtNcallback, CallWidgetAction, (XtPointer) Me);
    n        = 0;
    XtVaGetValues(Me, XtNchildren,    (XtArgVal) &Children,
                      XtNnumChildren, (XtArgVal) &n, NULL);
    for (i=0; i<n; i++)
        AddTree(Tree, Children[i], Brother);

    if (XtIsSubclass(Me, textWidgetClass) != False) {
        Widget Source, Sink;

        XtVaGetValues(Me, XtNtextSource, &Source, XtNtextSink, &Sink, NULL);
        if (Source) AddTree(Tree, Source, Brother);
        if (Sink)   AddTree(Tree, Sink,   Brother);
    }
}

static void CallInfoChildren(Widget w,
                             XtPointer clientdata, XtPointer calldata)
{
    _ShowTree((Widget) clientdata);
}

static void CallInfoPopups(Widget w,
                           XtPointer clientdata, XtPointer calldata)
{
    Widget     Parent;
    WidgetList Popups;
    int        i;

    Parent = (Widget) clientdata;
    Popups = Parent->core.popup_list;
    for (i=Parent->core.num_popups; i>0; i--, Popups++)
        _ShowTree(*Popups);
}

void _ShowTree(Widget w)
{
    char   title[80], icon[80];
    Widget topwidget, Root, Port, Close, Tree, Collect;
    Window Group;
    Dimension CollectWidth, CollectHeight, PortWidth, PortHeight;
    Dimension TreeWidth, TreeHeight;
    XtPointer Work;

    topwidget = w;
    while ((Root = XtParent(topwidget)) != 0) topwidget = Root;
    XtVaGetValues(topwidget, XtNwindowGroup, (XtArgVal) &Group, NULL);
    if (FindCallback(topwidget, XtNdestroyCallback, CallFakeChildDestroy,
                     &Work)) topwidget = (Widget) Work;

    sprintf(title, "Widget tree of %s", XtName(w));
    strcpy(icon, title);

    Root = MyVaCreateManagedWidget("widgetTree", topwidget,
                                   XtNtitle,       (XtArgVal) title,
                                   XtNiconName,    (XtArgVal) icon,
                                   XtNwindowGroup, (XtArgVal) Group,
                                   NULL);

    Collect = XtNameToWidget(Root, "*collect");
    Close   = XtNameToWidget(Root, "*restreeClose");
    Tree    = XtNameToWidget(Root, "*tree");
    Port    = XtNameToWidget(Root, "*viewport");
    AddTree(Tree, w, NULL);
    XawTreeForceLayout(Tree);

    XtAddCallback(Close, XtNcallback, CallDestroy, (XtPointer) Root);
    XtVaGetValues(Collect, XtNwidth,  (XtArgVal) &CollectWidth,
                           XtNheight, (XtArgVal) &CollectHeight, NULL);
    XtVaGetValues(Port,    XtNwidth,  (XtArgVal) &PortWidth,
                           XtNheight, (XtArgVal) &PortHeight, NULL);
    XtVaGetValues(Tree,    XtNwidth,  (XtArgVal) &TreeWidth,
                           XtNheight, (XtArgVal) &TreeHeight, NULL);
    XtRealizeWidget(Root);
    XtVaSetValues(Collect, 
                  XtNwidth,  (XtArgVal) (CollectWidth +TreeWidth -PortWidth),
                  XtNheight, (XtArgVal) (CollectHeight+TreeHeight-PortHeight),
                  NULL);
                              
    XtInstallAllAccelerators(Collect, Collect);
    DeleteProtocol(Root);
    XtPopup(Root, XtGrabNone);
}

Boolean MyIsSubclass(WidgetClass Here, WidgetClass Check)
{
    WidgetClass wc;

    for (wc = Here; wc; wc = MySuperclass(wc))
        if (wc == Check) return True;
    return False;
}

char *FullWidgetName(char *ptr, Widget w)
{
    String name;
    size_t length;

    if (w) {
        ptr = FullWidgetName(ptr, XtParent(w));
        *ptr++ = '.';
        name = XtName(w);
        length = strlen(name);
        memcpy(ptr, name, length);
        ptr += length;
        *ptr++ = '(';
        name = ClassName(XtClass(w));
        length = strlen(name);
        memcpy(ptr, name, length);
        ptr += length;
        *ptr++ = ')';
    }
    return ptr;
}

static const char *NumToCursor(XtArgVal nr)
{
    switch((int) nr) {
      case   0: return "None";
/* There seems to be no easy way to recover the char from the CursorId. Bummer
      case   2: return "arrow";
      case   4: return "based_arrow_down";
      case   6: return "based_arrow_up";
      case   8: return "boat";
      case  10: return "bogosity";
      case  12: return "bottom_left_corner";
      case  14: return "bottom_right_corner";
      case  16: return "bottom_side";
      case  18: return "bottom_tee";
      case  20: return "box_spiral";
      case  22: return "center_ptr";
      case  24: return "circle";
      case  26: return "clock";
      case  28: return "coffee_mug";
      case  30: return "cross";
      case  32: return "cross_reverse";
      case  34: return "crosshair";
      case  36: return "diamond_cross";
      case  38: return "dot";
      case  40: return "dotbox";
      case  42: return "double_arrow";
      case  44: return "draft_large";
      case  46: return "draft_small";
      case  48: return "draped_box";
      case  50: return "exchange";
      case  52: return "fleur";
      case  54: return "gobbler";
      case  56: return "gumby";
      case  58: return "hand1";
      case  60: return "hand2";
      case  62: return "heart";
      case  64: return "icon";
      case  66: return "iron_cross";
      case  68: return "left_ptr";
      case  70: return "left_side";
      case  72: return "left_tee";
      case  74: return "leftbutton";
      case  76: return "ll_angle";
      case  78: return "lr_angle";
      case  80: return "man";
      case  82: return "middlebutton";
      case  84: return "mouse";
      case  86: return "pencil";
      case  88: return "pirate";
      case  90: return "plus";
      case  92: return "question_arrow";
      case  94: return "right_ptr";
      case  96: return "right_side";
      case  98: return "right_tee";
      case 100: return "rightbutton";
      case 102: return "rtl_logo";
      case 104: return "sailboat";
      case 106: return "sb_down_arrow";
      case 108: return "sb_h_double_arrow";
      case 110: return "sb_left_arrow";
      case 112: return "sb_right_arrow";
      case 114: return "sb_up_arrow";
      case 116: return "sb_v_double_arrow";
      case 118: return "shuttle";
      case 120: return "sizing";
      case 122: return "spider";
      case 124: return "spraycan";
      case 126: return "star";
      case 128: return "target";
      case 130: return "tcross";
      case 132: return "top_left_arrow";
      case 134: return "top_left_corner";
      case 136: return "top_right_corner";
      case 138: return "top_side";
      case 140: return "top_tee";
      case 142: return "trek";
      case 144: return "ul_angle";
      case 146: return "umbrella";
      case 148: return "ur_angle";
      case 150: return "watch";
      case 152: return "xterm";
*/
    }
    return NULL;
}

#ifdef SHOW_CORES
#include <signal.h>
#ifndef RETSIGTYPE
# define RETSIGTYPE void
#endif /* RETSIGTYPE */
#include "except.h"

Exception Segmentation = { "Segmentation violation" };
RETSIGTYPE RaiseSeg(void)
{
    Raise(Segmentation);
}
#endif /* SHOW_CORES */

#define STARTTYPE()
#define STOPTYPE()
/*
#define STARTTYPE()  fprintf(stderr, "Starting on name %s, type %s\n",       \
                     Resources[j].resource_name, Type); fflush(stderr);
#define STOPTYPE()   fprintf(stderr, "Stopping on name %s type %s\n",        \
                     Resources[j].resource_name, Type); fflush(stderr);
*/

typedef void (*SomeFun)(void);

static void ShowTranslations(Widget w, size_t Indent, char *Trans)
{
    char  *ptr, *pos, **Lines, **Here, *Out, *OutPtr;
    int   NrLines, Length, Before, OldBefore;
    size_t Temp;

    NrLines = 0;
    for (ptr = Trans; (ptr = strchr(ptr, '\n')) != NULL; ptr++) NrLines++;

    Lines = mynews(char *, NrLines+1);
    WITH_UNWIND {
        Here = Lines;
        *Here++ = ptr = Trans;
        while ((ptr = strchr(ptr, '\n')) != NULL) {
            *ptr++  = 0;
            *Here++ = ptr;
        }
        Here--;
        Length = *Here-Trans;
        *Here = NULL;
        Before = OldBefore = 0;
        for (Here = Lines; (ptr = *Here) != NULL; Here++) {
            pos = strchr(ptr+1, ':');
            if (pos) {
                Temp = pos-ptr;
                OldBefore += Temp;
                if (Temp > Before) Before = Temp;
            }
        }
        Length += NrLines*(Indent+Before)-OldBefore;
        Out = mynews(char, Length+1);

        OutPtr = Out;
        Before++;
        for (Here = Lines; (ptr = *Here) != NULL; Here++) {
            memset(OutPtr, ' ', Indent);
            OutPtr += Indent;
            pos = strchr(ptr+1, ':');
            if (pos) {
                pos++;
                Temp = pos-ptr;
                memcpy(OutPtr, ptr, Temp);
                OutPtr += Temp;
                Temp = Before-Temp;
                memset(OutPtr, ' ', Temp);
                OutPtr += Temp;
            } else pos = ptr;
            Temp = strlen(pos);
            memcpy(OutPtr, pos, Temp);
            OutPtr+= Temp;
            *OutPtr++ = '\n';
        }
        *OutPtr = 0;

        for (ptr = Out; OutPtr-ptr > 500; ptr += 500)
            AddText(w, "%.500s", ptr);
        AddText(w, "%s", ptr);
        myfree(Out);
    } ON_UNWIND {
        myfree(Lines);
    } END_UNWIND;
}

extern void _XtDestroyServerGrabs(Widget w,
                                  XtPointer clientdata, XtPointer calldata);

#define NAME(n) (0 == strcmp(n, Resources[j].resource_name))
#define TYPE(t) (0 == strcmp(t, Type))
#define VAL(t)  (* (t *) Val)

static void ShowVal(Widget Text, char *Temp, int TempLength, Widget w,
                    XtResourceList Resources, Cardinal m, int Mode,
                    Display *Cdisplay, Colormap Cmap,
                    Cardinal nrargs, Cardinal children)
{
    int         j, NameLength, ClassLength, TypeLength;
    size_t      Length;
    XColor      Colors[100], *Color;
    char       *ptr, *Item;
    const char *cptr;
    String      Type, Trans;
    void       *Val;

    strcat(Temp, "  %-*s  %-*s  %-*s  %-s\n"
                 "  %-*s  %-*s  %-*s  %-s\n");
    Item = ((WidgetRec *) w)->core.constraints;

    NameLength = ClassLength = TypeLength = 0;
    Color = Colors;
    for (j=0; j<m; j++) {
        Length = strlen(Resources[j].resource_name);
        if (Length > NameLength)  NameLength = Length;
        Length = strlen(Resources[j].resource_class);
        if (Length > ClassLength) ClassLength = Length;
        if (Mode == 2) Type = Resources[j].default_type;
        else           Type = Resources[j].resource_type;
        if (TYPE(XtRPixel)) {
            if      (Mode == 0)
                Val = &((char *) w)[Resources[j].resource_offset];
            else if (Mode == 1)
                Val = &((char *) Item)[Resources[j].resource_offset];
            else { /* Mode == 2 */
                if (TYPE(XtRImmediate)) {
                    Val = &Resources[j].default_addr;
                    Type = Resources[j].resource_type;
                } else Val = Resources[j].default_addr;
            }
            Color++->pixel = VAL(Pixel);
        }
        Length = strlen(Type);
        if (Length > TypeLength)  TypeLength = Length;
    }
    if (Cmap) XQueryColors(Cdisplay, Cmap, Colors, Color-Colors);
    Color = Colors;

    AddText(Text, "being_destroyed: %s\n",
            ((WidgetRec *) w)->core.being_destroyed == False ?
            "False" : "True");
    if (XtIsWidget(w) != False) {
        AddText(Text, "managed: %s\n",
                ((WidgetRec *) w)->core.managed == False ?
                "False" : "True");
        AddText(Text, "visible: %s\n",
                ((WidgetRec *) w)->core.visible == False ?
                "False" : "True");
    }
    AddText(Text, Temp, NameLength, "Name", ClassLength, "Class",
            TypeLength, "Type", Mode == 2 ? "Default" : "Value",
            NameLength, "----", ClassLength, "-----",
            TypeLength, "----", "-----");

    for (j=0; j<m; j++) {
        Trans = NULL;
        cptr  = Temp;

        if      (Mode == 0) {
            Type = Resources[j].resource_type;
            Val = &((char *) w)[Resources[j].resource_offset];
        } else if (Mode == 1) {
            Type = Resources[j].resource_type;
            Val = &((char *) Item)[Resources[j].resource_offset];
        } else { /* Mode == 2 */
            Type = Resources[j].default_type;
            if (TYPE(XtRImmediate)) {
                Val = &Resources[j].default_addr;
                Type = Resources[j].resource_type; 
            } else if (Resources[j].default_addr) {
                if (TYPE(XtRString)) Val = &Resources[j].default_addr;
                else                 Val = Resources[j].default_addr;
            } else {
                cptr = NULLNAME;
                Val  = NULL; /* get rid of compiler warning */
            }
        }

        if (cptr == Temp) {
#ifdef SHOW_CORES
            RETSIGTYPE (*oldSeg)();
            STARTTYPE();
            oldSeg = signal(SIGSEGV, RaiseSeg);
            WITH_HANDLING {
#endif /* SHOW_CORES */
            if      (TYPE(XtRInt))
                sprintf(Temp, "%d", VAL(int));
            else if (TYPE(XtRCardinal))
                sprintf(Temp, "%d", (int) VAL(Cardinal));
            else if (TYPE(XtRDimension))
                sprintf(Temp, "%d", (int) VAL(Dimension));
            else if (TYPE(XtRPosition))
                sprintf(Temp, "%d", (int) VAL(Position));
            else if (TYPE(XtRPointer))
                sprintf(Temp, PRTPTR, (void *) VAL(XtPointer));
            else if (TYPE(XtRUnsignedChar)) {
                int ch;

                ch = VAL(unsigned char);
                if (isalpha(ch)) sprintf(Temp, "'%c'(0x%02x)", ch, (int) ch);
                else             sprintf(Temp, "'\\x%02x'", ch);
            }
            else if (TYPE(XtRShort)) sprintf(Temp, "%hd", VAL(short));
            else if (TYPE(XtRFloat)) sprintf(Temp, "%f",  (double) VAL(float));
            else if (TYPE(XtRFunction))
                if (VAL(SomeFun)) sprintf(Temp, PRTPTR,
                                          FUNPTR(VAL(SomeFun)));
                else cptr = NULLNAME;
            else if (TYPE(XtRCursor)) {
                XtArgVal cursor;

                cursor = (XtArgVal) VAL(Cursor);
                cptr = NumToCursor(cursor);
                if (!cptr) {
                    sprintf(Temp, "#%08lx", (long) cursor);
                    cptr = Temp;
                }
            }
            else if (TYPE(XtRBitmap))
                if (VAL(Pixmap) == 0) cptr = "None";
                else sprintf(Temp, PRTPTR, (void *) VAL(Pixmap));
            else if (TYPE(XtRPixmap))
                switch(VAL(Pixmap)) {
                  case CopyFromParent:      cptr ="(default)"; break; /* also none */
                  case ParentRelative:      cptr ="ParentRelative"; break;
                  case XtUnspecifiedPixmap: cptr ="XtUnspecifiedPixmap"; break;
                  default: sprintf(Temp, PRTPTR, (void *) VAL(Pixmap)); break;
                }
            else if (TYPE(XtRPixel)) {
                if (Cmap) sprintf(Temp, "#%04x %04x %04x",
                                  Color->red, Color->green, Color->blue);
                else sprintf(Temp, "#%08lx in unknown colormap",
                             (long) Color->pixel);
                Color++;
            }
            else if (TYPE(XtRFontStruct)) {
                XFontStruct  *font;
                unsigned long Result;

                font = VAL(XFontStruct *);
                if (XGetFontProperty(font, XA_FONT, &Result) != False)
                    cptr = XGetAtomName(XtDisplayOfObject(w), (Atom) Result);
                else cptr = "...";
            }
            else if (TYPE(XtRCallback)) {
                InternalCallbackList *icl;
                XtCallbackList calls;
                XtCallbackProc fun;

                icl = &VAL(InternalCallbackList);
                if (icl) {
                    calls = _XtGetCallbackList(icl);
                    ptr = Temp;
                    ptr[2] = ')';
                    ptr[3] = 0;
                    while ((fun = calls->callback) != 0) {
                        if (ptr-Temp > TempLength-40) {
                            memcpy(ptr, ", ...", 5);
                            ptr += 5;
                            break;
                        }
                        if      (fun == CallNoDestroyWidget)
                            sprintf(ptr, ", (CallNoDestroyWidget, %s)",
                                    XtName((Widget) calls->closure));
                        else if (fun == CallToggleOn)
                            sprintf(ptr, ", (CallToggleOn, %s)",
                                    XtName((Widget) calls->closure));
                        else if (fun == CallToggleOff)
                            sprintf(ptr, ", (CallToggleOff, %s)",
                                    XtName((Widget) calls->closure));
                        else if (fun == CallToggleUpDown)
                            sprintf(ptr, ", (CallToggleUpDown, %s)",
                                    XtName((Widget) calls->closure));
                        else if (fun == CallReadToggle)
                            sprintf(ptr, ", (CallReadToggle, " PRTPTR ")",
                                    (void *) calls->closure);
                        else if (fun == CallPopup)
                            sprintf(ptr, ", (CallPopup, %s)",
                                    XtName((Widget) calls->closure));
                        else if (fun == CallPopdown)
                            sprintf(ptr, ", (CallPopdown, %s)",
                                    XtName((Widget) calls->closure));
                        else if (fun == CallDestroyWidgetReference)
                            sprintf(ptr, ", (CallDestroyWidgetReference, "
                                    PRTPTR "(%s))", (void *) calls->closure,
                                    *(Widget *) calls->closure ?
                                    XtName(*(Widget *) calls->closure) :
                                    NULLNAME);
                        else if (fun == CallDestroy)
                            sprintf(ptr, ", (CallDestroy, %s)",
                                    XtName((Widget) calls->closure));
                        else if (fun == CallDestroyWidget)
                            sprintf(ptr, ", (CallDestroyWidget, %s)",
                                    XtName((Widget) calls->closure));
                        else if (fun == CallFree)
                            sprintf(ptr, ", (CallFree, " PRTPTR ")",
                                    (void *) calls->closure);
                        else if (fun == CallPrint) {
                            int Left;

                            Left = TempLength-(ptr-Temp);
                            if ((int) strlen((char *) calls->closure) > Left-40)
                                sprintf(ptr, ", (CallPrint, %.*s...)",
                                        Left-20, (char *) calls->closure);
                            else 
                                sprintf(ptr, ", (CallPrint, %s)",
                                        (char *) calls->closure);
                        } else if (fun == CallNoCouple)
                            sprintf(ptr, ", (CallNoCouple, %s)",
                                    XtName((Widget) calls->closure));
                        else if (fun == CallNoToggle) {
                            CoupleData *data;

                            data = (CoupleData *) calls->closure;
                            sprintf(ptr, ", (CallNoToggle, (" PRTPTR ", "
                                    PRTPTR ", %s))", data->InitFun,
                                    (void *) data->Closure,
                                    data->Root ? XtName(data->Root):NULLNAME);
                        } else if (fun == CallCreateCouple) {
                            CoupleData *data;

                            data = (CoupleData *) calls->closure;
                            sprintf(ptr, ", (CallCreateCouple, (" PRTPTR ", "
                                    PRTPTR ", %s))", data->InitFun,
                                    (void *) data->Closure,
                                    data->Root ? XtName(data->Root):NULLNAME);
                        } else if (fun == _CallFreeTemplate)
                            sprintf(ptr, ", (_CallFreeTemplate, " PRTPTR ")",
                                    (void *) calls->closure);
                        else if (fun == CallDestroyWitchet)
                            sprintf(ptr, ", (CallDestroyWitchet, " PRTPTR ")",
                                    (void *) calls->closure);
                        else if (fun == CallFakeChildDestroy)
                            sprintf(ptr, ", (CallFakeChildDestroy, " PRTPTR
                                    ")", (void *) calls->closure);
                        else if (fun == XtCallbackReleaseCacheRef)
                            sprintf(ptr, ", (XtCallbackReleaseCacheRef, "
                                    PRTPTR ")", (void *) calls->closure);
                        else if (fun == XtCallbackReleaseCacheRefList)
                            sprintf(ptr, ", (XtCallbackReleaseCacheRefList, "
                                    PRTPTR ")", (void *) calls->closure);
                        else if (fun == _XtDestroyServerGrabs)
                            sprintf(ptr, ", (_XtDestroyServerGrabs, "
                                    PRTPTR ")", (void *) calls->closure);
                        else sprintf(ptr, ", (" PRTPTR ", " PRTPTR ")",
                                     FUNPTR(fun), (void *) calls->closure);
                        ptr += strlen(ptr);
                        calls++;
                    }
                    ptr[0] = ')';
                    ptr[1] = 0;
                    Temp[1] = '(';
                    cptr = Temp+1;
                } else cptr = NULLNAME;
            }
            else if (TYPE(XtRInitialState))
                switch(VAL(int)) {
                  case WithdrawnState: cptr = "WithdrawnState"; break;
                  case NormalState:    cptr = "NormalState"; break;
                  case ZoomState:      cptr = "ZoomState"; break;
                  case IconicState:    cptr = "IconicState"; break;
                  case InactiveState:  cptr = "InactiveState"; break;
                  default:             cptr = "Unknown initial state"; break;
                }
            else if (TYPE(XtROrientation))
                switch(VAL(XtOrientation)) {
                  case XtorientVertical:   cptr = XtEvertical;           break;
                  case XtorientHorizontal: cptr = XtEhorizontal;         break;
                  default:                 cptr = "unknown orientation"; break;
                }
            else if (TYPE(XtRJustify))
                switch(VAL(XtJustify)) {
                  case XtJustifyLeft:   cptr = "Left";  break;
                  case XtJustifyRight:  cptr = "Right"; break;
                  case XtJustifyCenter: cptr = "Center"; break;
                  default:              cptr = "unknown justify"; break;
                }
            else if (TYPE(XtRShapeStyle))
                switch(VAL(int)) {
                  case XmuShapeRectangle:        cptr = "Rectangle"; break;
                  case XmuShapeOval:             cptr = "Oval";  break;
                  case XmuShapeEllipse:          cptr = "Ellipse"; break;
                  case XmuShapeRoundedRectangle: cptr = "RoundedRectangle"; break;
                  default:                       cptr = "unknown shapestyle"; break;
                }
            else if (TYPE("ResizeMode"))
                switch(VAL(XawTextResizeMode)) {
                  case XawtextResizeNever:  cptr = "never";  break;
                  case XawtextResizeWidth:  cptr = "width";  break;
                  case XawtextResizeHeight: cptr = "height"; break;
                  case XawtextResizeBoth:   cptr = "both";   break;
                  default:                  cptr = "unknown resizemode"; break;
                }
            else if (TYPE("ScrollMode"))
                switch(VAL(XawTextScrollMode)) {
                  case XawtextScrollNever:      cptr = "Never"; break;
                  case XawtextScrollWhenNeeded: cptr = "WhenNeeded"; break;
                  case XawtextScrollAlways:     cptr = "Always";     break;
                  default:                      cptr = "unknown scrollmode"; break;
                }
            else if (TYPE("WrapMode"))
                switch(VAL(XawTextWrapMode)) {
                  case XawtextWrapNever: cptr = "Never"; break;
                  case XawtextWrapLine:  cptr = "Line";  break;
                  case XawtextWrapWord:  cptr = "Word";  break;
                  default:               cptr = "Unknown wrapmode"; break;
                }
            else if (TYPE(XtREdgeType))
                switch(VAL(XtEdgeType)) {
                  case XtChainTop:    cptr = "ChainTop";         break;
                  case XtChainBottom: cptr = "ChainBottom";      break;
                  case XtChainLeft:   cptr = "ChainLeft";        break;
                  case XtChainRight:  cptr = "ChainRight";       break;
                  case XtRubber:      cptr = "Rubber";           break;
                  default:            cptr = "Unknown edgetype"; break;
                }
            else if (TYPE(XtRGravity))
                switch(VAL(XtGravity)) {
                  case ForgetGravity:    cptr = XtEForget;    break;
                  case NorthWestGravity: cptr = XtENorthWest; break;
                  case NorthGravity:     cptr = XtENorth;     break;
                  case NorthEastGravity: cptr = XtENorthEast; break;
                  case WestGravity:      cptr = XtEWest;      break;
                  case CenterGravity:    cptr = XtECenter;    break;
                  case EastGravity:      cptr = XtEEast;      break;
                  case SouthWestGravity: cptr = XtESouthWest; break;
                  case SouthGravity:     cptr = XtESouth;     break;
                  case SouthEastGravity: cptr = XtESouthEast; break;
                  case StaticGravity:    cptr = XtEStatic;    break;
/*
                  case UnmapGravity:     cptr = XtEUnmap;     break;
*/
                  default: cptr = "Unknown gravity type";     break;
                }
            else if (TYPE(XtREditMode))
                switch(VAL(XawTextEditType)) {
                  case XawtextRead:   cptr = XtEtextRead;        break;
                  case XawtextAppend: cptr = XtEtextAppend;      break;
                  case XawtextEdit:   cptr = XtEtextEdit;        break;
                  default:            cptr = "Unknown editmode"; break;
                }
            else if (TYPE(XtRAsciiType))
                switch(VAL(XawAsciiType)) {
                  case XawAsciiString: cptr = XtEstring;           break;
                  case XawAsciiFile:   cptr = XtEfile;             break;
                  default:             cptr = "Unknown asciitype"; break;
                }
            else if (TYPE(XtRBackingStore))
                switch(VAL(int)) {
                  case NotUseful:  cptr = XtEnotUseful;           break;
                  case WhenMapped: cptr = XtEwhenMapped;          break;
                  case Always:     cptr = XtEalways;              break;
                  case Always + WhenMapped + NotUseful: 
                                   cptr = XtEdefault;             break;
                  default:         cptr = "Unknown backingstore"; break;
                }
            else if (TYPE(XtRVisual))
                if (VAL(Visual *))
                    switch(VAL(Visual *)->class) {
                      case StaticGray:  cptr = "StaticGray";  break;
                      case GrayScale:   cptr = "GrayScale";   break;
                      case StaticColor: cptr = "StaticColor"; break;
                      case PseudoColor: cptr = "PseudoColor"; break;
                      case TrueColor:   cptr = "TrueColor";   break;
                      case DirectColor: cptr = "DirectColor"; break;
                      default:          cptr = "Unknown visual"; break;
                    }
                else cptr = "CopyFromParent";
            else if (TYPE(XtRScreen)) {
                Screen  *scr;

                scr = VAL(Screen *);
                sprintf(Temp, "Screen %d of %s", XScreenNumberOfScreen(scr),
                        DisplayString(DisplayOfScreen(scr)));
            }
            else if (TYPE(XtRAtom)) {
                cptr = XGetAtomName(XtDisplayOfObject(w), VAL(Atom));
            } else if (TYPE(XtRWidget))
                if   (VAL(Widget)) cptr = XtName(VAL(Widget));
                else               cptr = NULLNAME;
            else if (TYPE(XtRWindow))
                sprintf(Temp, PRTPTR, (void *) VAL(Window));
            else if (TYPE(XtRString))
                if (VAL(String)) {
                    cptr = VAL(String);
                    if ((int) strlen(cptr) >= TempLength) {
                        Length = TempLength - 5;
                        memcpy(Temp, cptr, Length);
                        memcpy(Temp+Length, " ...", 5);
                        cptr = Temp;
                    }
                } else cptr = NULLNAME;
            else if (TYPE(XtRStringList)) {
                StringList *strings;
                int         i;
                size_t     *Len;
                char      **Str;

                strings = VAL(StringList *);
                if (strings) {
                    ptr = Temp;
                    Len = strings->Length;
                    Str = strings->String;
                    for (i=strings->Nr; i>0; i--, Str++, Len++) {
                        if (ptr-Temp+*Len+7 >= TempLength) {
                            strcpy(ptr, "\n ...");
                            break;
                        }
                        *ptr++ = '\n';
                        *ptr++ = '\t';
                        memcpy(ptr, *Str, *Len);
                        ptr += *Len;
                    }
                    if (ptr == Temp) cptr = "";
                    else cptr = Temp;
                } else cptr = NULLNAME;
            } else if (TYPE(XtRStringArray) && NAME(XtNargv)) {
                String *strings;
                Cardinal k;

                strings = VAL(String *);
                ptr = Temp;
                for (k=0; k<nrargs; k++) {
                    Length = strlen(strings[k]);
                    if (ptr-Temp+Length+7 >= TempLength) {
                        strcpy(ptr, ", ...");
                        break;
                    }
                    *ptr++ = ',';
                    *ptr++ = ' ';
                    memcpy(ptr, strings[k], Length);
                    ptr += Length;
                }
                if (ptr == Temp) cptr = "()";
                else {
                    *ptr++ = ')';
                    *ptr   = 0;
                    Temp[1] = '(';
                    cptr = Temp+1;
                }
            }
            else if (TYPE(XtRWidgetList) && NAME(XtNchildren)) {
                Widget     *widgets;
                const char *WidgetName;
                Cardinal    k;

                widgets = VAL(Widget *);
                ptr = Temp;
                for (k=0; k<children; k++) {
                    if (widgets[k]) WidgetName = XtName(widgets[k]);
                    else            WidgetName = NULLNAME;
                    Length = strlen(WidgetName);
                    if (ptr-Temp+Length+7 >= TempLength) {
                        strcpy(ptr, ", ...");
                        break;
                    }
                    *ptr++ = ',';
                    *ptr++ = ' ';
                    memcpy(ptr, WidgetName, Length);
                    ptr += Length;
                }
                if (ptr == Temp) cptr = "()";
                else {
                    *ptr++ = ')';
                    *ptr   = 0;
                    Temp[1] = '(';
                    cptr = Temp+1;
                }
            }
            else if (TYPE(XtRBoolean))
                if (VAL(Boolean) != False) cptr = XtEtrue;
                else                       cptr = XtEfalse;
            else if (TYPE(XtRBool))
                if (VAL(Boolean) != False) cptr = XtEtrue;
                else                       cptr = XtEfalse;
            else if (TYPE(XtRTranslationTable)) {
#ifdef NO_MYPRINT
                Trans = NULL;
                cptr = "...";
#else
                Trans = MyPrintXlations(w, VAL(XtTranslations), NULL, True); 
                cptr = "";
#endif /* NO_MYPRINT */
            }
            else if (TYPE(XtRAcceleratorTable)) {
#ifdef NO_MYPRINT
                Trans = NULL;
                cptr = "...";
#else
                Trans = MyPrintXlations(w, VAL(XtTranslations), NULL, True); 
                cptr = "";
#endif /* NO_MYPRINT */
            }
            else strcpy(Temp, "...");
#ifdef SHOW_CORES
            } ON_EXCEPTION {
                cptr = "(Illegal)";
                ClearException();
            } END_HANDLING;
            signal(SIGSEGV, oldSeg);
            STOPTYPE();
#endif /* SHOW_CORES */
        }
        AddText(Text, "  %-*s  %-*s  %-*s  %s\n",
                NameLength,  Resources[j].resource_name,
                ClassLength, Resources[j].resource_class,
                TypeLength,  Type, cptr);
        if (Trans) {
            ShowTranslations(Text, 6, Trans);
            XtFree(Trans);
        }
    }
}
#undef VAL
#undef TYPE
#undef NAME

void InfoOfWidget(Widget w)
{
    Window         Group;
    Widget         Root, topwidget, Close, parent, Text;
    Widget         Parent, Children, Popups, ChangeW;
    WidgetClass    Wc;
    char           title[80], icon[80], Temp[1501];
    XtResourceList Resources;
    XtActionList   Actions, APtr;
    Cardinal       m, children, nrargs, NrActions;
    Display       *Cdisplay;
    Colormap       Cmap;
    XtPointer      Work;

    topwidget = w;
    while ((Root = XtParent(topwidget)) != 0) topwidget = Root;
    XtVaGetValues(topwidget, XtNwindowGroup, (XtArgVal) &Group, NULL);
    if (FindCallback(topwidget, XtNdestroyCallback, CallFakeChildDestroy,
                     &Work)) topwidget = (Widget) Work;

    children = nrargs = 0;
    Cmap     = 0;
    XtVaGetValues(w, XtNcolormap,    (XtArgVal) &Cmap,
                     XtNnumChildren, (XtArgVal) &children,
                     XtNargc,        (XtArgVal) &nrargs, NULL);
    Root = w;
    while (!Cmap) {
        Root = XtParent(Root);
        if (!Root) break;
        XtVaGetValues(Root, XtNcolormap, (XtArgVal) &Cmap, NULL);
    }
    if (Cmap) Cdisplay = XtDisplayOfObject(Root);
    else      Cdisplay = NULL;
    Parent = XtParent(w);
    sprintf(title, "Info on widget %s", XtName(w)); 
    strcpy(icon, title);

    Root = MyVaCreateManagedWidget("widgetInfo", topwidget,
                                   XtNtitle,       (XtArgVal) title,
                                   XtNiconName,    (XtArgVal) icon ,
                                   XtNwindowGroup, (XtArgVal) Group,
                                   NULL);
    Close   = XtNameToWidget(Root, "*restreeClose");
    ChangeW = XtNameToWidget(Root, "*change");
    Text    = XtNameToWidget(Root, "*info");
    if (Parent && 
        (parent = MyVaCreateManagedWidget("parent", Root, NULL)) != 0) {
        XtAddCallback(parent, XtNcallback, CallWidgetAction,
                      (XtPointer) Parent);
        MyDependsOn(parent, Parent);
    }

    if (ChangeW) {
        XtAddCallback(ChangeW, XtNcallback, CallChangeWidget, (XtPointer) w);
        MyDependsOn(ChangeW, w);
    }

    if (children &&
        (Children = MyVaCreateManagedWidget("children", Root, NULL)) != 0) {
        XtAddCallback(Children, XtNcallback, CallInfoChildren, (XtPointer) w);
        MyDependsOn(Children, w);
    }

    if (XtIsWidget(w) != False && w->core.num_popups &&
        (Popups = MyVaCreateManagedWidget("popups", Root, NULL)) != 0) {
        XtAddCallback(Popups, XtNcallback, CallInfoPopups, (XtPointer) w);
        MyDependsOn(Popups, w);
    }

    if (Close)
        XtAddCallback(Close, XtNcallback, CallDestroy, (XtPointer) Root);

    if (Text) {
        /* First get all normal resources */
        XtGetResourceList(XtClass(w), &Resources, &m);
        sprintf(FullWidgetName(Temp, w), " = " PRTPTR "\n", (void *) w);
        ShowVal(Text, Temp+1, sizeof(Temp)-1, w, Resources, m, 0,
                Cdisplay, Cmap, nrargs, children);
        XtFree((char *) Resources);

        /* Next get constraintresources from parent */
        if (Parent) {
            XtGetConstraintResourceList(XtClass(Parent), &Resources, &m);
            if (Resources) {
                strcpy(Temp, "\nThis widget sets the folllowing resources "
                       "for its constraint parent:\n");
                ShowVal(Text, Temp, sizeof(Temp), w, Resources, m, 1,
                        Cdisplay, Cmap, nrargs, children);

                XtFree((char *) Resources);
            }
        }

        /* Get constraintresources we may define */
        XtGetConstraintResourceList(XtClass(w), &Resources, &m);
        if (Resources) {
            strcpy(Temp, "\nThis constraint class allows its children "
                         "to specify the following resources:\n");

            ShowVal(Text, Temp, sizeof(Temp), w, Resources, m, 2,
                    Cdisplay, Cmap, nrargs, children);
            XtFree((char *) Resources);
        }

        /* And get possible actions */
        for (Wc = XtClass(w); Wc; Wc = MySuperclass(Wc)) {
            XtGetActionList(Wc, &Actions, &NrActions);
            if (Actions) {
                APtr = Actions;
                AddText(Text,
                        "\nClass %s contributes the following actions: %s",
                        ((CoreClassPart *) Wc)->class_name, APtr->string);
                for (APtr++, NrActions--; NrActions>0; NrActions--, APtr++)
                    AddText(Text, ", %s", APtr->string);
                XtFree((char *) Actions);
            }
        }

        XawTextSetInsertionPoint(Text, 0);
    }
    MyRealizeWidget(Root);
}
