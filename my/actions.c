#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <limits.h>

#include <X11/StringDefs.h>
#include <X11/IntrinsicP.h>
#include <X11/ShellP.h>
#include <X11/CoreP.h>
#include <X11/Xos.h>
#include <X11/Xaw/AsciiText.h>

extern int close(/* int fd */);
extern pid_t fork(/* void */);
extern int execvp(/* char *file, char **argv */);

#if XlibSpecificationRelease <= 5
/* In IntrinsicP.h in X11R6 */
typedef struct _XtTypedArg {
    String      name;
    String      type;
    XtArgVal    value;
    int         size;
} XtTypedArg, *XtTypedArgList;
#endif /* XlibSpecificationRelease */

#include "myxlib.h"
#include "mymalloc.h"
#include "except.h"
#include "reslang.h"
#include "TearofMenu.h"

extern void   _ChangeWidget(Widget widget);
extern void   _ShowTree(Widget w);
extern Widget _HelpOnWidget(Widget w, const char *Name);

Atom ProtocolList[1];
static Atom DELETE;

/*****************************************************************************/
/* An exit procedure that handles pending events                             */
/*****************************************************************************/

static int DoLoop;
static XtPointer LoopRc;

static Boolean ExitWorkProc(XtPointer closure)
{
    exit(XTPOINTER_TO_INT(closure));
    return True;
}

void Xexit(XtAppContext App, int status)
{
    XtAppAddWorkProc(App, ExitWorkProc, INT_TO_XTPOINTER(status));
}

static Boolean QuitWorkProc(XtPointer closure)
{
    LoopRc = closure;
    DoLoop = 0;
    return True;
}

void Xquit(XtAppContext App, XtPointer status)
{
    XtAppAddWorkProc(App, QuitWorkProc, status);
}

XtPointer MyAppMainLoop(XtAppContext app_context)
{
    XEvent event;

    DoLoop = 1;
    while (DoLoop) {
        XtAppNextEvent(app_context, &event);
        XtDispatchEvent(&event);
    }
    return LoopRc;
}

/*****************************************************************************/
/* Multiple WM_PROTOCOL messages                                             */
/*****************************************************************************/

const char *ResourceStringToString(const char *From)
{
/*
    ResParse *Parse;

    Parse = resParse(From, strlen(From));
    fprintParse(stdout, Parse);
    putc('\n', stdout);
    FreeParse(Parse);
*/
    return From;
}

/* return LONG_MIN if the argument is invalid */
static long ResourceStringToLong(const char *From)
{
    char *ptr;
    long  Val;
/*
    ResParse *Parse;

    Parse = resParse(From, strlen(From));
    fprintParse(stdout, Parse);
    putc('\n', stdout);
    FreeParse(Parse);
*/
    while (isspace(*From)) From++;
    Val = strtol(From, &ptr, 0);
    while (isspace(*ptr)) ptr++;
    if (!*From || *ptr) Val = LONG_MIN;
    return Val;
}

/* Return INT_MIN if the argument is invalid */
int ResourceStringToInt(const char *From)
{
    long Val;

    Val = ResourceStringToLong(From);
    if      (Val <= INT_MIN) Val = INT_MIN;
    else if (Val >  INT_MAX) Val = INT_MIN;
    return Val;
}

#define ClassName(x) (((CoreClassPart *)(x))->class_name)

#ifndef   HAVE_NO_STDARG_H
void WidgetWarning(Widget w, const char *Format, ...)
#else  /* HAVE_NO_STDARG_H */
void WidgetWarning(w, va_alist)
Widget w;
va_dcl
#endif /* HAVE_NO_STDARG_H */
{
    va_list         args;
    char            Buffer[2048];
#ifndef   HAVE_NO_STDARG_H
    va_start(args, Format);
#else  /* HAVE_NO_STDARG_H */
    const char *Format;

    va_start(args);
    Format = va_arg(args, const char *);
#endif /* HAVE_NO_STDARG_H */
    sprintf(Buffer, "%.*s%s %.*s: ",
            (int) sizeof(Buffer)/4-9, ClassName(XtClass(w)),
            XtIsWidget(w) == False ? "Object" : "Widget",
            (int) sizeof(Buffer)/4, XtName(w));
    vsprintf(strchr(Buffer, 0), Format, args);
    va_end(args);
    XtAppWarning(XtWidgetToApplicationContext(w), Buffer);
}

