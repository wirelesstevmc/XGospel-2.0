/**********************************************************/
/* utils.c: Several utility functions used in the program */
/*                                                        */
/* Author:  Ton Hospel                                    */
/*          ton@linux.cc.kuleuven.ac.be                   */
/*          (1993, 1994, 1995)                            */
/*                                                        */
/* Copyright: GNU copyleft                                */
/**********************************************************/

#include <myxlib.h>
#include <mymalloc.h>
#include <except.h>

#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <X11/Xaw/AsciiText.h>
#include <X11/Xaw/SimpleMenu.h>
#include <X11/Xaw/Toggle.h>

#include <stdlib.h>

#include "SmeBell.h"

#include "utils.h"
#include "xgospel.h"

static void EndText(     Widget w, XEvent *event, String *string, Cardinal *n);
static void SetText(     Widget w, XEvent *event, String *string, Cardinal *n);
static void NextText(    Widget w, XEvent *event, String *string, Cardinal *n);
static void PreviousText(Widget w, XEvent *event, String *string, Cardinal *n);

static XtActionsRec actionTable[] = {
    { (String) "asknexttext",     NextText     },
    { (String) "askprevioustext", PreviousText },
    { (String) "askendtext",      EndText      },
    { (String) "asksettext",      SetText      },
};

/***************************************************************************/

static Exception OpenException = { "Could not open file",
                                    0, ErrnoExceptionAction };

FILE *OpenWrite(const char *FileName, Boolean Overwrite)
{
    FILE *fp;

    /* Very naive but portable test */
    if (Overwrite == False && (fp = fopen(FileName, "r")) != NULL) {
        fclose(fp);
        return NULL;
    }

    fp = fopen(FileName, "w");
    if (!fp) Raise2(OpenException, ExceptionCopy(FileName), "for write");
    return fp;
}

XtPointer SaveWrite(const char *FileName, Boolean Overwrite,
                    Widget Bell, Widget Raize, const char *ErrorType,
                    XtPointer (*WriteFun)(FILE *fp, XtPointer Closure),
                    XtPointer Closure)
{
    FILE       *fp;
    const char *Error;
    XtPointer   Result;

    Result = NULL;
    Error  = NULL;
    WITH_HANDLING {
        fp = OpenWrite(FileName, Overwrite);
        if (fp)
            WITH_UNWIND {
                Result = WriteFun(fp, Closure);
            } ON_UNWIND {
                if (ExceptionPending()) {
                    if (ferror(fp)) {
                        Error = strerrno();
                        ClearException();
                    }
                    fclose(fp);
                } else if (fclose(fp)) Error = strerrno();

                if (Error) {
                    IfBell(Bell);
                    IfRaise(Raize, Raize);
                    PopMessage(ErrorType, "Could not write file %s: %s",
                               FileName, Error);
                }
            } END_UNWIND
        else {
            IfBell(Bell);
            IfRaise(Raize, Raize);
            PopMessage(ErrorType,
                       "Will not overwrite existing file %s",
                       FileName);
        }
    } ON_EXCEPTION {
        if (ExceptionP(OpenException)) {
            ClearException();
            IfBell(Bell);
            IfRaise(Raize, Raize);
            PopMessage(ErrorType, "Could not open file %s: %s",
                       FileName, StrExceptionErrno());
        } /* otherwise -> ReRaise(); */
    } END_HANDLING;
    return Result;
}

XtPointer SaveTextFun(FILE *fp, XtPointer Closure)
{
    return (XtPointer) MySaveText(fp, (Widget) Closure);
}

static Boolean ReturnTrue(String Name)
{
    return True;
}

