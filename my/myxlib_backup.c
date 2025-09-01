#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>

#include <X11/StringDefs.h>
#include <X11/IntrinsicP.h>
#include <X11/CoreP.h>
#include <X11/Xmu/Error.h>

#include "widgettree.h"
#include "YShell.h"
#include "myxlib.h"
#include "except.h"
#include "mymalloc.h"

#ifdef HAVE_NO_STRERROR_PROTO
extern char *strerror(int err);
#endif /* HAVE_NO_STRERROR_PROTO */

#define ClassName(x) (((CoreClassPart *)(x))->class_name)

/* #define META   "Mod1"          The modifier you like as META or ALT key */
#define META      "Meta"       /* The modifier you like as META or ALT key */

static Witchet _MyCreateWidget(const char *Name, Witchet Parent, int Managed,
                               ArgList args, Cardinal num_args);
extern Display *_XtAppInit(
#if NeedFunctionPrototypes
    XtAppContext *app_context_return, String application_class,
    XrmOptionDescRec *options, Cardinal num_options,
    int *argc_in_out, String **argv_in_out, String *fallback_resources
#endif
);

/*****************************************************************************/
/* Very simple usage function. Displays the invalid options that were given  */
/* and displays the valid ones                                               */
/*****************************************************************************/

void Usage(int argc, char const*const*argv,
           char const*const*Messages, int NrMessages)
{
    int i;

    fprintf(stderr, "Unknown options to %s:", ExceptionProgram);
    for (i=1; i<argc; i++) fprintf(stderr, " %s", argv[i]);
    fprintf(stderr, "\n");

    fprintf(stderr, "usage:  %s [-options ...]\n\n", ExceptionProgram);
    fprintf(stderr, "  Supports all normal X options%s\n",
            NrMessages ? " and also:" : "");
    for (i=0; i<NrMessages; i++) fprintf(stderr, "%s\n", Messages[i]);
}

/*****************************************************************************/
/* Replacement for the default Error handlers. Dumps core all over the place */
/*****************************************************************************/

static int DumpError (Display *dpy, XErrorEvent *event)
{
    int rc;

    rc = XmuPrintDefaultErrorMessage(dpy, event, stderr);
    if (rc) {
        fprintf(stderr, "Dumping core as requested\n");
        fflush(stderr);
        abort();
        return rc;                   /* Just in case */
    } else return 0;
}

static int DumpIOError(Display *dpy)
{
    fprintf(stderr, "XIO:  fatal IO error %d (%s) on X server \"%s\"\r\n",
            errno, strerror(errno), DisplayString(dpy));
    fprintf(stderr, "      after %lu requests (%lu known processed) with"
            " %d events remaining.\r\n", NextRequest(dpy) - 1,
            LastKnownRequestProcessed(dpy), QLength(dpy));

    if (errno == EPIPE)
        fprintf(stderr, "      The connection was probably broken by a server"
                " shutdown or KillClient.\r\n");
    fprintf(stderr, "Dumping core as requested\n");
    fflush(stderr);
    abort();
    return 1;                   /* Just in case */
}

/* the following two were both "X Toolkit " prior to R4 */
#ifndef XTERROR_PREFIX
#define XTERROR_PREFIX ""
#endif

#ifndef XTWARNING_PREFIX
#define XTWARNING_PREFIX ""
#endif

static void DumpXtError(String message)
{
    (void) fprintf(stderr, "%sError: %s\nDumping core as requested\n",
                   XTERROR_PREFIX, message);
    fflush(stderr);
    abort();
    exit(1);                   /* Just in case */
}

void DumpOnIOError(XtAppContext app_con)
{
    XSetIOErrorHandler(DumpIOError);
    XSetErrorHandler(DumpError);
    XtAppSetErrorHandler(app_con, DumpXtError);
    /* XtAppSetWarningHandler(); */
}

/*****************************************************************************/
/* Fake children handler                                                     */
/*****************************************************************************/

void AddFakeChild(Widget Parent, Widget Child)
{
    XtWidgetProc	    insert_child;
    Widget                  old;

    if (XtIsComposite(Parent) == False)
        WidgetWarning(Parent, "is not composite");
    else {
        insert_child = ((CompositeWidgetClass) Parent->core.widget_class)->
            composite_class.insert_child;
        if (insert_child) {
            old = Child->core.parent;
            Child->core.parent = Parent;
            insert_child(Child);
            Child->core.parent = old;
            XtAddCallback(Child, XtNdestroyCallback,
                          CallFakeChildDestroy, (XtPointer) Parent);
        } else WidgetWarning(Parent, "has nullProc as insertChild");
    }
}

