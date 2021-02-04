
/*  A Bison parser, made from ./ywidgettree.y with Bison version GNU Bison version 1.22
  */

#define YYBISON 1  /* Identify Bison output.  */

#define	NAME	258

#line 1 "./ywidgettree.y"

#include <stdlib.h>

#include "mymalloc.h"
#include "except.h"
#include "widgettree.h"

#include <X11/Shell.h>

#define YYDEBUG		1
#define YYERROR_VERBOSE
#define TreeYYoverflow(x1, x2, x3, x4, x5, x8) MyOverflow(x1)
#define	xmalloc	mymalloc
/* Kludge to get rid of TreeYY_bcopy warnings --Ton */
#ifndef __GNUC__
# define __GNUC__ 2
#endif /* __GNUC__ */

static void TreeYYerror(const char *s);
static void MyOverflow(const char *Text);

extern int           TreeYYlex(void);
extern char         *_TreeText(void);
extern TreeTemplate *TreeResult;

static Exception InvalidClass    = {"Class is unknown to the program:"};
static Exception InvalidTemplate = {"This does not look like a treetemplate:"};

#line 30 "./ywidgettree.y"
typedef union {
    char           *Name;
    TreeTemplate   *Tree;
    OptionTemplate *Options;
} YYSTYPE;

#ifndef YYLTYPE
typedef
  struct TreeYYltype
    {
      int timestamp;
      int first_line;
      int first_column;
      int last_line;
      int last_column;
      char *text;
   }
  TreeYYltype;

#define YYLTYPE TreeYYltype
#endif

#include <stdio.h>

#ifndef __cplusplus
#ifndef __STDC__
#define const
#endif
#endif



#define	YYFINAL		33
#define	YYFLAG		-32768
#define	YYNTBASE	10

#define YYTRANSLATE(x) ((unsigned)(x) <= 258 ? TreeYYtranslate[x] : 16)