#ifndef HAVE_NO_STDARG_H
char *StringToFilename(const char *Filename, ...)
#else  /* HAVE_NO_STDARG_H */
char *StringToFilename(va_alist)
va_dcl
#endif /* HAVE_NO_STDARG_H */
{
    va_list          Args;
    Cardinal         NrSubs;
    SubstitutionRec *Subs, *Sub;
    char            *Result, *Temp;

#ifndef HAVE_NO_STDARG_H
    va_start(Args, Filename);
#else  /* HAVE_NO_STDARG_H */
    char *Filename;
    
    va_start(Args);
    Filename = va_arg(Args, char **);
#endif /* HAVE_NO_STDARG_H */
    for (NrSubs = 2; va_arg(Args, int); NrSubs++) (void) va_arg(Args, char *);
    va_end(Args);

    Subs = mynews(SubstitutionRec, NrSubs);

#ifndef HAVE_NO_STDARG_H
    va_start(Args, Filename);
#else  /* HAVE_NO_STDARG_H */
    va_start(Args);
    Filename = va_arg(Args, char **);
#endif /* HAVE_NO_STDARG_H */
    for (Sub = Subs; (Sub->match = va_arg(Args, int)) != 0; Sub++)
        Sub->substitution = va_arg(Args, char *);
    va_end(Args);
    Sub->match        = 'D';
    Sub->substitution = appdata.Directory;
    Sub++;
    Sub->match        = 'd';
    Sub->substitution = appdata.Directory;
    Temp = XtFindFile((char *) Filename, Subs, NrSubs, ReturnTrue);
    myfree(Subs);
    Result = mystrdup(Temp);
    XtFree(Temp);

    return Result;
}

#ifndef HAVE_NO_STDARG_H
Widget ChangeFilename(Widget Toplevel, const char *Type, const char *Title,
                      char **Filename, ...)
#else  /* HAVE_NO_STDARG_H */
Widget ChangeFilename(Toplevel, Type, Title, va_alist)
Widget Toplevel;
const char *Type;
const char *Title;
va_dcl
#endif /* HAVE_NO_STDARG_H */
{
    va_list       args;
    int           count;
    XtVarArgsList Args;
    Widget        Root;

#ifndef HAVE_NO_STDARG_H
    va_start(args, Filename);
#else  /* HAVE_NO_STDARG_H */
    char **Filename;
    
    va_start(args);
    FileName = va_arg(args, char **);
#endif /* HAVE_NO_STDARG_H */
    count = CountVarArgs(args);
    va_end(args);
#ifndef HAVE_NO_STDARG_H
    va_start(args, Filename);
#else  /* HAVE_NO_STDARG_H */
    char **Filename;
    
    va_start(args);
    FileName = va_arg(args, char **);
#endif /* HAVE_NO_STDARG_H */
    Args = VarArgsToList(args, count);
    va_end(args);

    Root = AskString(Toplevel, 0, NULL, Title, Type, Filename, Args, NULL);
    myfree(Args);
    return Root;
}

void ChangeSgfFilename(Widget w,
                       XtPointer clientdata, XtPointer calldata)
{
    char **Filename;
    Widget Root;

    Filename = (char **) clientdata;
    Root = ChangeFilename(toplevel, "sgfFilename", "Enter name of sgf file",
                          Filename, "filename", *Filename, NULL);
    MyDependsOn(Root, w);
}

void ChangePsFilename(Widget w,
                      XtPointer clientdata, XtPointer calldata)
{
    char **Filename;
    Widget Root;

    Filename = (char **) clientdata;
    Root = ChangeFilename(toplevel, "psFilename",
                          "Enter name of postscript file",
                          Filename, "filename", *Filename, NULL);
    MyDependsOn(Root, w);
}

void ChangeKibitzFilename(Widget w,
                          XtPointer clientdata, XtPointer calldata)
{
    char **Filename;
    Widget Root;

    Filename = (char **) clientdata;
    Root = ChangeFilename(toplevel, "kibitzFilename",
                          "Enter name of kibitzfile",
                          Filename, "filename", *Filename, NULL);
    MyDependsOn(Root, w);
}

/***************************************************************************/

void IfBell(Widget w)
{
    Boolean Beep;

    if (w)
        if (XtIsSubclass(w, smeBellObjectClass) != False) SmeBell(w);
        else {
            Beep = True;
            XtVaGetValues(w, XtNstate, (XtArgVal) &Beep, NULL);
            if (Beep != False) XBell(XtDisplayOfObject(w), 20);
        }
    else XBell(XtDisplay(toplevel), 20);
}

void IfRaise(Widget w, Widget root)
{
    Boolean Raise;
    Widget  Here;

    if (w) {
        Raise = False;
        XtVaGetValues(w, XtNstate, (XtArgVal) &Raise, NULL);
        if (Raise == False) return;
        for (Here = root; Here; Here = XtParent(Here))
            if (XtIsShell(Here) != False &&
                XtIsSubclass(Here, simpleMenuWidgetClass) == False) {
                XtPopup(Here, XtGrabNone);
                return;
            }
    }
    XBell(XtDisplay(toplevel), 20);
}
/***************************************************************************/