/*****************************************************************************/
/* Managing and unmanaging children in one fell swoop                        */
/*****************************************************************************/
void VaSetManagementChildren(Widget first, ...)
{
#define MAXCHILD 100
    CompositeWidget parent;
    XtWidgetProc    change_managed;
    Bool            parent_realized;
    Widget          cache[MAXCHILD], w, *unique_children;
    va_list         args;
    int             NrChildren, num_unique_children, i, manage, changes;

    if (!first) return;
    parent = (CompositeWidget) first->core.parent;

    if (XtIsComposite((Widget) parent) == False)
        XtAppErrorMsg(XtWidgetToApplicationContext((Widget)parent),
                      "invalidParent","VaSetManagementChildren",
                      "myProgsError", "Attempt to chane management of a"
                      "child whose parent is not Composite",
                      (String *) NULL, (Cardinal *) NULL);

    change_managed = ((CompositeWidgetClass) parent->core.widget_class)
        ->composite_class.change_managed;
    parent_realized = XtIsRealized((Widget)parent);

    if (parent->core.being_destroyed) return;

    NrChildren = 0;
    va_start(args, first);
    for (w = first; w; w = va_arg(args, Widget)) {
        NrChildren++;
        (void) va_arg(args, int);
    }
    va_end(args);

    /* Construct new list of children that really need to be operated upon. */
    if (NrChildren <= MAXCHILD) unique_children = cache;
    else                        unique_children = mynews(Widget, NrChildren);

    num_unique_children = 0;
    changes             = 0;
    va_start(args, first);
    for (w = first; w; w = va_arg(args, Widget)) {
        manage = va_arg(args, int);
        if ((CompositeWidget) w->core.parent != parent)
            WidgetWarning(w, "does not have the same parent as the first "
                          "widget in VaSetManagementChildren");
        else if ((w->core.managed == False) == manage) {
            if (manage) {
                if (w->core.being_destroyed == False) {
                    changes = 1;
                    w->core.managed = True;
                    unique_children[num_unique_children++] = w;
                }
            } else {
                changes = 1;
                w->core.managed = False;
                if (XtIsWidget(w) != False) {
                    if (XtIsRealized(w) != False &&
                        w->core.mapped_when_managed != False)
                        XtUnmapWidget(w);
                } else { /* RectObj w */
                    Widget  pw;
                    RectObj r;

                    for (pw = (Widget) parent; pw; pw = pw->core.parent)
                        if (XtIsWidget(pw) != False) {
                            if (XtIsRealized(pw) != False) {
                                r = (RectObj) w;
                                XClearArea (XtDisplay(pw), XtWindow(pw),
                                            r->rectangle.x, r->rectangle.y,
                                            (unsigned int)
                                            (r->rectangle.width +
                                             (r->rectangle.border_width << 1)),
                                            (unsigned int)
                                            (r->rectangle.height +
                                             (r->rectangle.border_width << 1)),
                                            True);
                            }
                            break;
                        }
                }
            }
        }
    }
    va_end(args);

    if (changes && change_managed && parent_realized != False)
	(*change_managed) ((Widget)parent);
    if (parent_realized != False) {
	/* Realize each child if necessary, then map if necessary */
	for (i = 0; i < num_unique_children; i++) {
	    w = unique_children[i];
	    if (XtIsWidget(w) != False) {
		if (XtIsRealized(w) == False) XtRealizeWidget(w);
		if (w->core.mapped_when_managed != False) XtMapWidget(w);
	    } else { /* RectObj w */
		Widget pw;
		RectObj r;

                for (pw = (Widget) parent; pw; pw = pw->core.parent)
                    if (XtIsWidget(pw) != False) {
                        r = (RectObj) w;
                        XClearArea (XtDisplay (pw), XtWindow (pw),
                                    r->rectangle.x, r->rectangle.y,
                                    (unsigned int)
                                    (r->rectangle.width +
                                     (r->rectangle.border_width << 1)),
                                    (unsigned int)
                                    (r->rectangle.height +
                                     (r->rectangle.border_width << 1)),
                                    True);
                        break;
                    }
            }
        }
    }

    if (unique_children != cache) myfree(unique_children);
}

void SetManagementChildren(Widget *Manage,   int NrManage,
                           Widget *UnManage, int NrUnManage)
{
#define MAXCHILD 100
    CompositeWidget parent;
    XtWidgetProc    change_managed;
    Bool            parent_realized;
    Widget          first, cache[MAXCHILD], w, *unique_children;
    int             NrChildren, num_unique_children, i, j, changes;

    if      (NrManage)   first = Manage[0];
    else if (NrUnManage) first = UnManage[0];
    else return;

    parent = (CompositeWidget) first->core.parent;

    if (XtIsComposite((Widget) parent) == False)
        XtAppErrorMsg(XtWidgetToApplicationContext((Widget)parent),
                      "invalidParent","SetManagementChildren",
                      "myProgsError", "Attempt to chane management of a"
                      "child whose parent is not Composite",
                      (String *) NULL, (Cardinal *) NULL);

    change_managed = ((CompositeWidgetClass) parent->core.widget_class)
        ->composite_class.change_managed;
    parent_realized = XtIsRealized((Widget)parent);

    if (parent->core.being_destroyed) return;

    NrChildren = NrManage+NrUnManage;
    /* Construct new list of children that really need to be operated upon. */
    if (NrChildren <= MAXCHILD) unique_children = cache;
    else                        unique_children = mynews(Widget, NrChildren);

    num_unique_children = 0;
    changes             = 0;

    for (j=0; j<NrManage; j++) {
        w = Manage[j];
        if ((CompositeWidget) w->core.parent != parent)
            WidgetWarning(w, "does not have the same parent as the first "
                          "widget in SetManagementChildren");
        else if (w->core.managed == False) {
            if (w->core.being_destroyed == False) {
                changes = 1;
                w->core.managed = True;
                unique_children[num_unique_children++] = w;
            }
        }
    }

    for (j=0; j<NrUnManage; j++) {
        w = UnManage[j];
        if ((CompositeWidget) w->core.parent != parent)
            WidgetWarning(w, "does not have the same parent as the first "
                          "widget in VaSetManagementChildren");
        else if (w->core.managed != False) {
            changes = 1;
            w->core.managed = False;
            if (XtIsWidget(w) != False) {
                if (XtIsRealized(w) != False &&
                    w->core.mapped_when_managed != False)
                    XtUnmapWidget(w);
            } else {            /* RectObj w */
                Widget  pw;
                RectObj r;

                for (pw = (Widget) parent; pw; pw = pw->core.parent)
                    if (XtIsWidget(pw) != False) {
                        if (XtIsRealized(pw) != False) {
                            r = (RectObj) w;
                            XClearArea (XtDisplay(pw), XtWindow(pw),
                                        r->rectangle.x, r->rectangle.y,
                                        (unsigned int)
                                        (r->rectangle.width +
                                         (r->rectangle.border_width << 1)),
                                        (unsigned int)
                                        (r->rectangle.height +
                                         (r->rectangle.border_width << 1)),
                                        True);
                        }
                        break;
                    }
            }
        }
    }

    if (changes && change_managed && parent_realized != False)
	(*change_managed) ((Widget)parent);
    if (parent_realized != False) {
	/* Realize each child if necessary, then map if necessary */
	for (i = 0; i < num_unique_children; i++) {
	    w = unique_children[i];
	    if (XtIsWidget(w) != False) {
		if (XtIsRealized(w) == False) XtRealizeWidget(w);
		if (w->core.mapped_when_managed != False) XtMapWidget(w);
	    } else { /* RectObj w */
		Widget pw;
		RectObj r;

                for (pw = (Widget) parent; pw; pw = pw->core.parent)
                    if (XtIsWidget(pw) != False) {
                        r = (RectObj) w;
                        XClearArea (XtDisplay (pw), XtWindow (pw),
                                    r->rectangle.x, r->rectangle.y,
                                    (unsigned int)
                                    (r->rectangle.width +
                                     (r->rectangle.border_width << 1)),
                                    (unsigned int)
                                    (r->rectangle.height +
                                     (r->rectangle.border_width << 1)),
                                    True);
                        break;
                    }
            }
        }
    }

    if (unique_children != cache) myfree(unique_children);
}