/* This code is stolen from SimpleMenu */
/* ARGSUSED */
static void PopupMenu(Widget w, XEvent *evnt, String *str, Cardinal *n)
/* popupmenu params:
 *  1st = widget name
 *  2nd = "1" for tearofMenu (menu stays) or "-1" for no grab
 *  3rd = "middle" to popup menu in middle of parent shell
 *  IN assertion: evnt can be NULL only if str[1] == "-1".
 */
{
    Widget      menu, temp, last;
    int         menu_x, menu_y, menu_width, menu_height, tear;
    Position    button_x, button_y;
    const char *Name;
    XtPointer   data;

    if (*n == 1 || *n == 2 || *n == 3) {
        Name = ResourceStringToString(str[0]);
        menu = last = NULL;
        for (temp = w; temp; temp = XtParent(temp)) {
            last = temp;
            menu = XtNameToWidget(temp, (String) Name);
            if (menu) break;
        }
        if (!menu && last && FindCallback(last, XtNdestroyCallback,
                                          CallFakeChildDestroy, &data))
            menu = XtNameToWidget((Widget) data, (String) Name);

        if (menu) {
            if (XtIsSubclass(menu, tearofMenuWidgetClass) != False) {
                tear = *n > 1 && ResourceStringToInt(str[1]) > 0;
                if (!tear) XtVaSetValues(menu, XtNtearState,
                                         (XtArgVal) SIMPLEMENUSTATE, NULL);
            } else tear = 0;

            if (XtIsRealized(menu) == False) XtRealizeWidget(menu);

            menu_width    = menu->core.width  + 2 * menu->core.border_width;
            menu_height   = menu->core.height + 2 * menu->core.border_width;

            if (*n > 2)
                for (temp = w; temp; temp = XtParent(temp))
                    if (XtIsShell(temp) != False) {
                        w = temp;
                        break;
                    }
            XtTranslateCoords(w, 0, 0, &button_x, &button_y);
            menu_x = button_x - (menu_width - (int) w->core.width)/2;
            if (*n > 2)
                menu_y = button_y - (menu_height - (int) w->core.height)/2;
            else menu_y = button_y + w->core.height + w->core.border_width;

            if (menu_x >= 0) {
                int scr_width = WidthOfScreen(XtScreen(menu));
                if (menu_x + menu_width > scr_width)
                    menu_x = scr_width - menu_width;
            }
            if (menu_x < 0) menu_x = 0;

            if (menu_y >= 0) {
                int scr_height = HeightOfScreen(XtScreen(menu));
                if (menu_y + menu_height > scr_height)
                    menu_y = scr_height - menu_height;
            }
            if (menu_y < 0) menu_y = 0;

            if (tear) {
                XtVaSetValues(menu,
                              XtNx,         (XtArgVal) menu_x,
                              XtNy,         (XtArgVal) menu_y,
                              XtNtearState, (XtArgVal) BEINGTEAREDSTATE,
                              XtNoverrideRedirect, (XtArgVal) True,
                              XtNxRoot,     (XtArgVal)
                                      (((XButtonEvent *) evnt)->x_root-menu_x),
                              XtNyRoot,     (XtArgVal)
                                      (((XButtonEvent *) evnt)->y_root-menu_y),
                              NULL);
                XtPopupSpringLoaded(menu);
                XtGrabPointer(menu, False, PointerMotionMask | ButtonPressMask
                              | ButtonReleaseMask, 
                              GrabModeAsync, GrabModeAsync, None, None, 
                              ((XButtonEvent *) evnt)->time);
            } else {
                XtVaSetValues(menu,
                              XtNx, (XtArgVal) menu_x,
                              XtNy, (XtArgVal) menu_y,
                              XtNoverrideRedirect, (XtArgVal) True,
                              NULL);
		if (*n > 1 && ResourceStringToInt(str[1]) < 0) {
		    XtPopup(menu, XtGrabNone);
		} else {
		    XtPopupSpringLoaded(menu);
		}
            }
        } else WidgetWarning(w, "popupmenu() action could not find widget "
                             "named %.*s", 500, Name);
    } else WidgetWarning(w, "popupmenu() action must be called with just the "
                         "popupshell name as an argument");
}

static void WidgetTree(Widget w, XEvent *evnt, String *str, Cardinal *n)
{
    Widget         Test;

    for (Test = w; Test; Test = XtParent(Test))
        if (XtIsShell(Test) != False) {
            _ShowTree(Test);
            return;
        }
    WidgetWarning(w, "could not find Shell parent. This is bad(TM)");
}

static void Info(Widget w, XEvent *evnt, String *str, Cardinal *n)
{
    Widget         Test;

    Test = XtWindowToWidget(XtDisplay(w), ((XButtonEvent *) evnt)->window);
    if (Test) w = Test;
    InfoOfWidget(w);
}

