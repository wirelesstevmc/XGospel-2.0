/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or ResYY_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with ResYY or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output, and Bison version.  */
#define YYBISON 30802

/* Bison version string.  */
#define YYBISON_VERSION "3.8.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* First part of user prologue.  */
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

#line 94 "y.tab.c"

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

/* Use api.header.include to #include this header
   instead of duplicating it here.  */
#ifndef YY_YY_Y_TAB_H_INCLUDED
# define YY_YY_Y_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int ResYYdebug;
#endif

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum ResYYtokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    NAME = 258                     /* NAME  */
  };
  typedef enum ResYYtokentype ResYYtoken_kind_t;
#endif
/* Token kinds.  */
#define YYEMPTY -2
#define YYEOF 0
#define YYerror 256
#define YYUNDEF 257
#define NAME 258

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 24 "./yreslang.y"

    char     *Name;
    ResParse *Parse;

#line 158 "y.tab.c"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE ResYYlval;


int ResYYparse (void);


#endif /* !YY_YY_Y_TAB_H_INCLUDED  */
/* Symbol kind.  */
enum ResYYsymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_NAME = 3,                       /* NAME  */
  YYSYMBOL_4_ = 4,                         /* ','  */
  YYSYMBOL_5_ = 5,                         /* '('  */
  YYSYMBOL_6_ = 6,                         /* ')'  */
  YYSYMBOL_YYACCEPT = 7,                   /* $accept  */
  YYSYMBOL_init = 8,                       /* init  */
  YYSYMBOL_argset = 9,                     /* argset  */
  YYSYMBOL_commaargs = 10,                 /* commaargs  */
  YYSYMBOL_args = 11,                      /* args  */
  YYSYMBOL_arg = 12                        /* arg  */
};
typedef enum ResYYsymbol_kind_t ResYYsymbol_kind_t;




#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ ResYYtype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t ResYYtype_int8;
#else
typedef signed char ResYYtype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ ResYYtype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t ResYYtype_int16;
#else
typedef short ResYYtype_int16;
#endif

/* Work around bug in HP-UX 11.23, which defines these macros
   incorrectly for preprocessor constants.  This workaround can likely
   be removed in 2023, as HPE has promised support for HP-UX 11.23
   (aka HP-UX 11i v2) only through the end of 2022; see Table 2 of
   <https://h20195.www2.hpe.com/V2/getpdf.aspx/4AA4-7673ENW.pdf>.  */
#ifdef __hpux
# undef UINT_LEAST8_MAX
# undef UINT_LEAST16_MAX
# define UINT_LEAST8_MAX 255
# define UINT_LEAST16_MAX 65535
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ ResYYtype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t ResYYtype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char ResYYtype_uint8;
#else
typedef short ResYYtype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ ResYYtype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t ResYYtype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short ResYYtype_uint16;
#else
typedef int ResYYtype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))


/* Stored state numbers (used for stacks). */
typedef ResYYtype_int8 ResYY_state_t;

/* State numbers in computations.  */
typedef int ResYY_state_fast_t;

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif


#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YY_USE(E) ((void) (E))
#else
# define YY_USE(E) /* empty */
#endif

/* Suppress an incorrect diagnostic about ResYYlval being uninitialized.  */
#if defined __GNUC__ && ! defined __ICC && 406 <= __GNUC__ * 100 + __GNUC_MINOR__
# if __GNUC__ * 100 + __GNUC_MINOR__ < 407
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")
# else
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# endif
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if !defined ResYYoverflow

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* !defined ResYYoverflow */

