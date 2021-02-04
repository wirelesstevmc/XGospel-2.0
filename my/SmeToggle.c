#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/Xaw/XawInit.h>

#include <stdio.h>

#include "SmeToggleP.h"

#define INT_TO_XTPOINTER(x) ((XtPointer) (long) (x))

#define offset(field) XtOffsetOf(SmeToggleRec, sme_toggle.field)
static XtResource resources[] = {
    {(String) XtNstate, (String) XtCState, XtRBoolean, sizeof(Boolean), 
     offset(set), XtRString, (XtPointer) "off"},
    {(String) XtNincompleteNotify, (String) XtCIncompleteNotify, XtRBoolean,
         sizeof(Boolean), 
     offset(incomplete_notify), XtRString, (XtPointer) "False"},
    {(String) XtNradioGroup, (String) XtCWidget, XtRWidget, sizeof(Widget), 
     offset(widget), XtRWidget, (XtPointer) NULL },
    {(String) XtNradioData, (String) XtCRadioData, XtRPointer,
         sizeof(XtPointer), 
     offset(radio_data), XtRPointer, (XtPointer) NULL },
    {(String) XtNonBitmap, (String) XtCOnBitmap, XtRBitmap, sizeof(Pixmap),
     offset(on_bitmap), XtRString, (XtPointer) "builtin(CheckMark)" },
    {(String) XtNoffBitmap, (String) XtCOffBitmap, XtRBitmap, sizeof(Pixmap),
     offset(off_bitmap), XtRImmediate, (XtPointer) None},
};   
#undef offset

/*
 * Semi Public function definitions. 
 */

static void ClassInitialize(void), Notify(Widget w), Destroy(Widget w);
static void Initialize(Widget request, Widget new);
static void GetDefaultSize(Widget w, Dimension *width, Dimension *height);
static void GetBitmapInfo(Widget w, Boolean is_left);
static Boolean SetValues(Widget current, Widget request, Widget new);
static RadioGroup *GetRadioGroup(Widget w);
static void CreateRadioGroup(Widget w1, Widget w2);
static void AddToRadioGroup(RadioGroup *group, Widget w);
static void TurnOffRadioSiblings(Widget w);
static void RemoveFromRadioGroup(Widget w);

#define superclass (&smeBSBClassRec)
SmeToggleClassRec smeToggleClassRec = {
  {
    /* superclass         */    (WidgetClass) superclass,
    /* class_name         */    (String) "SmeToggle",
    /* size               */    sizeof(SmeToggleRec),
    /* class_initializer  */	ClassInitialize,
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
    /* destroy            */    Destroy,
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
    /* notify */		Notify,
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
  }
};

WidgetClass smeToggleObjectClass = (WidgetClass) &smeToggleClassRec;

/************************************************************
 *
 * Semi-Public Functions.
 *
 ************************************************************/

/*	Function Name: ClassInitialize
 *	Description: Initializes the SmeToggleObject. 
 *	Arguments: none.
 *	Returns: none.
 */

static void ClassInitialize(void)
{
    static XtConvertArgRec parentCvtArgs[] = {
    { XtWidgetBaseOffset, (XtPointer) XtOffset(WidgetRec *, core.parent),
          sizeof(Widget) },
    };

    XawInitializeWidgetSet();
    XtSetTypeConverter(XtRString, XtRWidget, XmuNewCvtStringToWidget,
                       parentCvtArgs, XtNumber(parentCvtArgs), XtCacheNone,
                       NULL);
}

/*      Function Name: Initialize
 *      Description: Initializes the SmeToggleObject
 *      Arguments: request - the widget requested by the argument list.
 *                 new     - the new widget with both resource and non
 *                           resource values.
 *      Returns: none.
 */

