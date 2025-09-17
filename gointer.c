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
   especially those whose name start with YY_ or IgsYY_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with IgsYY or YY, to avoid
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
#line 1 "gointer.y"

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#include <mymalloc.h>
#include <except.h>
#include <myxlib.h>

#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>

#include "broadcast.h"
#include "connect.h"
#include "events.h"
#include "games.h"
#include "gointer.h"
#include "match.h"
#include "messages.h"
#include "players.h"
#include "reviews.h"
#include "stats.h"
#include "tell.h"
#include "utils.h"
#include "xgospel.h"
#include "version.h"

#define YYDEBUG		1
#define YYERROR_VERBOSE
#define IgsYYoverflow(x1, x2, x3, x4, x5, x8) MyOverflow(x1)
#define	xmalloc	mymalloc
/* Kludge to get rid of IgsYY_bcopy warnings --Ton */
#ifndef __GNUC__
# define __GNUC__ 2
#endif /* __GNUC__ */

static void IgsYYerror(const char *s);
static void MyOverflow(const char *Text);

extern int         WhoTracking;
extern int         nrplayers, maxplayers, nrgames;
extern char       *MyPassword;
extern int         SetServerTime;
extern struct tm   LocalTime, ServerTime;

static int Passed, eEmpty, PreEmpty, SeenAdd, gamesSeen, RegisteredUserSent;

/* Function to reset login state for reconnection */
void ResetLoginState(void) {
    RegisteredUserSent = 0;
}

/*
static int WhoseMove(NameVal *moves);
*/
static void ReceivedKibitz(Player *player, int Id,
                           const char *black, const char *white,
                           const char *kibitz, size_t Length);
static void OverObserve(int MaxGames);
static void PlayerPasses(const char *Name);
static void EnterString(XtPointer Closure);
static void Entering(void);

/* From goserver.l */
extern       int   IgsYYlex(void);
extern const char *_GoText(void);
extern       void  _IgsDefaultParse(void);
extern const char *_FormatError(void);
extern const char *Parsing(Connection conn);


#line 139 "y.tab.c"

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
extern int IgsYYdebug;
#endif

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum IgsYYtokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    NAME = 258,                    /* NAME  */
    SERVERMESSAGE = 259,           /* SERVERMESSAGE  */
    STATSENTRY = 260,              /* STATSENTRY  */
    ILLEGALMOVE = 261,             /* ILLEGALMOVE  */
    ILLEGALUNDO = 262,             /* ILLEGALUNDO  */
    REQUESTINGMATCH = 263,         /* REQUESTINGMATCH  */
    REMOVEGAMEFILE = 264,          /* REMOVEGAMEFILE  */
    MAILED = 265,                  /* MAILED  */
    REMOVEGROUP = 266,             /* REMOVEGROUP  */
    GIVEBYOYOMI = 267,             /* GIVEBYOYOMI  */
    RESULTLINE = 268,              /* RESULTLINE  */
    INVALID = 269,                 /* INVALID  */
    AUTOMATCHDISPUTE = 270,        /* AUTOMATCHDISPUTE  */
    NEWCHANNEL = 271,              /* NEWCHANNEL  */
    MUSTPASS = 272,                /* MUSTPASS  */
    OPPMUSTPASS = 273,             /* OPPMUSTPASS  */
    GUEST = 274,                   /* GUEST  */
    TELLDONE = 275,                /* TELLDONE  */
    REVIEWSTART = 276,             /* REVIEWSTART  */
    REVLITERAL = 277,              /* REVLITERAL  */
    REVUNKNOWN = 278,              /* REVUNKNOWN  */
    WELCOME = 279,                 /* WELCOME  */
    SERVERFULL = 280,              /* SERVERFULL  */
    XSHOUT2 = 281,                 /* XSHOUT2  */
    MYBET = 282,                   /* MYBET  */
    YELL = 283,                    /* YELL  */
    TELL = 284,                    /* TELL  */
    RESIGN = 285,                  /* RESIGN  */
    KOMIREQUEST = 286,             /* KOMIREQUEST  */
    DISPUTEMATCHTYPE = 287,        /* DISPUTEMATCHTYPE  */
    XSHOUT = 288,                  /* XSHOUT  */
    DECLINE = 289,                 /* DECLINE  */
    JOIN = 290,                    /* JOIN  */
    LEAVE = 291,                   /* LEAVE  */
    NEWTITLE = 292,                /* NEWTITLE  */
    BROADCAST = 293,               /* BROADCAST  */
    ITBROADCAST = 294,             /* ITBROADCAST  */
    ENTERBYOYOMI = 295,            /* ENTERBYOYOMI  */
    NOTIME = 296,                  /* NOTIME  */
    PERSON = 297,                  /* PERSON  */
    BEEPING = 298,                 /* BEEPING  */
    PLAYERON = 299,                /* PLAYERON  */
    PROBA = 300,                   /* PROBA  */
    STORED = 301,                  /* STORED  */
    IDLE = 302,                    /* IDLE  */
    PROMPT = 303,                  /* PROMPT  */
    GAMES = 304,                   /* GAMES  */
    REMOVE = 305,                  /* REMOVE  */
    MOVE = 306,                    /* MOVE  */
    GAME = 307,                    /* GAME  */
    OVEROBSERVE = 308,             /* OVEROBSERVE  */
    MESSAGES = 309,                /* MESSAGES  */
    NEWMATCH = 310,                /* NEWMATCH  */
    STATUSLINE = 311,              /* STATUSLINE  */
    CHANNEL = 312,                 /* CHANNEL  */
    CHANGECHANNEL = 313,           /* CHANGECHANNEL  */
    FREE = 314,                    /* FREE  */
    TEXTFILE = 315,                /* TEXTFILE  */
    FIRSTREMOVE = 316,             /* FIRSTREMOVE  */
    REVIEWTYPE = 317,              /* REVIEWTYPE  */
    GAMECOLOR = 318,               /* GAMECOLOR  */
    GAMESECONDS = 319,             /* GAMESECONDS  */
    BYOYOMI = 320,                 /* BYOYOMI  */
    MATCHTYPE = 321,               /* MATCHTYPE  */
    NATURAL = 322,                 /* NATURAL  */
    BETRESULT = 323,               /* BETRESULT  */
    RATING = 324,                  /* RATING  */
    STOREDNUM = 325,               /* STOREDNUM  */
    UNDO = 326,                    /* UNDO  */
    END = 327,                     /* END  */
    FAIL = 328,                    /* FAIL  */
    OLDPROMPT = 329,               /* OLDPROMPT  */
    SEMIPROMPT = 330,              /* SEMIPROMPT  */
    INFOMESSAGE = 331,             /* INFOMESSAGE  */
    LUSER = 332,                   /* LUSER  */
    OLDPASSWORD = 333,             /* OLDPASSWORD  */
    PASSWORD = 334,                /* PASSWORD  */
    INVALIDPASSWORD = 335,         /* INVALIDPASSWORD  */
    IGSENTRY = 336,                /* IGSENTRY  */
    TITLESET = 337,                /* TITLESET  */
    TOGGLE = 338,                  /* TOGGLE  */
    PLAYERS = 339,                 /* PLAYERS  */
    UNKNOWNANSWER = 340,           /* UNKNOWNANSWER  */
    MATCHCLOSED = 341,             /* MATCHCLOSED  */
    MATCHOPEN = 342,               /* MATCHOPEN  */
    OBSERVE = 343,                 /* OBSERVE  */
    WATCHING = 344,                /* WATCHING  */
    EXTSTATSENTRY = 345,           /* EXTSTATSENTRY  */
    ADD = 346,                     /* ADD  */
    KIBITZ = 347,                  /* KIBITZ  */
    KOMISET = 348,                 /* KOMISET  */
    TRANSLATION = 349,             /* TRANSLATION  */
    GAMETIME = 350,                /* GAMETIME  */
    LOSTCONNECTION = 351,          /* LOSTCONNECTION  */
    MYADJOURN = 352,               /* MYADJOURN  */
    RESTORE = 353,                 /* RESTORE  */
    RESTART = 354,                 /* RESTART  */
    NOTURN = 355,                  /* NOTURN  */
    GAMESAVED = 356,               /* GAMESAVED  */
    UNDID = 357,                   /* UNDID  */
    EMPTY = 358,                   /* EMPTY  */
    DONE = 359,                    /* DONE  */
    RESTORESCORING = 360,          /* RESTORESCORING  */
    STATUSHEADER = 361,            /* STATUSHEADER  */
    REMOVELIBERTY = 362,           /* REMOVELIBERTY  */
    OBSERVEWHILEPLAY = 363,        /* OBSERVEWHILEPLAY  */
    NOTELLTARGET = 364,            /* NOTELLTARGET  */
    GMTTIME = 365,                 /* GMTTIME  */
    LOCALTIME = 366,               /* LOCALTIME  */
    SERVERUP = 367,                /* SERVERUP  */
    UPTIMEENTRY = 368,             /* UPTIMEENTRY  */
    THROWCOPY = 369,               /* THROWCOPY  */
    SORRY = 370,                   /* SORRY  */
    WRONGCHANNEL = 371,            /* WRONGCHANNEL  */
    AUTOMATCHREQUEST = 372,        /* AUTOMATCHREQUEST  */
    DISPUTE = 373,                 /* DISPUTE  */
    OPPONENTDISPUTE = 374,         /* OPPONENTDISPUTE  */
    LATEFREE = 375,                /* LATEFREE  */
    NOPLAY = 376,                  /* NOPLAY  */
    CHANNELHEADER = 377,           /* CHANNELHEADER  */
    OBSERVERS = 378,               /* OBSERVERS  */
    GAMENOTFOUND = 379,            /* GAMENOTFOUND  */
    MATCHREQUEST = 380,            /* MATCHREQUEST  */
    GOEMATCHREQUEST = 381,         /* GOEMATCHREQUEST  */
    TOURNAMENTMATCHREQUEST = 382,  /* TOURNAMENTMATCHREQUEST  */
    TOURNAMENTGOEMATCHREQUEST = 383, /* TOURNAMENTGOEMATCHREQUEST  */
    USERESIGN = 384,               /* USERESIGN  */
    GAMETITLE = 385,               /* GAMETITLE  */
    ERASE = 386,                   /* ERASE  */
    PLEASEREDONE = 387,            /* PLEASEREDONE  */
    TELLTARGET = 388,              /* TELLTARGET  */
    TELLOFF = 389,                 /* TELLOFF  */
    NOREMOVETURN = 390,            /* NOREMOVETURN  */
    ADJOURNSENTREQUEST = 391,      /* ADJOURNSENTREQUEST  */
    ADJOURNREQUEST = 392,          /* ADJOURNREQUEST  */
    OPPONENTNOTON = 393,           /* OPPONENTNOTON  */
    NOLOAD = 394,                  /* NOLOAD  */
    DISAGREEREMOVE = 395,          /* DISAGREEREMOVE  */
    OPPDISAGREEREMOVE = 396,       /* OPPDISAGREEREMOVE  */
    DECLINEADJOURN = 397,          /* DECLINEADJOURN  */
    REVIEWLIST = 398,              /* REVIEWLIST  */
    REVIEWSTOP = 399,              /* REVIEWSTOP  */
    REVNODE = 400,                 /* REVNODE  */
    REVCOMMENT = 401,              /* REVCOMMENT  */
    REVEVENT = 402,                /* REVEVENT  */
    REVRESULT = 403,               /* REVRESULT  */
    REVPLACE = 404,                /* REVPLACE  */
    REVUSER = 405,                 /* REVUSER  */
    REVDATE = 406,                 /* REVDATE  */
    REVKOMI = 407,                 /* REVKOMI  */
    REVGAMENAME = 408,             /* REVGAMENAME  */
    REVWHITERANK = 409,            /* REVWHITERANK  */
    REVBLACKRANK = 410,            /* REVBLACKRANK  */
    REVWHITENAME = 411,            /* REVWHITENAME  */
    REVBLACKNAME = 412,            /* REVBLACKNAME  */
    REVSIZE = 413,                 /* REVSIZE  */
    REVGAME = 414,                 /* REVGAME  */
    REVBLACK = 415,                /* REVBLACK  */
    REVWHITE = 416,                /* REVWHITE  */
    REVADDBLACK = 417,             /* REVADDBLACK  */
    REVADDWHITE = 418,             /* REVADDWHITE  */
    REVADDEMPTY = 419,             /* REVADDEMPTY  */
    REVNODENAME = 420,             /* REVNODENAME  */
    REVIEWEND = 421,               /* REVIEWEND  */
    REVBLACKTIME = 422,            /* REVBLACKTIME  */
    REVWHITETIME = 423,            /* REVWHITETIME  */
    REVCOPYRIGHT = 424,            /* REVCOPYRIGHT  */
    REVHANDICAP = 425,             /* REVHANDICAP  */
    REVLETTERS = 426,              /* REVLETTERS  */
    REVIEWVARIATIONS = 427,        /* REVIEWVARIATIONS  */
    NOREVIEW = 428,                /* NOREVIEW  */
    SGFLIST = 429,                 /* SGFLIST  */
    NOSGF = 430,                   /* NOSGF  */
    NOMOREMOVES = 431,             /* NOMOREMOVES  */
    BETWINNERS = 432,              /* BETWINNERS  */
    BETEVEN = 433,                 /* BETEVEN  */
    BETLOSERS = 434,               /* BETLOSERS  */
    USER = 435,                    /* USER  */
    CURRENTSCORE = 436,            /* CURRENTSCORE  */
    FINALSCORE = 437,              /* FINALSCORE  */
    TEAMGAME = 438,                /* TEAMGAME  */
    OBSERVETEAM = 439,             /* OBSERVETEAM  */
    RESTARTTEAMGAME = 440,         /* RESTARTTEAMGAME  */
    SETPROBA = 441,                /* SETPROBA  */
    NOTREVIEWING = 442,            /* NOTREVIEWING  */
    NOTREQUESTGAME = 443           /* NOTREQUESTGAME  */
  };
  typedef enum IgsYYtokentype IgsYYtoken_kind_t;
#endif
/* Token kinds.  */
#define YYEMPTY -2
#define YYEOF 0
#define YYerror 256
#define YYUNDEF 257
#define NAME 258
#define SERVERMESSAGE 259
#define STATSENTRY 260
#define ILLEGALMOVE 261
#define ILLEGALUNDO 262
#define REQUESTINGMATCH 263
#define REMOVEGAMEFILE 264
#define MAILED 265
#define REMOVEGROUP 266
#define GIVEBYOYOMI 267
#define RESULTLINE 268
#define INVALID 269
#define AUTOMATCHDISPUTE 270
#define NEWCHANNEL 271
#define MUSTPASS 272
#define OPPMUSTPASS 273
#define GUEST 274
#define TELLDONE 275
#define REVIEWSTART 276
#define REVLITERAL 277
#define REVUNKNOWN 278
#define WELCOME 279
#define SERVERFULL 280
#define XSHOUT2 281
#define MYBET 282
#define YELL 283
#define TELL 284
#define RESIGN 285
#define KOMIREQUEST 286
#define DISPUTEMATCHTYPE 287
#define XSHOUT 288
#define DECLINE 289
#define JOIN 290
#define LEAVE 291
#define NEWTITLE 292
#define BROADCAST 293
#define ITBROADCAST 294
#define ENTERBYOYOMI 295
#define NOTIME 296
#define PERSON 297
#define BEEPING 298
#define PLAYERON 299
#define PROBA 300
#define STORED 301
#define IDLE 302
#define PROMPT 303
#define GAMES 304
#define REMOVE 305
#define MOVE 306
#define GAME 307
#define OVEROBSERVE 308
#define MESSAGES 309
#define NEWMATCH 310
#define STATUSLINE 311
#define CHANNEL 312
#define CHANGECHANNEL 313
#define FREE 314
#define TEXTFILE 315
#define FIRSTREMOVE 316
#define REVIEWTYPE 317
#define GAMECOLOR 318
#define GAMESECONDS 319
#define BYOYOMI 320
#define MATCHTYPE 321
#define NATURAL 322
#define BETRESULT 323
#define RATING 324
#define STOREDNUM 325
#define UNDO 326
#define END 327
#define FAIL 328
#define OLDPROMPT 329
#define SEMIPROMPT 330
#define INFOMESSAGE 331
#define LUSER 332
#define OLDPASSWORD 333
#define PASSWORD 334
#define INVALIDPASSWORD 335
#define IGSENTRY 336
#define TITLESET 337
#define TOGGLE 338
#define PLAYERS 339
#define UNKNOWNANSWER 340
#define MATCHCLOSED 341
#define MATCHOPEN 342
#define OBSERVE 343
#define WATCHING 344
#define EXTSTATSENTRY 345
#define ADD 346
#define KIBITZ 347
#define KOMISET 348
#define TRANSLATION 349
#define GAMETIME 350
#define LOSTCONNECTION 351
#define MYADJOURN 352
#define RESTORE 353
#define RESTART 354
#define NOTURN 355
#define GAMESAVED 356
#define UNDID 357
#define EMPTY 358
#define DONE 359
#define RESTORESCORING 360
#define STATUSHEADER 361
#define REMOVELIBERTY 362
#define OBSERVEWHILEPLAY 363
#define NOTELLTARGET 364
#define GMTTIME 365
#define LOCALTIME 366
#define SERVERUP 367
#define UPTIMEENTRY 368
#define THROWCOPY 369
#define SORRY 370
#define WRONGCHANNEL 371
#define AUTOMATCHREQUEST 372
#define DISPUTE 373
#define OPPONENTDISPUTE 374
#define LATEFREE 375
#define NOPLAY 376
#define CHANNELHEADER 377
#define OBSERVERS 378
#define GAMENOTFOUND 379
#define MATCHREQUEST 380
#define GOEMATCHREQUEST 381
#define TOURNAMENTMATCHREQUEST 382
#define TOURNAMENTGOEMATCHREQUEST 383
#define USERESIGN 384
#define GAMETITLE 385
#define ERASE 386
#define PLEASEREDONE 387
#define TELLTARGET 388
#define TELLOFF 389
#define NOREMOVETURN 390
#define ADJOURNSENTREQUEST 391
#define ADJOURNREQUEST 392
#define OPPONENTNOTON 393
#define NOLOAD 394
#define DISAGREEREMOVE 395
#define OPPDISAGREEREMOVE 396
#define DECLINEADJOURN 397
#define REVIEWLIST 398
#define REVIEWSTOP 399
#define REVNODE 400
#define REVCOMMENT 401
#define REVEVENT 402
#define REVRESULT 403
#define REVPLACE 404
#define REVUSER 405
#define REVDATE 406
#define REVKOMI 407
#define REVGAMENAME 408
#define REVWHITERANK 409
#define REVBLACKRANK 410
#define REVWHITENAME 411
#define REVBLACKNAME 412
#define REVSIZE 413
#define REVGAME 414
#define REVBLACK 415
#define REVWHITE 416
#define REVADDBLACK 417
#define REVADDWHITE 418
#define REVADDEMPTY 419
#define REVNODENAME 420
#define REVIEWEND 421
#define REVBLACKTIME 422
#define REVWHITETIME 423
#define REVCOPYRIGHT 424
#define REVHANDICAP 425
#define REVLETTERS 426
#define REVIEWVARIATIONS 427
#define NOREVIEW 428
#define SGFLIST 429
#define NOSGF 430
#define NOMOREMOVES 431
#define BETWINNERS 432
#define BETEVEN 433
#define BETLOSERS 434
#define USER 435
#define CURRENTSCORE 436
#define FINALSCORE 437
#define TEAMGAME 438
#define OBSERVETEAM 439
#define RESTARTTEAMGAME 440
#define SETPROBA 441
#define NOTREVIEWING 442
#define NOTREQUESTGAME 443

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 68 "gointer.y"

   char         *Name;
   int           Value;
   NameVal      *Nameval;
   NumVal       *Numval;
   NameList     *Namelist;
   NameListList *NameListlist;
   ChannelData  *Channeldata;
   GameDesc     *Gamedesc;
   Game         *Game;
   Player       *Person;
   void         *Dummy;
   DisputeDesc  *Disputedesc;
   BetDesc      *Bet;

#line 584 "y.tab.c"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE IgsYYlval;


int IgsYYparse (void);


