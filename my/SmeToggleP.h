#ifndef _XawSmeToggleP_h
#define _XawSmeToggleP_h

/***********************************************************************
 *
 * Sme Object Private Data
 *
 ***********************************************************************/

#ifdef    XAW3D
# include <X11/Xaw3d/SmeBSBP.h>
# include <X11/Xaw3d/ToggleP.h>
#else  /* XAW3D */
# include <X11/Xaw/SmeBSBP.h>
# include <X11/Xaw/ToggleP.h>
#endif /* XAW3D */
#include "SmeToggle.h"

/************************************************************
 *
 * New fields for the Sme Object class record.
 *
 ************************************************************/

typedef struct _SmeToggleClassPart {
  XtPointer extension;
} SmeToggleClassPart;

/* Full class record declaration */
typedef struct _SmeToggleClassRec {
    RectObjClassPart   rect_class;
    SmeClassPart       sme_class;
#ifdef    XAW3D
    SmeThreeDClassPart sme_threeD_class;
#endif /* XAW3D */
    SmeBSBClassPart    sme_bsb_class;
    SmeToggleClassPart sme_toggle_class;
} SmeToggleClassRec;

extern SmeToggleClassRec smeToggleClassRec;

/* New fields for the Sme Object record */
typedef struct {
    /* resources */
    Boolean     set;
    Boolean     incomplete_notify;
    Widget      widget;
    XtPointer   radio_data;
    Pixmap      on_bitmap, off_bitmap; /* bitmaps to show left. */

/* private resources. */
    RadioGroup *radio_group;
} SmeTogglePart;

/****************************************************************
 *
 * Full instance record declaration
 *
 ****************************************************************/

typedef struct _SmeToggleRec {
  ObjectPart         object;
  RectObjPart        rectangle;
  SmePart	     sme;
#ifdef    XAW3D
  SmeThreeDPart      sme_threeD;
#endif /* XAW3D */
  SmeBSBPart         sme_bsb;
  SmeTogglePart      sme_toggle;
} SmeToggleRec;

/************************************************************
 *
 * Private declarations.
 *
 ************************************************************/

#endif /* _XawSmeToggleP_h */
