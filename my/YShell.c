#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>

#include <string.h>

#include "YShellP.h"
#include "mymalloc.h"
#include "except.h"

#define offset(field) XtOffset(YShellRec *, yShell.field)
static XtResource resources[] = {
    { (String) XtNvisual, (String) XtCVisual, XtRVisual, sizeof(Visual *),
      offset(visual), XtRImmediate, NULL},
    { (String) XtNdepth, (String) XtCDepth, XtRInt, sizeof(int),
      offset(depth), XtRImmediate, NULL},
    { (String) XtNnameClassList, (String) XtCNameClassList,
          (String) XtRNameClassList, sizeof(NameClassList),
      offset(class_names), XtRImmediate, NULL },
    { (String) XtNwidgetTree, (String) XtCWidgetTree, XtRString,sizeof(String),
      offset(tree), XtRString, (XtPointer) "No widgettree given" },
    { (String) XtNtopName, (String) XtCTopName, XtRString, sizeof(String),
      offset(top_name), XtRString, (XtPointer) "default" },
    { (String) XtNprintResources, (String) XtCPrintYShell, XtRBoolean,
          sizeof(Boolean),
      offset(print_resources), XtRBoolean, False },
    { (String) XtNprintTree, (String) XtCPrintYShell, XtRBoolean,
          sizeof(Boolean),
      offset(print_tree), XtRBoolean, False },
    { (String) XtNdumpOnXError, (String) XtCDumpOnXError, XtRBoolean,
          sizeof(Boolean),
      offset(dump_on_X_error), XtRBoolean, False },
    { (String) XtNresourceTree, (String) XtCWidgetTree, XtRString,
          sizeof(String),
      offset(res_tree), XtRString, (XtPointer)
          "(resourceTree.TopLevelShell "
              /* You can find these expansions in lwidgettree.l */
              "(#WidgetInfo"
              " #WidgetTree"
              " #WidgetChange"
              ")"
           " widgetHelp.OverrideShell "
              "( text.Label"
              ")"
          ")"
    }
};
#undef offset

/*
 * Semi Public function definitions. 
 */

static void Initialize(Widget request, Widget new), Destroy(Widget old);

/*
 *
 */

static TreeTemplate *FindCorresponding(TreeTemplate *Where, const char *Name);
static void HashOptions(TreeTemplate *To, TreeTemplate *From);

static CompositeClassExtensionRec compositeClassExtension = {
    /* next_extension	*/	    NULL,
    /* record_type	*/	    NULLQUARK,
    /* version		*/	    XtCompositeExtensionVersion,
    /* record_size	*/	    sizeof(CompositeClassExtensionRec),
    /* accepts_objects	*/	    TRUE,
#if XlibSpecificationRelease > 5
    /* allows_change_managed_set */ FALSE
#endif /* XlibSpecificationRelease */
};

