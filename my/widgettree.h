#ifndef _WIDGETTREE_H
# define _WIDGETTREE_H

# include "myxlib.h"
# include "YShell.h"

# define ISSHELL     1
# define ISAPPSHELL  2

# define  MYNAME     1
# define  MYCLASS    2
# define  MYREALIZED 4

#if XlibSpecificationRelease <= 5
/* In IntrinsicP.h in X11R6 */
typedef struct _XtTypedArg {
    String      name;
    String      type;
    XtArgVal    value;
    int         size;
} XtTypedArg, *XtTypedArgList;
#else  /* XlibSpecificationRelease */
# include <X11/IntrinsicP.h>
#endif /* XlibSpecificationRelease */

/* List of option translations for a certain widget in the witchet tree */
typedef struct _OptionTemplate {
    struct _OptionTemplate *Next;          /* Next option for this widget */
/*  XtTypedArg             *Pos; */
    char                   *WitchetName;   /* Name of option in witchet tree */
    int                     Flags;
    char                    WidgetName[1]; /* Xt name of option              */
} OptionTemplate;

/* List of places where a witchet optionname is found */
typedef struct _TreeTemplateList {
    struct _TreeTemplateList *Next;   /*Next entry where name found          */
    struct _OptionTemplate   *Option; /*Description of translation of option */
    struct _TreeTemplate     *Pos;    /*Position in witchettree having name  */
} TreeTemplateList;

/* binary tree for translating witchet option names to Xt names and which
   widget they are meant for */
typedef struct _OptionHash {
    struct _OptionHash *Left, *Right;    /* Binary tree parts */
    const char         *WitchetName;     /* Name of option in witchet tree */
    TreeTemplateList   *Positions;       /* Single linked list of where the 
                                            name was found */
} OptionHash;

typedef struct _TreeTemplate {
    struct _TreeTemplate *Children, *Next, *Previous;
    OptionTemplate       *Options;     /* List of option translations
                                          for this widget */
    OptionHash           *HashOptions; /* Option lookup tree for option trans-
                                          lations for this witchet subtree */
    int                   NrWidgetChildren;
    int                   NrArgs;      /* Count how many args we encountered*/
    XtTypedArg           *Args;        /* Temporary place to keep options
                                            while expanding */
    const char           *UseName;     /* Name to be used on creation */
    int                   Flags;
    WidgetClass           Class;       /* Class requested */
    char                  Name[1];     /* Name  requested by tree resource */
} TreeTemplate;

typedef struct _ExpansionHash {
   struct _ExpansionHash *Next;
   Widget                 Parent;
   TreeTemplate          *Tree;
} ExpansionHash;

typedef struct _witchet {
    TreeTemplate  *Where;
    ExpansionHash *Expander;
    int            NrWidgets;
    Widget         Widgets[1];
} _Witchet, *Witchet;

extern void CallDestroyWitchet(Widget w,
                               XtPointer clientdata, XtPointer calldata);
extern TreeTemplate *MakeTemplate(const char *Name, const char *Class);
extern void FreeOptionTemplates(OptionTemplate *Options);
extern void FreeTemplate(TreeTemplate *Tree);
extern TreeTemplate *treeParse(const NameClassList Base, Widget w,
                               const char *Str, int Len);
#endif /* _WIDGETTREE_H */