/*****************************************************************************/
/* XawScrollbarSetThumb can be defined with args float or args double.       */
/* So we can't use it if we don't know the NARROWPROTO define. That's the    */
/* bummer with not necessarily using the X-windows config and C-compiler.    */
/*****************************************************************************/

#include <X11/Xaw/Scrollbar.h>

/* Declared double to give people who forget the prototype a fighting chance */
void MyScrollbarSetThumb(Widget w, double top, double shown)
{
    if (sizeof(float) > sizeof(XtArgVal)) {
        float ltop   = top;
        float lshown = shown;

        XtVaSetValues(w,
                      XtNshown,      (XtArgVal) &lshown,
                      XtNtopOfThumb, (XtArgVal) &ltop,
                      NULL);
    } else {
        XtArgVal ltop, lshown;

        *(float *) &ltop   = top;
        *(float *) &lshown = shown;
        XtVaSetValues(w,
                      XtNshown,      lshown,
                      XtNtopOfThumb, ltop,
                      NULL);
    }
}

/*****************************************************************************/
/* Reorganize widget to it's preferred geometry                              */
/*****************************************************************************/

void NaturalWidgetSize(Widget w)
{
    XtWidgetGeometry preferred;

    XtQueryGeometry(w, NULL, &preferred);
    XtMakeGeometryRequest(w, &preferred, NULL);
}

/*****************************************************************************/
/* Simple tree interpreter                                                   */
/*****************************************************************************/

/* Only works with option name translation ! */
const char MyNname[]     = "name";
const char MyCName[]     = "Name";
const char MyNclass[]    = "class";
const char MyCClass[]    = "Class";
const char MyNrealized[] = "realized";
const char MyCRealized[] = "Realized";

#define ONENTER(action)             \
        "Ctrl<Key>j:" action "\\n"  \
        "Ctrl<Key>m:" action "\\n"  \
        "Ctrl<Key>o:" action "\\n"  \
        "<Key>Return:" action "\\n" \
        "<Key>KP_Enter:" action
#define ONTAB(action)                    \
        "~Shift Ctrl<Key>i:" action "\\n"\
        "~Shift<Key>Tab:"    action
#define ONBACKTAB(action)                \
        "Shift Ctrl<Key>i:" action "\\n" \
        "Shift<Key>Tab:" action

static String myfallback_resources[] = {
    (String) "*resourceTree*widgetInfo.?.width:             650",
    (String) "*resourceTree*widgetInfo.?.height:            500",

    (String) "*resourceTree*widgetTree.width:               50",
    (String) "*resourceTree*widgetTree.height:              50",
    (String) "*resourceTree*widgetTree.allowShellResize:    True",
    (String) "*resourceTree*widgetTree.?.width:             50",/* functions as a min */
    (String) "*resourceTree*widgetTree.?.height:            50",/* functions as a min */
    (String) "*resourceTree*widgetTree.collect.?.allowResize:            True",
    (String) "*resourceTree*widgetTree.collect.viewport.allowHoriz:      True",
    (String) "*resourceTree*widgetTree.collect.viewport.allowVert:       True",
    (String) "*resourceTree*widgetTree.collect.viewport.tree.x:          10",
    (String) "*resourceTree*widgetTree.collect.viewport.tree.y:          10",
    (String) "*resourceTree*widgetTree.collect.buttons.restreeClose.background:  Red",
    (String) "*resourceTree*widgetTree.collect*Command.background:       Green",
    (String) "*resourceTree*widgetTree.collect.viewport.tree.treeEntry.translations: #override \\n"
        "<Btn1Down>: set()                \\n"
        "<Btn1Up>:   widgetAction(info)   \\n"
        "<Btn3Down>: set()                \\n"
        "<Btn3Up>:   widgetAction(change)",

    (String) "*resourceTree*widgetChange.allowShellResize:  True",
    (String) "*resourceTree*widgetChange*Text.resize:       both",
    (String) "*resourceTree*widgetChange*Text.translations: #override\\n"
        ONTAB("nexttext()")"\\n"
        ONENTER("nexttext()")"\\n"
        ONBACKTAB("previoustext()")"\\n"
        "<Btn1Down>:thistext() select-start()\\n"
        "<Btn2Down>:thistext() insert-selection(\"PRIMARY\",\"CUT_BUFFER0\")\\n"
        "<Btn3Down>:thistext() extend-start()\\n"
        META "<Key>c:change() \\n"
        META "<Key>w:widgets()\\n"
        META "<Key>h:info()",

    (String) "*widgetHelp.allowShellResize:                 True",
    (String) "*widgetHelp*background:                       XtDefaultBackground",

    (String) "*resourceTree*collect.?.resizeToPreferred:    True",
    (String) "*resourceTree*TopLevelShell.translations:     #override \\n"
        "<ClientMessage>WM_PROTOCOLS:   WMprotocol() \\n"
        "<ClientMessage>DELETE:         destroy()",
#ifndef   XAW3D
    (String) "*resourceTree*buttons*shapeStyle:             roundedRectangle",
#endif /* XAW3D */
    (String) "*resourceTree*Command.background:             Green",
    (String) "*resourceTree*Command.foreground:             XtDefaultForeground",
    (String) "*resourceTree*Toggle.background:              Green",
    (String) "*resourceTree*Toggle.foreground:              XtDefaultForeground",
    (String) "*resourceTree*restreeClose.background:        Red",
    (String) "*resourceTree*restreeClose.foreground:        Yellow",
    (String) "*resourceTree*restreeClose.label:             Close",
    (String) "*resourceTree*cancel.background:              Red",
    (String) "*resourceTree*cancel.foreground:              Yellow",
    (String) "*resourceTree*info.scrollVertical:            whenNeeded",
    (String) "*resourceTree*info.wrap:                      word",
    (String) "*resourceTree*Paned.?.resizeToPreferred:      True",
    (String) "*resourceTree*background:                     #BFD8D8",
    (String) "*widgetHelp.translations:                     #override \\n"
        "<Btn1Up>: MenuPopdown()",

    (String) "*?*accelerators:                              #augment \\n"
        META "<Key>c: change()  \\n"
        META "<Key>w: widgets() \\n"
        META "<Key>h: info()",
    (String) "*TopLevelShell.translations:                  #override \\n"
        "<ClientMessage>WM_PROTOCOLS: WMprotocol() \\n"
        "<ClientMessage>DELETE:       destroy()",
    (String) "?.translations:                               #override \\n"
        "<ClientMessage>WM_PROTOCOLS: WMprotocol() \\n"
        "<ClientMessage>DELETE:       quit()",
    (String) NULL
};

