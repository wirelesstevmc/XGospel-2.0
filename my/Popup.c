/* $XConsortium: Popup.c,v 1.30 91/05/09 18:07:59 swick Exp $ */

/***********************************************************
Copyright 1987, 1988 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/

#include <X11/IntrinsicP.h>
#include <X11/Shell.h>
#include <X11/ShellP.h>
#include <X11/StringDefs.h>

#include "myxlib.h"

#ifdef XtNchangeHook
# define HAS_X_HOOKS
#endif /* XtNchangeHook */

#ifdef HAS_X_HOOKS
typedef struct _HookObjRec *HookObject;
typedef struct _HookObjPart {
    /* resources */
    XtCallbackList createhook_callbacks;
    XtCallbackList changehook_callbacks;
    XtCallbackList confighook_callbacks;
    XtCallbackList geometryhook_callbacks;
    XtCallbackList destroyhook_callbacks;
    WidgetList shells;
    Cardinal num_shells;
    /* private data */
    Cardinal max_shells;
    Screen* screen;
} HookObjPart;

typedef struct _HookObjRec {
    ObjectPart object;
    HookObjPart hooks;
} HookObjRec;
#endif /* HAS_X_HOOKS */

#if NeedFunctionPrototypes
# ifndef _XtBoolean
#  ifndef NeedWidePrototypes
#    define _XtBoolean   int
#  else /* NeedWidePrototypes */
#   if NeedWidePrototypes
#    define _XtBoolean	int
#   else
#    define _XtBoolean	Boolean
#   endif /* NeedWidePrototypes */
#  endif /* NeedWidePrototypes */
# endif /* XtBoolean */
extern void _XtPopup(Widget widget, XtGrabKind grab_kind,
                     _XtBoolean spring_loaded);
void _XtPopup(Widget widget, XtGrabKind grab_kind,
              _XtBoolean spring_loaded)
#else
void _XtPopup(widget, grab_kind, spring_loaded)
    Widget      widget;
    XtGrabKind  grab_kind;
    Boolean     spring_loaded;
#endif
{
    register ShellWidget shell_widget = (ShellWidget) widget;

    if (! XtIsShell(widget)) {
	XtAppErrorMsg(XtWidgetToApplicationContext(widget),
		"invalidClass","xtPopup","XtToolkitError",
                "XtPopup requires a subclass of shellWidgetClass",
                  (String *)NULL, (Cardinal *)NULL);
    }

    if (! shell_widget->shell.popped_up) {
	XtGrabKind call_data = grab_kind;
	XtCallCallbacks(widget, XtNpopupCallback, (XtPointer)&call_data);
	shell_widget->shell.popped_up = TRUE;
	shell_widget->shell.grab_kind = grab_kind;
	shell_widget->shell.spring_loaded = spring_loaded;
	if (shell_widget->shell.create_popup_child_proc != NULL) {
	    (*(shell_widget->shell.create_popup_child_proc))(widget);
	}
	if (grab_kind == XtGrabExclusive)
	    XtAddGrab(widget, TRUE, spring_loaded);
        else if (grab_kind == XtGrabNonexclusive)
	    XtAddGrab(widget, FALSE, spring_loaded);
	XtRealizeWidget(widget);
    } else if (shell_widget->shell.grab_kind != grab_kind) {
        if (shell_widget->shell.grab_kind == XtGrabNone) {
            if (grab_kind == XtGrabExclusive)
                XtAddGrab(widget, TRUE, spring_loaded);
            else if (grab_kind == XtGrabNonexclusive)
                XtAddGrab(widget, FALSE, spring_loaded);
            shell_widget->shell.grab_kind = grab_kind;
        } else if (grab_kind == XtGrabNone) {
            XtRemoveGrab(widget);
            shell_widget->shell.grab_kind = grab_kind;
        } else {
            /* Don't handle exclusive <-> non exclusive yet */
        }
    }
    XMapRaised(XtDisplay(widget), XtWindow(widget));
} /* _XtPopup */

#if NeedFunctionPrototypes
void XtPopup(
    Widget     widget	 /* popup_shell */,
    XtGrabKind grab_kind /* grab_kind */
)
#else  /* NeedFunctionPrototypes */
void XtPopup (widget, grab_kind)
    Widget  widget;
    XtGrabKind grab_kind;