#if (! defined ResYYoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union ResYYalloc
{
  ResYY_state_t ResYYss_alloc;
  YYSTYPE ResYYvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union ResYYalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (ResYY_state_t) + YYSIZEOF (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYPTRDIFF_T ResYYnewbytes;                                         \
        YYCOPY (&ResYYptr->Stack_alloc, Stack, ResYYsize);                    \
        Stack = &ResYYptr->Stack_alloc;                                    \
        ResYYnewbytes = ResYYstacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        ResYYptr += ResYYnewbytes / YYSIZEOF (*ResYYptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T ResYYi;                      \
          for (ResYYi = 0; ResYYi < (Count); ResYYi++)   \
            (Dst)[ResYYi] = (Src)[ResYYi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  3
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   9

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  7
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  6
/* YYNRULES -- Number of rules.  */
#define YYNRULES  10
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  14

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   258


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by ResYYlex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (ResYYsymbol_kind_t, ResYYtranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by ResYYlex.  */
static const ResYYtype_int8 ResYYtranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       5,     6,     2,     2,     4,     2,     2,     2,     2,     2,
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
       2,     2,     2,     2,     2,     2,     1,     2,     3
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const ResYYtype_int8 ResYYrline[] =
{
       0,    33,    33,    39,    45,    50,    55,    58,    62,    68,
      79
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (ResYYsymbol_kind_t, ResYYstos[State])

#if YYDEBUG || 0
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *ResYYsymbol_name (ResYYsymbol_kind_t ResYYsymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const ResYYtname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "NAME", "','", "'('",
  "')'", "$accept", "init", "argset", "commaargs", "args", "arg", YY_NULLPTR
};

static const char *
ResYYsymbol_name (ResYYsymbol_kind_t ResYYsymbol)
{
  return ResYYtname[ResYYsymbol];
}
#endif

#define YYPACT_NINF (-7)

#define ResYYpact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-1)

#define ResYYtable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const ResYYtype_int8 ResYYpact[] =
{
      -7,     3,    -1,    -7,     4,    -7,    -2,    -7,    -7,     2,
      -7,    -1,    -7,    -7
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const ResYYtype_int8 ResYYdefact[] =
{
       4,     0,     2,     1,     9,     3,     0,    10,     7,     0,
       6,     0,     8,     5
};

/* YYPGOTO[NTERM-NUM].  */
static const ResYYtype_int8 ResYYpgoto[] =
{
      -7,    -7,    -7,    -7,    -7,    -6
};

/* YYDEFGOTO[NTERM-NUM].  */
static const ResYYtype_int8 ResYYdefgoto[] =
{
       0,     1,     2,     9,     7,     5
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const ResYYtype_int8 ResYYtable[] =
{
      10,     4,     4,     3,     8,    13,    11,     0,    12,     6
};

static const ResYYtype_int8 ResYYcheck[] =
{
       6,     3,     3,     0,     6,    11,     4,    -1,     6,     5
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const ResYYtype_int8 ResYYstos[] =
{
       0,     8,     9,     0,     3,    12,     5,    11,     6,    10,
      12,     4,     6,    12
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const ResYYtype_int8 ResYYr1[] =
{
       0,     7,     8,     9,     9,    10,    10,    11,    11,    12,
      12
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const ResYYtype_int8 ResYYr2[] =
{
       0,     2,     1,     2,     0,     3,     1,     2,     3,     1,
       2
};


enum { YYENOMEM = -2 };

#define ResYYerrok         (ResYYerrstatus = 0)
#define ResYYclearin       (ResYYchar = YYEMPTY)

#define YYACCEPT        goto ResYYacceptlab
#define YYABORT         goto ResYYabortlab
#define YYERROR         goto ResYYerrorlab
#define YYNOMEM         goto ResYYexhaustedlab


#define YYRECOVERING()  (!!ResYYerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (ResYYchar == YYEMPTY)                                        \
      {                                                           \
        ResYYchar = (Token);                                         \
        ResYYlval = (Value);                                         \
        YYPOPSTACK (ResYYlen);                                       \
        ResYYstate = *ResYYssp;                                         \
        goto ResYYbackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        ResYYerror (YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Backward compatibility with an undocumented macro.
   Use YYerror or YYUNDEF. */
#define YYERRCODE YYUNDEF


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (ResYYdebug)                                  \
    YYFPRINTF Args;                             \
} while (0)




# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (ResYYdebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      ResYY_symbol_print (stderr,                                            \
                  Kind, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
ResYY_symbol_value_print (FILE *ResYYo,
                       ResYYsymbol_kind_t ResYYkind, YYSTYPE const * const ResYYvaluep)
{
  FILE *ResYYoutput = ResYYo;
  YY_USE (ResYYoutput);
  if (!ResYYvaluep)
    return;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (ResYYkind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
ResYY_symbol_print (FILE *ResYYo,
                 ResYYsymbol_kind_t ResYYkind, YYSTYPE const * const ResYYvaluep)
{
  YYFPRINTF (ResYYo, "%s %s (",
             ResYYkind < YYNTOKENS ? "token" : "nterm", ResYYsymbol_name (ResYYkind));

  ResYY_symbol_value_print (ResYYo, ResYYkind, ResYYvaluep);
  YYFPRINTF (ResYYo, ")");
}

/*------------------------------------------------------------------.
| ResYY_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
ResYY_stack_print (ResYY_state_t *ResYYbottom, ResYY_state_t *ResYYtop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; ResYYbottom <= ResYYtop; ResYYbottom++)
    {
      int ResYYbot = *ResYYbottom;
      YYFPRINTF (stderr, " %d", ResYYbot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (ResYYdebug)                                                  \
    ResYY_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
ResYY_reduce_print (ResYY_state_t *ResYYssp, YYSTYPE *ResYYvsp,
                 int ResYYrule)
{
  int ResYYlno = ResYYrline[ResYYrule];
  int ResYYnrhs = ResYYr2[ResYYrule];
  int ResYYi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             ResYYrule - 1, ResYYlno);
  /* The symbols being reduced.  */
  for (ResYYi = 0; ResYYi < ResYYnrhs; ResYYi++)
    {
      YYFPRINTF (stderr, "   $%d = ", ResYYi + 1);
      ResYY_symbol_print (stderr,
                       YY_ACCESSING_SYMBOL (+ResYYssp[ResYYi + 1 - ResYYnrhs]),
                       &ResYYvsp[(ResYYi + 1) - (ResYYnrhs)]);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (ResYYdebug)                          \
    ResYY_reduce_print (ResYYssp, ResYYvsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int ResYYdebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif






/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
ResYYdestruct (const char *ResYYmsg,
            ResYYsymbol_kind_t ResYYkind, YYSTYPE *ResYYvaluep)
{
  YY_USE (ResYYvaluep);
  if (!ResYYmsg)
    ResYYmsg = "Deleting";
  YY_SYMBOL_PRINT (ResYYmsg, ResYYkind, ResYYvaluep, ResYYlocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (ResYYkind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/* Lookahead token kind.  */
int ResYYchar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE ResYYlval;
/* Number of syntax errors so far.  */
int ResYYnerrs;




/*----------.
| ResYYparse.  |
`----------*/

int
ResYYparse (void)
{
    ResYY_state_fast_t ResYYstate = 0;
    /* Number of tokens to shift before error messages enabled.  */
    int ResYYerrstatus = 0;

    /* Refer to the stacks through separate pointers, to allow ResYYoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T ResYYstacksize = YYINITDEPTH;

    /* The state stack: array, bottom, top.  */
    ResYY_state_t ResYYssa[YYINITDEPTH];
    ResYY_state_t *ResYYss = ResYYssa;
    ResYY_state_t *ResYYssp = ResYYss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE ResYYvsa[YYINITDEPTH];
    YYSTYPE *ResYYvs = ResYYvsa;
    YYSTYPE *ResYYvsp = ResYYvs;

  int ResYYn;
  /* The return value of ResYYparse.  */
  int ResYYresult;
  /* Lookahead symbol kind.  */
  ResYYsymbol_kind_t ResYYtoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE ResYYval;



#define YYPOPSTACK(N)   (ResYYvsp -= (N), ResYYssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int ResYYlen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  ResYYchar = YYEMPTY; /* Cause a token to be read.  */

  goto ResYYsetstate;


/*------------------------------------------------------------.
| ResYYnewstate -- push a new state, which is found in ResYYstate.  |
`------------------------------------------------------------*/
ResYYnewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  ResYYssp++;


/*--------------------------------------------------------------------.
| ResYYsetstate -- set current state (the top of the stack) to ResYYstate.  |
`--------------------------------------------------------------------*/
ResYYsetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", ResYYstate));
  YY_ASSERT (0 <= ResYYstate && ResYYstate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *ResYYssp = YY_CAST (ResYY_state_t, ResYYstate);
  YY_IGNORE_USELESS_CAST_END
  YY_STACK_PRINT (ResYYss, ResYYssp);

  if (ResYYss + ResYYstacksize - 1 <= ResYYssp)
#if !defined ResYYoverflow && !defined YYSTACK_RELOCATE
    YYNOMEM;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T ResYYsize = ResYYssp - ResYYss + 1;

# if defined ResYYoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        ResYY_state_t *ResYYss1 = ResYYss;
        YYSTYPE *ResYYvs1 = ResYYvs;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if ResYYoverflow is a macro.  */
        ResYYoverflow (YY_("memory exhausted"),
                    &ResYYss1, ResYYsize * YYSIZEOF (*ResYYssp),
                    &ResYYvs1, ResYYsize * YYSIZEOF (*ResYYvsp),
                    &ResYYstacksize);
        ResYYss = ResYYss1;
        ResYYvs = ResYYvs1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= ResYYstacksize)
        YYNOMEM;
      ResYYstacksize *= 2;
      if (YYMAXDEPTH < ResYYstacksize)
        ResYYstacksize = YYMAXDEPTH;

      {
        ResYY_state_t *ResYYss1 = ResYYss;
        union ResYYalloc *ResYYptr =
          YY_CAST (union ResYYalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (ResYYstacksize))));
        if (! ResYYptr)
          YYNOMEM;
        YYSTACK_RELOCATE (ResYYss_alloc, ResYYss);
        YYSTACK_RELOCATE (ResYYvs_alloc, ResYYvs);
#  undef YYSTACK_RELOCATE
        if (ResYYss1 != ResYYssa)
          YYSTACK_FREE (ResYYss1);
      }
# endif

      ResYYssp = ResYYss + ResYYsize - 1;
      ResYYvsp = ResYYvs + ResYYsize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, ResYYstacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (ResYYss + ResYYstacksize - 1 <= ResYYssp)
        YYABORT;
    }
#endif /* !defined ResYYoverflow && !defined YYSTACK_RELOCATE */


  if (ResYYstate == YYFINAL)
    YYACCEPT;

  goto ResYYbackup;


/*-----------.
| ResYYbackup.  |
`-----------*/
ResYYbackup:
  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  ResYYn = ResYYpact[ResYYstate];
  if (ResYYpact_value_is_default (ResYYn))
    goto ResYYdefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (ResYYchar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      ResYYchar = ResYYlex ();
    }

  if (ResYYchar <= YYEOF)
    {
      ResYYchar = YYEOF;
      ResYYtoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (ResYYchar == YYerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      ResYYchar = YYUNDEF;
      ResYYtoken = YYSYMBOL_YYerror;
      goto ResYYerrlab1;
    }
  else
    {
      ResYYtoken = YYTRANSLATE (ResYYchar);
      YY_SYMBOL_PRINT ("Next token is", ResYYtoken, &ResYYlval, &ResYYlloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  ResYYn += ResYYtoken;
  if (ResYYn < 0 || YYLAST < ResYYn || ResYYcheck[ResYYn] != ResYYtoken)
    goto ResYYdefault;
  ResYYn = ResYYtable[ResYYn];
  if (ResYYn <= 0)
    {
      if (ResYYtable_value_is_error (ResYYn))
        goto ResYYerrlab;
      ResYYn = -ResYYn;
      goto ResYYreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (ResYYerrstatus)
    ResYYerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", ResYYtoken, &ResYYlval, &ResYYlloc);
  ResYYstate = ResYYn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++ResYYvsp = ResYYlval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  /* Discard the shifted token.  */
  ResYYchar = YYEMPTY;
  goto ResYYnewstate;


/*-----------------------------------------------------------.
| ResYYdefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
ResYYdefault:
  ResYYn = ResYYdefact[ResYYstate];
  if (ResYYn == 0)
    goto ResYYerrlab;
  goto ResYYreduce;


/*-----------------------------.
| ResYYreduce -- do a reduction.  |
`-----------------------------*/
ResYYreduce:
  /* ResYYn is the number of a rule to reduce with.  */
  ResYYlen = ResYYr2[ResYYn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  ResYYval = ResYYvsp[1-ResYYlen];


  YY_REDUCE_PRINT (ResYYn);
  switch (ResYYn)
    {
  case 2: /* init: argset  */
#line 34 "./yreslang.y"
            {
                ResResult = (ResYYvsp[0].Parse);
            }
#line 1143 "y.tab.c"
    break;

  case 3: /* argset: argset arg  */
#line 40 "./yreslang.y"
            {
                (ResYYvsp[0].Parse)->Next = (ResYYvsp[-1].Parse);
                (ResYYval.Parse) = (ResYYvsp[0].Parse);
            }
#line 1152 "y.tab.c"
    break;

  case 4: /* argset: %empty  */
#line 45 "./yreslang.y"
            {
                (ResYYval.Parse) = NULL;
            }
#line 1160 "y.tab.c"
    break;

  case 5: /* commaargs: commaargs ',' arg  */
#line 51 "./yreslang.y"
            {
                (ResYYvsp[0].Parse)->Next = (ResYYvsp[-2].Parse)->Next;
                (ResYYval.Parse) = (ResYYvsp[-2].Parse)->Next = (ResYYvsp[0].Parse);
            }
#line 1169 "y.tab.c"
    break;

  case 7: /* args: '(' ')'  */
#line 59 "./yreslang.y"
            {
                (ResYYval.Parse) = NULL;
            }
#line 1177 "y.tab.c"
    break;

  case 8: /* args: '(' commaargs ')'  */
#line 63 "./yreslang.y"
            {
                (ResYYval.Parse) = (ResYYvsp[-1].Parse);
            }
#line 1185 "y.tab.c"
    break;

  case 9: /* arg: NAME  */
#line 69 "./yreslang.y"
            {
                ResParse *Parse;

                Parse = mynew(ResParse);
                Parse->NrArgs = -1;
                Parse->Name   = (ResYYvsp[0].Name);
                Parse->Arg[0] = NULL;
                Parse->Next = Parse;
                (ResYYval.Parse) = Parse;
            }
#line 1200 "y.tab.c"
    break;

  case 10: /* arg: NAME args  */
#line 80 "./yreslang.y"
            {
                ResParse *Parse, **Ptr, *Next;
                int      n;

                if ((ResYYvsp[0].Parse)) {
                    for (Parse = (ResYYvsp[0].Parse)->Next,n=1;
                         Parse != (ResYYvsp[0].Parse);
                         Parse = Parse->Next) n++;
                    Parse = mymalloc(sizeof(ResParse)+n*sizeof(ResParse *));
                    Parse->NrArgs = n;
                    (ResYYvsp[0].Parse) = (ResYYvsp[0].Parse)->Next;
                    for (Ptr = &Parse->Arg[0]; n>0; n--, Ptr++, (ResYYvsp[0].Parse) = Next) {
                        Next = (ResYYvsp[0].Parse)->Next;
                        (ResYYvsp[0].Parse)->Next = NULL;
                        *Ptr = (ResYYvsp[0].Parse);
                    }
                    *Ptr = NULL;
                } else {
                    Parse = mynew(ResParse);
                    Parse->NrArgs = 0;
                    Parse->Arg[0] = NULL;
                }
                Parse->Name   = (ResYYvsp[-1].Name);
                (ResYYval.Parse) = Parse;
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
#line 1243 "y.tab.c"
    break;


#line 1247 "y.tab.c"

      default: break;
    }
  /* User semantic actions sometimes alter ResYYchar, and that requires
     that ResYYtoken be updated with the new translation.  We take the
     approach of translating immediately before every use of ResYYtoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering ResYYchar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (ResYYsymbol_kind_t, ResYYr1[ResYYn]), &ResYYval, &ResYYloc);

  YYPOPSTACK (ResYYlen);
  ResYYlen = 0;

  *++ResYYvsp = ResYYval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int ResYYlhs = ResYYr1[ResYYn] - YYNTOKENS;
    const int ResYYi = ResYYpgoto[ResYYlhs] + *ResYYssp;
    ResYYstate = (0 <= ResYYi && ResYYi <= YYLAST && ResYYcheck[ResYYi] == *ResYYssp
               ? ResYYtable[ResYYi]
               : ResYYdefgoto[ResYYlhs]);
  }

  goto ResYYnewstate;


/*--------------------------------------.
| ResYYerrlab -- here on detecting error.  |
`--------------------------------------*/
ResYYerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  ResYYtoken = ResYYchar == YYEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (ResYYchar);
  /* If not already recovering from an error, report this error.  */
  if (!ResYYerrstatus)
    {
      ++ResYYnerrs;
      ResYYerror (YY_("syntax error"));
    }

  if (ResYYerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (ResYYchar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (ResYYchar == YYEOF)
            YYABORT;
        }
      else
        {
          ResYYdestruct ("Error: discarding",
                      ResYYtoken, &ResYYlval);
          ResYYchar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto ResYYerrlab1;


/*---------------------------------------------------.
| ResYYerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
ResYYerrorlab:
  /* Pacify compilers when the user code never invokes YYERROR and the
     label ResYYerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;
  ++ResYYnerrs;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (ResYYlen);
  ResYYlen = 0;
  YY_STACK_PRINT (ResYYss, ResYYssp);
  ResYYstate = *ResYYssp;
  goto ResYYerrlab1;


/*-------------------------------------------------------------.
| ResYYerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
ResYYerrlab1:
  ResYYerrstatus = 3;      /* Each real token shifted decrements this.  */

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      ResYYn = ResYYpact[ResYYstate];
      if (!ResYYpact_value_is_default (ResYYn))
        {
          ResYYn += YYSYMBOL_YYerror;
          if (0 <= ResYYn && ResYYn <= YYLAST && ResYYcheck[ResYYn] == YYSYMBOL_YYerror)
            {
              ResYYn = ResYYtable[ResYYn];
              if (0 < ResYYn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (ResYYssp == ResYYss)
        YYABORT;


      ResYYdestruct ("Error: popping",
                  YY_ACCESSING_SYMBOL (ResYYstate), ResYYvsp);
      YYPOPSTACK (1);
      ResYYstate = *ResYYssp;
      YY_STACK_PRINT (ResYYss, ResYYssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++ResYYvsp = ResYYlval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (ResYYn), ResYYvsp, ResYYlsp);

  ResYYstate = ResYYn;
  goto ResYYnewstate;


/*-------------------------------------.
| ResYYacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
ResYYacceptlab:
  ResYYresult = 0;
  goto ResYYreturnlab;


/*-----------------------------------.
| ResYYabortlab -- YYABORT comes here.  |
`-----------------------------------*/
ResYYabortlab:
  ResYYresult = 1;
  goto ResYYreturnlab;


/*-----------------------------------------------------------.
| ResYYexhaustedlab -- YYNOMEM (memory exhaustion) comes here.  |
`-----------------------------------------------------------*/
ResYYexhaustedlab:
  ResYYerror (YY_("memory exhausted"));
  ResYYresult = 2;
  goto ResYYreturnlab;


/*----------------------------------------------------------.
| ResYYreturnlab -- parsing is finished, clean up and return.  |
`----------------------------------------------------------*/
ResYYreturnlab:
  if (ResYYchar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      ResYYtoken = YYTRANSLATE (ResYYchar);
      ResYYdestruct ("Cleanup: discarding lookahead",
                  ResYYtoken, &ResYYlval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (ResYYlen);
  YY_STACK_PRINT (ResYYss, ResYYssp);
  while (ResYYssp != ResYYss)
    {
      ResYYdestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*ResYYssp), ResYYvsp);
      YYPOPSTACK (1);
    }
#ifndef ResYYoverflow
  if (ResYYss != ResYYssa)
    YYSTACK_FREE (ResYYss);
#endif

  return ResYYresult;
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

