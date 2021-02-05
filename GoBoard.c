/*
 * GoBoard.c - Go Board widget
 *
 */

#include <X11/StringDefs.h>
#include <X11/keysymdef.h>
#include <X11/IntrinsicP.h>
#ifndef   XAW3D
# include <X11/Xaw/XawInit.h>
#else  /* XAW3D */
# include <X11/Xaw3d/XawInit.h>
#endif /* XAW3D */
/* #include <X11/Xmu/Converters.h> */
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include <except.h>
#include <mymalloc.h>
#include "GoBoardP.h"

#ifndef RAND_MAX
#  define RAND_MAX 32767
#endif

/****************************************************************
 *
 * Full class record constant
 *
 ****************************************************************/

/* raw Sparc audio: Go stone hitting the board
 * contributed by Nici Schraudolph (nici@cs.ucsd.edu, nic on IGS)
 */
static const unsigned char Sound[] = {
    0xae, 0xa9, 0x56, 0x2b, 0xcf, 0xa8, 0xba, 0x30, 0x2a, 0x3a,
    0x38, 0x35, 0x48, 0xbf, 0xa8, 0xab, 0xec, 0x34, 0xcd, 0xaf,
    0xce, 0x32, 0x26, 0x33, 0xc8, 0xaf, 0xbc, 0x3c, 0x34, 0x43,
    0x4b, 0x4e, 0xec, 0xe5, 0x44, 0x4f, 0xbf, 0xbe, 0xcc, 0xbf,
    0xbc, 0x7b, 0x4a, 0xfe, 0xcf, 0xd6, 0x52, 0x34, 0x31, 0x58,
    0xcf, 0xe0, 0x55, 0xc7, 0xbb, 0xce, 0xd0, 0xb8, 0xb7, 0xe9,
    0x40, 0x3f, 0x4f, 0xc8, 0xae, 0xad, 0xc6, 0x71, 0xd8, 0xe4,
    0x4e, 0x4d, 0x4a, 0x34, 0x2d, 0x3d, 0x7b, 0xfe, 0x4e, 0x4d,
    0x66, 0x61, 0x45, 0x45, 0xd5, 0xe0, 0x3d, 0x47, 0xc8, 0xbd,
    0xc6, 0xc8, 0xb8, 0xbb, 0x66, 0x4d, 0xdf, 0xca, 0xd1, 0xcf,
    0xc4, 0xc3, 0x7d, 0x40, 0x4a, 0xe9, 0x61, 0x3c, 0x43, 0xca,
    0xbb, 0xe6, 0x3d, 0x4d, 0xda, 0x49, 0x37, 0x5f, 0xbd, 0xd3,
    0x3f, 0x49, 0xcd, 0xc2, 0xc3, 0xcd, 0xeb, 0x74, 0xec, 0xe9,
    0x7d, 0x79, 0xd8, 0xc5, 0xbe, 0xc4, 0x5c, 0x34, 0x2f, 0x38,
    0x44, 0x3d, 0x42, 0xdc, 0xc8, 0xd0, 0xc9, 0xb5, 0xaf, 0xbd,
    0x5a, 0x40, 0x6a, 0xbb, 0xbd, 0x73, 0x45, 0x4b, 0x5c, 0xff,
    0xc6, 0xbe, 0x66, 0x35, 0x32, 0x36, 0x45, 0xdd, 0xcd, 0xe9,
    0x4d, 0x49, 0x52, 0xe3, 0xc9, 0xc3, 0xc7, 0xca, 0xc7, 0xbf,
    0xbd, 0xce, 0x61, 0xf9, 0xe5, 0x5c, 0x3f, 0x3f, 0x51, 0xe8,
    0xdf, 0x56, 0x48, 0x50, 0x63, 0x56, 0x5a, 0x5f, 0x69, 0xea,
    0xd0, 0xd7, 0xe0, 0xd6, 0xc9, 0xc8, 0xdb, 0xdf, 0xe2, 0xe9,
    0xd9, 0xce, 0x7f, 0x3b, 0x37, 0x4a, 0x53, 0x3c, 0x3d, 0x6a,
    0xc6, 0xb8, 0xb6, 0xc5, 0x5b, 0x41, 0x4a, 0xd0, 0xc4, 0xcf,
    0xd2, 0xdd, 0x65, 0x5c, 0xd6, 0xc7, 0xd2, 0x55, 0x45, 0x4c,
    0x72, 0xed, 0x62, 0x5c, 0xea, 0xd9, 0x63, 0x4e, 0x4f, 0x56,
    0x50, 0x54, 0xe7, 0xd8, 0xf2, 0x00
};

/* Private Data */
#define offset(field) XtOffsetOf(BoardRec, board.field)
static XtResource resources[] = {
    { (String) XtNdebug, (String) XtCDebug, XtRBoolean, sizeof(Boolean),
          offset(debug), XtRString, (XtPointer) "False"},
    { (String) XtNsimpleStones, (String) XtCSimpleStones, XtRBoolean, sizeof(Boolean),
      offset(simpleStones), XtRString, (XtPointer) "False" },
    { (String) XtNboardSize, (String) XtCBoardSize, XtRInt, sizeof(int),
          offset(size), XtRString, (XtPointer) "19"},
    { (String) XtNforeground, (String) XtCForeground, XtRPixel, sizeof(Pixel),
          offset(foreground), XtRString, (XtPointer) XtDefaultForeground},
    { (String) XtNblackColor, (String) XtCBlackColor, XtRPixel, sizeof(Pixel),
          offset(black_color), XtRString, (XtPointer) "Black"},
    { (String) XtNwhiteColor, (String) XtCWhiteColor, XtRPixel, sizeof(Pixel),
          offset(white_color), XtRString, (XtPointer) "White"},
    { (String) XtNdameColor, (String) XtCDameColor, XtRPixel, sizeof(Pixel),
          offset(dame_color), XtRString, (XtPointer) "Green"},
    { (String) XtNboardFont, (String) XtCFont, XtRFontStruct, sizeof(XFontStruct *),
          offset(font), XtRString, (XtPointer) XtDefaultFont},
    { (String) XtNlineWidth, (String) XtCLineWidth, XtRInt, sizeof(int),
          offset(line_width), XtRString, (XtPointer) "0"},
    { (String) XtNoffTimeout, (String) XtCTimeout, XtRInt, sizeof(int),
          offset(off_time), XtRString, (XtPointer) "500"},
    { (String) XtNonTimeout, (String) XtCTimeout, XtRInt, sizeof(int),
          offset(on_time), XtRString, (XtPointer) "500"},
    { (String) XtNuserData, (String) XtCUserData, XtRPointer, sizeof(XtPointer),
          offset(userdata), XtRPointer, NULL},
    { (String) XtNstoneSound, (String) XtCSound, XtRString, sizeof(String),
          offset(sound), XtRString, (XtPointer) Sound },
    { (String) XtNaudioFile, (String) XtCAudioFile, XtRString, sizeof(String),
          offset(audio_file), XtRString, (XtPointer) NULL}, 
    { (String) XtNbuttonUp, (String) XtCCallback, XtRCallback, sizeof(XtCallbackList),
          offset(b_up), XtRPointer, NULL}, 
    { (String) XtNbuttonDown, (String) XtCCallback, XtRCallback, sizeof(XtCallbackList),
          offset(b_down), XtRPointer, NULL}, 
    { (String) XtNkeyDown, (String) XtCCallback, XtRCallback, sizeof(XtCallbackList),
          offset(k_down), XtRPointer, NULL},
#ifdef    XAW3D
# undef offset /* field */
# define offset(field) XtOffsetOf(RectObjRec, rectangle.field)
    { (String) XtNborderWidth, (String) XtCBorderWidth, XtRDimension, sizeof(Dimension),
          offset(border_width), XtRImmediate, (XtPointer) 1},
#endif /* XAW3D */
};
#undef offset /* field */

static void  Stonesound(Widget w, XEvent *evnt, String *str, Cardinal *n);
static void  CallButtonUp(Widget w, XEvent *evnt, String *str, Cardinal *n);
static void  CallButtonDown(Widget w, XEvent *evnt, String *str, Cardinal *n);
static void  CallKeyDown(Widget w, XEvent *evnt, String *str, Cardinal *n);

static char defaultTranslations[] =
 ":<Btn1Up>:     button_up(0) \n\
  :<Btn2Up>:     button_up(1) \n\
  :<Btn3Up>:     button_up(2) \n\
  :<Btn1Down>:   button_down(0) \n\
  :<Btn2Down>:   button_down(1) \n\
  :<Btn3Down>:   button_down(2) \n\
";

