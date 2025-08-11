#ifndef _GoBoard_h
#define _GoBoard_h

/***********************************************************************
 *
 * Go Board Widget
 *
 ***********************************************************************/

#include <X11/Xaw/Simple.h>

/* Resources:

 Name		     Class		RepType		Default Value
 ----		     -----		-------		-------------
 audioFile  	     AudioFile		String		NULL
 background	     Background		Pixel		XtDefaultBackground
 blackColor          BlackColor         Pixel           Black
 boardFont           Font               FontStruct      XtDefaultFont
 boardSize           BoardSize          Int             19
 border		     BorderColor	Pixel		XtDefaultForeground
 borderWidth	     BorderWidth	Dimension	1
 buttonDown          Callback           XtCallbackList  NULL
 buttonUp            Callback           XtCallbackList  NULL
 cursor		     Cursor		Cursor		None
 cursorName	     Cursor		String		NULL
 debug               Debug              Boolean         False
 destroyCallback     Callback		XtCallbackList	NULL
 foreground          Foreground         Pixel           XtDefaultForeground
 height		     Height		Dimension	text height
 insensitiveBorder   Insensitive	Pixmap		Gray
 keyDown             Callback           XtCallbackList  NULL
 lineWidth           LineWidth          Int             0
 mappedWhenManaged   MappedWhenManaged	Boolean		True
 offTimeout          Timeout            Int             500
 onTimeout           Timeout            Int             500
 pointerColor	     Foreground		Pixel		XtDefaultForeground
 pointerColorBackground Background	Pixel		XtDefaultBackground
 sensitive	     Sensitive		Boolean		True
 simpleStones        SimpleStones       Boolean         False
 userData            UserData           XtPointer       NULL
 width		     Width		Dimension	text width
 whiteColor          WhiteColor         Pixel           White
 x		     Position		Position	0
 y		     Position		Position	0

*/

#define	XtNdebug	"debug"
#define	XtNsimpleStones	"simpleStones"
#define XtNboardFont	"boardFont"
#define XtNboardSize	"boardSize"
#define XtNwhiteColor	"whiteColor"
#define XtNblackColor	"blackColor"
#define XtNdameColor	"dameColor"
#define XtNuserData     "userData"
#define XtNoffTimeout   "offTimeout"
#define XtNonTimeout    "onTimeout"
#define XtNstoneSound   "stoneSound"
#define XtNaudioFile    "audioFile"
#define XtCAudioFile    "AudioFile"
#define XtNbuttonDown	"buttonDown"
#define XtNbuttonUp	"buttonUp"
#define XtNkeyUp	"keyUp"
#define XtNkeyDown	"keyDown"
#define XtNlineWidth    "lineWidth"

#define XtCDebug	"Debug"
#define	XtCSimpleStones	"SimpleStones"
#define XtCBoardSize	"BoardSize"
#define XtCWhiteColor	"WhiteColor"
#define XtCBlackColor	"BlackColor"
#define XtCDameColor	"DameColor"
#define XtCUserData     "UserData"
#define XtCTimeout      "Timeout"
#define XtCSound        "Sound"
#define XtCLineWidth    "LineWidth"

extern WidgetClass boardWidgetClass;

typedef struct _BoardClassRec	*BoardWidgetClass;
typedef struct _BoardRec	*BoardWidget;

typedef enum {
    Empty, Black, White /* , Dame, BlackTerritory, WhiteTerritory */
} BWPiece;

#define OpponentColor(Who) ((Who)==White?Black :((Who)==Black ? White:Empty))

typedef enum {
    NoMark, CircleMark, DameMark, WhiteMark, BlackMark, SquareMark
} BWMark;

typedef struct {
  BWPiece p;
  BWMark m;
  int v;  /* stone version */
  char l;
} BWIntPiece;

typedef struct {
    int     x, y;
    String *str;
    int     params;
    Time    time;
} GoButton;

extern void    OpenBoard(Widget w);
extern void    CloseBoard(Widget w);
extern int     PutPiece(Widget w, int x, int y, BWPiece piece);
extern void    SetMark( Widget w, int x, int y, BWMark m);
extern void    LastMove(Widget w, int x, int y);
extern void    FreeBoard(char **Board, size_t sizeY);
extern char  **AllocBoard(size_t sizeX, size_t sizeY);
extern void    SetBoard(Widget w, char **Board);
extern void    GetBoard(Widget w, char **Board);
extern BWPiece GetPiece(Widget w, int x, int y);
extern void    StoneSound(Widget w);
#endif /* _GoBoard_h */
