#ifndef _TearofMenu_h
#define _TearofMenu_h

#include <X11/Intrinsic.h>
#include <X11/Xaw/SimpleMenu.h>

/****************************************************************
 *
 * TearofMenu widget
 *
 ****************************************************************/

/* TearofMenu Resources:

 Name		     Class		RepType		Default Value
 ----		     -----		-------		-------------
 background	     Background		Pixel		XtDefaultBackground
 backgroundPixmap    BackgroundPixmap	Pixmap          None
 borderColor	     BorderColor	Pixel		XtDefaultForeground
 borderPixmap	     BorderPixmap	Pixmap		None
 borderWidth	     BorderWidth	Dimension	1
 bottomMargin        VerticalMargins    Dimension       VerticalSpace
 columnWidth         ColumnWidth        Dimension       Width of widest text
 cursor              Cursor             Cursor          None
 destroyCallback     Callback		Pointer		NULL
 height		     Height		Dimension	0
 label               Label              String          NULL (No label)
 labelClass          LabelClass         Pointer         smeBSBObjectClass
 mappedWhenManaged   MappedWhenManaged	Boolean		True
 rowHeight           RowHeight          Dimension       Height of Font
 sensitive	     Sensitive		Boolean		True
 tearofState         TearofState        Int             0
 topMargin           VerticalMargins    Dimension       VerticalSpace
 width		     Width		Dimension	0
 x		     Position		Position	0
 xRoot               Position           Position        0
 y		     Position		Position	0
 yRoot               Position           Position        0
*/

typedef struct _TearofMenuClassRec*	TearofMenuWidgetClass;
typedef struct _TearofMenuRec*		TearofMenuWidget;

extern WidgetClass tearofMenuWidgetClass;

#define XtNtearState "tearState"
#define XtNxRoot     "xRoot"
#define XtNyRoot     "yRoot"
#define XtCTearState "TearState"

enum _TearState {
    SIMPLEMENUSTATE,
    POPUPMENUSTATE,
    BEINGTEAREDSTATE
};

/************************************************************
 *
 * Public Functions.
 *
 ************************************************************/

_XFUNCPROTOBEGIN

_XFUNCPROTOEND

#endif /* _TearofMenu_h */