#endif /* !YY_YY_Y_TAB_H_INCLUDED  */
/* Symbol kind.  */
enum IgsYYsymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_NAME = 3,                       /* NAME  */
  YYSYMBOL_SERVERMESSAGE = 4,              /* SERVERMESSAGE  */
  YYSYMBOL_STATSENTRY = 5,                 /* STATSENTRY  */
  YYSYMBOL_ILLEGALMOVE = 6,                /* ILLEGALMOVE  */
  YYSYMBOL_ILLEGALUNDO = 7,                /* ILLEGALUNDO  */
  YYSYMBOL_REQUESTINGMATCH = 8,            /* REQUESTINGMATCH  */
  YYSYMBOL_REMOVEGAMEFILE = 9,             /* REMOVEGAMEFILE  */
  YYSYMBOL_MAILED = 10,                    /* MAILED  */
  YYSYMBOL_REMOVEGROUP = 11,               /* REMOVEGROUP  */
  YYSYMBOL_GIVEBYOYOMI = 12,               /* GIVEBYOYOMI  */
  YYSYMBOL_RESULTLINE = 13,                /* RESULTLINE  */
  YYSYMBOL_INVALID = 14,                   /* INVALID  */
  YYSYMBOL_AUTOMATCHDISPUTE = 15,          /* AUTOMATCHDISPUTE  */
  YYSYMBOL_NEWCHANNEL = 16,                /* NEWCHANNEL  */
  YYSYMBOL_MUSTPASS = 17,                  /* MUSTPASS  */
  YYSYMBOL_OPPMUSTPASS = 18,               /* OPPMUSTPASS  */
  YYSYMBOL_GUEST = 19,                     /* GUEST  */
  YYSYMBOL_TELLDONE = 20,                  /* TELLDONE  */
  YYSYMBOL_REVIEWSTART = 21,               /* REVIEWSTART  */
  YYSYMBOL_REVLITERAL = 22,                /* REVLITERAL  */
  YYSYMBOL_REVUNKNOWN = 23,                /* REVUNKNOWN  */
  YYSYMBOL_WELCOME = 24,                   /* WELCOME  */
  YYSYMBOL_SERVERFULL = 25,                /* SERVERFULL  */
  YYSYMBOL_XSHOUT2 = 26,                   /* XSHOUT2  */
  YYSYMBOL_MYBET = 27,                     /* MYBET  */
  YYSYMBOL_YELL = 28,                      /* YELL  */
  YYSYMBOL_TELL = 29,                      /* TELL  */
  YYSYMBOL_RESIGN = 30,                    /* RESIGN  */
  YYSYMBOL_KOMIREQUEST = 31,               /* KOMIREQUEST  */
  YYSYMBOL_DISPUTEMATCHTYPE = 32,          /* DISPUTEMATCHTYPE  */
  YYSYMBOL_XSHOUT = 33,                    /* XSHOUT  */
  YYSYMBOL_DECLINE = 34,                   /* DECLINE  */
  YYSYMBOL_JOIN = 35,                      /* JOIN  */
  YYSYMBOL_LEAVE = 36,                     /* LEAVE  */
  YYSYMBOL_NEWTITLE = 37,                  /* NEWTITLE  */
  YYSYMBOL_BROADCAST = 38,                 /* BROADCAST  */
  YYSYMBOL_ITBROADCAST = 39,               /* ITBROADCAST  */
  YYSYMBOL_ENTERBYOYOMI = 40,              /* ENTERBYOYOMI  */
  YYSYMBOL_NOTIME = 41,                    /* NOTIME  */
  YYSYMBOL_PERSON = 42,                    /* PERSON  */
  YYSYMBOL_BEEPING = 43,                   /* BEEPING  */
  YYSYMBOL_PLAYERON = 44,                  /* PLAYERON  */
  YYSYMBOL_PROBA = 45,                     /* PROBA  */
  YYSYMBOL_STORED = 46,                    /* STORED  */
  YYSYMBOL_IDLE = 47,                      /* IDLE  */
  YYSYMBOL_PROMPT = 48,                    /* PROMPT  */
  YYSYMBOL_GAMES = 49,                     /* GAMES  */
  YYSYMBOL_REMOVE = 50,                    /* REMOVE  */
  YYSYMBOL_MOVE = 51,                      /* MOVE  */
  YYSYMBOL_GAME = 52,                      /* GAME  */
  YYSYMBOL_OVEROBSERVE = 53,               /* OVEROBSERVE  */
  YYSYMBOL_MESSAGES = 54,                  /* MESSAGES  */
  YYSYMBOL_NEWMATCH = 55,                  /* NEWMATCH  */
  YYSYMBOL_STATUSLINE = 56,                /* STATUSLINE  */
  YYSYMBOL_CHANNEL = 57,                   /* CHANNEL  */
  YYSYMBOL_CHANGECHANNEL = 58,             /* CHANGECHANNEL  */
  YYSYMBOL_FREE = 59,                      /* FREE  */
  YYSYMBOL_TEXTFILE = 60,                  /* TEXTFILE  */
  YYSYMBOL_FIRSTREMOVE = 61,               /* FIRSTREMOVE  */
  YYSYMBOL_REVIEWTYPE = 62,                /* REVIEWTYPE  */
  YYSYMBOL_GAMECOLOR = 63,                 /* GAMECOLOR  */
  YYSYMBOL_GAMESECONDS = 64,               /* GAMESECONDS  */
  YYSYMBOL_BYOYOMI = 65,                   /* BYOYOMI  */
  YYSYMBOL_MATCHTYPE = 66,                 /* MATCHTYPE  */
  YYSYMBOL_NATURAL = 67,                   /* NATURAL  */
  YYSYMBOL_BETRESULT = 68,                 /* BETRESULT  */
  YYSYMBOL_RATING = 69,                    /* RATING  */
  YYSYMBOL_STOREDNUM = 70,                 /* STOREDNUM  */
  YYSYMBOL_UNDO = 71,                      /* UNDO  */
  YYSYMBOL_END = 72,                       /* END  */
  YYSYMBOL_FAIL = 73,                      /* FAIL  */
  YYSYMBOL_OLDPROMPT = 74,                 /* OLDPROMPT  */
  YYSYMBOL_SEMIPROMPT = 75,                /* SEMIPROMPT  */
  YYSYMBOL_INFOMESSAGE = 76,               /* INFOMESSAGE  */
  YYSYMBOL_LUSER = 77,                     /* LUSER  */
  YYSYMBOL_OLDPASSWORD = 78,               /* OLDPASSWORD  */
  YYSYMBOL_PASSWORD = 79,                  /* PASSWORD  */
  YYSYMBOL_INVALIDPASSWORD = 80,           /* INVALIDPASSWORD  */
  YYSYMBOL_IGSENTRY = 81,                  /* IGSENTRY  */
  YYSYMBOL_TITLESET = 82,                  /* TITLESET  */
  YYSYMBOL_TOGGLE = 83,                    /* TOGGLE  */
  YYSYMBOL_PLAYERS = 84,                   /* PLAYERS  */
  YYSYMBOL_UNKNOWNANSWER = 85,             /* UNKNOWNANSWER  */
  YYSYMBOL_MATCHCLOSED = 86,               /* MATCHCLOSED  */
  YYSYMBOL_MATCHOPEN = 87,                 /* MATCHOPEN  */
  YYSYMBOL_OBSERVE = 88,                   /* OBSERVE  */
  YYSYMBOL_WATCHING = 89,                  /* WATCHING  */
  YYSYMBOL_EXTSTATSENTRY = 90,             /* EXTSTATSENTRY  */
  YYSYMBOL_ADD = 91,                       /* ADD  */
  YYSYMBOL_KIBITZ = 92,                    /* KIBITZ  */
  YYSYMBOL_KOMISET = 93,                   /* KOMISET  */
  YYSYMBOL_TRANSLATION = 94,               /* TRANSLATION  */
  YYSYMBOL_GAMETIME = 95,                  /* GAMETIME  */
  YYSYMBOL_LOSTCONNECTION = 96,            /* LOSTCONNECTION  */
  YYSYMBOL_MYADJOURN = 97,                 /* MYADJOURN  */
  YYSYMBOL_RESTORE = 98,                   /* RESTORE  */
  YYSYMBOL_RESTART = 99,                   /* RESTART  */
  YYSYMBOL_NOTURN = 100,                   /* NOTURN  */
  YYSYMBOL_GAMESAVED = 101,                /* GAMESAVED  */
  YYSYMBOL_UNDID = 102,                    /* UNDID  */
  YYSYMBOL_EMPTY = 103,                    /* EMPTY  */
  YYSYMBOL_DONE = 104,                     /* DONE  */
  YYSYMBOL_RESTORESCORING = 105,           /* RESTORESCORING  */
  YYSYMBOL_STATUSHEADER = 106,             /* STATUSHEADER  */
  YYSYMBOL_REMOVELIBERTY = 107,            /* REMOVELIBERTY  */
  YYSYMBOL_OBSERVEWHILEPLAY = 108,         /* OBSERVEWHILEPLAY  */
  YYSYMBOL_NOTELLTARGET = 109,             /* NOTELLTARGET  */
  YYSYMBOL_GMTTIME = 110,                  /* GMTTIME  */
  YYSYMBOL_LOCALTIME = 111,                /* LOCALTIME  */
  YYSYMBOL_SERVERUP = 112,                 /* SERVERUP  */
  YYSYMBOL_UPTIMEENTRY = 113,              /* UPTIMEENTRY  */
  YYSYMBOL_THROWCOPY = 114,                /* THROWCOPY  */
  YYSYMBOL_SORRY = 115,                    /* SORRY  */
  YYSYMBOL_WRONGCHANNEL = 116,             /* WRONGCHANNEL  */
  YYSYMBOL_AUTOMATCHREQUEST = 117,         /* AUTOMATCHREQUEST  */
  YYSYMBOL_DISPUTE = 118,                  /* DISPUTE  */
  YYSYMBOL_OPPONENTDISPUTE = 119,          /* OPPONENTDISPUTE  */
  YYSYMBOL_LATEFREE = 120,                 /* LATEFREE  */
  YYSYMBOL_NOPLAY = 121,                   /* NOPLAY  */
  YYSYMBOL_CHANNELHEADER = 122,            /* CHANNELHEADER  */
  YYSYMBOL_OBSERVERS = 123,                /* OBSERVERS  */
  YYSYMBOL_GAMENOTFOUND = 124,             /* GAMENOTFOUND  */
  YYSYMBOL_MATCHREQUEST = 125,             /* MATCHREQUEST  */
  YYSYMBOL_GOEMATCHREQUEST = 126,          /* GOEMATCHREQUEST  */
  YYSYMBOL_TOURNAMENTMATCHREQUEST = 127,   /* TOURNAMENTMATCHREQUEST  */
  YYSYMBOL_TOURNAMENTGOEMATCHREQUEST = 128, /* TOURNAMENTGOEMATCHREQUEST  */
  YYSYMBOL_USERESIGN = 129,                /* USERESIGN  */
  YYSYMBOL_GAMETITLE = 130,                /* GAMETITLE  */
  YYSYMBOL_ERASE = 131,                    /* ERASE  */
  YYSYMBOL_PLEASEREDONE = 132,             /* PLEASEREDONE  */
  YYSYMBOL_TELLTARGET = 133,               /* TELLTARGET  */
  YYSYMBOL_TELLOFF = 134,                  /* TELLOFF  */
  YYSYMBOL_NOREMOVETURN = 135,             /* NOREMOVETURN  */
  YYSYMBOL_ADJOURNSENTREQUEST = 136,       /* ADJOURNSENTREQUEST  */
  YYSYMBOL_ADJOURNREQUEST = 137,           /* ADJOURNREQUEST  */
  YYSYMBOL_OPPONENTNOTON = 138,            /* OPPONENTNOTON  */
  YYSYMBOL_NOLOAD = 139,                   /* NOLOAD  */
  YYSYMBOL_DISAGREEREMOVE = 140,           /* DISAGREEREMOVE  */
  YYSYMBOL_OPPDISAGREEREMOVE = 141,        /* OPPDISAGREEREMOVE  */
  YYSYMBOL_DECLINEADJOURN = 142,           /* DECLINEADJOURN  */
  YYSYMBOL_REVIEWLIST = 143,               /* REVIEWLIST  */
  YYSYMBOL_REVIEWSTOP = 144,               /* REVIEWSTOP  */
  YYSYMBOL_REVNODE = 145,                  /* REVNODE  */
  YYSYMBOL_REVCOMMENT = 146,               /* REVCOMMENT  */
  YYSYMBOL_REVEVENT = 147,                 /* REVEVENT  */
  YYSYMBOL_REVRESULT = 148,                /* REVRESULT  */
  YYSYMBOL_REVPLACE = 149,                 /* REVPLACE  */
  YYSYMBOL_REVUSER = 150,                  /* REVUSER  */
  YYSYMBOL_REVDATE = 151,                  /* REVDATE  */
  YYSYMBOL_REVKOMI = 152,                  /* REVKOMI  */
  YYSYMBOL_REVGAMENAME = 153,              /* REVGAMENAME  */
  YYSYMBOL_REVWHITERANK = 154,             /* REVWHITERANK  */
  YYSYMBOL_REVBLACKRANK = 155,             /* REVBLACKRANK  */
  YYSYMBOL_REVWHITENAME = 156,             /* REVWHITENAME  */
  YYSYMBOL_REVBLACKNAME = 157,             /* REVBLACKNAME  */
  YYSYMBOL_REVSIZE = 158,                  /* REVSIZE  */
  YYSYMBOL_REVGAME = 159,                  /* REVGAME  */
  YYSYMBOL_REVBLACK = 160,                 /* REVBLACK  */
  YYSYMBOL_REVWHITE = 161,                 /* REVWHITE  */
  YYSYMBOL_REVADDBLACK = 162,              /* REVADDBLACK  */
  YYSYMBOL_REVADDWHITE = 163,              /* REVADDWHITE  */
  YYSYMBOL_REVADDEMPTY = 164,              /* REVADDEMPTY  */
  YYSYMBOL_REVNODENAME = 165,              /* REVNODENAME  */
  YYSYMBOL_REVIEWEND = 166,                /* REVIEWEND  */
  YYSYMBOL_REVBLACKTIME = 167,             /* REVBLACKTIME  */
  YYSYMBOL_REVWHITETIME = 168,             /* REVWHITETIME  */
  YYSYMBOL_REVCOPYRIGHT = 169,             /* REVCOPYRIGHT  */
  YYSYMBOL_REVHANDICAP = 170,              /* REVHANDICAP  */
  YYSYMBOL_REVLETTERS = 171,               /* REVLETTERS  */
  YYSYMBOL_REVIEWVARIATIONS = 172,         /* REVIEWVARIATIONS  */
  YYSYMBOL_NOREVIEW = 173,                 /* NOREVIEW  */
  YYSYMBOL_SGFLIST = 174,                  /* SGFLIST  */
  YYSYMBOL_NOSGF = 175,                    /* NOSGF  */
  YYSYMBOL_NOMOREMOVES = 176,              /* NOMOREMOVES  */
  YYSYMBOL_BETWINNERS = 177,               /* BETWINNERS  */
  YYSYMBOL_BETEVEN = 178,                  /* BETEVEN  */
  YYSYMBOL_BETLOSERS = 179,                /* BETLOSERS  */
  YYSYMBOL_USER = 180,                     /* USER  */
  YYSYMBOL_CURRENTSCORE = 181,             /* CURRENTSCORE  */
  YYSYMBOL_FINALSCORE = 182,               /* FINALSCORE  */
  YYSYMBOL_TEAMGAME = 183,                 /* TEAMGAME  */
  YYSYMBOL_OBSERVETEAM = 184,              /* OBSERVETEAM  */
  YYSYMBOL_RESTARTTEAMGAME = 185,          /* RESTARTTEAMGAME  */
  YYSYMBOL_SETPROBA = 186,                 /* SETPROBA  */
  YYSYMBOL_NOTREVIEWING = 187,             /* NOTREVIEWING  */
  YYSYMBOL_NOTREQUESTGAME = 188,           /* NOTREQUESTGAME  */
  YYSYMBOL_189_ = 189,                     /* '['  */
  YYSYMBOL_190_ = 190,                     /* ']'  */
  YYSYMBOL_191_ = 191,                     /* '}'  */
  YYSYMBOL_192_ = 192,                     /* ':'  */
  YYSYMBOL_193_ = 193,                     /* '@'  */
  YYSYMBOL_194_ = 194,                     /* '>'  */
  YYSYMBOL_195_ = 195,                     /* '<'  */
  YYSYMBOL_196_ = 196,                     /* '('  */
  YYSYMBOL_197_ = 197,                     /* ')'  */
  YYSYMBOL_YYACCEPT = 198,                 /* $accept  */
  YYSYMBOL_start = 199,                    /* start  */
  YYSYMBOL_session = 200,                  /* session  */
  YYSYMBOL_pass = 201,                     /* pass  */
  YYSYMBOL_202_1 = 202,                    /* $@1  */
  YYSYMBOL_203_2 = 203,                    /* $@2  */
  YYSYMBOL_enterorfail = 204,              /* enterorfail  */
  YYSYMBOL_205_3 = 205,                    /* $@3  */
  YYSYMBOL_loginmessages = 206,            /* loginmessages  */
  YYSYMBOL_loginmessage = 207,             /* loginmessage  */
  YYSYMBOL_inputs = 208,                   /* inputs  */
  YYSYMBOL_prompt = 209,                   /* prompt  */
  YYSYMBOL_moreinput = 210,                /* moreinput  */
  YYSYMBOL_input = 211,                    /* input  */
  YYSYMBOL_textfile = 212,                 /* textfile  */
  YYSYMBOL_erase = 213,                    /* erase  */
  YYSYMBOL_igsentry = 214,                 /* igsentry  */
  YYSYMBOL_servermessages = 215,           /* servermessages  */
  YYSYMBOL_servermessage = 216,            /* servermessage  */
  YYSYMBOL_xshout = 217,                   /* xshout  */
  YYSYMBOL_infomessage = 218,              /* infomessage  */
  YYSYMBOL_tell = 219,                     /* tell  */
  YYSYMBOL_playeron = 220,                 /* playeron  */
  YYSYMBOL_beeping = 221,                  /* beeping  */
  YYSYMBOL_idle = 222,                     /* idle  */
  YYSYMBOL_stored = 223,                   /* stored  */
  YYSYMBOL_broadcast = 224,                /* broadcast  */
  YYSYMBOL_kibitz = 225,                   /* kibitz  */
  YYSYMBOL_messages = 226,                 /* messages  */
  YYSYMBOL_yell = 227,                     /* yell  */
  YYSYMBOL_join = 228,                     /* join  */
  YYSYMBOL_leave = 229,                    /* leave  */
  YYSYMBOL_newtitle = 230,                 /* newtitle  */
  YYSYMBOL_changechannel = 231,            /* changechannel  */
  YYSYMBOL_wrongchannel = 232,             /* wrongchannel  */
  YYSYMBOL_matchopen = 233,                /* matchopen  */
  YYSYMBOL_matchclosed = 234,              /* matchclosed  */
  YYSYMBOL_automatchrequest = 235,         /* automatchrequest  */
  YYSYMBOL_automatchdispute = 236,         /* automatchdispute  */
  YYSYMBOL_ruledmatchrequest = 237,        /* ruledmatchrequest  */
  YYSYMBOL_matchrequest = 238,             /* matchrequest  */
  YYSYMBOL_requestingmatch = 239,          /* requestingmatch  */
  YYSYMBOL_komirequest = 240,              /* komirequest  */
  YYSYMBOL_komiset = 241,                  /* komiset  */
  YYSYMBOL_freemessage = 242,              /* freemessage  */
  YYSYMBOL_freeconfirm = 243,              /* freeconfirm  */
  YYSYMBOL_latefree = 244,                 /* latefree  */
  YYSYMBOL_noplay = 245,                   /* noplay  */
  YYSYMBOL_noload = 246,                   /* noload  */
  YYSYMBOL_titleset = 247,                 /* titleset  */
  YYSYMBOL_statsentry = 248,               /* statsentry  */
  YYSYMBOL_statsentries = 249,             /* statsentries  */
  YYSYMBOL_extendstatsentry = 250,         /* extendstatsentry  */
  YYSYMBOL_optextend = 251,                /* optextend  */
  YYSYMBOL_stats = 252,                    /* stats  */
  YYSYMBOL_betentry = 253,                 /* betentry  */
  YYSYMBOL_betentries = 254,               /* betentries  */
  YYSYMBOL_optmybet = 255,                 /* optmybet  */
  YYSYMBOL_bet = 256,                      /* bet  */
  YYSYMBOL_toggle = 257,                   /* toggle  */
  YYSYMBOL_channelentry = 258,             /* channelentry  */
  YYSYMBOL_channelentries = 259,           /* channelentries  */
  YYSYMBOL_channels = 260,                 /* channels  */
  YYSYMBOL_observerentries = 261,          /* observerentries  */
  YYSYMBOL_observers = 262,                /* observers  */
  YYSYMBOL_gamenotfound = 263,             /* gamenotfound  */
  YYSYMBOL_nomoremoves = 264,              /* nomoremoves  */
  YYSYMBOL_notrequestgame = 265,           /* notrequestgame  */
  YYSYMBOL_gamesline = 266,                /* gamesline  */
  YYSYMBOL_gameslines = 267,               /* gameslines  */
  YYSYMBOL_games = 268,                    /* games  */
  YYSYMBOL_269_4 = 269,                    /* $@4  */
  YYSYMBOL_remove = 270,                   /* remove  */
  YYSYMBOL_move = 271,                     /* move  */
  YYSYMBOL_movelist = 272,                 /* movelist  */
  YYSYMBOL_optgamesaved = 273,             /* optgamesaved  */
  YYSYMBOL_gamedesc = 274,                 /* gamedesc  */
  YYSYMBOL_optgametitle = 275,             /* optgametitle  */
  YYSYMBOL_add = 276,                      /* add  */
  YYSYMBOL_doneobserve = 277,              /* doneobserve  */
  YYSYMBOL_mustpass = 278,                 /* mustpass  */
  YYSYMBOL_oppmustpass = 279,              /* oppmustpass  */
  YYSYMBOL_disagreeremove = 280,           /* disagreeremove  */
  YYSYMBOL_opponentdisagreeremove = 281,   /* opponentdisagreeremove  */
  YYSYMBOL_optnotreviewing = 282,          /* optnotreviewing  */
  YYSYMBOL_observe = 283,                  /* observe  */
  YYSYMBOL_optfirst = 284,                 /* optfirst  */
  YYSYMBOL_doneopponentobserve = 285,      /* doneopponentobserve  */
  YYSYMBOL_opponentobserve = 286,          /* opponentobserve  */
  YYSYMBOL_opponentoptobserve = 287,       /* opponentoptobserve  */
  YYSYMBOL_betresult = 288,                /* betresult  */
  YYSYMBOL_undidlist = 289,                /* undidlist  */
  YYSYMBOL_undid = 290,                    /* undid  */
  YYSYMBOL_opponentundid = 291,            /* opponentundid  */
  YYSYMBOL_restore = 292,                  /* restore  */
  YYSYMBOL_opponentrestart = 293,          /* opponentrestart  */
  YYSYMBOL_restart = 294,                  /* restart  */
  YYSYMBOL_newmatch1 = 295,                /* newmatch1  */
  YYSYMBOL_newmatch2 = 296,                /* newmatch2  */
  YYSYMBOL_decline = 297,                  /* decline  */
  YYSYMBOL_disputeline = 298,              /* disputeline  */
  YYSYMBOL_disputelines = 299,             /* disputelines  */
  YYSYMBOL_opponentdispute = 300,          /* opponentdispute  */
  YYSYMBOL_dispute = 301,                  /* dispute  */
  YYSYMBOL_matchtypes = 302,               /* matchtypes  */
  YYSYMBOL_disputematchtype = 303,         /* disputematchtype  */
  YYSYMBOL_optobserve = 304,               /* optobserve  */
  YYSYMBOL_undolist = 305,                 /* undolist  */
  YYSYMBOL_undo = 306,                     /* undo  */
  YYSYMBOL_watching = 307,                 /* watching  */
  YYSYMBOL_overobserve = 308,              /* overobserve  */
  YYSYMBOL_observewhileplay = 309,         /* observewhileplay  */
  YYSYMBOL_playerline = 310,               /* playerline  */
  YYSYMBOL_playerlines = 311,              /* playerlines  */
  YYSYMBOL_playersstatusline = 312,        /* playersstatusline  */
  YYSYMBOL_players = 313,                  /* players  */
  YYSYMBOL_314_5 = 314,                    /* $@5  */
  YYSYMBOL_315_6 = 315,                    /* $@6  */
  YYSYMBOL_userline = 316,                 /* userline  */
  YYSYMBOL_userlines = 317,                /* userlines  */
  YYSYMBOL_users = 318,                    /* users  */
  YYSYMBOL_player = 319,                   /* player  */
  YYSYMBOL_playertime = 320,               /* playertime  */
  YYSYMBOL_optbyo = 321,                   /* optbyo  */
  YYSYMBOL_gametime = 322,                 /* gametime  */
  YYSYMBOL_gamescore = 323,                /* gamescore  */
  YYSYMBOL_translation = 324,              /* translation  */
  YYSYMBOL_translations = 325,             /* translations  */
  YYSYMBOL_byoyomi = 326,                  /* byoyomi  */
  YYSYMBOL_notime = 327,                   /* notime  */
  YYSYMBOL_lostconnection = 328,           /* lostconnection  */
  YYSYMBOL_gamesaved = 329,                /* gamesaved  */
  YYSYMBOL_optadjourn = 330,               /* optadjourn  */
  YYSYMBOL_adjourn = 331,                  /* adjourn  */
  YYSYMBOL_adjournsentrequest = 332,       /* adjournsentrequest  */
  YYSYMBOL_adjournrequest = 333,           /* adjournrequest  */
  YYSYMBOL_oppadjourn = 334,               /* oppadjourn  */
  YYSYMBOL_declineadjourn = 335,           /* declineadjourn  */
  YYSYMBOL_resign = 336,                   /* resign  */
  YYSYMBOL_mailed = 337,                   /* mailed  */
  YYSYMBOL_removegamefile = 338,           /* removegamefile  */
  YYSYMBOL_notelltarget = 339,             /* notelltarget  */
  YYSYMBOL_telltarget = 340,               /* telltarget  */
  YYSYMBOL_telldone = 341,                 /* telldone  */
  YYSYMBOL_telloff = 342,                  /* telloff  */
  YYSYMBOL_illegalmove = 343,              /* illegalmove  */
  YYSYMBOL_illegalundo = 344,              /* illegalundo  */
  YYSYMBOL_noturn = 345,                   /* noturn  */
  YYSYMBOL_noremoveturn = 346,             /* noremoveturn  */
  YYSYMBOL_useresign = 347,                /* useresign  */
  YYSYMBOL_removeliberty = 348,            /* removeliberty  */
  YYSYMBOL_removegroup = 349,              /* removegroup  */
  YYSYMBOL_restorescoring = 350,           /* restorescoring  */
  YYSYMBOL_pleaseredone = 351,             /* pleaseredone  */
  YYSYMBOL_statusheader = 352,             /* statusheader  */
  YYSYMBOL_statusline = 353,               /* statusline  */
  YYSYMBOL_statuslines = 354,              /* statuslines  */
  YYSYMBOL_resultline = 355,               /* resultline  */
  YYSYMBOL_status = 356,                   /* status  */
  YYSYMBOL_date = 357,                     /* date  */
  YYSYMBOL_uptimeentry = 358,              /* uptimeentry  */
  YYSYMBOL_uptime = 359,                   /* uptime  */
  YYSYMBOL_sgflist = 360,                  /* sgflist  */
  YYSYMBOL_reviewlist = 361,               /* reviewlist  */
  YYSYMBOL_reviewvariations = 362,         /* reviewvariations  */
  YYSYMBOL_reviewstart = 363,              /* reviewstart  */
  YYSYMBOL_reviewliterals = 364,           /* reviewliterals  */
  YYSYMBOL_reviewentry = 365,              /* reviewentry  */
  YYSYMBOL_reviewentries = 366,            /* reviewentries  */
  YYSYMBOL_review = 367,                   /* review  */
  YYSYMBOL_368_7 = 368,                    /* $@7  */
  YYSYMBOL_auxreviews = 369,               /* auxreviews  */
  YYSYMBOL_reviews = 370,                  /* reviews  */
  YYSYMBOL_reviewstop = 371,               /* reviewstop  */
  YYSYMBOL_noreview = 372,                 /* noreview  */
  YYSYMBOL_throwcopy = 373,                /* throwcopy  */
  YYSYMBOL_proba = 374,                    /* proba  */
  YYSYMBOL_setproba = 375,                 /* setproba  */
  YYSYMBOL_sorry = 376,                    /* sorry  */
  YYSYMBOL_invalid = 377,                  /* invalid  */
  YYSYMBOL_unknown = 378,                  /* unknown  */
  YYSYMBOL_names = 379,                    /* names  */
  YYSYMBOL_namesset = 380,                 /* namesset  */
  YYSYMBOL_promptnames = 381,              /* promptnames  */
  YYSYMBOL_promptname = 382,               /* promptname  */
  YYSYMBOL_optname = 383,                  /* optname  */
  YYSYMBOL_literallines = 384              /* literallines  */
};
typedef enum IgsYYsymbol_kind_t IgsYYsymbol_kind_t;




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
typedef __INT_LEAST8_TYPE__ IgsYYtype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t IgsYYtype_int8;
#else
typedef signed char IgsYYtype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ IgsYYtype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t IgsYYtype_int16;
#else
typedef short IgsYYtype_int16;
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
typedef __UINT_LEAST8_TYPE__ IgsYYtype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t IgsYYtype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char IgsYYtype_uint8;
#else
typedef short IgsYYtype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ IgsYYtype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t IgsYYtype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short IgsYYtype_uint16;
#else
typedef int IgsYYtype_uint16;
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
typedef IgsYYtype_int16 IgsYY_state_t;

/* State numbers in computations.  */
typedef int IgsYY_state_fast_t;

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

/* Suppress an incorrect diagnostic about IgsYYlval being uninitialized.  */
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

#if !defined IgsYYoverflow

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
#endif /* !defined IgsYYoverflow */

