/*
 * SmeLabel.h - Public Header file for SmeLabel object.
 *
 * This is the public header file for the myxlib Label Sme object.
 * It is intended to be used with the simple menu widget.
 *
 * Date:    May 23, 1993
 *
 * By:      Ton Hospel
 *          ton@linux4.kuleuven.ac.be
 */

#ifndef _SmeLabel_h
#define _SmeLabel_h
#ifdef    XAW3D
# include <X11/Xaw3d/SmeBSB.h>
#else  /* XAW3D */
# include <X11/Xaw/SmeBSB.h>
#endif /* XAW3D */

/****************************************************************
 *
 * SmeLabel object
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
 label               Label              String          Name of entry
 leftBitmap          LeftBitmap         Bitmap          None
 leftMargin          HorizontalMargins  Dimension       4
 rightBitmap         RightBitmap        Bitmap          None
 rightMargin         HorizontalMargins  Dimension       4
 sensitive	     Sensitive		Boolean		True
 state               State              Boolean         Off
 vertSpace           VertSpace          int             25
 width		     Width		Dimension	0
 x		     Position		Position	0
 y		     Position		Position	0

*/

typedef struct _SmeLabelClassRec    *SmeLabelObjectClass;
typedef struct _SmeLabelRec         *SmeLabelObject;

extern WidgetClass smeLabelObjectClass;

#endif /* _SmeLabel_h */