/* ARGSUSED */
static void Initialize(Widget request, Widget new)
{
    SmeToggleObject entry = (SmeToggleObject) new;

    entry->sme_toggle.radio_group = NULL;

    if (entry->sme_toggle.radio_data == NULL)
        entry->sme_toggle.radio_data = (XtPointer) XtName(new);

    if (entry->sme_toggle.widget != NULL)
        if (GetRadioGroup(entry->sme_toggle.widget) == NULL)
            CreateRadioGroup(new, entry->sme_toggle.widget);
        else AddToRadioGroup(GetRadioGroup(entry->sme_toggle.widget), new);
    
    /*
     * If this widget is in a radio group then it may cause another
     * widget to be unset, thus calling the notify proceedure.
     *
     * I want to set the toggle if the user set the state to "On" in 
     * the resource group, reguardless of what my ancestors did.
     */

    if (entry->sme_toggle.set != False) {
        TurnOffRadioSiblings(new);
        entry->sme_bsb.left_bitmap = entry->sme_toggle.on_bitmap;
        if (entry->sme_toggle.incomplete_notify != False)
            XtCallCallbacks(new, XtNcallback,
                            INT_TO_XTPOINTER((int) entry->sme_toggle.set));
    } else entry->sme_bsb.left_bitmap = entry->sme_toggle.off_bitmap;

    GetBitmapInfo(new, (Boolean) True);	 /* Left  Bitmap Info */
    GetBitmapInfo(new, (Boolean) False); /* Right Bitmap Info */
}

/*      Function Name: Destroy
 *      Description: Called at destroy time, cleans up.
 *      Arguments: w - the simple menu widget.
 *      Returns: none.
 */

static void Destroy(Widget w)
{
    SmeToggleObject entry = (SmeToggleObject) w;

    if (entry->sme_toggle.set != False &&
        entry->sme_toggle.incomplete_notify != False) {
        entry->sme_toggle.set = False;
        entry->sme_bsb.left_bitmap = entry->sme_toggle.off_bitmap;
        XtCallCallbacks(w, XtNcallback,
                        INT_TO_XTPOINTER((int) entry->sme_toggle.set));
    }
    RemoveFromRadioGroup(w);
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
    SmeToggleObject entry     = (SmeToggleObject) new;
    SmeToggleObject old_entry = (SmeToggleObject) current;
    Boolean ret_val;

    if (entry->rectangle.sensitive != old_entry->rectangle.sensitive)
         ret_val = True;
    else ret_val = False;

    if (old_entry->sme_toggle.widget != entry->sme_toggle.widget)
        SmeToggleChangeRadioGroup(new, entry->sme_toggle.widget);

    if (old_entry->sme_toggle.set != entry->sme_toggle.set)
        if (entry->sme_toggle.set == False) {
            RadioGroup *group;

            group = GetRadioGroup(new);
            if (group) {
                /* Go to the top of the group. */
                while (group->prev) group = group->prev;
                while (group) {
                    SmeToggleObject local_tog = (SmeToggleObject)group->widget;
                    if (local_tog->sme_toggle.set != False) goto ok;
                    group = group->next;
                }
                /* We seem to be the last one. Refuse to be turned off */
                entry->sme_toggle.set = old_entry->sme_toggle.set;
            } else {
              ok:
                entry->sme_bsb.left_bitmap = entry->sme_toggle.off_bitmap;
            }
        } else {
            TurnOffRadioSiblings(new);
            entry->sme_bsb.left_bitmap = entry->sme_toggle.on_bitmap;
        }

    if (entry->sme_bsb.left_bitmap != old_entry->sme_bsb.left_bitmap) {
	GetBitmapInfo(new, (Boolean) True);
	ret_val = True;
    }

    if (entry->sme_bsb.right_bitmap != old_entry->sme_bsb.right_bitmap) {
	GetBitmapInfo(new, (Boolean) False);
	ret_val = True;
    }

    if (ret_val) {
	GetDefaultSize(new, 
		       &entry->rectangle.width, &entry->rectangle.height);
	entry->sme_bsb.set_values_area_cleared = TRUE;
    }
    return ret_val;
}

/************************************************************
 *
 * Private Functions.
 *
 ************************************************************/