static Exception NoWitchet      = { "No witchet corresponding to widget"};
static Exception NoCorresponding= { "Could not find name corresponding to"};
static Exception InvalidArgs    = { "Invalid arguments."};

void FreeOptionTemplates(OptionTemplate *Options)
{
    OptionTemplate *Next, *Option;

    for (Option = Options; Option; Option = Next) {
        Next = Option->Next;
        myfree(Option);
    }
}

static void FreeOptionHashes(OptionHash *Hashes)
{
    TreeTemplateList *Here, *Next;

    if (Hashes) {
        FreeOptionHashes(Hashes->Left);
        FreeOptionHashes(Hashes->Right);

        for (Here = Hashes->Positions; Here; Here = Next) {
            Next = Here->Next;
            myfree(Here);
        }
        myfree(Hashes);}
}

void FreeTemplate(TreeTemplate *Tree)
{
    TreeTemplate *Next;

    if (Tree) {
        for (Tree->Previous->Next = NULL; Tree; Tree = Next) {
            Next = Tree->Next;
            FreeTemplate(Tree->Children);
            FreeOptionTemplates(Tree->Options);
            FreeOptionHashes(Tree->HashOptions);
            myfree(Tree);
        }
    }
}

extern void _CallFreeTemplate(Widget w,
                              XtPointer clientdata, XtPointer calldata);
void _CallFreeTemplate(Widget w, XtPointer clientdata, XtPointer calldata)
{
    FreeTemplate((TreeTemplate *) clientdata);
}

static void FreeExpander(ExpansionHash *Here)
{
    ExpansionHash *Next;

    while (Here) {
        Next = Here->Next;
        myfree(Here);
        Here = Next;
    }
}

void CallDestroyWitchet(Widget w, XtPointer clientdata, XtPointer calldata)
{
    Witchet witchet;

    witchet = (Witchet) clientdata;
    FreeExpander(witchet->Expander);
    myfree(witchet);
}

static Witchet WidgetToWitchet(Widget w)
{
    XtPointer Result;

    if (!FindCallback(w, XtNdestroyCallback, CallDestroyWitchet, &Result))
        Raise1(NoWitchet, XtName(w));
    return (Witchet) Result;
}

Widget WitchetOfWidget(Widget w)
{
    Widget    Here, Last;
    XtPointer data;

    Last = 0;
    for (Here = w; Here; Here = XtParent(Here)) {
        Last = Here;
        if (FindCallback(Here, XtNdestroyCallback, CallDestroyWitchet, NULL))
            return Here;
    }
    if (Last && FindCallback(Last, XtNdestroyCallback, CallFakeChildDestroy,
                             &data)) return (Widget) data;
    Raise1(NoWitchet, XtName(w));
    return 0;
}

static void ShowOptionHash(FILE *fp, OptionHash *Hash)
{
    TreeTemplateList *Here;

    if (Hash) {
        putc('(', fp);
        ShowOptionHash(fp, Hash->Left);
        fprintf(fp, " %s ", Hash->WitchetName);
        for (Here = Hash->Positions; Here; Here = Here->Next)
            fprintf(fp, "%s(%s,%s,%d) ", Here->Pos->Name,
                    Here->Option->WidgetName, Here->Option->WitchetName,
                    Here->Option->Flags);
        ShowOptionHash(fp, Hash->Right);
        putc(')', fp);
    }
}

static void fprintResources(FILE *fp, String *resources)
{
    String resource;

    while ((resource = *resources++) != NULL) {
        fputs(resource, fp);
        putc('\n', fp);
    }
}

static void PrintTree(FILE *fp, TreeTemplate *ToPrint, int indent, int HashP)
{
    int jump;
    TreeTemplate   *Here;
    OptionTemplate *OHere;

    if (ToPrint) {
        jump = 3 * (indent+1);
        fprintf(fp, "%*s(\n", jump-3, "");
        for (Here = ToPrint->Next; Here != ToPrint; Here = Here->Next) {
            if (Here->Class) fprintf(fp, "%*s%s(%s)", jump, "", Here->Name,
                                     ClassName(Here->Class));
            else fprintf(fp, "%*s%s", jump, "", Here->Name);
            for (OHere = Here->Options; OHere; OHere = OHere->Next)
                fprintf(fp, "[%s,%s]", OHere->WidgetName, OHere->WitchetName);
            if (HashP && Here->HashOptions) {
                ShowOptionHash(fp, Here->HashOptions);
                putc('\n', fp);
            }
            putc('\n', fp);
            PrintTree(fp, Here->Children, indent+1, HashP);
        }
        fprintf(fp, "%*s)\n", jump-3, "");
    }
}

void fprintTree(FILE *fp, Widget w)
{
    TreeTemplate   *Here;
    OptionTemplate *OHere;
    static int      HashP = 0;

    Here = WidgetToWitchet(w)->Where;
    if (Here->Class) fprintf(fp, "%s(%s)", Here->Name, ClassName(Here->Class));
    else fprintf(fp, "%s", Here->Name);
    for (OHere = Here->Options; OHere; OHere = OHere->Next)
        fprintf(fp, "[%s,%s]", OHere->WidgetName, OHere->WitchetName);
    if (HashP && Here->HashOptions) {
        ShowOptionHash(fp, Here->HashOptions);
        putc('\n', fp);
    }
    putc('\n', fp);
    PrintTree(fp, Here->Children, 0, HashP);
}

