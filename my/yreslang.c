
/*  A Bison parser, made from ./yreslang.y with Bison version GNU Bison version 1.22
  */

#define YYBISON 1  /* Identify Bison output.  */

#define	NAME	258

#line 1 "./yreslang.y"

#include <string.h>

#include "mymalloc.h"
#include "except.h"
#include "reslang.h"

#define YYDEBUG		1
#define YYERROR_VERBOSE
#define ResYYoverflow(x1, x2, x3, x4, x5, x8) MyOverflow(x1)
#define	xmalloc	mymalloc
/* Kludge to get rid of ResYY_bcopy warnings --Ton */
#ifndef __GNUC__
# define __GNUC__ 2
#endif /* __GNUC__ */

static void        ResYYerror(const char *s);
static void        MyOverflow(const char *Text);
extern int         ResYYlex(void);
extern const char *_ResText(void);
extern ResParse   *ResResult;

#line 24 "./yreslang.y"
typedef union {
    char     *Name;
    ResParse *Parse;
} YYSTYPE;

#ifndef YYLTYPE
typedef
  struct ResYYltype
    {
      int timestamp;
      int first_line;
      int first_column;
      int last_line;
      int last_column;
      char *text;
   }
  ResYYltype;

#define YYLTYPE ResYYltype
#endif

#include <stdio.h>

#ifndef __cplusplus
#ifndef __STDC__
#define const
#endif
#endif



#define	YYFINAL		14
#define	YYFLAG		-32768
#define	YYNTBASE	7

#define YYTRANSLATE(x) ((unsigned)(x) <= 258 ? ResYYtranslate[x] : 12)

static const char ResYYtranslate[] = {     0,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     5,
     6,     2,     2,     4,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     1,     2,     3
};

#if YYDEBUG != 0
static const short ResYYprhs[] = {     0,
     0,     2,     5,     6,    10,    12,    15,    19,    21
};

static const short ResYYrhs[] = {     8,
     0,     8,    11,     0,     0,     9,     4,    11,     0,    11,
     0,     5,     6,     0,     5,     9,     6,     0,     3,     0,
     3,    10,     0
};

#endif

#if YYDEBUG != 0
static const short ResYYrline[] = { 0,
    33,    39,    44,    50,    55,    58,    62,    68,    79
};

static const char * const ResYYtname[] = {   "$","error","$illegal.","NAME","','",
"'('","')'","init","argset","commaargs","args","arg",""
};
#endif

static const short ResYYr1[] = {     0,
     7,     8,     8,     9,     9,    10,    10,    11,    11
};

static const short ResYYr2[] = {     0,
     1,     2,     0,     3,     1,     2,     3,     1,     2
};

static const short ResYYdefact[] = {     3,
     1,     8,     2,     0,     9,     6,     0,     5,     0,     7,
     4,     0,     0,     0
};

static const short ResYYdefgoto[] = {    12,
     1,     7,     5,     3
};

static const short ResYYpact[] = {-32768,
    -1,     4,-32768,    -2,-32768,-32768,     2,-32768,    -1,-32768,
-32768,     3,     7,-32768
};

static const short ResYYpgoto[] = {-32768,
-32768,-32768,-32768,    -4
};


#define	YYLAST		9


static const short ResYYtable[] = {     8,
     2,     2,    13,     6,    11,     9,    14,    10,     4
};

static const short ResYYcheck[] = {     4,
     3,     3,     0,     6,     9,     4,     0,     6,     5
};
/* -*-C-*-  Note some compilers choke on comments on `#line' lines.  */
#line 3 "/usr/lib/bison.simple"

/* Skeleton output parser for bison,
   Copyright (C) 1984, 1989, 1990 Bob Corbett and Richard Stallman

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 1, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */


#ifndef alloca
#ifdef __GNUC__
#define alloca __builtin_alloca
#else /* not GNU C.  */
#if (!defined (__STDC__) && defined (sparc)) || defined (__sparc__) || defined (__sparc) || defined (__sgi)
#include <alloca.h>
#else /* not sparc */
#if defined (MSDOS) && !defined (__TURBOC__)
#include <malloc.h>
#else /* not MSDOS, or __TURBOC__ */
#if defined(_AIX)
#include <malloc.h>
 #pragma alloca