static void Notify(Widget w) 
{
    SmeToggleObject entry = (SmeToggleObject) w;

    if (entry->sme_toggle.set != False)
        XtVaSetValues(w, XtNstate, (XtArgVal) False, NULL);
    else XtVaSetValues(w, XtNstate, (XtArgVal) True,  NULL);
    XtCallCallbacks(w, XtNcallback,
                    INT_TO_XTPOINTER((int) entry->sme_toggle.set));
}

/*	Function Name: GetDefaultSize
 *	Description: Calculates the Default (preferred) size of
 *                   this menu entry.
 *	Arguments: w - the menu entry widget.
 *                 width, height - default sizes (RETURNED).
 *	Returns: none.
 */

#ifndef ONE_HUNDRED
# define ONE_HUNDRED 100
#endif /* ONE_HUNDERD */

static void GetDefaultSize(Widget w, Dimension *width, Dimension *height) 
{
    SmeToggleObject entry = (SmeToggleObject) w;

    if (entry->sme_bsb.label == NULL) *width = 0;
    else *width = XTextWidth(entry->sme_bsb.font, entry->sme_bsb.label,
                             (int) strlen(entry->sme_bsb.label));

    *width += entry->sme_bsb.left_margin + entry->sme_bsb.right_margin;
    
    *height = (entry->sme_bsb.font->max_bounds.ascent +
	       entry->sme_bsb.font->max_bounds.descent);

    *height = ((int) *height * (ONE_HUNDRED + 
                                entry->sme_bsb.vert_space )) / ONE_HUNDRED;
}

/*      Function Name: GetBitmapInfo
 *      Description: Gets the bitmap information from either of the bitmaps.
 *      Arguments: w - the Toggle menu entry widget.
 *                 is_left - TRUE if we are testing left bitmap,
 *                           FALSE if we are testing the right bitmap.
 *      Returns: none
 */

static void GetBitmapInfo(Widget w, Boolean is_left)
{
    SmeToggleObject entry = (SmeToggleObject) w;    
    unsigned int depth, bw;
    Window root;
    int x, y;
    unsigned int width, height;
    char buf[200];
    
    if (is_left != False) {
	if (entry->sme_bsb.left_bitmap != None) {
	    if (!XGetGeometry(XtDisplayOfObject(w), 
			      entry->sme_bsb.left_bitmap, &root, 
			      &x, &y, &width, &height, &bw, &depth)) {
		sprintf(buf, "SmeToggle Object: %s %s \"%.*s\".", "Could not",
			"get Left Bitmap geometry information for menu entry ",
                        (int) sizeof(buf)-90, XtName(w));
		XtAppError(XtWidgetToApplicationContext(w), buf);
	    }
	    if (depth != 1) {
		sprintf(buf, "SmeToggle Object: %s \"%s\"%s.", 
			"Left Bitmap of entry ", 
			XtName(w), " is not one bit deep.");
		XtAppError(XtWidgetToApplicationContext(w), buf);
	    }
	    entry->sme_bsb.left_bitmap_width = (Dimension) width; 
	    entry->sme_bsb.left_bitmap_height = (Dimension) height;
	}
    } else if (entry->sme_bsb.right_bitmap != None) {
	if (!XGetGeometry(XtDisplayOfObject(w),
			  entry->sme_bsb.right_bitmap, &root,
			  &x, &y, &width, &height, &bw, &depth)) {
	    sprintf(buf, "SmeToggle Object: %s %s \"%s\".", "Could not",
		    "get Right Bitmap geometry information for menu entry ",
		    XtName(w));
	    XtAppError(XtWidgetToApplicationContext(w), buf);
	}
	if (depth != 1) {
	    sprintf(buf, "SmeToggle Object: %s \"%s\"%s.", 
		    "Right Bitmap of entry ", XtName(w),
		    " is not one bit deep.");
	    XtAppError(XtWidgetToApplicationContext(w), buf);
	}
	entry->sme_bsb.right_bitmap_width = (Dimension) width; 
	entry->sme_bsb.right_bitmap_height = (Dimension) height;
    }
}