static void WorkExpandTree(Widget Parent, TreeTemplate *Child,
                           ExpansionHash **Expander)
{
    Widget CorrespondingChild;
    TreeTemplate  *Children, *Here;
    ExpansionHash *NewHash;

    if (Child->Class) {
        if (Child->Flags & ISAPPSHELL) {
            CorrespondingChild =
                XtVaAppCreateShell(*Child->UseName ? Child->UseName : NULL,
                                   (String) ApplicationClassName(Parent),
                                   Child->Class, XtDisplay(Parent),
                                   XtVaNestedList, Child->Args, NULL);
            AddFakeChild(Parent, CorrespondingChild);
        } else if (Child->Flags & ISSHELL)
            CorrespondingChild =
                XtVaCreatePopupShell(Child->UseName, Child->Class, Parent,
                                     XtVaNestedList, Child->Args, NULL);
        else
            CorrespondingChild =
                XtVaCreateManagedWidget(Child->UseName, Child->Class, Parent,
                                        XtVaNestedList, Child->Args, NULL);

        Children = Child->Children;
        if (Children)
            for (Here = Children->Next; Here != Children; Here = Here->Next)
                WorkExpandTree(CorrespondingChild, Here, Expander);
    } else {
        NewHash = mynew(ExpansionHash);
        NewHash->Tree   = Child;
        NewHash->Parent = Parent;

        NewHash->Next   = *Expander;
        *Expander        = NewHash;
    }
}

#ifdef DEBUGGING
static void PrintExpander(ExpansionHash *Here)
{
    WidgetClass WClass;

    while(Here) {
        if (WClass = Here->Tree->Class)
            printf("---%s[%s]----\n", Here->Tree->Name, ClassName(WClass));
        else printf("---%s----\n", Here->Tree->Name);
        PrintTree(Here->Tree->Children, 0);
        Here = Here->Next;
    }
}
#endif /* DEBUGGING */

/* Can leave a partially expanded widget tree in case of failure */
static void ExpandTree(Widget Parent, int Managed, TreeTemplate *Correspond,
                       Witchet Result, XtVarArgsList args)
{
    Widget CorrespondingChild;
    TreeTemplate  *SubChildren, *Here, *Child, *Children;
    int    NrChildren;
    ExpansionHash * volatile Expander, *NewHash;

    Expander = NULL;
    Children = Correspond->Children;
    if (!Children || Children->Next == Children)
        Raise1(AssertException, "witchet without children not supported yet");
    NrChildren = 0;
    WITH_HANDLING {
        for (Child = Children->Next; Child != Children; Child = Child->Next) {
            if (Child->Class) {
                if (Child->Flags & ISAPPSHELL) {
                    CorrespondingChild =
                        XtVaAppCreateShell(NULL, (String)
                                           ApplicationClassName(Parent),
                                           Child->Class, XtDisplay(Parent),
                                           XtVaNestedList, args,
                                           XtVaNestedList, Child->Args, NULL);
                    AddFakeChild(Parent, CorrespondingChild);
                } else if (Child->Flags & ISSHELL) CorrespondingChild =
                    XtVaCreatePopupShell(Child->UseName, Child->Class, Parent,
                                         XtVaNestedList, args,
                                         XtVaNestedList, Child->Args,
                                         NULL);
                else if (Managed) CorrespondingChild =
                    XtVaCreateManagedWidget(Child->UseName,Child->Class,Parent,
                                            XtVaNestedList, args,
                                            XtVaNestedList, Child->Args,
                                            NULL);
                else CorrespondingChild =
                    XtVaCreateWidget(Child->UseName, Child->Class, Parent,
                                     XtVaNestedList, args,
                                     XtVaNestedList, Child->Args,
                                     NULL);
                SubChildren = Child->Children;
                if (SubChildren)
                    for (Here = SubChildren->Next;
                         Here != SubChildren;
                         Here = Here->Next)
                        WorkExpandTree(CorrespondingChild, Here,
                                       (ExpansionHash **) &Expander);
                Result->Widgets[NrChildren++] = CorrespondingChild;
            } else {
                NewHash = mynew(ExpansionHash);
                NewHash->Tree   = Child;
                NewHash->Parent = Parent;

                NewHash->Next   = Expander;
                Expander        = NewHash;
            }
        }
    } ON_EXCEPTION {
/*      if (Parent) XtDestroyWidget(CorrespondingChild); */
        FreeExpander(Expander);
    } END_HANDLING;
    Result->Where      = Correspond;
    Result->Expander   = Expander;
    Result->NrWidgets  = NrChildren;
}

void PrintVarArgsList(FILE *fp, XtVarArgsList List)
{
    XtTypedArg *AvList;
    String      Name;

    putc('(', fp);

    for (AvList = (XtTypedArgList) List;
         (Name = AvList->name) != NULL;
         AvList++) {
        if (strcmp(Name, XtVaNestedList) == 0)
            PrintVarArgsList(fp, (XtVarArgsList) AvList->value);
        else if (AvList->type)
            fprintf(fp, "[%s %s %08lx %d]", AvList->name, AvList->type,
                    (long) AvList->value, AvList->size);
        else fprintf(fp, "[%s %08lx]", AvList->name, AvList->value);
    }
    putc(')', fp);
}

XtVarArgsList ArgArrayToList(ArgList args, Cardinal num_args)
{
    XtTypedArg *AvList, *Av;

    AvList = mynews(XtTypedArg, num_args+1);
    for (Av = AvList; num_args>0; num_args--, Av++, args++) {
        Av->name  = args->name;
        Av->type  = NULL;
        Av->value = args->value;
    }
    Av->name = NULL;
    return (XtVarArgsList) AvList;
}