static XtActionsRec actions[] =
{
    { (String) "stonesound",   Stonesound     },
    { (String) "button_up",    CallButtonUp   },
    { (String) "button_down",  CallButtonDown },
    { (String) "key_down",     CallKeyDown    },
};

static void             Initialize(Widget request, Widget New);
static void             Redisplay(Widget w, XExposeEvent *evn, Region region);
static void             Destroy(Widget w), Resize(Widget w);
static Boolean          SetValues(BoardWidget current, BoardWidget request,
                                  BoardWidget new);
static XtGeometryResult QueryGeometry(Widget w, 
                                      XtWidgetGeometry *proposed,
                                      XtWidgetGeometry *answer);

BoardClassRec boardClassRec = {
    {
/* core_class fields */
#ifndef XAW3D
# define SuperClass               ((SimpleWidgetClass)&simpleClassRec)
#else  /* XAW3D */
# define SuperClass               ((ThreeDWidgetClass)&threeDClassRec)
#endif /* XAW3D */
        /* superclass            */  (WidgetClass) SuperClass,
        /* class_name            */  (String) "Board",
        /* widget_size           */  sizeof(BoardRec),
        /* class_initialize      */  XawInitializeWidgetSet,
        /* class_part_initialize */  NULL,
        /* class_inited          */  FALSE,
        /* initialize            */  (XtInitProc) Initialize,
        /* initialize_hook       */  NULL,
        /* realize               */  XtInheritRealize,
        /* actions               */  (XtActionList) actions,
        /* num_actions           */  XtNumber(actions),
        /* resources             */  (XtResourceList) resources,
        /* num_resources         */  XtNumber(resources),
        /* xrm_class             */  NULLQUARK,
        /* compress_motion       */  TRUE,
        /* compress_exposure     */  XtExposeCompressMaximal,
        /* compress_enterleave   */  TRUE,
        /* visible_interest      */  FALSE,
        /* destroy               */  Destroy,
        /* resize                */  Resize,
        /* expose                */  (XtExposeProc) Redisplay,
        /* set_values            */  (XtSetValuesFunc) SetValues,
        /* set_values_hook       */  NULL,
        /* set_values_almost     */  XtInheritSetValuesAlmost,
        /* get_values_hook       */  NULL,
        /* accept_focus          */  NULL,
        /* version               */  XtVersion,
        /* callback_private      */  NULL,
        /* tm_table              */  defaultTranslations,
        /* query_geometry        */  QueryGeometry,
        /* display_accelerator   */  XtInheritDisplayAccelerator,
        /* extension             */  NULL
    },
/* Simple class fields initialization */
    {
        /* change_sensitive      */  XtInheritChangeSensitive
    },
#ifdef  XAW3D
/* threeD class fields initialization */
    {
        /* ignore		 */  0
    },
#endif /* XAW3D */
};

WidgetClass boardWidgetClass = (WidgetClass) &boardClassRec;

/****************************************************************
 *
 * Private Procedures
 *
 ****************************************************************/

static void DrawStone(BoardWidget bw, int x, int y, int color);
static int  DrawNiceStone (BoardWidget bw, int x, int y, int size, int color,
			   int StoneVersion);
static void DrawBoard(BoardWidget bw);
static void GetMeasures(BoardWidget bw);
static void FigureMouseClick(BoardWidget bw, int mx, int my, int *x, int *y);
static void BoardTimeOut(XtPointer bw, XtIntervalId *id);
static void PutMarks(BoardWidget bw, int x, int y, GC gc);
static void PutString(BoardWidget bw, int x, int y,
                      const char *text, int len);
static void SetTimer(BoardWidget bw);
static void MarkOn(BoardWidget bw, int x, int y);
static void MarkOff(BoardWidget bw, int x, int y);

static void Initialize(Widget request, Widget New)
{
    int          i, j;
    XGCValues    values;
    XtGCMask     mask;
    BoardWidget  req, new;
    Display     *dpy;
    Colormap     cmap;
    XColor       color;
    int          colorMask;

    req = (BoardWidget) request;
    new = (BoardWidget) New;

    if (new->board.debug) {
        printf("Initialize\n");
        fflush(stdout);
    }

    req->board.Redraw = 1;
    if (req->core.height <= 0)
        new->core.height = 200; /* a resonable default */

    if (req->core.width <= 0)
        new->core.width = 200;

    new->board.sizex = new->board.sizey = new->board.size;
    if      (new->board.sizex > MAXX) new->board.sizex = MAXX;
    else if (new->board.sizex < 1)    new->board.sizex = 1;
    if      (new->board.sizey > MAXY) new->board.sizey = MAXY;
    else if (new->board.sizey < 1)    new->board.sizey = 1;

    for(i=0; i < new->board.sizex; i++)
        for(j=0; j < new->board.sizey; j++) {
           new->board.pieces[i][j].p = Empty;
           new->board.pieces[i][j].m = NoMark;
	   new->board.pieces[i][j].v = -1;
           new->board.pieces[i][j].l = '\0';
        }

    new->board.fore_gc = new->board.black_gc = new->board.white_gc = 0;
    new->board.glint_gc = new->board.back_gc = new->board.stones_gc = 0;
    new->board.dame_gc = 0;
    new->board.LastX   = new->board.LastY = -1;

    mask = GCLineWidth;
    values.line_width   = new->board.line_width;
    switch (values.tile = new->core.background_pixmap) {
      default:
        mask |= GCTile | GCFillStyle;
        values.fill_style = FillTiled;
        break;
      case ParentRelative:
        /* What should we really do here ? Cause ExposeEvents ? -Ton */
      case CopyFromParent:
      case XtUnspecifiedPixmap:
        mask |= GCForeground;
        values.foreground   = new->core.background_pixel;
        break;
    }
    new->board.back_gc  = XtGetGC((Widget) new, mask, &values);

    mask = GCForeground | GCLineWidth | GCCapStyle | GCFont;
    values.foreground   = new->board.foreground;
    values.line_width   = new->board.line_width;
    values.cap_style    = CapRound;
    values.font         = new->board.font->fid;
    new->board.fore_gc  = XtGetGC((Widget) new, mask, &values);

    mask = GCForeground | GCLineWidth;
    values.foreground   = new->board.black_color;
    values.line_width   = new->board.line_width;
    new->board.black_gc = XtGetGC((Widget) new, mask, &values);
    
    mask = GCForeground | GCLineWidth;
    values.foreground   = new->board.white_color;
    values.line_width   = new->board.line_width;
    new->board.white_gc = XtGetGC((Widget) new, mask, &values);

    mask = GCForeground | GCLineWidth;
    values.foreground   = new->board.dame_color;
    values.line_width   = new->board.line_width;
    new->board.dame_gc = XtGetGC((Widget) new, mask, &values);

    mask = GCForeground | GCLineWidth | GCCapStyle | GCFunction;
    values.foreground   = new->board.black_color ^ new->board.white_color;
    values.line_width   = new->board.line_width;
    values.cap_style    = CapRound;
    values.function     = GXxor;
    new->board.glint_gc = XtGetGC((Widget) new, mask, &values);

    mask = GCFunction | GCPlaneMask | GCLineStyle | GCCapStyle |
	GCJoinStyle | GCFillStyle | GCFillRule | GCGraphicsExposures;
    values.function     = GXcopy;
    values.plane_mask   = AllPlanes;
    values.line_style   = LineSolid;
    values.cap_style    = CapButt;
    values.join_style   = JoinMiter;
    values.fill_style   = FillSolid;
    values.fill_rule    = EvenOddRule;
    values.graphics_exposures = False;
    new->board.stones_gc = XtGetGC((Widget) new, mask, &values);

    SetTimer(new);

    if (!new->board.simpleStones) {
	new->board.numPics = 0;
	new->board.pics    = None;

	/* Allocate colors for the stones. If the screen depth is > 8, allocate
	 * 256 colors, otherwise 16 colors only.
	 */
	dpy   = XtDisplay(New);
	cmap  = DefaultColormap(dpy, DefaultScreen(dpy));
	colorMask = DefaultDepth(dpy, DefaultScreen(dpy)) > 8 ? 0xff : 0xf0;
	/* Set default pixel if XAllocColor fails: */
	color.pixel = BlackPixel(dpy, DefaultScreen(dpy));
	color.flags = DoRed | DoGreen | DoBlue;

	for (i = 0; i < MAX_GREY_LEVELS; i++) {
	    color.red = color.green = color.blue =
		 (65535L * (i & colorMask) + 127) / 255;
	    (void)XAllocColor(dpy, cmap, &color);
	    new->board.colors[i] = color.pixel;
	}
    }
}

