#ifndef _XawSmeLabelP_h
#define _XawSmeLabelP_h

/***********************************************************************
 *
 * Sme Object Private Data
 *
 ***********************************************************************/

#ifdef    XAW3D
# include <X11/Xaw3d/SmeBSBP.h>
#else  /* XAW3D */
# include <X11/Xaw/SmeBSBP.h>
#endif /* XAW3D */
#include "SmeLabel.h"

/************************************************************
 *
 * New fields for the Sme Object class record.
 *
 ************************************************************/

typedef struct _SmeLabelClassPart {
  XtPointer extension;
} SmeLabelClassPart;

/* Full class record declaration */
typedef struct _SmeLabelClassRec {
    RectObjClassPart   rect_class;
    SmeClassPart       sme_class;
#ifdef    XAW3D
    SmeThreeDClassPart sme_threeD_class;
#endif /* XAW3D */
    SmeBSBClassPart    sme_bsb_class;
    SmeLabelClassPart  sme_label_class;
} SmeLabelClassRec;

extern SmeLabelClassRec smeLabelClassRec;

/* New fields for the Sme Object record */
typedef struct {
    /* resources */
/* private resources. */
    XtPointer dummy;
} SmeLabelPart;

/****************************************************************
 *
 * Full instance record declaration
 *
 ****************************************************************/

typedef struct _SmeLabelRec {
  ObjectPart   object;
  RectObjPart  rectangle;
  SmePart      sme;
#ifdef    XAW3D
  SmeThreeDPart      sme_threeD;
#endif /* XAW3D */
  SmeBSBPart   sme_bsb;
  SmeLabelPart sme_label;
} SmeLabelRec;

/************************************************************
 *
 * Private declarations.
 *
 ************************************************************/

#endif /* _XawSmeLabelP_h */
