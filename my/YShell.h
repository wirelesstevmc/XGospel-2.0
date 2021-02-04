#ifndef _YShell_h
#define _YShell_h
# include <X11/Shell.h>
# include "myxlib.h"

/****************************************************************
 *
 * YShell widget
 *
 ****************************************************************/

/* Resources:

 Name		     Class		RepType		Default Value
 ----		     -----		-------		-------------
 background	     Background		Pixel		XtDefaultBackground
 border		     BorderColor	Pixel		XtDefaultForeground
 borderWidth	     BorderWidth	Dimension	1
 destroyCallback     Callback		Pointer		NULL
 depth               Depth              Int             NULL
 dumpOnXError        DumpOnXError       Boolean         False
 height		     Height		Dimension	0
 mappedWhenManaged   MappedWhenManaged	Boolean		True
 nameClassList       NameClassList      NameClassList   NULL
 printResources      PrintResources     Boolean         False
 printTree           PrintTree          Boolean         False
 resourceTree        ResourceTree       String          see YShell.c
 sensitive	     Sensitive		Boolean		True
 topName             TopName            String          "default"
 visual              Visual             Visual          NULL        
 widgetTree          WidgetTree         String          "No widgetTree given"
 width		     Width		Dimension	0
 x		     Position		Position	0
 y		     Position		Position	0

*/

# ifndef XtNvisual
#  define XtNvisual       "visual"
# endif /* XtNvisual */
# ifndef XtCVisual
#  define XtCVisual       "Visual"
# endif /* XtCVisual */
# define XtNnameClassList       "nameClassList"
# define XtCNameClassList       "NameClassList"
# define XtRNameClassList       "NameClassList" 
# define XtNwidgetTree          "widgetTree"
# define XtNresourceTree        "resourceTree"
# define XtCWidgetTree          "WidgetTree"
# define XtNtopName             "topName"
# define XtCTopName             "TopName"
# define XtNprintResources      "printResources"
# define XtNprintTree           "printTree"
# define XtCPrintYShell         "PrintYShell"
# define XtNdumpOnXError        "dumpOnXError"
# define XtCDumpOnXError        "DumpOnXError"

/* declare specific YShellWidget class and instance datatypes */

typedef struct _YShellClassRec*	YShellWidgetClass;
typedef struct _YShellRec*      YShellWidget;

/* declare the class constant */

extern WidgetClass yShellWidgetClass;

typedef struct _NameClass NameClass, *NameClassList;

struct _NameClass {
    NameClassList Next;
    WidgetClass   Class;
    int           Shell;
    char          Name[1];
};

extern NameClassList NameBase;

extern NameClassList InitClassList(void);
extern WidgetClass LookupClass(const NameClassList Base, const char *Name);
extern void   AddClassList(NameClassList *Base,
                           const char *Name, WidgetClass class);
extern void   FreeClassList(NameClassList Base);
extern NameClassList ExtraClassList(const char *Name, ...);
extern MyContext YShellContext(Widget w);
extern void      MyFixShell(void);
#endif /* _YShell_h */