int CountVarArgs (va_list var)
{
    int            count;
    String	   attr;

    count = 0;
    for(attr = va_arg(var, String) ; attr; attr = va_arg(var, String)) {
        count++;
        if (strcmp(attr, XtVaTypedArg)) (void) va_arg(var, XtArgVal);
        else {
            (void) va_arg(var, String);
            (void) va_arg(var, String);
            (void) va_arg(var, XtArgVal);
            (void) va_arg(var, int);
        }
    }
    return count;
}

XtVarArgsList VarArgsToList(va_list var, int count)
{
    XtTypedArgList AvList;
    String	   attr;

    AvList = mynews(XtTypedArg, count+1);
    count = 0;
    while ((attr = va_arg(var, String)) != NULL) {
        if (strcmp(attr, XtVaTypedArg)) {
            AvList[count].name = attr;
            AvList[count].type = NULL;
            AvList[count].value = va_arg(var, XtArgVal);
        } else {
            AvList[count].name  = va_arg(var, String);
            AvList[count].type  = va_arg(var, String);
            AvList[count].value = va_arg(var, XtArgVal);
            AvList[count].size  = va_arg(var, int);
        }
        count++;
    }
    AvList[count].name = NULL;
    return (XtVarArgsList) AvList;
}

static void InitTreeNrArgs(TreeTemplate *Tree)
{
    TreeTemplate *Here;

    Tree->NrArgs  = 0;
    Tree->Args    = NULL;
    Tree->UseName = Tree->Name;
    Tree = Tree->Children;
    if (Tree)
        for (Here = Tree->Next; Here != Tree; Here = Here->Next)
            if (Here->Class) InitTreeNrArgs(Here);
}

static void AllocTreeArgs(TreeTemplate *Tree)
{
    TreeTemplate *Here;

    Tree->Args   = mynews(XtTypedArg, Tree->NrArgs+1);
    Tree->Args[Tree->NrArgs].name = NULL;
    Tree->NrArgs = 0;
    Tree = Tree->Children;
    if (Tree)
        for (Here = Tree->Next; Here != Tree; Here = Here->Next)
            if (Here->Class) AllocTreeArgs(Here);
}

static void FreeTreeArgs(TreeTemplate *Tree)
{
    TreeTemplate *Here;

    myfree(Tree->Args);
    Tree = Tree->Children;
    if (Tree)
        for (Here = Tree->Next; Here != Tree; Here = Here->Next)
            if (Here->Class) FreeTreeArgs(Here);
}

static int UpdateNrArgs(const char *Name, const OptionHash *Hashes)
{
    int               rc;
    const OptionHash *Here;
    TreeTemplateList *Pos;

    Here = Hashes;
    while (Here) {
        rc = strcmp(Name, Here->WitchetName);
        if      (rc < 0) Here = Here->Left;
        else if (rc > 0) Here = Here->Right;
        else {
            for (Pos = Here->Positions; Pos; Pos = Pos->Next)
                if (!Pos->Option->Flags) Pos->Pos->NrArgs++;
            return 0;
        }
    }
    return 1;
}

static int UpdateArgs(XtTypedArg *Argh, const OptionHash *Hashes)
{
    int               rc;
    const OptionHash *Here;
    TreeTemplateList *Pos;
    const char       *Name;
    WidgetClass       WClass;

    Name = Argh->name;
    Here = Hashes;
    while (Here) {
        rc = strcmp(Name, Here->WitchetName);
        if      (rc < 0) Here = Here->Left;
        else if (rc > 0) Here = Here->Right;
        else {
            for (Pos = Here->Positions; Pos; Pos = Pos->Next)
                switch(Pos->Option->Flags) {
                  case MYNAME:
                    Pos->Pos->UseName  = (const char *) Argh->value;
                    break;
                  case MYCLASS:
                    if (Argh->value) {
                        WClass = LookupClass(NameBase,
                                             (const char *) Argh->value);
                        if (WClass) {
                            Pos->Pos->Class = WClass;
                            Pos->Pos->Flags &= ~(ISSHELL | ISAPPSHELL);
                            if (WClass &&
                                MyIsSubclass(WClass,
                                             shellWidgetClass) != False) {
                                Pos->Pos->Flags |= ISSHELL;
                                if (MyIsSubclass(WClass,
                                                 applicationShellWidgetClass)
                                    != False)
                                    Pos->Pos->Flags |= ISAPPSHELL;
                            }
                        } else {
                            fprintf(stderr, "Unknown class %s for dynamic "
                                    "witchet class.\n",
                                    (const char *) Argh->value);
                            fflush(stderr);
                        }
                    }
                    break;
                  default:
                    Argh->name = Pos->Option->WidgetName;
                    Pos->Pos->Args[Pos->Pos->NrArgs++] = *Argh;
                    break;
                }
            return 0;
        }
    }
    return 1;
}

static int CountArgArray(ArgList args, Cardinal num_args,
                         const OptionHash *Hashes)
{
    int count;

    for (count = 0; num_args > 0; num_args--, args++)
        count += UpdateNrArgs(args->name, Hashes);
    return count;
}

static XtVarArgsList ArgArrayToDistributedList(ArgList args, Cardinal num_args,
                                               int count,
                                               const OptionHash *Hashes)
{
    XtTypedArg *AvList, *Av;
    XtTypedArg Work;

    AvList = Av = mynews(XtTypedArg, count+1);
    for (Av = AvList; num_args>0; num_args--, args++) {
        Work.name  = args->name;
        Work.type  = NULL;
        Work.value = args->value;
        if (UpdateArgs(&Work, Hashes)) *Av++ = Work;
    }
    Av->name = NULL;
    return (XtVarArgsList) AvList;
}

static int CountNestedNrArgs(const XtTypedArg *Args, const OptionHash *Hashes)
{
    const char       *Name;
    const XtTypedArg *Argh;
    int               count;

    count = 0;
    for (Argh = Args; (Name = Argh->name) != NULL; Argh++)
        if (strcmp(Name, XtVaNestedList) == 0)
            count += CountNestedNrArgs((XtTypedArg *) Argh->value, Hashes);
        else count += UpdateNrArgs(Name, Hashes);
    return count;
}

