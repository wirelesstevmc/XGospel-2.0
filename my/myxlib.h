#ifndef _MYXLIB_H
#define _MYXLIB_H

# ifdef	__GNUC__
/* Figure out how to declare functions that (1) depend only on their
   parameters and have no side effects, or (2) don't return.  */
#  if __GNUC__ < 2 || (__GNUC__ == 2 && __GNUC_MINOR__ < 5) /* Old GCC way. */
#   define       __PRINTF2
#  else                                                     /* New GCC way. */
#   define       __PRINTF2     __attribute__ ((format (printf, 2, 3)))
#  endif
# else	/* Not GCC.  */
#  define        __PRINTF2
# endif	/* GCC.  */

#define XTPOINTER_TO_INT(x) ((int)(long) (x))
#define INT_TO_XTPOINTER(x) ((XtPointer) (long) (x))

# include <stdio.h>
# include <stddef.h>
# include <X11/Intrinsic.h>

# define DeleteProtocol(shell)                                 \
    XSetWMProtocols(XtDisplay(shell), XtWindow(shell),         \
                    ProtocolList, XtNumber(ProtocolList))

extern Atom ProtocolList[1];

/**************************************************/
/* New types for the new converters               */
/**************************************************/

#define XtRStringList     "StringList"
#define XtRStringPairList "StringPairList"

typedef struct _StringList {
    int     Nr;
    size_t *Length;
    char  **String;
} StringList, *StringListPtr;

typedef struct _StringPairList {
    int     Nr;
    size_t *Length1,  *Length2;
    char  **String1, **String2;
} StringPairList, *StringPairListPtr;

extern void MyFreeStringList(    StringList     *target);
extern void MyFreeStringPairList(StringPairList *target);

/**************************************************/
/* Converting a resource argument to a given type */
/**************************************************/

extern const char *ResourceStringToString(const char *From);
extern int         ResourceStringToInt(   const char *From);

/****************************/
/* Special context function */
/****************************/

typedef struct _MyContext *MyContext;

extern MyContext MyAllocContext(void);
extern void MyFreeContext(MyContext Context);
extern void MySaveContext(MyContext Context, Widget w,
                          XrmQuark Name, XtPointer Value);
extern int  MyFindContext(MyContext Context, Widget w,
                          XrmQuark Name, XtPointer *Value);
extern void MyDeleteContext(MyContext Context, Widget w, XrmQuark Name);
extern void MyQSaveContext(MyContext Context, XtPointer q,
                           XrmQuark Name, XtPointer Value);
extern int  MyQFindContext(MyContext Context, XtPointer q,
                           XrmQuark Name, XtPointer *Value);
extern void MyQDeleteContext(MyContext Context, XtPointer q, XrmQuark Name);

/********************************************/
/* Some callbacks that might be interesting */
/********************************************/

extern void CallDestroy(Widget w, XtPointer clientdata, XtPointer calldata);
extern void CallFree(Widget w, XtPointer clientdata, XtPointer calldata);
extern void CallReadToggle(Widget w, XtPointer clientdata, XtPointer calldata);
extern void CallToggleOn(Widget w, XtPointer clientdata, XtPointer calldata);
extern void CallToggleOff(Widget w, XtPointer clientdata, XtPointer calldata);
extern void CallPopup(Widget w, XtPointer clientdata, XtPointer calldata);
extern void CallPopdown(Widget w, XtPointer clientdata, XtPointer calldata);
extern void CallToggleUpDown(Widget w, XtPointer clientdata,
                             XtPointer calldata);
extern void CallDestroyWidgetReference(Widget w, XtPointer clientdata,
                                       XtPointer calldata);
extern void CallPrint(Widget w, XtPointer clientdata, XtPointer calldata);
extern void CallAllowShellResize(Widget w, XtPointer clientdata,
                                 XtPointer calldata);
extern void CallAllowResize(Widget w, XtPointer clientdata, XtPointer calldata);
extern void CallFakeChildDestroy(Widget w, XtPointer clientdata,
                                 XtPointer calldata);

/******************************/
/* Access to internal X stuff */
/******************************/

typedef struct internalCallbackRec *InternalCallbackList;

extern InternalCallbackList *FetchInternalList(Widget widget, String name);
extern int                   ReverseP(Display *dpy);
extern String MyPrintXlations(Widget w, XtTranslations xlations,
                              Widget accelWidget, int includeRHS);
extern void MyDisplayTranslations(Widget widget, XEvent *event,
                                  String *params, Cardinal *num_params);
extern void MyDisplayAccelerators(Widget widget, XEvent *event,
                                  String *params, Cardinal *num_params);
extern void MyDisplayInstalledAccelerators(Widget widget, XEvent *event,
                                           String *params,
                                           Cardinal *num_params);