#ifndef   HAVE_NO_STDARG_H
Widget PopMessage(const char *Title, const char *Format, ...)
#else  /* HAVE_NO_STDARG_H */
Widget PopMessage(Title, va_alist)
const char *Title;
va_dcl
#endif /* HAVE_NO_STDARG_H */
{
    va_list rest;
    char    Text[2048];
    Widget  Root;

#ifndef   HAVE_NO_STDARG_H
    va_start(rest, Format);
#else  /* HAVE_NO_STDARG_H */
    const char *Format;

    va_start(rest);
    Format = va_arg(rest, const char *);
#endif /* HAVE_NO_STDARG_H */
    vsprintf(Text, Format, rest);
    va_end(rest);
    Root = MyVaCreateManagedWidget("popMessage", toplevel,
                                   "text", Text, "title", Title, NULL);
    MyRealizeWidget(Root);
    return Root;
}

/***************************************************************************/

typedef struct {
    String Title, IconName;
} IconTitle, *IconTitlePtr;

#define offset(field) XtOffset(IconTitlePtr, field)
static XtResource resources[] = {
    { XtNtitle, XtCTitle, XtRString, sizeof(String),
      offset(Title), XtRString, NULL }, 
    { XtNiconName, XtCIconName, XtRString, sizeof(String),
      offset(IconName), XtRString, NULL }, 
};
#undef offset

void SetWidgetTitles(Widget w,
                     char *(*fun)(const char *Pattern, XtPointer Closure),
                     XtPointer Closure)
{
    IconTitle  iconTitle;
    char      *Title, *IconName;

    XtGetApplicationResources(w, &iconTitle, resources, XtNumber(resources),
                              NULL, 0);
    Title = (*fun)(iconTitle.Title,    Closure);
    WITH_UNWIND {
        IconName = (*fun)(iconTitle.IconName, Closure);
        XtVaSetValues(w,
                      XtNtitle,    (XtArgVal) Title,
                      XtNiconName, (XtArgVal) IconName,
                      NULL);
        myfree(IconName);
    } ON_UNWIND {
        myfree(Title);
    } END_UNWIND;
}

typedef struct {
    String Name;
} PropName, *PropNamePtr;

#define offset(field) XtOffset(PropNamePtr, field)
static XtResource PResources[] = {
    { NULL, NULL, XtRString, sizeof(String),
      offset(Name), XtRString, NULL }, 
};
#undef offset

void SetWidgetProperty(Widget w, const char *Name, const char *Class,
		       char *(*fun)(const char *Pattern, XtPointer Closure),
		       XtPointer Closure)
{
    PropName Prop;
    char    *PName;

    PResources[0].resource_name  = (String) Name;
    PResources[0].resource_class = (String) Class;
    XtGetApplicationResources(w, &Prop, PResources, XtNumber(PResources),
                              NULL, 0);
    PName = (*fun)(Prop.Name, Closure);
    XtVaSetValues(w, (String) Name, (XtArgVal) PName, NULL);
    myfree(PName);
}

/***************************************************************************/

#define MAXSIZE 25

int ConvertSize(const char *Name, size_t *Target, XtPointer Closure, int Free)
{
    char **Result, *ptr;
    size_t Size;
    int    rc;

    rc = 0;
    Result = (char **) Closure;
    if (*Result) {
        Size = strtol(*Result, &ptr, 0);
        if (*ptr || ptr == *Result)
            PopMessage(Name, "'%.40s' is not an integer", *Result);
        else if (1 >= Size || Size > MAXSIZE)
            PopMessage(Name, "Size must be between 2 and %d", MAXSIZE);
        else {
            rc = *Target != Size;
            *Target = Size;
        }
        if (Free) {
            myfree(*Result);
            *Result = NULL;
        }
    } else PopMessage(Name, "no value has been entered");
    return rc;
}