static void Help(Widget w, XEvent *evnt, String *str, Cardinal *n)
/* help() params: 
 *  1st = name of widget containing help string
 *  2nd = "0" or absent to popup a help window with grab
 *        "1"  to popup a tearofMenu (menu stays)
 *        "-1" to popup a help window (no grab)
 *        "down" to popdown the help window
 *  IN assertion: evnt can be NULL only if str[1] == "-1" or "down".
 */
{
    const char *Name;
    Widget      Root;
    const char *WorkStr[3];
    Cardinal    WorkN;

    WorkStr[1] = "0";
    switch(*n) {
      default:
        WidgetWarning(w, "help() action must have 0, 1 or 2 arguments. "
                      "Extra arguments ignored");
      case 2:
        WorkStr[1] = str[1];
        /* Intended drop through */
      case 1:
        Name = ResourceStringToString(str[0]);
        break;
      case 0:
        Name = XtName(w);
        break;
    }
    Root = _HelpOnWidget(w, Name);
    if (!Root) {
	WidgetWarning(w, "help() action could not find widget "
		      "named %.*s", 500, Name);
	return;
    }

    WorkStr[0] = XtName(Root);
    WorkStr[2] = "middle";
    WorkN      = XtNumber(WorkStr);
    if (strcmp(WorkStr[1], "down")) {
	PopupMenu(w, evnt, (String *) WorkStr, &WorkN);
    } else {
	Widget      menu, temp, last;
	XtPointer   data;

	Name = ResourceStringToString(WorkStr[0]);
	menu = last = NULL;
	for (temp = w; temp; temp = XtParent(temp)) {
	    last = temp;
	    menu = XtNameToWidget(temp, (String) Name);
	    if (menu) break;
	}
	if (!menu && last && FindCallback(last, XtNdestroyCallback,
					  CallFakeChildDestroy, &data)) {
	    menu = XtNameToWidget((Widget) data, (String) Name);
	}
	if (menu) {
	    XtPopdown(menu);
	} else {
	    WidgetWarning(w, "help() action could not find widget "
			 "named %.*s", 500, Name);
	}
    }
}

static void Beep(Widget w, XEvent *evnt, String *str, Cardinal *n)
{
    int i, m;

    m = *n;
    if (m) for (i=0; i<m; i++)
        XBell(XtDisplay(w), ResourceStringToInt(str[i]));
    else XBell(XtDisplay(w), 20);
}

static void WidgetAction(Widget w, XEvent *evnt,
                         String *str, Cardinal *n)
{
    if (*n) XtCallCallbacks(w, XtNcallback,
                            (XtPointer) ResourceStringToString(str[0]));
    else    XtCallCallbacks(w, XtNcallback, (XtPointer) NULL);
}

static void SetFocus(Widget w, XEvent *event, String *str, Cardinal *n)
{
    Widget   Here, Source, Dest, *Children;
    Cardinal NumChildren;

    Here = Dest = Source = w;
    while (XtIsShell(Here) == False) Here = XtParent(Here);
    XtVaGetValues(Here,
                  XtNchildren,    (XtArgVal) &Children,
                  XtNnumChildren, (XtArgVal) &NumChildren,
                  NULL);
    /* Should always be at least one (1 on R5, 2 on R6), but I'm paranoid */
    if (NumChildren) Source = Children[NumChildren-1];

    switch(*n) {
      case 2:
        w = MyNameToWidget(Here, ResourceStringToString(str[1]));
        if (w) Source = w;
        else WidgetWarning(w, "setfocus() called with an invalid widgetname "
                           "as source");
        /* Intended drop through */
      case 1:
        w = MyNameToWidget(Here, ResourceStringToString(str[0]));
        if (w) Dest = w;
        else WidgetWarning(w, "setfocus() called with an invalid widgetname "
                           "as destination");
        /* Intended drop through */
      case 0:
        XtSetKeyboardFocus(Source, Dest);
        break;
      default:
        WidgetWarning(w, "setfocus() action called with more arguments than "
                      "just source and destination widgets");
        break;
    }
}

static void Unmanage(Widget w, XEvent *event, String *str, Cardinal *n)
{
    Widget *Widgets, *Ptr, Root;
    int     i;

    Root = w;
    while (XtIsShell(Root) == False) Root = XtParent(Root);

    Ptr = Widgets = mynews(Widget, *n);
    for (i= *n; i>0; i--, str++)
        if (0 ==(*Ptr++ =MyNameToWidget(Root, ResourceStringToString(*str)))) {
            WidgetWarning(w, "unmanage() called on invalid widget name");
            break;
        }
    if (!i) XtUnmanageChildren(Widgets, *n);
    myfree(Widgets);
}