#define superclass (&applicationShellClassRec)
YShellClassRec yShellClassRec = {
  { /* core fields */
    /* superclass		*/	(WidgetClass) superclass,
    /* class_name		*/	(String) "YShell",
    /* widget_size		*/	sizeof(YShellRec),
    /* class_initialize		*/	NULL,
    /* class_part_initialize	*/	NULL,
    /* class_inited		*/	FALSE,
    /* initialize		*/	(XtInitProc) Initialize,
    /* initialize_hook		*/	NULL,
    /* realize			*/	XtInheritRealize,
    /* actions			*/	NULL,
    /* num_actions		*/	0,
    /* resources		*/	resources,
    /* num_resources		*/	XtNumber(resources),
    /* xrm_class		*/	NULLQUARK,
    /* compress_motion		*/	TRUE,
    /* compress_exposure	*/	TRUE,
    /* compress_enterleave	*/	TRUE,
    /* visible_interest		*/	FALSE,
    /* destroy			*/	Destroy,
    /* resize			*/	XtInheritResize,
    /* expose			*/	NULL,
    /* set_values		*/	NULL,
    /* set_values_hook		*/	NULL,
    /* set_values_almost	*/	XtInheritSetValuesAlmost,
    /* get_values_hook		*/	NULL,
    /* accept_focus		*/	NULL,
    /* version			*/	XtVersion,
    /* callback_private		*/	NULL,
    /* tm_table			*/	XtInheritTranslations,
    /* query_geometry		*/	XtInheritQueryGeometry,
    /* display_accelerator	*/	XtInheritDisplayAccelerator,
    /* extension		*/	NULL
  },{ /* composite fields */
    /* geometry_manager   */    XtInheritGeometryManager,
    /* change_managed     */    XtInheritChangeManaged,
    /* insert_child	  */	XtInheritInsertChild,
    /* delete_child	  */	XtInheritDeleteChild,
    /* extension	  */	(XtPointer)&compositeClassExtension
  },{ /* Shellfields */
    /* extension	  */	NULL
  },{ /* wmShell fields */
    /* extension	  */	NULL
  },{ /* vendorShell fields */
    /* extension	  */	NULL
  },{ /* topLevelShell fields */
    /* extension	  */	NULL
  },{ /* applicationShell fields */
    /* extension	  */	NULL
  },{ /* yShell fields */
    /* extension		*/	NULL
  }
};

WidgetClass yShellWidgetClass = (WidgetClass)&yShellClassRec;

/************************************************************
 *
 * Semi-Public Functions.
 *
 ************************************************************/

/*      Function Name: Initialize
 *      Description: Initializes the YShellWidget
 *      Arguments: request - the widget requested by the argument list.
 *                 new     - the new widget with both resource and non
 *                           resource values.
 *      Returns: none.
 */

/* ARGSUSED */
static void Initialize(Widget request, Widget new)
{
    YShellWidget   w;
    TreeTemplate  *Fill, *Found, *Here, *Use;
    Witchet        witchet;
    ExpansionHash *expansion;

    w = (YShellWidget) new;

    if (!w->yShell.class_names) w->yShell.class_names = InitClassList();
    Fill  = NULL;
    Found = treeParse(w->yShell.class_names, (Widget) w, w->yShell.tree, -1);
    WITH_HANDLING {
        w->yShell.tree_shape = Fill = MakeTemplate("", NULL);
        Fill->Children = Found;
        if (Found) Fill->NrWidgetChildren = Found->NrWidgetChildren;
    } ON_EXCEPTION {
        FreeTemplate(Found);
    } END_HANDLING;
    WITH_HANDLING {
        /* Make fake witchet and expander entry */
        witchet = mynew(_Witchet);
        witchet->Expander   = NULL;
        witchet->Where      = Fill;
        witchet->Widgets[0] = new;
        WITH_HANDLING {
            expansion = mynew(ExpansionHash);
            expansion->Next   = NULL;
            expansion->Parent = new;
            expansion->Tree   = Fill;
            witchet->Expander = expansion;

            Here = treeParse(w->yShell.class_names, (Widget) w,
                             w->yShell.res_tree, -1);
            if (Here) Fill->NrWidgetChildren += Here->NrWidgetChildren;
            Use = Fill->Children;
            if (Use) {
                Use->Previous->Next = Here->Next;
                Here->Next->Previous = Use->Previous;
                Here->Previous->Next = Use;
                Use->Previous = Here->Previous;
                /* Decouple and free */
                Here->Previous = Here->Next = Here;
                FreeTemplate(Here);
            } else Fill->Children = Here;

            HashOptions(Fill, Fill);
            w->yShell.context = MyAllocContext();
            XtAddCallback(new, XtNdestroyCallback,
                          CallDestroyWitchet, (XtPointer) witchet);
        } ON_EXCEPTION {
            CallDestroyWitchet(NULL, (XtPointer) witchet, NULL);
        } END_HANDLING;
    } ON_EXCEPTION {
        FreeTemplate(Fill);
    } END_HANDLING;
    w->yShell.tree_use = Found;
}