int ConvertPositive(const char *Name, int *Target, XtPointer Closure, int Free)
{
    char **Result, *ptr;
    int    Value, rc;

    rc = 0;
    Result = (char **) Closure;
    if (*Result) {
        Value = strtol(*Result, &ptr, 0);
        if (*ptr || ptr == *Result)
            PopMessage(Name, "'%.40s' is not an integer", *Result);
        else if (Value < 1) PopMessage(Name, "Value must be greater than 0");
        else {
            rc = *Target != Value;
            *Target = Value;
        }
        if (Free) {
            myfree(*Result);
            *Result = NULL;
        }
    } else PopMessage(Name, "no value has been entered");
    return rc;
}

int ConvertNatural(const char *Name, int *Target, XtPointer Closure, int Free)
{
    char **Result, *ptr;
    int    Value, rc;

    rc = 0;
    Result = (char **) Closure;
    if (*Result) {
        Value = strtol(*Result, &ptr, 0);
        if (*ptr || ptr == *Result)
            PopMessage(Name, "'%.40s' is not an integer", *Result);
        else if (Value < 0)
            PopMessage(Name, "Value must be greater or equal than 0");
        else {
            rc = *Target != Value;
            *Target = Value;
        }
        if (Free) {
            myfree(*Result);
            *Result = NULL;
        }
    } else PopMessage(Name, "no value has been entered");
    return rc;
}

int ConvertBoolean(const char *Name, Boolean *Target,
                   XtPointer Closure, int Free)
{
    int      rc;
    char   **Result;
    XrmValue src, dst;

    rc = 0;
    Result = (char **) Closure;
    if (*Result) {
        src.size = strlen(*Result)+1;
        src.addr = (XPointer) *Result;
        dst.size = 0;
        dst.addr = NULL;

        if (XtConvertAndStore(toplevel,
                              XtRString, &src, XtRBoolean, &dst) != False) {
            rc = *Target != *(Boolean *) dst.addr;
            *Target = *(Boolean *) dst.addr;
        } else PopMessage(Name, "'%.40s' is not a boolean", *Result);
        if (Free) {
            myfree(*Result);
            *Result = NULL;
        }
    } else PopMessage(Name, "no value has been entered");
    return rc;
}

/**************************************/
/* Regular expression yes/no function */
/**************************************/

void EmptyRegexYesNo(RegexYesNo *Pattern)
{
    Pattern->NrYesPatterns = Pattern->NrNoPatterns = -1;
    Pattern->YesPatterns   = Pattern->NoPatterns   = NULL;
}

/* This function assumes there is something to free */
void FreeRegexYesNo(RegexYesNo *Convert)
{
    int i;
    regex_t *Here;

    Here = Convert->YesPatterns;
    for (i=Convert->NrYesPatterns+1; i>0; i--, Here++) REGPREFIX(free)(Here);
    myfree(Convert->YesPatterns);
    Convert->YesPatterns = NULL;
    Convert->NrYesPatterns = -1;

    Here = Convert->NoPatterns;
    for (i=Convert->NrNoPatterns+1; i>0; i--, Here++)  REGPREFIX(free)(Here);
    myfree(Convert->NoPatterns);
    Convert->NoPatterns = NULL;
    Convert->NrNoPatterns = -1;
}

