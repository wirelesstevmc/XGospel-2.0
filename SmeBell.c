#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/Xaw/XawInit.h>

#include <stdio.h>

#include "SmeBellP.h"

#define offset(field) XtOffsetOf(SmeBellRec, sme_bell.field)
static XtResource resources[] = {
    { (String) XtNpercent, (String) XtCPercent, XtRInt, sizeof(int),
     offset(percent), XtRImmediate, (XtPointer) 20},
};   
#undef offset

#define MINBELL -100
#define MAXBELL  100

/*
 * Semi Public function definitions. 
 */

static void    Initialize(Widget request, Widget new);
static Boolean SetValues(Widget current, Widget request, Widget new);

#define superclass (&smeToggleClassRec)
SmeBellClassRec smeBellClassRec = {
  {
    /* superclass         */    (WidgetClass) superclass,
    /* class_name         */    (String) "SmeBell",
    /* size               */    sizeof(SmeBellRec),
    /* class_initializer  */	XawInitializeWidgetSet,
    /* class_part_initialize*/	NULL,
    /* Class init'ed      */	FALSE,
    /* initialize         */    (XtInitProc) Initialize,
    /* initialize_hook    */	NULL,
    /* realize            */    NULL,
    /* actions            */    NULL,
    /* num_actions        */    0,
    /* resources          */    resources,
    /* resource_count     */	XtNumber(resources),
    /* xrm_class          */    NULLQUARK,
    /* compress_motion    */    FALSE, 
    /* compress_exposure  */    FALSE,
    /* compress_enterleave*/ 	FALSE,
    /* visible_interest   */    FALSE,
    /* destroy            */    NULL,
    /* resize             */    NULL,
    /* expose             */    XtInheritExpose,
    /* set_values         */    (XtSetValuesFunc) SetValues,
    /* set_values_hook    */	NULL,
    /* set_values_almost  */	XtInheritSetValuesAlmost,  
    /* get_values_hook    */	NULL,			
    /* accept_focus       */    NULL,
    /* intrinsics version */	XtVersion,
    /* callback offsets   */    NULL,
    /* tm_table		  */    NULL,
    /* query_geometry	  */    XtInheritQueryGeometry,
    /* display_accelerator*/    NULL,
    /* extension	  */    NULL
  },{
    /* Menu Entry Fields */
      
    /* highlight */             XtInheritHighlight,
    /* unhighlight */           XtInheritUnhighlight,
    /* notify */		XtInheritNotify,
    /* extension	  */    NULL
  }, {
#ifdef    XAW3D
    /* ThreeDClass Fields */
    /* shadowdraw         */    XtInheritXawSme3dShadowDraw
  }, {
#endif /* XAW3D */
    /* BSB Menu entry Fields */  

    /* extension	  */    NULL
  }, {
    /* Toggle Menu entry Fields */  

    /* extension	  */    NULL
  }, {
    /* Bell Menu entry Fields */  

    /* extension	  */    NULL
  }
};

WidgetClass smeBellObjectClass = (WidgetClass) &smeBellClassRec;

/************************************************************
 *
 * Semi-Public Functions.
 *
 ************************************************************/

/* ARGSUSED */
static void
Initialize(Widget request, Widget new)
{
    SmeBellObject entry = (SmeBellObject) new;
    char          Buffer[200];

    if (entry->sme_bell.percent < MINBELL) {
        sprintf(Buffer, "SmeBell widget (%.*s):\n %d percent is too small."
                " Changed to %d.", (int) sizeof(Buffer)-100, XtName(new),
                entry->sme_bell.percent, MINBELL);
        XtAppWarning(XtWidgetToApplicationContext(new), Buffer);
        entry->sme_bell.percent = MINBELL;
    }
    if (entry->sme_bell.percent > MAXBELL) {
        sprintf(Buffer, "SmeBell widget (%.*s):\n %d percent is too big."
                " Changed to %d.", (int) sizeof(Buffer)-100, XtName(new),
                entry->sme_bell.percent, MAXBELL);
        XtAppWarning(XtWidgetToApplicationContext(new), Buffer);
        entry->sme_bell.percent = MAXBELL;
    }
}

/*      Function Name: SetValues
 *      Description: Relayout the menu when one of the resources is changed.
 *      Arguments: current - current state of the widget.
 *                 request - what was requested.
 *                 new - what the widget will become.
 *      Returns: none
 */

/* ARGSUSED */
static Boolean SetValues(Widget current, Widget request, Widget new)
{
    SmeBellObject entry = (SmeBellObject) new;
    char          Buffer[200];

    if (entry->sme_bell.percent < MINBELL) {
        sprintf(Buffer, "SmeBell widget (%.*s):\n %d percent is too small."
                " Changed to %d.", (int) sizeof(Buffer)-100, XtName(new),
                entry->sme_bell.percent, MINBELL);
        XtAppWarning(XtWidgetToApplicationContext(new), Buffer);
        entry->sme_bell.percent = MINBELL;
    }
    if (entry->sme_bell.percent > MAXBELL) {
        sprintf(Buffer, "SmeBell widget (%.*s):\n %d percent is too big."
                " Changed to %d.", (int) sizeof(Buffer)-100, XtName(new),
                entry->sme_bell.percent, MAXBELL);
        XtAppWarning(XtWidgetToApplicationContext(new), Buffer);
        entry->sme_bell.percent = MAXBELL;
    }

    return False;
}

/************************************************************
 *
 * Public Routines
 *
 ************************************************************/
   
/*	Function Name: SmeBell
 *	Description: execute XBell with the given percent if toggled on
 *	Arguments: w - any SmeBell object
 *	Returns: none.
 */

void
#if NeedFunctionPrototypes
SmeBell(Widget w)
#else
SmeBell(w)
Widget w;
#endif
{
    SmeBellObject obj = (SmeBellObject) w;

    if (obj) {
        if (obj->sme_toggle.set != False)
            XBell(XtDisplayOfObject(w), obj->sme_bell.percent);
    } else XtWarning("Called SmeBell(null)");
}