/*********************************/
/* Some general widget functions */
/*********************************/

# ifndef   HAVE_NO_STDARG_H
#  include <stdarg.h>
extern void      RelaxText(Widget w);
extern void      AddText(Widget w, const char *Format, ...)       __PRINTF2;
extern size_t    BatchAddText(Widget w, const char *Format, ...)  __PRINTF2;
extern int       CountVarArgs (va_list var);
extern XtVarArgsList VarArgsToList(va_list var, int count);
extern void      WidgetWarning(Widget w, const char *Format, ...) __PRINTF2;
# else  /* HAVE_NO_STDARG_H */
#  include <varargs.h>
extern void      AddText();
extern size_t    BatchAddText();
extern int       CountVarArgs ();
extern XtVarArgsList VarArgsToList();
extern void      WidgetWarning();
# endif /* HAVE_NO_STDARG_H */
extern XtVarArgsList ArgArrayToList(ArgList args, Cardinal num_args);
extern long      MySaveText(FILE *fp, Widget w);
extern void      InitTextBatch(void);
extern void      CleanTextBatch(void);
extern void      BatchAppendText(Widget w, const char *Text, size_t Length,
                                 int Mode);
extern void      ForceText(Widget w);
extern char     *TextWidgetCursorLine(Widget w);
extern int       AppendText(Widget w, const char *Text, size_t Length);
extern void      PrintVarArgsList(FILE *fp, XtVarArgsList List);

extern void      CoupleToggleWidget(Widget Root, Widget Toggle,
                                    Widget (*InitFun)(XtPointer Closure),
                                    XtPointer Closure);

extern void      myStringToBindingQuarkList(const char *name,
                                            XrmBindingList bindings,
                                            XrmQuarkList quarks);
extern const char *ApplicationClassName(Widget w);
extern void      NaturalWidgetSize(Widget w);
extern Atom      GetAtom(Widget w, const char *name);
extern void      AddFakeChild(Widget Parent, Widget Child);
extern void      VaSetManagementChildren(Widget first, ...);
extern void      SetManagementChildren(Widget *Manage,   int NrManage,
                                       Widget *UnManage, int NrUnManage);
extern void      MyScrollbarSetThumb(Widget w, double top, double shown);
extern void      NaturalSize(Widget w);
extern void      MyDependsOn(Widget w1, Widget w2);
extern Widget    MyNameToWidget(Widget root, const char *name);
extern Widget    FindNextWidget(Widget root, Widget from, const char *name);
extern void      DumpOnIOError(XtAppContext app_con);
extern void      Xexit(XtAppContext App, int status);
extern void      Xquit(XtAppContext App, XtPointer status);
extern XtPointer MyAppMainLoop(XtAppContext app_context);
extern void      Usage(int argc, char const*const* argv,
                       char const*const*Messages, int NrMessages);
extern void      GetConverters(void);
extern void      InitWMProtocol(Widget top);
extern void      InfoOfWidget(Widget w);
extern int       FindCallback(Widget w, String CallbackName,
                               XtCallbackProc callback, XtPointer *client_data);
extern char     *FullWidgetName(char *ptr, Widget w);
extern Boolean   MyIsPopped(Widget object);
extern Boolean   MyIsSubclass(WidgetClass Here, WidgetClass Check);
#define MySuperclass(x) (*(WidgetClass *)(x))

/*********************/
/* Witchet functions */
/*********************/

extern const char MyNname[];
extern const char MyCName[];
extern const char MyNclass[];
extern const char MyCClass[];
extern const char MyNrealized[];
extern const char MyCRealized[];

extern void   fprintTree(FILE *fp, Widget w);
extern Widget MyCreateManagedWidget(const char *Name, Widget Parent,
                                    ArgList args, Cardinal num_args);
extern Widget MyCreateWidget(const char *Name, Widget Parent,
                             ArgList args, Cardinal num_args);
extern Widget MyVaCreateManagedWidget(const char *Name, Widget Parent, ...);
extern Widget MyVaCreateWidget(const char *Name, Widget parent, ...);
extern Widget MyAppInitialize(XtAppContext *app_context,
                              const char *application_class,
                              XrmOptionDescList options, Cardinal num_options,
                              int *argc_in_out, String * argv_in_out,
                              String *fallback_resources, ArgList args,
                              Cardinal num_args,
                              char const*const*Messages, int NrMessages,
                              void (*CallBack)(XtAppContext app));
extern Widget MyInitialize(Widget top, ArgList args, Cardinal num_args);
extern void   MyRealizeWidget(Widget parent);
extern void   MyRealizeWidgetNoPopup(Widget parent);
extern Widget WitchetOfWidget(Widget w);
extern void NameClassArg(Arg *arg, const char *Name, ...);
# undef __PRINTF2
#endif /* _MYXLIB_H */