const char *CompileRegexYesNo(struct _StringList *YesBase,
                              struct _StringList *NoBase,
                              RegexYesNo *Compiled)
{
    int i, Nr, Length, error;
    size_t  size;
    char  Buffer[100], *Pat, *Ptr;
    regex_t *Here;
    static char Error[300];
    RegexYesNo  New;

    New.NrYesPatterns = New.NrNoPatterns = -1;
    New.YesPatterns   = New.NoPatterns   = NULL;

    if (YesBase && YesBase->Nr) {
        Nr   = YesBase->Nr;
        Length = Nr*3;
        Here = New.YesPatterns = mynews(regex_t, Nr+1);
        for (i = 0; i<Nr; i++, Here++) {
            error = REGPREFIX(comp)(Here, YesBase->String[i],
                                    REG_EXTENDED | REG_NEWLINE | REG_NOSUB);
            if (error) {
                size = REGPREFIX(error)(error, Here, Buffer, sizeof(Buffer));
                sprintf(Error, "Regular expression '%.100s': %.180s%s",
                        YesBase->String[i], Buffer,
                        size > sizeof(Buffer) ? "..." : "");
                New.NrYesPatterns = i-1;
                FreeRegexYesNo(&New);
                return Error;
            }
            Length += YesBase->Length[i];
        }
        Pat = Ptr = mynews(char, Length);
        for (i = 0; i<Nr; i++) {
            *Ptr++ = '(';
            memcpy(Ptr, YesBase->String[i], YesBase->Length[i]);
            Ptr += YesBase->Length[i];
            *Ptr++ = ')';
            *Ptr++ = '|';
        }
        Ptr[-1] = 0;
        error = REGPREFIX(comp)(Here, Pat,
                                REG_EXTENDED | REG_NEWLINE | REG_NOSUB);
        New.NrYesPatterns = Nr;
        if (error) {
            size = REGPREFIX(error)(error, Here, Buffer, sizeof(Buffer));
            sprintf(Error, "Combined regular expression '%.100s': %.180s%s",
                    Pat, Buffer, size > sizeof(Buffer) ? "..." : "");
            New.NrYesPatterns--;
            FreeRegexYesNo(&New);
            myfree(Pat);
            return Error;
        }
        myfree(Pat);
    }

    if (NoBase && NoBase->Nr) {
        Nr   = NoBase->Nr;
        Length = Nr*3;
        Here = New.NoPatterns = mynews(regex_t, Nr+1);
        for (i = 0; i<Nr; i++, Here++) {
            error = REGPREFIX(comp)(Here, NoBase->String[i],
                                    REG_EXTENDED | REG_NEWLINE | REG_NOSUB);
            if (error) {
                size = REGPREFIX(error)(error, Here, Buffer, sizeof(Buffer));
                sprintf(Error, "Regular expression '%.100s': %.180s%s",
                        NoBase->String[i], Buffer,
                        size > sizeof(Buffer) ? "..." : "");
                New.NrNoPatterns = i-1;
                FreeRegexYesNo(&New);
                return Error;
            }
            Length += NoBase->Length[i];
        }
        Pat = Ptr = mynews(char, Length);
        for (i = 0; i<Nr; i++) {
            *Ptr++ = '(';
            memcpy(Ptr, NoBase->String[i], NoBase->Length[i]);
            Ptr += NoBase->Length[i];
            *Ptr++ = ')';
            *Ptr++ = '|';
        }
        Ptr[-1] = 0;
        error = REGPREFIX(comp)(Here, Pat,
                                REG_EXTENDED | REG_NEWLINE | REG_NOSUB);
        New.NrNoPatterns = Nr;
        if (error) {
            size = REGPREFIX(error)(error, Here, Buffer, sizeof(Buffer));
            sprintf(Error, "Combined regular expression '%.100s': %.180s%s",
                    Pat, Buffer, size > sizeof(Buffer) ? "..." : "");
            New.NrNoPatterns--;
            FreeRegexYesNo(&New);
            myfree(Pat);
            return Error;
        }
        myfree(Pat);
    }

    FreeRegexYesNo(Compiled);
    *Compiled = New;
    return NULL;
}

const char *TestRegexYesNo(const char *Text, RegexYesNo *Pattern,
                           int *Yes, int *No)
{
    int error;
    size_t size;
    char Buffer[100];
    static char Error[300];

    if (Yes) {
        *Yes = 1;
        if (Pattern->YesPatterns) {
            error =
                REGPREFIX(exec)(&Pattern->YesPatterns[Pattern->NrYesPatterns],
                                Text, 0, NULL, 0);
            if (error == REG_NOMATCH) *Yes = 0;
            else if (error) {
                size = REGPREFIX(error)
                    (error, &Pattern->YesPatterns[Pattern->NrYesPatterns],
                     Buffer, sizeof(Buffer));
                sprintf(Error, "Test string '%.100s': %.180s%s",
                        Text, Buffer, size > sizeof(Buffer) ? "..." : "");
                return Error;
            }
        }
    }

    if (No) {
        *No = 0;
        if (Pattern->NoPatterns) {
            error =
                REGPREFIX(exec)(&Pattern->NoPatterns[Pattern->NrNoPatterns],
                                Text, 0, NULL, 0);
            if (error == REG_NOMATCH) /* nothing */ ;
            else if (error) {
                size = REGPREFIX(error)
                    (error, &Pattern->NoPatterns[Pattern->NrNoPatterns],
                     Buffer, sizeof(Buffer));
                sprintf(Error, "Test string '%.100s': %.180s%s",
                        Text, Buffer, size > sizeof(Buffer) ? "..." : "");
                return Error;
            } else *No = 1;
        }
    }

    return NULL;
}

