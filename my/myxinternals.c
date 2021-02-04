#include "myxlib.h"
#include <X11/IntrinsicP.h>
#include <X11/CoreP.h>

#include "except.h"
#include "mymalloc.h"

/*****************************************************************************/
/*                                                                           */
/*****************************************************************************/

#define _XtGetPerDisplay(display) \
    ((_XtperDisplayList != NULL && (_XtperDisplayList->dpy == (display))) \
     ? &_XtperDisplayList->perDpy \
     : _XtSortPerDisplayList(display))

typedef struct _CaseConverterRec *CaseConverterPtr;
typedef struct _CaseConverterRec {
    KeySym		start;		/* first KeySym valid in converter */
    KeySym		stop;		/* last KeySym valid in converter */
    XtCaseProc		proc;		/* case converter function */
    CaseConverterPtr	next;		/* next converter record */
} CaseConverterRec;

typedef struct _ModToKeysymTable {
    Modifiers mask;
    int count;
    int idx;
} ModToKeysymTable;

typedef struct {
    char*	start;
    char*	current;
    int		bytes_remaining;
} Heap;

typedef struct _XtGrabRec  *XtGrabList;
typedef enum {
    XtNoServerGrab, 
    XtPassiveServerGrab,
    XtActiveServerGrab,
    XtPseudoPassiveServerGrab,
    XtPseudoActiveServerGrab
} XtServerGrabType;

typedef struct _XtServerGrabRec {
    struct _XtServerGrabRec 	*next;
    Widget			widget;
    unsigned int		ownerEvents:1;
    unsigned int		pointerMode:1;
    unsigned int		keyboardMode:1;
    unsigned int		hasExt:1;
    KeyCode			keybut;
    unsigned short		modifiers;
    unsigned short		eventMask;
} XtServerGrabRec, *XtServerGrabPtr;

typedef struct _XtDeviceRec {
    XtServerGrabRec	grab; 	/* need copy in order to protect during grab */
    XtServerGrabType	grabType;
} XtDeviceRec, *XtDevice;

typedef struct XtPerDisplayInputRec{
    XtGrabList 	grabList;
    XtDeviceRec keyboard, pointer;
    KeyCode	activatingKey;
    Widget 	*trace;
    int		traceDepth, traceMax;
    Widget 	focusWidget;
} XtPerDisplayInputRec, *XtPerDisplayInput;

typedef struct _XtPerDisplayStruct {
    InternalCallbackList destroy_callbacks;
    Region region;
    CaseConverterPtr case_cvt;		/* user-registered case converters */
    XtKeyProc defaultKeycodeTranslator;
    XtAppContext appContext;
    unsigned long keysyms_serial;      /* for tracking MappingNotify events */
    KeySym *keysyms;                   /* keycode to keysym table */
    int keysyms_per_keycode;           /* number of keysyms for each keycode*/
    int min_keycode, max_keycode;      /* range of keycodes */
    KeySym *modKeysyms;                /* keysym values for modToKeysysm */
    ModToKeysymTable *modsToKeysyms;   /* modifiers to Keysysms index table*/
    unsigned char isModifier[32];      /* key-is-modifier-p bit table */
    KeySym lock_meaning;	       /* Lock modifier meaning */
    Modifiers mode_switch;	       /* keyboard group modifiers */
    Boolean being_destroyed;
    Boolean rv;			       /* reverse_video resource */
    XrmName name;		       /* resolved app name */
    XrmClass class;		       /* application class */
    Heap heap;
    struct _GCrec *GClist;	       /* support for XtGetGC */
    Drawable **pixmap_tab;             /* ditto for XtGetGC */
    String language;		       /* XPG language string */
    Time last_timestamp;	       /* from last event dispatched */
    int multi_click_time;	       /* for XtSetMultiClickTime */
    struct _TMKeyContextRec* tm_context;     /* for XtGetActionKeysym */
    InternalCallbackList mapping_callbacks;  /* special case for TM */
    XtPerDisplayInputRec pdi;	       /* state for modal grabs & kbd focus */
    struct _WWTable *WWtable;	       /* window to widget table */
    XrmDatabase *per_screen_db;        /* per screen resource databases */
    XrmDatabase cmd_db;		       /* db from command line, if needed */
    XrmDatabase server_db;	       /* resource property else .Xdefaults */
} XtPerDisplayStruct, *XtPerDisplay;