#else /* not MSDOS, __TURBOC__, or _AIX */
#ifdef __hpux
#ifdef __cplusplus
extern "C" {
void *alloca (unsigned int);
};
#else /* not __cplusplus */
void *alloca ();
#endif /* not __cplusplus */
#endif /* __hpux */
#endif /* not _AIX */
#endif /* not MSDOS, or __TURBOC__ */
#endif /* not sparc.  */
#endif /* not GNU C.  */
#endif /* alloca not defined.  */

/* This is the parser code that is written into each bison parser
  when the %semantic_parser declaration is not specified in the grammar.
  It was written by Richard Stallman by simplifying the hairy parser
  used when %semantic_parser is specified.  */

/* Note: there must be only one dollar sign in this file.
   It is replaced by the list of actions, each action
   as one case of the switch.  */

#define ResYYerrok		(ResYYerrstatus = 0)
#define ResYYclearin	(ResYYchar = YYEMPTY)
#define YYEMPTY		-2
#define YYEOF		0
#define YYACCEPT	return(0)
#define YYABORT 	return(1)
#define YYERROR		goto ResYYerrlab1
/* Like YYERROR except do call ResYYerror.
   This remains here temporarily to ease the
   transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */
#define YYFAIL		goto ResYYerrlab
#define YYRECOVERING()  (!!ResYYerrstatus)
#define YYBACKUP(token, value) \
do								\
  if (ResYYchar == YYEMPTY && ResYYlen == 1)				\
    { ResYYchar = (token), ResYYlval = (value);			\
      ResYYchar1 = YYTRANSLATE (ResYYchar);				\
      YYPOPSTACK;						\
      goto ResYYbackup;						\
    }								\
  else								\
    { ResYYerror ("syntax error: cannot back up"); YYERROR; }	\
while (0)

#define YYTERROR	1
#define YYERRCODE	256

#ifndef YYPURE
#define YYLEX		ResYYlex()
#endif

#ifdef YYPURE
#ifdef YYLSP_NEEDED
#define YYLEX		ResYYlex(&ResYYlval, &ResYYlloc)
#else
#define YYLEX		ResYYlex(&ResYYlval)
#endif
#endif

/* If nonreentrant, generate the variables here */

#ifndef YYPURE

int	ResYYchar;			/*  the lookahead symbol		*/
YYSTYPE	ResYYlval;			/*  the semantic value of the		*/
				/*  lookahead symbol			*/

#ifdef YYLSP_NEEDED
YYLTYPE ResYYlloc;			/*  location data for the lookahead	*/
				/*  symbol				*/
#endif

int ResYYnerrs;			/*  number of parse errors so far       */
#endif  /* not YYPURE */

#if YYDEBUG != 0
int ResYYdebug;			/*  nonzero means print parse trace	*/
/* Since this is uninitialized, it does not stop multiple parsers
   from coexisting.  */
#endif

/*  YYINITDEPTH indicates the initial size of the parser's stacks	*/

#ifndef	YYINITDEPTH
#define YYINITDEPTH 200
#endif

/*  YYMAXDEPTH is the maximum size the stacks can grow to
    (effective only if the built-in stack extension method is used).  */

#if YYMAXDEPTH == 0
#undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
#define YYMAXDEPTH 10000
#endif

/* Prevent warning if -Wstrict-prototypes.  */
#ifdef __GNUC__
int ResYYparse (void);
#endif

#if __GNUC__ > 1		/* GNU C and GNU C++ define this.  */
#define __ResYY_bcopy(FROM,TO,COUNT)	__builtin_memcpy(TO,FROM,COUNT)
#else				/* not GNU C or C++ */
#ifndef __cplusplus

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */
static void
__ResYY_bcopy (from, to, count)
     char *from;
     char *to;
     int count;
{
  register char *f = from;
  register char *t = to;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}

#else /* __cplusplus */

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */
static void
__ResYY_bcopy (char *from, char *to, int count)
{
  register char *f = from;
  register char *t = to;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}

#endif
#endif