typedef struct {
    StringList  **Kill, **Pass;
    int          Which;
    Widget       Root;
    const char  *Error;
    RegexYesNo  *Pattern;
} EditStringData;

static void CallStringList(Widget w, XtPointer clientdata, XtPointer calldata)
{
    EditStringData *Data;
    XrmValue        src, dst;
    StringList     *Result, **ToEdit;
    String          text;
    Widget          Text;
    const char     *Error;

    Data = (EditStringData *) clientdata;
    Text = XtNameToWidget(Data->Root, "*text");
    if (Text) {
        XtVaGetValues(Text, XtNstring, (XtArgVal) &text, NULL);

        src.size = strlen(text)+1;
        src.addr = (XPointer) text;
        dst.size = sizeof(Result);
        dst.addr = (XPointer) &Result;
    
        if (XtConvertAndStore(Data->Root,
                              XtRString, &src, XtRStringList, &dst) != False) {
            if (Data->Which)  {
                Error = CompileRegexYesNo(*Data->Pass, Result, Data->Pattern);
                ToEdit = Data->Kill;
            } else {
                Error = CompileRegexYesNo(Result, *Data->Kill, Data->Pattern);
                ToEdit = Data->Pass;
            }
            
            if (Error) {
                PopMessage(Data->Error,
                           "Error while compiling regex pattern: %s", Error);
                MyFreeStringList(Result);
            } else {
                MyFreeStringList(*ToEdit);
                *ToEdit = Result;
                XtDestroyWidget(Data->Root);
            }
        } else PopMessage(Data->Error, "'%s' is not a valid pattern", text);
    }
}

void EditStringList(Widget w, const char *Title, const char *Error,
                    struct _StringList **Pass, struct _StringList **Kill,
                    RegexYesNo *Pattern, int which)
{
    char           *text, *Ptr;
    size_t          Length;
    int             i;
    EditStringData *Data;
    Widget          Root, Ok, Collect, Text;
    StringList     *ToEdit;

    ToEdit = *(which ? Kill : Pass);
    Length = 1;
    if (ToEdit) for (i=0; i<ToEdit->Nr; i++) Length += ToEdit->Length[i]+1;
    Ptr = text = mynews(char, Length);
    if (ToEdit) 
        for (i=0; i<ToEdit->Nr; i++) {
            Length = ToEdit->Length[i];
            memcpy(Ptr, ToEdit->String[i], Length);
            Ptr += Length;
            *Ptr++ = '\n';
        }
    *Ptr = 0;
    WITH_UNWIND {
        Root = MyVaCreateManagedWidget("stringList", toplevel,
                                       XtNtitle,    (XtArgVal) Title,
                                       XtNiconName, (XtArgVal) Title,
                                       "text", text,
                                       NULL);
    } ON_UNWIND {
        myfree(text);
    } END_UNWIND;
    Data = mynew(EditStringData);
    WITH_HANDLING {
        Data->Pass    = Pass;
        Data->Kill    = Kill;
        Data->Which   = which;
        Data->Error   = Error;
        Data->Pattern = Pattern;
        Data->Root    = Root;

        XtAddCallback(Root, XtNdestroyCallback, CallFree, (XtPointer)Data);
        Ok = XtNameToWidget(Root, "*ok");
        if (Ok) XtAddCallback(Ok,XtNcallback,
                              CallStringList, (XtPointer) Data);
        MyDependsOn(Root, w);
        Text    = XtNameToWidget(Root, "*text");
        if (Text) RelaxText(Text);
        if (!Text) WidgetWarning(Root, "EditStringlist called on tree "
                                 "without text widget");
        Collect = XtNameToWidget(Root, "*collect");
        if (Collect && Text) XtSetKeyboardFocus(Collect, Text);
        MyRealizeWidget(Root);
    } ON_EXCEPTION {
        myfree(Data);
    } END_HANDLING;
}

/***************************************************************************/