static void Manage(Widget w, XEvent *event, String *str, Cardinal *n)
{
    Widget *Widgets, *Ptr, Root;
    int     i;

    Root = w;
    while (XtIsShell(Root) == False) Root = XtParent(Root);

    Ptr = Widgets = mynews(Widget, *n);
    for (i= *n; i>0; i--, str++)
        if (0 ==(*Ptr++ =MyNameToWidget(Root, ResourceStringToString(*str)))) {
            WidgetWarning(w, "manage() called on invalid widget name");
            break;
        }
    if (!i) XtManageChildren(Widgets, *n);
    myfree(Widgets);
}

static void SetManage(Widget w, XEvent *event, String *str, Cardinal *n)
{
    Widget *ManageOn, *ManageOff, Here, Root;
    int     m, i, Val;
    const   char *Name;

    m = *n;
    if (m%2)
        WidgetWarning(w, "setmanage() must have an even number of arguments, "
                      "not %d", m);
    else {
        ManageOn = mynews(Widget, m/2);
        WITH_UNWIND {
            ManageOff = mynews(Widget, m/2);
            WITH_UNWIND {
                int Ons, Offs;

                Root = w;
                while (XtIsShell(Root) == False) Root = XtParent(Root);

                Ons = Offs = 0;
                for (i=0; i<m; i+=2) {
                    Name = ResourceStringToString(str[i]);
                    Here = MyNameToWidget(Root, Name);
                    if (Here) {
                        Val = ResourceStringToInt(str[i+1]);
                        if (Val != INT_MIN)
                            if (Val) ManageOn [Ons ++] = Here;
                            else     ManageOff[Offs++] = Here;
                        else WidgetWarning(w, "setmanage() could not convert"
                                             " arg %.100s to %.100s to an"
                                             " integer", str[i+1], Name);
                    } else WidgetWarning(w, "setmanage() could not convert "
                                         "string %.200s to widget", Name);
                }
                SetManagementChildren(ManageOn, Ons, ManageOff, Offs);
            } ON_UNWIND {
                myfree(ManageOff);
            } END_UNWIND;
        } ON_UNWIND {
            myfree(ManageOn);
        } END_UNWIND;
    }
}

static void Unmap(Widget w, XEvent *event, String *str, Cardinal *n)
{
    while (XtIsShell(w) == False) w = XtParent(w);
    XtUnmapWidget(w);
}

static void Map(Widget w, XEvent *event, String *str, Cardinal *n)
{
    Widget pop;

    switch(*n) {
      case 0:
        /* Or map all children */
        WidgetWarning(w, "map() action called without widget name");
        break;
      case 1:
        pop = MyNameToWidget(w, ResourceStringToString(str[0]));
        if (pop) XtMapWidget(pop);
        else     WidgetWarning(w, "map() called on invalid widget name");
        break;
      default:
        WidgetWarning(w, "map() action called with more than one argument");
        break;
    }
}

static void Popdown(Widget w, XEvent *event, String *str, Cardinal *n)
{
    while (XtIsShell(w) == False) w = XtParent(w);
    XtPopdown(w);
}

static void Popup(Widget w, XEvent *event, String *str, Cardinal *n)
{
    Widget      pop, Here;
    const char *Name;

    switch(*n) {
      case 0:
        /* Or popup all children */
        WidgetWarning(w, "popup() action called without widget name");
        break;
      case 2:
        /* Set grabtype here ??  */
        /* intended drop through */
      case 1:
        Name = ResourceStringToString(str[0]);
        for (Here = w; Here; Here = XtParent(Here))
            if ((pop = MyNameToWidget(Here, Name)) != 0) {
                if (XtIsRealized(pop) != False) XtPopup(pop, XtGrabNone);
                else MyRealizeWidget(pop);
                return;
            }
        WidgetWarning(w, "popup() called on invalid widget name %.200s", Name);
        break;
      default:
        WidgetWarning(w, "popup() action called with more than two arguments");
        break;
    }
}

static void PopToggle(Widget w, XEvent *event, String *str, Cardinal *n)
{
    Widget      pop, Here;
    const char *Name;

    switch(*n) {
      case 0:
        /* Or toggle all children */
        WidgetWarning(w, "poptoggle() action called without widget name");
        break;
      case 2:
        /* Set grabtype here ??  */
        /* intended drop through */
      case 1:
        Name = ResourceStringToString(str[0]);
        for (Here = w; Here; Here = XtParent(Here))
            if ((pop = MyNameToWidget(Here, Name)) != 0) {
                if (((ShellWidget) pop)->shell.popped_up == False)
                    if (XtIsRealized(pop) != False) XtPopup(pop, XtGrabNone);
                    else MyRealizeWidget(pop);
                else XtPopdown(pop);
                return;
            }
        WidgetWarning(w, "poptoggle() called on invalid widget name %.200s",
                      Name);
        break;
      default:
        WidgetWarning(w, "poptoggle() action called with more than two "
                      "arguments");
        break;
    }
}