static XtTypedArg *UpdateNestedArgs(XtTypedArg *Target,
                                    const XtTypedArg *Args,
                                    const OptionHash *Hashes)
{
    const XtTypedArg *Argh;
    const char       *Name;
    XtTypedArg        Work;

    for (Argh = Args; (Name = Argh->name) != NULL; Argh++)
        if (strcmp(Name, XtVaNestedList) == 0)
            Target = UpdateNestedArgs(Target, (XtTypedArg *) Argh->value,
                                      Hashes);
        else {
            Work = *Argh;
            if (UpdateArgs(&Work, Hashes)) *Target++ = Work;
        }
    return Target;
}

static int CountVarArgNrArgs(va_list var, const OptionHash *Hashes)
{
    int               count;
    String	      name;

    count = 0;
    for(name = va_arg(var, String) ; name; name = va_arg(var, String))
        if (strcmp(name, XtVaNestedList) == 0)
            count += CountNestedNrArgs((XtTypedArg *) va_arg(var, XtArgVal),
                                       Hashes);
        else {
            if (strcmp(name, XtVaTypedArg) == 0) {
                name = va_arg(var, String);
                (void) va_arg(var, String);
                (void) va_arg(var, XtArgVal);
                (void) va_arg(var, int);
            } else (void) va_arg(var, XtArgVal);
            count += UpdateNrArgs(name, Hashes);
        }
    return count;
}

static XtVarArgsList VarArgsToDistributedList(va_list var, int count,
                                              const OptionHash *Hashes)
{
    XtTypedArgList  AvList, Av;
    XtTypedArg      NewArg;
    String	    Name;

    AvList = Av = mynews(XtTypedArg, count+1);
    while ((Name = va_arg(var, String)) != NULL) {
        if (strcmp(Name, XtVaNestedList) == 0)
            Av = UpdateNestedArgs(Av, (XtTypedArg *) va_arg(var, XtArgVal),
                                  Hashes);
        else {
            if (strcmp(Name, XtVaTypedArg) == 0) {
                NewArg.name  = va_arg(var, String);
                NewArg.type  = va_arg(var, String);
                NewArg.value = va_arg(var, XtArgVal);
                NewArg.size  = va_arg(var, int);
            } else {
                NewArg.name = Name;
                NewArg.type = NULL;
                NewArg.value = va_arg(var, XtArgVal);
            }
            if (UpdateArgs(&NewArg, Hashes)) *Av++ = NewArg;
        }
    }
    Av->name = NULL;
    return (XtVarArgsList) AvList;
}

Widget MyAppInitialize(XtAppContext *app_context,
                       const char *application_class,
                       XrmOptionDescList options, Cardinal num_options,
                       int *argc_in_out, String *argv_in_out,
                       String *fallback_resources, ArgList args_in,
                       Cardinal num_args_in,
                       char const*const*Messages, int NrMessages,
                       void (*CallBack)(XtAppContext app))
{
    XtAppContext   app_con;
    Widget         toplevel;
    Witchet        w, parent;
    ExpansionHash *hash;
    int            i, j, saved_argc;
    String        *fb;
    Arg            args[3], *merged_args;
    Cardinal       num;
    Display       *dpy;
    Boolean        printTree, printResources, dump;

    toplevel = 0;
    i = 1; j = 0;
    if (fallback_resources)
        for (fb =   fallback_resources; *fb; fb++) i++;
    for (fb = myfallback_resources; *fb; fb++) j++;
    fb = mynews(String, i+j);
    printTree = printResources = dump = False;
    WITH_UNWIND {
	memcpy(fb, myfallback_resources, j*sizeof(String));
        if (fallback_resources)
            memcpy(fb+j, fallback_resources, i*sizeof(String));
        else fb[j] = NULL;
        if (!ExceptionProgram) ExceptionProgram = argv_in_out[0];

        XtToolkitInitialize(); /* cannot be moved into _XtAppInit */

        saved_argc = *argc_in_out;
        dpy = _XtAppInit(&app_con, (String)application_class,
                         options, num_options, argc_in_out, &argv_in_out, fb);
        if (*argc_in_out != 1) {
            Usage(*argc_in_out, (char const * const *) argv_in_out,
                  Messages, NrMessages);
            XtCloseDisplay(dpy);
            XtDestroyApplicationContext(app_con);
            Raise(InvalidArgs);
        }

        GetConverters();
        MyFixShell();

        num = 0;
        XtSetArg(args[num], XtNscreen, DefaultScreenOfDisplay(dpy)); num++;
        XtSetArg(args[num], XtNargc, saved_argc);	             num++;
        XtSetArg(args[num], XtNargv, argv_in_out);	             num++;

        merged_args = XtMergeArgLists(args_in, num_args_in, args, num);
        num += num_args_in;

        toplevel = XtAppCreateShell(NULL, (String) application_class,
                                    yShellWidgetClass, dpy, merged_args, num);

        if (app_context) *app_context = app_con;

        XtVaGetValues(toplevel,
                      XtNprintTree,      (XtArgVal) &printTree,
                      XtNprintResources, (XtArgVal) &printResources,
                      XtNdumpOnXError,   (XtArgVal) &dump,
                      NULL);
        if (dump != False) DumpOnIOError(app_con);
        InitWMProtocol(toplevel);

        if (CallBack) (*CallBack)(app_con);

        parent = WidgetToWitchet(toplevel);
        w = _MyCreateWidget("", parent, 1, merged_args, num);
        hash = w->Expander;
        w->Expander = parent->Expander;
        parent->Expander = hash;
        CallDestroyWitchet(0, (XtPointer) w, NULL);

        XtFree((XtPointer)merged_args);
        XtFree((XtPointer)argv_in_out);

        if (printResources != False) fprintResources(stdout, fb);
    } ON_UNWIND {
        myfree(fb);
    } END_UNWIND;
    if (printTree != False) fprintTree(stdout, toplevel);
    return toplevel;
}

static ExpansionHash *FindExpansion(Witchet Parent, const char *Name)
{
    ExpansionHash *Here;

    for (Here = Parent->Expander; Here; Here = Here->Next)
        if (0 == strcmp(Here->Tree->Name, Name)) return Here;
    return NULL;
}