typedef struct {
    void    (*Callback)(XtPointer);
    XtPointer Closure;
    int       Count;
    Widget    Collect, Root;
} AskData;

typedef struct _AskAddress_ {
    struct _AskAddress_ *Next, *Previous;
    AskData             *askdata;
    char               **Address;
    Widget               Text;
} AskAddress;

void InitUtils(Widget TopLevel)
{
    XtAppAddActions(XtWidgetToApplicationContext(TopLevel),
                    actionTable, XtNumber(actionTable));
}

static void CallSetString(Widget w, XtPointer clientdata, XtPointer calldata)
{
    String text;
    char *Address;
    AskAddress *askaddress;
    AskData    *askdata;

    XtVaGetValues(w, XtNstring, &text, NULL);
    askaddress = (AskAddress *) clientdata;
    Address = *askaddress->Address;
    WITH_UNWIND {
        *askaddress->Address = mystrdup(text);
    } ON_UNWIND {
        myfree(Address);
        askdata = askaddress->askdata;
        if (0 == --askdata->Count) {
            if (askdata->Callback) (*askdata->Callback)(askdata->Closure);
            myfree(askdata);
        }
        myfree(askaddress);
    } END_UNWIND;
}

static void EndText(Widget w, XEvent *event, String *string, Cardinal *n)
{
    XtPointer client_data;

    if (FindCallback(w, XtNdestroyCallback, CallSetString, &client_data))
        XtDestroyWidget(((AskAddress *) client_data)->askdata->Root);
    else WidgetWarning(w, "endtext() called on invalid widget");
}

static void SetText(Widget w, XEvent *event, String *string, Cardinal *n)
{
    XtPointer client_data;
    AskAddress  *askaddress, *Here;
    Widget       Text, Collect;

    if (FindCallback(w, XtNdestroyCallback, CallSetString, &client_data)) {
        askaddress = (AskAddress *) client_data;
        Collect    = askaddress->askdata->Collect;
        if (*n > 0) {
            Text = XtNameToWidget(Collect, string[0]);
            if (!Text) Text = w;
        } else Text = w;
        if (Text) {
            XawTextDisplayCaret(askaddress->Text, False);
            for (Here=askaddress->Next; Here != askaddress; Here = Here->Next)
                XawTextDisplayCaret(Here->Text, False);
            XtSetKeyboardFocus(Collect, Text);
            XawTextDisplayCaret(Text, True);
        } else XtDestroyWidget(askaddress->askdata->Root);
    } else WidgetWarning(w, "settext() called on invalid widget");
}

static void NextText(Widget w, XEvent *event, String *string, Cardinal *n)
{
    XtPointer client_data;
    AskAddress  *askaddress;
    Widget       Nexttext, Collect;

    if (FindCallback(w, XtNdestroyCallback, CallSetString, &client_data)) {
        askaddress = (AskAddress *) client_data;
        Collect    = askaddress->askdata->Collect;
        if (*n > 0) {
            Nexttext = XtNameToWidget(Collect, string[0]);
            if (!Nexttext) Nexttext = askaddress->Next->Text;
        } else Nexttext = askaddress->Next->Text;
        if (Nexttext) {
            XawTextDisplayCaret(w, False);
            XtSetKeyboardFocus(Collect, Nexttext);
            XawTextDisplayCaret(Nexttext, True);
        } else XtDestroyWidget(askaddress->askdata->Root);
    } else WidgetWarning(w, "nexttext() called on invalid widget");
}

static void PreviousText(Widget w, XEvent *event, String *string, Cardinal *n)
{
    XtPointer client_data;
    AskAddress  *askaddress;
    Widget       Previoustext, Collect;

    if (FindCallback(w, XtNdestroyCallback, CallSetString, &client_data)) {
        askaddress = (AskAddress *) client_data;
        Collect    = askaddress->askdata->Collect;
        if (*n > 0) {
            Previoustext = XtNameToWidget(Collect, string[0]);
            if (!Previoustext) Previoustext = askaddress->Previous->Text;
        } else Previoustext = askaddress->Previous->Text;
        if (Previoustext) {
            XawTextDisplayCaret(w, False);
            XtSetKeyboardFocus(Collect, Previoustext);
            XawTextDisplayCaret(Previoustext, True);
        } else XtDestroyWidget(askaddress->askdata->Root);
    } else WidgetWarning(w, "previoustext() called on invalid widget");
}