#line 184 "/usr/lib/bison.simple"
int
ResYYparse()
{
  register int ResYYstate;
  register int ResYYn;
  register short *ResYYssp;
  register YYSTYPE *ResYYvsp;
  int ResYYerrstatus;	/*  number of tokens to shift before error messages enabled */
  int ResYYchar1 = 0;		/*  lookahead token as an internal (translated) token number */

  short	ResYYssa[YYINITDEPTH];	/*  the state stack			*/
  YYSTYPE ResYYvsa[YYINITDEPTH];	/*  the semantic value stack		*/

  short *ResYYss = ResYYssa;		/*  refer to the stacks thru separate pointers */
  YYSTYPE *ResYYvs = ResYYvsa;	/*  to allow ResYYoverflow to reallocate them elsewhere */

#ifdef YYLSP_NEEDED
  YYLTYPE ResYYlsa[YYINITDEPTH];	/*  the location stack			*/
  YYLTYPE *ResYYls = ResYYlsa;
  YYLTYPE *ResYYlsp;

#define YYPOPSTACK   (ResYYvsp--, ResYYssp--, ResYYlsp--)
#else
#define YYPOPSTACK   (ResYYvsp--, ResYYssp--)
#endif

  int ResYYstacksize = YYINITDEPTH;

#ifdef YYPURE
  int ResYYchar;
  YYSTYPE ResYYlval;
  int ResYYnerrs;
#ifdef YYLSP_NEEDED
  YYLTYPE ResYYlloc;
#endif
#endif

  YYSTYPE ResYYval;		/*  the variable used to return		*/
				/*  semantic values from the action	*/
				/*  routines				*/

  int ResYYlen;

#if YYDEBUG != 0
  if (ResYYdebug)
    fprintf(stderr, "Starting parse\n");
#endif

  ResYYstate = 0;
  ResYYerrstatus = 0;
  ResYYnerrs = 0;
  ResYYchar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  ResYYssp = ResYYss - 1;
  ResYYvsp = ResYYvs;
#ifdef YYLSP_NEEDED
  ResYYlsp = ResYYls;
#endif

/* Push a new state, which is found in  ResYYstate  .  */
/* In all cases, when you get here, the value and location stacks
   have just been pushed. so pushing a state here evens the stacks.  */
ResYYnewstate:

  *++ResYYssp = ResYYstate;

  if (ResYYssp >= ResYYss + ResYYstacksize - 1)
    {
      /* Give user a chance to reallocate the stack */
      /* Use copies of these so that the &'s don't force the real ones into memory. */
      YYSTYPE *ResYYvs1 = ResYYvs;
      short *ResYYss1 = ResYYss;
#ifdef YYLSP_NEEDED
      YYLTYPE *ResYYls1 = ResYYls;
#endif

      /* Get the current used size of the three stacks, in elements.  */
      int size = ResYYssp - ResYYss + 1;

#ifdef ResYYoverflow
      /* Each stack pointer address is followed by the size of
	 the data in use in that stack, in bytes.  */
#ifdef YYLSP_NEEDED
      /* This used to be a conditional around just the two extra args,
	 but that might be undefined if ResYYoverflow is a macro.  */
      ResYYoverflow("parser stack overflow",
		 &ResYYss1, size * sizeof (*ResYYssp),
		 &ResYYvs1, size * sizeof (*ResYYvsp),
		 &ResYYls1, size * sizeof (*ResYYlsp),
		 &ResYYstacksize);
#else
      ResYYoverflow("parser stack overflow",
		 &ResYYss1, size * sizeof (*ResYYssp),
		 &ResYYvs1, size * sizeof (*ResYYvsp),
		 &ResYYstacksize);
#endif

      ResYYss = ResYYss1; ResYYvs = ResYYvs1;
#ifdef YYLSP_NEEDED
      ResYYls = ResYYls1;
#endif
#else /* no ResYYoverflow */
      /* Extend the stack our own way.  */
      if (ResYYstacksize >= YYMAXDEPTH)
	{
	  ResYYerror("parser stack overflow");
	  return 2;
	}
      ResYYstacksize *= 2;
      if (ResYYstacksize > YYMAXDEPTH)
	ResYYstacksize = YYMAXDEPTH;
      ResYYss = (short *) alloca (ResYYstacksize * sizeof (*ResYYssp));
      __ResYY_bcopy ((char *)ResYYss1, (char *)ResYYss, size * sizeof (*ResYYssp));
      ResYYvs = (YYSTYPE *) alloca (ResYYstacksize * sizeof (*ResYYvsp));
      __ResYY_bcopy ((char *)ResYYvs1, (char *)ResYYvs, size * sizeof (*ResYYvsp));
#ifdef YYLSP_NEEDED
      ResYYls = (YYLTYPE *) alloca (ResYYstacksize * sizeof (*ResYYlsp));
      __ResYY_bcopy ((char *)ResYYls1, (char *)ResYYls, size * sizeof (*ResYYlsp));
#endif
#endif /* no ResYYoverflow */

      ResYYssp = ResYYss + size - 1;
      ResYYvsp = ResYYvs + size - 1;
#ifdef YYLSP_NEEDED
      ResYYlsp = ResYYls + size - 1;
#endif

#if YYDEBUG != 0
      if (ResYYdebug)
	fprintf(stderr, "Stack size increased to %d\n", ResYYstacksize);
#endif

      if (ResYYssp >= ResYYss + ResYYstacksize - 1)
	YYABORT;
    }

#if YYDEBUG != 0
  if (ResYYdebug)
    fprintf(stderr, "Entering state %d\n", ResYYstate);
#endif

  goto ResYYbackup;
 ResYYbackup:

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
/* ResYYresume: */

  /* First try to decide what to do without reference to lookahead token.  */

  ResYYn = ResYYpact[ResYYstate];
  if (ResYYn == YYFLAG)
    goto ResYYdefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* ResYYchar is either YYEMPTY or YYEOF
     or a valid token in external form.  */

  if (ResYYchar == YYEMPTY)
    {
#if YYDEBUG != 0
      if (ResYYdebug)
	fprintf(stderr, "Reading a token: ");
#endif
      ResYYchar = YYLEX;
    }

  /* Convert token to internal form (in ResYYchar1) for indexing tables with */

  if (ResYYchar <= 0)		/* This means end of input. */
    {
      ResYYchar1 = 0;
      ResYYchar = YYEOF;		/* Don't call YYLEX any more */

#if YYDEBUG != 0
      if (ResYYdebug)
	fprintf(stderr, "Now at end of input.\n");
#endif
    }
  else
    {
      ResYYchar1 = YYTRANSLATE(ResYYchar);

#if YYDEBUG != 0
      if (ResYYdebug)
	{
	  fprintf (stderr, "Next token is %d (%s", ResYYchar, ResYYtname[ResYYchar1]);
	  /* Give the individual parser a way to print the precise meaning
	     of a token, for further debugging info.  */
#ifdef YYPRINT
	  YYPRINT (stderr, ResYYchar, ResYYlval);
#endif
	  fprintf (stderr, ")\n");
	}
#endif
    }

  ResYYn += ResYYchar1;
  if (ResYYn < 0 || ResYYn > YYLAST || ResYYcheck[ResYYn] != ResYYchar1)
    goto ResYYdefault;

  ResYYn = ResYYtable[ResYYn];

  /* ResYYn is what to do for this token type in this state.
     Negative => reduce, -ResYYn is rule number.
     Positive => shift, ResYYn is new state.
       New state is final state => don't bother to shift,
       just return success.
     0, or most negative number => error.  */

  if (ResYYn < 0)
    {
      if (ResYYn == YYFLAG)
	goto ResYYerrlab;
      ResYYn = -ResYYn;
      goto ResYYreduce;
    }
  else if (ResYYn == 0)
    goto ResYYerrlab;

  if (ResYYn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */

#if YYDEBUG != 0
  if (ResYYdebug)
    fprintf(stderr, "Shifting token %d (%s), ", ResYYchar, ResYYtname[ResYYchar1]);
#endif

  /* Discard the token being shifted unless it is eof.  */
  if (ResYYchar != YYEOF)
    ResYYchar = YYEMPTY;

  *++ResYYvsp = ResYYlval;
#ifdef YYLSP_NEEDED
  *++ResYYlsp = ResYYlloc;
#endif

  /* count tokens shifted since error; after three, turn off error status.  */
  if (ResYYerrstatus) ResYYerrstatus--;

  ResYYstate = ResYYn;
  goto ResYYnewstate;

/* Do the default action for the current state.  */
ResYYdefault:

  ResYYn = ResYYdefact[ResYYstate];
  if (ResYYn == 0)
    goto ResYYerrlab;

/* Do a reduction.  ResYYn is the number of a rule to reduce with.  */
ResYYreduce:
  ResYYlen = ResYYr2[ResYYn];
  if (ResYYlen > 0)
    ResYYval = ResYYvsp[1-ResYYlen]; /* implement default value of the action */

#if YYDEBUG != 0
  if (ResYYdebug)
    {
      int i;

      fprintf (stderr, "Reducing via rule %d (line %d), ",
	       ResYYn, ResYYrline[ResYYn]);

      /* Print the symbols being reduced, and their result.  */
      for (i = ResYYprhs[ResYYn]; ResYYrhs[i] > 0; i++)
	fprintf (stderr, "%s ", ResYYtname[ResYYrhs[i]]);
      fprintf (stderr, " -> %s\n", ResYYtname[ResYYr1[ResYYn]]);
    }
#endif


  switch (ResYYn) {

case 1:
#line 34 "./yreslang.y"
{
                ResResult = ResYYvsp[0].Parse;
            ;
    break;}
case 2:
#line 40 "./yreslang.y"
{
                ResYYvsp[0].Parse->Next = ResYYvsp[-1].Parse;
                ResYYval.Parse = ResYYvsp[0].Parse;
            ;
    break;}
case 3:
#line 45 "./yreslang.y"
{
                ResYYval.Parse = NULL;
            ;
    break;}
case 4:
#line 51 "./yreslang.y"
{
                ResYYvsp[0].Parse->Next = ResYYvsp[-2].Parse->Next;
                ResYYval.Parse = ResYYvsp[-2].Parse->Next = ResYYvsp[0].Parse;
            ;
    break;}
case 6:
#line 59 "./yreslang.y"
{
                ResYYval.Parse = NULL;
            ;
    break;}
case 7:
#line 63 "./yreslang.y"
{
                ResYYval.Parse = ResYYvsp[-1].Parse;
            ;
    break;}
case 8:
#line 69 "./yreslang.y"
{
                ResParse *Parse;

                Parse = mynew(ResParse);
                Parse->NrArgs = -1;
                Parse->Name   = ResYYvsp[0].Name;
                Parse->Arg[0] = NULL;
                Parse->Next = Parse;
                ResYYval.Parse = Parse;
            ;
    break;}
case 9:
#line 80 "./yreslang.y"
{
                ResParse *Parse, **Ptr, *Next;
                int      n;

                if (ResYYvsp[0].Parse) {
                    for (Parse = ResYYvsp[0].Parse->Next,n=1;
                         Parse != ResYYvsp[0].Parse;
                         Parse = Parse->Next) n++;
                    Parse = mymalloc(sizeof(ResParse)+n*sizeof(ResParse *));
                    Parse->NrArgs = n;
                    ResYYvsp[0].Parse = ResYYvsp[0].Parse->Next;
                    for (Ptr = &Parse->Arg[0]; n>0; n--, Ptr++, ResYYvsp[0].Parse = Next) {
                        Next = ResYYvsp[0].Parse->Next;
                        ResYYvsp[0].Parse->Next = NULL;
                        *Ptr = ResYYvsp[0].Parse;
                    }
                    *Ptr = NULL;
                } else {
                    Parse = mynew(ResParse);
                    Parse->NrArgs = 0;
                    Parse->Arg[0] = NULL;
                }
                Parse->Name   = ResYYvsp[-1].Name;
                ResYYval.Parse = Parse;
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
            ;
    break;}
}
   /* the action file gets copied in in place of this dollarsign */
#line 465 "/usr/lib/bison.simple"

  ResYYvsp -= ResYYlen;
  ResYYssp -= ResYYlen;
#ifdef YYLSP_NEEDED
  ResYYlsp -= ResYYlen;
#endif

#if YYDEBUG != 0
  if (ResYYdebug)
    {
      short *ssp1 = ResYYss - 1;
      fprintf (stderr, "state stack now");
      while (ssp1 != ResYYssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

  *++ResYYvsp = ResYYval;

#ifdef YYLSP_NEEDED
  ResYYlsp++;
  if (ResYYlen == 0)
    {
      ResYYlsp->first_line = ResYYlloc.first_line;
      ResYYlsp->first_column = ResYYlloc.first_column;
      ResYYlsp->last_line = (ResYYlsp-1)->last_line;
      ResYYlsp->last_column = (ResYYlsp-1)->last_column;
      ResYYlsp->text = 0;
    }
  else
    {
      ResYYlsp->last_line = (ResYYlsp+ResYYlen-1)->last_line;
      ResYYlsp->last_column = (ResYYlsp+ResYYlen-1)->last_column;
    }
#endif

  /* Now "shift" the result of the reduction.
     Determine what state that goes to,
     based on the state we popped back to
     and the rule number reduced by.  */

  ResYYn = ResYYr1[ResYYn];

  ResYYstate = ResYYpgoto[ResYYn - YYNTBASE] + *ResYYssp;
  if (ResYYstate >= 0 && ResYYstate <= YYLAST && ResYYcheck[ResYYstate] == *ResYYssp)
    ResYYstate = ResYYtable[ResYYstate];
  else
    ResYYstate = ResYYdefgoto[ResYYn - YYNTBASE];

  goto ResYYnewstate;

ResYYerrlab:   /* here on detecting error */

  if (! ResYYerrstatus)
    /* If not already recovering from an error, report this error.  */
    {
      ++ResYYnerrs;

#ifdef YYERROR_VERBOSE
      ResYYn = ResYYpact[ResYYstate];

      if (ResYYn > YYFLAG && ResYYn < YYLAST)
	{
	  int size = 0;
	  char *msg;
	  int x, count;

	  count = 0;
	  /* Start X at -ResYYn if nec to avoid negative indexes in ResYYcheck.  */
	  for (x = (ResYYn < 0 ? -ResYYn : 0);
	       x < (sizeof(ResYYtname) / sizeof(char *)); x++)
	    if (ResYYcheck[x + ResYYn] == x)
	      size += strlen(ResYYtname[x]) + 15, count++;
	  msg = (char *) malloc(size + 15);
	  if (msg != 0)
	    {
	      strcpy(msg, "parse error");

	      if (count < 5)
		{
		  count = 0;
		  for (x = (ResYYn < 0 ? -ResYYn : 0);
		       x < (sizeof(ResYYtname) / sizeof(char *)); x++)
		    if (ResYYcheck[x + ResYYn] == x)
		      {
			strcat(msg, count == 0 ? ", expecting `" : " or `");
			strcat(msg, ResYYtname[x]);
			strcat(msg, "'");
			count++;
		      }
		}
	      ResYYerror(msg);
	      free(msg);
	    }
	  else
	    ResYYerror ("parse error; also virtual memory exceeded");
	}
      else
#endif /* YYERROR_VERBOSE */
	ResYYerror("parse error");
    }

  goto ResYYerrlab1;
ResYYerrlab1:   /* here on error raised explicitly by an action */

  if (ResYYerrstatus == 3)
    {
      /* if just tried and failed to reuse lookahead token after an error, discard it.  */

      /* return failure if at end of input */
      if (ResYYchar == YYEOF)
	YYABORT;

#if YYDEBUG != 0
      if (ResYYdebug)
	fprintf(stderr, "Discarding token %d (%s).\n", ResYYchar, ResYYtname[ResYYchar1]);
#endif

      ResYYchar = YYEMPTY;
    }

  /* Else will try to reuse lookahead token
     after shifting the error token.  */

  ResYYerrstatus = 3;		/* Each real token shifted decrements this */

  goto ResYYerrhandle;

ResYYerrdefault:  /* current state does not do anything special for the error token. */

#if 0
  /* This is wrong; only states that explicitly want error tokens
     should shift them.  */
  ResYYn = ResYYdefact[ResYYstate];  /* If its default is to accept any token, ok.  Otherwise pop it.*/
  if (ResYYn) goto ResYYdefault;
#endif

ResYYerrpop:   /* pop the current state because it cannot handle the error token */

  if (ResYYssp == ResYYss) YYABORT;
  ResYYvsp--;
  ResYYstate = *--ResYYssp;
#ifdef YYLSP_NEEDED
  ResYYlsp--;
#endif

#if YYDEBUG != 0
  if (ResYYdebug)
    {
      short *ssp1 = ResYYss - 1;
      fprintf (stderr, "Error: state stack now");
      while (ssp1 != ResYYssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

ResYYerrhandle:

  ResYYn = ResYYpact[ResYYstate];
  if (ResYYn == YYFLAG)
    goto ResYYerrdefault;

  ResYYn += YYTERROR;
  if (ResYYn < 0 || ResYYn > YYLAST || ResYYcheck[ResYYn] != YYTERROR)
    goto ResYYerrdefault;

  ResYYn = ResYYtable[ResYYn];
  if (ResYYn < 0)
    {
      if (ResYYn == YYFLAG)
	goto ResYYerrpop;
      ResYYn = -ResYYn;
      goto ResYYreduce;
    }
  else if (ResYYn == 0)
    goto ResYYerrpop;

  if (ResYYn == YYFINAL)
    YYACCEPT;

#if YYDEBUG != 0
  if (ResYYdebug)
    fprintf(stderr, "Shifting error token, ");
#endif

  *++ResYYvsp = ResYYlval;
#ifdef YYLSP_NEEDED
  *++ResYYlsp = ResYYlloc;
#endif

  ResYYstate = ResYYn;
  goto ResYYnewstate;
}
#line 119 "./yreslang.y"

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

static void ResYYerror(const char *s)
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