#define AddStone(Outline, Glint, x, y, Col)     \
do {                                            \
    (Outline)->x  = oox + (x) * dx;             \
    (Outline)->y  = ooy + (y) * dy;             \
    (Outline)->width  = xedge;                  \
    (Outline)->height = yedge;                  \
    (Outline)->angle1 = 0;                      \
    (Outline)->angle2 = 360 * 64;               \
                                                \
    (Glint)->x      = (Outline)->x + xglint;    \
    (Glint)->y      = (Outline)->y + yglint;    \
    (Glint)->width  = xedge - 2*xglint;         \
    (Glint)->height = yedge - 2*yglint;         \
    (Glint)->angle1 = Col == White ? -15 * 64 : (180-15) * 64; \
    (Glint)->angle2 = -60 * 64;                 \
} while(0);

#define OFF(w,l)        (((w)/2)+(((l)&1)?0:((w)&1)))

static const char  BoardLetters[MAXX] = { 'A', 'B', 'C', 'D', 'E',
                                          'F', 'G', 'H', 'J', 'K',
                                          'L', 'M', 'N', 'O', 'P',
                                          'Q', 'R', 'S', 'T', 'U',
                                          'V', 'W', 'X', 'Y', 'Z' };
static const char *BoardNumbers[MAXY] = { " 1", " 2", " 3", " 4", " 5",
                                          " 6", " 7", " 8", " 9", "10",
                                          "11", "12", "13", "14", "15",
                                          "16", "17", "18", "19", "20",
                                          "21", "22", "23", "24", "25" };
static XArc Arcs[  MAXX*MAXY], *Arc,   *OldArc;
static XArc Glints[MAXX*MAXY], *Glint, *OldGlint;

static void RedrawArea(BoardWidget bw, int XMin, int YMin,
                       unsigned int Width, unsigned int Height)
{
    int        ox, oy, dx, dy, xmin, xmax, ymin, ymax, oxmax, oymax, oox, ooy;
    int        x, y, sizex, sizey, domark, lwidth, xedge, yedge;
    int        xglint, yglint;
    BWIntPiece (*pieces)[MAXY];
    XSegment   Segments[MAXX+MAXY+4], *Segment;
    XArc       Pts[9], *Pt;
/* This exposed a bug in the AIX370 metaware compiler
    XArc       Arcs[  MAXX*MAXY], *Arc,   *OldArc;
    XArc       Glints[MAXX*MAXY], *Glint, *OldGlint;
    */
    Display   *dpy;
    Window     win;

    dpy = XtDisplay((Widget) bw);
    win = XtWindow( (Widget) bw);

    GetMeasures(bw);
    pieces = bw->board.pieces;
    ox     = bw->board.ox;
    oy     = bw->board.oy;
    dx     = bw->board.dx;
    dy     = bw->board.dy;
    sizex  = bw->board.sizex;
    sizey  = bw->board.sizey;
    lwidth = bw->board.line_width;
    if (lwidth == 0) lwidth = 1;
    oxmax  = ox + (sizex-1) * dx;
    oymax  = oy + (sizey-1) * dy;

    xmin = XMin;
    xmax = Width = XMin+Width-1 /*+ 1 */;
    ymin = YMin;
    ymax = Height = Height+YMin-1 /*+ 1 */;
    
    if (xmin < ox)    xmin = ox;
    if (ymin < oy)    ymin = oy;
    if (xmax > oxmax) xmax = oxmax;
    if (ymax > oymax) ymax = oymax;

    Segment = Segments;

    if (ymax >= ymin && xmax >= xmin) {
        if (dx > 0)
            for(x = ox + ((xmin-ox+dx-1)/dx)*dx; x <= xmax; x+=dx, Segment++) {
                Segment->x1 = Segment->x2 = x;
                Segment->y1 = ymin;
                Segment->y2 = ymax;
            }
        if (dy > 0)
            for(y = oy + ((ymin-oy+dy-1)/dy)*dy; y <= ymax; y+=dy, Segment++) {
                Segment->y1 = Segment->y2 = y;
                Segment->x1 = xmin;
                Segment->x2 = xmax;
            }
    }

    xmin = XMin;
    xmax = Width;
    ymin = YMin;
    ymax = Height;
    xedge = dx*(sizex+4) / 184;
    yedge = dy*(sizey+4) / 184;
    
    if (xmin < ox-xedge)    xmin = ox-xedge;
    if (ymin < oy-yedge)    ymin = oy-yedge;
    if (xmax > oxmax+xedge) xmax = oxmax+xedge;
    if (ymax > oymax+yedge) ymax = oymax+yedge;

    if (ymax >= ymin && xmax >= xmin) {
        if (xmin == ox-xedge) {
            Segment->x1 = Segment->x2 = xmin;
            Segment->y1 = ymin;
            Segment->y2 = ymax;
            Segment++;
        }
        if (xmax == oxmax+xedge) {
            Segment->x1 = Segment->x2 = oxmax+xedge;
            Segment->y1 = ymin;
            Segment->y2 = ymax;
            Segment++;
        }
        if (ymin == oy-yedge) {
            Segment->y1 = Segment->y2 = ymin;
            Segment->x1 = xmin;
            Segment->x2 = xmax;
            Segment++;
        }
        if (ymax == oymax+yedge) {
            Segment->y1 = Segment->y2 = oymax+yedge;
            Segment->x1 = xmin;
            Segment->x2 = xmax;
            Segment++;
        }
    }
            
    ox -= OFF(dx,lwidth);
    oy -= OFF(dy,lwidth);
    oox = ox;
    ooy = oy;

    domark = 0;
    if (XMin < ox) {
        for (y=0; y<sizey; y++) PutString(bw, -1, y,  BoardNumbers[y], 2);
        domark = 1;
    }
    if (Height > oy + bw->board.sizey * dy) {
        for (x=0; x<sizex; x++) PutString(bw, x, -1, &BoardLetters[x], 1);
        domark = 1;
    }
    if (domark)
        PutMarks(bw, bw->board.LastX, bw->board.LastY, bw->board.fore_gc);

    xmin = XMin-ox;
    xmax = Width+dx-ox;
    ymin = YMin-oy;
    ymax = Height+dy-oy;
    xedge = dx - lwidth;
    yedge = dy - lwidth;
    xglint = (dx+lwidth+3)/6;
    yglint = (dy+lwidth+3)/6;

    if (dx > 0) {
        xmin /= dx;
        xmax /= dx;
    } else xmin = xmax = 0;
    if (dy > 0) {
        ymin /= dy;
        ymax /= dy;
    } else ymin = ymax = 0;

    if (xmin < 0) xmin = 0;
    if (ymin < 0) ymin = 0;
    if (xmax > sizex) xmax = sizex;
    if (ymax > sizey) ymax = sizey;

    x = bw->board.LastX;
    y = bw->board.LastY;
    domark = xmin <= x && x < xmax && ymin <= sizey-y-1 && sizey-y-1 < ymax &&
        pieces[x][y].p != Empty && pieces[x][y].m != NoMark;
    Arc   = Arcs;
    Glint = Glints;
    if (bw->board.simpleStones) {
	for (x=xmin; x<xmax; x++)
	    for (y=ymin; y<ymax; y++)
		if (pieces[x][sizey-y-1].p == White) {
		    AddStone(Arc, Glint ,x, y, White);
		    if (xedge  >= 0 && yedge  >= 0) {
			Arc++;
			if (xedge >= 2*xglint && yedge >= 2*yglint) Glint++;
		    }
		}
    }
    OldArc   = Arc;
    OldGlint = Glint;
    if (bw->board.simpleStones) {
	for (x=xmin; x<xmax; x++)
	    for (y=ymin; y<ymax; y++)
		if (pieces[x][sizey-y-1].p == Black) {
		    AddStone(Arc, Glint ,x, y, Black);
		    if (xedge  >= 0 && yedge  >= 0) {
			Arc++;
			if (xedge >= 2*xglint && yedge >= 2*yglint) Glint++;
		    }
		}
    }

    Pt = Pts;
    ox += OFF(dx,lwidth)-dx/6;
    oy += OFF(dy,lwidth)-dy/6;
    if (sizex == 19 && sizey == 19) {
	/* Draw the hoshi points */
        for(x=3; x<=15; x+=6)
            if (xmin <= x && x < xmax)
                for(y=3; y<=15; y+=6)
                    if (ymin <= y && y < ymax &&
                        pieces[x][sizey-y-1].p == Empty) {
                        Pt->x = ox  + x * dx;
                        Pt->y = oy  + y * dy;
                        Pt->width   = (dx/6)*2;
                        Pt->height  = (dy/6)*2;
                        Pt->angle1  = 0;
                        Pt->angle2  = 64*360;
                        Pt++;
                    }
    }

    XDrawSegments(dpy, win, bw->board.fore_gc, Segments, Segment-Segments);
    if (bw->board.simpleStones) {
	XFillArcs(dpy, win, bw->board.white_gc,   Arcs, OldArc  -   Arcs);
	XFillArcs(dpy, win, bw->board.black_gc, OldArc,     Arc  -OldArc);
    }
    XFillArcs(dpy, win, bw->board.fore_gc,     Pts,     Pt  -   Pts);
    if (bw->board.simpleStones) {
	XDrawArcs(dpy, win, bw->board.fore_gc,    Arcs,    Arc  -   Arcs);
	XDrawArcs(dpy, win, bw->board.glint_gc, Glints,   Glint - Glints);
    };
    for (x=xmin; x<xmax; x++) {
	for (y=ymin; y<ymax; y++) {
	    BWPiece piece = pieces[x][sizey-y-1].p;
	    if (!bw->board.simpleStones && (piece == White || piece == Black)){
		pieces[x][sizey-y-1].v = 
		     DrawNiceStone(bw, oox+x*dx, ooy+y*dy, dx, 
				   piece, pieces[x][sizey-y-1].v);
	    }
	    if (piece == Empty && pieces[x][sizey-y-1].m != NoMark) {
		MarkOn(bw, x, sizey-y-1);
	    }
	}
    }
    if (domark) MarkOn(bw, bw->board.LastX, bw->board.LastY);
}

