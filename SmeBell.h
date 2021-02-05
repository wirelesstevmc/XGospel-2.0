/*
 * SmeBell.h - Public Header file for SmeLabel object.
 *
 * This is the public header file for the Bell Sme object.
 * It is intended to be used with the simple menu widget.
 *
 * Date:    May 23, 1993
 *
 * By:      Ton Hospel
 *          ton@linux4.kuleuven.ac.be
 */

#ifndef _SmeBell_h
# define _SmeBell_h
# include <SmeToggle.h>

/****************************************************************
 *
 * SmeBell object
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
 percent             Percent            int             20
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

typedef struct _SmeBellClassRec    *SmeBellObjectClass;
typedef struct _SmeBellRec         *SmeBellObject;

extern WidgetClass smeBellObjectClass;

#define XtNpercent "percent"
#define XtCPercent "Percent"

/*	Function Name: SmeBell
 *	Description: execute XBell with the given percent if toggled on
 *	Arguments: w - any SmeBell object
 *	Returns: none.
 */

extern void SmeBell(
#if NeedFunctionPrototypes
Widget w
#endif
);

#endif /* _SmeBell_h */
