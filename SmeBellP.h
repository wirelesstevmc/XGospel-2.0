#ifndef _XawSmeBellP_h
#define _XawSmeBellP_h

/***********************************************************************
 *
 * Sme Object Private Data
 *
 ***********************************************************************/

#include <SmeToggleP.h>
#include "SmeBell.h"

/************************************************************
 *
 * New fields for the Sme Object class record.
 *
 ************************************************************/

typedef struct _SmeBellClassPart {
  XtPointer extension;
} SmeBellClassPart;

/* Full class record declaration */
typedef struct _SmeBellClassRec {
    RectObjClassPart   rect_class;
    SmeClassPart       sme_class;
#ifdef    XAW3D
    SmeThreeDClassPart sme_threeD_class;
#endif /* XAW3D */
    SmeBSBClassPart    sme_bsb_class;
    SmeToggleClassPart sme_toggle_class;
    SmeBellClassPart   sme_bell_class;
} SmeBellClassRec;

extern SmeBellClassRec smeBellClassRec;

/* New fields for the Sme Object record */
typedef struct {
    /* resources */
    int percent;
    /* private resources. */
} SmeBellPart;

/****************************************************************
 *
 * Full instance record declaration
 *
 ****************************************************************/

typedef struct _SmeBellRec {
  ObjectPart    object;
  RectObjPart   rectangle;
  SmePart       sme;
#ifdef    XAW3D
  SmeThreeDPart sme_threeD;
#endif /* XAW3D */
  SmeBSBPart    sme_bsb;
  SmeTogglePart sme_toggle;
  SmeBellPart   sme_bell;
} SmeBellRec;

/************************************************************
 *
 * Private declarations.
 *
 ************************************************************/

#endif /* _XawSmeBellP_h */