#define bw ((BoardWidget) w)
static void Redisplay(Widget w, XExposeEvent *event, Region region)
{
    if (SuperClass->core_class.expose)
        (*SuperClass->core_class.expose)(w, (XEvent *) event, region);

    if (bw->board.debug) {
        printf("Redisplay(Board, %d, %d, %d, %d, %s)\n",
               event->x, event->y, event->width, event->height,
               region ? "region" : "NULL");
        fflush(stdout);
    }
    RedrawArea(bw, event->x, event->y,
               (unsigned int) event->width, (unsigned int) event->height);
}

static void Resize(Widget w)
{
    if (bw->board.debug) {
        printf("Resize\n");
        fflush(stdout);
    }
    GetMeasures(bw);
}

static XtGeometryResult QueryGeometry(Widget w, 
                                      XtWidgetGeometry *proposed,
                                      XtWidgetGeometry *answer)
{
    int width, height, size;

    if (bw->board.debug) {
        printf("QueryGeometry\n");
        fflush(stdout);
    }

    if ((proposed->request_mode & (CWWidth | CWHeight)) == 0) 
        return XtGeometryYes;

    if      ((proposed->request_mode & CWWidth)  == 0)  
        width = height = proposed->height;
    else if ((proposed->request_mode & CWHeight) == 0) 
        width = height = proposed->width;
    else {
        height = proposed->height;
        width  = proposed->width;
    }

    size = (width+height) / 2;
    answer->width  = size;
    answer->height = size;
    answer->request_mode = CWWidth | CWHeight;

    if (answer->width == bw->core.width && answer->height == bw->core.height)
        return XtGeometryNo;
    return XtGeometryAlmost;
}

static void Destroy(Widget w)
{
    if (bw->board.debug) {
        printf("Destroy\n");
        fflush(stdout);
    }

    if (bw->board.Timer) XtRemoveTimeOut(bw->board.Timer);
    if (bw->board.back_gc)  XtReleaseGC((Widget) bw, bw->board.back_gc);
    if (bw->board.fore_gc)  XtReleaseGC((Widget) bw, bw->board.fore_gc);
    if (bw->board.black_gc) XtReleaseGC((Widget) bw, bw->board.black_gc);
    if (bw->board.white_gc) XtReleaseGC((Widget) bw, bw->board.white_gc);
    if (bw->board.glint_gc) XtReleaseGC((Widget) bw, bw->board.glint_gc);
    if (bw->board.stones_gc)XtReleaseGC((Widget) bw, bw->board.stones_gc);
}

static Boolean SetValues(BoardWidget current,
                         BoardWidget request,
                         BoardWidget new)
{
    XGCValues values;
    XtGCMask  mask;
    int       i, j, NewSize, ReDisplay;

    ReDisplay = False;

    if (request->board.debug) {
        printf("SetValues\n");
        fflush(stdout);
    }

    if (current->board.debug != request->board.debug)
        new->board.debug = request->board.debug;

    if (current->board.userdata != request->board.userdata)
        new->board.userdata = request->board.userdata;

    if (current->board.off_time != request->board.off_time ||
        current->board.on_time  != request->board.on_time) {
        new->board.off_time = request->board.off_time;
        new->board.on_time  = request->board.on_time;
        if (current->board.Timer) XtRemoveTimeOut(current->board.Timer);
        SetTimer(new);
        if (current->board.marks_on != new->board.marks_on &&
            current->board.LastX >= 0 && current->board.LastY >= 0) {
            if (new->board.marks_on) {
		current->board.marks_on = TRUE;
		MarkOn(current, current->board.LastX, current->board.LastY);
		current->board.marks_on = FALSE; /* restore for safety */
            } else {
                MarkOff(current, current->board.LastX, current->board.LastY);
	    }
	}
    }
        
    NewSize = 0;

    if (current->board.sizex != request->board.sizex) {
        if (request->board.sizex > MAXX) {
            XtWarning("GoBoard: board width too big");
            new->board.sizex = MAXX;
        } else if (request->board.sizex < 1) {
            XtWarning("GoBoard: board width too small");
            new->board.sizex = 1;
        } else new->board.sizex = request->board.sizex;
        NewSize = 1;
        ReDisplay = True;
    }
        
    if (current->board.sizey != request->board.sizey) {
        if (request->board.sizey > MAXY) {
            XtWarning("GoBoard: board height too big");
            new->board.sizey = MAXY;
        } else if (request->board.sizey < 1) {
            XtWarning("GoBoard: board height too small");
            new->board.sizey = 1;
        } else new->board.sizey = request->board.sizey;
        NewSize = 1;
        ReDisplay = True;
    }

    if (NewSize)
        for(i=0 ; i < new->board.sizex; i++) {
            for(j=0 ; j < new->board.sizey; j++) {
                new->board.pieces[i][j].p = Empty;
                new->board.pieces[i][j].m = NoMark;
		new->board.pieces[i][j].v = -1;  /* stone version */
                new->board.pieces[i][j].l = '\0';
            }
        }

    if (current->board.foreground       != request->board.foreground       ||
        current->board.font->fid        != request->board.font->fid        ||
        current->board.black_color      != request->board.black_color      ||
        current->board.white_color      != request->board.white_color      ||
        current->board.line_width       != request->board.line_width       ||
        current->core.background_pixel  != request->core.background_pixel  ||
        current->core.background_pixmap != request->core.background_pixmap) {
        if (current->board.back_gc)
            XtReleaseGC((Widget) current, current->board.back_gc);
        if (current->board.fore_gc)
            XtReleaseGC((Widget) current, current->board.fore_gc);
        if (current->board.black_gc)  
            XtReleaseGC((Widget) current, current->board.black_gc);
        if (current->board.white_gc) 
            XtReleaseGC((Widget) current, current->board.white_gc);
        if (current->board.glint_gc) 
            XtReleaseGC((Widget) current, current->board.glint_gc);
        if (current->board.stones_gc) 
            XtReleaseGC((Widget) current, current->board.stones_gc);

        new->board.foreground  = request->board.foreground;
        new->board.line_width  = request->board.line_width;
        new->board.black_color = request->board.black_color;
        new->board.white_color = request->board.white_color;

        mask = GCLineWidth;
        values.line_width   = new->board.line_width;
        switch (values.tile = new->core.background_pixmap) {
          default:
            mask |= GCTile | GCFillStyle;
            values.fill_style = FillTiled;
            break;
          case ParentRelative:
            /* What should we really do here ? Cause ExposeEvents ? -Ton */
          case CopyFromParent:
          case XtUnspecifiedPixmap:
            mask |= GCForeground;
            values.foreground   = new->core.background_pixel;
            break;
        }
        new->board.back_gc  = XtGetGC((Widget) new, mask, &values);

        mask = GCForeground | GCLineWidth | GCCapStyle | GCFont;
        values.foreground   = new->board.foreground;
        values.line_width   = new->board.line_width;
        values.cap_style    = CapRound;
        values.font         = new->board.font->fid;
        new->board.fore_gc  = XtGetGC((Widget) new, mask, &values);

        mask = GCForeground | GCLineWidth;
        values.foreground   = new->board.black_color;
        values.line_width   = new->board.line_width;
        new->board.black_gc = XtGetGC((Widget) new, mask, &values);
    
        mask = GCForeground | GCLineWidth;
        values.foreground   = new->board.white_color;
        values.line_width   = new->board.line_width;
        new->board.white_gc = XtGetGC((Widget) new, mask, &values);

        mask = GCForeground | GCLineWidth | GCCapStyle | GCFunction;
        values.foreground   = new->board.black_color ^ new->board.white_color;
        values.line_width   = new->board.line_width;
        values.cap_style    = CapRound;
        values.function     = GXxor;
        new->board.glint_gc = XtGetGC((Widget) new, mask, &values);

	mask = GCFunction | GCPlaneMask | GCLineStyle | GCCapStyle |
	    GCJoinStyle | GCFillStyle | GCFillRule | GCGraphicsExposures;
	values.function     = GXcopy;
	values.plane_mask   = AllPlanes;
	values.line_style   = LineSolid;
	values.cap_style    = CapButt;
	values.join_style   = JoinMiter;
	values.fill_style   = FillSolid;
	values.fill_rule    = EvenOddRule;
	values.graphics_exposures = False;
	new->board.stones_gc = XtGetGC((Widget) new, mask, &values);

        ReDisplay = True;
    }
    return ReDisplay;
}