static void SetValues(Widget w, XEvent *event, String *str, Cardinal *n)
{
    Widget      Here, Next;
    int         m;
    XtTypedArg *AvList, *Av;
    const char *Ptr;
    char *Open, *Close;

    if (*n % 2) {
        Here = w;
        Ptr = ResourceStringToString(str[0]);
        if (Ptr && *Ptr) {
            while ((Next = XtParent(Here)) != 0) Here = Next;
            Close = NULL;
            if ((Open = strchr(Ptr, '(')) != NULL &&
                (Close = strchr(Open, ')')) != NULL) {
                Close = mystrndup(Open+1, (size_t) (Close-Open-1));
                Open  = mystrndup(Ptr,    (size_t) (Open-Ptr));
                Here  = MyNameToWidget(Here, Close);
                if (Here) XtVaGetValues(Here, Open, &Here, NULL);
                myfree(Open);
                myfree(Close);
            } else Here = MyNameToWidget(Here, Ptr);
        }
        if (Here) {
            m = *n/2;
            if (m)
                if (m == 1) {
                    Ptr = ResourceStringToString(str[2]);
                    XtVaSetValues(Here, XtVaTypedArg,
                                  ResourceStringToString(str[1]), XtRString,
                                  (XtArgVal) Ptr, strlen(Ptr)+1, NULL);
                } else {
                    AvList = mynews(XtTypedArg, m+1);
                    str++;
                    for (Av = AvList; m>0; m--, Av++) {
                        Av->name  = (String) ResourceStringToString(*str++);
                        Av->type  = XtRString;
                        Ptr = ResourceStringToString(*str++);
                        Av->value = (XtArgVal) Ptr;
                        Av->size  = strlen(Ptr)+1;
                    }
                    Av->name = NULL;
                    XtVaSetValues(Here, XtVaNestedList, AvList, NULL);
                    myfree(AvList);
                }
        } else WidgetWarning(w, "setvalues() action called on unknown widget "
                             "name %.200s", Ptr);
    } else WidgetWarning(w, "setvalues() action called with an"
                         " even number of arguments");
}

static void CreateWitchet(Widget w, XEvent *event, String *str, Cardinal *n)
{
    Widget      Here, Next;
    int         m;
    XtTypedArg *AvList, *Av;
    const char *Ptr;
    const char *Name;
    XtPointer   data;

    if (*n >= 2) {
        if (*n % 2 == 0) {
            Name = ResourceStringToString(str[0]);
            Here = w;
            while ((Next = XtParent(Here)) != NULL) Here = Next;
            if (FindCallback(Here, XtNdestroyCallback, CallFakeChildDestroy,
                             &data)) Here = (Widget) data;
            Here = MyNameToWidget(Here, ResourceStringToString(str[1]));
            if (Here) {
                m = *n/2-1;
                AvList = mynews(XtTypedArg, m+1);
                str+=2;
                for (Av = AvList; m>0; m--, Av++) {
                    Av->name  = (String) ResourceStringToString(*str++);
                    Av->type  = XtRString;
                    Ptr = ResourceStringToString(*str++);
                    Av->value = (XtArgVal) Ptr;
                    Av->size  = strlen(Ptr)+1;
                }
                Av->name = NULL;
                w = MyVaCreateManagedWidget(Name, Here,
                                            XtVaNestedList, AvList, NULL);
                MyRealizeWidget(w);
                myfree(AvList);
            } else WidgetWarning(w, "createwitchet() action called on an"
                                 " unknown widget name");
        } else WidgetWarning(w, "createwitchet() action called with an"
                             " odd number of arguments");
    } else WidgetWarning(w, "createwitchet() action called with less than"
                         " two arguments");
}

typedef const char *constString;

static void System(Widget w, XEvent *event, String *str, Cardinal *n)
{
    pid_t        Id;
    int          m;
    constString *Args, *Argh;

    if (*n) {
        switch(Id = fork()) {
          case 1:
            WidgetWarning(w, "system() failed, could not execute fork()");
            break;
          case 0:               /* Child */
            close(ConnectionNumber(XtDisplay(w)));
            Args = mynews(constString, *n+1);
            for (Argh = Args, m = *n; m>0; m--, Argh++, str++)
                *Argh = ResourceStringToString(*str);
            *Argh = NULL;
            CleanHandlers();
            /* should be: execvp(Args[0], (char * const *) Args),
               but some compilers want const char ** or char ** */
            execvp(Args[0], (void *) Args);
            WidgetWarning(w, "system() failed, could not execute execvp()");
            exit(0);
            break;
          default:              /* Parent */
            break;
        }
    } else WidgetWarning(w, "system() called without arguments");
}

