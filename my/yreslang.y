%{
#include <string.h>

#include "mymalloc.h"
#include "except.h"
#include "reslang.h"

#define YYDEBUG		1
#define YYERROR_VERBOSE
#define YYOVERFLOW MyOverflow(x1)
#define	xmalloc	mymalloc
/* Kludge to get rid of yy_bcopy warnings --Ton */
#ifndef __GNUC__
# define __GNUC__ 2
#endif /* __GNUC__ */

static void        yyerror(const char *s);
static void        MyOverflow(const char *Text);
extern int         yylex(void);
extern const char *_ResText(void);
extern ResParse   *ResResult;
%}

%union {
    char     *Name;
    ResParse *Parse;
}

%token <Name>   NAME

%type  <Parse>  argset arg args commaargs
%%
init    : argset
            {
                ResResult = $1;
            }
        ;

argset  : argset arg
            {
                $2->Next = $1;
                $$ = $2;
            }
        |   
            {
                $$ = NULL;
            }
        ;

commaargs: commaargs ',' arg
            {
                $3->Next = $1->Next;
                $$ = $1->Next = $3;
            }
        | arg
        ;

args    : '(' ')'
            {
                $$ = NULL;
            }
        | '(' commaargs ')'
            {
                $$ = $2;
            }
        ;

arg     : NAME
            {
                ResParse *Parse;

                Parse = mynew(ResParse);
                Parse->NrArgs = -1;
                Parse->Name   = $1;
                Parse->Arg[0] = NULL;
                Parse->Next = Parse;
                $$ = Parse;
            }
        | NAME args
            {
                ResParse *Parse, **Ptr, *Next;
                int      n;

                if ($2) {
                    for (Parse = $2->Next,n=1;
                         Parse != $2;
                         Parse = Parse->Next) n++;
                    Parse = mymalloc(sizeof(ResParse)+n*sizeof(ResParse *));
                    Parse->NrArgs = n;
                    $2 = $2->Next;
                    for (Ptr = &Parse->Arg[0]; n>0; n--, Ptr++, $2 = Next) {
                        Next = $2->Next;
                        $2->Next = NULL;
                        *Ptr = $2;
                    }
                    *Ptr = NULL;
                } else {
                    Parse = mynew(ResParse);
                    Parse->NrArgs = 0;
                    Parse->Arg[0] = NULL;
                }
                Parse->Name   = $1;
                $$ = Parse;
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
    fprintf(stderr, "%s, but found `'%s''\n", s, _ResText());
    ResResult = NULL;
}

void FreeParse(ResParse *Parse)
{
    ResParse **Ptr, *Pos, *Next;

    while (Parse) {
        Next = Parse->Next;
        for (Ptr = &Parse->Arg[0]; (Pos = *Ptr) != NULL; Ptr++) FreeParse(Pos);
        myfree(Parse->Name);
        myfree(Parse);
        Parse = Next;
    }
}

void fprintParse(FILE *fp, const ResParse *Parse)
{
    int n;
    const ResParse *Next;
    ResParse * const *Ptr;

    while(Parse) {
        Next = Parse->Next;
        fputs(Parse->Name, fp);
        n = Parse->NrArgs;
        if (n >= 0) {
            putc('(', fp);
            if (n>0) {
                fprintParse(fp, Parse->Arg[0]);
                for (Ptr = &Parse->Arg[1]; (Parse = *Ptr) != NULL; Ptr++) {
                    putc(',', fp);
                    putc(' ', fp);
                    fprintParse(fp, Parse);
                }
            }
            putc(')', fp);
        }
        Parse = Next;
    }
}