static void Stonesound(Widget w, XEvent *evnt, String *str, Cardinal *n)
{
    StoneSound(w);
}

static void CallButtonUp(Widget w, XEvent *evnt, String *str, Cardinal *n)
{
    GoButton Button;
    int x, y;

    FigureMouseClick(bw, ((XButtonEvent *)evnt)->x, ((XButtonEvent *)evnt)->y,
                     &x, &y);
    Button.x   = x;
    Button.y   = y;
    Button.str = str;
    Button.params = *n;
    Button.time = ((XButtonEvent *)evnt)->time;
    XtCallCallbacks(w, XtNbuttonUp, &Button);
}

static void CallButtonDown(Widget w, XEvent *evnt, String *str, Cardinal *n)
{
    GoButton Button;
    int x, y;

    FigureMouseClick(bw, ((XButtonEvent *)evnt)->x, ((XButtonEvent *)evnt)->y,
                     &x, &y);
    Button.x      = x;
    Button.y      = y;
    Button.str    = str;
    Button.params = *n;
    Button.time = ((XButtonEvent *)evnt)->time;
    XtCallCallbacks(w, XtNbuttonDown, &Button);
}

static void CallKeyDown(Widget w, XEvent *evnt, String *str, Cardinal *n)
{
    XtCallCallbacks(w, XtNkeyDown, evnt);
}
#undef bw

/***************************************************************************/

static void PutMarks(BoardWidget bw, int x, int y, GC gc)
/* Display the arrows showing where last move was played, or erase
 * them if gc is the background context.
 */
{
    XSegment     Segments[4], *Segment;
    int          dx, dy, ox, oy, sizex, sizey;
    unsigned int width, height;

    sizex = bw->board.sizex;
    sizey = bw->board.sizey;

    if (0 <= x && x < sizex && 0 <= y && y < sizey &&
        XtIsRealized((Widget) bw) != False && bw->board.Redraw == 0) {
        dx  = bw->board.dx;
        dy  = bw->board.dy;
        ox  = bw->board.ox;
        oy  = bw->board.oy + (sizey-1) * dy;
        Segment = Segments;
            
        height = dy / 4;
        Segment[0].x1 = Segment[1].x1 = ox-dx-3*dx/4;
        Segment[0].x2 = Segment[1].x2 = Segment[0].x1 - height;
        Segment[0].y1 = Segment[1].y1 = oy-y*dy;
        height /= 2;
        Segment[0].y2 = Segment[0].y1-height;
        Segment[1].y2 = Segment[0].y1+height;
        Segment += 2;

        height = dx / 4;
        Segment[0].y1 = Segment[1].y1 = oy+dy+dy/2;
        Segment[0].y2 = Segment[1].y2 = Segment[0].y1 + height;
        Segment[0].x1 = Segment[1].x1 = ox+x*dx;
        height /= 2;
        Segment[0].x2 = Segment[0].x1+height;
        Segment[1].x2 = Segment[0].x1-height;
        Segment += 2;

        if (gc == bw->board.back_gc) {
            width = bw->board.line_width;
            if (width == 0) width = 1;
            height = dy / 4;
            XFillRectangle(XtDisplay((Widget) bw), XtWindow((Widget) bw), gc,
                           Segments[0].x2-(int)width/2,
                           Segments[0].y2-(int)width/2,
                           height+width+1, (height|1)+width);
            height = dx / 4;
            XFillRectangle(XtDisplay((Widget) bw), XtWindow((Widget) bw), gc,
                           Segments[3].x2-(int)width/2,
                           Segments[2].y1-(int)width/2,
                           (height|1)+width, height+width+1);
        } else XDrawSegments(XtDisplay((Widget) bw), XtWindow((Widget) bw), gc,
                             Segments, Segment-Segments);
    }
}

static void PutString(BoardWidget bw, int x, int y,
                      const char *text, int len)
{
    int         width, height, dx, dy, direction, ascent, descent;
    XCharStruct overall;
    Display    *dpy;
    Window      win;

    if (XtIsRealized((Widget) bw) != False) {
        dpy = XtDisplay((Widget) bw);
        win = XtWindow( (Widget) bw);

        XTextExtents(bw->board.font, (char *) text, len,
                     &direction, &ascent, &descent, &overall);
        width  = overall.width;
        height = overall.ascent /* - overall.descent */ + 1;
/*        height = ascent-descent; */

        dx = bw->board.dx;
        dy = bw->board.dy;
        x  = bw->board.ox + x * dx - width / 2;
        y  = bw->board.oy + (bw->board.sizey-y-1) * dy + height / 2;
        XDrawString(dpy, win, bw->board.fore_gc, x, y, (char *) text, len);
    }
}

static void RefreshArea(BoardWidget bw, int x, int y,
                        unsigned int width, unsigned int height)
{
    XFillRectangle(XtDisplay((Widget) bw), XtWindow((Widget) bw),
                   bw->board.back_gc, x, y, width, height);
    RedrawArea(bw, x, y, width, height);
}

static void MarkOn(BoardWidget bw, int x, int y)
/* Draws the mark associated to this point. If *simpleStones is true, a
 * circle mark is drawn as a cross. A circle mark is drawn only if
 * marks_on is true, otherwise the mark is delayed to the next timeout.
 * IN assertion: x,y is within the board.
 */
{
    Display    *dpy;
    Window      win;
    GC          gc;
    int         dx, dy, Dx, Dy;
    BWPiece     col;
    BWMark      mark;
    int         off;

    if (XtIsRealized((Widget) bw) != False && bw->board.Redraw == 0) {
        dpy = XtDisplay((Widget) bw);
        win = XtWindow( (Widget) bw);
	col = bw->board.pieces[x][y].p;
	mark = bw->board.pieces[x][y].m;
        dx  = bw->board.dx; Dx = 2*dx / 11;
        dy  = bw->board.dy; Dy = 2*dy / 11;
        x   = bw->board.ox + x * dx;
        y   = bw->board.oy + (bw->board.sizey-y-1) * dy;

	if (mark >= DameMark) {
	    if (mark == DameMark) {
		gc = bw->board.dame_gc;
	    } else if (col == Black || mark == WhiteMark) {
		gc = bw->board.white_gc;
	    } else {
		gc = bw->board.black_gc;
	    }
	    if (mark == SquareMark) {
		XDrawRectangle(dpy, win, gc, x-Dx, y-Dy, 2*Dx, 2*Dy);
	    } else {
		XFillRectangle(dpy, win, gc, x-Dx, y-Dy, 2*Dx, 2*Dy);
	    }

        /* Following is for CircleMark only */

	} else if (!bw->board.marks_on) {
	    return; /* the mark will be displayed at next timeout */

	} else if (bw->board.simpleStones) {
	    gc  = (col == Empty) ? bw->board.fore_gc : bw->board.glint_gc;
	    XDrawLine(dpy, win, gc, x-Dx, y-Dy, x+Dx, y+Dy);
	    XDrawLine(dpy, win, gc, x-Dx, y+Dy, x+Dx, y-Dy);
	} else {
	    gc  = (col == Black) ? bw->board.white_gc : bw->board.black_gc;
	    off = (dx + 2) / 4;
	    XDrawArc(dpy, win, gc, x - off - 1, y - off - 1,
		     2*off + 1, 2*off + 1, 0, 360*64);
	}
    }
}

