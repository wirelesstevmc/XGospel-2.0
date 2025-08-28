/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison interface for Yacc-like parsers in C

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

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or IgsYY_.  They are
   private implementation details that can be changed or removed.  */

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

#line 459 "y.tab.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE IgsYYlval;


int IgsYYparse (void);


#endif /* !YY_YY_Y_TAB_H_INCLUDED  */