static void Exit(Widget w, XEvent *event, String *str, Cardinal *n)
{
    int rc;

    if (*n > 0) rc = ResourceStringToInt(str[0]);
    else        rc = 0;
    Xexit(XtWidgetToApplicationContext(w), rc);
}

static void Quit(Widget w, XEvent *event, String *str, Cardinal *n)
{
    int rc;

    if (*n > 0) rc = ResourceStringToInt(str[0]);
    else        rc = 0;
    Xquit(XtWidgetToApplicationContext(w), INT_TO_XTPOINTER(rc));
}

static void Mallocstats(Widget w, XEvent *event, String *str, Cardinal *n)
{
    mallocstats();
}

static void Destroy(Widget w, XEvent *event, String *str, Cardinal *n)
{
    Widget Here;
    int    m;

    if (*n)
        for (m= *n; m>0; m--) {
            while (XtIsShell(w) == False) w = XtParent(w);

            Here = MyNameToWidget(w, ResourceStringToString(*str++));
            if (Here) {
                /* XtUnrealizeWidget(Here); */
                XtDestroyWidget(Here);
            } else WidgetWarning(w, "destroy() refers to invalid widget name");
        }
    else {
        /* XtUnrealizeWidget(w); */
        XtDestroyWidget(w);
    }
}

static void Echo(Widget w, XEvent *evnt, String *str, Cardinal *n)
{
    Cardinal i;

    if (*n) fputs(ResourceStringToString(str[0]), stdout);
    for (i=1; i<*n; i++) {
        putc(' ', stdout);
        fputs(ResourceStringToString(str[i]), stdout);
    }
    putc('\n', stdout);
    fflush(stdout);
}

Atom GetAtom(Widget w, const char *name)
{
    Atom     a;
    XrmValue src, dst;

    src.size = strlen(name)+1;
    src.addr = (XPointer) name;
    dst.size = sizeof(Atom);
    dst.addr = (XPointer) &a;

    XtConvertAndStore(w, XtRString, &src, XtRAtom, &dst);
    return a;
}

/* Multiplex WM_PROTOCOL events. Is a bit silly when we handle only one type */

static void WMProtocol(Widget w, XEvent *event, String *str, Cardinal *n)
{
    Atom   protocol;
    XEvent myEvent;
    int    rc;

    protocol = (Atom) event->xclient.data.l[0];
    if (event->type == ClientMessage) {
        if (protocol == ProtocolList[0]) { /* WM_DELETE_WINDOW */
            myEvent.xclient.type         = ClientMessage;
            myEvent.xclient.display      = event->xclient.display;
            myEvent.xclient.window       = event->xclient.window;
            myEvent.xclient.message_type = DELETE;
            myEvent.xclient.format       = 8;
            rc = XSendEvent(event->xclient.display, event->xclient.window,
                            False, NoEventMask, &myEvent);
        }
    }
}

static void Change(Widget w, XEvent *evnt, String *str, Cardinal *n)
{
    Widget         Test;

    Test = XtWindowToWidget(XtDisplay(w), ((XButtonEvent *) evnt)->window);
    if (Test) w = Test;
    _ChangeWidget(w);
}