static Witchet _MyCreateWidget(const char *Name, Witchet Parent,
                               int Managed, ArgList args, Cardinal num_args)
{
    Witchet         Result;
    ExpansionHash  *Expansion;
    TreeTemplate   *Tree;
    XtVarArgsList   AvList;
    int             count;

    Expansion = FindExpansion(Parent, Name);
    if (!Expansion) Raise1(NoCorresponding, Name);
    Tree = Expansion->Tree;
    InitTreeNrArgs(Tree);
    count = CountArgArray(args, num_args, Tree->HashOptions);

    Result = (Witchet) mymalloc(sizeof(_Witchet) + sizeof(Widget) *
                                (Tree->NrWidgetChildren-1));
    WITH_HANDLING {
        AllocTreeArgs(Tree);
        WITH_UNWIND {
            AvList = ArgArrayToDistributedList(args, num_args, count,
                                               Tree->HashOptions);
            WITH_UNWIND {
                ExpandTree(Expansion->Parent, Managed, Tree, Result,
                           (XtVarArgsList) AvList);
            } ON_UNWIND {
                myfree(AvList);
            } END_UNWIND;
        } ON_UNWIND {
            FreeTreeArgs(Tree);
        } END_UNWIND;
    } ON_EXCEPTION {
        myfree(Result);
    } END_HANDLING;
    return Result;
}

Widget MyCreateManagedWidget(const char *Name, Widget parent,
                             ArgList args, Cardinal num_args)
{
    Witchet         Result, Parent;

    Parent = WidgetToWitchet(parent);
    Result = _MyCreateWidget(Name, Parent, 1, args, num_args);
    XtAddCallback(Result->Widgets[0], XtNdestroyCallback,
                  CallDestroyWitchet, (XtPointer) Result);
    return Result->Widgets[0];
}

Widget MyCreateWidget(const char *Name, Widget parent,
                      ArgList args, Cardinal num_args)
{
    Witchet         Result, Parent;

    Parent = WidgetToWitchet(parent);
    Result = _MyCreateWidget(Name, Parent, 0, args, num_args);
    XtAddCallback(Result->Widgets[0], XtNdestroyCallback,
                  CallDestroyWitchet, (XtPointer) Result);
    return Result->Widgets[0];
}

Widget MyVaCreateManagedWidget(const char *Name, Widget parent, ...)
{
    int            count;
    va_list        var;
    Witchet        Result, Parent;
    ExpansionHash *Expansion;
    TreeTemplate  *Tree;
    XtVarArgsList  AvList;

    Parent = WidgetToWitchet(parent);

    Expansion = FindExpansion(Parent, Name);
    if (!Expansion) Raise1(NoCorresponding, Name);
    Tree = Expansion->Tree;
    InitTreeNrArgs(Tree);
    va_start(var, parent);
    count = CountVarArgNrArgs(var, Tree->HashOptions);
    va_end(var);

    Result = mynew(_Witchet);
    WITH_HANDLING {
        va_start(var, parent);
        WITH_UNWIND {
            AllocTreeArgs(Tree);
            WITH_UNWIND {
                AvList = VarArgsToDistributedList(var, count,
                                                  Tree->HashOptions);
                WITH_UNWIND {
                    ExpandTree(Expansion->Parent, 1, Tree, Result,
                               (XtVarArgsList) AvList);
                } ON_UNWIND {
                    myfree(AvList);
                } END_UNWIND;
            } ON_UNWIND {
                FreeTreeArgs(Tree);
            } END_UNWIND;
        } ON_UNWIND {
            va_end(var);
        } END_UNWIND;
    } ON_EXCEPTION {
        myfree(Result);
    } END_HANDLING;
    XtAddCallback(Result->Widgets[0], XtNdestroyCallback,
                  CallDestroyWitchet, (XtPointer) Result);
    return Result->Widgets[0];
}

Widget MyVaCreateWidget(const char *Name, Widget parent, ...)
{
    int            count;
    va_list        var;
    Witchet        Result, Parent;
    ExpansionHash *Expansion;
    TreeTemplate  *Tree;
    XtVarArgsList  AvList;

    Parent = WidgetToWitchet(parent);

    Expansion = FindExpansion(Parent, Name);
    if (!Expansion) Raise1(NoCorresponding, Name);
    Tree = Expansion->Tree;
    InitTreeNrArgs(Tree);
    va_start(var, parent);
    count = CountVarArgNrArgs(var, Tree->HashOptions);
    va_end(var);

    Result = mynew(_Witchet);
    WITH_HANDLING {
        va_start(var, parent);
        WITH_UNWIND {
            AllocTreeArgs(Tree);
            WITH_UNWIND {
                AvList = VarArgsToDistributedList(var, count,
                                                  Tree->HashOptions);
                WITH_UNWIND {
                    ExpandTree(Expansion->Parent, 0, Tree, Result,
                               (XtVarArgsList) AvList);
                } ON_UNWIND {
                    myfree(AvList);
                } END_UNWIND;
            } ON_UNWIND {
                FreeTreeArgs(Tree);
            } END_UNWIND;
        } ON_UNWIND {
            va_end(var);
       } END_UNWIND;
    } ON_EXCEPTION {
        myfree(Result);
    } END_HANDLING;
    XtAddCallback(Result->Widgets[0], XtNdestroyCallback,
                  CallDestroyWitchet, (XtPointer) Result);
    return Result->Widgets[0];
}

void MyRealizeWidgetNoPopup(Widget parent)
{
/*
    Witchet     w;
*/
    WidgetList Children;
    Cardinal   i, NrChildren;

/*
  If you start expanding this, remember the resourcetree should not be -Ton
    w = WidgetToWitchet(parent);
*/
    XtRealizeWidget(parent);
    if (XtIsShell(parent) != False) {
        XtVaGetValues(parent, XtNchildren, &Children,
                      XtNnumChildren, &NrChildren, NULL);
        for (i=0; i<NrChildren; i++)
            XtInstallAllAccelerators(Children[i], Children[i]);
        DeleteProtocol(parent);
    }
}

void MyRealizeWidget(Widget parent)
{
    MyRealizeWidgetNoPopup(parent);
    if (XtIsShell(parent) && XtParent(parent)) {
        XtPopup(parent, XtGrabNone);
    }
}