/************************************************************
 *
 * Below are all the private proceedures that handle 
 * radio toggle buttons.
 *
 ************************************************************/

/*	Function Name: GetRadioGroup
 *	Description: Gets the radio group associated with a give sme toggle
 *                   widget.
 *	Arguments: w - the sme toggle widget who's radio group we are getting.
 *	Returns: the radio group associated with this toggle group.
 */

static RadioGroup *GetRadioGroup(Widget w)
{
    SmeToggleObject tw;

    tw = (SmeToggleObject) w;
    if (tw) return tw->sme_toggle.radio_group;
    else    return NULL;
}

/*	Function Name: CreateRadioGroup
 *	Description: Creates a radio group. give two widgets.
 *	Arguments: w1, w2 - the toggle widgets to add to the radio group.
 *	Returns: none
 * 
 *      NOTE:  A pointer to the group is added to each widget's radio_group
 *             field.
 *             If the widget does not refer to a group widget yet, point to
 *             the second argument
 *             If both sets are false, the second widget is toggled.
 */

static void CreateRadioGroup(Widget w1, Widget w2)
{
    char error_buf[200];
    SmeToggleObject tw1 = (SmeToggleObject) w1;
    SmeToggleObject tw2 = (SmeToggleObject) w2;
    
    if (tw1->sme_toggle.radio_group || tw2->sme_toggle.radio_group) {
        sprintf(error_buf, "%s %s", "Sme Toggle Widget Error - Attempting",
                "to create a new toggle group, when one already exists.");
        XtWarning(error_buf);
    }
    
    AddToRadioGroup(NULL, w1);
    AddToRadioGroup(GetRadioGroup(w1), w2);
    if (!tw2->sme_toggle.widget) tw2->sme_toggle.widget = w2;
    if (!tw1->sme_toggle.widget) tw1->sme_toggle.widget = w2;
    if (tw1->sme_toggle.set == False && tw2->sme_toggle.set == False)
        ((SmeToggleObjectClass) w2->core.widget_class)->sme_class.notify(w2);
}

/*	Function Name: AddToRadioGroup
 *	Description: Adds a toggle to the radio group.
 *	Arguments: group - any element of the radio group the we are adding to.
 *                 w - the new toggle widget to add to the group.
 *	Returns: none.
 */

static void AddToRadioGroup(RadioGroup *group, Widget w)
{
    SmeToggleObject tw = (SmeToggleObject) w;
    RadioGroup * local;

    local = (RadioGroup *) XtMalloc(sizeof(RadioGroup));
    local->widget = w;
    tw->sme_toggle.radio_group = local;

    if (!group) {        /* Creating new group. */
        group = local;
        group->next = NULL;
        group->prev = NULL;
        return;
    }
    local->prev = group;        /* Adding to previous group. */
    if ((local->next = group->next) != NULL) local->next->prev = local;
    group->next = local;
}

/*	Function Name: TurnOffRadioSiblings
 *	Description: Deactivates all radio siblings (but not w itself).
 *	Arguments: widget - a toggle widget.
 *	Returns: none.
 */

static void TurnOffRadioSiblings(Widget w)
{
    RadioGroup *group;
    SmeToggleObjectClass class = (SmeToggleObjectClass) w->core.widget_class;

    if ((group = GetRadioGroup(w)) == NULL) /* Punt if there is no group */
        return;

    /* Go to the top of the group. */

    while (group->prev) group = group->prev;

    while (group) {
        SmeToggleObject local_tog = (SmeToggleObject) group->widget;
        if (local_tog->sme_toggle.set != False && w != (Widget) local_tog)
            class->sme_class.notify(group->widget);
        group = group->next;
    }
}

/*	Function Name: RemoveFromRadioGroup
 *	Description: Removes a toggle from a RadioGroup.
 *	Arguments: w - the toggle widget to remove.
 *	Returns: none.
 */

