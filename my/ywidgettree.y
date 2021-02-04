%{
#include <stdlib.h>

#include "mymalloc.h"
#include "except.h"
#include "widgettree.h"

#include <X11/Shell.h>

#define YYDEBUG		1
#define YYERROR_VERBOSE
#define YYOVERFLOW MyOverflow(x1)
#define	xmalloc	mymalloc
/* Kludge to get rid of yy_bcopy warnings --Ton */
#ifndef __GNUC__
# define __GNUC__ 2
#endif /* __GNUC__ */

static void yyerror(const char *s);
static void MyOverflow(const char *Text);

extern int           yylex(void);
extern char         *_TreeText(void);
extern TreeTemplate *TreeResult;

static Exception InvalidClass    = {"Class is unknown to the program:"};
static Exception InvalidTemplate = {"This does not look like a treetemplate:"};
%}

%union {
    char           *Name;
    TreeTemplate   *Tree;
    OptionTemplate *Options;
}

%token  <Name>    NAME

%type   <Tree>    init witchetlist witchet children
%type   <Options> option options
%%

init    : children
            {
                TreeResult = $1;
            }
        ;

witchetlist: witchetlist witchet
            {
                $2->Next     = $1;
                $2->Previous = $1->Previous;
                $2->Previous->Next = $2->Next->Previous = $2;
                if ($2->Class) $1->NrWidgetChildren++;
                $$ = $1;
            }
        | witchet
            {
                $$ = MakeTemplate("", NULL);
                $$->Next = $$->Previous = $1;
                $1->Next = $1->Previous = $$;
                if ($1->Class) $$->NrWidgetChildren = 1;
            }
        ;

witchet : NAME '.' NAME '.' NAME options children
            {
                TreeTemplate * volatile Template1, *Template2, *Template3;

                WITH_UNWIND {
                    Template1 = MakeTemplate($5, NULL);
                    Template1->Children = Template3 = MakeTemplate("", NULL);
                    Template2 = MakeTemplate($1, $3);
                    Template3->Next = Template3->Previous = Template2;
                    Template2->Next = Template2->Previous = Template3;
                    Template3->NrWidgetChildren =
                        Template1->NrWidgetChildren = 1;
                    
                    Template2->Children = $7;
                    if (Template2->Children) {
                        Template2->NrWidgetChildren = $7->NrWidgetChildren;
                        $7 = 0;
                    }
                    Template2->Options  = $6;
                    $6 = 0;
                    $$ = Template1;
                    Template1 = NULL;
                } ON_UNWIND {
                    myfree($1);
                    myfree($3);
                    myfree($5);
                    if ($6) FreeOptionTemplates($6);
                    if ($7) FreeTemplate($7);
                    if (Template1) FreeTemplate(Template1);
                } END_UNWIND;
            }
        | NAME '.' NAME options children
            {
                TreeTemplate *Template;

                WITH_UNWIND {
                    $$ = Template = MakeTemplate($1, $3);
                    Template->Options  = $4;
                    Template->Children = $5;
                    if (Template->Children) {
                        Template->NrWidgetChildren = $5->NrWidgetChildren;
                        $5 = NULL;
                    }
                    $4 = NULL;
                } ON_UNWIND {
                    myfree($1);
                    myfree($3);
                    if ($4) FreeOptionTemplates($4);
                    if ($5) FreeTemplate($5);
                } END_UNWIND;
            }
        | ',' NAME options children
            {
                TreeTemplate *Template;

                WITH_UNWIND {
                    $$ = Template = MakeTemplate("", $2);
                    Template->Options  = $3;
                    Template->Children = $4;
                    if (Template->Children) {
                        Template->NrWidgetChildren = $4->NrWidgetChildren;
                        $4 = NULL;
                    }
                    $3 = NULL;
                } ON_UNWIND {
                    myfree($2);
                    if ($3) FreeOptionTemplates($3);
                    if ($4) FreeTemplate($4);
                } END_UNWIND;
            }
        | ',' NAME '.' NAME options children
            {
                TreeTemplate * volatile Template1, *Template2, *Template3;

                WITH_UNWIND {
                    Template1 = MakeTemplate($4, NULL);
                    Template1->Children = Template3 = MakeTemplate("", NULL);
                    Template2 = MakeTemplate("", $2);
                    Template3->Next = Template3->Previous = Template2;
                    Template2->Next = Template2->Previous = Template3;
                    Template3->NrWidgetChildren =
                        Template1->NrWidgetChildren = 1;
                    
                    Template2->Children = $6;
                    if (Template2->Children) {
                        Template2->NrWidgetChildren = $6->NrWidgetChildren;
                        $6 = 0;
                    }
                    Template2->Options = $5;
                    $5 = 0;
                    $$ = Template1;
                    Template1 = NULL;
                } ON_UNWIND {
                    myfree($2);
                    myfree($4);
                    if ($5) FreeOptionTemplates($5);
                    if ($6) FreeTemplate($6);
                    if (Template1) FreeTemplate(Template1);
                } END_UNWIND;
            }
        ;