static const char TreeYYtranslate[] = {     0,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     8,
     9,     2,     2,     5,     2,     4,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     6,     2,     7,     2,     2,     2,     2,     2,     2,     2,
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
static const short TreeYYprhs[] = {     0,
     0,     2,     5,     7,    15,    21,    26,    33,    36,    37,
    43,    47,    51
};

static const short TreeYYrhs[] = {    15,
     0,    11,    12,     0,    12,     0,     3,     4,     3,     4,
     3,    13,    15,     0,     3,     4,     3,    13,    15,     0,
     5,     3,    13,    15,     0,     5,     3,     4,     3,    13,
    15,     0,    13,    14,     0,     0,     6,     3,     4,     3,
     7,     0,     6,     3,     7,     0,     8,    11,     9,     0,
     0
};

#endif

#if YYDEBUG != 0
static const short TreeYYrline[] = { 0,
    42,    48,    56,    65,    96,   116,   135,   167,   180,   186,
   205,   224,   228
};

static const char * const TreeYYtname[] = {   "$","error","$illegal.","NAME","'.'",
"','","'['","']'","'('","')'","init","witchetlist","witchet","options","option",
"children",""
};
#endif

static const short TreeYYr1[] = {     0,
    10,    11,    11,    12,    12,    12,    12,    13,    13,    14,
    14,    15,    15
};

static const short TreeYYr2[] = {     0,
     1,     2,     1,     7,     5,     4,     6,     2,     0,     5,
     3,     3,     0
};

static const short TreeYYdefact[] = {    13,
     0,     1,     0,     0,     0,     3,     0,     9,    12,     2,
     9,     0,    13,     0,    13,     9,     0,     8,     6,     9,
     5,    13,     0,    13,     7,     0,    11,     4,     0,    10,
     0,     0,     0
};

static const short TreeYYdefgoto[] = {    31,
     5,     6,    13,    18,     2
};

static const short TreeYYpact[] = {     8,
    10,-32768,     4,    14,     9,-32768,    16,    17,-32768,-32768,
    18,    20,    -3,    21,    -3,-32768,    22,-32768,-32768,-32768,
-32768,    -3,     0,    -3,-32768,    23,-32768,-32768,    13,-32768,
    27,    28,-32768
};

static const short TreeYYpgoto[] = {-32768,
-32768,    24,   -10,-32768,   -13
};


#define	YYLAST		29


static const short TreeYYtable[] = {    19,
    15,    21,    17,    26,     1,    22,    27,     7,    25,    24,
    28,     3,     3,     4,     4,     1,     8,     9,    11,    30,
    12,    14,    16,    20,    23,    29,    32,    33,    10
};

static const short TreeYYcheck[] = {    13,
    11,    15,     6,     4,     8,    16,     7,     4,    22,    20,
    24,     3,     3,     5,     5,     8,     3,     9,     3,     7,
     4,     4,     3,     3,     3,     3,     0,     0,     5
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

#define TreeYYerrok		(TreeYYerrstatus = 0)
#define TreeYYclearin	(TreeYYchar = YYEMPTY)
#define YYEMPTY		-2
#define YYEOF		0
#define YYACCEPT	return(0)
#define YYABORT 	return(1)
#define YYERROR		goto TreeYYerrlab1
/* Like YYERROR except do call TreeYYerror.
   This remains here temporarily to ease the
   transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */
#define YYFAIL		goto TreeYYerrlab
#define YYRECOVERING()  (!!TreeYYerrstatus)
#define YYBACKUP(token, value) \
do								\
  if (TreeYYchar == YYEMPTY && TreeYYlen == 1)				\
    { TreeYYchar = (token), TreeYYlval = (value);			\
      TreeYYchar1 = YYTRANSLATE (TreeYYchar);				\
      YYPOPSTACK;						\
      goto TreeYYbackup;						\
    }								\
  else								\
    { TreeYYerror ("syntax error: cannot back up"); YYERROR; }	\
while (0)

#define YYTERROR	1
#define YYERRCODE	256

#ifndef YYPURE
#define YYLEX		TreeYYlex()
#endif

#ifdef YYPURE
#ifdef YYLSP_NEEDED
#define YYLEX		TreeYYlex(&TreeYYlval, &TreeYYlloc)
#else
#define YYLEX		TreeYYlex(&TreeYYlval)
#endif
#endif

/* If nonreentrant, generate the variables here */

#ifndef YYPURE

int	TreeYYchar;			/*  the lookahead symbol		*/
YYSTYPE	TreeYYlval;			/*  the semantic value of the		*/
				/*  lookahead symbol			*/

#ifdef YYLSP_NEEDED
YYLTYPE TreeYYlloc;			/*  location data for the lookahead	*/
				/*  symbol				*/
#endif

int TreeYYnerrs;			/*  number of parse errors so far       */
#endif  /* not YYPURE */

#if YYDEBUG != 0
int TreeYYdebug;			/*  nonzero means print parse trace	*/
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
int TreeYYparse (void);
#endif

#if __GNUC__ > 1		/* GNU C and GNU C++ define this.  */
#define __TreeYY_bcopy(FROM,TO,COUNT)	__builtin_memcpy(TO,FROM,COUNT)
#else				/* not GNU C or C++ */
#ifndef __cplusplus

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */
static void
__TreeYY_bcopy (from, to, count)
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
__TreeYY_bcopy (char *from, char *to, int count)
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
TreeYYparse()
{
  register int TreeYYstate;
  register int TreeYYn;
  register short *TreeYYssp;
  register YYSTYPE *TreeYYvsp;
  int TreeYYerrstatus;	/*  number of tokens to shift before error messages enabled */
  int TreeYYchar1 = 0;		/*  lookahead token as an internal (translated) token number */

  short	TreeYYssa[YYINITDEPTH];	/*  the state stack			*/
  YYSTYPE TreeYYvsa[YYINITDEPTH];	/*  the semantic value stack		*/

  short *TreeYYss = TreeYYssa;		/*  refer to the stacks thru separate pointers */
  YYSTYPE *TreeYYvs = TreeYYvsa;	/*  to allow TreeYYoverflow to reallocate them elsewhere */

#ifdef YYLSP_NEEDED
  YYLTYPE TreeYYlsa[YYINITDEPTH];	/*  the location stack			*/
  YYLTYPE *TreeYYls = TreeYYlsa;
  YYLTYPE *TreeYYlsp;

#define YYPOPSTACK   (TreeYYvsp--, TreeYYssp--, TreeYYlsp--)
#else
#define YYPOPSTACK   (TreeYYvsp--, TreeYYssp--)
#endif

  int TreeYYstacksize = YYINITDEPTH;

#ifdef YYPURE
  int TreeYYchar;
  YYSTYPE TreeYYlval;
  int TreeYYnerrs;
#ifdef YYLSP_NEEDED
  YYLTYPE TreeYYlloc;
#endif
#endif

  YYSTYPE TreeYYval;		/*  the variable used to return		*/
				/*  semantic values from the action	*/
				/*  routines				*/

  int TreeYYlen;

#if YYDEBUG != 0
  if (TreeYYdebug)
    fprintf(stderr, "Starting parse\n");
#endif

  TreeYYstate = 0;
  TreeYYerrstatus = 0;
  TreeYYnerrs = 0;
  TreeYYchar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  TreeYYssp = TreeYYss - 1;
  TreeYYvsp = TreeYYvs;
#ifdef YYLSP_NEEDED
  TreeYYlsp = TreeYYls;
#endif

/* Push a new state, which is found in  TreeYYstate  .  */
/* In all cases, when you get here, the value and location stacks
   have just been pushed. so pushing a state here evens the stacks.  */
TreeYYnewstate:

  *++TreeYYssp = TreeYYstate;

  if (TreeYYssp >= TreeYYss + TreeYYstacksize - 1)
    {
      /* Give user a chance to reallocate the stack */
      /* Use copies of these so that the &'s don't force the real ones into memory. */
      YYSTYPE *TreeYYvs1 = TreeYYvs;
      short *TreeYYss1 = TreeYYss;
#ifdef YYLSP_NEEDED
      YYLTYPE *TreeYYls1 = TreeYYls;
#endif

      /* Get the current used size of the three stacks, in elements.  */
      int size = TreeYYssp - TreeYYss + 1;

#ifdef TreeYYoverflow
      /* Each stack pointer address is followed by the size of
	 the data in use in that stack, in bytes.  */
#ifdef YYLSP_NEEDED
      /* This used to be a conditional around just the two extra args,
	 but that might be undefined if TreeYYoverflow is a macro.  */
      TreeYYoverflow("parser stack overflow",
		 &TreeYYss1, size * sizeof (*TreeYYssp),
		 &TreeYYvs1, size * sizeof (*TreeYYvsp),
		 &TreeYYls1, size * sizeof (*TreeYYlsp),
		 &TreeYYstacksize);
#else
      TreeYYoverflow("parser stack overflow",
		 &TreeYYss1, size * sizeof (*TreeYYssp),
		 &TreeYYvs1, size * sizeof (*TreeYYvsp),
		 &TreeYYstacksize);
#endif

      TreeYYss = TreeYYss1; TreeYYvs = TreeYYvs1;
#ifdef YYLSP_NEEDED
      TreeYYls = TreeYYls1;
#endif
#else /* no TreeYYoverflow */
      /* Extend the stack our own way.  */
      if (TreeYYstacksize >= YYMAXDEPTH)
	{
	  TreeYYerror("parser stack overflow");
	  return 2;
	}
      TreeYYstacksize *= 2;
      if (TreeYYstacksize > YYMAXDEPTH)
	TreeYYstacksize = YYMAXDEPTH;
      TreeYYss = (short *) alloca (TreeYYstacksize * sizeof (*TreeYYssp));
      __TreeYY_bcopy ((char *)TreeYYss1, (char *)TreeYYss, size * sizeof (*TreeYYssp));
      TreeYYvs = (YYSTYPE *) alloca (TreeYYstacksize * sizeof (*TreeYYvsp));
      __TreeYY_bcopy ((char *)TreeYYvs1, (char *)TreeYYvs, size * sizeof (*TreeYYvsp));
#ifdef YYLSP_NEEDED
      TreeYYls = (YYLTYPE *) alloca (TreeYYstacksize * sizeof (*TreeYYlsp));
      __TreeYY_bcopy ((char *)TreeYYls1, (char *)TreeYYls, size * sizeof (*TreeYYlsp));
#endif
#endif /* no TreeYYoverflow */

      TreeYYssp = TreeYYss + size - 1;
      TreeYYvsp = TreeYYvs + size - 1;
#ifdef YYLSP_NEEDED
      TreeYYlsp = TreeYYls + size - 1;
#endif

#if YYDEBUG != 0
      if (TreeYYdebug)
	fprintf(stderr, "Stack size increased to %d\n", TreeYYstacksize);
#endif

      if (TreeYYssp >= TreeYYss + TreeYYstacksize - 1)
	YYABORT;
    }

#if YYDEBUG != 0
  if (TreeYYdebug)
    fprintf(stderr, "Entering state %d\n", TreeYYstate);
#endif

  goto TreeYYbackup;
 TreeYYbackup:

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
/* TreeYYresume: */

  /* First try to decide what to do without reference to lookahead token.  */

  TreeYYn = TreeYYpact[TreeYYstate];
  if (TreeYYn == YYFLAG)
    goto TreeYYdefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* TreeYYchar is either YYEMPTY or YYEOF
     or a valid token in external form.  */

  if (TreeYYchar == YYEMPTY)
    {
#if YYDEBUG != 0
      if (TreeYYdebug)
	fprintf(stderr, "Reading a token: ");
#endif
      TreeYYchar = YYLEX;
    }

  /* Convert token to internal form (in TreeYYchar1) for indexing tables with */

  if (TreeYYchar <= 0)		/* This means end of input. */
    {
      TreeYYchar1 = 0;
      TreeYYchar = YYEOF;		/* Don't call YYLEX any more */

#if YYDEBUG != 0
      if (TreeYYdebug)
	fprintf(stderr, "Now at end of input.\n");
#endif
    }
  else
    {
      TreeYYchar1 = YYTRANSLATE(TreeYYchar);

#if YYDEBUG != 0
      if (TreeYYdebug)
	{
	  fprintf (stderr, "Next token is %d (%s", TreeYYchar, TreeYYtname[TreeYYchar1]);
	  /* Give the individual parser a way to print the precise meaning
	     of a token, for further debugging info.  */
#ifdef YYPRINT
	  YYPRINT (stderr, TreeYYchar, TreeYYlval);
#endif
	  fprintf (stderr, ")\n");
	}
#endif
    }

  TreeYYn += TreeYYchar1;
  if (TreeYYn < 0 || TreeYYn > YYLAST || TreeYYcheck[TreeYYn] != TreeYYchar1)
    goto TreeYYdefault;

  TreeYYn = TreeYYtable[TreeYYn];

  /* TreeYYn is what to do for this token type in this state.
     Negative => reduce, -TreeYYn is rule number.
     Positive => shift, TreeYYn is new state.
       New state is final state => don't bother to shift,
       just return success.
     0, or most negative number => error.  */

  if (TreeYYn < 0)
    {
      if (TreeYYn == YYFLAG)
	goto TreeYYerrlab;
      TreeYYn = -TreeYYn;
      goto TreeYYreduce;
    }
  else if (TreeYYn == 0)
    goto TreeYYerrlab;

  if (TreeYYn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */

#if YYDEBUG != 0
  if (TreeYYdebug)
    fprintf(stderr, "Shifting token %d (%s), ", TreeYYchar, TreeYYtname[TreeYYchar1]);
#endif

  /* Discard the token being shifted unless it is eof.  */
  if (TreeYYchar != YYEOF)
    TreeYYchar = YYEMPTY;

  *++TreeYYvsp = TreeYYlval;
#ifdef YYLSP_NEEDED
  *++TreeYYlsp = TreeYYlloc;
#endif

  /* count tokens shifted since error; after three, turn off error status.  */
  if (TreeYYerrstatus) TreeYYerrstatus--;

  TreeYYstate = TreeYYn;
  goto TreeYYnewstate;

/* Do the default action for the current state.  */
TreeYYdefault:

  TreeYYn = TreeYYdefact[TreeYYstate];
  if (TreeYYn == 0)
    goto TreeYYerrlab;

/* Do a reduction.  TreeYYn is the number of a rule to reduce with.  */
TreeYYreduce:
  TreeYYlen = TreeYYr2[TreeYYn];
  if (TreeYYlen > 0)
    TreeYYval = TreeYYvsp[1-TreeYYlen]; /* implement default value of the action */

#if YYDEBUG != 0
  if (TreeYYdebug)
    {
      int i;

      fprintf (stderr, "Reducing via rule %d (line %d), ",
	       TreeYYn, TreeYYrline[TreeYYn]);

      /* Print the symbols being reduced, and their result.  */
      for (i = TreeYYprhs[TreeYYn]; TreeYYrhs[i] > 0; i++)
	fprintf (stderr, "%s ", TreeYYtname[TreeYYrhs[i]]);
      fprintf (stderr, " -> %s\n", TreeYYtname[TreeYYr1[TreeYYn]]);
    }
#endif


  switch (TreeYYn) {

case 1:
#line 43 "./ywidgettree.y"
{
                TreeResult = TreeYYvsp[0].Tree;
            ;
    break;}
case 2:
#line 49 "./ywidgettree.y"
{
                TreeYYvsp[0].Tree->Next     = TreeYYvsp[-1].Tree;
                TreeYYvsp[0].Tree->Previous = TreeYYvsp[-1].Tree->Previous;
                TreeYYvsp[0].Tree->Previous->Next = TreeYYvsp[0].Tree->Next->Previous = TreeYYvsp[0].Tree;
                if (TreeYYvsp[0].Tree->Class) TreeYYvsp[-1].Tree->NrWidgetChildren++;
                TreeYYval.Tree = TreeYYvsp[-1].Tree;
            ;
    break;}
case 3:
#line 57 "./ywidgettree.y"
{
                TreeYYval.Tree = MakeTemplate("", NULL);
                TreeYYval.Tree->Next = TreeYYval.Tree->Previous = TreeYYvsp[0].Tree;
                TreeYYvsp[0].Tree->Next = TreeYYvsp[0].Tree->Previous = TreeYYval.Tree;
                if (TreeYYvsp[0].Tree->Class) TreeYYval.Tree->NrWidgetChildren = 1;
            ;
    break;}
case 4:
#line 66 "./ywidgettree.y"
{
                TreeTemplate * volatile Template1, *Template2, *Template3;

                WITH_UNWIND {
                    Template1 = MakeTemplate(TreeYYvsp[-2].Name, NULL);
                    Template1->Children = Template3 = MakeTemplate("", NULL);
                    Template2 = MakeTemplate(TreeYYvsp[-6].Name, TreeYYvsp[-4].Name);
                    Template3->Next = Template3->Previous = Template2;
                    Template2->Next = Template2->Previous = Template3;
                    Template3->NrWidgetChildren =
                        Template1->NrWidgetChildren = 1;
                    
                    Template2->Children = TreeYYvsp[0].Tree;
                    if (Template2->Children) {
                        Template2->NrWidgetChildren = TreeYYvsp[0].Tree->NrWidgetChildren;
                        TreeYYvsp[0].Tree = 0;
                    }
                    Template2->Options  = TreeYYvsp[-1].Options;
                    TreeYYvsp[-1].Options = 0;
                    TreeYYval.Tree = Template1;
                    Template1 = NULL;
                } ON_UNWIND {
                    myfree(TreeYYvsp[-6].Name);
                    myfree(TreeYYvsp[-4].Name);
                    myfree(TreeYYvsp[-2].Name);
                    if (TreeYYvsp[-1].Options) FreeOptionTemplates(TreeYYvsp[-1].Options);
                    if (TreeYYvsp[0].Tree) FreeTemplate(TreeYYvsp[0].Tree);
                    if (Template1) FreeTemplate(Template1);
                } END_UNWIND;
            ;
    break;}
case 5:
#line 97 "./ywidgettree.y"
{
                TreeTemplate *Template;

                WITH_UNWIND {
                    TreeYYval.Tree = Template = MakeTemplate(TreeYYvsp[-4].Name, TreeYYvsp[-2].Name);
                    Template->Options  = TreeYYvsp[-1].Options;
                    Template->Children = TreeYYvsp[0].Tree;
                    if (Template->Children) {
                        Template->NrWidgetChildren = TreeYYvsp[0].Tree->NrWidgetChildren;
                        TreeYYvsp[0].Tree = NULL;
                    }
                    TreeYYvsp[-1].Options = NULL;
                } ON_UNWIND {
                    myfree(TreeYYvsp[-4].Name);
                    myfree(TreeYYvsp[-2].Name);
                    if (TreeYYvsp[-1].Options) FreeOptionTemplates(TreeYYvsp[-1].Options);
                    if (TreeYYvsp[0].Tree) FreeTemplate(TreeYYvsp[0].Tree);
                } END_UNWIND;
            ;
    break;}
case 6:
#line 117 "./ywidgettree.y"
{
                TreeTemplate *Template;

                WITH_UNWIND {
                    TreeYYval.Tree = Template = MakeTemplate("", TreeYYvsp[-2].Name);
                    Template->Options  = TreeYYvsp[-1].Options;
                    Template->Children = TreeYYvsp[0].Tree;
                    if (Template->Children) {
                        Template->NrWidgetChildren = TreeYYvsp[0].Tree->NrWidgetChildren;
                        TreeYYvsp[0].Tree = NULL;
                    }
                    TreeYYvsp[-1].Options = NULL;
                } ON_UNWIND {
                    myfree(TreeYYvsp[-2].Name);
                    if (TreeYYvsp[-1].Options) FreeOptionTemplates(TreeYYvsp[-1].Options);
                    if (TreeYYvsp[0].Tree) FreeTemplate(TreeYYvsp[0].Tree);
                } END_UNWIND;
            ;
    break;}
case 7:
#line 136 "./ywidgettree.y"
{
                TreeTemplate * volatile Template1, *Template2, *Template3;

                WITH_UNWIND {
                    Template1 = MakeTemplate(TreeYYvsp[-2].Name, NULL);
                    Template1->Children = Template3 = MakeTemplate("", NULL);
                    Template2 = MakeTemplate("", TreeYYvsp[-4].Name);
                    Template3->Next = Template3->Previous = Template2;
                    Template2->Next = Template2->Previous = Template3;
                    Template3->NrWidgetChildren =
                        Template1->NrWidgetChildren = 1;
                    
                    Template2->Children = TreeYYvsp[0].Tree;
                    if (Template2->Children) {
                        Template2->NrWidgetChildren = TreeYYvsp[0].Tree->NrWidgetChildren;
                        TreeYYvsp[0].Tree = 0;
                    }
                    Template2->Options = TreeYYvsp[-1].Options;
                    TreeYYvsp[-1].Options = 0;
                    TreeYYval.Tree = Template1;
                    Template1 = NULL;
                } ON_UNWIND {
                    myfree(TreeYYvsp[-4].Name);
                    myfree(TreeYYvsp[-2].Name);
                    if (TreeYYvsp[-1].Options) FreeOptionTemplates(TreeYYvsp[-1].Options);
                    if (TreeYYvsp[0].Tree) FreeTemplate(TreeYYvsp[0].Tree);
                    if (Template1) FreeTemplate(Template1);
                } END_UNWIND;
            ;
    break;}
case 8:
#line 168 "./ywidgettree.y"
{
                if      (strcmp(TreeYYvsp[0].Options->WidgetName, MyNname) == 0)
                    TreeYYvsp[0].Options->Flags = MYNAME;
                else if (strcmp(TreeYYvsp[0].Options->WidgetName, MyNclass) == 0)
                    TreeYYvsp[0].Options->Flags = MYCLASS;
                else if (strcmp(TreeYYvsp[0].Options->WidgetName, MyNrealized) == 0)
                    TreeYYvsp[0].Options->Flags = MYREALIZED;
                else
                    TreeYYvsp[0].Options->Flags = 0;
                TreeYYvsp[0].Options->Next = TreeYYvsp[-1].Options;
                TreeYYval.Options = TreeYYvsp[0].Options;
            ;
    break;}
case 9:
#line 181 "./ywidgettree.y"
{
                TreeYYval.Options = NULL;
            ;
    break;}
case 10:
#line 187 "./ywidgettree.y"
{
                size_t          Length1, Length2;
                OptionTemplate *Option;

                WITH_UNWIND {
                    Length1 = strlen(TreeYYvsp[-3].Name)+1;
                    Length2 = strlen(TreeYYvsp[-1].Name)+1;
                    Option = (OptionTemplate *) mymalloc(sizeof(OptionTemplate)
                                                         +Length1+Length2-1);
                    memcpy(Option->WidgetName, TreeYYvsp[-3].Name, Length1);
                    Option->WitchetName = Option->WidgetName+Length1;
                    memcpy(Option->WitchetName, TreeYYvsp[-1].Name, Length2);
                    TreeYYval.Options = Option;
                } ON_UNWIND {
                    myfree(TreeYYvsp[-3].Name);
                    myfree(TreeYYvsp[-1].Name);
                } END_UNWIND;
            ;
    break;}
case 11:
#line 206 "./ywidgettree.y"
{
                size_t          Length;
                OptionTemplate *Option;

                WITH_UNWIND {
                    Length = strlen(TreeYYvsp[-1].Name);
                    Option = (OptionTemplate *) mymalloc(sizeof(OptionTemplate)
                                                         +Length);
                    Length++;
                    memcpy(Option->WidgetName, TreeYYvsp[-1].Name, Length);
                    Option->WitchetName = Option->WidgetName;
                    TreeYYval.Options = Option;
                } ON_UNWIND {
                    myfree(TreeYYvsp[-1].Name);
                } END_UNWIND;
            ;
    break;}
case 12:
#line 225 "./ywidgettree.y"
{
                TreeYYval.Tree = TreeYYvsp[-1].Tree;
            ;
    break;}
case 13:
#line 229 "./ywidgettree.y"
{
                TreeYYval.Tree = NULL;
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

  TreeYYvsp -= TreeYYlen;
  TreeYYssp -= TreeYYlen;
#ifdef YYLSP_NEEDED
  TreeYYlsp -= TreeYYlen;
#endif

#if YYDEBUG != 0
  if (TreeYYdebug)
    {
      short *ssp1 = TreeYYss - 1;
      fprintf (stderr, "state stack now");
      while (ssp1 != TreeYYssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

  *++TreeYYvsp = TreeYYval;

#ifdef YYLSP_NEEDED
  TreeYYlsp++;
  if (TreeYYlen == 0)
    {
      TreeYYlsp->first_line = TreeYYlloc.first_line;
      TreeYYlsp->first_column = TreeYYlloc.first_column;
      TreeYYlsp->last_line = (TreeYYlsp-1)->last_line;
      TreeYYlsp->last_column = (TreeYYlsp-1)->last_column;
      TreeYYlsp->text = 0;
    }
  else
    {
      TreeYYlsp->last_line = (TreeYYlsp+TreeYYlen-1)->last_line;
      TreeYYlsp->last_column = (TreeYYlsp+TreeYYlen-1)->last_column;
    }
#endif

  /* Now "shift" the result of the reduction.
     Determine what state that goes to,
     based on the state we popped back to
     and the rule number reduced by.  */

  TreeYYn = TreeYYr1[TreeYYn];

  TreeYYstate = TreeYYpgoto[TreeYYn - YYNTBASE] + *TreeYYssp;
  if (TreeYYstate >= 0 && TreeYYstate <= YYLAST && TreeYYcheck[TreeYYstate] == *TreeYYssp)
    TreeYYstate = TreeYYtable[TreeYYstate];
  else
    TreeYYstate = TreeYYdefgoto[TreeYYn - YYNTBASE];

  goto TreeYYnewstate;

TreeYYerrlab:   /* here on detecting error */

  if (! TreeYYerrstatus)
    /* If not already recovering from an error, report this error.  */
    {
      ++TreeYYnerrs;

#ifdef YYERROR_VERBOSE
      TreeYYn = TreeYYpact[TreeYYstate];

      if (TreeYYn > YYFLAG && TreeYYn < YYLAST)
	{
	  int size = 0;
	  char *msg;
	  int x, count;

	  count = 0;
	  /* Start X at -TreeYYn if nec to avoid negative indexes in TreeYYcheck.  */
	  for (x = (TreeYYn < 0 ? -TreeYYn : 0);
	       x < (sizeof(TreeYYtname) / sizeof(char *)); x++)
	    if (TreeYYcheck[x + TreeYYn] == x)
	      size += strlen(TreeYYtname[x]) + 15, count++;
	  msg = (char *) malloc(size + 15);
	  if (msg != 0)
	    {
	      strcpy(msg, "parse error");

	      if (count < 5)
		{
		  count = 0;
		  for (x = (TreeYYn < 0 ? -TreeYYn : 0);
		       x < (sizeof(TreeYYtname) / sizeof(char *)); x++)
		    if (TreeYYcheck[x + TreeYYn] == x)
		      {
			strcat(msg, count == 0 ? ", expecting `" : " or `");
			strcat(msg, TreeYYtname[x]);
			strcat(msg, "'");
			count++;
		      }
		}
	      TreeYYerror(msg);
	      free(msg);
	    }
	  else
	    TreeYYerror ("parse error; also virtual memory exceeded");
	}
      else
#endif /* YYERROR_VERBOSE */
	TreeYYerror("parse error");
    }

  goto TreeYYerrlab1;
TreeYYerrlab1:   /* here on error raised explicitly by an action */

  if (TreeYYerrstatus == 3)
    {
      /* if just tried and failed to reuse lookahead token after an error, discard it.  */

      /* return failure if at end of input */
      if (TreeYYchar == YYEOF)
	YYABORT;

#if YYDEBUG != 0
      if (TreeYYdebug)
	fprintf(stderr, "Discarding token %d (%s).\n", TreeYYchar, TreeYYtname[TreeYYchar1]);
#endif

      TreeYYchar = YYEMPTY;
    }

  /* Else will try to reuse lookahead token
     after shifting the error token.  */

  TreeYYerrstatus = 3;		/* Each real token shifted decrements this */

  goto TreeYYerrhandle;

TreeYYerrdefault:  /* current state does not do anything special for the error token. */

#if 0
  /* This is wrong; only states that explicitly want error tokens
     should shift them.  */
  TreeYYn = TreeYYdefact[TreeYYstate];  /* If its default is to accept any token, ok.  Otherwise pop it.*/
  if (TreeYYn) goto TreeYYdefault;
#endif

TreeYYerrpop:   /* pop the current state because it cannot handle the error token */

  if (TreeYYssp == TreeYYss) YYABORT;
  TreeYYvsp--;
  TreeYYstate = *--TreeYYssp;
#ifdef YYLSP_NEEDED
  TreeYYlsp--;
#endif

#if YYDEBUG != 0
  if (TreeYYdebug)
    {
      short *ssp1 = TreeYYss - 1;
      fprintf (stderr, "Error: state stack now");
      while (ssp1 != TreeYYssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

TreeYYerrhandle:

  TreeYYn = TreeYYpact[TreeYYstate];
  if (TreeYYn == YYFLAG)
    goto TreeYYerrdefault;

  TreeYYn += YYTERROR;
  if (TreeYYn < 0 || TreeYYn > YYLAST || TreeYYcheck[TreeYYn] != YYTERROR)
    goto TreeYYerrdefault;

  TreeYYn = TreeYYtable[TreeYYn];
  if (TreeYYn < 0)
    {
      if (TreeYYn == YYFLAG)
	goto TreeYYerrpop;
      TreeYYn = -TreeYYn;
      goto TreeYYreduce;
    }
  else if (TreeYYn == 0)
    goto TreeYYerrpop;

  if (TreeYYn == YYFINAL)
    YYACCEPT;

#if YYDEBUG != 0
  if (TreeYYdebug)
    fprintf(stderr, "Shifting error token, ");
#endif

  *++TreeYYvsp = TreeYYlval;
#ifdef YYLSP_NEEDED
  *++TreeYYlsp = TreeYYlloc;
#endif

  TreeYYstate = TreeYYn;
  goto TreeYYnewstate;
}
#line 246 "./ywidgettree.y"

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

static void TreeYYerror(const char *s)
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