static void RemoveFromRadioGroup(Widget w)
{
    RadioGroup * group = GetRadioGroup(w);

    if (group) {
        if (group->prev != NULL) group->prev->next = group->next;
        if (group->next != NULL) group->next->prev = group->prev;
        XtFree((char *) group);
    }
}

/************************************************************
 *
 * Public Routines
 *
 ************************************************************/
   
/*	Function Name: SmeToggleChangeRadioGroup
 *	Description: Allows a toggle widget to change radio groups.
 *	Arguments: w - The toggle widget to change groups.
 *                 radio_group - any widget in the new group.
 *	Returns: none.
 */

void SmeToggleChangeRadioGroup(Widget w, Widget radio_group)
{
    SmeToggleObject tw = (SmeToggleObject) w;
    RadioGroup *group;

    RemoveFromRadioGroup(w);

    /*
     * If the toggle that we are about to add is set then we will 
     * unset all toggles in the new radio group.
     */

    if (tw->sme_toggle.set && radio_group != NULL )
        SmeToggleUnsetCurrent(radio_group);

    if (radio_group)
        if (NULL == (group = GetRadioGroup(radio_group)))
            CreateRadioGroup(w, radio_group);
        else AddToRadioGroup(group, w);
}

/*	Function Name: SmeToggleGetCurrent
 *	Description: Returns the RadioData associated with the toggle
 *                   widget that is currently active in a toggle group.
 *	Arguments: w - any toggle widget in the toggle group.
 *	Returns: The XtNradioData associated with the toggle widget.
 */

XtPointer SmeToggleGetCurrent(Widget w)
{
    RadioGroup * group;

    if (NULL == (group = GetRadioGroup(w))) return(NULL);
    while (group->prev) group = group->prev;

    while (group) {
        SmeToggleObject local_tog = (SmeToggleObject) group->widget;
        if (local_tog->sme_toggle.set != False)
            return local_tog->sme_toggle.radio_data;
        group = group->next;
    }
    return(NULL);
}

/*	Function Name: SmeToggleSetCurrent
 *	Description: Sets the Toggle widget associated with the
 *                   radio_data specified.
 *	Arguments: radio_group - any toggle widget in the toggle group.
 *                 radio_data - radio data of the toggle widget to set.
 *	Returns: none.
 */

void SmeToggleSetCurrent(Widget radio_group, XtPointer radio_data)
{
    RadioGroup * group;
    SmeToggleObject local_tog; 
    SmeToggleObjectClass class;

    /* Special case case of no radio group. */

    if (NULL == (group = GetRadioGroup(radio_group))) {
        local_tog = (SmeToggleObject) radio_group;
        if (local_tog->sme_toggle.radio_data == radio_data)     
            if (local_tog->sme_toggle.set == False) {
                class = (SmeToggleObjectClass) radio_group->core.widget_class;
                class->sme_class.notify(radio_group);
            }
        return;
    }
    
    /*
     * find top of radio_roup 
     */

    while (group->prev) group = group->prev;

    /*
     * search for matching radio data.
     */

    while (group) {
        local_tog = (SmeToggleObject) group->widget;
        if (local_tog->sme_toggle.radio_data == radio_data) {
            if (!local_tog->sme_toggle.set) { /* if not already set. */
                class = (SmeToggleObjectClass) radio_group->core.widget_class;
                class->sme_class.notify(group->widget);
            }
            return;             /* found it, done */
        }
        group = group->next;
    }
}
 
/*	Function Name: SmeToggleUnsetCurrent
 *	Description: Unsets all Toggles in the radio_group specified.
 *	Arguments: radio_group - any toggle widget in the toggle group.
 *	Returns: none.
 */

void SmeToggleUnsetCurrent(Widget radio_group)
{
    SmeToggleObjectClass class;
    SmeToggleObject local_tog = (SmeToggleObject) radio_group;

    /* Special Case no radio group. */
    if (local_tog->sme_toggle.set != False) {
        class = (SmeToggleObjectClass) radio_group->core.widget_class;
        class->sme_class.notify(radio_group);
    }
    TurnOffRadioSiblings(radio_group);
}