#if (! defined IgsYYoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union IgsYYalloc
{
  IgsYY_state_t IgsYYss_alloc;
  YYSTYPE IgsYYvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union IgsYYalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (IgsYY_state_t) + YYSIZEOF (YYSTYPE)) \
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
        YYPTRDIFF_T IgsYYnewbytes;                                         \
        YYCOPY (&IgsYYptr->Stack_alloc, Stack, IgsYYsize);                    \
        Stack = &IgsYYptr->Stack_alloc;                                    \
        IgsYYnewbytes = IgsYYstacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        IgsYYptr += IgsYYnewbytes / YYSIZEOF (*IgsYYptr);                        \
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
          YYPTRDIFF_T IgsYYi;                      \
          for (IgsYYi = 0; IgsYYi < (Count); IgsYYi++)   \
            (Dst)[IgsYYi] = (Src)[IgsYYi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  4
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   729

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  198
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  187
/* YYNRULES -- Number of rules.  */
#define YYNRULES  412
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  776

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   443


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by IgsYYlex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (IgsYYsymbol_kind_t, IgsYYtranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by IgsYYlex.  */
static const IgsYYtype_uint8 IgsYYtranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     196,   197,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,   192,     2,
     195,     2,   194,     2,   193,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   189,     2,   190,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,   191,     2,     2,     2,     2,
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
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   129,   130,   131,   132,   133,   134,
     135,   136,   137,   138,   139,   140,   141,   142,   143,   144,
     145,   146,   147,   148,   149,   150,   151,   152,   153,   154,
     155,   156,   157,   158,   159,   160,   161,   162,   163,   164,
     165,   166,   167,   168,   169,   170,   171,   172,   173,   174,
     175,   176,   177,   178,   179,   180,   181,   182,   183,   184,
     185,   186,   187,   188
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const IgsYYtype_int16 IgsYYrline[] =
{
       0,   138,   138,   139,   142,   160,   167,   175,   174,   183,
     182,   198,   197,   206,   209,   210,   213,   218,   224,   260,
     268,   269,   272,   278,   285,   298,   299,   300,   301,   302,
     303,   304,   305,   306,   307,   308,   309,   310,   311,   312,
     313,   314,   315,   316,   317,   318,   319,   320,   321,   322,
     323,   324,   325,   326,   327,   328,   329,   330,   331,   332,
     333,   334,   335,   336,   337,   338,   339,   340,   341,   342,
     343,   344,   345,   346,   347,   348,   349,   350,   351,   352,
     353,   354,   355,   356,   357,   358,   359,   360,   361,   362,
     363,   364,   365,   366,   367,   368,   369,   370,   371,   372,
     373,   374,   375,   376,   377,   378,   379,   380,   381,   382,
     383,   384,   385,   386,   387,   388,   389,   390,   391,   392,
     393,   394,   395,   396,   397,   398,   399,   405,   406,   407,
     408,   409,   410,   411,   412,   413,   414,   415,   416,   417,
     418,   419,   424,   427,   450,   456,   473,   474,   475,   478,
     485,   490,   499,   510,   520,   529,   540,   554,   570,   575,
     582,   589,   593,   599,   606,   612,   617,   624,   636,   650,
     656,   663,   669,   675,   682,   688,   695,   701,   707,   714,
     723,   724,   725,   726,   729,   741,   748,   761,   774,   781,
     788,   796,   802,   808,   819,   830,   837,   840,   843,   870,
     880,   893,   902,   908,   913,   914,   917,   938,   950,   958,
     971,   986,   993,   995,  1009,  1026,  1035,  1044,  1073,  1074,
    1078,  1077,  1087,  1090,  1107,  1115,  1125,  1126,  1129,  1155,
    1190,  1191,  1194,  1197,  1203,  1210,  1217,  1228,  1237,  1243,
    1246,  1268,  1290,  1291,  1330,  1339,  1351,  1361,  1373,  1377,
    1383,  1389,  1396,  1404,  1414,  1427,  1442,  1445,  1453,  1470,
    1482,  1499,  1502,  1512,  1518,  1531,  1539,  1545,  1558,  1571,
    1576,  1581,  1587,  1590,  1594,  1601,  1610,  1617,  1627,  1634,
    1640,  1646,  1654,  1661,  1663,  1668,  1680,  1693,  1697,  1692,
    1703,  1729,  1746,  1759,  1766,  1773,  1781,  1793,  1799,  1802,
    1818,  1839,  1846,  1847,  1850,  1858,  1864,  1870,  1879,  1882,
    1886,  1894,  1899,  1907,  1913,  1919,  1924,  1931,  1938,  1945,
    1952,  1958,  1974,  1979,  1990,  1995,  2002,  2009,  2015,  2021,
    2027,  2033,  2040,  2046,  2052,  2058,  2066,  2074,  2082,  2090,
    2099,  2128,  2140,  2154,  2170,  2179,  2180,  2183,  2188,  2194,
    2201,  2215,  2222,  2234,  2244,  2245,  2246,  2250,  2261,  2270,
    2279,  2284,  2289,  2294,  2299,  2304,  2309,  2314,  2319,  2324,
    2329,  2334,  2339,  2344,  2349,  2354,  2363,  2372,  2377,  2386,
    2395,  2400,  2405,  2412,  2413,  2417,  2416,  2423,  2424,  2427,
    2431,  2437,  2443,  2449,  2456,  2465,  2470,  2479,  2484,  2491,
    2498,  2503,  2514,  2524,  2533,  2543,  2544,  2547,  2548,  2555,
    2556,  2559,  2586
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (IgsYYsymbol_kind_t, IgsYYstos[State])

#if YYDEBUG || 0
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *IgsYYsymbol_name (IgsYYsymbol_kind_t IgsYYsymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const IgsYYtname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "NAME",
  "SERVERMESSAGE", "STATSENTRY", "ILLEGALMOVE", "ILLEGALUNDO",
  "REQUESTINGMATCH", "REMOVEGAMEFILE", "MAILED", "REMOVEGROUP",
  "GIVEBYOYOMI", "RESULTLINE", "INVALID", "AUTOMATCHDISPUTE", "NEWCHANNEL",
  "MUSTPASS", "OPPMUSTPASS", "GUEST", "TELLDONE", "REVIEWSTART",
  "REVLITERAL", "REVUNKNOWN", "WELCOME", "SERVERFULL", "XSHOUT2", "MYBET",
  "YELL", "TELL", "RESIGN", "KOMIREQUEST", "DISPUTEMATCHTYPE", "XSHOUT",
  "DECLINE", "JOIN", "LEAVE", "NEWTITLE", "BROADCAST", "ITBROADCAST",
  "ENTERBYOYOMI", "NOTIME", "PERSON", "BEEPING", "PLAYERON", "PROBA",
  "STORED", "IDLE", "PROMPT", "GAMES", "REMOVE", "MOVE", "GAME",
  "OVEROBSERVE", "MESSAGES", "NEWMATCH", "STATUSLINE", "CHANNEL",
  "CHANGECHANNEL", "FREE", "TEXTFILE", "FIRSTREMOVE", "REVIEWTYPE",
  "GAMECOLOR", "GAMESECONDS", "BYOYOMI", "MATCHTYPE", "NATURAL",
  "BETRESULT", "RATING", "STOREDNUM", "UNDO", "END", "FAIL", "OLDPROMPT",
  "SEMIPROMPT", "INFOMESSAGE", "LUSER", "OLDPASSWORD", "PASSWORD",
  "INVALIDPASSWORD", "IGSENTRY", "TITLESET", "TOGGLE", "PLAYERS",
  "UNKNOWNANSWER", "MATCHCLOSED", "MATCHOPEN", "OBSERVE", "WATCHING",
  "EXTSTATSENTRY", "ADD", "KIBITZ", "KOMISET", "TRANSLATION", "GAMETIME",
  "LOSTCONNECTION", "MYADJOURN", "RESTORE", "RESTART", "NOTURN",
  "GAMESAVED", "UNDID", "EMPTY", "DONE", "RESTORESCORING", "STATUSHEADER",
  "REMOVELIBERTY", "OBSERVEWHILEPLAY", "NOTELLTARGET", "GMTTIME",
  "LOCALTIME", "SERVERUP", "UPTIMEENTRY", "THROWCOPY", "SORRY",
  "WRONGCHANNEL", "AUTOMATCHREQUEST", "DISPUTE", "OPPONENTDISPUTE",
  "LATEFREE", "NOPLAY", "CHANNELHEADER", "OBSERVERS", "GAMENOTFOUND",
  "MATCHREQUEST", "GOEMATCHREQUEST", "TOURNAMENTMATCHREQUEST",
  "TOURNAMENTGOEMATCHREQUEST", "USERESIGN", "GAMETITLE", "ERASE",
  "PLEASEREDONE", "TELLTARGET", "TELLOFF", "NOREMOVETURN",
  "ADJOURNSENTREQUEST", "ADJOURNREQUEST", "OPPONENTNOTON", "NOLOAD",
  "DISAGREEREMOVE", "OPPDISAGREEREMOVE", "DECLINEADJOURN", "REVIEWLIST",
  "REVIEWSTOP", "REVNODE", "REVCOMMENT", "REVEVENT", "REVRESULT",
  "REVPLACE", "REVUSER", "REVDATE", "REVKOMI", "REVGAMENAME",
  "REVWHITERANK", "REVBLACKRANK", "REVWHITENAME", "REVBLACKNAME",
  "REVSIZE", "REVGAME", "REVBLACK", "REVWHITE", "REVADDBLACK",
  "REVADDWHITE", "REVADDEMPTY", "REVNODENAME", "REVIEWEND", "REVBLACKTIME",
  "REVWHITETIME", "REVCOPYRIGHT", "REVHANDICAP", "REVLETTERS",
  "REVIEWVARIATIONS", "NOREVIEW", "SGFLIST", "NOSGF", "NOMOREMOVES",
  "BETWINNERS", "BETEVEN", "BETLOSERS", "USER", "CURRENTSCORE",
  "FINALSCORE", "TEAMGAME", "OBSERVETEAM", "RESTARTTEAMGAME", "SETPROBA",
  "NOTREVIEWING", "NOTREQUESTGAME", "'['", "']'", "'}'", "':'", "'@'",
  "'>'", "'<'", "'('", "')'", "$accept", "start", "session", "pass", "$@1",
  "$@2", "enterorfail", "$@3", "loginmessages", "loginmessage", "inputs",
  "prompt", "moreinput", "input", "textfile", "erase", "igsentry",
  "servermessages", "servermessage", "xshout", "infomessage", "tell",
  "playeron", "beeping", "idle", "stored", "broadcast", "kibitz",
  "messages", "yell", "join", "leave", "newtitle", "changechannel",
  "wrongchannel", "matchopen", "matchclosed", "automatchrequest",
  "automatchdispute", "ruledmatchrequest", "matchrequest",
  "requestingmatch", "komirequest", "komiset", "freemessage",
  "freeconfirm", "latefree", "noplay", "noload", "titleset", "statsentry",
  "statsentries", "extendstatsentry", "optextend", "stats", "betentry",
  "betentries", "optmybet", "bet", "toggle", "channelentry",
  "channelentries", "channels", "observerentries", "observers",
  "gamenotfound", "nomoremoves", "notrequestgame", "gamesline",
  "gameslines", "games", "$@4", "remove", "move", "movelist",
  "optgamesaved", "gamedesc", "optgametitle", "add", "doneobserve",
  "mustpass", "oppmustpass", "disagreeremove", "opponentdisagreeremove",
  "optnotreviewing", "observe", "optfirst", "doneopponentobserve",
  "opponentobserve", "opponentoptobserve", "betresult", "undidlist",
  "undid", "opponentundid", "restore", "opponentrestart", "restart",
  "newmatch1", "newmatch2", "decline", "disputeline", "disputelines",
  "opponentdispute", "dispute", "matchtypes", "disputematchtype",
  "optobserve", "undolist", "undo", "watching", "overobserve",
  "observewhileplay", "playerline", "playerlines", "playersstatusline",
  "players", "$@5", "$@6", "userline", "userlines", "users", "player",
  "playertime", "optbyo", "gametime", "gamescore", "translation",
  "translations", "byoyomi", "notime", "lostconnection", "gamesaved",
  "optadjourn", "adjourn", "adjournsentrequest", "adjournrequest",
  "oppadjourn", "declineadjourn", "resign", "mailed", "removegamefile",
  "notelltarget", "telltarget", "telldone", "telloff", "illegalmove",
  "illegalundo", "noturn", "noremoveturn", "useresign", "removeliberty",
  "removegroup", "restorescoring", "pleaseredone", "statusheader",
  "statusline", "statuslines", "resultline", "status", "date",
  "uptimeentry", "uptime", "sgflist", "reviewlist", "reviewvariations",
  "reviewstart", "reviewliterals", "reviewentry", "reviewentries",
  "review", "$@7", "auxreviews", "reviews", "reviewstop", "noreview",
  "throwcopy", "proba", "setproba", "sorry", "invalid", "unknown", "names",
  "namesset", "promptnames", "promptname", "optname", "literallines", YY_NULLPTR
};

static const char *
IgsYYsymbol_name (IgsYYsymbol_kind_t IgsYYsymbol)
{
  return IgsYYtname[IgsYYsymbol];
}
#endif

#define YYPACT_NINF (-399)

#define IgsYYpact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-403)

#define IgsYYtable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const IgsYYtype_int16 IgsYYpact[] =
{
      32,  -399,    77,    78,  -399,    49,  -399,  -399,  -399,  -399,
    -399,  -399,  -399,  -399,  -399,  -399,  -399,   541,    23,    23,
    -399,    55,    85,     4,  -399,  -399,  -399,   126,  -399,  -399,
    -399,  -399,  -399,  -399,  -399,  -399,  -399,    92,   176,   151,
     218,  -399,   222,  -399,   223,   224,   216,  -399,  -399,   141,
     111,   160,   228,  -399,  -399,   229,  -399,  -399,   158,  -399,
     145,  -399,  -399,   182,   232,   234,  -399,   235,  -399,   236,
    -399,  -399,  -399,  -399,    27,  -399,  -399,   237,   238,   239,
     240,   147,    36,  -399,   182,  -399,   242,   243,   244,    68,
    -399,  -399,  -399,  -399,  -399,   246,   246,   247,   248,  -399,
    -399,   250,  -399,  -399,  -399,   251,  -399,   252,  -399,  -399,
    -399,  -399,  -399,  -399,  -399,  -399,   253,  -399,  -399,  -399,
    -399,   108,  -399,  -399,  -399,  -399,  -399,  -399,  -399,    82,
    -399,  -399,  -399,   255,   150,  -399,  -399,     1,  -399,  -399,
    -399,   244,  -399,  -399,  -399,  -399,  -399,  -399,  -399,  -399,
    -399,  -399,  -399,  -399,  -399,  -399,  -399,  -399,  -399,  -399,
    -399,  -399,  -399,  -399,  -399,  -399,  -399,  -399,  -399,  -399,
    -399,  -399,  -399,  -399,  -399,    15,  -399,  -399,  -399,  -399,
     245,  -399,  -399,  -399,  -399,  -399,  -399,  -399,   204,  -399,
    -399,  -399,  -399,  -399,  -399,  -399,  -399,  -399,  -399,   -15,
    -399,  -399,  -399,  -399,  -399,  -399,  -399,  -399,  -399,  -399,
    -399,   141,  -399,  -399,  -399,  -399,  -399,  -399,  -399,  -399,
    -399,   166,  -399,  -399,  -399,  -399,  -399,  -399,  -399,  -399,
    -399,  -399,  -399,  -399,  -399,  -399,  -399,  -399,  -399,  -399,
    -399,  -399,  -399,  -399,  -399,  -399,  -399,   156,  -399,  -399,
    -399,   105,  -399,  -399,  -399,  -399,  -399,   -51,  -399,  -399,
    -399,  -399,  -399,  -399,  -399,  -399,  -399,  -399,  -399,  -399,
    -399,  -399,   141,  -399,   260,    40,  -399,  -399,  -399,  -399,
     124,  -399,  -399,  -399,  -399,  -399,  -399,   195,  -399,  -399,
    -399,  -399,    69,   263,  -399,  -399,   264,  -399,    47,  -399,
    -399,  -399,  -399,   265,     9,  -399,   266,  -399,   267,  -399,
      11,   204,  -399,    50,    83,    79,  -399,  -399,    81,   173,
    -399,   174,  -399,   141,   188,  -399,  -399,  -399,  -399,   202,
      58,   275,   207,   209,   277,  -399,  -399,    62,   241,   241,
    -399,    86,   212,   260,  -399,   260,   260,  -399,    63,    -8,
      33,   103,   219,  -399,  -399,  -399,  -399,  -399,     7,  -399,
    -399,   197,  -399,  -399,  -399,   -13,   286,  -399,    30,  -399,
    -399,  -399,  -399,  -399,  -399,  -399,  -399,  -399,  -399,  -399,
     287,   249,   290,  -399,  -399,  -399,    -5,   291,    16,   292,
     293,   104,  -399,   294,   237,  -399,   296,   297,   298,  -399,
    -399,   299,   -24,  -399,   182,   -13,   182,  -399,   300,  -399,
    -399,   301,  -399,   254,  -399,   302,  -399,  -399,   256,  -399,
    -399,  -399,    12,   303,   304,   306,    66,  -399,   307,   308,
     309,  -399,   -29,  -399,    -9,   311,  -399,   259,    51,   101,
     313,   237,  -399,   315,  -399,  -399,  -399,   305,   310,   312,
     314,   316,   317,   318,   319,   320,   321,   323,   325,   326,
     327,   328,   329,  -399,  -399,  -399,   330,   331,   332,   333,
     334,  -399,  -399,  -399,  -399,  -399,   131,   322,   134,   354,
    -399,   355,  -399,  -399,   137,   136,   356,   258,   173,  -399,
     357,  -399,  -399,   173,  -399,  -399,   358,   359,   360,   361,
     139,   -11,  -399,  -399,  -399,   362,   138,  -399,  -399,  -399,
     363,   173,   148,   364,   173,   365,    -6,   366,  -399,  -399,
    -399,  -399,  -399,   110,   367,   368,   369,   351,  -399,  -399,
    -399,  -399,  -399,  -399,  -399,  -399,  -399,  -399,  -399,  -399,
    -399,  -399,  -399,  -399,   351,   351,   351,  -399,  -399,  -399,
    -399,  -399,   351,   335,    22,   371,   372,   373,   374,   113,
     375,  -399,   376,   285,  -399,   378,   -12,   -23,   190,   380,
     273,   381,   324,  -399,   382,  -399,   383,  -399,  -399,  -399,
     384,  -399,   385,   386,  -399,  -399,  -399,  -399,   237,   193,
    -399,  -399,   389,   237,   390,  -399,   337,     3,  -399,   391,
    -399,   394,   395,   396,   397,   398,  -399,     5,   399,   400,
     339,   213,  -399,    48,  -399,    10,   402,   403,  -399,   408,
     217,   409,    13,   225,   226,  -399,   411,   412,   415,   416,
     231,   227,   418,   419,   173,   173,   233,   352,   370,   257,
    -399,  -399,  -399,   379,   423,   424,   425,   426,   427,  -399,
     428,   388,   392,   429,   261,   393,   430,   431,   432,   401,
     433,  -399,  -399,   434,  -399,  -399,   404,   164,   141,   405,
     435,   436,   270,   262,    18,   437,  -399,  -399,   438,   440,
    -399,   268,   271,   272,   -47,   441,   442,  -399,   141,   338,
     338,   406,   443,   444,   410,   413,   276,   445,   447,   449,
     414,   278,  -399,  -399,   417,  -399,  -399,   101,  -399,   173,
     173,   338,   451,   452,  -399,  -399,   420,   421,   453,   281,
     459,   460,   173,  -399,  -399,  -399,   469,   471,  -399,  -399,
     472,   422,  -399,   288,   295,  -399,   478,   336,   480,   481,
     484,   485,   439,  -399,   446,   487,  -399,  -399,   340,   407,
       6,   341,   448,   492,   493,   342,   494,  -399,  -399,   343,
     495,   496,   497,   344,   498,   346,   450,   500,   348,  -399,
     454,   460,  -399,   295,   455,  -399
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const IgsYYtype_int16 IgsYYdefact[] =
{
       0,     3,     0,    15,     1,     0,    16,     9,    17,    19,
      18,     7,     6,    21,    14,    15,    15,     0,    13,    13,
     141,     0,     0,   324,   326,   185,   319,   318,   331,   338,
     399,   402,   402,   234,   235,   322,   351,     0,     0,   315,
       0,   270,     0,   263,     0,     0,     0,   305,   162,   273,
       0,     0,     0,   220,   222,     0,   279,   169,     0,   174,
     189,   402,   385,     0,     0,     0,     4,     0,   193,     0,
     287,   412,   177,   176,     0,   402,   232,     0,     0,     0,
       0,     0,   313,   256,     0,   327,     0,     0,   137,   243,
     332,   402,   330,   280,   320,     0,     0,     0,     0,   393,
     398,     0,   402,   266,   266,     0,   191,     0,   214,   180,
     181,   182,   183,   329,   144,   333,     0,   402,   328,   311,
     312,     0,   236,   237,   314,   402,   391,   402,   392,   402,
     215,   203,   293,     0,     0,   216,    20,     0,    58,    59,
      25,    26,   147,    27,    28,    32,   108,    29,    30,    31,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,   402,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,   196,   199,    56,    57,    60,   210,
     211,    61,    62,    63,    64,    65,    66,    67,   225,    68,
      69,    71,    70,    72,    73,    74,    75,    76,    77,     0,
      87,    88,    78,    79,    80,    81,    82,    83,    84,    85,
      86,   273,    89,    90,    91,    92,    94,    93,    95,    96,
     303,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   110,   109,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,     0,   124,   125,
     346,   126,   127,   128,   129,   130,   388,   389,   131,   132,
     133,   134,   135,   136,   138,   139,   140,    11,    10,     8,
     149,   194,   273,   317,   179,     0,   151,   158,   316,   186,
       0,   150,   165,   166,   304,   272,   160,     0,   395,   164,
     163,   219,     0,     0,   171,   172,     0,   188,     0,   384,
     225,   249,   248,     0,     0,   145,     0,   284,   400,   161,
       0,     0,   261,     0,     0,     0,   187,   301,     0,   227,
     308,     0,   225,   273,   251,   148,   242,   225,   233,     0,
       0,     0,     0,     0,     0,   344,   175,     0,   268,   267,
     190,     0,     0,   323,   192,   349,   350,   348,     0,     0,
     402,     0,     0,   397,    22,    23,    24,   146,     0,   402,
     195,     0,   200,   209,   262,   231,     0,   225,     0,   302,
     337,   345,   390,   387,   406,   325,   401,   404,   269,   271,
       0,   221,     0,   170,   173,   143,     0,     0,     0,     0,
       0,     0,   411,     0,     0,   278,     0,     0,     0,   306,
     226,     0,   231,   307,     0,   231,     0,   334,     0,   341,
     342,     0,   178,     0,   265,     0,   321,   347,     0,   203,
     202,   292,     0,     0,     0,     0,     0,   198,     0,     0,
       0,   224,   227,   250,   231,     0,   225,   339,     0,   208,
       0,     0,   218,     0,   353,   386,   356,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   353,   353,   353,     0,     0,     0,     0,
       0,   353,   354,   355,   383,   275,     0,     0,     0,     0,
     288,     0,   283,   159,     0,     0,     0,     0,   227,   257,
       0,   259,   225,   227,   225,   244,     0,     0,     0,     0,
       0,     0,   290,   291,   300,     0,     0,   197,   223,   230,
       0,   227,   239,     0,   227,     0,   231,     0,   336,   408,
      12,   407,   405,     0,     0,     0,     0,   357,   359,   367,
     366,   364,   362,   365,   360,   368,   369,   370,   371,   372,
     373,   374,   376,   375,   380,   381,   382,   358,   379,   378,
     363,   361,   377,     0,     0,     0,     0,   410,     0,     0,
       0,   295,     0,     0,   310,     0,   227,   231,     0,     0,
       0,     0,     0,   203,     0,   402,     0,   245,   238,   240,
       0,   252,     0,     0,   276,   335,   403,   394,     0,     0,
     352,   153,     0,     0,     0,   409,     0,     0,   289,     0,
     282,     0,     0,     0,     0,     0,   254,     0,     0,     0,
       0,     0,   201,   205,   396,     0,     0,     0,   274,     0,
       0,     0,     0,     0,     0,   207,     0,     0,     0,     0,
       0,     0,     0,     0,   227,   227,     0,     0,     0,     0,
     204,   206,   402,     0,     0,     0,     0,     0,     0,   402,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   247,   246,     0,   343,   264,     0,     0,   273,     0,
       0,     0,     0,     0,     0,     0,   154,   152,     0,     0,
     281,     0,     0,     0,   231,     0,     0,   404,   273,   231,
     231,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   258,   260,     0,   340,   213,   212,   184,   227,
     227,   231,     0,     0,   157,   155,     0,     0,     0,     0,
       0,     0,   227,   241,   253,   277,     0,     0,   156,   286,
       0,     0,   168,     0,   298,   255,     0,     0,     0,     0,
       0,     0,     0,   402,     0,     0,   167,   296,     0,     0,
       0,   228,     0,     0,     0,     0,     0,   285,   297,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   217,
       0,     0,   229,   298,     0,   299
};

/* YYPGOTO[NTERM-NUM].  */
static const IgsYYtype_int16 IgsYYpgoto[] =
{
    -399,  -399,  -399,  -399,  -399,  -399,   486,  -399,   208,  -399,
    -399,  -399,  -399,  -399,  -399,  -399,  -399,  -399,   -42,  -399,
    -399,  -399,  -399,  -399,  -399,  -399,  -399,  -399,  -399,  -399,
    -399,  -399,  -399,  -399,  -399,  -399,  -399,  -399,  -399,  -399,
    -399,  -399,  -399,  -399,  -399,  -399,  -399,  -399,  -399,  -399,
     350,  -399,   143,  -399,  -399,  -399,  -396,  -399,  -399,  -399,
     349,  -399,  -399,  -399,  -399,  -399,  -399,  -399,  -399,  -399,
    -399,  -399,  -399,  -399,  -305,  -314,   -60,  -398,  -399,  -399,
    -399,  -399,  -399,  -399,  -399,   121,  -399,  -399,   456,  -399,
    -399,  -399,  -399,  -399,  -399,  -399,  -399,  -399,   457,  -399,
    -399,   461,  -399,  -399,  -399,  -399,  -209,  -399,  -399,  -399,
    -399,  -399,  -399,  -399,  -399,  -399,  -399,  -399,  -399,  -399,
    -399,  -386,  -265,  -266,  -399,  -399,   289,  -399,  -399,  -399,
    -399,   491,  -399,  -399,  -399,  -399,  -399,  -399,  -399,  -399,
    -399,  -399,  -399,  -399,  -399,  -399,  -399,  -399,  -399,  -399,
    -399,  -399,  -399,  -399,   269,  -399,  -399,  -399,  -399,   464,
     279,  -399,  -399,  -399,  -399,  -399,  -251,  -399,  -399,   345,
    -399,  -399,  -399,  -399,  -399,  -399,  -399,  -399,  -399,  -399,
    -399,   -31,  -178,  -399,  -399,  -399,  -399
};

/* YYDEFGOTO[NTERM-NUM].  */
static const IgsYYtype_int16 IgsYYdefgoto[] =
{
       0,     2,     3,    13,    16,    15,   268,   374,     5,    14,
      17,   356,   136,   137,   138,   139,   140,   141,   142,   143,
     144,   145,   146,   147,   148,   149,   150,   151,   152,   153,
     154,   155,   156,   157,   158,   159,   160,   161,   162,   163,
     164,   165,   166,   167,   168,   169,   170,   171,   172,   173,
     174,   175,   361,   362,   176,   420,   349,   641,   177,   178,
     179,   180,   181,   706,   182,   183,   184,   185,   442,   381,
     186,   291,   187,   431,   365,   512,   188,   432,   189,   190,
     191,   192,   193,   194,   579,   195,   329,   196,   197,   495,
     198,   199,   200,   201,   202,   203,   204,   205,   206,   207,
     414,   338,   208,   209,   280,   210,   286,   211,   212,   213,
     214,   215,   482,   391,   598,   216,   307,   558,   421,   350,
     217,   315,   734,   742,   218,   219,   220,   221,   222,   223,
     224,   400,   321,   226,   227,   228,   229,   230,   231,   232,
     233,   234,   235,   236,   237,   238,   239,   240,   241,   242,
     243,   244,   245,   246,   247,   518,   437,   248,   249,   332,
     250,   251,   252,   253,   254,   255,   527,   474,   386,   256,
     299,   257,   258,   259,   260,   261,   262,   263,   264,   265,
     266,   523,   439,   438,   522,   596,   308
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const IgsYYtype_int16 IgsYYtable[] =
{
     274,   275,   368,   300,   491,   399,   626,   493,   484,   376,
     376,    62,   388,   376,   311,   376,   648,   402,   444,   476,
      22,   376,   405,   501,   322,   592,     6,   428,   428,   327,
     298,   418,    -5,     1,   418,    -5,   514,    55,   428,   428,
     393,   702,   428,   376,   313,   428,   325,     8,     9,   354,
     376,    -5,     6,   376,   519,   525,    -5,    -5,   270,   511,
     330,   376,   434,   375,   489,   376,   376,   445,     7,   376,
     309,   337,    86,     8,     9,   640,   355,     4,    -2,    55,
     634,  -294,    55,   429,   502,   503,   343,   366,   271,    86,
     418,   555,   272,   635,   345,   276,   346,   267,   348,   357,
      10,   435,   310,   394,  -402,   359,   429,   429,  -294,    -5,
      -5,    -5,   377,   376,   403,   372,   599,   429,   584,   385,
      55,   429,   395,   520,   429,   521,    10,    11,    12,   326,
     407,   516,   358,   320,   412,   417,   273,  -309,   507,   367,
     446,   447,   448,   449,   450,   451,   452,   453,   454,   455,
     456,   457,   458,   459,   460,   461,   462,   463,   464,   465,
     466,   490,   467,   468,   469,   470,   471,   376,   573,   607,
     419,   430,   605,  -402,   564,   513,   480,   613,   583,   277,
     287,   278,   586,   288,   301,   600,   293,   566,   481,   567,
     378,   472,   473,   294,   295,   296,   379,   577,   389,   627,
     581,   425,   620,   755,   642,   649,   650,   623,   477,   695,
     328,   396,   544,   545,   546,    95,    96,    97,    98,   352,
     552,   279,   353,    18,    19,   281,   282,   283,   284,   285,
     289,   290,   292,   297,    55,   303,   688,   304,   305,   306,
     314,   316,   317,   318,   319,   323,   324,   344,    21,   331,
     334,   335,   606,   336,   340,   341,   342,   347,   351,   364,
      79,    32,    91,   376,   380,   382,   383,   384,   387,   390,
     392,   397,   396,   398,    86,   401,   404,   406,   408,   409,
     411,   410,   415,   413,   416,   423,   703,   359,   424,   433,
     440,   709,   710,   443,   475,   478,   479,   483,   441,   485,
     486,   487,   488,   496,   497,   499,   504,   505,   436,   506,
     508,   509,   510,   725,   515,   517,   524,   498,   526,   422,
     661,   662,   553,   500,   556,   554,   561,   528,   426,   560,
     563,   572,   529,   575,   530,   578,   531,   610,   532,   533,
     534,   535,   536,   537,   492,   538,   494,   539,   540,   541,
     542,   543,   547,   548,   549,   550,   551,   557,   559,   562,
     565,   568,   569,   570,   571,   574,   576,   580,   582,   585,
     587,   588,   589,   590,   593,   594,   595,   597,   601,   602,
     603,   604,   608,   609,   611,   614,   616,   617,   618,   619,
     621,   612,   622,   624,   628,   723,   724,   629,   630,   631,
     632,   633,   636,   637,   638,   643,   644,   591,   735,   625,
     639,   645,   647,   646,   653,   654,   651,   652,   655,   656,
     657,   659,   660,   658,   664,   663,   669,   670,   671,   672,
     673,   675,   678,   681,   682,   683,   685,   686,   691,   692,
     696,   697,   665,   698,   704,   705,   712,   713,   717,   666,
     718,   668,   719,   694,   726,   727,   730,   699,   679,   689,
     676,   700,   732,   733,   677,   680,   693,   716,   429,   701,
     721,   731,   736,   684,   737,   738,   687,   690,   711,   708,
     740,   743,   714,   745,   746,   715,   720,   747,   748,   722,
     752,   741,   728,   729,   739,   758,   759,   761,   763,   764,
     765,   767,   754,   770,   427,   269,   773,   774,   225,   707,
     369,   749,     0,     0,     0,     0,   370,     0,   751,   302,
     757,     0,   769,     0,   756,   360,   772,   775,     0,   363,
     371,   312,     0,   744,     0,     0,     0,   753,   760,   762,
     771,   766,    20,   768,   615,    21,    22,    23,    24,    25,
      26,    27,    28,     0,    29,    30,    31,    32,    33,    34,
     333,    35,    36,     0,     0,   339,     0,    37,     0,     0,
      38,    39,    40,    41,    42,    43,     0,     0,     0,    44,
      45,    46,    47,     0,    48,    49,    50,    51,    52,  -142,
      53,    54,     0,    55,    56,    57,     0,     0,    58,    59,
      60,    61,   373,    62,     0,     0,     0,     0,     0,    63,
       0,   667,    64,     0,     0,     0,  -142,    65,   674,     0,
       0,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,     0,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,     0,   107,   108,   109,   110,   111,   112,
     113,     0,   114,   115,   116,   117,   118,   119,   120,   121,
       0,   122,   123,   124,   125,   126,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   750,   127,   128,   129,     0,   130,   131,     0,
       0,   132,   133,     0,     0,     0,     0,   134,     0,   135
};

static const IgsYYtype_int16 IgsYYcheck[] =
{
      31,    32,   211,    63,   402,   319,     3,   405,   394,     3,
       3,    62,     3,     3,    74,     3,     3,   322,    23,     3,
       5,     3,   327,   419,    84,     3,     3,    51,    51,    89,
      61,    42,     0,     1,    42,     3,   434,    52,    51,    51,
      29,    88,    51,     3,    75,    51,    88,    24,    25,    48,
       3,    19,     3,     3,     3,   441,    24,    25,     3,    88,
      91,     3,   367,   272,    88,     3,     3,    72,    19,     3,
      43,   102,   101,    24,    25,    27,    75,     0,     0,    52,
      75,    48,    52,   130,    72,    73,   117,   102,     3,   101,
      42,   477,    88,    88,   125,     3,   127,    74,   129,   141,
      77,    71,    75,    92,     3,    90,   130,   130,    75,    77,
      78,    79,    72,     3,   323,   166,     3,   130,   516,    72,
      52,   130,    72,    72,   130,    74,    77,    78,    79,    61,
      72,   436,   163,    97,    72,    72,    10,   101,    72,   199,
     145,   146,   147,   148,   149,   150,   151,   152,   153,   154,
     155,   156,   157,   158,   159,   160,   161,   162,   163,   164,
     165,   185,   167,   168,   169,   170,   171,     3,   179,   567,
     178,   184,   184,    72,   488,   184,    72,   573,   184,     3,
      69,    30,    72,    72,    63,    72,    28,   492,    84,   494,
      66,   196,   197,    35,    36,    37,    72,   511,   189,   196,
     514,   194,   588,   197,   194,   192,   193,   593,   192,   191,
      89,   189,   463,   464,   465,   110,   111,   112,   113,    69,
     471,     3,    72,    15,    16,     3,     3,     3,    12,    88,
      70,     3,     3,    88,    52,     3,    72,     3,     3,     3,
       3,     3,     3,     3,    97,     3,     3,   139,     4,     3,
       3,     3,   566,     3,     3,     3,     3,   175,     3,    55,
      94,    16,   106,     3,    69,   196,     3,     3,     3,     3,
       3,   192,   189,   192,   101,   101,    88,    75,     3,    72,
       3,    72,   196,    42,    72,   182,   684,    90,    69,     3,
       3,   689,   690,     3,     3,     3,     3,     3,    49,     3,
       3,     3,     3,     3,     3,     3,     3,     3,   368,     3,
       3,     3,     3,   711,     3,    56,     3,    63,     3,   350,
     634,   635,   191,    67,   190,     3,   190,    22,   359,   192,
      72,   192,    22,   195,    22,   187,    22,    64,    22,    22,
      22,    22,    22,    22,   404,    22,   406,    22,    22,    22,
      22,    22,    22,    22,    22,    22,    22,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,    22,     3,     3,     3,     3,     3,     3,
      95,     3,   192,     3,     3,     3,     3,     3,     3,     3,
     197,    67,     3,     3,     3,   709,   710,     3,     3,     3,
       3,     3,     3,     3,    65,     3,     3,    72,   722,    72,
     197,     3,     3,   196,     3,     3,   191,   191,     3,     3,
     189,     3,     3,   196,    72,   192,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,    72,     3,     3,     3,     3,     3,     3,   192,
       3,    72,     3,   191,     3,     3,     3,   189,   197,   668,
      72,   190,     3,     3,    72,    72,   196,   191,   130,   197,
     192,   190,     3,    72,     3,     3,    72,    72,    72,   688,
     192,     3,    72,     3,     3,    72,    72,     3,     3,    72,
       3,   196,    72,    72,    72,     3,     3,     3,     3,     3,
       3,     3,    95,     3,   361,    19,   771,   773,    17,   687,
     221,    72,    -1,    -1,    -1,    -1,   247,    -1,    72,    63,
      72,    -1,    72,    -1,   183,   175,    72,    72,    -1,   180,
     251,    74,    -1,   197,    -1,    -1,    -1,   197,   196,   196,
     192,   197,     1,   197,   575,     4,     5,     6,     7,     8,
       9,    10,    11,    -1,    13,    14,    15,    16,    17,    18,
      96,    20,    21,    -1,    -1,   104,    -1,    26,    -1,    -1,
      29,    30,    31,    32,    33,    34,    -1,    -1,    -1,    38,
      39,    40,    41,    -1,    43,    44,    45,    46,    47,    48,
      49,    50,    -1,    52,    53,    54,    -1,    -1,    57,    58,
      59,    60,   257,    62,    -1,    -1,    -1,    -1,    -1,    68,
      -1,   642,    71,    -1,    -1,    -1,    75,    76,   649,    -1,
      -1,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    -1,    91,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,    -1,   123,   124,   125,   126,   127,   128,
     129,    -1,   131,   132,   133,   134,   135,   136,   137,   138,
      -1,   140,   141,   142,   143,   144,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   743,   172,   173,   174,    -1,   176,   177,    -1,
      -1,   180,   181,    -1,    -1,    -1,    -1,   186,    -1,   188
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const IgsYYtype_int16 IgsYYstos[] =
{
       0,     1,   199,   200,     0,   206,     3,    19,    24,    25,
      77,    78,    79,   201,   207,   203,   202,   208,   206,   206,
       1,     4,     5,     6,     7,     8,     9,    10,    11,    13,
      14,    15,    16,    17,    18,    20,    21,    26,    29,    30,
      31,    32,    33,    34,    38,    39,    40,    41,    43,    44,
      45,    46,    47,    49,    50,    52,    53,    54,    57,    58,
      59,    60,    62,    68,    71,    76,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   123,   124,   125,
     126,   127,   128,   129,   131,   132,   133,   134,   135,   136,
     137,   138,   140,   141,   142,   143,   144,   172,   173,   174,
     176,   177,   180,   181,   186,   188,   210,   211,   212,   213,
     214,   215,   216,   217,   218,   219,   220,   221,   222,   223,
     224,   225,   226,   227,   228,   229,   230,   231,   232,   233,
     234,   235,   236,   237,   238,   239,   240,   241,   242,   243,
     244,   245,   246,   247,   248,   249,   252,   256,   257,   258,
     259,   260,   262,   263,   264,   265,   268,   270,   274,   276,
     277,   278,   279,   280,   281,   283,   285,   286,   288,   289,
     290,   291,   292,   293,   294,   295,   296,   297,   300,   301,
     303,   305,   306,   307,   308,   309,   313,   318,   322,   323,
     324,   325,   326,   327,   328,   329,   331,   332,   333,   334,
     335,   336,   337,   338,   339,   340,   341,   342,   343,   344,
     345,   346,   347,   348,   349,   350,   351,   352,   355,   356,
     358,   359,   360,   361,   362,   363,   367,   369,   370,   371,
     372,   373,   374,   375,   376,   377,   378,    74,   204,   204,
       3,     3,    88,    10,   379,   379,     3,     3,    30,     3,
     302,     3,     3,     3,    12,    88,   304,    69,    72,    70,
       3,   269,     3,    28,    35,    36,    37,    88,   379,   368,
     274,   283,   286,     3,     3,     3,     3,   314,   384,    43,
      75,   274,   296,   379,     3,   319,     3,     3,     3,    97,
      97,   330,   274,     3,     3,   216,    61,   274,   283,   284,
     379,     3,   357,   357,     3,     3,     3,   379,   299,   299,
       3,     3,     3,   379,   139,   379,   379,   175,   379,   254,
     317,     3,    69,    72,    48,    75,   209,   216,   379,    90,
     248,   250,   251,   258,    55,   272,   102,   274,   304,   324,
     352,   358,   166,   367,   205,   304,     3,    72,    66,    72,
      69,   267,   196,     3,     3,    72,   366,     3,     3,   189,
       3,   311,     3,    29,    92,    72,   189,   192,   192,   273,
     329,   101,   272,   304,    88,   272,    75,    72,     3,    72,
      72,     3,    72,    42,   298,   196,    72,    72,    42,   178,
     253,   316,   379,   182,    69,   194,   379,   250,    51,   130,
     184,   271,   275,     3,   272,    71,   274,   354,   381,   380,
       3,    49,   266,     3,    23,    72,   145,   146,   147,   148,
     149,   150,   151,   152,   153,   154,   155,   156,   157,   158,
     159,   160,   161,   162,   163,   164,   165,   167,   168,   169,
     170,   171,   196,   197,   365,     3,     3,   192,     3,     3,
      72,    84,   310,     3,   319,     3,     3,     3,     3,    88,
     185,   275,   274,   275,   274,   287,     3,     3,    63,     3,
      67,   254,    72,    73,     3,     3,     3,    72,     3,     3,
       3,    88,   273,   184,   275,     3,   272,    56,   353,     3,
      72,    74,   382,   379,     3,   319,     3,   364,    22,    22,
      22,    22,    22,    22,    22,    22,    22,    22,    22,    22,
      22,    22,    22,    22,   364,   364,   364,    22,    22,    22,
      22,    22,   364,   191,     3,   319,   190,     3,   315,     3,
     192,   190,     3,    72,   273,     3,   272,   272,     3,     3,
       3,     3,   192,   179,     3,   195,     3,   273,   187,   282,
       3,   273,     3,   184,   275,     3,    72,     3,     3,     3,
      22,    72,     3,     3,     3,     3,   383,     3,   312,     3,
      72,     3,     3,    95,     3,   184,   273,   275,   192,     3,
      64,     3,    67,   254,     3,   379,     3,     3,     3,     3,
     319,   197,     3,   319,     3,    72,     3,   196,     3,     3,
       3,     3,     3,     3,    75,    88,     3,     3,    65,   197,
      27,   255,   194,     3,     3,     3,   196,     3,     3,   192,
     193,   191,   191,     3,     3,     3,     3,   189,   196,     3,
       3,   273,   273,   192,    72,    72,   192,   379,    72,     3,
       3,     3,     3,     3,   379,     3,    72,    72,     3,   197,
      72,     3,     3,     3,    72,     3,     3,    72,    72,   304,
      72,     3,     3,   196,   191,   191,     3,     3,     3,   189,
     190,   197,    88,   275,     3,     3,   261,   380,   304,   275,
     275,    72,     3,     3,    72,    72,   191,     3,     3,     3,
      72,   192,    72,   273,   273,   275,     3,     3,    72,    72,
       3,   190,     3,     3,   320,   273,     3,     3,     3,    72,
     192,   196,   321,     3,   197,     3,     3,     3,     3,    72,
     379,    72,     3,   197,    95,   197,   183,    72,     3,     3,
     196,     3,   196,     3,     3,     3,   197,     3,   197,    72,
       3,   192,    72,   320,   321,    72
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const IgsYYtype_int16 IgsYYr1[] =
{
       0,   198,   199,   199,   200,   200,   201,   202,   201,   203,
     201,   205,   204,   204,   206,   206,   207,   207,   207,   207,
     208,   208,   209,   209,   210,   211,   211,   211,   211,   211,
     211,   211,   211,   211,   211,   211,   211,   211,   211,   211,
     211,   211,   211,   211,   211,   211,   211,   211,   211,   211,
     211,   211,   211,   211,   211,   211,   211,   211,   211,   211,
     211,   211,   211,   211,   211,   211,   211,   211,   211,   211,
     211,   211,   211,   211,   211,   211,   211,   211,   211,   211,
     211,   211,   211,   211,   211,   211,   211,   211,   211,   211,
     211,   211,   211,   211,   211,   211,   211,   211,   211,   211,
     211,   211,   211,   211,   211,   211,   211,   211,   211,   211,
     211,   211,   211,   211,   211,   211,   211,   211,   211,   211,
     211,   211,   211,   211,   211,   211,   211,   211,   211,   211,
     211,   211,   211,   211,   211,   211,   211,   211,   211,   211,
     211,   211,   211,   212,   213,   214,   215,   215,   215,   216,
     217,   217,   218,   218,   218,   218,   218,   218,   219,   219,
     220,   221,   221,   222,   223,   224,   224,   225,   225,   226,
     227,   228,   229,   230,   231,   232,   233,   234,   235,   236,
     237,   237,   237,   237,   238,   239,   240,   241,   242,   243,
     244,   245,   246,   247,   248,   249,   249,   250,   251,   251,
     252,   253,   254,   254,   255,   255,   256,   257,   258,   259,
     259,   260,   261,   262,   263,   264,   265,   266,   267,   267,
     269,   268,   270,   271,   272,   272,   273,   273,   274,   274,
     275,   275,   276,   277,   278,   279,   280,   281,   282,   282,
     283,   283,   284,   284,   285,   286,   287,   287,   288,   288,
     289,   289,   290,   290,   291,   291,   292,   293,   293,   294,
     294,   295,   296,   297,   298,   299,   299,   300,   301,   302,
     302,   303,   304,   304,   305,   305,   306,   306,   307,   308,
     309,   310,   310,   311,   311,   312,   312,   314,   315,   313,
     316,   316,   317,   317,   318,   319,   320,   321,   321,   322,
     323,   324,   325,   325,   326,   327,   328,   329,   330,   330,
     331,   332,   333,   334,   335,   336,   336,   337,   337,   338,
     339,   340,   341,   342,   343,   343,   344,   345,   346,   347,
     348,   349,   350,   351,   352,   353,   354,   354,   355,   356,
     357,   358,   358,   358,   358,   359,   359,   360,   360,   361,
     362,   363,   364,   364,   365,   365,   365,   365,   365,   365,
     365,   365,   365,   365,   365,   365,   365,   365,   365,   365,
     365,   365,   365,   365,   365,   365,   365,   365,   365,   365,
     365,   365,   365,   366,   366,   368,   367,   369,   369,   370,
     370,   371,   372,   373,   374,   374,   375,   375,   376,   377,
     378,   379,   379,   380,   380,   381,   381,   382,   382,   383,
     383,   384,   384
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const IgsYYtype_int8 IgsYYr2[] =
{
       0,     2,     1,     1,     5,     0,     1,     0,     4,     0,
       4,     0,     4,     0,     2,     0,     1,     1,     1,     1,
       2,     0,     1,     1,     2,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     0,     3,     1,     2,     2,     1,     2,     2,
       2,     2,     9,     6,     9,    11,    12,    11,     2,     4,
       2,     2,     1,     2,     2,     2,     2,    14,    12,     1,
       3,     2,     2,     3,     1,     2,     1,     1,     3,     2,
       1,     1,     1,     1,    10,     1,     2,     2,     2,     1,
       2,     1,     2,     1,     2,     2,     1,     3,     2,     0,
       2,     4,     2,     0,     1,     0,     7,     7,     4,     2,
       1,     1,     1,    10,     1,     1,     1,    17,     2,     0,
       0,     3,     1,     2,     2,     0,     1,     0,    15,    21,
       2,     0,     1,     2,     1,     1,     1,     1,     1,     0,
       5,    11,     1,     0,     4,     5,     5,     5,     2,     2,
       3,     2,     5,    11,     6,    12,     1,     4,    10,     4,
      10,     2,     2,     1,     6,     2,     0,     2,     2,     2,
       0,     3,     1,     0,     6,     4,     5,    11,     3,     1,
       1,     6,     3,     2,     0,    11,     7,     0,     0,     6,
       2,     2,     2,     0,     2,     4,     3,     4,     0,    23,
       4,     2,     2,     1,     2,     1,     3,     3,     1,     0,
       5,     1,     1,     1,     1,     1,     2,     2,     1,     1,
       1,     3,     1,     2,     1,     3,     1,     1,     1,     1,
       1,     1,     1,     1,     3,     2,     2,     0,     1,     3,
       9,     3,     3,     8,     2,     2,     1,     3,     2,     2,
       2,     1,     2,     0,     1,     1,     1,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     0,     0,     4,     2,     1,     1,
       2,     1,     1,     1,     6,     2,     6,     2,     1,     1,
       2,     2,     0,     3,     0,     2,     0,     1,     1,     1,
       0,     2,     0
};


enum { YYENOMEM = -2 };

#define IgsYYerrok         (IgsYYerrstatus = 0)
#define IgsYYclearin       (IgsYYchar = YYEMPTY)

#define YYACCEPT        goto IgsYYacceptlab
#define YYABORT         goto IgsYYabortlab
#define YYERROR         goto IgsYYerrorlab
#define YYNOMEM         goto IgsYYexhaustedlab


#define YYRECOVERING()  (!!IgsYYerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (IgsYYchar == YYEMPTY)                                        \
      {                                                           \
        IgsYYchar = (Token);                                         \
        IgsYYlval = (Value);                                         \
        YYPOPSTACK (IgsYYlen);                                       \
        IgsYYstate = *IgsYYssp;                                         \
        goto IgsYYbackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        IgsYYerror (YY_("syntax error: cannot back up")); \
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
  if (IgsYYdebug)                                  \
    YYFPRINTF Args;                             \
} while (0)




# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (IgsYYdebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      IgsYY_symbol_print (stderr,                                            \
                  Kind, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
IgsYY_symbol_value_print (FILE *IgsYYo,
                       IgsYYsymbol_kind_t IgsYYkind, YYSTYPE const * const IgsYYvaluep)
{
  FILE *IgsYYoutput = IgsYYo;
  YY_USE (IgsYYoutput);
  if (!IgsYYvaluep)
    return;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (IgsYYkind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
IgsYY_symbol_print (FILE *IgsYYo,
                 IgsYYsymbol_kind_t IgsYYkind, YYSTYPE const * const IgsYYvaluep)
{
  YYFPRINTF (IgsYYo, "%s %s (",
             IgsYYkind < YYNTOKENS ? "token" : "nterm", IgsYYsymbol_name (IgsYYkind));

  IgsYY_symbol_value_print (IgsYYo, IgsYYkind, IgsYYvaluep);
  YYFPRINTF (IgsYYo, ")");
}

/*------------------------------------------------------------------.
| IgsYY_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
IgsYY_stack_print (IgsYY_state_t *IgsYYbottom, IgsYY_state_t *IgsYYtop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; IgsYYbottom <= IgsYYtop; IgsYYbottom++)
    {
      int IgsYYbot = *IgsYYbottom;
      YYFPRINTF (stderr, " %d", IgsYYbot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (IgsYYdebug)                                                  \
    IgsYY_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
IgsYY_reduce_print (IgsYY_state_t *IgsYYssp, YYSTYPE *IgsYYvsp,
                 int IgsYYrule)
{
  int IgsYYlno = IgsYYrline[IgsYYrule];
  int IgsYYnrhs = IgsYYr2[IgsYYrule];
  int IgsYYi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             IgsYYrule - 1, IgsYYlno);
  /* The symbols being reduced.  */
  for (IgsYYi = 0; IgsYYi < IgsYYnrhs; IgsYYi++)
    {
      YYFPRINTF (stderr, "   $%d = ", IgsYYi + 1);
      IgsYY_symbol_print (stderr,
                       YY_ACCESSING_SYMBOL (+IgsYYssp[IgsYYi + 1 - IgsYYnrhs]),
                       &IgsYYvsp[(IgsYYi + 1) - (IgsYYnrhs)]);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (IgsYYdebug)                          \
    IgsYY_reduce_print (IgsYYssp, IgsYYvsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int IgsYYdebug;
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
IgsYYdestruct (const char *IgsYYmsg,
            IgsYYsymbol_kind_t IgsYYkind, YYSTYPE *IgsYYvaluep)
{
  YY_USE (IgsYYvaluep);
  if (!IgsYYmsg)
    IgsYYmsg = "Deleting";
  YY_SYMBOL_PRINT (IgsYYmsg, IgsYYkind, IgsYYvaluep, IgsYYlocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (IgsYYkind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/* Lookahead token kind.  */
int IgsYYchar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE IgsYYlval;
/* Number of syntax errors so far.  */
int IgsYYnerrs;




/*----------.
| IgsYYparse.  |
`----------*/

int
IgsYYparse (void)
{
    IgsYY_state_fast_t IgsYYstate = 0;
    /* Number of tokens to shift before error messages enabled.  */
    int IgsYYerrstatus = 0;

    /* Refer to the stacks through separate pointers, to allow IgsYYoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T IgsYYstacksize = YYINITDEPTH;

    /* The state stack: array, bottom, top.  */
    IgsYY_state_t IgsYYssa[YYINITDEPTH];
    IgsYY_state_t *IgsYYss = IgsYYssa;
    IgsYY_state_t *IgsYYssp = IgsYYss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE IgsYYvsa[YYINITDEPTH];
    YYSTYPE *IgsYYvs = IgsYYvsa;
    YYSTYPE *IgsYYvsp = IgsYYvs;

  int IgsYYn;
  /* The return value of IgsYYparse.  */
  int IgsYYresult;
  /* Lookahead symbol kind.  */
  IgsYYsymbol_kind_t IgsYYtoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE IgsYYval;



#define YYPOPSTACK(N)   (IgsYYvsp -= (N), IgsYYssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int IgsYYlen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  IgsYYchar = YYEMPTY; /* Cause a token to be read.  */

  goto IgsYYsetstate;


/*------------------------------------------------------------.
| IgsYYnewstate -- push a new state, which is found in IgsYYstate.  |
`------------------------------------------------------------*/
IgsYYnewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  IgsYYssp++;


/*--------------------------------------------------------------------.
| IgsYYsetstate -- set current state (the top of the stack) to IgsYYstate.  |
`--------------------------------------------------------------------*/
IgsYYsetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", IgsYYstate));
  YY_ASSERT (0 <= IgsYYstate && IgsYYstate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *IgsYYssp = YY_CAST (IgsYY_state_t, IgsYYstate);
  YY_IGNORE_USELESS_CAST_END
  YY_STACK_PRINT (IgsYYss, IgsYYssp);

  if (IgsYYss + IgsYYstacksize - 1 <= IgsYYssp)
#if !defined IgsYYoverflow && !defined YYSTACK_RELOCATE
    YYNOMEM;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T IgsYYsize = IgsYYssp - IgsYYss + 1;

# if defined IgsYYoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        IgsYY_state_t *IgsYYss1 = IgsYYss;
        YYSTYPE *IgsYYvs1 = IgsYYvs;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if IgsYYoverflow is a macro.  */
        IgsYYoverflow (YY_("memory exhausted"),
                    &IgsYYss1, IgsYYsize * YYSIZEOF (*IgsYYssp),
                    &IgsYYvs1, IgsYYsize * YYSIZEOF (*IgsYYvsp),
                    &IgsYYstacksize);
        IgsYYss = IgsYYss1;
        IgsYYvs = IgsYYvs1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= IgsYYstacksize)
        YYNOMEM;
      IgsYYstacksize *= 2;
      if (YYMAXDEPTH < IgsYYstacksize)
        IgsYYstacksize = YYMAXDEPTH;

      {
        IgsYY_state_t *IgsYYss1 = IgsYYss;
        union IgsYYalloc *IgsYYptr =
          YY_CAST (union IgsYYalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (IgsYYstacksize))));
        if (! IgsYYptr)
          YYNOMEM;
        YYSTACK_RELOCATE (IgsYYss_alloc, IgsYYss);
        YYSTACK_RELOCATE (IgsYYvs_alloc, IgsYYvs);
#  undef YYSTACK_RELOCATE
        if (IgsYYss1 != IgsYYssa)
          YYSTACK_FREE (IgsYYss1);
      }
# endif

      IgsYYssp = IgsYYss + IgsYYsize - 1;
      IgsYYvsp = IgsYYvs + IgsYYsize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, IgsYYstacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (IgsYYss + IgsYYstacksize - 1 <= IgsYYssp)
        YYABORT;
    }
#endif /* !defined IgsYYoverflow && !defined YYSTACK_RELOCATE */


  if (IgsYYstate == YYFINAL)
    YYACCEPT;

  goto IgsYYbackup;


/*-----------.
| IgsYYbackup.  |
`-----------*/
IgsYYbackup:
  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  IgsYYn = IgsYYpact[IgsYYstate];
  if (IgsYYpact_value_is_default (IgsYYn))
    goto IgsYYdefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (IgsYYchar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      IgsYYchar = IgsYYlex ();
    }

  if (IgsYYchar <= YYEOF)
    {
      IgsYYchar = YYEOF;
      IgsYYtoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (IgsYYchar == YYerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      IgsYYchar = YYUNDEF;
      IgsYYtoken = YYSYMBOL_YYerror;
      goto IgsYYerrlab1;
    }
  else
    {
      IgsYYtoken = YYTRANSLATE (IgsYYchar);
      YY_SYMBOL_PRINT ("Next token is", IgsYYtoken, &IgsYYlval, &IgsYYlloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  IgsYYn += IgsYYtoken;
  if (IgsYYn < 0 || YYLAST < IgsYYn || IgsYYcheck[IgsYYn] != IgsYYtoken)
    goto IgsYYdefault;
  IgsYYn = IgsYYtable[IgsYYn];
  if (IgsYYn <= 0)
    {
      if (IgsYYtable_value_is_error (IgsYYn))
        goto IgsYYerrlab;
      IgsYYn = -IgsYYn;
      goto IgsYYreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (IgsYYerrstatus)
    IgsYYerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", IgsYYtoken, &IgsYYlval, &IgsYYlloc);
  IgsYYstate = IgsYYn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++IgsYYvsp = IgsYYlval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  /* Discard the shifted token.  */
  IgsYYchar = YYEMPTY;
  goto IgsYYnewstate;


/*-----------------------------------------------------------.
| IgsYYdefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
IgsYYdefault:
  IgsYYn = IgsYYdefact[IgsYYstate];
  if (IgsYYn == 0)
    goto IgsYYerrlab;
  goto IgsYYreduce;


/*-----------------------------.
| IgsYYreduce -- do a reduction.  |
`-----------------------------*/
IgsYYreduce:
  /* IgsYYn is the number of a rule to reduce with.  */
  IgsYYlen = IgsYYr2[IgsYYn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  IgsYYval = IgsYYvsp[1-IgsYYlen];


  YY_REDUCE_PRINT (IgsYYn);
  switch (IgsYYn)
    {
  case 4: /* session: session loginmessages pass inputs INVALIDPASSWORD  */
#line 143 "gointer.y"
                {
                    printf("DEBUG: yacc parser active - processing session rule\n");
                    fflush(stdout);
#ifndef __STDC__
# ifdef const
#  undef const
# endif /* const */
#endif /* __STDC__ */
                    Output("Invalid password\n");
                    myfree(MyPassword);
                    MyPassword = NULL;
                    myfree(MyName);
                    MyName = NULL;
                    Passed = 0;
                    RegisteredUserSent = 0;
                }
#line 2576 "y.tab.c"
    break;

  case 5: /* session: %empty  */
#line 160 "gointer.y"
                {
                    printf("DEBUG: yacc parser active - empty session rule triggered\n");
                    fflush(stdout);
                    Passed = 0;
                }
#line 2586 "y.tab.c"
    break;

  case 6: /* pass: PASSWORD  */
#line 168 "gointer.y"
                {
                    if (MyPassword) ForceCommand(NULL, MyPassword);
                    else AskString(toplevel, EnterString,
                                   (XtPointer) &MyPassword, "Enter password",
                                   "password", &MyPassword, NULL, NULL);
                }
#line 2597 "y.tab.c"
    break;

  case 7: /* $@1: %empty  */
#line 175 "gointer.y"
                {
                    if (MyPassword) ForceCommand(NULL, MyPassword);
                    else AskString(toplevel, EnterString,
                                   (XtPointer) &MyPassword, "Enter password",
                                   "password", &MyPassword, NULL, NULL);
                }
#line 2608 "y.tab.c"
    break;

  case 9: /* $@2: %empty  */
#line 183 "gointer.y"
                {
                    printf("DEBUG: GUEST token received in yacc parser\n");
                    fflush(stdout);
                    myfree(MyName);
                    MyName = (IgsYYvsp[0].Name);
                }
#line 2619 "y.tab.c"
    break;

  case 10: /* pass: GUEST $@2 loginmessages enterorfail  */
#line 190 "gointer.y"
                {
                    Outputf("This is a guest account. Please see "
                            "'help register' to register.\n"
                            "Your account name is %s\n", (IgsYYvsp[-3].Name));
                }
#line 2629 "y.tab.c"
    break;

  case 11: /* $@3: %empty  */
#line 198 "gointer.y"
                {
                    if (!Passed) {
                        Passed = 1;
                        PlayerPasses(MyName);
                    }
                    ForceCommand(NULL, "toggle client on");
                }
#line 2641 "y.tab.c"
    break;

  case 16: /* loginmessage: NAME  */
#line 214 "gointer.y"
                {
                    Outputf("%s\n", (IgsYYvsp[0].Name));
                    myfree((IgsYYvsp[0].Name));
                }
#line 2650 "y.tab.c"
    break;

  case 17: /* loginmessage: WELCOME  */
#line 219 "gointer.y"
                {
                    Outputf("          Welcome to IGS at %s ", (IgsYYvsp[0].Name));
                    SiteLogon(NULL, (IgsYYvsp[0].Name));
                    myfree((IgsYYvsp[0].Name));
                }
#line 2660 "y.tab.c"
    break;

  case 18: /* loginmessage: LUSER  */
#line 225 "gointer.y"
                {
                    printf("DEBUG: LUSER token received - login prompt detected\n");
                    fflush(stdout);
                    if (MyName) {
                        if (strcmp(MyName, "guest") == 0) {
                            printf("DEBUG: Sending username: %s\n", MyName);
                            fflush(stdout);
                            ForceCommand(NULL, MyName);
                        } else {
                            /* Registered user - check if we already sent username */
                            if (RegisteredUserSent) {
                                printf("DEBUG: Already sent registered username, ignoring duplicate LUSER\n");
                                fflush(stdout);
                            } else {
                                /* First time - send registered username */
                                if (MyPassword) {
                                    printf("DEBUG: Sending registered username: %s\n", MyName);
                                    fflush(stdout);
                                    ForceCommand(NULL, MyName);
                                    RegisteredUserSent = 1;
                                } else {
                                    printf("DEBUG: Registered user %s needs password\n", MyName);
                                    fflush(stdout);
                                    AskString(toplevel, EnterString,
                                              (XtPointer) &MyPassword, "Enter password",
                                              "password", &MyPassword, NULL, NULL);
                                }
                            }
                        }
                    } else {
                        printf("DEBUG: No username set, sending 'guest'\n");
                        fflush(stdout);
                        ForceCommand(NULL, "guest");
                    }
                }
#line 2700 "y.tab.c"
    break;

  case 19: /* loginmessage: SERVERFULL  */
#line 261 "gointer.y"
                {
                    ServerMessage("%s\n", (IgsYYvsp[0].Name));
                    Outputf("%s\n", (IgsYYvsp[0].Name));
                    myfree((IgsYYvsp[0].Name));
                }
#line 2710 "y.tab.c"
    break;

  case 20: /* inputs: inputs moreinput  */
#line 268 "gointer.y"
                                { eEmpty = 0; }
#line 2716 "y.tab.c"
    break;

  case 21: /* inputs: %empty  */
#line 269 "gointer.y"
                                { eEmpty = PreEmpty = 0; }
#line 2722 "y.tab.c"
    break;

  case 22: /* prompt: PROMPT  */
#line 273 "gointer.y"
                {
                    PreEmpty = 0;
                    SeenAdd = 0;
                    (IgsYYval.Value) = (IgsYYvsp[0].Value);
                }
#line 2732 "y.tab.c"
    break;

  case 23: /* prompt: SEMIPROMPT  */
#line 279 "gointer.y"
                {
                    PreEmpty = eEmpty;
                    (IgsYYval.Value) = 0;
                }
#line 2741 "y.tab.c"
    break;

  case 24: /* moreinput: input prompt  */
#line 286 "gointer.y"
                {
                    if (!Passed && (IgsYYvsp[0].Value)) {
                        Passed = 1;
                        PlayerPasses(MyName);
                    }
                    if ((IgsYYvsp[0].Value)) {
                        ChangeCommand(NULL, 1);
                        ResyncCommand(NULL);
                    }
                }
#line 2756 "y.tab.c"
    break;

  case 26: /* input: servermessages  */
#line 299 "gointer.y"
                                  { ChangeCommand(NULL, -1); }
#line 2762 "y.tab.c"
    break;

  case 27: /* input: xshout  */
#line 300 "gointer.y"
                                  { ChangeCommand(NULL, -1); }
#line 2768 "y.tab.c"
    break;

  case 28: /* input: infomessage  */
#line 301 "gointer.y"
                                  { ChangeCommand(NULL, -1); }
#line 2774 "y.tab.c"
    break;

  case 29: /* input: beeping  */
#line 302 "gointer.y"
                                  { ChangeCommand(NULL, -1); }
#line 2780 "y.tab.c"
    break;

  case 32: /* input: tell  */
#line 305 "gointer.y"
                                  { ChangeCommand(NULL, -1); }
#line 2786 "y.tab.c"
    break;

  case 33: /* input: broadcast  */
#line 306 "gointer.y"
                                  { ChangeCommand(NULL, -1); }
#line 2792 "y.tab.c"
    break;

  case 34: /* input: kibitz  */
#line 307 "gointer.y"
                                  { ChangeCommand(NULL, -1); }
#line 2798 "y.tab.c"
    break;

  case 36: /* input: yell  */
#line 309 "gointer.y"
                                  { ChangeCommand(NULL, -1); }
#line 2804 "y.tab.c"
    break;

  case 37: /* input: join  */
#line 310 "gointer.y"
                                  { ChangeCommand(NULL, -1); }
#line 2810 "y.tab.c"
    break;

  case 38: /* input: leave  */
#line 311 "gointer.y"
                                  { ChangeCommand(NULL, -1); }
#line 2816 "y.tab.c"
    break;

  case 39: /* input: newtitle  */
#line 312 "gointer.y"
                                  { ChangeCommand(NULL, -1); }
#line 2822 "y.tab.c"
    break;

  case 46: /* input: matchrequest  */
#line 319 "gointer.y"
                                  { ChangeCommand(NULL, -1); }
#line 2828 "y.tab.c"
    break;

  case 50: /* input: freemessage  */
#line 323 "gointer.y"
                                  { ChangeCommand(NULL, -1); }
#line 2834 "y.tab.c"
    break;

  case 71: /* input: mustpass  */
#line 344 "gointer.y"
                                  { ChangeCommand(NULL, -1); }
#line 2840 "y.tab.c"
    break;

  case 73: /* input: opponentdisagreeremove  */
#line 346 "gointer.y"
                                     { ChangeCommand(NULL, -1); }
#line 2846 "y.tab.c"
    break;

  case 75: /* input: doneopponentobserve  */
#line 348 "gointer.y"
                                  { ChangeCommand(NULL, -1); }
#line 2852 "y.tab.c"
    break;

  case 76: /* input: opponentobserve  */
#line 349 "gointer.y"
                                  { ChangeCommand(NULL, -1); }
#line 2858 "y.tab.c"
    break;

  case 77: /* input: betresult  */
#line 350 "gointer.y"
                                  { ChangeCommand(NULL, -1); }
#line 2864 "y.tab.c"
    break;

  case 79: /* input: opponentrestart  */
#line 352 "gointer.y"
                                  { ChangeCommand(NULL, -1); }
#line 2870 "y.tab.c"
    break;

  case 81: /* input: newmatch1  */
#line 354 "gointer.y"
                                  { ChangeCommand(NULL, -1); }
#line 2876 "y.tab.c"
    break;

  case 84: /* input: opponentdispute  */
#line 357 "gointer.y"
                                  { ChangeCommand(NULL, -1); }
#line 2882 "y.tab.c"
    break;

  case 88: /* input: opponentundid  */
#line 361 "gointer.y"
                                  { ChangeCommand(NULL, -1); }
#line 2888 "y.tab.c"
    break;

  case 89: /* input: undo  */
#line 362 "gointer.y"
                                  { ChangeCommand(NULL, -1); }
#line 2894 "y.tab.c"
    break;

  case 100: /* input: lostconnection  */
#line 373 "gointer.y"
                                  { ChangeCommand(NULL, -1); }
#line 2900 "y.tab.c"
    break;

  case 103: /* input: adjournsentrequest  */
#line 376 "gointer.y"
                                  { ChangeCommand(NULL, -1); }
#line 2906 "y.tab.c"
    break;

  case 104: /* input: adjournrequest  */
#line 377 "gointer.y"
                                  { ChangeCommand(NULL, -1); }
#line 2912 "y.tab.c"
    break;

  case 105: /* input: oppadjourn  */
#line 378 "gointer.y"
                                  { ChangeCommand(NULL, -1); }
#line 2918 "y.tab.c"
    break;

  case 106: /* input: declineadjourn  */
#line 379 "gointer.y"
                                  { ChangeCommand(NULL, -1); }
#line 2924 "y.tab.c"
    break;

  case 107: /* input: resign  */
#line 380 "gointer.y"
                                  { ChangeCommand(NULL, -1); }
#line 2930 "y.tab.c"
    break;

  case 108: /* input: playeron  */
#line 381 "gointer.y"
                                  { ChangeCommand(NULL, -1); }
#line 2936 "y.tab.c"
    break;

  case 109: /* input: removegamefile  */
#line 382 "gointer.y"
                                  { ChangeCommand(NULL, -1); }
#line 2942 "y.tab.c"
    break;

  case 126: /* input: uptime  */
#line 400 "gointer.y"
                {
                    if (!Entered) {
                        Entering();
                    }
                }
#line 2952 "y.tab.c"
    break;

  case 137: /* input: EMPTY  */
#line 415 "gointer.y"
                                  { ChangeCommand(NULL, -1); }
#line 2958 "y.tab.c"
    break;

  case 141: /* input: error  */
#line 420 "gointer.y"
                {
                    /* 1 in case next token is SEMIPROMPT */
                    SetCommand(NULL, 1);
                }
#line 2967 "y.tab.c"
    break;

  case 142: /* input: %empty  */
#line 424 "gointer.y"
                                  { eEmpty = 1; }
#line 2973 "y.tab.c"
    break;

  case 143: /* textfile: TEXTFILE names END  */
#line 428 "gointer.y"
                {
                    NameList   *Names;
                    const char *User;

                    switch((IgsYYvsp[-2].Value)) {
                      case 25: /* Results */
                        if (UserCommandP(NULL)) goto user;
                        User = StripFirstArgCommand(NULL, "results");
                        if (!User) goto user;
			if (*User == '-') User++;
                        /* AddResults keeps $2 */
                        AddResults(User, (IgsYYvsp[-1].Namelist));
                        break;
                      default:
                      user:
                        for (Names = (IgsYYvsp[-1].Namelist)->Next;Names != (IgsYYvsp[-1].Namelist); Names = Names->Next)
                            Outputf("%s\n", Names->Name);
                        FreeNameList((IgsYYvsp[-1].Namelist));
                    }
                }
#line 2998 "y.tab.c"
    break;

  case 144: /* erase: ERASE  */
#line 451 "gointer.y"
                {
                    Output("Please erase your messages (see help erase)\n");
                }
#line 3006 "y.tab.c"
    break;

  case 145: /* igsentry: IGSENTRY NAME  */
#line 457 "gointer.y"
                {
                    char *ptr;

                    Outputf("Logging into %s %s\n", (IgsYYvsp[-1].Dummy), (IgsYYvsp[0].Name));
                    ptr = mystrdup((IgsYYvsp[-1].Dummy));
                    myfree(ServerName);
                    ServerName = ptr;
                    switch(ptr[0]) {
                      case 'N': ServerType = NNGS; break;
                      default:  ServerType = IGS;  break;
                    }
                    myfree((IgsYYvsp[-1].Dummy));
                    myfree((IgsYYvsp[0].Name));
                }
#line 3025 "y.tab.c"
    break;

  case 148: /* servermessages: EMPTY servermessage  */
#line 475 "gointer.y"
                                  {}
#line 3031 "y.tab.c"
    break;

  case 149: /* servermessage: SERVERMESSAGE NAME  */
#line 479 "gointer.y"
                {
                    ServerMessage("%s\n", (IgsYYvsp[0].Name));
                    myfree((IgsYYvsp[0].Name));
                }
#line 3040 "y.tab.c"
    break;

  case 150: /* xshout: XSHOUT NAME  */
#line 486 "gointer.y"
                {
                    ServerMessage("%s: %s\n", PlayerString((IgsYYvsp[-1].Person)), (IgsYYvsp[0].Name));
                    myfree((IgsYYvsp[0].Name));
                }
#line 3049 "y.tab.c"
    break;

  case 151: /* xshout: XSHOUT2 NAME  */
#line 491 "gointer.y"
                {
		    /* dummy player name such as "*8^)*" */
                    ServerMessage("%s: %s\n", (IgsYYvsp[-1].Name), (IgsYYvsp[0].Name));
                    myfree((IgsYYvsp[-1].Name));
                    myfree((IgsYYvsp[0].Name));
                }
#line 3060 "y.tab.c"
    break;

  case 152: /* infomessage: INFOMESSAGE NAME '[' NAME ']' NAME NAME '}' END  */
#line 500 "gointer.y"
                {
                    /* A Connect */
                    if (strcmp((IgsYYvsp[-3].Name), "has") || strcmp((IgsYYvsp[-2].Name), "connected."))
                        YYERROR;
                    PlayerConnect((IgsYYvsp[-7].Name), (IgsYYvsp[-5].Name));
                    myfree((IgsYYvsp[-7].Name));
                    myfree((IgsYYvsp[-5].Name));
                    myfree((IgsYYvsp[-3].Name));
                    myfree((IgsYYvsp[-2].Name));
                }
#line 3075 "y.tab.c"
    break;

  case 153: /* infomessage: INFOMESSAGE NAME NAME NAME '}' END  */
#line 511 "gointer.y"
                {
                    /* A disconnect */
                    if (strcmp((IgsYYvsp[-3].Name), "has") || strcmp((IgsYYvsp[-2].Name), "disconnected"))
                        YYERROR;
                    PlayerDisconnect((IgsYYvsp[-4].Name));
                    myfree((IgsYYvsp[-4].Name));
                    myfree((IgsYYvsp[-3].Name));
                    myfree((IgsYYvsp[-2].Name));
                }
#line 3089 "y.tab.c"
    break;

  case 154: /* infomessage: INFOMESSAGE NAME NAME ':' player NAME player '}' END  */
#line 521 "gointer.y"
                {
                    /* A new match, format with game number */
                    if (strcmp((IgsYYvsp[-3].Name), "vs.") || strcmp((IgsYYvsp[-7].Name), "Match")) YYERROR;
                    NewMatch(atoi((IgsYYvsp[-6].Name)), (IgsYYvsp[-4].Person), (IgsYYvsp[-2].Person));
                    myfree((IgsYYvsp[-7].Name));
                    myfree((IgsYYvsp[-6].Name));
                    myfree((IgsYYvsp[-3].Name));
                }
#line 3102 "y.tab.c"
    break;

  case 155: /* infomessage: INFOMESSAGE NAME NAME ':' NAME NAME NAME ':' names '}' END  */
#line 530 "gointer.y"
                {
                    if (strcmp((IgsYYvsp[-5].Name), "vs") || strcmp((IgsYYvsp[-9].Name), "Game")) YYERROR;
                    GameInfo(atoi((IgsYYvsp[-8].Name)), (IgsYYvsp[-4].Name), (IgsYYvsp[-6].Name), (IgsYYvsp[-2].Namelist));
                    myfree((IgsYYvsp[-9].Name));
                    myfree((IgsYYvsp[-8].Name));
                    myfree((IgsYYvsp[-6].Name));
                    myfree((IgsYYvsp[-5].Name));
                    myfree((IgsYYvsp[-4].Name));
                    FreeNameList((IgsYYvsp[-2].Namelist));
                }
#line 3117 "y.tab.c"
    break;

  case 156: /* infomessage: INFOMESSAGE NAME NAME ':' NAME NAME NAME '@' NAME NAME '}' END  */
#line 541 "gointer.y"
                {
                    /* Resume */
                    if (strcmp((IgsYYvsp[-10].Name), "Game") || strcmp((IgsYYvsp[-6].Name), "vs") ||
                        strcmp((IgsYYvsp[-3].Name), "Move")) YYERROR;
                    Resume(atoi((IgsYYvsp[-9].Name)), (IgsYYvsp[-5].Name), (IgsYYvsp[-7].Name), atoi((IgsYYvsp[-2].Name)));
                    myfree((IgsYYvsp[-10].Name));
                    myfree((IgsYYvsp[-9].Name));
                    myfree((IgsYYvsp[-7].Name));
                    myfree((IgsYYvsp[-6].Name));
                    myfree((IgsYYvsp[-5].Name));
                    myfree((IgsYYvsp[-3].Name));
                    myfree((IgsYYvsp[-2].Name));
                }
#line 3135 "y.tab.c"
    break;

  case 157: /* infomessage: INFOMESSAGE NAME NAME ':' NAME NAME NAME NAME NAME '}' END  */
#line 555 "gointer.y"
                {
                    /* Adjourn */
                    if (strcmp((IgsYYvsp[-3].Name), "has") || strcmp((IgsYYvsp[-2].Name), "adjourned.") ||
                        strcmp((IgsYYvsp[-5].Name), "vs")) YYERROR;
                    Adjourn(atoi((IgsYYvsp[-8].Name)), (IgsYYvsp[-4].Name), (IgsYYvsp[-6].Name));
                    myfree((IgsYYvsp[-9].Name));
                    myfree((IgsYYvsp[-8].Name));
                    myfree((IgsYYvsp[-6].Name));
                    myfree((IgsYYvsp[-5].Name));
                    myfree((IgsYYvsp[-4].Name));
                    myfree((IgsYYvsp[-3].Name));
                    myfree((IgsYYvsp[-2].Name));
                }
#line 3153 "y.tab.c"
    break;

  case 158: /* tell: TELL NAME  */
#line 571 "gointer.y"
                {
                    ReceivedTell((IgsYYvsp[-1].Person), (IgsYYvsp[0].Name));
                    myfree((IgsYYvsp[0].Name));
                }
#line 3162 "y.tab.c"
    break;

  case 159: /* tell: OBSERVE SEMIPROMPT TELL NAME  */
#line 576 "gointer.y"
                {
                    ReceivedTell((IgsYYvsp[-1].Person), (IgsYYvsp[0].Name));
                    myfree((IgsYYvsp[0].Name));
                }
#line 3171 "y.tab.c"
    break;

  case 160: /* playeron: PLAYERON optobserve  */
#line 583 "gointer.y"
                {
                    ReceivedTell((IgsYYvsp[-1].Person), "is now on.");
                }
#line 3179 "y.tab.c"
    break;

  case 161: /* beeping: OBSERVE BEEPING  */
#line 590 "gointer.y"
                {
                    Beeping((IgsYYvsp[0].Person));
                }
#line 3187 "y.tab.c"
    break;

  case 162: /* beeping: BEEPING  */
#line 594 "gointer.y"
                {
                    Beeping((IgsYYvsp[0].Person));
                }
#line 3195 "y.tab.c"
    break;

  case 163: /* idle: IDLE NAME  */
#line 600 "gointer.y"
                {
		    Idle((IgsYYvsp[-1].Person), (IgsYYvsp[0].Name));
                    myfree((IgsYYvsp[0].Name));
                }
#line 3204 "y.tab.c"
    break;

  case 164: /* stored: STORED STOREDNUM  */
#line 607 "gointer.y"
                {
		    StoredNum((IgsYYvsp[-1].Person), (IgsYYvsp[0].Value));
                }
#line 3212 "y.tab.c"
    break;

  case 165: /* broadcast: BROADCAST NAME  */
#line 613 "gointer.y"
                {
                    ShowBroadcast((IgsYYvsp[-1].Person), ":", (IgsYYvsp[0].Name));
                    myfree((IgsYYvsp[0].Name));
                }
#line 3221 "y.tab.c"
    break;

  case 166: /* broadcast: ITBROADCAST NAME  */
#line 618 "gointer.y"
                {
                    ShowBroadcast((IgsYYvsp[-1].Person), "", (IgsYYvsp[0].Name));
                    myfree((IgsYYvsp[0].Name));
                }
#line 3230 "y.tab.c"
    break;

  case 167: /* kibitz: OBSERVE SEMIPROMPT KIBITZ player ':' NAME NAME NAME NAME '[' NAME ']' END NAME  */
#line 626 "gointer.y"
                {
                    if (strcmp((IgsYYvsp[-8].Name), "Game") || strcmp((IgsYYvsp[-6].Name), "vs")) YYERROR;
                    ReceivedKibitz((IgsYYvsp[-10].Person), atoi((IgsYYvsp[-3].Name)), (IgsYYvsp[-5].Name), (IgsYYvsp[-7].Name), (IgsYYvsp[0].Name), strlen((IgsYYvsp[0].Name)));
                    myfree((IgsYYvsp[-8].Name));
                    myfree((IgsYYvsp[-7].Name));
                    myfree((IgsYYvsp[-6].Name));
                    myfree((IgsYYvsp[-5].Name));
                    myfree((IgsYYvsp[-3].Name));
                    myfree((IgsYYvsp[0].Name));
                }
#line 3245 "y.tab.c"
    break;

  case 168: /* kibitz: KIBITZ player ':' NAME NAME NAME NAME '[' NAME ']' END NAME  */
#line 638 "gointer.y"
                  {
                    if (strcmp((IgsYYvsp[-8].Name), "Game") || strcmp((IgsYYvsp[-6].Name), "vs")) YYERROR;
                    ReceivedKibitz((IgsYYvsp[-10].Person), atoi((IgsYYvsp[-3].Name)), (IgsYYvsp[-5].Name), (IgsYYvsp[-7].Name), (IgsYYvsp[0].Name), strlen((IgsYYvsp[0].Name)));
                    myfree((IgsYYvsp[-8].Name));
                    myfree((IgsYYvsp[-7].Name));
                    myfree((IgsYYvsp[-6].Name));
                    myfree((IgsYYvsp[-5].Name));
                    myfree((IgsYYvsp[-3].Name));
                    myfree((IgsYYvsp[0].Name));
                  }
#line 3260 "y.tab.c"
    break;

  case 169: /* messages: MESSAGES  */
#line 651 "gointer.y"
                {
                    Outputf("You have %d line%s of messages\n",
                            (IgsYYvsp[0].Value), (IgsYYvsp[0].Value)==1 ? "" : "s");
                }
#line 3269 "y.tab.c"
    break;

  case 170: /* yell: CHANNEL YELL NAME  */
#line 657 "gointer.y"
                {
                    ShowYell((IgsYYvsp[-2].Value), (IgsYYvsp[-1].Person), (IgsYYvsp[0].Name));
                    myfree((IgsYYvsp[0].Name));
                }
#line 3278 "y.tab.c"
    break;

  case 171: /* join: CHANNEL JOIN  */
#line 664 "gointer.y"
                {
                    ChannelJoin((IgsYYvsp[-1].Value), (IgsYYvsp[0].Person));
                }
#line 3286 "y.tab.c"
    break;

  case 172: /* leave: CHANNEL LEAVE  */
#line 670 "gointer.y"
                {
                    ChannelLeave((IgsYYvsp[-1].Value), (IgsYYvsp[0].Person));
                }
#line 3294 "y.tab.c"
    break;

  case 173: /* newtitle: CHANNEL NEWTITLE NAME  */
#line 676 "gointer.y"
                {
                    ChannelTitle((IgsYYvsp[-2].Value), (IgsYYvsp[-1].Person), (IgsYYvsp[0].Name));
                    myfree((IgsYYvsp[0].Name));
                }
#line 3303 "y.tab.c"
    break;

  case 174: /* changechannel: CHANGECHANNEL  */
#line 683 "gointer.y"
                {
                    JoinChannel((IgsYYvsp[0].Value));
                }
#line 3311 "y.tab.c"
    break;

  case 175: /* wrongchannel: WRONGCHANNEL NAME  */
#line 689 "gointer.y"
                {
                    WrongChannel((IgsYYvsp[0].Name));
                    myfree((IgsYYvsp[0].Name));
                }
#line 3320 "y.tab.c"
    break;

  case 176: /* matchopen: MATCHOPEN  */
#line 696 "gointer.y"
                {
                    Output("Setting you open for matches\n");
                }
#line 3328 "y.tab.c"
    break;

  case 177: /* matchclosed: MATCHCLOSED  */
#line 702 "gointer.y"
                {
                    Output("You are not open for matches\n");
                }
#line 3336 "y.tab.c"
    break;

  case 178: /* automatchrequest: AUTOMATCHREQUEST names END  */
#line 708 "gointer.y"
                {
                    AutoMatchRequest((IgsYYvsp[-1].Namelist));
                    FreeNameList((IgsYYvsp[-1].Namelist));
                }
#line 3345 "y.tab.c"
    break;

  case 179: /* automatchdispute: AUTOMATCHDISPUTE names  */
#line 715 "gointer.y"
                {
                    AutoMatchDispute((IgsYYvsp[-1].Name), (IgsYYvsp[0].Namelist));
                    myfree((IgsYYvsp[-1].Name));
                    FreeNameList((IgsYYvsp[0].Namelist));
                }
#line 3355 "y.tab.c"
    break;

  case 180: /* ruledmatchrequest: MATCHREQUEST  */
#line 723 "gointer.y"
                                        { (IgsYYval.Value) = 'I'; }
#line 3361 "y.tab.c"
    break;

  case 181: /* ruledmatchrequest: GOEMATCHREQUEST  */
#line 724 "gointer.y"
                                        { (IgsYYval.Value) = 'G'; }
#line 3367 "y.tab.c"
    break;

  case 182: /* ruledmatchrequest: TOURNAMENTMATCHREQUEST  */
#line 725 "gointer.y"
                                        { (IgsYYval.Value) = 'i'; }
#line 3373 "y.tab.c"
    break;

  case 183: /* ruledmatchrequest: TOURNAMENTGOEMATCHREQUEST  */
#line 726 "gointer.y"
                                        { (IgsYYval.Value) = 'g'; }
#line 3379 "y.tab.c"
    break;

  case 184: /* matchrequest: ruledmatchrequest names '>' NAME '<' names '>' names END optobserve  */
#line 731 "gointer.y"
                {
                    if (strcmp((IgsYYvsp[-6].Name), "or")) YYERROR;
                    MatchRequest((IgsYYvsp[-9].Value), (IgsYYvsp[-8].Namelist));
                    FreeNameList((IgsYYvsp[-8].Namelist));
                    myfree((IgsYYvsp[-6].Name));
                    FreeNameList((IgsYYvsp[-4].Namelist));
                    FreeNameList((IgsYYvsp[-2].Namelist));
                }
#line 3392 "y.tab.c"
    break;

  case 185: /* requestingmatch: REQUESTINGMATCH  */
#line 742 "gointer.y"
                {
                    /* Outputf("%s\n", $1); */
                    myfree((IgsYYvsp[0].Name));
                }
#line 3401 "y.tab.c"
    break;

  case 186: /* komirequest: KOMIREQUEST NAME  */
#line 749 "gointer.y"
                {
                    char *Ptr;

                    Ptr = strchr((IgsYYvsp[0].Name), 0)-1;
                    if (*Ptr == '.') *Ptr = 0;
                    MyGameMessage("%s wants the komi to be %s",
	                          PlayerString((IgsYYvsp[-1].Person)), (IgsYYvsp[0].Name));
                    if (WhatCommand(NULL, "komi") < 0) ChangeCommand(NULL, -1);
                    myfree((IgsYYvsp[0].Name));
                }
#line 3416 "y.tab.c"
    break;

  case 187: /* komiset: KOMISET NAME  */
#line 762 "gointer.y"
                {
                    char *Ptr;

                    Ptr = strchr((IgsYYvsp[0].Name), 0)-1;
                    if (*Ptr == '.') *Ptr = 0;
                    MyGameMessage("The komi has been set to %s", (IgsYYvsp[0].Name));
                    CheckMyKomi((IgsYYvsp[0].Name));
                    if (WhatCommand(NULL, "komi") < 0) ChangeCommand(NULL, -1);
                    myfree((IgsYYvsp[0].Name));
                }
#line 3431 "y.tab.c"
    break;

  case 188: /* freemessage: FREE OBSERVE  */
#line 775 "gointer.y"
                {
                    Outputf("Game will %scount towards ratings\n",
                            (IgsYYvsp[-1].Value) ? "not" : "");
                }
#line 3440 "y.tab.c"
    break;

  case 189: /* freeconfirm: FREE  */
#line 782 "gointer.y"
                {
                    Outputf("Game will %scount towards ratings\n",
                            (IgsYYvsp[0].Value) ? "not " : "");
                }
#line 3449 "y.tab.c"
    break;

  case 190: /* latefree: LATEFREE NAME  */
#line 789 "gointer.y"
                {
                    Outputf("You cannot change into a free game after %s\n",
                            (IgsYYvsp[0].Name));
                    myfree((IgsYYvsp[0].Name));
                }
#line 3459 "y.tab.c"
    break;

  case 191: /* noplay: NOPLAY  */
#line 797 "gointer.y"
                {
                    Output("You are not playing a game\n");
                }
#line 3467 "y.tab.c"
    break;

  case 192: /* noload: OPPONENTNOTON NOLOAD  */
#line 803 "gointer.y"
                {
                    Output("Your opponent is not on currently. "
                           "Game failed to load\n");
                }
#line 3476 "y.tab.c"
    break;

  case 193: /* titleset: TITLESET  */
#line 809 "gointer.y"
                {
                    const char *title;

                    title = ArgsCommand(NULL, "title");
                    if (title) SetMyGameTitle(title);
                    else Warning("Title set, but I "
                                 "can't remember to what....\n");
                }
#line 3489 "y.tab.c"
    break;

  case 194: /* statsentry: STATSENTRY NAME  */
#line 820 "gointer.y"
                {
                    NameVal *nameval;

                    (IgsYYval.Nameval) = nameval = mynew(NameVal);
                    nameval->Next  = nameval->Previous = nameval;
                    nameval->Name  = (IgsYYvsp[-1].Name);
                    nameval->Value = (IgsYYvsp[0].Name);
                }
#line 3502 "y.tab.c"
    break;

  case 195: /* statsentries: statsentries statsentry  */
#line 831 "gointer.y"
                {
                    (IgsYYval.Nameval) = (IgsYYvsp[-1].Nameval);
                    (IgsYYvsp[0].Nameval)->Previous = (IgsYYvsp[-1].Nameval)->Previous;
                    (IgsYYvsp[0].Nameval)->Next     = (IgsYYvsp[-1].Nameval);
                    (IgsYYvsp[0].Nameval)->Previous->Next = (IgsYYvsp[0].Nameval)->Next->Previous = (IgsYYvsp[0].Nameval);
                }
#line 3513 "y.tab.c"
    break;

  case 196: /* statsentries: statsentry  */
#line 837 "gointer.y"
                         { (IgsYYval.Nameval) = (IgsYYvsp[0].Nameval); }
#line 3519 "y.tab.c"
    break;

  case 197: /* extendstatsentry: EXTSTATSENTRY names END  */
#line 840 "gointer.y"
                                          { (IgsYYval.Namelist) = (IgsYYvsp[-1].Namelist); }
#line 3525 "y.tab.c"
    break;

  case 198: /* optextend: extendstatsentry extendstatsentry  */
#line 844 "gointer.y"
                {
                    NameVal *nameval;
                    NameList *Pos1, *Pos2;

                    nameval = mynew(NameVal);
                    nameval->Previous = nameval->Next = nameval;
                    nameval->Name = nameval->Value = NULL;
                    (IgsYYval.Nameval) = nameval;

                    for (Pos1=(IgsYYvsp[-1].Namelist)->Next, Pos2=(IgsYYvsp[0].Namelist)->Next;
                         Pos1 != (IgsYYvsp[-1].Namelist) && Pos2 != (IgsYYvsp[0].Namelist);
                         Pos1 = Pos1->Next, Pos2 = Pos2->Next) {
                        nameval = mynew(NameVal);
                        nameval->Name  = Pos1->Name; Pos1->Name = NULL;
                        nameval->Value = Pos2->Name; Pos2->Name = NULL;
                        nameval->Next = (IgsYYval.Nameval);
                        nameval->Previous = (IgsYYval.Nameval)->Previous;
                        nameval->Previous->Next =
                            nameval->Next->Previous = nameval;
                    }
                    if (Pos1 != (IgsYYvsp[-1].Namelist) || Pos2 != (IgsYYvsp[0].Namelist))
                        Warning("Name value lists have different length\n");
                    FreeNameList(Pos1);
                    FreeNameList(Pos2);
                }
#line 3555 "y.tab.c"
    break;

  case 199: /* optextend: %empty  */
#line 870 "gointer.y"
                {
                    NameVal *nameval;

                    nameval = mynew(NameVal);
                    nameval->Previous = nameval->Next = nameval;
                    nameval->Name = nameval->Value = NULL;
                    (IgsYYval.Nameval) = nameval;
                }
#line 3568 "y.tab.c"
    break;

  case 200: /* stats: statsentries optextend  */
#line 881 "gointer.y"
                {
                    NameVal *ext;

                    (IgsYYvsp[0].Nameval)->Next->Previous = (IgsYYvsp[-1].Nameval)->Previous;
                    ext = (IgsYYvsp[-1].Nameval)->Previous->Next = (IgsYYvsp[0].Nameval)->Next;
                    (IgsYYvsp[0].Nameval)->Next = (IgsYYvsp[-1].Nameval);
                    (IgsYYvsp[-1].Nameval)->Previous = (IgsYYvsp[0].Nameval);
                    ShowStats((IgsYYvsp[0].Nameval), ext);
                    FreeNameValList((IgsYYvsp[0].Nameval));
                }
#line 3583 "y.tab.c"
    break;

  case 201: /* betentry: PERSON NATURAL ':' NATURAL  */
#line 894 "gointer.y"
                {
                    (IgsYYval.Bet) = mynew(BetDesc);
                    (IgsYYval.Bet)->Who  = (IgsYYvsp[-3].Person);
                    (IgsYYval.Bet)->Wins = (IgsYYvsp[-2].Value);
                    (IgsYYval.Bet)->Bets = (IgsYYvsp[0].Value);
                }
#line 3594 "y.tab.c"
    break;

  case 202: /* betentries: betentries betentry  */
#line 903 "gointer.y"
                {
                    (IgsYYvsp[0].Bet)->Next = (IgsYYvsp[-1].Bet);
                    (IgsYYval.Bet) = (IgsYYvsp[0].Bet);
                }
#line 3603 "y.tab.c"
    break;

  case 203: /* betentries: %empty  */
#line 908 "gointer.y"
                {
                    (IgsYYval.Bet) = NULL;
                }
#line 3611 "y.tab.c"
    break;

  case 204: /* optmybet: MYBET  */
#line 913 "gointer.y"
                    { (IgsYYval.Name) = (IgsYYvsp[0].Name);   }
#line 3617 "y.tab.c"
    break;

  case 205: /* optmybet: %empty  */
#line 914 "gointer.y"
                    { (IgsYYval.Name) = NULL; }
#line 3623 "y.tab.c"
    break;

  case 206: /* bet: BETWINNERS betentries BETEVEN betentries BETLOSERS betentries optmybet  */
#line 919 "gointer.y"
                {
                    BetDesc *Here, *Next;

                    BetResults((IgsYYvsp[-5].Bet), (IgsYYvsp[-3].Bet), (IgsYYvsp[-1].Bet), (IgsYYvsp[0].Name));
                    for (Here = (IgsYYvsp[-5].Bet); Here; Here = Next) {
                        Next = Here->Next;
                        myfree(Here);
                    }
                    for (Here = (IgsYYvsp[-3].Bet); Here; Here = Next) {
                        Next = Here->Next;
                        myfree(Here);
                    }
                    for (Here = (IgsYYvsp[-1].Bet); Here; Here = Next) {
                        Next = Here->Next;
                        myfree(Here);
                    }
                }
#line 3645 "y.tab.c"
    break;

  case 207: /* toggle: TOGGLE NAME NAME NAME NAME optname END  */
#line 939 "gointer.y"
                {
                    /* -Ton remove the optname */
                    /* eg: Set | verbose to be True. */
                    SetStat((IgsYYvsp[-5].Name), strcmp((IgsYYvsp[-2].Name)+1, "alse."));
                    myfree((IgsYYvsp[-5].Name));
                    myfree((IgsYYvsp[-4].Name));
                    myfree((IgsYYvsp[-3].Name));
                    myfree((IgsYYvsp[-2].Name));
                }
#line 3659 "y.tab.c"
    break;

  case 208: /* channelentry: NEWCHANNEL names END namesset  */
#line 951 "gointer.y"
                {
                    (IgsYYvsp[-2].Namelist)->Name = (char *) (IgsYYvsp[0].Namelist);
                    (IgsYYvsp[0].Namelist)->Name = (IgsYYvsp[-3].Name);
                    (IgsYYval.Namelist) = (IgsYYvsp[-2].Namelist);
                }
#line 3669 "y.tab.c"
    break;

  case 209: /* channelentries: channelentries channelentry  */
#line 959 "gointer.y"
                {
                    NameList *Names;

                    (IgsYYval.Channeldata) = (IgsYYvsp[-1].Channeldata);
                    Names = (NameList *) (IgsYYvsp[0].Namelist)->Name;
                    (IgsYYvsp[0].Namelist)->Name = NULL;
                    AddChannelData((IgsYYval.Channeldata), Names->Name, (IgsYYvsp[0].Namelist)->Next->Name,
                                   (IgsYYvsp[0].Namelist)->Next->Next->Name,
                                   (IgsYYvsp[0].Namelist)->Next->Next->Next->Name, Names);
                    Names->Name = NULL;
                    FreeNameList((IgsYYvsp[0].Namelist));
                }
#line 3686 "y.tab.c"
    break;

  case 210: /* channelentries: channelentry  */
#line 972 "gointer.y"
                {
                    NameList *Names;

                    (IgsYYval.Channeldata) = OpenChannelData();
                    Names = (NameList *) (IgsYYvsp[0].Namelist)->Name;
                    (IgsYYvsp[0].Namelist)->Name = NULL;
                    AddChannelData((IgsYYval.Channeldata), Names->Name, (IgsYYvsp[0].Namelist)->Next->Name,
                                   (IgsYYvsp[0].Namelist)->Next->Next->Name,
                                   (IgsYYvsp[0].Namelist)->Next->Next->Next->Name, Names);
                    Names->Name = NULL;
                    FreeNameList((IgsYYvsp[0].Namelist));
                }
#line 3703 "y.tab.c"
    break;

  case 211: /* channels: channelentries  */
#line 987 "gointer.y"
                {
                    ChannelList((IgsYYvsp[0].Channeldata));
                    CloseChannelData((IgsYYvsp[0].Channeldata));
                }
#line 3712 "y.tab.c"
    break;

  case 212: /* observerentries: namesset  */
#line 993 "gointer.y"
                          { (IgsYYval.Namelist) = (IgsYYvsp[0].Namelist); }
#line 3718 "y.tab.c"
    break;

  case 213: /* observers: OBSERVERS NAME '(' NAME NAME NAME ')' ':' END observerentries  */
#line 997 "gointer.y"
                {
                    if (strcmp((IgsYYvsp[-5].Name), "vs.")) YYERROR;
                    ShowObservers(atoi((IgsYYvsp[-8].Name)), (IgsYYvsp[-4].Name), (IgsYYvsp[-6].Name), (IgsYYvsp[0].Namelist));

                    myfree((IgsYYvsp[-8].Name));
                    myfree((IgsYYvsp[-6].Name));
                    myfree((IgsYYvsp[-5].Name));
                    myfree((IgsYYvsp[-4].Name));
                    FreeNameList((IgsYYvsp[0].Namelist));
                }
#line 3733 "y.tab.c"
    break;

  case 214: /* gamenotfound: GAMENOTFOUND  */
#line 1010 "gointer.y"
                {
                    const char *arg;

                    arg = StripFirstArgCommand(NULL, "games");
                    if (arg && *arg) {
			if (UserCommandP(NULL)) {
                            Outputf("Game %s not found.\n", arg);
                        } else if (appdata.GamesTimeout > 0) {
			    AutoCommand(NULL, "games");
			}
                    } else {
			Output("Game not found.\n");
		    }
                }
#line 3752 "y.tab.c"
    break;

  case 215: /* nomoremoves: NOMOREMOVES  */
#line 1027 "gointer.y"
                {
		    const char *msg = "There are no more moves";
		    StopMyGameForward(msg);
                    MyGameMessage(msg);
                }
#line 3762 "y.tab.c"
    break;

  case 216: /* notrequestgame: NOTREQUESTGAME  */
#line 1036 "gointer.y"
                {
		    const char *msg = "This teach game is not a request game";
		    StopMyGameForward(msg);
                    MyGameMessage(msg);
                }
#line 3772 "y.tab.c"
    break;

  case 217: /* gamesline: GAMES player NAME player '(' NAME NAME NAME NAME NAME NAME names ')' '(' NAME ')' END  */
#line 1046 "gointer.y"
                {
                    int    Mode, Rules;
                    size_t size;
                    char  *ptr;

                    ptr = (IgsYYvsp[-6].Name);
                    if (ptr[1]) Mode = *ptr++;
                    else Mode = ' ';
                    Rules = *ptr++;
                    if (*ptr) YYERROR;

                    size = atoi((IgsYYvsp[-10].Name));
                    (IgsYYval.Game) = FindGame((IgsYYvsp[-16].Value), (IgsYYvsp[-13].Person), (IgsYYvsp[-15].Person),
                                  atoi((IgsYYvsp[-11].Name)), size, size, atoi((IgsYYvsp[-9].Name)), (IgsYYvsp[-8].Name),
                                  atoi((IgsYYvsp[-7].Name)), Mode, Rules, atoi((IgsYYvsp[-2].Name)));
                    myfree((IgsYYvsp[-14].Name));
                    myfree((IgsYYvsp[-11].Name));
                    myfree((IgsYYvsp[-10].Name));
                    myfree((IgsYYvsp[-9].Name));
                    myfree((IgsYYvsp[-8].Name));
                    myfree((IgsYYvsp[-7].Name));
                    myfree((IgsYYvsp[-6].Name));
                    FreeNameList((IgsYYvsp[-5].Namelist));
                    myfree((IgsYYvsp[-2].Name));
                }
#line 3802 "y.tab.c"
    break;

  case 218: /* gameslines: gameslines gamesline  */
#line 1073 "gointer.y"
                                   { (IgsYYval.Value) = (IgsYYvsp[-1].Value)+1; gamesSeen++; }
#line 3808 "y.tab.c"
    break;

  case 219: /* gameslines: %empty  */
#line 1074 "gointer.y"
                                   { (IgsYYval.Value) = 0; gamesSeen = 0; }
#line 3814 "y.tab.c"
    break;

  case 220: /* $@4: %empty  */
#line 1078 "gointer.y"
                {
                    AssertGamesDeleted();
                }
#line 3822 "y.tab.c"
    break;

  case 221: /* games: GAMES $@4 gameslines  */
#line 1082 "gointer.y"
                {
                    TestGamesDeleted(gamesSeen);
                }
#line 3830 "y.tab.c"
    break;

  case 222: /* remove: REMOVE  */
#line 1087 "gointer.y"
                     { UnObserve((IgsYYvsp[0].Value)); }
#line 3836 "y.tab.c"
    break;

  case 223: /* move: MOVE NAME  */
#line 1091 "gointer.y"
                {
                    char     Num[20], *ptr;
                    NameVal *nameval;
                    (IgsYYval.Nameval) = nameval = mynew(NameVal);
                    sprintf(Num, "%d", (IgsYYvsp[-1].Value));
                    nameval->Next  = nameval->Previous = nameval;
                    nameval->Name  = mystrdup(Num);
                    ptr = (IgsYYvsp[0].Name);
                    /* Get rid of extra `removed stones' entries.
                       Maybe I ought to compare them with what I work out.. */
                    while (*ptr && !isspace(*ptr)) ptr++;
                    if (ptr-(IgsYYvsp[0].Name) <= 3) *ptr = 0;
                    nameval->Value = (IgsYYvsp[0].Name);
                }
#line 3855 "y.tab.c"
    break;

  case 224: /* movelist: movelist move  */
#line 1108 "gointer.y"
                {
                    (IgsYYval.Nameval) = (IgsYYvsp[-1].Nameval);
                    (IgsYYvsp[0].Nameval)->Previous = (IgsYYvsp[-1].Nameval)->Previous;
                    (IgsYYvsp[0].Nameval)->Next     = (IgsYYvsp[-1].Nameval);
                    (IgsYYvsp[0].Nameval)->Previous->Next = (IgsYYvsp[0].Nameval)->Next->Previous = (IgsYYvsp[0].Nameval);
                }
#line 3866 "y.tab.c"
    break;

  case 225: /* movelist: %empty  */
#line 1115 "gointer.y"
                {
                    NameVal *nameval;

                    (IgsYYval.Nameval) = nameval = mynew(NameVal);
                    nameval->Next  = nameval->Previous = nameval;
                    nameval->Name  = NULL;
                    nameval->Value = NULL;
                }
#line 3879 "y.tab.c"
    break;

  case 226: /* optgamesaved: gamesaved  */
#line 1125 "gointer.y"
                        {}
#line 3885 "y.tab.c"
    break;

  case 227: /* optgamesaved: %empty  */
#line 1126 "gointer.y"
              {}
#line 3891 "y.tab.c"
    break;

  case 228: /* gamedesc: GAME NAME '(' NAME NAME NAME ')' NAME NAME '(' NAME NAME NAME ')' END  */
#line 1131 "gointer.y"
                {
                    if (strcmp((IgsYYvsp[-7].Name), "vs")) YYERROR;

                    (IgsYYval.Gamedesc) = mynew(GameDesc);
                    (IgsYYval.Gamedesc)->Id            = (IgsYYvsp[-14].Value);
                    (IgsYYval.Gamedesc)->BlackName     = (IgsYYvsp[-6].Name);
                    (IgsYYval.Gamedesc)->BlackName2    = 0;
                    (IgsYYval.Gamedesc)->BlackCaptures = atoi((IgsYYvsp[-4].Name));
                    (IgsYYval.Gamedesc)->BlackTime     = atoi((IgsYYvsp[-3].Name));
                    (IgsYYval.Gamedesc)->BlackByo      = atoi((IgsYYvsp[-2].Name));
                    (IgsYYval.Gamedesc)->WhiteName     = (IgsYYvsp[-13].Name);
                    (IgsYYval.Gamedesc)->WhiteName2    = 0;
                    (IgsYYval.Gamedesc)->WhiteCaptures = atoi((IgsYYvsp[-11].Name));
                    (IgsYYval.Gamedesc)->WhiteTime     = atoi((IgsYYvsp[-10].Name));
                    (IgsYYval.Gamedesc)->WhiteByo      = atoi((IgsYYvsp[-9].Name));
                    myfree((IgsYYvsp[-11].Name));
                    myfree((IgsYYvsp[-10].Name));
                    myfree((IgsYYvsp[-9].Name));
                    myfree((IgsYYvsp[-7].Name));
                    myfree((IgsYYvsp[-4].Name));
                    myfree((IgsYYvsp[-3].Name));
                    myfree((IgsYYvsp[-2].Name));
                }
#line 3919 "y.tab.c"
    break;

  case 229: /* gamedesc: GAME NAME '(' NAME NAME NAME ')' NAME NAME '(' NAME NAME NAME ')' END TEAMGAME NAME NAME NAME NAME END  */
#line 1158 "gointer.y"
                {
		    Game *game;
                    if (strcmp((IgsYYvsp[-13].Name), "vs")) YYERROR;

                    (IgsYYval.Gamedesc) = mynew(GameDesc);
                    (IgsYYval.Gamedesc)->Id            = (IgsYYvsp[-20].Value);
                    (IgsYYval.Gamedesc)->BlackName     = (IgsYYvsp[-12].Name);
                    (IgsYYval.Gamedesc)->BlackName2    = (IgsYYvsp[-2].Name);
                    (IgsYYval.Gamedesc)->BlackCaptures = atoi((IgsYYvsp[-10].Name));
                    (IgsYYval.Gamedesc)->BlackTime     = atoi((IgsYYvsp[-9].Name));
                    (IgsYYval.Gamedesc)->BlackByo      = atoi((IgsYYvsp[-8].Name));
                    (IgsYYval.Gamedesc)->WhiteName     = (IgsYYvsp[-19].Name);
                    (IgsYYval.Gamedesc)->WhiteName2    = (IgsYYvsp[-1].Name);
                    (IgsYYval.Gamedesc)->WhiteCaptures = atoi((IgsYYvsp[-17].Name));
                    (IgsYYval.Gamedesc)->WhiteTime     = atoi((IgsYYvsp[-16].Name));
                    (IgsYYval.Gamedesc)->WhiteByo      = atoi((IgsYYvsp[-15].Name));
		    /* We must create the game now, since the "games Id"
                     * command will not give the 3rd and 4th players:
                     */
                    TeamGame((IgsYYval.Gamedesc)->Id, (IgsYYvsp[-4].Name), (IgsYYvsp[-3].Name), (IgsYYvsp[-2].Name), (IgsYYvsp[-1].Name), 0);
                    myfree((IgsYYvsp[-17].Name));
                    myfree((IgsYYvsp[-16].Name));
                    myfree((IgsYYvsp[-15].Name));
                    myfree((IgsYYvsp[-13].Name));
                    myfree((IgsYYvsp[-10].Name));
                    myfree((IgsYYvsp[-9].Name));
                    myfree((IgsYYvsp[-8].Name));
                    myfree((IgsYYvsp[-4].Name));
                    myfree((IgsYYvsp[-3].Name));
                }
#line 3954 "y.tab.c"
    break;

  case 230: /* optgametitle: GAMETITLE NAME  */
#line 1190 "gointer.y"
                             { (IgsYYval.Name) = (IgsYYvsp[0].Name); }
#line 3960 "y.tab.c"
    break;

  case 231: /* optgametitle: %empty  */
#line 1191 "gointer.y"
                             { (IgsYYval.Name) = 0;  }
#line 3966 "y.tab.c"
    break;

  case 232: /* add: ADD  */
#line 1194 "gointer.y"
                  { SeenAdd = 1; }
#line 3972 "y.tab.c"
    break;

  case 233: /* doneobserve: DONE observe  */
#line 1198 "gointer.y"
                {
                    Done();
                }
#line 3980 "y.tab.c"
    break;

  case 234: /* mustpass: MUSTPASS  */
#line 1204 "gointer.y"
                {
                    MyGameMessage((IgsYYvsp[0].Name));
                    myfree((IgsYYvsp[0].Name));
                }
#line 3989 "y.tab.c"
    break;

  case 235: /* oppmustpass: OPPMUSTPASS  */
#line 1211 "gointer.y"
                {
                    MyGameMessage((IgsYYvsp[0].Name));
                    myfree((IgsYYvsp[0].Name));
                }
#line 3998 "y.tab.c"
    break;

  case 236: /* disagreeremove: DISAGREEREMOVE  */
#line 1218 "gointer.y"
                {
                    MyGameMessage("There is a disagreement about the "
                                  "life/death of that stone. "
                                  "The game will resume.");
                    RestoreFromScoring();
                    MyGameMessage("Board is restored to what it was"
                                  " before you started scoring");
                }
#line 4011 "y.tab.c"
    break;

  case 237: /* opponentdisagreeremove: OPPDISAGREEREMOVE  */
#line 1229 "gointer.y"
                {
                    MyGameMessage("There is a disagreement about the "
                                  "life/death of that stone. "
                                  "The game will resume.");
                    RestoreFromScoring();
                }
#line 4022 "y.tab.c"
    break;

  case 238: /* optnotreviewing: NOTREVIEWING  */
#line 1238 "gointer.y"
                    { /* teach invalid-game. The gamdesc will force "games"
                       * since teaching games are not announced.
                       */
		      Output("Game not found\n");
                    }
#line 4032 "y.tab.c"
    break;

  case 240: /* observe: gamedesc movelist optgametitle optgamesaved optnotreviewing  */
#line 1247 "gointer.y"
                {
                    Game *game;
                    int   Nr;

		    /* Do not rely on WhatCommand(NULL, "moves"), this creates
                     * a race condition. Instead assume that the movelist
		     * is a result of "moves <id>" if it starts with move 0.
                     */
		    if (SeenAdd || (IgsYYvsp[-3].Nameval)->Next == (IgsYYvsp[-3].Nameval) ||
			strcmp((IgsYYvsp[-3].Nameval)->Next->Name, "0")) {
                        game = AddMove(SeenAdd, (IgsYYvsp[-4].Gamedesc), (IgsYYvsp[-3].Nameval));
/*                      if (game && !MyGameP(game)) ChangeCommand(NULL, -1); */
                        if (game) SetGameTitle(game, (IgsYYvsp[-2].Name));
                        myfree((IgsYYvsp[-2].Name));
                    } else {
		        Moves((IgsYYvsp[-4].Gamedesc), (IgsYYvsp[-3].Nameval));
		    }
                    FreeGameDesc((IgsYYvsp[-4].Gamedesc));
                    FreeNameValList((IgsYYvsp[-3].Nameval));
                }
#line 4057 "y.tab.c"
    break;

  case 241: /* observe: gamedesc movelist OBSERVETEAM NAME NAME NAME NAME END optobserve optgametitle optgamesaved  */
#line 1270 "gointer.y"
                {
                    Game *game;
                    int   Nr;

		    (IgsYYvsp[-10].Gamedesc)->BlackName2 = strcmp((IgsYYvsp[-10].Gamedesc)->BlackName, (IgsYYvsp[-7].Name)) ? (IgsYYvsp[-7].Name) : (IgsYYvsp[-5].Name);
		    (IgsYYvsp[-10].Gamedesc)->WhiteName2 = strcmp((IgsYYvsp[-10].Gamedesc)->WhiteName, (IgsYYvsp[-6].Name)) ? (IgsYYvsp[-6].Name) : (IgsYYvsp[-4].Name);
		    if (SeenAdd || (IgsYYvsp[-9].Nameval)->Next == (IgsYYvsp[-9].Nameval) ||
			strcmp((IgsYYvsp[-9].Nameval)->Next->Name, "0")) {
                        game = AddMove(SeenAdd, (IgsYYvsp[-10].Gamedesc), (IgsYYvsp[-9].Nameval));
/*                      if (game && !MyGameP(game)) ChangeCommand(NULL, -1); */
                        if (game) SetGameTitle(game, (IgsYYvsp[-1].Name));
                        myfree((IgsYYvsp[-1].Name));
                    } else {
		        Moves((IgsYYvsp[-10].Gamedesc), (IgsYYvsp[-9].Nameval));
		    }
                    FreeGameDesc((IgsYYvsp[-10].Gamedesc));
                    FreeNameValList((IgsYYvsp[-9].Nameval));
                }
#line 4080 "y.tab.c"
    break;

  case 242: /* optfirst: FIRSTREMOVE  */
#line 1290 "gointer.y"
                          { (IgsYYval.Value) = (IgsYYvsp[0].Value); }
#line 4086 "y.tab.c"
    break;

  case 243: /* optfirst: %empty  */
#line 1291 "gointer.y"
                          { (IgsYYval.Value) = Empty; }
#line 4092 "y.tab.c"
    break;

  case 244: /* doneopponentobserve: DONE optfirst SEMIPROMPT opponentoptobserve  */
#line 1331 "gointer.y"
                {
                    Done();
                    if ((IgsYYvsp[-2].Value) != Empty)
                        MyGameMessage("%s needs to remove a group first.",
                                      (IgsYYvsp[-2].Value) == White ? "White" : "Black");
                }
#line 4103 "y.tab.c"
    break;

  case 245: /* opponentobserve: gamedesc movelist optgametitle OBSERVE optgamesaved  */
#line 1340 "gointer.y"
                {
                    Game *game;

                    game = AddMove(0, (IgsYYvsp[-4].Gamedesc), (IgsYYvsp[-3].Nameval));
                    FreeGameDesc((IgsYYvsp[-4].Gamedesc));
                    FreeNameValList((IgsYYvsp[-3].Nameval));
                    if (game) SetGameTitle(game, (IgsYYvsp[-2].Name));
                    myfree((IgsYYvsp[-2].Name));
                }
#line 4117 "y.tab.c"
    break;

  case 246: /* opponentoptobserve: gamedesc movelist optgametitle OBSERVE optgamesaved  */
#line 1352 "gointer.y"
                {
                    Game *game;

                    game = AddMove(0, (IgsYYvsp[-4].Gamedesc), (IgsYYvsp[-3].Nameval));
                    FreeGameDesc((IgsYYvsp[-4].Gamedesc));
                    FreeNameValList((IgsYYvsp[-3].Nameval));
                    if (game) SetGameTitle(game, (IgsYYvsp[-2].Name));
                    myfree((IgsYYvsp[-2].Name));
                }
#line 4131 "y.tab.c"
    break;

  case 247: /* opponentoptobserve: gamedesc movelist optgametitle SEMIPROMPT optgamesaved  */
#line 1362 "gointer.y"
                {
                    Game *game;

                    game = AddMove(0, (IgsYYvsp[-4].Gamedesc), (IgsYYvsp[-3].Nameval));
                    FreeGameDesc((IgsYYvsp[-4].Gamedesc));
                    FreeNameValList((IgsYYvsp[-3].Nameval));
                    if (game) SetGameTitle(game, (IgsYYvsp[-2].Name));
                    myfree((IgsYYvsp[-2].Name));
                }
#line 4145 "y.tab.c"
    break;

  case 248: /* betresult: BETRESULT opponentobserve  */
#line 1374 "gointer.y"
                {
                    AutoCommand(NULL, "%%bet bet");
                }
#line 4153 "y.tab.c"
    break;

  case 249: /* betresult: BETRESULT observe  */
#line 1378 "gointer.y"
                {
                    AutoCommand(NULL, "%%bet bet");
                }
#line 4161 "y.tab.c"
    break;

  case 250: /* undidlist: undidlist UNDID NAME  */
#line 1384 "gointer.y"
                {
                    /* undo of multiple moves allowed in a teaching game */
		    MyGameUndo((IgsYYvsp[0].Name));
                    myfree((IgsYYvsp[0].Name));
                }
#line 4171 "y.tab.c"
    break;

  case 251: /* undidlist: UNDID NAME  */
#line 1390 "gointer.y"
                {
		    MyGameUndo((IgsYYvsp[0].Name));
                    myfree((IgsYYvsp[0].Name));
                }
#line 4180 "y.tab.c"
    break;

  case 252: /* undid: undidlist gamedesc movelist optgametitle optgamesaved  */
#line 1398 "gointer.y"
                {
                    FreeGameDesc((IgsYYvsp[-3].Gamedesc));
                    FreeNameValList((IgsYYvsp[-2].Nameval));
                    /* if (game) SetGameTitle(game, $4); */
                    myfree((IgsYYvsp[-1].Name));
                }
#line 4191 "y.tab.c"
    break;

  case 253: /* undid: undidlist gamedesc movelist OBSERVETEAM NAME NAME NAME NAME END optgametitle optgamesaved  */
#line 1406 "gointer.y"
                {
                    FreeGameDesc((IgsYYvsp[-9].Gamedesc));
                    FreeNameValList((IgsYYvsp[-8].Nameval));
                    /* if (game) SetGameTitle(game, $10); */
                    myfree((IgsYYvsp[-1].Name));
                }
#line 4202 "y.tab.c"
    break;

  case 254: /* opponentundid: UNDID NAME OBSERVE gamedesc movelist optgamesaved  */
#line 1416 "gointer.y"
                {
                    char *ptr;

                    ptr = strchr((IgsYYvsp[-4].Name), ')');
                    if (ptr) *ptr = 0;
                    else YYERROR;
                    Undo(0, (IgsYYvsp[-2].Gamedesc)->Id, (IgsYYvsp[-2].Gamedesc)->BlackName, (IgsYYvsp[-2].Gamedesc)->WhiteName, (IgsYYvsp[-4].Name));
                    myfree((IgsYYvsp[-4].Name));
                    FreeGameDesc((IgsYYvsp[-2].Gamedesc));
                    FreeNameValList((IgsYYvsp[-1].Nameval));
                }
#line 4218 "y.tab.c"
    break;

  case 255: /* opponentundid: UNDID NAME OBSERVE gamedesc movelist OBSERVETEAM NAME NAME NAME NAME END optgamesaved  */
#line 1429 "gointer.y"
                {
                    char *ptr;

                    ptr = strchr((IgsYYvsp[-10].Name), ')');
                    if (ptr) *ptr = 0;
                    else YYERROR;
                    Undo(0, (IgsYYvsp[-8].Gamedesc)->Id, (IgsYYvsp[-8].Gamedesc)->BlackName, (IgsYYvsp[-8].Gamedesc)->WhiteName, (IgsYYvsp[-10].Name));
                    myfree((IgsYYvsp[-10].Name));
                    FreeGameDesc((IgsYYvsp[-8].Gamedesc));
                    FreeNameValList((IgsYYvsp[-7].Nameval));
                }
#line 4234 "y.tab.c"
    break;

  case 256: /* restore: RESTORE  */
#line 1442 "gointer.y"
                      {}
#line 4240 "y.tab.c"
    break;

  case 257: /* opponentrestart: RESTART gamedesc movelist OBSERVE  */
#line 1446 "gointer.y"
                {
                    Resume((IgsYYvsp[-2].Gamedesc)->Id, (IgsYYvsp[-2].Gamedesc)->BlackName, (IgsYYvsp[-2].Gamedesc)->WhiteName,
                           (IgsYYvsp[-1].Nameval)->Previous->Name ?
                           1+atoi((IgsYYvsp[-1].Nameval)->Previous->Name) : 0);
                    FreeGameDesc((IgsYYvsp[-2].Gamedesc));
                    FreeNameValList((IgsYYvsp[-1].Nameval));
                }
#line 4252 "y.tab.c"
    break;

  case 258: /* opponentrestart: RESTART gamedesc movelist RESTARTTEAMGAME NAME NAME NAME NAME END OBSERVE  */
#line 1455 "gointer.y"
                {
                    Game *game;
		    /* We must create the game now, since the "games Id"
                     * command will not give the 3rd and 4th players:
		     */
                    game = ResumeTeam((IgsYYvsp[-8].Gamedesc)->Id, (IgsYYvsp[-5].Name), (IgsYYvsp[-4].Name), (IgsYYvsp[-3].Name), (IgsYYvsp[-2].Name),
                                  (IgsYYvsp[-7].Nameval)->Previous->Name ?
                                  1+atoi((IgsYYvsp[-7].Nameval)->Previous->Name) : 0);
                    if (game) SetGameTitle(game, (IgsYYvsp[0].Dummy));
                    FreeGameDesc((IgsYYvsp[-8].Gamedesc));
                    FreeNameValList((IgsYYvsp[-7].Nameval));
                    myfree((IgsYYvsp[0].Dummy));
                }
#line 4270 "y.tab.c"
    break;

  case 259: /* restart: RESTART gamedesc movelist optgametitle  */
#line 1471 "gointer.y"
                {
                    Game *game;

                    game = Resume((IgsYYvsp[-2].Gamedesc)->Id, (IgsYYvsp[-2].Gamedesc)->BlackName, (IgsYYvsp[-2].Gamedesc)->WhiteName,
                                  (IgsYYvsp[-1].Nameval)->Previous->Name ?
                                  1+atoi((IgsYYvsp[-1].Nameval)->Previous->Name) : 0);
                    if (game) SetGameTitle(game, (IgsYYvsp[0].Name));
                    FreeGameDesc((IgsYYvsp[-2].Gamedesc));
                    FreeNameValList((IgsYYvsp[-1].Nameval));
                    myfree((IgsYYvsp[0].Name));
                }
#line 4286 "y.tab.c"
    break;

  case 260: /* restart: RESTART gamedesc movelist RESTARTTEAMGAME NAME NAME NAME NAME END optgametitle  */
#line 1484 "gointer.y"
                {
                    Game *game;
		    /* We must create the game now, since the "games Id"
                     * command will not give the 3rd and 4th players:
		     */
                    game = ResumeTeam((IgsYYvsp[-8].Gamedesc)->Id, (IgsYYvsp[-5].Name), (IgsYYvsp[-4].Name), (IgsYYvsp[-3].Name), (IgsYYvsp[-2].Name),
                                  (IgsYYvsp[-7].Nameval)->Previous->Name ?
                                  1+atoi((IgsYYvsp[-7].Nameval)->Previous->Name) : 0);
                    if (game) SetGameTitle(game, (IgsYYvsp[0].Name));
                    FreeGameDesc((IgsYYvsp[-8].Gamedesc));
                    FreeNameValList((IgsYYvsp[-7].Nameval));
                    myfree((IgsYYvsp[0].Name));
                }
#line 4304 "y.tab.c"
    break;

  case 261: /* newmatch1: OBSERVE newmatch2  */
#line 1499 "gointer.y"
                                {}
#line 4310 "y.tab.c"
    break;

  case 262: /* newmatch2: gamedesc NEWMATCH  */
#line 1503 "gointer.y"
                {
                    SendCommand(NULL, INT_TO_XTPOINTER((IgsYYvsp[-1].Gamedesc)->Id+1),
				"games %d", (IgsYYvsp[-1].Gamedesc)->Id);
                    /* INT_TO_XTPOINTER(Id+1) will set ForceNew = Id
                     in AssertGamesDeleted() */
                    FreeGameDesc((IgsYYvsp[-1].Gamedesc));
                }
#line 4322 "y.tab.c"
    break;

  case 263: /* decline: DECLINE  */
#line 1513 "gointer.y"
                {
                    Decline((IgsYYvsp[0].Person));
                }
#line 4330 "y.tab.c"
    break;

  case 264: /* disputeline: PERSON GAMECOLOR NAME GAMESECONDS BYOYOMI END  */
#line 1519 "gointer.y"
                {
                    (IgsYYval.Disputedesc) = mynew(DisputeDesc);
                    (IgsYYval.Disputedesc)->Player = (IgsYYvsp[-5].Person);
                    (IgsYYval.Disputedesc)->Color  = (IgsYYvsp[-4].Value);
                    (IgsYYval.Disputedesc)->SizeX  = atoi((IgsYYvsp[-3].Name));
                    (IgsYYval.Disputedesc)->SizeY  = atoi(strchr((IgsYYvsp[-3].Name), 'x')+1);
                    (IgsYYval.Disputedesc)->Tim    = (IgsYYvsp[-2].Value);
                    (IgsYYval.Disputedesc)->ByoYomi= (IgsYYvsp[-1].Value);
                    myfree((IgsYYvsp[-3].Name));
                }
#line 4345 "y.tab.c"
    break;

  case 265: /* disputelines: disputelines disputeline  */
#line 1532 "gointer.y"
                {
                    (IgsYYvsp[0].Disputedesc)->Next     = (IgsYYvsp[-1].Disputedesc);
                    (IgsYYvsp[0].Disputedesc)->Previous = (IgsYYvsp[-1].Disputedesc)->Previous;
                    (IgsYYvsp[0].Disputedesc)->Next->Previous = (IgsYYvsp[0].Disputedesc)->Previous->Next = (IgsYYvsp[0].Disputedesc);
                    (IgsYYval.Disputedesc) = (IgsYYvsp[-1].Disputedesc);
                }
#line 4356 "y.tab.c"
    break;

  case 266: /* disputelines: %empty  */
#line 1539 "gointer.y"
                {
                    (IgsYYval.Disputedesc) = mynew(DisputeDesc);
                    (IgsYYval.Disputedesc)->Next = (IgsYYval.Disputedesc)->Previous = (IgsYYval.Disputedesc);
                }
#line 4365 "y.tab.c"
    break;

  case 267: /* opponentdispute: OPPONENTDISPUTE disputelines  */
#line 1546 "gointer.y"
                {
                    DisputeDesc *Here, *Next;

                    Dispute((IgsYYvsp[0].Disputedesc), 1);
                    for (Here = (IgsYYvsp[0].Disputedesc)->Next; Here != (IgsYYvsp[0].Disputedesc); Here = Next) {
                        Next = Here->Next;
                        myfree(Here);
                    }
                    myfree((IgsYYvsp[0].Disputedesc));
                }
#line 4380 "y.tab.c"
    break;

  case 268: /* dispute: DISPUTE disputelines  */
#line 1559 "gointer.y"
                {
                    DisputeDesc *Here, *Next;

                    Dispute((IgsYYvsp[0].Disputedesc), 0);
                    for (Here = (IgsYYvsp[0].Disputedesc)->Next; Here != (IgsYYvsp[0].Disputedesc); Here = Next) {
                        Next = Here->Next;
                        myfree(Here);
                    }
                    myfree((IgsYYvsp[0].Disputedesc));
                }
#line 4395 "y.tab.c"
    break;

  case 269: /* matchtypes: matchtypes MATCHTYPE  */
#line 1572 "gointer.y"
                {
                    (IgsYYval.Value) = (IgsYYvsp[-1].Value) | (IgsYYvsp[0].Value);
                }
#line 4403 "y.tab.c"
    break;

  case 270: /* matchtypes: %empty  */
#line 1576 "gointer.y"
                {
                    (IgsYYval.Value) = 0;
                }
#line 4411 "y.tab.c"
    break;

  case 271: /* disputematchtype: DISPUTEMATCHTYPE matchtypes END  */
#line 1582 "gointer.y"
                {
                    WantMatchType((IgsYYvsp[-2].Person), (IgsYYvsp[-1].Value));
                }
#line 4419 "y.tab.c"
    break;

  case 272: /* optobserve: OBSERVE  */
#line 1588 "gointer.y"
                {
                }
#line 4426 "y.tab.c"
    break;

  case 274: /* undolist: undolist optobserve UNDO NAME NAME NAME  */
#line 1595 "gointer.y"
                {             /* gameid white black move */
		    Undo(0, (IgsYYvsp[-3].Value), (IgsYYvsp[-1].Name), (IgsYYvsp[-2].Name), (IgsYYvsp[0].Name));
                    myfree((IgsYYvsp[0].Name));
                    myfree((IgsYYvsp[-1].Name));
                    myfree((IgsYYvsp[-2].Name));
                }
#line 4437 "y.tab.c"
    break;

  case 275: /* undolist: UNDO NAME NAME NAME  */
#line 1602 "gointer.y"
                {
		    Undo(0, (IgsYYvsp[-3].Value), (IgsYYvsp[-1].Name), (IgsYYvsp[-2].Name), (IgsYYvsp[0].Name));
                    myfree((IgsYYvsp[0].Name));
                    myfree((IgsYYvsp[-1].Name));
                    myfree((IgsYYvsp[-2].Name));
                }
#line 4448 "y.tab.c"
    break;

  case 276: /* undo: undolist optobserve gamedesc movelist optgametitle  */
#line 1611 "gointer.y"
                {
                    FreeGameDesc((IgsYYvsp[-2].Gamedesc));
                    FreeNameValList((IgsYYvsp[-1].Nameval));
                    /* if (game && $5) SetGameTitle(game, $5); */
                    myfree((IgsYYvsp[0].Name));
                }
#line 4459 "y.tab.c"
    break;

  case 277: /* undo: undolist optobserve gamedesc movelist OBSERVETEAM NAME NAME NAME NAME END optgametitle  */
#line 1619 "gointer.y"
                {
                    FreeGameDesc((IgsYYvsp[-8].Gamedesc));
                    FreeNameValList((IgsYYvsp[-7].Nameval));
                    /* if (game && $11) SetGameTitle(game, $11); */
                    myfree((IgsYYvsp[0].Name));
                }
#line 4470 "y.tab.c"
    break;

  case 278: /* watching: WATCHING names END  */
#line 1628 "gointer.y"
                {
                    Watching((IgsYYvsp[-1].Namelist));
                    FreeNameList((IgsYYvsp[-1].Namelist));
                }
#line 4479 "y.tab.c"
    break;

  case 279: /* overobserve: OVEROBSERVE  */
#line 1635 "gointer.y"
                {
                    OverObserve((IgsYYvsp[0].Value));
                }
#line 4487 "y.tab.c"
    break;

  case 280: /* observewhileplay: OBSERVEWHILEPLAY  */
#line 1641 "gointer.y"
                {
                    ObserveWhilePlaying();
                }
#line 4495 "y.tab.c"
    break;

  case 281: /* playerline: PLAYERS NAME NAME NAME NAME END  */
#line 1647 "gointer.y"
                {
                    FindPlayer((IgsYYvsp[-3].Name), (IgsYYvsp[-1].Name), (IgsYYvsp[-4].Name), (IgsYYvsp[-2].Name));
                    myfree((IgsYYvsp[-4].Name));
                    myfree((IgsYYvsp[-3].Name));
                    myfree((IgsYYvsp[-2].Name));
                    myfree((IgsYYvsp[-1].Name));
                }
#line 4507 "y.tab.c"
    break;

  case 282: /* playerline: PLAYERS NAME END  */
#line 1655 "gointer.y"
                {
                    FindPlayer((IgsYYvsp[-1].Name), "???", "?????  ???", UNKNOWN);
                    myfree((IgsYYvsp[-1].Name));
                }
#line 4516 "y.tab.c"
    break;

  case 284: /* playerlines: %empty  */
#line 1663 "gointer.y"
                {

                }
#line 4524 "y.tab.c"
    break;

  case 285: /* playersstatusline: NAME '(' NAME ')' NAME NAME NAME NAME NAME NAME END  */
#line 1669 "gointer.y"
                {
                    PlayerStatusLine(atoi((IgsYYvsp[-10].Name)), atoi((IgsYYvsp[-8].Name)), atoi((IgsYYvsp[-4].Name)));
                    myfree((IgsYYvsp[-10].Name));
                    myfree((IgsYYvsp[-8].Name));
                    myfree((IgsYYvsp[-6].Name));
                    myfree((IgsYYvsp[-5].Name));
                    myfree((IgsYYvsp[-4].Name));
                    myfree((IgsYYvsp[-3].Name));
                    myfree((IgsYYvsp[-2].Name));
                    myfree((IgsYYvsp[-1].Name));
                }
#line 4540 "y.tab.c"
    break;

  case 286: /* playersstatusline: NAME NAME NAME NAME NAME NAME END  */
#line 1681 "gointer.y"
                {
                    PlayerStatusLine(atoi((IgsYYvsp[-6].Name)), -1, atoi((IgsYYvsp[-4].Name)));
                    myfree((IgsYYvsp[-6].Name));
                    myfree((IgsYYvsp[-5].Name));
                    myfree((IgsYYvsp[-4].Name));
                    myfree((IgsYYvsp[-3].Name));
                    myfree((IgsYYvsp[-2].Name));
                    myfree((IgsYYvsp[-1].Name));
                }
#line 4554 "y.tab.c"
    break;

  case 287: /* $@5: %empty  */
#line 1693 "gointer.y"
                {
                    AssertPlayersDeleted();
                }
#line 4562 "y.tab.c"
    break;

  case 288: /* $@6: %empty  */
#line 1697 "gointer.y"
                {
                    TestPlayersDeleted();
                }
#line 4570 "y.tab.c"
    break;

  case 290: /* userline: names END  */
#line 1704 "gointer.y"
                {
                    NameList *Here;
                    int n;

                    n =0;
                    for (Here = (IgsYYvsp[-1].Namelist)->Next; Here != (IgsYYvsp[-1].Namelist); Here = Here->Next) n++;
                    if (n == 11) {
		      (IgsYYval.Namelist) = (IgsYYvsp[-1].Namelist);
		    } else if (n == 0) { /* list header */
		      (IgsYYval.Namelist) = NULL;
                    } else {
                        /* Don't call YYERROR. user command leads to easily
                           to parse errors */
		        Output("Got the expected parse error following a "
                               "\"user\" command:\n");
                        for (Here = (IgsYYvsp[-1].Namelist)->Next; Here != (IgsYYvsp[-1].Namelist); Here = Here->Next) {
                            Output(Here->Name);
                            Output(" ");
                        }
                        Output("\n");
                        _IgsDefaultParse();
                        FreeNameList((IgsYYvsp[-1].Namelist));
                        (IgsYYval.Namelist) = NULL;
                    }
                }
#line 4600 "y.tab.c"
    break;

  case 291: /* userline: names FAIL  */
#line 1730 "gointer.y"
                {
                    NameList *Here;

                    Output("Got the expected parse failure following a "
                           "\"user\" command:\n");
                    for (Here = (IgsYYvsp[-1].Namelist)->Next; Here != (IgsYYvsp[-1].Namelist); Here = Here->Next) {
                        Output(Here->Name);
                        Output(" ");
                    }
                    Output("\n");
                    _IgsDefaultParse();
                    FreeNameList((IgsYYvsp[-1].Namelist));
                    (IgsYYval.Namelist) = NULL;
                }
#line 4619 "y.tab.c"
    break;

  case 292: /* userlines: userlines userline  */
#line 1747 "gointer.y"
                {
                    if ((IgsYYvsp[0].Namelist)) {
                        NameListList *Last;
                        Last = mynew(NameListList);
                        Last->Names    = (IgsYYvsp[0].Namelist);
                        Last->Previous = (IgsYYvsp[-1].NameListlist)->Previous;
                        Last->Next     = (IgsYYvsp[-1].NameListlist);
                        Last->Next->Previous = Last->Previous->Next = Last;
                    }
                    (IgsYYval.NameListlist) = (IgsYYvsp[-1].NameListlist);
                }
#line 4635 "y.tab.c"
    break;

  case 293: /* userlines: %empty  */
#line 1759 "gointer.y"
                {
                    (IgsYYval.NameListlist) = mynew(NameListList);
                    (IgsYYval.NameListlist)->Previous = (IgsYYval.NameListlist)->Next = (IgsYYval.NameListlist);
                    (IgsYYval.NameListlist)->Names = NULL;
                }
#line 4645 "y.tab.c"
    break;

  case 294: /* users: USER userlines  */
#line 1767 "gointer.y"
                {
                    UserData((IgsYYvsp[0].NameListlist));
                    FreeNameListList((IgsYYvsp[0].NameListlist));
                }
#line 4654 "y.tab.c"
    break;

  case 295: /* player: NAME '[' NAME ']'  */
#line 1774 "gointer.y"
                {
                    (IgsYYval.Person) = FindPlayerByNameAndStrength((IgsYYvsp[-3].Name), (IgsYYvsp[-1].Name));
                    myfree((IgsYYvsp[-3].Name));
                    myfree((IgsYYvsp[-1].Name));
                }
#line 4664 "y.tab.c"
    break;

  case 296: /* playertime: NAME ':' NAME  */
#line 1782 "gointer.y"
                {
                    int sec;

                    sec = atoi((IgsYYvsp[0].Name));
                    if ((IgsYYvsp[-2].Name)[0] == '-') (IgsYYval.Value) = -60 * atoi((IgsYYvsp[-2].Name)+1)-sec;
                    else              (IgsYYval.Value) =  60 * atoi((IgsYYvsp[-2].Name))  +sec;
                    myfree((IgsYYvsp[-2].Name));
                    myfree((IgsYYvsp[0].Name));
                }
#line 4678 "y.tab.c"
    break;

  case 297: /* optbyo: '(' NAME ')' NAME  */
#line 1794 "gointer.y"
                {
                    (IgsYYval.Value) = atoi((IgsYYvsp[0].Name));
                    myfree((IgsYYvsp[-2].Name));
                    myfree((IgsYYvsp[0].Name));
                }
#line 4688 "y.tab.c"
    break;

  case 298: /* optbyo: %empty  */
#line 1799 "gointer.y"
                { (IgsYYval.Value) = -1; }
#line 4694 "y.tab.c"
    break;

  case 299: /* gametime: GAMETIME NAME ':' NAME END GAMETIME NAME '(' NAME ')' ':' playertime optbyo END GAMETIME NAME '(' NAME ')' ':' playertime optbyo END  */
#line 1805 "gointer.y"
                {
                    if (strcmp((IgsYYvsp[-21].Name), "Game") ||
                        strcmp((IgsYYvsp[-16].Name), "White") || strcmp((IgsYYvsp[-7].Name), "Black")) YYERROR;
                    GameTime(atoi((IgsYYvsp[-19].Name)), (IgsYYvsp[-5].Name), (IgsYYvsp[-2].Value), (IgsYYvsp[-1].Value), (IgsYYvsp[-14].Name), (IgsYYvsp[-11].Value), (IgsYYvsp[-10].Value));
                    myfree((IgsYYvsp[-21].Name));
                    myfree((IgsYYvsp[-19].Name));
                    myfree((IgsYYvsp[-16].Name));
                    myfree((IgsYYvsp[-14].Name));
                    myfree((IgsYYvsp[-7].Name));
                    myfree((IgsYYvsp[-5].Name));
                }
#line 4710 "y.tab.c"
    break;

  case 300: /* gamescore: CURRENTSCORE NAME FINALSCORE NAME  */
#line 1819 "gointer.y"
                {
                    int Nr;
                    Game     *game;

                    if (!UserCommandP(NULL) &&
                        (Nr = WhatCommand(NULL, "score")) >= 0 &&
                        (game = ServerIdToGame(Nr)) != NULL) {
                        GameMessage(game, "..........", "Current score:");
                        GameMessage(game, "..........", "%s", (IgsYYvsp[-2].Name));
                        GameMessage(game, "..........", "Final score:");
                        GameMessage(game, "..........", "%s", (IgsYYvsp[0].Name));
                    } else {
                        Outputf("Current score:\n %s\n", (IgsYYvsp[-2].Name));
                        Outputf("Final score:\n %s\n", (IgsYYvsp[0].Name));
                    }
                    myfree((IgsYYvsp[-2].Name));
                    myfree((IgsYYvsp[0].Name));
                }
#line 4733 "y.tab.c"
    break;

  case 301: /* translation: TRANSLATION NAME  */
#line 1840 "gointer.y"
                {
                    Outputf("%s\n", (IgsYYvsp[0].Name));
                    myfree((IgsYYvsp[0].Name));
                }
#line 4742 "y.tab.c"
    break;

  case 304: /* byoyomi: ENTERBYOYOMI GIVEBYOYOMI  */
#line 1851 "gointer.y"
                {
                    MyGameMessage("%s is now in byo-yomi, having %s",
                                  PlayerString((IgsYYvsp[-1].Person)), (IgsYYvsp[0].Name));
                    myfree((IgsYYvsp[0].Name));
                }
#line 4752 "y.tab.c"
    break;

  case 305: /* notime: NOTIME  */
#line 1859 "gointer.y"
                {
                    MyGameMessage("%s has run out of time.", PlayerString((IgsYYvsp[0].Person)));
                }
#line 4760 "y.tab.c"
    break;

  case 306: /* lostconnection: LOSTCONNECTION MYADJOURN optgamesaved  */
#line 1865 "gointer.y"
                {
                    MyGameMessage("Your opponent has lost his connection.");
                }
#line 4768 "y.tab.c"
    break;

  case 307: /* gamesaved: GAMESAVED NAME optobserve  */
#line 1871 "gointer.y"
                {
		    if (appdata.WantVerbose) {
                        MyGameMessage("Game saved.%s", (IgsYYvsp[-1].Name));
		    }
                    myfree((IgsYYvsp[-1].Name));
                }
#line 4779 "y.tab.c"
    break;

  case 308: /* optadjourn: MYADJOURN  */
#line 1880 "gointer.y"
                {
                }
#line 4786 "y.tab.c"
    break;

  case 310: /* adjourn: MYADJOURN optadjourn GAMESAVED NAME optgamesaved  */
#line 1887 "gointer.y"
                {
                    MyGameMessage("Game has been adjourned.");
                    MyGameMessage("Game saved.%s", (IgsYYvsp[-1].Name));
                    myfree((IgsYYvsp[-1].Name));
                }
#line 4796 "y.tab.c"
    break;

  case 311: /* adjournsentrequest: ADJOURNSENTREQUEST  */
#line 1895 "gointer.y"
                {
                }
#line 4803 "y.tab.c"
    break;

  case 312: /* adjournrequest: ADJOURNREQUEST  */
#line 1900 "gointer.y"
                {
                    MyGameMessage("Your opponent requests an adjournment");
                    MyGameMessage("Use the <adjourn> or <decline adjourn> "
                                  "entries in the commands menu.");
                }
#line 4813 "y.tab.c"
    break;

  case 313: /* oppadjourn: MYADJOURN  */
#line 1908 "gointer.y"
                {
                    MyGameMessage("Game has been adjourned.");
                }
#line 4821 "y.tab.c"
    break;

  case 314: /* declineadjourn: DECLINEADJOURN  */
#line 1914 "gointer.y"
                {
                    MyGameMessage("Your opponent declines to adjourn.");
                }
#line 4829 "y.tab.c"
    break;

  case 315: /* resign: RESIGN  */
#line 1920 "gointer.y"
                {
                    MyGameMessage("%s has resigned the game.",
                                  PlayerString((IgsYYvsp[0].Person)));
                }
#line 4838 "y.tab.c"
    break;

  case 316: /* resign: RESIGN RESIGN  */
#line 1925 "gointer.y"
                { /* Double message in teaching game --Ton */
                    MyGameMessage("%s has resigned the game.",
                                  PlayerString((IgsYYvsp[-1].Person)));
                }
#line 4847 "y.tab.c"
    break;

  case 317: /* mailed: MAILED MAILED  */
#line 1932 "gointer.y"
               {
                   Mailed((IgsYYvsp[-1].Name));
                   Mailed((IgsYYvsp[0].Name));
                   myfree((IgsYYvsp[-1].Name));
                   myfree((IgsYYvsp[0].Name));
               }
#line 4858 "y.tab.c"
    break;

  case 318: /* mailed: MAILED  */
#line 1939 "gointer.y"
               {
                   Mailed((IgsYYvsp[0].Name));
                   myfree((IgsYYvsp[0].Name));
               }
#line 4867 "y.tab.c"
    break;

  case 319: /* removegamefile: REMOVEGAMEFILE  */
#line 1946 "gointer.y"
               {
                   RemoveGameFile((IgsYYvsp[0].Name));
                   myfree((IgsYYvsp[0].Name));
               }
#line 4876 "y.tab.c"
    break;

  case 320: /* notelltarget: NOTELLTARGET  */
#line 1953 "gointer.y"
                {
                    NoTell();
                }
#line 4884 "y.tab.c"
    break;

  case 321: /* telltarget: TELLTARGET NAME END  */
#line 1959 "gointer.y"
                {
                    /* Kludge to stop bell/raise at telltarget change --Ton */
                    int OldEntered;

                    OldEntered = Entered;
                    Entered = 0;
		    if (appdata.WantVerbose) {
                        Outputf("Setting your '.' to %16s\n",
                                PlayerNameToString((IgsYYvsp[-1].Name)));
		    }
                    Entered = OldEntered;
                    myfree((IgsYYvsp[-1].Name));
                }
#line 4902 "y.tab.c"
    break;

  case 322: /* telldone: TELLDONE  */
#line 1975 "gointer.y"
                {
                }
#line 4909 "y.tab.c"
    break;

  case 323: /* telloff: TELLOFF names  */
#line 1980 "gointer.y"
                {
                    NameList *Here;

                    Output("User is not accepting tells.\n");
                    for (Here = (IgsYYvsp[0].Namelist)->Next; Here != (IgsYYvsp[0].Namelist); Here = Here->Next)
                        Outputf("%s\n", Here->Name);
                    FreeNameList((IgsYYvsp[0].Namelist));
                }
#line 4922 "y.tab.c"
    break;

  case 324: /* illegalmove: ILLEGALMOVE  */
#line 1991 "gointer.y"
                {
                    MyGameMessage("Illegal move: %s", (IgsYYvsp[0].Name));
                    myfree((IgsYYvsp[0].Name));
                }
#line 4931 "y.tab.c"
    break;

  case 325: /* illegalmove: ILLEGALMOVE OBSERVE optobserve  */
#line 1996 "gointer.y"
                {
                    MyGameMessage("Illegal move: %s", (IgsYYvsp[-2].Name));
                    myfree((IgsYYvsp[-2].Name));
                }
#line 4940 "y.tab.c"
    break;

  case 326: /* illegalundo: ILLEGALUNDO  */
#line 2003 "gointer.y"
                {
                    MyGameMessage("Cannot undo: %s", (IgsYYvsp[0].Name));
                    myfree((IgsYYvsp[0].Name));
                }
#line 4949 "y.tab.c"
    break;

  case 327: /* noturn: NOTURN  */
#line 2010 "gointer.y"
                {
                    MyGameMessage("It isn't your turn");
                }
#line 4957 "y.tab.c"
    break;

  case 328: /* noremoveturn: NOREMOVETURN  */
#line 2016 "gointer.y"
                {
                    MyGameMessage("It is not your turn to remove a group");
                }
#line 4965 "y.tab.c"
    break;

  case 329: /* useresign: USERESIGN  */
#line 2022 "gointer.y"
                {
                    MyGameMessage("To resign, please use 'resign'");
                }
#line 4973 "y.tab.c"
    break;

  case 330: /* removeliberty: REMOVELIBERTY  */
#line 2028 "gointer.y"
                {
                    MyGameMessage("You cannot remove liberties.");
                }
#line 4981 "y.tab.c"
    break;

  case 331: /* removegroup: REMOVEGROUP  */
#line 2034 "gointer.y"
                {
                    RemoveGroup((IgsYYvsp[0].Name));
                    myfree((IgsYYvsp[0].Name));
                }
#line 4990 "y.tab.c"
    break;

  case 332: /* restorescoring: RESTORESCORING  */
#line 2041 "gointer.y"
                {
                    RestoreScoring();
                }
#line 4998 "y.tab.c"
    break;

  case 333: /* pleaseredone: PLEASEREDONE  */
#line 2047 "gointer.y"
                {
                    MyGameMessage("Please repeat 'done'");
                }
#line 5006 "y.tab.c"
    break;

  case 334: /* statusheader: STATUSHEADER names END  */
#line 2053 "gointer.y"
                {
                    (IgsYYval.Namelist) = (IgsYYvsp[-1].Namelist);
                }
#line 5014 "y.tab.c"
    break;

  case 335: /* statusline: STATUSLINE NAME  */
#line 2059 "gointer.y"
                {
                    (IgsYYval.Numval) = mynew(NumVal);
                    (IgsYYval.Numval)->Num   = (IgsYYvsp[-1].Value);
                    (IgsYYval.Numval)->Value = (IgsYYvsp[0].Name);
                }
#line 5024 "y.tab.c"
    break;

  case 336: /* statuslines: statuslines statusline  */
#line 2067 "gointer.y"
                {
                    (IgsYYvsp[0].Numval)->Next = (IgsYYvsp[-1].Numval);
                    (IgsYYvsp[0].Numval)->Previous = (IgsYYvsp[-1].Numval)->Previous;
                    (IgsYYvsp[0].Numval)->Next->Previous = (IgsYYvsp[0].Numval)->Previous->Next = (IgsYYvsp[0].Numval);
                    (IgsYYval.Numval) = (IgsYYvsp[-1].Numval);
                }
#line 5035 "y.tab.c"
    break;

  case 337: /* statuslines: %empty  */
#line 2074 "gointer.y"
                {
                    (IgsYYval.Numval) = mynew(NumVal);
                    (IgsYYval.Numval)->Next  = (IgsYYval.Numval)->Previous = (IgsYYval.Numval);
                    (IgsYYval.Numval)->Num   = -1;
                    (IgsYYval.Numval)->Value = NULL;
                }
#line 5046 "y.tab.c"
    break;

  case 338: /* resultline: RESULTLINE  */
#line 2083 "gointer.y"
                {
	            /* 20 jl (W:O):  2.5 to jloup (B:#):  3.0 */
                    MyGameMessage("%s", (IgsYYvsp[0].Name));
                    myfree((IgsYYvsp[0].Name));
                }
#line 5056 "y.tab.c"
    break;

  case 339: /* status: statusheader statusheader statuslines  */
#line 2091 "gointer.y"
                {
                    if (GamePosition((IgsYYvsp[-1].Namelist), (IgsYYvsp[-2].Namelist), (IgsYYvsp[0].Numval))) ChangeCommand(NULL, 1);
                    FreeNameList((IgsYYvsp[-2].Namelist));
                    FreeNameList((IgsYYvsp[-1].Namelist));
                    FreeNumValList((IgsYYvsp[0].Numval));
                }
#line 5067 "y.tab.c"
    break;

  case 340: /* date: NAME NAME NAME NAME ':' NAME ':' NAME NAME  */
#line 2100 "gointer.y"
                {
                    struct tm *FullTime;
                    int        i;
                    static const char *Month[] = {
                        "Jan", "Feb", "Mar", "Apr", "May", "Jun",
                        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

                    FullTime = mynew(struct tm);
                    for (i=0; i<12; i++) if (strcmp((IgsYYvsp[-7].Name), Month[i]) == 0) break;
                    FullTime->tm_year  = atoi((IgsYYvsp[0].Name))-1900;
                    FullTime->tm_mon   = i;
                    FullTime->tm_mday  = atoi((IgsYYvsp[-6].Name));
                    FullTime->tm_hour  = atoi((IgsYYvsp[-5].Name));
                    FullTime->tm_min   = atoi((IgsYYvsp[-3].Name));
                    FullTime->tm_sec   = atoi((IgsYYvsp[-1].Name));
                    FullTime->tm_isdst = LocalTime.tm_isdst;
                    mktime(FullTime);
                    (IgsYYval.Dummy) = FullTime;
                    myfree((IgsYYvsp[-8].Name));
                    myfree((IgsYYvsp[-7].Name));
                    myfree((IgsYYvsp[-6].Name));
                    myfree((IgsYYvsp[-5].Name));
                    myfree((IgsYYvsp[-3].Name));
                    myfree((IgsYYvsp[-1].Name));
                    myfree((IgsYYvsp[0].Name));
                }
#line 5098 "y.tab.c"
    break;

  case 341: /* uptimeentry: GMTTIME date END  */
#line 2129 "gointer.y"
                {
                    char *ptr;
                    int   Length;

		    if (appdata.WantVerbose) {
			ptr = asctime((IgsYYvsp[-1].Dummy));
			Length = strlen(ptr);
			Outputf("Gmt    time: %.*s\n", Length-1, ptr);
		    }
                    myfree((IgsYYvsp[-1].Dummy));
                }
#line 5114 "y.tab.c"
    break;

  case 342: /* uptimeentry: LOCALTIME date END  */
#line 2141 "gointer.y"
                {
                    char *ptr;
                    int   Length;

                    ServerTime = *(struct tm *) (IgsYYvsp[-1].Dummy);
                    SetServerTime = 1;
		    if (appdata.WantVerbose) {
			ptr = asctime((IgsYYvsp[-1].Dummy));
			Length = strlen(ptr);
			Outputf("Server time: %.*s\n", Length-1, ptr);
		    }
                    myfree((IgsYYvsp[-1].Dummy));
                }
#line 5132 "y.tab.c"
    break;

  case 343: /* uptimeentry: SERVERUP NAME NAME NAME NAME NAME NAME END  */
#line 2155 "gointer.y"
                {
                    long Uptime;

		    if (appdata.WantVerbose) {
			Uptime = ((24*atoi((IgsYYvsp[-6].Name)) + atoi((IgsYYvsp[-4].Name)))*60 + atoi((IgsYYvsp[-2].Name))) * 60;
			Outputf("Uptime: %s %s %s %s %s %s (%ld seconds)\n",
				(IgsYYvsp[-6].Name), (IgsYYvsp[-5].Name), (IgsYYvsp[-4].Name), (IgsYYvsp[-3].Name), (IgsYYvsp[-2].Name), (IgsYYvsp[-1].Name), Uptime);
		    }
                    myfree((IgsYYvsp[-6].Name));
                    myfree((IgsYYvsp[-5].Name));
                    myfree((IgsYYvsp[-4].Name));
                    myfree((IgsYYvsp[-3].Name));
                    myfree((IgsYYvsp[-2].Name));
                    myfree((IgsYYvsp[-1].Name));
                }
#line 5152 "y.tab.c"
    break;

  case 344: /* uptimeentry: UPTIMEENTRY NAME  */
#line 2171 "gointer.y"
                {
		    if (appdata.WantVerbose) {
			Outputf("%s\n", (IgsYYvsp[0].Name));
		    }
                    myfree((IgsYYvsp[0].Name));
                }
#line 5163 "y.tab.c"
    break;

  case 347: /* sgflist: SGFLIST names END  */
#line 2184 "gointer.y"
                {
                    SgfList((IgsYYvsp[-1].Namelist));
                    FreeNameList((IgsYYvsp[-1].Namelist));
                }
#line 5172 "y.tab.c"
    break;

  case 348: /* sgflist: SGFLIST NOSGF  */
#line 2189 "gointer.y"
                {
                    Output("sgf needs arguments\n");
                }
#line 5180 "y.tab.c"
    break;

  case 349: /* reviewlist: REVIEWLIST names  */
#line 2195 "gointer.y"
                {
                    ReviewList((IgsYYvsp[0].Namelist));
                    FreeNameList((IgsYYvsp[0].Namelist));
                }
#line 5189 "y.tab.c"
    break;

  case 350: /* reviewvariations: REVIEWVARIATIONS names  */
#line 2202 "gointer.y"
                {
/* For the moment we just ignore the variations list
                    NameList *Here;

                    Output("Variations:");
                    for (Here = $2->Next; Here != $2; Here = Here->Next)
                        Outputf(" %s", Here->Name);
                    Output("\n");
*/
                    FreeNameList((IgsYYvsp[0].Namelist));
                }
#line 5205 "y.tab.c"
    break;

  case 351: /* reviewstart: REVIEWSTART  */
#line 2216 "gointer.y"
                {
                    ReviewStart((IgsYYvsp[0].Name));
                    myfree((IgsYYvsp[0].Name));
                }
#line 5214 "y.tab.c"
    break;

  case 352: /* reviewliterals: reviewliterals REVLITERAL  */
#line 2223 "gointer.y"
                {
                    NameList *names;

                    names = mynew(NameList);
                    names->Name     = (IgsYYvsp[0].Name);
                    names->Next     = (IgsYYvsp[-1].Namelist);
                    names->Previous = (IgsYYvsp[-1].Namelist)->Previous;
                    names->Next->Previous = names->Previous->Next = names;
                    (IgsYYval.Namelist) = (IgsYYvsp[-1].Namelist);
                }
#line 5229 "y.tab.c"
    break;

  case 353: /* reviewliterals: %empty  */
#line 2234 "gointer.y"
                {
                    NameList *header;

                    header = mynew(NameList);
                    header->Name = NULL;
                    header->Next = header->Previous = header;
                    (IgsYYval.Namelist) = header;
                }
#line 5242 "y.tab.c"
    break;

  case 354: /* reviewentry: '('  */
#line 2244 "gointer.y"
                  { ReviewOpenVariation(); }
#line 5248 "y.tab.c"
    break;

  case 355: /* reviewentry: ')'  */
#line 2245 "gointer.y"
                  { ReviewCloseVariation(); }
#line 5254 "y.tab.c"
    break;

  case 356: /* reviewentry: REVNODE  */
#line 2247 "gointer.y"
                {
                    ReviewNewNode();
                }
#line 5262 "y.tab.c"
    break;

  case 357: /* reviewentry: REVUNKNOWN reviewliterals  */
#line 2251 "gointer.y"
                {
                    NameList *Here;

                    Outputf("Unknown: %s:", (IgsYYvsp[-1].Name));
                    for (Here = (IgsYYvsp[0].Namelist)->Next; Here != (IgsYYvsp[0].Namelist); Here = Here->Next)
                        Outputf(" %s", Here->Name);
                    Output("\n");
                    myfree((IgsYYvsp[-1].Name));
                    FreeNameList((IgsYYvsp[0].Namelist));
                }
#line 5277 "y.tab.c"
    break;

  case 358: /* reviewentry: REVNODENAME REVLITERAL  */
#line 2262 "gointer.y"
                {
                    NameList Entry;

                    Entry.Name = (IgsYYvsp[0].Name);
                    Entry.Next = Entry.Previous = &Entry;
                    ReviewLocalProperty(retNODENAME, &Entry);
                    myfree((IgsYYvsp[0].Name));
                }
#line 5290 "y.tab.c"
    break;

  case 359: /* reviewentry: REVCOMMENT REVLITERAL  */
#line 2271 "gointer.y"
                {
                    NameList Entry;

                    Entry.Name = (IgsYYvsp[0].Name);
                    Entry.Next = Entry.Previous = &Entry;
                    ReviewLocalProperty(retCOMMENT, &Entry);
                    myfree((IgsYYvsp[0].Name));
                }
#line 5303 "y.tab.c"
    break;

  case 360: /* reviewentry: REVKOMI REVLITERAL  */
#line 2280 "gointer.y"
                {
                    ReviewGlobalProperty(retKOMI, (IgsYYvsp[0].Name));
                    myfree((IgsYYvsp[0].Name));
                }
#line 5312 "y.tab.c"
    break;

  case 361: /* reviewentry: REVHANDICAP REVLITERAL  */
#line 2285 "gointer.y"
                {
                    ReviewGlobalProperty(retHANDICAP, (IgsYYvsp[0].Name));
                    myfree((IgsYYvsp[0].Name));
                }
#line 5321 "y.tab.c"
    break;

  case 362: /* reviewentry: REVUSER REVLITERAL  */
#line 2290 "gointer.y"
                {
                    ReviewGlobalProperty(retENTEREDBY, (IgsYYvsp[0].Name));
                    myfree((IgsYYvsp[0].Name));
                }
#line 5330 "y.tab.c"
    break;

  case 363: /* reviewentry: REVCOPYRIGHT REVLITERAL  */
#line 2295 "gointer.y"
                {
                    ReviewGlobalProperty(retCOPYRIGHT, (IgsYYvsp[0].Name));
                    myfree((IgsYYvsp[0].Name));
                }
#line 5339 "y.tab.c"
    break;

  case 364: /* reviewentry: REVPLACE REVLITERAL  */
#line 2300 "gointer.y"
                {
                    ReviewGlobalProperty(retPLACE, (IgsYYvsp[0].Name));
                    myfree((IgsYYvsp[0].Name));
                }
#line 5348 "y.tab.c"
    break;

  case 365: /* reviewentry: REVDATE REVLITERAL  */
#line 2305 "gointer.y"
                {
                    ReviewGlobalProperty(retDATE, (IgsYYvsp[0].Name));
                    myfree((IgsYYvsp[0].Name));
                }
#line 5357 "y.tab.c"
    break;

  case 366: /* reviewentry: REVRESULT REVLITERAL  */
#line 2310 "gointer.y"
                {
                    ReviewGlobalProperty(retRESULT, (IgsYYvsp[0].Name));
                    myfree((IgsYYvsp[0].Name));
                }
#line 5366 "y.tab.c"
    break;

  case 367: /* reviewentry: REVEVENT REVLITERAL  */
#line 2315 "gointer.y"
                {
                    ReviewGlobalProperty(retTOURNAMENT, (IgsYYvsp[0].Name));
                    myfree((IgsYYvsp[0].Name));
                }
#line 5375 "y.tab.c"
    break;

  case 368: /* reviewentry: REVGAMENAME REVLITERAL  */
#line 2320 "gointer.y"
                {
                    ReviewGlobalProperty(retNAME, (IgsYYvsp[0].Name));
                    myfree((IgsYYvsp[0].Name));
                }
#line 5384 "y.tab.c"
    break;

  case 369: /* reviewentry: REVWHITERANK REVLITERAL  */
#line 2325 "gointer.y"
                {
                    ReviewGlobalProperty(retWHITESTRENGTH, (IgsYYvsp[0].Name));
                    myfree((IgsYYvsp[0].Name));
                }
#line 5393 "y.tab.c"
    break;

  case 370: /* reviewentry: REVBLACKRANK REVLITERAL  */
#line 2330 "gointer.y"
                {
                    ReviewGlobalProperty(retBLACKSTRENGTH, (IgsYYvsp[0].Name));
                    myfree((IgsYYvsp[0].Name));
                }
#line 5402 "y.tab.c"
    break;

  case 371: /* reviewentry: REVWHITENAME REVLITERAL  */
#line 2335 "gointer.y"
                {
                    ReviewGlobalProperty(retWHITENAME, (IgsYYvsp[0].Name));
                    myfree((IgsYYvsp[0].Name));
                }
#line 5411 "y.tab.c"
    break;

  case 372: /* reviewentry: REVBLACKNAME REVLITERAL  */
#line 2340 "gointer.y"
                {
                    ReviewGlobalProperty(retBLACKNAME, (IgsYYvsp[0].Name));
                    myfree((IgsYYvsp[0].Name));
                }
#line 5420 "y.tab.c"
    break;

  case 373: /* reviewentry: REVSIZE REVLITERAL  */
#line 2345 "gointer.y"
                {
                    ReviewGlobalProperty(retSIZE, (IgsYYvsp[0].Name));
                    myfree((IgsYYvsp[0].Name));
                }
#line 5429 "y.tab.c"
    break;

  case 374: /* reviewentry: REVGAME REVLITERAL  */
#line 2350 "gointer.y"
                {
                    ReviewGlobalProperty(retGAME, (IgsYYvsp[0].Name));
                    myfree((IgsYYvsp[0].Name));
                }
#line 5438 "y.tab.c"
    break;

  case 375: /* reviewentry: REVWHITE REVLITERAL  */
#line 2355 "gointer.y"
                {
                    NameList Entry;

                    Entry.Name = (IgsYYvsp[0].Name);
                    Entry.Next = Entry.Previous = &Entry;
                    ReviewLocalProperty(retWHITE, &Entry);
                    myfree((IgsYYvsp[0].Name));
                }
#line 5451 "y.tab.c"
    break;

  case 376: /* reviewentry: REVBLACK REVLITERAL  */
#line 2364 "gointer.y"
                {
                    NameList Entry;

                    Entry.Name = (IgsYYvsp[0].Name);
                    Entry.Next = Entry.Previous = &Entry;
                    ReviewLocalProperty(retBLACK, &Entry);
                    myfree((IgsYYvsp[0].Name));
                }
#line 5464 "y.tab.c"
    break;

  case 377: /* reviewentry: REVLETTERS reviewliterals  */
#line 2373 "gointer.y"
                {
                    ReviewLocalProperty(retLETTERS, (IgsYYvsp[0].Namelist));
                    FreeNameList((IgsYYvsp[0].Namelist));
                }
#line 5473 "y.tab.c"
    break;

  case 378: /* reviewentry: REVWHITETIME REVLITERAL  */
#line 2378 "gointer.y"
                {
                    NameList Entry;

                    Entry.Name = (IgsYYvsp[0].Name);
                    Entry.Next = Entry.Previous = &Entry;
                    ReviewLocalProperty(retWHITETIME, &Entry);
                    myfree((IgsYYvsp[0].Name));
                }
#line 5486 "y.tab.c"
    break;

  case 379: /* reviewentry: REVBLACKTIME REVLITERAL  */
#line 2387 "gointer.y"
                {
                    NameList Entry;

                    Entry.Name = (IgsYYvsp[0].Name);
                    Entry.Next = Entry.Previous = &Entry;
                    ReviewLocalProperty(retBLACKTIME, &Entry);
                    myfree((IgsYYvsp[0].Name));
                }
#line 5499 "y.tab.c"
    break;

  case 380: /* reviewentry: REVADDBLACK reviewliterals  */
#line 2396 "gointer.y"
                {
                    ReviewLocalProperty(retBLACKSET, (IgsYYvsp[0].Namelist));
                    FreeNameList((IgsYYvsp[0].Namelist));
                }
#line 5508 "y.tab.c"
    break;

  case 381: /* reviewentry: REVADDWHITE reviewliterals  */
#line 2401 "gointer.y"
                {
                    ReviewLocalProperty(retWHITESET, (IgsYYvsp[0].Namelist));
                    FreeNameList((IgsYYvsp[0].Namelist));
                }
#line 5517 "y.tab.c"
    break;

  case 382: /* reviewentry: REVADDEMPTY reviewliterals  */
#line 2406 "gointer.y"
                {
                    ReviewLocalProperty(retEMPTYSET, (IgsYYvsp[0].Namelist));
                    FreeNameList((IgsYYvsp[0].Namelist));
                }
#line 5526 "y.tab.c"
    break;

  case 385: /* $@7: %empty  */
#line 2417 "gointer.y"
                {
                    ReviewEntryBegin((IgsYYvsp[0].Value));
                }
#line 5534 "y.tab.c"
    break;

  case 389: /* reviews: auxreviews  */
#line 2428 "gointer.y"
                {
                    ReviewEnd(0);
                }
#line 5542 "y.tab.c"
    break;

  case 390: /* reviews: auxreviews REVIEWEND  */
#line 2432 "gointer.y"
                {
                    ReviewEnd(1);
                }
#line 5550 "y.tab.c"
    break;

  case 391: /* reviewstop: REVIEWSTOP  */
#line 2438 "gointer.y"
                {
                    ReviewStop();
                }
#line 5558 "y.tab.c"
    break;

  case 392: /* noreview: NOREVIEW  */
#line 2444 "gointer.y"
                {
                    ReviewNotFound();
                }
#line 5566 "y.tab.c"
    break;

  case 393: /* throwcopy: THROWCOPY  */
#line 2450 "gointer.y"
                {
                    Output("You are already logged on. "
                           "Throwing other copy out\n");
                }
#line 5575 "y.tab.c"
    break;

  case 394: /* proba: PROBA RATING RATING NAME NAME NAME  */
#line 2457 "gointer.y"
                { /* my rating, their rating, handicap,
                   * proba lose as white, proba lose as black
                   */
                    MyLoseProbas((IgsYYvsp[-5].Person), (IgsYYvsp[-4].Value), (IgsYYvsp[-3].Value), (IgsYYvsp[-2].Name), (IgsYYvsp[-1].Name), (IgsYYvsp[0].Name));
                    myfree((IgsYYvsp[-2].Name));
                    myfree((IgsYYvsp[-1].Name));
                    myfree((IgsYYvsp[0].Name));
                }
#line 5588 "y.tab.c"
    break;

  case 395: /* proba: PROBA END  */
#line 2466 "gointer.y"
                { /* other player does not have a rating */
                }
#line 5595 "y.tab.c"
    break;

  case 396: /* setproba: SETPROBA RATING RATING NAME NAME NAME  */
#line 2471 "gointer.y"
                { /* my rating, their rating, handicap,
                   * proba lose as white, proba lose as black
                   */
                    MyLoseProbas(NULL, (IgsYYvsp[-4].Value), (IgsYYvsp[-3].Value), (IgsYYvsp[-2].Name), (IgsYYvsp[-1].Name), (IgsYYvsp[0].Name));
                    myfree((IgsYYvsp[-2].Name));
                    myfree((IgsYYvsp[-1].Name));
                    myfree((IgsYYvsp[0].Name));
                }
#line 5608 "y.tab.c"
    break;

  case 397: /* setproba: SETPROBA END  */
#line 2480 "gointer.y"
                { /* I do not have a rating */
                }
#line 5615 "y.tab.c"
    break;

  case 398: /* sorry: SORRY  */
#line 2485 "gointer.y"
                {
                    if (ArgsCommand(NULL, ";")) ChannelDisallowed();
                    else Output("Sorry.\n");
                }
#line 5624 "y.tab.c"
    break;

  case 399: /* invalid: INVALID  */
#line 2492 "gointer.y"
                {
                    Outputf("Unknown command %s\n", (IgsYYvsp[0].Name));
                    myfree((IgsYYvsp[0].Name));
                }
#line 5633 "y.tab.c"
    break;

  case 400: /* unknown: UNKNOWNANSWER literallines  */
#line 2499 "gointer.y"
                {
                }
#line 5640 "y.tab.c"
    break;

  case 401: /* names: names NAME  */
#line 2504 "gointer.y"
                {
                    NameList *names;

                    names = mynew(NameList);
                    names->Name     = (IgsYYvsp[0].Name);
                    names->Next     = (IgsYYvsp[-1].Namelist);
                    names->Previous = (IgsYYvsp[-1].Namelist)->Previous;
                    names->Next->Previous = names->Previous->Next = names;
                    (IgsYYval.Namelist) = (IgsYYvsp[-1].Namelist);
                }
#line 5655 "y.tab.c"
    break;

  case 402: /* names: %empty  */
#line 2514 "gointer.y"
                {
                    NameList *header;

                    header = mynew(NameList);
                    header->Name = NULL;
                    header->Next = header->Previous = header;
                    (IgsYYval.Namelist) = header;
                }
#line 5668 "y.tab.c"
    break;

  case 403: /* namesset: namesset names END  */
#line 2525 "gointer.y"
                {
                    (IgsYYvsp[-2].Namelist)->Previous->Next = (IgsYYvsp[-1].Namelist)->Next;
                    (IgsYYvsp[-1].Namelist)->Next->Previous = (IgsYYvsp[-2].Namelist)->Previous;
                    (IgsYYvsp[-1].Namelist)->Previous->Next = (IgsYYvsp[-2].Namelist);
                    (IgsYYvsp[-2].Namelist)->Previous       = (IgsYYvsp[-1].Namelist)->Previous;
                    myfree((IgsYYvsp[-1].Namelist));
                    (IgsYYval.Namelist) = (IgsYYvsp[-2].Namelist);
                }
#line 5681 "y.tab.c"
    break;

  case 404: /* namesset: %empty  */
#line 2533 "gointer.y"
                {
                    NameList *header;

                    header = mynew(NameList);
                    header->Name = NULL;
                    header->Next = header->Previous = header;
                    (IgsYYval.Namelist) = header;
                }
#line 5694 "y.tab.c"
    break;

  case 407: /* promptname: OLDPROMPT  */
#line 2547 "gointer.y"
                              {}
#line 5700 "y.tab.c"
    break;

  case 408: /* promptname: NAME  */
#line 2549 "gointer.y"
                {
                    Outputf("%s\n", (IgsYYvsp[0].Name));
                    myfree((IgsYYvsp[0].Name));
                }
#line 5709 "y.tab.c"
    break;

  case 409: /* optname: NAME  */
#line 2555 "gointer.y"
                   { (IgsYYval.Name) = (IgsYYvsp[0].Name);   }
#line 5715 "y.tab.c"
    break;

  case 410: /* optname: %empty  */
#line 2556 "gointer.y"
                   { (IgsYYval.Name) = NULL; }
#line 5721 "y.tab.c"
    break;

  case 411: /* literallines: literallines NAME  */
#line 2560 "gointer.y"
                {
                    char *ptr;

                    ptr = (IgsYYvsp[0].Name);
                    if (ptr[0] == '\r') ptr++;
                    if (isdigit(ptr[0])) {
                        ptr++;
                        while (isdigit(ptr[0])) ptr++;
                        if (ptr[0] == ' ') ptr++;
                    }
                    Outputf("%s\n", ptr);
                    myfree((IgsYYvsp[0].Name));
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
#line 5752 "y.tab.c"
    break;


#line 5756 "y.tab.c"

      default: break;
    }
  /* User semantic actions sometimes alter IgsYYchar, and that requires
     that IgsYYtoken be updated with the new translation.  We take the
     approach of translating immediately before every use of IgsYYtoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering IgsYYchar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (IgsYYsymbol_kind_t, IgsYYr1[IgsYYn]), &IgsYYval, &IgsYYloc);

  YYPOPSTACK (IgsYYlen);
  IgsYYlen = 0;

  *++IgsYYvsp = IgsYYval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int IgsYYlhs = IgsYYr1[IgsYYn] - YYNTOKENS;
    const int IgsYYi = IgsYYpgoto[IgsYYlhs] + *IgsYYssp;
    IgsYYstate = (0 <= IgsYYi && IgsYYi <= YYLAST && IgsYYcheck[IgsYYi] == *IgsYYssp
               ? IgsYYtable[IgsYYi]
               : IgsYYdefgoto[IgsYYlhs]);
  }

  goto IgsYYnewstate;


/*--------------------------------------.
| IgsYYerrlab -- here on detecting error.  |
`--------------------------------------*/
IgsYYerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  IgsYYtoken = IgsYYchar == YYEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (IgsYYchar);
  /* If not already recovering from an error, report this error.  */
  if (!IgsYYerrstatus)
    {
      ++IgsYYnerrs;
      IgsYYerror (YY_("syntax error"));
    }

  if (IgsYYerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (IgsYYchar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (IgsYYchar == YYEOF)
            YYABORT;
        }
      else
        {
          IgsYYdestruct ("Error: discarding",
                      IgsYYtoken, &IgsYYlval);
          IgsYYchar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto IgsYYerrlab1;


/*---------------------------------------------------.
| IgsYYerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
IgsYYerrorlab:
  /* Pacify compilers when the user code never invokes YYERROR and the
     label IgsYYerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;
  ++IgsYYnerrs;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (IgsYYlen);
  IgsYYlen = 0;
  YY_STACK_PRINT (IgsYYss, IgsYYssp);
  IgsYYstate = *IgsYYssp;
  goto IgsYYerrlab1;


/*-------------------------------------------------------------.
| IgsYYerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
IgsYYerrlab1:
  IgsYYerrstatus = 3;      /* Each real token shifted decrements this.  */

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      IgsYYn = IgsYYpact[IgsYYstate];
      if (!IgsYYpact_value_is_default (IgsYYn))
        {
          IgsYYn += YYSYMBOL_YYerror;
          if (0 <= IgsYYn && IgsYYn <= YYLAST && IgsYYcheck[IgsYYn] == YYSYMBOL_YYerror)
            {
              IgsYYn = IgsYYtable[IgsYYn];
              if (0 < IgsYYn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (IgsYYssp == IgsYYss)
        YYABORT;


      IgsYYdestruct ("Error: popping",
                  YY_ACCESSING_SYMBOL (IgsYYstate), IgsYYvsp);
      YYPOPSTACK (1);
      IgsYYstate = *IgsYYssp;
      YY_STACK_PRINT (IgsYYss, IgsYYssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++IgsYYvsp = IgsYYlval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (IgsYYn), IgsYYvsp, IgsYYlsp);

  IgsYYstate = IgsYYn;
  goto IgsYYnewstate;


/*-------------------------------------.
| IgsYYacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
IgsYYacceptlab:
  IgsYYresult = 0;
  goto IgsYYreturnlab;


/*-----------------------------------.
| IgsYYabortlab -- YYABORT comes here.  |
`-----------------------------------*/
IgsYYabortlab:
  IgsYYresult = 1;
  goto IgsYYreturnlab;


/*-----------------------------------------------------------.
| IgsYYexhaustedlab -- YYNOMEM (memory exhaustion) comes here.  |
`-----------------------------------------------------------*/
IgsYYexhaustedlab:
  IgsYYerror (YY_("memory exhausted"));
  IgsYYresult = 2;
  goto IgsYYreturnlab;


/*----------------------------------------------------------.
| IgsYYreturnlab -- parsing is finished, clean up and return.  |
`----------------------------------------------------------*/
IgsYYreturnlab:
  if (IgsYYchar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      IgsYYtoken = YYTRANSLATE (IgsYYchar);
      IgsYYdestruct ("Cleanup: discarding lookahead",
                  IgsYYtoken, &IgsYYlval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (IgsYYlen);
  YY_STACK_PRINT (IgsYYss, IgsYYssp);
  while (IgsYYssp != IgsYYss)
    {
      IgsYYdestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*IgsYYssp), IgsYYvsp);
      YYPOPSTACK (1);
    }
#ifndef IgsYYoverflow
  if (IgsYYss != IgsYYssa)
    YYSTACK_FREE (IgsYYss);
#endif

  return IgsYYresult;
}

#line 2588 "gointer.y"

/* Kludge in case bison template defined const to nothing */
#ifndef __cplusplus
# ifndef __STDC__
#  undef const
# endif
#endif

NameList *NameListDup(const NameList *Model)
{
    volatile NameList *Header, *Copy;
    NameList *Here;

    Header = mynew(NameList);
    WITH_HANDLING {
        Header->Previous = Header->Next = (NameList *) Header;
        Header->Name = NULL;
        if (Model->Name) Header->Name = mystrdup(Model->Name);
        for (Here = Model->Next; Here != Model; Here = Here->Next) {
            Copy = mynew(NameList);
            Copy->Previous = Header->Previous;
            Copy->Next     = (NameList *) Header;
            Copy->Previous->Next = Copy->Next->Previous = (NameList *) Copy;
            Copy->Name = NULL;
            if (Here->Name) Copy->Name = mystrdup(Here->Name);
        }
    } ON_EXCEPTION {
        FreeNameList((NameList *) Header);
    } END_HANDLING;
    return (NameList *) Header;
}

void FreeNameList(NameList *Header)
{
    NameList *Next;

    for (Header->Previous->Next = NULL; Header; Header = Next) {
        Next = Header->Next;
        myfree(Header->Name);
        myfree(Header);
    }
}

void FreeNumValList(NumVal *Header)
{
    NumVal *Next;

    for (Header->Previous->Next = NULL; Header; Header = Next) {
        Next = Header->Next;
        myfree(Header->Value);
        myfree(Header);
    }
}

void FreeNameValList(NameVal *Header)
{
    NameVal *Next;

    for (Header->Previous->Next = NULL; Header; Header = Next) {
        Next = Header->Next;
        myfree(Header->Name);
        myfree(Header->Value);
        myfree(Header);
    }
}

void FreeNameListList(NameListList *Header)
{
    NameListList *Next;

    for (Header->Previous->Next = NULL; Header; Header = Next) {
        Next = Header->Next;
        if (Header->Names) FreeNameList(Header->Names);
        myfree(Header);
    }
}

void FreeNumNameListList(NumNameListList *Header)
{
    NumNameListList *Next;

    for (Header->Previous->Next = NULL; Header; Header = Next) {
        Next = Header->Next;
        if (Header->Value) FreeNameList(Header->Value);
        myfree(Header);
    }
}

/*
static int WhoseMove(NameVal *moves)
{
    if (moves == moves->Next || atoi(moves->Previous->Name) % 2 ) return Black;
    return White;
}
*/

void FreeGameDesc(GameDesc *Desc)
{
    myfree(Desc->BlackName);
    myfree(Desc->WhiteName);
    myfree(Desc->BlackName2);
    myfree(Desc->WhiteName2);
    myfree(Desc);
}

static void ReceivedKibitz(Player *player, int Id,
                           const char *black, const char *white,
                           const char *kibitz, size_t Length)
{
    Game *game;

    if (player && (game = IdPlayersToGame(Id, black, white)) != NULL)
        Kibitz(game, player, kibitz, Length);
}

static void OverObserve(int MaxGames)
{
    AutoCommand(NULL, "watching");
    AddText(gameinfo, "Observing too many, maximum of %d games\n", MaxGames);
    XBell(XtDisplay(gameinfo), 20);
}

static void EnterString(XtPointer Closure)
{
    ForceCommand(NULL, *(char **) Closure);
}

static void PlayerPasses(const char *Name)
{
    Widget Main, stats;
    char  *title;

    Me   = PlayerFromName(Name);
    Name = PlayerToName(Me);
    RejoinChannel();
    /* FirstCommand(NULL, "review"); */
    if (appdata.GamesTimeout > 0) FirstCommand(NULL, "games");
    /* Will cause "games" due to nrgames inconsistency */
    if (appdata.WhoTimeout > 0) FirstCommand(NULL, "who");
    /* So who comes BEFORE games (inversion made by FirstCommand) */
    /* FirstCommand(NULL, "toggle bell on"); */
    FirstCommand(NULL, "toggle quiet off");
    if (appdata.GamesTimeout > 0 && appdata.WhoTimeout > 0) {
#if 0
	LastCommand(NULL, "id xgospel %s", VERSION);
#endif
        LastCommand(NULL, "uptime");
	/* uptime must come *after* to run Entering with Me defined */
    }
    EnterServer(Me);
    Main  = XtNameToWidget(toplevel, "*main");
    if (Main) {
        stats = XtNameToWidget(Main, "*statsMe");
        if (stats) XtDestroyWidget(stats);
        stats = MyVaCreateManagedWidget("statsMe", WitchetOfWidget(Main),
                                        NULL);
        XtAddCallback(stats, XtNcallback, CallGetStats, (XtPointer) Me);
        XtVaGetValues(stats, XtNlabel, (XtArgVal) &title, NULL);
        title = PlayerTemplateDescription(Me, title);
        XtVaSetValues(stats, XtNlabel, (XtArgVal) title, NULL);
        myfree(title);
        stats = XtNameToWidget(Main, "*commandMenu");
        if (stats) {
            /* FIXME: I'm programming around a X bug here. A popup that's not
               child of a composite gets an unmanage ateempt even if not
               managed. That is not according to the Xt specs (-Ton) */
            Widget Temp;

            Temp = XtParent(stats);
            XtParent(stats) = NULL;
            XtUnrealizeWidget(stats);
            XtParent(stats) = Temp;
            XtRealizeWidget(stats);
        }
    }
    /* Run Entering() when receiving output of "uptime" or now. Entering
     * is normally delayed to avoid many beeps initially when beep on output
     * is set on the main window.
     */
    if (appdata.GamesTimeout == 0 || appdata.WhoTimeout == 0) {
        Entering();
    }
}

static void Entering(void)
{
    /* Hook for when player is fully logged on */
    GetExactRating(Me);
    Entered = 1;
    UserCommands(); /* run .xgospelrc */
}
/*----------*/

static void MyOverflow(const char *Text)
{
    Raise1(FatalException, Text);
}

static void IgsYYerror(const char *s)
{
    char *ptr1, *ptr2;

    if (ConnectedP(NULL)) {
        ptr1 = mystrdup(_GoText());
        WITH_UNWIND {
            ptr2 = mystrdup(Parsing(NULL));
            Warning("%s.\nFound ``%s''%s..:\nSomewhere in\n----------\n%s"
                    "\n----------\n", s, ptr1, _FormatError(), ptr2);
            myfree(ptr2);
        } ON_UNWIND {
            myfree(ptr1);
        } END_UNWIND;
    }
}

#ifndef HAVE_NO_STDARG_H
void Warning(const char *Comment, ...)
#else  /* HAVE_NO_STDARG_H */
void Warning(va_alist)
va_dcl
#endif /* HAVE_NO_STDARG_H */
{
    char    Text[2048];
    va_list args;

#ifndef HAVE_NO_STDARG_H
    va_start(args, Comment);
#else  /* HAVE_NO_STDARG_H */
    const char *Comment;

    va_start(args);
    Comment = va_arg(args, const char *);
#endif /* HAVE_NO_STDARG_H */
    strcpy(Text, "Warning: ");
    vsprintf(strchr(Text, 0), Comment, args);
    va_end(args);
/*
    fflush(stdout);
    fputs(Text, stderr);
    fputc('\n', stderr);
    fflush(stderr);
*/
    Output(Text);
    if (DebugFile) {
        fprintf(DebugFile, "* ( ) %s", Text);
        fflush(DebugFile);
    }
    if (appdata.WantStdout != False) fputs(Text, stdout);
}
