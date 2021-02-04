%x ERROR

%{
#include <stdlib.h>
#include <string.h>
#include "mymalloc.h"
#define YYTEXT   ((char *) yytext)
#define YYLENG   ((size_t) yyleng)
#define ECHO     (void) fwrite(YYTEXT, YYLENG, 1, yyout)

extern int   isatty(int fd);
extern int   yylex(void);

static const char *Ptr;
static int         Length;

#undef YY_INPUT
#define YY_INPUT(buf, result, max_size)                                 \
        if (max_size > Length) result = Length;                         \
        else                   result = max_size;                       \
        memcpy(buf, Ptr, (size_t) result);                              \
        Ptr += result;                                                  \
        Length -= result;
#define YY_BREAK

#include "reslang.h"
#include "yreslang.tab.h"

ResParse *ResResult;
extern int yyparse(void);
%}
WHITE             [ \t\f\v\r\n]
ID                [^ \[\](){},\n\t\f\v\r]+
DIGIT             [0-9]
NUM               ({DIGIT}+)
%%
{ID}                          { yylval.Name = mystrndup(YYTEXT, YYLENG);
                                return NAME; }
{WHITE}                       { break; }
.                             { return YYTEXT[0]; }

<ERROR>(.|\n){0,40}           { yylval.Name = mystrndup(YYTEXT, YYLENG);
                                yymore();
                                return NAME; }
%%
ResParse *resParse(const char *Str, size_t Len)
{
    Ptr = Str;
    Length = Len;
    yy_init = 1;
    yyparse();
    return ResResult;
}

extern const char *_ResText(void);
const char *_ResText(void)
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
