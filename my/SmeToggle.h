/*
 * SmeToggle.h - Public Header file for SmeToggle object.
 *
 * This is the public header file for the myxlib Toggle Sme object.
 * It is intended to be used with the simple menu widget.  This object
 * provides bitmap - string - bitmap style entries where the left bitmap
 * can be toggled on or off
 *
 * Date:    May 23, 1993
 *
 * By:      Ton Hospel
 *          ton@linux4.kuleuven.ac.be
 */

#ifndef _SmeToggle_h
#define _SmeToggle_h
#ifdef    XAW3D
# include <X11/Xaw3d/SmeBSB.h>
#else  /* XAW3D */
# include <X11/Xaw/SmeBSB.h>
#endif /* XAW3D */
#include <X11/Xaw/Toggle.h>             /* For XtNstate and stuff */

/****************************************************************
 *
 * SmeToggle object
 *
 ****************************************************************/

/* Toggle Menu Entry Resources:

 Name		     Class		RepType		Default Value
 ----		     -----		-------		-------------
 callback            Callback           Callback        NULL
 destroyCallback     Callback		Pointer		NULL
 font                Font               XFontStruct *   XtDefaultFont
 foreground          Foreground         Pixel           XtDefaultForeground
 height		     Height		Dimension	0
 incompleteNotify    IncompleteNotify   Boolean         False
 label               Label              String          Name of entry
 leftBitmap          LeftBitmap         Bitmap          None
 leftMargin          HorizontalMargins  Dimension       4
 onBitmap            OnBitmap           Bitmap          None
 offBitmap           OffBitmap          Bitmap          None
 radioGroup          RadioGroup         Widget          NULL              +
 radioData           RadioData          Pointer         (caddr_t) Widget  ++
 rightBitmap         RightBitmap        Bitmap          None
 rightMargin         HorizontalMargins  Dimension       4
 sensitive	     Sensitive		Boolean		True
 state               State              Boolean         Off
 vertSpace           VertSpace          int             25
 width		     Width		Dimension	0
 x		     Position		Position	0
 y		     Position		Position	0

+ To use the toggle as a radio toggle button, set this resource to point to
  any other widget in the radio group.

++ This is the data returned from a call to XtToggleGetCurrent, by default
   this is set to the name of toggle widget.

*/

typedef struct _SmeToggleClassRec    *SmeToggleObjectClass;
typedef struct _SmeToggleRec         *SmeToggleObject;

extern WidgetClass smeToggleObjectClass;

#define XtNonBitmap         "onBitmap"
#define XtNoffBitmap        "offBitmap"
#define XtNincompleteNotify "incompleteNotify"

#define XtCOnBitmap   "OnBitmap"
#define XtCOffBitmap  "OffBitmap"
#define XtCIncompleteNotify "IncompleteNotify"

/*	Function Name: SmeToggleChangeRadioGroup
 *	Description: Allows a toggle widget to change radio groups.
 *	Arguments: w - The toggle widget to change groups.
 *                 radio_group - any widget in the new group.
 *	Returns: none.
 */

extern void SmeToggleChangeRadioGroup(Widget w, Widget radio_group);

/*	Function Name: SmeToggleGetCurrent
 *	Description: Returns the RadioData associated with the toggle
 *                   widget that is currently active in a toggle group.
 *	Arguments: w - any toggle widget in the toggle group.
 *	Returns: The XtNradioData associated with the toggle widget.
 */

extern XtPointer SmeToggleGetCurrent(Widget w);

/*	Function Name: SmeToggleSetCurrent
 *	Description: Sets the Toggle widget associated with the
 *                   radio_data specified.
 *	Arguments: radio_group - any toggle widget in the toggle group.
 *                 radio_data - radio data of the toggle widget to set.
 *	Returns: none.
 */

extern void SmeToggleSetCurrent(Widget radio_group, XtPointer radio_data);
 
/*	Function Name: SmeToggleUnsetCurrent
 *	Description: Unsets all Toggles in the radio_group specified.
 *	Arguments: radio_group - any toggle widget in the toggle group.
 *	Returns: none.
 */

extern void SmeToggleUnsetCurrent(Widget radio_group);

#endif /* _SmeToggle_h */