static void MarkOff(BoardWidget bw, int x, int y)
{
    Display     *dpy;
    Window       win;
    BWPiece      col;
    unsigned int dx, dy, Dx, Dy, width;

    if (XtIsRealized((Widget) bw) != False && bw->board.Redraw == 0) {
        col = bw->board.pieces[x][y].p;


	if (!bw->board.simpleStones) {
	    DrawStone(bw, x, y, col);
	    /* this would also work for simpleStones true, should test speed */
        } else {
	    dx  = bw->board.dx; Dx = 2*dx / 11;
	    dy  = bw->board.dy; Dy = 2*dy / 11;
	    x   = bw->board.ox + x * dx;
	    y   = bw->board.oy + (bw->board.sizey-y-1) * dy;

	    if (col == Empty) {
		width = bw->board.line_width;
		if (width == 0) width = 1;
		Dx += width;
		Dy += width;
		RefreshArea(bw, x-(int)Dx, y-(int)Dy, 2*Dx+1, 2*Dy+1);
	    } else {
		/* Assume here that the mark was a cross (other marks are
                 * on empty points only)
                 */
		dpy =  XtDisplay((Widget) bw);
		win =  XtWindow( (Widget) bw);
		XDrawLine(dpy, win, bw->board.glint_gc,
			  x-(int)Dx, y-(int)Dy, x+(int)Dx, y+(int)Dy);
		XDrawLine(dpy, win, bw->board.glint_gc,
			  x-(int)Dx, y+(int)Dy, x+(int)Dx, y-(int)Dy);
	    }
	}
    }
}

static void GetMeasures(BoardWidget bw)
{
    Dimension  sw, cw, ch;
    unsigned int awidth, aheight, smallest, sizex, sizey;

#ifndef   XAW3D
    sw = 0;
#else  /* XAW3D */
    sw = 2*bw->threeD.shadow_width;
#endif /* XAW3D */
    cw = bw->core.width;
    ch = bw->core.height;

    if (cw < ch) smallest = cw;
    else         smallest = ch;

    if (smallest > sw) smallest -= sw;
    else               smallest  = 0;

    sizex = 2*bw->board.sizex + 3;
    sizey = 2*bw->board.sizey + 3;

    bw->board.dx = 2*smallest / sizex;
    bw->board.dy = 2*smallest / sizey;

    awidth  = bw->board.dx * sizex / 2;
    aheight = bw->board.dy * sizey / 2;
     
    bw->board.ox = (cw - awidth  + 4 * bw->board.dx) / 2;
    bw->board.oy = (ch - aheight +     bw->board.dy) / 2;
}

static void FigureMouseClick(BoardWidget bw, int mx, int my, int *x, int *y)
{
    *x = (mx + bw->board.dx/2 - bw->board.ox) / bw->board.dx;
    *y = (my + bw->board.dy/2 - bw->board.oy) / bw->board.dy;
    *y = bw->board.sizey - 1 - *y;
}

static void DrawBoard(BoardWidget bw)
{
    Display *dpy;
    Window   win;
    int      dx, dy;

    dpy = XtDisplay((Widget) bw);
    win = XtWindow( (Widget) bw);
    dx = bw->board.dx;
    dy = bw->board.dy;

    XClearArea(dpy, win, bw->board.ox-dx/2, bw->board.oy-dy/2,
               bw->board.dx * bw->board.sizex,
               bw->board.dy * bw->board.sizey, 
               True);
}

static void DrawStone(BoardWidget bw, int x, int y, int color)
{
    Display     *dpy;
    Window       win;
    int          ox, oy, oox, ooy, xedge, yedge, xglint, yglint, yorig;
    unsigned int dx, dy, lwidth;
    XArc         arc, glint;

    if (XtIsRealized((Widget) bw) != False) {
        dpy = XtDisplay((Widget) bw);
        win = XtWindow( (Widget) bw);

        lwidth = bw->board.line_width;
        if (lwidth == 0) lwidth = 1;
        dx     = bw->board.dx;
        dy     = bw->board.dy;
        ox     = bw->board.ox - OFF(dx,lwidth);
        oy     = bw->board.oy - OFF(dy,lwidth);
	yorig  = y;
        y      = bw->board.sizey-y-1;

	if (bw->board.simpleStones) {

	    oox    = ox + lwidth/2;
	    ooy    = oy + lwidth/2;
	    xedge = dx - lwidth;
	    yedge = dy - lwidth;
	    xglint = (dx+lwidth+3)/6;
	    yglint = (dy+lwidth+3)/6;

	    switch(color) {
	      case White:
		AddStone(&arc, &glint, x, y, White);
		if (xedge >= 0 && yedge >=0) {
		    XFillArcs(dpy, win, bw->board.white_gc, &arc,   1);
		    XDrawArcs(dpy, win, bw->board.fore_gc,  &arc,   1);
		    if (xedge >= 2*xglint && yedge >= 2*yglint)
			XDrawArcs(dpy, win, bw->board.glint_gc, &glint, 1);
		}
		break;
	      case Black:
		AddStone(&arc, &glint, x, y, Black);
		if (xedge >= 0 && yedge >=0) {
		    XFillArcs(dpy, win, bw->board.black_gc, &arc,   1);
		    XDrawArcs(dpy, win, bw->board.fore_gc,  &arc,   1);
		    if (xedge >= 2*xglint && yedge >= 2*yglint)
			XDrawArcs(dpy, win, bw->board.glint_gc, &glint, 1);
		}
		break;
	      default:
		/* XClearArea(dpy, win, ox+x*dx, oy+y*dy, dx, dy, True); */
		RefreshArea(bw, ox+x*(int)dx, oy+y*(int)dy, dx, dy);
		break;
	    }
        } else if (color == White || color == Black) {
	     bw->board.pieces[x][yorig].v =
	       DrawNiceStone(bw, ox+x*(int)dx, oy+y*(int)dy, dx, color,
			     bw->board.pieces[x][yorig].v);
	} else {
            RefreshArea(bw, ox+x*(int)dx, oy+y*(int)dy, dx, dy);
	}
    }
}

#define bw ((BoardWidget) w)
int PutPiece(Widget w, int x, int y, BWPiece piece)
{
    BWMark mark;

    if (x < 0 || x >= bw->board.sizex ||
        y < 0 || y >= bw->board.sizey) {
      return 0;
    }
    if (bw->board.pieces[x][y].p != piece) {
	bw->board.pieces[x][y].p = piece;
	if (!bw->board.Redraw) {
	    DrawStone(bw, x, y, piece);
	    mark = bw->board.pieces[x][y].m;
	    /* No circle mark on an empty point: */
	    if (mark > CircleMark || (mark == CircleMark && piece != Empty)) {
		MarkOn(bw, x, y);
	    }
	}
    }
    return 1;
}

BWPiece GetPiece(Widget w, int x, int y)
{
    return bw->board.pieces[x][y].p;
}

void LastMove(Widget w, int x, int y)
/* Mark the last move with a circle (or a cross if *simpleNames is true).
 * If x,y is outside the board, just erase the mark on the previous last move.
 */
{
    int lastX = bw->board.LastX;
    int lastY = bw->board.LastY;

    if (lastX != x || lastY != y) {

	/* If the last move was a click on a dead stone, the point is now
         * empty and a dame or territory mark should be kept intact.
	 */
	if (lastX >= 0 && lastY >= 0 &&
	    bw->board.pieces[lastX][lastY].m <= CircleMark) {

	    SetMark(w, bw->board.LastX, bw->board.LastY, NoMark);
	}
        SetMark(w, x, y, CircleMark);
    
        PutMarks(bw, bw->board.LastX, bw->board.LastY, bw->board.back_gc);
        PutMarks(bw, x, y, bw->board.fore_gc);

        bw->board.LastX = x;
        bw->board.LastY = y;
    } else {
        SetMark(w, x, y, CircleMark);
    }
}