typedef struct _PerDisplayTable {
	Display *dpy;
	XtPerDisplayStruct perDpy;
	struct _PerDisplayTable *next;
} PerDisplayTable, *PerDisplayTablePtr;

extern PerDisplayTablePtr _XtperDisplayList;
extern XtPerDisplay _XtSortPerDisplayList(Display * dpy);

int ReverseP(Display *dpy)
{
    XtPerDisplay pd;

    pd = _XtGetPerDisplay(dpy);
    return pd->rv;
}

/******************/
/* MyNameToWidget */
/******************/

static Widget NameListToWidget(Widget root, Widget *fail, XrmNameList names,
                               XrmBindingList bindings,
                               int in_depth, int *out_depth, int *found_depth);

typedef Widget (*NameMatchProc)(Widget *fail, XrmNameList names,
                                XrmBindingList bindings,
                                WidgetList children, Cardinal num,
                                int in_depth,int *out_depth, int *found_depth);
typedef unsigned long Signature;
typedef unsigned char XrmBits;
extern XrmQuark _XrmInternalStringToQuark(const char *name, register int len,
                                          Signature sig, Bool permstring);

#define MAXWIDGETDEPTH 10000

#define BSLASH                  ((XrmBits) (1 << 5))
#define NORMAL	                ((XrmBits) (1 << 4))
#define EOQ	                ((XrmBits) (1 << 3))
#define SEP	                ((XrmBits) (1 << 2))
#define ENDOF	                ((XrmBits) (1 << 1))
#define SPACE	                (NORMAL|EOQ|SEP|(XrmBits)0)
#define RSEP	                (NORMAL|EOQ|SEP|(XrmBits)1)
#define EOS	                (EOQ|SEP|ENDOF|(XrmBits)0)
#define EOL	                (EOQ|SEP|ENDOF|(XrmBits)1)
#define BINDING	                (NORMAL|EOQ)
#define ODIGIT	                (NORMAL|(XrmBits)1)

#define next_char(ch,str)       xrmtypes[(unsigned char)((ch) = *(++(str)))]
#define is_EOF(bits)		((bits) == EOS)
#define is_binding(bits)	((bits) == BINDING)

/* parsing types */
static const XrmBits xrmtypes[256] = {
    EOS,    0,      0,      0,      0,      0,      0,      0,
    0,      SPACE,  EOL,    0,      0,      0,      0,      0,
    0,      0,      0,      0,      0,      0,      0,      0,
    0,      0,      0,      0,      0,      0,      0,      0,
    SPACE,  NORMAL, NORMAL, NORMAL, NORMAL, NORMAL, NORMAL, NORMAL,
    NORMAL, NORMAL, BINDING,NORMAL, NORMAL, NORMAL, BINDING,NORMAL,
    ODIGIT, ODIGIT, ODIGIT, ODIGIT, ODIGIT, ODIGIT, ODIGIT, ODIGIT,
    NORMAL, NORMAL, RSEP,   NORMAL, NORMAL, NORMAL, NORMAL, NORMAL,
    NORMAL, NORMAL, NORMAL, NORMAL, NORMAL, NORMAL, NORMAL, NORMAL,
    NORMAL, NORMAL, NORMAL, NORMAL, NORMAL, NORMAL, NORMAL, NORMAL,
    NORMAL, NORMAL, NORMAL, NORMAL, NORMAL, NORMAL, NORMAL, NORMAL,
    NORMAL, NORMAL, NORMAL, NORMAL, BSLASH, NORMAL, NORMAL, NORMAL,
    NORMAL, NORMAL, NORMAL, NORMAL, NORMAL, NORMAL, NORMAL, NORMAL,
    NORMAL, NORMAL, NORMAL, NORMAL, NORMAL, NORMAL, NORMAL, NORMAL,
    NORMAL, NORMAL, NORMAL, NORMAL, NORMAL, NORMAL, NORMAL, NORMAL,
    NORMAL, NORMAL, NORMAL, NORMAL, NORMAL, NORMAL, NORMAL, 0,
    /* The rest will be automatically initialized to zero. */
};

