#include "TearofMenP.h"
#include <X11/StringDefs.h>

static void Highlight( Widget w, XEvent * event, String *params, Cardinal *n);
static void TearofDone(Widget w, XEvent * event, String *params, Cardinal *n);
static Boolean SetValues (Widget current, Widget request, Widget new,
                          ArgList args, Cardinal *num_args);

#define offset(field) XtOffsetOf(TearofMenuRec, tearof_menu.field)
static XtResource resources[] = { 
  {(String) XtNtearState, (String) XtCTearState, XtRInt, sizeof(int),
     offset(state), XtRImmediate, (XtPointer) SIMPLEMENUSTATE },
  {(String) XtNxRoot, (String) XtCPosition, XtRPosition, sizeof(Position),
     offset(x_root), XtRImmediate, (XtPointer) 0 },
  {(String) XtNyRoot, (String) XtCPosition, XtRPosition, sizeof(Position),
     offset(y_root), XtRImmediate, (XtPointer) 0 },
};  
#undef offset

static char defaultTranslations[] =
    "<EnterWindow>:     highlight(enter)        \n\
     <LeaveWindow>:     unhighlight()           \n\
     <Motion>:          highlight(motion)       \n\
     <BtnUp>:           TearofDone()";

static XtActionsRec actionsList[] =
{
    { (String) "highlight",  Highlight  },
    { (String) "TearofDone", TearofDone },
};
 
#define superclass (&simpleMenuClassRec)
    
TearofMenuClassRec tearofMenuClassRec = {
  {
    /* superclass         */    (WidgetClass) superclass,
    /* class_name         */    (String) "TearofMenu",
    /* size               */    sizeof(TearofMenuRec),
    /* class_initialize   */	NULL,
    /* class_part_initialize*/	NULL,
    /* Class init'ed      */	FALSE,
    /* initialize         */    NULL,
    /* initialize_hook    */	NULL,
    /* realize            */    XtInheritRealize,
    /* actions            */    actionsList,
    /* num_actions        */    XtNumber(actionsList),
    /* resources          */    resources,
    /* resource_count     */	XtNumber(resources),
    /* xrm_class          */    NULLQUARK,
    /* compress_motion    */    TRUE, 
    /* compress_exposure  */    TRUE,
    /* compress_enterleave*/ 	TRUE,
    /* visible_interest   */    FALSE,
    /* destroy            */    NULL,
    /* resize             */    XtInheritResize,
    /* expose             */    XtInheritExpose,
    /* set_values         */    SetValues,
    /* set_values_hook    */	NULL,
    /* set_values_almost  */	XtInheritSetValuesAlmost,  
    /* get_values_hook    */	NULL,			
    /* accept_focus       */    NULL,
    /* intrinsics version */	XtVersion,
    /* callback offsets   */    NULL,
    /* tm_table		  */    defaultTranslations,
    /* query_geometry	  */    NULL,
    /* display_accelerator*/    NULL,
    /* extension	  */    NULL
  },{
    /* geometry_manager   */    XtInheritRootGeometryManager,
    /* change_managed     */    XtInheritChangeManaged,
    /* insert_child	  */	XtInheritInsertChild,
    /* delete_child	  */	XtInheritDeleteChild,
    /* extension	  */    NULL
  },{
    /* Shell extension	  */    NULL
  },{
    /* Override extension */    NULL
  },{
    /* Simple Menu extension*/  NULL
  },{
    /* Tearof menu extension*/  NULL
  }
};

WidgetClass tearofMenuWidgetClass = (WidgetClass) &tearofMenuClassRec;

static void Highlight(Widget w, XEvent *event, String *params, Cardinal *n)
{
    XtActionList   Actions;
    Cardinal       NrActions;

    TearofMenuWidget tmw = (TearofMenuWidget) w;
    
    switch(tmw->tearof_menu.state) {
      case SIMPLEMENUSTATE:
      case POPUPMENUSTATE:
        XtGetActionList(simpleMenuWidgetClass, &Actions, &NrActions);
        /* We really should look up if offset 0 is highlight --Ton */
        (*Actions[0].proc)(w, event, params, n);
        break;
      case BEINGTEAREDSTATE:
        XtVaSetValues(w,
                      XtNx, (XtArgVal) (((XButtonEvent *) event)->x_root - tmw->tearof_menu.x_root),
                      XtNy, (XtArgVal) (((XButtonEvent *) event)->y_root - tmw->tearof_menu.y_root),
                      NULL);
        break;
    }
}

/* ARGSUSED */
static Boolean SetValues (Widget current, Widget request, Widget new,
                          ArgList args, Cardinal *num_args)
{
    TearofMenuWidget oldtw = (TearofMenuWidget) current;
    TearofMenuWidget tw    = (TearofMenuWidget) new;
/*  TearofMenuWidget rtw   = (TearofMenuWidget) request; */

    if (oldtw->tearof_menu.state != tw->tearof_menu.state) {
        /* What semantics do we want here ??? --Ton */
    }
    return False;
}

static Widget _XtFindPopup(Widget widget, String name)
{
    register Cardinal i;
    register XrmQuark q;
    register Widget w;

    q = XrmStringToQuark(name);

    for (w=widget; w != NULL; w=w->core.parent)
        for (i=0; i<w->core.num_popups; i++)
            if (w->core.popup_list[i]->core.xrm_name == q)
                return w->core.popup_list[i];
    return NULL;
}

extern void _MyRemoveSpringLoaded(Widget w);
static void TearofDone(Widget w, XEvent *event, String *params, Cardinal *n)
{
    Widget          popup_shell;
    XtActionList    Actions;
    Cardinal        NrActions;
    static Cardinal m=0;

    if (*n == 0) popup_shell = w;
    else if (*n == 1) {
        popup_shell = _XtFindPopup(w, params[0]);
        if (!popup_shell) {
            XtAppWarningMsg(XtWidgetToApplicationContext(w),
                            "invalidPopup","tearofDone","MyToolkitError",
                            "Can't find popup widget \"%s\" in TearofDone",
                            params, n);
            return;
        }
    } else {
        XtAppWarningMsg(XtWidgetToApplicationContext(w),
                        "invalidParameters","tearofDone","MyToolkitError",
                        "TearofDone called with num_params != 0 or 1",
                        (String *)NULL, (Cardinal *)NULL);
        return;
    }

    if (XtIsSubclass(popup_shell, tearofMenuWidgetClass) != False) {
        TearofMenuWidget tmw = (TearofMenuWidget) popup_shell;

        switch(tmw->tearof_menu.state) {
          case SIMPLEMENUSTATE:
            XtPopdown(popup_shell);
          case POPUPMENUSTATE:
            XtGetActionList(simpleMenuWidgetClass, &Actions, &NrActions);
            (*Actions[1].proc)(w, event, params, n); /* notify      */
            (*Actions[2].proc)(w, event, params, n); /* unhighlight */
            break;
          case BEINGTEAREDSTATE:
            Highlight(popup_shell, event, NULL, &m);
            XtUngrabPointer(popup_shell, ((XButtonEvent *) event)->time);
            _MyRemoveSpringLoaded(popup_shell);
            XtVaSetValues(popup_shell,
                          XtNoverrideRedirect, (XtArgVal) False, NULL);
            tmw->tearof_menu.state = POPUPMENUSTATE;
            break;
        }
    } else XtPopdown(popup_shell);
}