void SetMark(Widget w, int x, int y, BWMark m)
/* Set or clear a mark. If m is the circle mark and blinking is on but
 * we are in a "mark off" period, the mark will be displayed only at the
 * end of the period.
 */
{
    BWIntPiece *Piece;

    if (0 <= x && x < bw->board.sizex &&
        0 <= y && y < bw->board.sizey) {
        Piece = &bw->board.pieces[x][y];
        if (Piece->m != m) {
            Piece->m = m;
            if (!bw->board.Redraw) {
		if (m == NoMark) {
		    MarkOff(bw, x, y);
		} else if (m != CircleMark || bw->board.marks_on) {
		    MarkOn(bw, x, y);
                }
	    }  
        }
    }
}

void OpenBoard(Widget w)
{
    /* Just make Sure */
    /* bw->board.Redraw = XtIsRealized(w) == False; */
}

void CloseBoard(Widget w)
{
    if (bw->board.Redraw) {
        bw->board.Redraw = 0;
        if (XtIsRealized(w) != False) DrawBoard(bw);
    }
}

static void SetTimer(BoardWidget w)
{
    XtAppContext context;

    w->board.Timer = 0;
    context = XtWidgetToApplicationContext((Widget) w);
    if (w->board.off_time) {
        w->board.marks_on = 0;
        if (w->board.on_time) 
            w->board.Timer =
                XtAppAddTimeOut(context, (unsigned long) w->board.off_time,
                                BoardTimeOut, w);
    } else if (w->board.on_time) w->board.marks_on = 1;
    else {
        XtWarning("GoBoard: on and off time both 0, using 500");
        w->board.off_time = 500;
        w->board.on_time  = 500;
        w->board.marks_on = 0;
        w->board.Timer =
            XtAppAddTimeOut(context, (unsigned long) w->board.off_time,
                            BoardTimeOut, w);
    }
}

static void BoardTimeOut(XtPointer ClientData, XtIntervalId *id)
{
    XtAppContext context;
    Widget       w;

    w = (Widget) ClientData;
    context = XtWidgetToApplicationContext(w);
    if (bw->board.marks_on) {
        bw->board.marks_on = 0;
        bw->board.Timer =
            XtAppAddTimeOut(context, (unsigned long) bw->board.off_time,
                            BoardTimeOut, w);
        if (bw->board.LastX >= 0 && bw->board.LastY >= 0)
            MarkOff( bw, bw->board.LastX, bw->board.LastY);
    } else {
        bw->board.marks_on = 1;
        bw->board.Timer =
            XtAppAddTimeOut(context, (unsigned long) bw->board.on_time,
                            BoardTimeOut, w);
        if (bw->board.LastX >= 0 && bw->board.LastY >= 0)
            MarkOn( bw, bw->board.LastX, bw->board.LastY);
    }
}

char **AllocBoard(size_t sizeX, size_t sizeY)
{
    char **Board, **B;
    int    j;

    Board = AllocMatrix(char, sizeY, sizeX);
    for (j=sizeY, B=Board; j>0; j--, B++) memset(*B, Empty, sizeX);
    return Board;
}

void FreeBoard(char **Board, size_t sizeY)
{
    FreeMatrix(Board, sizeY);
}

void GetBoard(Widget w, char **Board)
{
    int         i, j, MaxX, MaxY;
    char       *b;
    BWIntPiece (*pieces)[MAXY];
    
    MaxX   = bw->board.sizex;
    MaxY   = bw->board.sizey;
    pieces = bw->board.pieces;

    for (j=0; j<MaxY; j++, Board++)
        for (i=0, b= *Board; i<MaxX; i++, b++) *b = pieces[i][j].p;
}

void SetBoard(Widget w, char **Board)
{
    int         i, j, MaxX, MaxY;
    char       *b;
    
    MaxX   = bw->board.sizex;
    MaxY   = bw->board.sizey;

    for (j=0; j<MaxY; j++, Board++)
        for (i=0, b= *Board; i<MaxX; i++, b++) 
            PutPiece(w, i, j, (BWPiece)*b);
}

void StoneSound(Widget w)
{
    FILE *fp;
    String sound, audioFile;

    sound = ((BoardWidget)w)->board.sound;
    audioFile = ((BoardWidget)w)->board.audio_file;
    if (sound && audioFile) {
        fp = fopen(audioFile, "w");
        if (fp) {
            fputs(((BoardWidget)w)->board.sound, fp);
            fclose(fp);
        } else {
            fprintf(stderr, "Could not open %s for write: %s\n",
                    audioFile, strerrno());
            XBell(XtDisplay(w), 40);
        }
    } else XBell(XtDisplay(w), 20);
}

#undef bw

/* ============================================================================
 * The following code was adapted from cgoban with permission of its author
 * William Shubert. See functions drawStone_newPics and cgbuts_drawp
 * in cgoban drawStone.c and cgbuts.c respectively.
 */

#define  DRAWSTONE_NUMWHITE  10

#ifndef M_PI
#  define M_PI 3.14159265358979323846
#endif

typedef struct WhiteDesc_struct  {
  float  cosTheta, sinTheta;
  float  stripeWidth, xAdd;
  float  stripeMul, zMul;
} WhiteDesc;


static void    decideAppearance(WhiteDesc *desc, int size);
static float   calcCosAngleReflection2View(int x, int y, int r,
					  float *lambertian, float *z);
static XImage *butEnv_imageCreate(BoardWidget bw, int w, int h);
static void    butEnv_imageDestroy(XImage *img);


void  drawStone_newPics(BoardWidget bw,
			Pixmap *stonePixmap, Pixmap *maskBitmap, int size)
{
  XImage  *stones;
  Display *dpy = XtDisplay((Widget) bw);
  unsigned char  *maskData;
  int  maxRadius;
  int  maskW;
  int  x, y, i, wBright, bBright;
  XGCValues  values;
  float  bright, lambertian, z;
  float  wStripeLoc, wStripeColor;
  WhiteDesc  white[DRAWSTONE_NUMWHITE];

  stones = butEnv_imageCreate(bw, size * (DRAWSTONE_NUMWHITE + 1), size);
  maskW = (size * 6 + 7) >> 3;
  maskData = mymalloc(maskW * size);
  memset(maskData, 0, maskW * size);
  for (i = 0;  i < DRAWSTONE_NUMWHITE;  ++i)  {
    decideAppearance(&white[i], size);
  }
  maxRadius = size * size;

  for (y = 0;  y < size;  ++y)  {
    for (x = 0;  x < size;  ++x)  {
      /*
       * Here we build the masks.  We need three basic masks; the solid
       *   circular mask, and two dithered masks used for semitransparent
       *   stones.
       */
      if (((size - 1) - (x+x)) * ((size - 1) - (x+x)) +
	  ((size - 1) - (y+y)) * ((size - 1) - (y+y)) <= maxRadius)  {
	maskData[(x >> 3) + (y * maskW)] |= (1 << (x & 7));
	x += size;
	if ((x & 1) == (y & 1))
	  maskData[(x >> 3) + (y * maskW)] |= (1 << (x & 7));
	x += size;
	if ((x & 1) == (y & 1))
	  maskData[(x >> 3) + (y * maskW)] |= (1 << (x & 7));
	x -= size*2;

	/*
	 * Now we do the actual stone.
	 * All right, time to add some color.  First we calculate the
	 *   cosine of the angle of reflection to the angle of the viewer.
	 *   This is the basic quantity used to calculate the amount of
	 *   light seen in phong shading.  You raise this value to a
	 *   power; the higher the power, the "shinier" the object you are
	 *   rendering.  For the black stones, which aren't very shiny,
	 *   I use bright^4.  For the white stones, which are very shiny,
	 *   I use bright^32.  Then you multiply this value by the
	 *   intensity of the object at the point of rendering, add a
	 *   little bit of lambertian reflection and a tiny bit of ambient
	 *   light, and you're done!
	 */
	bright = calcCosAngleReflection2View((size - 1) - (x+x),
					     (size - 1) - (y + y),
					     size+size, &lambertian, &z);
	bright *= bright;
	bright *= bright;
	bBright = bright*165.0 + lambertian*10.0 - 5.0;
	bright *= bright;
	bright *= bright;
	bright *= bright;
	if (bBright > 255)
	  bBright = 255;
	if (bBright < 0)
	  bBright = 0;
	XPutPixel(stones, x, y, bw->board.colors[bBright]);

	/*
	 * OK, the black stones are done.  Now for the white stones.
	 * Here we have to add the stripes.  The algorithm for stripe
	 *   intensity is just something I made up.  I kept tweaking
	 *   parameters and screwing around with it until it looked sort
	 *   of like my stones.  The stripes are too regular, some day I
	 *   may go back and change that, but for now it is acceptable
	 *   looking IMHO.
	 */
	for (i = 0;  i < DRAWSTONE_NUMWHITE;  ++i)  {
	  wStripeLoc = (x*white[i].cosTheta - y*white[i].sinTheta) +
	    white[i].xAdd;
	  wStripeColor = fmod(wStripeLoc + (z * z * z * white[i].zMul) *
			      white[i].stripeWidth,
			      white[i].stripeWidth) / white[i].stripeWidth;
	  wStripeColor = wStripeColor * white[i].stripeMul - 0.5;
	  if (wStripeColor < 0.0)
	    wStripeColor = -2.0 * wStripeColor;
	  if (wStripeColor > 1.0)
	    wStripeColor = 1.0;
	  wStripeColor = wStripeColor * 0.15 + 0.85;
	  wBright = bright*bright*250.0 +
	    wStripeColor * (lambertian*120.0 + 110.0);
	  if (wBright > 255)
	    wBright = 255;
	  if (wBright < 0)
	    wBright = 0;
	  XPutPixel(stones, x+(i+1)*size, y, bw->board.colors[wBright]);
	}
      }
    }
  }
  *stonePixmap =
    XCreatePixmap(dpy, RootWindow(dpy, DefaultScreen(dpy)),
		  size * (DRAWSTONE_NUMWHITE + 1),
		  size, DefaultDepth(dpy, DefaultScreen(dpy)));
  XPutImage(dpy, *stonePixmap, bw->board.stones_gc, stones, 0,0, 0,0,
	    size * (DRAWSTONE_NUMWHITE + 1), size);
  butEnv_imageDestroy(stones);
  *maskBitmap =
    XCreateBitmapFromData(dpy, RootWindow(dpy, DefaultScreen(dpy)),
			  (void *)maskData, maskW<<3, size);
  /*
   * The "void *" above is because maskData should be a uchar, but the
   *   function prototype specifies a char.
   */
  myfree(maskData);
}