static void Increase(Widget w, XEvent *evnt, String *str, Cardinal *n)
{
    Widget Root, Target;
    long   Val, Inc, Min, Max;
    String Content;
    char   Work[80];

    if (*n > 4){
        WidgetWarning(w, "increase() expect 1 to 4 arguments, not %d",
                      (int) *n);
        return;
    }

    Root = w;
    while (XtIsShell(Root) == False) Root = XtParent(Root);

    if (*n) {
        Target = MyNameToWidget(Root, ResourceStringToString(str[0]));
        if (!Target) {
            WidgetWarning(w, "increase() could not convert '%.200s' to a "
                          "widget", str[0]);
            return;
        }
    } else Target = w;

    if (*n <= 1) Inc = 1;
    else {
        Inc = ResourceStringToLong(str[1]);
        if (Inc == LONG_MIN) {
            WidgetWarning(w, "increase() could not convert '%.200s' to a long",
                          str[1]);
            return;
        }
    }

    if (*n <= 2) Min = LONG_MIN+1;
    else {
        Min = ResourceStringToLong(str[2]);
        if (Min == LONG_MIN) {
            WidgetWarning(w, "increase() could not convert '%.200s' to a long",
                          str[2]);
            return;
        }
    }

    if (*n <= 3) Max = LONG_MAX-1;
    else {
        Max = ResourceStringToLong(str[3]);
        if (Max == LONG_MIN) {
            WidgetWarning(w, "increase() could not convert '%.200s' to a long",
                          str[3]);
            return;
        }
    }

    if (Min > Max) {
        WidgetWarning(w, "increase() has a minimum %ld bigger than "
                      "its maximum %ld", Min, Max);
        return;
    }

    if (XtIsSubclass(Target, textWidgetClass) == False) {
        WidgetWarning(w, "increase() cannot be applied to non text widget "
                      "'%.200s'", str[0]);
        return;
    }

    XtVaGetValues(Target, XtNstring, (XtArgVal) &Content, NULL);
    Val = ResourceStringToLong(Content);
    if (Val == LONG_MIN) {
        WidgetWarning(w, "increase() applied to text widget containing "
                      "'%.200s' which is not a valid number", Content);
        return;
    }
    if (Inc < 0)
        if (Min-Inc > Val) Val  = Max;
        else               Val += Inc;
    else
        if (Max-Inc < Val) Val = Min;
        else               Val += Inc;
    sprintf(Work, "%ld", Val);
    XtVaSetValues(Target, XtNstring, (XtArgVal) Work, NULL);
}

static void ToggleBoolean(Widget w, XEvent *evnt, String *str, Cardinal *n)
{
    Widget   Root, Target;
    String   Content;
    XrmValue src,  dst;

    if (*n > 1) {
        WidgetWarning(w, "toggleboolean() expects at most 1 argument, not %d",
                      (int) *n);
        return;
    }

    Root = w;
    while (XtIsShell(Root) == False) Root = XtParent(Root);

    if (*n) {
        Target = MyNameToWidget(Root, ResourceStringToString(str[0]));
        if (!Target) {
            WidgetWarning(w, "toggleboolean() could not convert '%.200s' "
                          "to a widget", str[0]);
            return;
        }
    } else Target = w;

    XtVaGetValues(Target, XtNstring, (XtArgVal) &Content, NULL);
    src.size = strlen(Content)+1;
    src.addr = (XPointer) Content;
    dst.size = 0;
    dst.addr = NULL;
    
    if (XtConvertAndStore(Target,XtRString,&src,XtRBoolean, &dst) == False) {
        WidgetWarning(w, "toggleboolean() applied to text widget containing "
                      "'%.200s' which is not a valid boolean", Content);
        return;
    }
    
    XtVaSetValues(Target, XtNstring,
                  (XtArgVal) (*(Boolean *) dst.addr == False ? "True":"False"),
                  NULL);
}

static int UnactivateTexts(Widget Root, Widget Current,
                            Widget *Previous, Widget *Next)
{
    Widget          Here, Prev, Nex, First;
    XawTextEditType Edit;
    int             JustFound;
    Boolean         Caret;

    JustFound = 0;
    if (Next)     *Next = NULL;
    if (Previous) *Previous = NULL;
    Prev = Nex = Here = First = NULL;
    for (;;) {
        Here = FindNextWidget(Root, Here, "*Text");
        if (!Here || First == Here) break;
        if (!First) First = Here;
        XtVaGetValues(Here,
                      XtNeditType,     (XtArgVal) &Edit,
                      XtNdisplayCaret, (XtArgVal) &Caret,
                      NULL);
        if (Edit == XawtextRead) continue;

        if (!Nex) Nex = Here;
        if (Here == Current) {
            JustFound = 1;
            if (Previous) *Previous = Prev;
        } else if (JustFound) {
            Nex = Here;
            JustFound = 0;
        }
        Prev = Here;
        if (Caret != False)
            XtVaSetValues(Here, XtNdisplayCaret, (XtArgVal) False, NULL);
    }
    if (Previous && !*Previous) *Previous = Prev;
    if (Next)                   *Next     = Nex;
    return JustFound;
}

static void ActivateText(Widget Root, Widget Text)
{
    Widget  *Children;
    Cardinal NumChildren;

    if (!Text) Raise1(AssertException, "No text widget to activate given");
    XtVaSetValues(Text, XtNdisplayCaret, (XtArgVal) True, NULL);
    
    XtVaGetValues(Root,
                  XtNchildren,    (XtArgVal) &Children,
                  XtNnumChildren, (XtArgVal) &NumChildren,
                  NULL);
    if (!NumChildren)
        Raise1(AssertException, "Root has no children to activate");
    XtSetKeyboardFocus(Children[NumChildren-1], Text);
}

