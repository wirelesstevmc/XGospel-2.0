/* 
 * BoardP.h - Private definitions for Board widget
 * 
 */

#ifndef _GoBoardP_h
#define _GoBoardP_h

/***********************************************************************
 *
 * Go Board Widget Private Data
 *
 ***********************************************************************/

#ifndef XAW3D
# include <X11/Xaw/SimpleP.h>
#else  /* XAW3D */
# include <X11/Xaw3d/ThreeDP.h>
# include <X11/Xaw3d/SimpleP.h>
#endif /* XAW3D */
#include "GoBoard.h"

#define	MAXX	25
#define MAXY    25

typedef struct _BoardClassRec {
    CoreClassPart   core_class;
    SimpleClassPart simple_class;
#ifdef    XAW3D
    ThreeDClassPart threeD_class;
#endif /* XAW3D */
} BoardClassRec;

extern BoardClassRec boardClassRec;

/* for stone drawing code taken from cgoban: */
#define  CGBUTS_NUMWHITE  5
#define  MAX_GREY_LEVELS  256

typedef struct CgbutsPics_struct  {
  Pixmap  stonePixmaps;
  Pixmap  maskBitmaps;
} CgbutsPic;

/* New fields for the Board widget record */
typedef struct {
    /* resources */
    Boolean             debug;
    Boolean             simpleStones;
    Pixel		foreground, white_color, black_color, dame_color;
    String              sound, audio_file;
    XFontStruct        *font;
    XtCallbackList	b_up, b_down, k_down;
    int		        off_time, on_time, line_width, size;
    size_t              sizex, sizey;
    XtPointer           userdata;
    
    /* private state */
    XtIntervalId        Timer;
    int                 Redraw;
    Pixmap		Board;
    GC		        back_gc, fore_gc, black_gc, white_gc,
	                glint_gc, stones_gc, dame_gc;
    BWIntPiece	        pieces[MAXX][MAXY];
    unsigned int        dx, dy;
    int                 ox, oy;
    int                 LastX, LastY, marks_on;

    /* for stone drawing code taken from cgoban: */
    int                 numPics;
    CgbutsPic           *pics;
    unsigned long       colors[MAX_GREY_LEVELS];

} BoardPart;

/****************************************************************
 *
 * Full instance record declaration
 *
 ****************************************************************/

typedef struct _BoardRec {
    CorePart   core;
    SimplePart simple;
#ifdef    XAW3D
    ThreeDPart threeD;
#endif /* XAW3D */
    BoardPart  board;
} BoardRec;

#endif /* _GoBoardP_h */