/*      Function Name: Destroy
 *      Description: Called at destroy time, cleans up.
 *      Arguments: w - the y Shell widget.
 *      Returns: none.
 */

static void Destroy(Widget new)
{
    YShellWidget w;

    w = (YShellWidget) new;
    MyFreeContext(w->yShell.context);
    FreeClassList(w->yShell.class_names);
    FreeTemplate(w->yShell.tree_shape);
}

static TreeTemplate *FindCorresponding(TreeTemplate *Where, const char *Name)
{
    TreeTemplate *Here, *Found;

    if (Where)
        for (Here= Where->Next; Here != Where; Here = Here->Next) {
            if (!Here->Class && (0 == strcmp(Here->Name, Name))) return Here;
            Found = FindCorresponding(Here->Children, Name);
            if (Found) return Found;
        }
    return NULL;
}

/*
extern void _CallFreeTemplate(Widget w,
                              XtPointer clientdata, XtPointer calldata);
static void UnParse(Widget w)
{
    InternalCallbackList *callbacks;
    XtCallbackList        calls;
    XtCallbackProc        fun;
    
    callbacks = FetchInternalList(w, XtNdestroyCallback);
    if (callbacks) {
        for (calls = _XtGetCallbackList(callbacks);
             (fun = calls->callback) != 0; calls++)
            if (fun == CallFreeTemplate) {
                FreeTemplate((TreeTemplate *) calls->closure);
                XtRemoveCallback(w, XtNdestroyCallback,
                                 CallFreeTemplate, calls->closure);
                break;
            }
}
*/

static void HashOptions(TreeTemplate *To, TreeTemplate *From)
{
    TreeTemplate     *Here;
    TreeTemplateList *Tree;
    OptionTemplate   *Option;
    OptionHash      **Hash;
    int rc;

    if (!From->Class && To != From) HashOptions(From, From);
    else {
        for (Option = From->Options; Option; Option = Option->Next) {
            Hash = &To->HashOptions;
            while (*Hash) {
                rc = strcmp((*Hash)->WitchetName,
                            Option->WitchetName);
                if      (rc < 0) Hash = &(*Hash)->Right;
                else if (rc > 0) Hash = &(*Hash)->Left;
                else             goto found;
            }
            *Hash = mynew(OptionHash);
            (*Hash)->Left = (*Hash)->Right = NULL;
            (*Hash)->WitchetName = Option->WitchetName;
            (*Hash)->Positions = NULL;
          found:
            Tree = mynew(TreeTemplateList);
            Tree->Option = Option;
            Tree->Pos = From;
            Tree->Next = (*Hash)->Positions;
            (*Hash)->Positions = Tree;
        }
        From = From->Children;
        if (From)
            for (Here = From->Next; Here != From; Here = Here->Next)
                HashOptions(To, Here);
    }
}

const char *ApplicationClassName(Widget w)
{
    Widget Next;

    while ((Next = XtParent(w)) != 0) w = Next;
    return ((ApplicationShellWidget) w)->application.class;
}

MyContext YShellContext(Widget w)
{
    return ((YShellWidget) w)->yShell.context;
}

/*********************************************************************/
#include <X11/Xmu/Converters.h>

static void MyVendorShellClassInitialize(void)
{
    static XtConvertArgRec ScreenConvertArg[] = {
        {XtWidgetBaseOffset, (XtPointer) XtOffsetOf(WidgetRec, core.screen),
	     sizeof(Screen *)}
    };

    XtAddConverter(XtRString, XtRCursor, XmuCvtStringToCursor,
		   ScreenConvertArg, XtNumber(ScreenConvertArg));
/*
    XtAddConverter(XtRString, XtRBitmap, XmuCvtStringToBitmap,
		   ScreenConvertArg, XtNumber(ScreenConvertArg));
*/
}

/*
   To be called before the first Shell widget. Gives alternate StringToCursor
   converter
 */
void MyFixShell(void)
{
    vendorShellClassRec.core_class.class_initialize =
        MyVendorShellClassInitialize;
}