void myStringToBindingQuarkList(const char *name, XrmBindingList bindings,
                                XrmQuarkList quarks)
{
    XrmBits     bits;
    Signature   sig;
    char        ch;
    const char *tname;
    XrmBinding  binding;
    int         i;

    sig = 0;
    i   = 0;
    ch  = 0;
    tname = name;
    if (tname) {
	tname--;
	binding = XrmBindTightly;
	while (!is_EOF(bits = next_char(ch, tname))) {
	    if (is_binding (bits)) {
		if (i) {
		    /* Found a complete name */
		    *bindings++ = binding;
		    *quarks++ = _XrmInternalStringToQuark(name, tname - name,
							  sig, False);

		    i = 0;
		    sig = 0;
		    binding = XrmBindTightly;
		}
		name = tname+1;

		if (ch == '*') binding = XrmBindLoosely;
	    } else {
		sig = (sig << 1) + ch; /* Compute the signature. */
		i++;
	    }
	}
	*bindings = binding;
	*quarks++ = _XrmInternalStringToQuark(name, tname - name, sig, False);
    }
    *quarks = NULLQUARK;
}

/* Order for fail widgets is absolutely wrong. Can loop !!
   It will however work in simple cases --Ton */
static Widget MatchExactChildren(Widget *fail, XrmNameList names,
                                 XrmBindingList bindings,
                                 WidgetList children, Cardinal num,
                                 int in_depth,int *out_depth, int *found_depth)
{
    Cardinal   i;
    XrmName    name = *names;
    Widget     w, result = NULL;
    int        d, mini;

    mini = MAXWIDGETDEPTH;
    for (i = 0; i < num; i++) {
	if (name == children[i]->core.xrm_name ||
            name == children[i]->core.widget_class->core_class.xrm_class) {
	    w = NameListToWidget(children[i], fail, &names[1], &bindings[1],
                                 in_depth+1, &d, found_depth);
	    if (w != NULL && d < mini) {
                result = w;
                mini = d;
            }
	}
    }
    *out_depth = mini;
    return result;
}

static Widget MatchWildChildren(Widget *fail, XrmNameList names,
                                XrmBindingList bindings,
                                WidgetList children, Cardinal num,
                                int in_depth, int *out_depth, int *found_depth)
{
    Cardinal i;
    Widget   w, result = NULL;
    int      d, mini;

    mini = MAXWIDGETDEPTH;
    for (i = 0; i < num; i++) {
	w = NameListToWidget(children[i], fail, names, bindings,
                             in_depth+1, &d, found_depth);
	if (w != NULL && d < mini) {
            result = w;
            mini = d;
        }
    }
    *out_depth = mini;
    return result;
}

static Widget SearchChildren(Widget root, Widget *fail, XrmNameList names,
                             XrmBindingList bindings, NameMatchProc matchproc,
                             int in_depth, int *out_depth, int *found_depth)
{
    Widget w1, w2, infail;
    int    d1, d2;

    if (XtIsComposite(root)) {
	w1 = (*matchproc)(fail, names, bindings,
                          ((CompositeWidget) root)->composite.children,
                          ((CompositeWidget) root)->composite.num_children,
                          in_depth, &d1, found_depth);
        infail = *fail;
    } else {
        d1 = MAXWIDGETDEPTH;
        w1 = 0;
        infail = NULL;
    }

    w2 = (*matchproc)(fail, names, bindings, root->core.popup_list,
                      root->core.num_popups, in_depth, &d2, found_depth);
    if (infail && !w2 && *fail == NULL) /* w2 would have found it */
	w1 = (*matchproc)(fail, names, bindings,
                          ((CompositeWidget) root)->composite.children,
                          ((CompositeWidget) root)->composite.num_children,
                          in_depth, &d1, found_depth);
    *out_depth = (d1 < d2 ? d1 : d2);
    return (d1 < d2 ? w1 : w2);
}