/*
   Popup a widget, child of TopLevel. create with title resource set to title
   If the last subwidget is destroyed, call Callback with Closure as argument
   Each of the ... is a set of subwidgetdescriptions of format:
     char *name, char **Result, XtVarArgsList Args
   ended with NULL.

   For each of these a MyCreateManagedWidget(name, Args) is executed.
   If this thing contains a textwidget named text, it will start up with the
   text determined by resources (if not NULL), and at the end it will free the
   old value and reallocate a new one and fill it in in Result.
*/

#ifndef HAVE_NO_STDARG_H
Widget AskString(Widget TopLevel, void (*Callback)(XtPointer),
                 XtPointer Closure, const char *title, ...)
#else  /* HAVE_NO_STDARG_H */
Widget AskString(Widget TopLevel, Callback, Closure, va_alist)
void (*Callback)(XtPointer);
XtPointer Closure;
va_dcl
#endif /* HAVE_NO_STDARG_H */
{
    va_list args;
    const char   *Name;
    char        **Address;
    XtVarArgsList Args;
    Widget        Root, collect, entry, text;
    AskData      *askdata;
    AskAddress   *askaddress, *oldaskaddress, *oldestaskaddress;

#ifndef HAVE_NO_STDARG_H
    askdata = mynew(AskData);
    WITH_HANDLING {
        va_start(args, title);
        WITH_UNWIND {
#else  /* HAVE_NO_STDARG_H */
    askdata = mynew(Askdata);
    WITH_HANDLING {
        const char *title;

        va_start(args);
        WITH_UNWIND {
            title = va_arg(args, const char *);
#endif /* HAVE_NO_STDARG_H */
            Root =
                MyVaCreateManagedWidget("askString", TopLevel,
                                        "title", (XtArgVal) title,
                                        NULL);
            askdata->Callback = Callback;
            askdata->Count    = 0;
            askdata->Root     = Root;
            askdata->Closure  = Closure;
            WITH_HANDLING {
                askaddress = oldestaskaddress = NULL;
                while ((Name = va_arg(args, const char *)) != NULL) {
                    Address = va_arg(args, char **);
                    if (Address) *Address = NULL;
                    Args = va_arg(args, XtVarArgsList);
                    if (Args)
                        entry = MyVaCreateManagedWidget(Name, Root,
                                                        XtVaNestedList, Args,
                                                        NULL);
                    else entry = MyVaCreateManagedWidget(Name, Root, NULL);
                    text  = XtNameToWidget(entry, "*text");

                    if (text) {
                        AddText(text, ""); /* Forces resize */
                        XawTextSetInsertionPoint(text,
                            XawTextSourceScan(XawTextGetSource(text), 0,
                                              XawstAll, XawsdRight, 1, True));
                        if (Address) {
                            oldaskaddress = askaddress;
                            askaddress = mynew(AskAddress);
                            askaddress->askdata = askdata;
                            askaddress->Address = Address;
                            askaddress->Text    = text;
                            askdata->Count++;
                            if (oldaskaddress) {
                                oldaskaddress->Next  = askaddress;
                                askaddress->Previous = oldaskaddress;
                            } else oldestaskaddress = askaddress;
                            XtAddCallback(text, XtNdestroyCallback,
                                          CallSetString,
                                          (XtPointer) askaddress);
                        }
                    }
                }
                if (askaddress) {
                    askaddress->Next = oldestaskaddress;
                    oldestaskaddress->Previous = askaddress;
                }
                if ((collect = XtNameToWidget(Root, "*collect")) != 0 &&
                    oldestaskaddress) {
                    XtSetKeyboardFocus(collect, oldestaskaddress->Text);
                    XawTextDisplayCaret(oldestaskaddress->Text, True);
                }
                askdata->Collect = collect;
                MyRealizeWidget(Root);
            } ON_EXCEPTION {
                XtDestroyWidget(Root);
            } END_HANDLING;
        } ON_UNWIND {
            va_end(args);
        } END_UNWIND;
    } ON_EXCEPTION {
        myfree(askdata);
    } END_HANDLING;
    return Root;
}
