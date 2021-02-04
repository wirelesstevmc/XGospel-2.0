%x ERROR

%{
#include <stdlib.h>
#include "mymalloc.h"
#include "widgettree.h"
#include "ywidgettree.tab.h"

#define YYTEXT   ((char *) yytext)
#define YYLENG   ((size_t) yyleng)
#define ECHO     (void) fwrite(YYTEXT, YYLENG, 1, yyout)

extern int   isatty(int fd);
extern int   yylex(void);

static void Insert(const char *Text);
static const char *TreeMacro(const char *Name);

typedef struct _macrostack_ {
    struct _macrostack_ *Next;
    YY_BUFFER_STATE State;
    const char     *Ptr;
    int             StrLength;
} MacroStack;

static MacroStack *Stack;
static MacroStack  StackBase = { NULL };

static Widget      ContextWidget;

/* Xrm.c trashes memory with resources bigger than BUFSIZ. So if we get
   a resource longer than that, we are in trouble */
#ifdef BUFSIZ
# define MAXRESLEN (BUFSIZ-10)
#else /* BUFSIZ */
# define MAXRESLEN (1024-10)
#endif /* BUFSIZ */

#define LENRESTEXT "very long resource (%d bytes): %.40s..."
static const char WidgetInfo[] =
    " widgetInfo.TopLevelShell.widgetInfo "
        "(collect.Paned "
            "(buttons.Box"
                "(restreeClose.Command"
                " parent.Command.parent"
                " children.Command.children"
                " popups.Command.popups"
                " change.Command"
                ")"
            " info.AsciiText"
            ")"
        ")";
static const char WidgetTree[] =
    " widgetTree.TopLevelShell.widgetTree "
        "(collect.Paned "
            "(buttons.Box "
                "(restreeClose.Command"
                ")"
            "viewport.Viewport"
                "(tree.Tree"
                ")"
            ")"
        ")";
static const char WidgetChange[] =
    " widgetChange.TopLevelShell.widgetChange "
        "(box.Box "
            "(cancel.Command"
            " ok.Command"
            " name.AsciiText  [editType.editType]"
            " value.AsciiText [editType.editType]"
            ")"
        ")";

#define YYTEXT  ((char *) yytext)
#define YYLENG  ((size_t) yyleng)
#undef YY_INPUT
#define YY_INPUT(buf, result, max_size)                             \
do {                                                                \
    if (max_size > Stack->StrLength) result = Stack->StrLength;     \
    else                             result = max_size;             \
    memcpy(buf, Stack->Ptr, (size_t) result);                       \
    Stack->Ptr += result;                                           \
    Stack->StrLength -= result;                                     \
} while(0)
#define YY_BREAK

TreeTemplate *TreeResult;
NameClassList NameBase;
extern int yyparse(void);
%}
WHITE             [ \t\f\v\r\n]
ID                [^ #\[\]().,\n\t\f\v\r]+
DIGIT             [0-9]
MACRO             (#{ID})
NUM               ({DIGIT}+)
%%
{ID}                          { yylval.Name = mystrndup(YYTEXT, YYLENG);
                                return NAME; }
{WHITE}                       { break; }
.                             { return YYTEXT[0]; }
"#WidgetInfo"                 { Insert(WidgetInfo);
                                break; }
"#WidgetTree"                 { Insert(WidgetTree);
                                break; }
"#WidgetChange"               { Insert(WidgetChange);
                                break; }
{MACRO}                       { const char *Result;
                                
                                Result = TreeMacro(YYTEXT+1);
                                if (Result) Insert(Result);
                                else
                                    WidgetWarning(ContextWidget, "Could not "
                                                  "find definition of macro "
                                                  "%.40s", YYTEXT+1);
                                break; }
<<EOF>>                       { MacroStack *Deeper;

                                if (Stack == &StackBase) yyterminate();
                                else {
                                    Deeper = Stack;
                                    Stack = Stack->Next;
                                    yy_delete_buffer(YY_CURRENT_BUFFER);
                                    yy_switch_to_buffer(Stack->State);
                                    myfree(Deeper);
                                }
                                break; }

<ERROR>(.|\n){0,40}           { yylval.Name = mystrndup(YYTEXT, YYLENG);
                                yymore();
                                return NAME; }
%%

static const char Empty[] = "text";
#define XtNtext ((String) Empty)
#define XtCText "Text"

typedef struct {
    String Text;
} MacroData, *MacroDataPtr;

#ifndef XtRString
# define XtRString "String"
#endif /* XtRString */

#define offset(field) XtOffset(MacroDataPtr, field)
static XtResource Modelresources[] = {
    { (String) XtNtext, (String) XtCText, (String) XtRString, sizeof(String),
      offset(Text), (String) XtRString, (XtPointer) NULL }, 
};
#undef offset

static const char *TreeMacro(const char *Name)
{
    MacroData macrodata;
    XtResource resource;

    resource = Modelresources[0];
    resource.resource_name = (String) Name;
    macrodata.Text = NULL;
    XtGetSubresources(ContextWidget, (XtPointer) &macrodata, "widgetTreeMacro",
                      "WidgetTreeMacro", &resource, 1, NULL, 0);
    return (const char *) macrodata.Text;
}

static void Insert(const char *Text)
{
    MacroStack *Deeper;
    Deeper = mynew(MacroStack);
    Deeper->Next = Stack;
    Deeper->Ptr  = Text;
                                
    Deeper->StrLength = strlen(Deeper->Ptr);
    if (Deeper->StrLength > MAXRESLEN)
        WidgetWarning(ContextWidget, LENRESTEXT,
                      Deeper->StrLength, Deeper->Ptr);
    Stack->State = YY_CURRENT_BUFFER;
    yy_switch_to_buffer(yy_create_buffer(0, YY_BUF_SIZE));
    Stack = Deeper;
}

TreeTemplate *treeParse(const NameClassList Base, Widget w,
                        const char *Str, int Len)
{
    MacroStack *Next;

    Stack = &StackBase;

    NameBase      = Base;
    ContextWidget = w;

    Stack->Ptr = Str;
    if (Len < 0) Stack->StrLength = strlen(Stack->Ptr);
    else         Stack->StrLength = Len;
    if (Stack->StrLength > MAXRESLEN)
        WidgetWarning(ContextWidget, LENRESTEXT, Stack->StrLength, Stack->Ptr);

    yy_init = 1;
    yyparse();
    while (Stack != &StackBase) {
        Next = Stack->Next;
        myfree(Stack);
        Stack = Next;
    }
    return TreeResult;
}

extern char *_TreeText(void);
char *_TreeText(void)
{
    static char Error[42];

    BEGIN(ERROR);
    yymore();
    yylex();
    if (YYLENG < sizeof(Error)) strcpy(Error, yylval.Name);
    else {
        memcpy(Error, yylval.Name, sizeof(Error)-1);
        Error[sizeof(Error)-1] = 0;
    }
    myfree(yylval.Name);
    return Error;
}

/****************************************** flex fun **********************/
/*
void yy_flex_free(void *Data)
{
    myfree(Data);
}

void *yy_flex_alloc(int size)
{
    return mymalloc(size);
}

void *yy_flex_realloc(void *ptr, int size)
{
    return myrealloc(ptr, size);
}
*/

int yywrap(void)
{
    return 1;
}

/* Kludge to shut up some compiler warnings for some versions of flex */
#include "except.h"
static void yy_flex_strcpy(char *s1, const char *s2);

static void loopy(void)
{
    yy_flex_strcpy(NULL, NULL);
}

static void yy_flex_strcpy(char *s1, const char *s2)
{
    Raise1(AssertException, "flex kludge function called");
#ifndef YY_NO_PUSH_STATE
    yy_push_state(s1 == s2);
#endif
#ifndef YY_NO_POP_STATE
    yy_pop_state();
#endif
#ifndef YY_NO_TOP_STATE
    yy_top_state();
#endif
    unput(' ');
    loopy();
}