static Widget NameListToWidget(Widget root, Widget *fail, XrmNameList names,
                               XrmBindingList bindings,
                               int in_depth, int *out_depth, int *found_depth)
{
    Widget w1, w2, infail;
    int    d1, d2;

    if (*fail == NULL && in_depth >= *found_depth) {
	*out_depth = MAXWIDGETDEPTH;
	return NULL;
    }

    if (names[0] == NULLQUARK)
        if (*fail == NULL) {
            *out_depth = *found_depth = in_depth;
            return root;
        } else if (*fail == root) *fail = NULL;

    if (! XtIsWidget(root)) {
	*out_depth = MAXWIDGETDEPTH;
	return NULL;
    }

    if (*bindings == XrmBindTightly)
	return SearchChildren(root, fail, names, bindings, MatchExactChildren,
                              in_depth, out_depth, found_depth);

    else {                      /* XrmBindLoosely */
	w1 = SearchChildren(root, fail, names, bindings, MatchExactChildren,
                            in_depth, &d1, found_depth);
        infail = *fail;
	w2 = SearchChildren(root, fail, names, bindings, MatchWildChildren,
                            in_depth, &d2, found_depth);
        if (infail && !w2 && *fail == NULL) /* w2 would have found it */
            w1 = SearchChildren(root, fail, names, bindings,MatchExactChildren,
                                in_depth, &d1, found_depth);
	*out_depth = (d1 < d2 ? d1 : d2);
	return (d1 < d2 ? w1 : w2);
    }
}                               /* NameListToWidget */

/*****************************************************************************/

Widget MyNameToWidget(Widget root, const char *name)
{
    XrmName    *names;
    XrmBinding *bindings;
    int         len, depth, found = MAXWIDGETDEPTH;
    Widget      result;
    static Widget fail = NULL;

    if (!name)  return NULL;
    if (!*name) return root;
    len = strlen(name);

    result = NULL;
    names = mynews(XrmName, len+1);
    WITH_UNWIND {
        bindings = mynews(XrmBinding, len+1);
        WITH_UNWIND {
            XrmStringToBindingQuarkList((String) name, bindings, names);
            if (names[0] != NULLQUARK) 
                result =
                    NameListToWidget(root, &fail, names, bindings,
                                     0, &depth, &found);
        } ON_UNWIND {
            myfree(bindings);
        } END_UNWIND;
    } ON_UNWIND {
        myfree(names);
    } END_UNWIND;
    return result;
}

Widget FindNextWidget(Widget root, Widget from, const char *name)
{
    XrmName    *names;
    XrmBinding *bindings;
    int         len, depth, found = MAXWIDGETDEPTH;
    Widget      result, fail;

    if (!name)  return NULL;
    if (!*name) return root;
    len = strlen(name);

    result = NULL;
    names = mynews(XrmName, len+1);
    WITH_UNWIND {
        bindings = mynews(XrmBinding, len+1);
        WITH_UNWIND {
            XrmStringToBindingQuarkList((String) name, bindings, names);
            if (names[0] != NULLQUARK) {
                fail = from;
                result = NameListToWidget(root, &fail, names, bindings,
                                          0, &depth, &found);
            }
        } ON_UNWIND {
            myfree(bindings);
        } END_UNWIND;
    } ON_UNWIND {
        myfree(names);
    } END_UNWIND;
    return result;
}

/*****************************************************************************/

#define StringToQuark(string) XrmStringToQuark(string)

typedef XrmResource **CallbackTable;
typedef struct internalCallbackRec {
    unsigned short count;
    char	   is_padded;	/* contains NULL padding for external form */
    char	   call_state;  /* combination of _XtCB{FreeAfter}Calling */
    /* XtCallbackList */
} InternalCallbackRec;

InternalCallbackList *FetchInternalList(Widget widget, String name)
{
    XrmQuark      quark;
    int 	  n;
    CallbackTable offsets;

    quark = StringToQuark(name);
    offsets = (CallbackTable) 
	widget->core.widget_class->core_class.callback_private;

    for (n = (int) (long) *(offsets++); --n >= 0; offsets++)
	if (quark == (*offsets)->xrm_name)
	    return (InternalCallbackList *) 
		((char *) widget - (*offsets)->xrm_offset - 1);
    return NULL;
}