/*
 * Viewing vector is simply "z".
 * I model the stones as the top part of the sphere with the viewer looking
 *   straight down on them.  lx*i+ly*j+lz*k is the angle of incident light.
 *   If you draw out the vectors and work out the equation, you'll notice that
 *   it very nicely simplifies down to what you get here.  The lambertian
 *   intensity (lambertian) and the z magnitude of the surface normal (z)
 *   aren't really related to the cosine being calculated, but they share
 *   many operations with it so it saves CPU time to calculate them at the
 *   same time.
 */
static float  calcCosAngleReflection2View(int x, int y, int r,
					  float *lambertian, float *z)  {
  const float  lx = 0.35355339, ly = 0.35355339, lz = 0.8660254;
  float  nx, ny, nz, rz;
  float  nDotL;

  nz = sqrt((double)(r * r - x * x - y * y));
  *z = 1.0 - (nz / r);
  nx = (float)x;
  ny = (float)y;
  nDotL = (nx*lx + ny*ly + nz*lz) / r;
  rz = (2.0 * nz * nDotL) / r - lz;
  *lambertian = nDotL;
  return(rz);
}

static float  rnd_float()
   /* Return a random float between 0.0 and 1.0 */
{
   return rand()/(float)RAND_MAX;
}

static void  decideAppearance(WhiteDesc *desc, int size)  {
  double  minStripeW, maxStripeW, theta;

  minStripeW = (float)size / 20.0;
  if (minStripeW < 2.5)
    minStripeW = 2.5;
  maxStripeW = (float)size / 5.0;
  if (maxStripeW < 4.0)
    maxStripeW = 4.0;
  theta = rnd_float() * 2.0 * M_PI;
  desc->cosTheta = cos(theta);
  desc->sinTheta = sin(theta);
  desc->stripeWidth = minStripeW +
    (rnd_float() * (maxStripeW - minStripeW));
  desc->xAdd = rnd_float() * desc->stripeWidth +
    (float)size * 3;  /* Make sure that all x's are positive! */
  desc->stripeMul = rnd_float() * 4.0 + 1.5;
  desc->zMul = rnd_float() * 650.0 + 70.0;
}

static XImage  *butEnv_imageCreate(BoardWidget bw, int w, int h)
{
  Display *dpy = XtDisplay((Widget) bw);
  XImage  *image;

  image = XCreateImage(dpy, DefaultVisual(dpy, DefaultScreen(dpy)),
		       DefaultDepth(dpy, DefaultScreen(dpy)),
		       ZPixmap, 0, NULL, w, h, 32, 0);
  image->data = mymalloc(image->bytes_per_line * h);
  return(image);
}

static void  butEnv_imageDestroy(XImage *img)
{
  myfree(img->data);
  img->data = NULL;
  XDestroyImage(img);
}

/* ============================================================================
 * DrawNiceStone is derived from cgbuts_drawp in cgbuts.c
 * A new random stone is created if saved_version < 0.
 */

static void  grid_morePixmaps(BoardWidget bw, int newNumPixmaps);


static int  DrawNiceStone (BoardWidget bw, int x, int y, int size, int color, 
			   int saved_version)
{
  int grey = FALSE;
  int dx = x;
  int dy = y;
  int dw = size;
  int dh = size;
  Display *dpy = XtDisplay((Widget) bw);
  Window   win = XtWindow( (Widget) bw);
  GC  gc = bw->board.stones_gc;
  int  copyStart;
  int  xOff = 0;
  int  yOff = 0;
  int stoneVersion = saved_version < 0 ? rand() % DRAWSTONE_NUMWHITE
                                       : saved_version;

  /* Buil new pixmaps for stones of this size if not already done: */
  if (size >= bw->board.numPics) {
    grid_morePixmaps(bw, size + 1);
  }
  if (bw->board.pics[size].stonePixmaps == None) {
    /* Draw the black stone and all white stones for this size: */
    drawStone_newPics(bw,
		      &bw->board.pics[size].stonePixmaps,
		      &bw->board.pics[size].maskBitmaps, size);
  }

  if ((dx >= x+size) || (dy >= y+size) ||
      (dx+dw <= x) || (dy+dh <= y)) {
    return stoneVersion;
  }
  if (dx < x)  {
    dw -= x - dx;
    dx = x;
  }
  if (dy < y)  {
    dh -= y - dy;
    dy = y;
  }
  if (dx + dw > x + size)
    dw = x + size - dx;
  if (dy + dh > y + size)
    dh = y + size - dy;
  XSetClipMask(dpy, gc, bw->board.pics[size].maskBitmaps);
  if (grey)  {
    if ((x & 1) == (y & 1))
      XSetClipOrigin(dpy, gc, x - size - xOff, y - yOff);
    else
      XSetClipOrigin(dpy, gc, x - size * 2 - xOff, y - yOff);
  } else  {
    XSetClipOrigin(dpy, gc, x - xOff, y - yOff);
  }
  copyStart = 0;
  if (color == White) {
    copyStart = (stoneVersion + 1) * size;
  }
  XCopyArea(dpy, bw->board.pics[size].stonePixmaps, win, gc,
	    copyStart+dx-x,dy-y, dw,dh, dx - xOff, dy - yOff);
  XSetClipMask(dpy, gc, None);
  return stoneVersion;
}

static void  grid_morePixmaps(BoardWidget bw, int newNumPics)  {
  int  i;
  CgbutsPic  *newPics;

  newPics = mymalloc(newNumPics * sizeof(CgbutsPic));
  for (i = 0;  i < bw->board.numPics;  ++i)  {
    newPics[i] = bw->board.pics[i];
  }
  for (;  i < newNumPics;  ++i)  {
    newPics[i].stonePixmaps = None;
    newPics[i].maskBitmaps = None;
  }
  bw->board.numPics = newNumPics;
  if (bw->board.pics)  {
    myfree(bw->board.pics);
  }
  bw->board.pics = newPics;
}