static void PreviousText(Widget w, XEvent *evnt, String *str, Cardinal *n)
{
    Widget Target, Root, Previous;

    if (*n > 1) {
        WidgetWarning(w, "previoustext() at most 1 argument, not %d",
                      (int) *n);
        return;
    }

    if (*n) {
        /* Currently rather useless, but if MyNameToWidget gets stronger... */
        Target = MyNameToWidget(w, ResourceStringToString(str[0]));
        if (!Target) {
            WidgetWarning(w, "previoustext() could not convert '%.200s' "
                          "to a widget", str[0]);
            return;
        }
    } else Target = w;

    Root = Target;
    while (XtIsShell(Root) == False) Root = XtParent(Root);

    UnactivateTexts(Root, Target, &Previous, NULL);
    if (Previous) ActivateText(Root, Previous);
    else WidgetWarning(w, "previoustext() could not find a valid text widget");
}

static void NextText(Widget w, XEvent *evnt, String *str, Cardinal *n)
{
    Widget Target, Root, Next;

    if (*n > 1) {
        WidgetWarning(w, "nexttext() at most 1 argument, not %d",
                      (int) *n);
        return;
    }

    if (*n) {
        /* Currently rather useless, but if MyNameToWidget gets stronger... */
        Target = MyNameToWidget(w, ResourceStringToString(str[0]));
        if (!Target) {
            WidgetWarning(w, "nexttext() could not convert '%.200s' "
                          "to a widget", str[0]);
            return;
        }
    } else Target = w;

    Root = Target;
    while (XtIsShell(Root) == False) Root = XtParent(Root);

    UnactivateTexts(Root, Target, NULL, &Next);
    if (Next) ActivateText(Root, Next);
    else WidgetWarning(w, "nexttext() could not find a valid text widget");
}

static void ThisText(Widget w, XEvent *evnt, String *str, Cardinal *n)
{
    Widget Target, Root;

    if (*n > 1) {
        WidgetWarning(w, "settext() at most 1 argument, not %d",
                      (int) *n);
        return;
    }

    if (*n) {
        /* Currently rather useless, but if MyNameToWidget gets stronger... */
        Target = MyNameToWidget(w, ResourceStringToString(str[0]));
        if (!Target) {
            WidgetWarning(w, "settext() could not convert '%.200s' "
                          "to a widget", str[0]);
            return;
        }
    } else Target = w;
    
    if (XtIsSubclass(Target, textWidgetClass) == False) {
        WidgetWarning(w, "settext() cannot be applied to non text widget "
                      "'%.200s'", XtName(Target));
        return;
    }

    Root = Target;
    while (XtIsShell(Root) == False) Root = XtParent(Root);

    UnactivateTexts(Root, Target, NULL, NULL);
    ActivateText(Root, Target);
}

static XtActionsRec actionTable[] = {
    { (String) "WMprotocol",    WMProtocol    },
    { (String) "destroy",       Destroy       },
    { (String) "exit",          Exit          },
    { (String) "quit",          Quit          },
    { (String) "mallocstats",   Mallocstats   },
    { (String) "setfocus",      SetFocus      },
    { (String) "map",           Map           },
    { (String) "manage",        Manage        },
    { (String) "unmanage",      Unmanage      },
    { (String) "setmanage",     SetManage     },
    { (String) "unmap",         Unmap         },
    { (String) "popup",         Popup         },
    { (String) "popdown",       Popdown       },
    { (String) "poptoggle",     PopToggle     },
    { (String) "popupmenu",     PopupMenu     },
    { (String) "setvalues",     SetValues     },
    { (String) "createwitchet", CreateWitchet },
    { (String) "system",        System        },
    { (String) "beep",          Beep          },
    { (String) "echo",          Echo          },
    { (String) "info",          Info          },
    { (String) "help",          Help,         },
    { (String) "change",        Change        },
    { (String) "widgetAction",  WidgetAction  },
    { (String) "widgets",       WidgetTree    },
    { (String) "increase",      Increase      },
    { (String) "toggleboolean", ToggleBoolean },
    { (String) "previoustext",  PreviousText  },
    { (String) "nexttext",      NextText      },
    { (String) "thistext",      ThisText      },
};

void InitWMProtocol(Widget top)
{
    XtRegisterGrabAction(PopupMenu, True, ButtonPressMask | ButtonReleaseMask,
			 GrabModeAsync, GrabModeAsync);
    XtAppAddActions(XtWidgetToApplicationContext(top),
                    actionTable, XtNumber(actionTable));
    ProtocolList[0] = GetAtom(top, "WM_DELETE_WINDOW");
    DELETE          = GetAtom(top, "DELETE");
}