#endif /* NeedFunctionPrototypes */
{
#ifdef HAS_X_HOOKS
    Widget hookobj;
#endif /* HAS_X_HOOKS */

    switch (grab_kind) {

      case XtGrabNone:
      case XtGrabExclusive:
      case XtGrabNonexclusive:
	break;

      default:
	XtAppWarningMsg(
		XtWidgetToApplicationContext(widget),
		"invalidGrabKind","xtPopup","XtToolkitError",
		"grab kind argument has invalid value; XtGrabNone assumed",
		(String *)NULL, (Cardinal *)NULL);
	grab_kind = XtGrabNone;
    }
	
    _XtPopup(widget, grab_kind, FALSE);

#ifdef HAS_X_HOOKS
    hookobj = XtHooksOfDisplay(XtDisplay(widget));
    if (XtHasCallbacks(hookobj, XtNchangeHook) == XtCallbackHasSome) {
	XtChangeHookDataRec call_data;

	call_data.type = XtHpopup;
	call_data.widget = widget;
	call_data.event_data = (XtPointer)grab_kind;
	XtCallCallbackList(hookobj, 
		((HookObject)hookobj)->hooks.changehook_callbacks, 
		(XtPointer)&call_data);
    }
#endif /* HAS_X_HOOKS */
} /* XtPopup */

void XtPopupSpringLoaded (widget)
    Widget widget;
{
    _XtPopup(widget, XtGrabExclusive, True);
}

void XtPopdown(widget)
    Widget  widget;
{
    /* Unmap a shell widget if it is mapped, and remove from grab list */
#ifdef HAS_X_HOOKS
    Widget hookobj;
#endif /* HAS_X_HOOKS */
    ShellWidget shell_widget = (ShellWidget) widget;

    if (! XtIsShell(widget)) {
	XtAppErrorMsg(XtWidgetToApplicationContext(widget),
		"invalidClass","xtPopdown","XtToolkitError",
            "XtPopdown requires a subclass of shellWidgetClass",
              (String *)NULL, (Cardinal *)NULL);
    }

    if (shell_widget->shell.popped_up) {
	XtGrabKind grab_kind = shell_widget->shell.grab_kind;
	XtUnmapWidget(widget);
	XWithdrawWindow(XtDisplay(widget), XtWindow(widget),
			XScreenNumberOfScreen(XtScreen(widget)));
 	if (grab_kind != XtGrabNone) {
	    XtRemoveGrab(widget);
	}
	shell_widget->shell.popped_up = FALSE;
	XtCallCallbacks(widget, XtNpopdownCallback, (XtPointer)&grab_kind);
    }

#ifdef HAS_X_HOOKS
    hookobj = XtHooksOfDisplay(XtDisplay(widget));
    if (XtHasCallbacks(hookobj, XtNchangeHook) == XtCallbackHasSome) {
	XtChangeHookDataRec call_data;

	call_data.type = XtHpopdown;
	call_data.widget = widget;
	XtCallCallbackList(hookobj, 
		((HookObject)hookobj)->hooks.changehook_callbacks, 
		(XtPointer)&call_data);
    }
#endif /* HAS_X_HOOKS */
} /* XtPopdown */

extern void _MyRemoveSpringLoaded(Widget w);
/* Well, at the moment we only remove the grab */
void _MyRemoveSpringLoaded(Widget w)
{
    register ShellWidget shell_widget = (ShellWidget) w;

    if (!XtIsShell(w)) {
	XtAppErrorMsg(XtWidgetToApplicationContext(w),
                      "invalidClass","myRemoveSpringLoaded","MyToolkitError",
                      "myRemoveSpringLoaded requires a subclass of "
                      "shellWidgetClass", (String *)NULL, (Cardinal *)NULL);
    }
    if (shell_widget->shell.popped_up) {
	XtGrabKind grab_kind = shell_widget->shell.grab_kind;
 	if (grab_kind != XtGrabNone) {
	    XtRemoveGrab(w);
            shell_widget->shell.grab_kind = XtGrabNone;
	}
	shell_widget->shell.spring_loaded = False;
    }
}

/* ARGSUSED */
void XtCallbackPopdown(widget, closure, call_data)
    Widget  widget;
    XtPointer closure;
    XtPointer call_data;
{
    register XtPopdownID id = (XtPopdownID) closure;

    XtPopdown(id->shell_widget);
    if (id->enable_widget != NULL) {
	XtSetSensitive(id->enable_widget, TRUE);
    }
} /* XtCallbackPopdown */

Boolean MyIsPopped(Widget object)
{
    Boolean retval;
/*
    WIDGET_TO_APPCON(object);

    LOCK_APP(app);
*/
    while (XtIsShell(object) == False) object = XtParent(object);
    retval = ((ShellWidget) object)->shell.popped_up == FALSE ? False : True;
/*
    UNLOCK_APP(app);
*/
    return retval;
}