options : options option
            {
                if      (strcmp($2->WidgetName, MyNname) == 0)
                    $2->Flags = MYNAME;
                else if (strcmp($2->WidgetName, MyNclass) == 0)
                    $2->Flags = MYCLASS;
                else if (strcmp($2->WidgetName, MyNrealized) == 0)
                    $2->Flags = MYREALIZED;
                else
                    $2->Flags = 0;
                $2->Next = $1;
                $$ = $2;
            }
        |
            {
                $$ = NULL;
            }
        ;

option  : '[' NAME '.' NAME ']'
            {
                size_t          Length1, Length2;
                OptionTemplate *Option;

                WITH_UNWIND {
                    Length1 = strlen($2)+1;
                    Length2 = strlen($4)+1;
                    Option = (OptionTemplate *) mymalloc(sizeof(OptionTemplate)
                                                         +Length1+Length2-1);
                    memcpy(Option->WidgetName, $2, Length1);
                    Option->WitchetName = Option->WidgetName+Length1;
                    memcpy(Option->WitchetName, $4, Length2);
                    $$ = Option;
                } ON_UNWIND {
                    myfree($2);
                    myfree($4);
                } END_UNWIND;
            }
        | '[' NAME ']'
            {
                size_t          Length;
                OptionTemplate *Option;

                WITH_UNWIND {
                    Length = strlen($2);
                    Option = (OptionTemplate *) mymalloc(sizeof(OptionTemplate)
                                                         +Length);
                    Length++;
                    memcpy(Option->WidgetName, $2, Length);
                    Option->WitchetName = Option->WidgetName;
                    $$ = Option;
                } ON_UNWIND {
                    myfree($2);
                } END_UNWIND;
            }
        ;

children: '(' witchetlist ')'
            {
                $$ = $2;
            }
        |
            {
                $$ = NULL;
/* To match xmalloc in bison template: */
# ifdef free
#  undef free
# endif /* free */
# define free(n)      myfree(n)   
# ifdef malloc
#  undef malloc
# endif /* malloc */
# define malloc(n)    mymalloc(n)
# ifdef calloc
#  undef calloc
# endif /* calloc */
# define calloc(m, n) mycalloc(m, n)
            }
        ;
%%
/* Kludge in case bison template defined const to nothing */
#ifndef __cplusplus
# ifndef __STDC__
#  undef const
# endif
#endif

static void MyOverflow(const char *Text)
{
    Raise1(FatalException, Text);
}

static void yyerror(const char *s)
{
    Raise4(InvalidTemplate, ExceptionCopy(s),
           "but found '",     ExceptionCopy(_TreeText()), "'");
}

TreeTemplate *MakeTemplate(const char *Name, const char *Class)
{
    TreeTemplate *Template;
    size_t        Length;
    WidgetClass   WClass;

    Length = strlen(Name);
    if (Class) {
        WClass = LookupClass(NameBase, Class);
        if (!WClass) Raise1(InvalidClass, ExceptionCopy(Class));
    } else WClass = 0;
    Template = (TreeTemplate *) mymalloc(sizeof(TreeTemplate)+Length);
    Template->Previous = Template->Next = Template;
    Template->Flags       = 0;
    Template->Options     = NULL;
    Template->HashOptions = NULL;
    Template->Children    = NULL;
    Template->NrWidgetChildren = 0;
    memcpy(Template->Name, Name, Length+1);
    Template->Class = WClass;
    if (WClass && MyIsSubclass(WClass, shellWidgetClass) != False) {
        Template->Flags |= ISSHELL;
        if (MyIsSubclass(WClass, applicationShellWidgetClass) != False)
            Template->Flags |= ISAPPSHELL;
    }
    return Template;
}
