
/*  A Bison parser, made from ./gointer.y
 by  GNU Bison version 1.25
  */

#define YYBISON 1  /* Identify Bison output.  */

#define	NAME	258
#define	SERVERMESSAGE	259
#define	STATSENTRY	260
#define	ILLEGALMOVE	261
#define	ILLEGALUNDO	262
#define	REQUESTINGMATCH	263
#define	REMOVEGAMEFILE	264
#define	MAILED	265
#define	REMOVEGROUP	266
#define	GIVEBYOYOMI	267
#define	RESULTLINE	268
#define	INVALID	269
#define	AUTOMATCHDISPUTE	270
#define	NEWCHANNEL	271
#define	MUSTPASS	272
#define	OPPMUSTPASS	273
#define	GUEST	274
#define	TELLDONE	275
#define	REVIEWSTART	276
#define	REVLITERAL	277
#define	REVUNKNOWN	278
#define	WELCOME	279
#define	SERVERFULL	280
#define	XSHOUT2	281
#define	MYBET	282
#define	YELL	283
#define	TELL	284
#define	RESIGN	285
#define	KOMIREQUEST	286
#define	DISPUTEMATCHTYPE	287
#define	XSHOUT	288
#define	DECLINE	289
#define	JOIN	290
#define	LEAVE	291
#define	NEWTITLE	292
#define	BROADCAST	293
#define	ITBROADCAST	294
#define	ENTERBYOYOMI	295
#define	NOTIME	296
#define	PERSON	297
#define	BEEPING	298
#define	PLAYERON	299
#define	PROBA	300
#define	STORED	301
#define	IDLE	302
#define	PROMPT	303
#define	GAMES	304
#define	REMOVE	305
#define	MOVE	306
#define	GAME	307
#define	OVEROBSERVE	308
#define	MESSAGES	309
#define	NEWMATCH	310
#define	STATUSLINE	311
#define	CHANNEL	312
#define	CHANGECHANNEL	313
#define	FREE	314
#define	TEXTFILE	315
#define	FIRSTREMOVE	316
#define	REVIEWTYPE	317
#define	GAMECOLOR	318
#define	GAMESECONDS	319
#define	BYOYOMI	320
#define	MATCHTYPE	321
#define	NATURAL	322
#define	BETRESULT	323
#define	RATING	324
#define	STOREDNUM	325
#define	UNDO	326
#define	END	327
#define	FAIL	328
#define	OLDPROMPT	329
#define	SEMIPROMPT	330
#define	INFOMESSAGE	331
#define	LUSER	332
#define	OLDPASSWORD	333
#define	PASSWORD	334
#define	INVALIDPASSWORD	335
#define	IGSENTRY	336
#define	TITLESET	337
#define	TOGGLE	338
#define	PLAYERS	339
#define	UNKNOWNANSWER	340
#define	MATCHCLOSED	341
#define	MATCHOPEN	342
#define	OBSERVE	343
#define	WATCHING	344
#define	EXTSTATSENTRY	345
#define	ADD	346
#define	KIBITZ	347
#define	KOMISET	348
#define	TRANSLATION	349
#define	GAMETIME	350
#define	LOSTCONNECTION	351
#define	MYADJOURN	352
#define	RESTORE	353
#define	RESTART	354
#define	NOTURN	355
#define	GAMESAVED	356
#define	UNDID	357
#define	EMPTY	358
#define	DONE	359
#define	RESTORESCORING	360
#define	STATUSHEADER	361
#define	REMOVELIBERTY	362
#define	OBSERVEWHILEPLAY	363
#define	NOTELLTARGET	364
#define	GMTTIME	365
#define	LOCALTIME	366
#define	SERVERUP	367
#define	UPTIMEENTRY	368
#define	THROWCOPY	369
#define	SORRY	370
#define	WRONGCHANNEL	371
#define	AUTOMATCHREQUEST	372
#define	DISPUTE	373
#define	OPPONENTDISPUTE	374
#define	LATEFREE	375
#define	NOPLAY	376
#define	CHANNELHEADER	377
#define	OBSERVERS	378
#define	GAMENOTFOUND	379
#define	MATCHREQUEST	380
#define	GOEMATCHREQUEST	381
#define	TOURNAMENTMATCHREQUEST	382
#define	TOURNAMENTGOEMATCHREQUEST	383
#define	USERESIGN	384
#define	GAMETITLE	385
#define	ERASE	386
#define	PLEASEREDONE	387
#define	TELLTARGET	388
#define	TELLOFF	389
#define	NOREMOVETURN	390
#define	ADJOURNSENTREQUEST	391
#define	ADJOURNREQUEST	392
#define	OPPONENTNOTON	393
#define	NOLOAD	394
#define	DISAGREEREMOVE	395
#define	OPPDISAGREEREMOVE	396
#define	DECLINEADJOURN	397
#define	REVIEWLIST	398
#define	REVIEWSTOP	399
#define	REVNODE	400
#define	REVCOMMENT	401
#define	REVEVENT	402
#define	REVRESULT	403
#define	REVPLACE	404
#define	REVUSER	405
#define	REVDATE	406
#define	REVKOMI	407
#define	REVGAMENAME	408
#define	REVWHITERANK	409
#define	REVBLACKRANK	410
#define	REVWHITENAME	411
#define	REVBLACKNAME	412
#define	REVSIZE	413
#define	REVGAME	414
#define	REVBLACK	415
#define	REVWHITE	416
#define	REVADDBLACK	417
#define	REVADDWHITE	418
#define	REVADDEMPTY	419
#define	REVNODENAME	420
#define	REVIEWEND	421
#define	REVBLACKTIME	422
#define	REVWHITETIME	423
#define	REVCOPYRIGHT	424
#define	REVHANDICAP	425
#define	REVLETTERS	426
#define	REVIEWVARIATIONS	427
#define	NOREVIEW	428
#define	SGFLIST	429
#define	NOSGF	430
#define	NOMOREMOVES	431
#define	BETWINNERS	432
#define	BETEVEN	433
#define	BETLOSERS	434
#define	USER	435
#define	CURRENTSCORE	436
#define	FINALSCORE	437
#define	TEAMGAME	438
#define	OBSERVETEAM	439
#define	RESTARTTEAMGAME	440
#define	SETPROBA	441
#define	NOTREVIEWING	442
#define	NOTREQUESTGAME	443

#line 1 "./gointer.y"

#include <stdlib.h>
#include <string.h>
#include <time.h>

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

static int Passed, eEmpty, PreEmpty, SeenAdd, gamesSeen;

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


#line 67 "./gointer.y"
typedef union {
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
} YYSTYPE;
#include <stdio.h>

#ifndef __cplusplus
#ifndef __STDC__
#define const
#endif
#endif



#define	YYFINAL		776
#define	YYFLAG		-32768
#define	YYNTBASE	198

#define YYTRANSLATE(x) ((unsigned)(x) <= 443 ? IgsYYtranslate[x] : 384)

static const short IgsYYtranslate[] = {     0,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,   196,
   197,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,   192,     2,   195,
     2,   194,     2,   193,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
   189,     2,   190,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,   191,     2,     2,     2,     2,     2,
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
     2,     2,     2,     2,     2,     1,     2,     3,     4,     5,
     6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
    16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
    26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
    36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
    46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
    56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
    66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
    76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
    86,    87,    88,    89,    90,    91,    92,    93,    94,    95,
    96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
   106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
   116,   117,   118,   119,   120,   121,   122,   123,   124,   125,
   126,   127,   128,   129,   130,   131,   132,   133,   134,   135,
   136,   137,   138,   139,   140,   141,   142,   143,   144,   145,
   146,   147,   148,   149,   150,   151,   152,   153,   154,   155,
   156,   157,   158,   159,   160,   161,   162,   163,   164,   165,
   166,   167,   168,   169,   170,   171,   172,   173,   174,   175,
   176,   177,   178,   179,   180,   181,   182,   183,   184,   185,
   186,   187,   188
};

#if YYDEBUG != 0
static const short IgsYYprhs[] = {     0,
     0,     2,     4,    10,    11,    13,    14,    19,    20,    25,
    26,    31,    32,    35,    36,    38,    40,    42,    44,    47,
    48,    50,    52,    55,    57,    59,    61,    63,    65,    67,
    69,    71,    73,    75,    77,    79,    81,    83,    85,    87,
    89,    91,    93,    95,    97,    99,   101,   103,   105,   107,
   109,   111,   113,   115,   117,   119,   121,   123,   125,   127,
   129,   131,   133,   135,   137,   139,   141,   143,   145,   147,
   149,   151,   153,   155,   157,   159,   161,   163,   165,   167,
   169,   171,   173,   175,   177,   179,   181,   183,   185,   187,
   189,   191,   193,   195,   197,   199,   201,   203,   205,   207,
   209,   211,   213,   215,   217,   219,   221,   223,   225,   227,
   229,   231,   233,   235,   237,   239,   241,   243,   245,   247,
   249,   251,   253,   255,   257,   259,   261,   263,   265,   267,
   269,   271,   273,   275,   277,   279,   281,   283,   285,   287,
   289,   290,   294,   296,   299,   302,   304,   307,   310,   313,
   316,   326,   333,   343,   355,   368,   380,   383,   388,   391,
   394,   396,   399,   402,   405,   408,   423,   436,   438,   442,
   445,   448,   452,   454,   457,   459,   461,   465,   468,   470,
   472,   474,   476,   487,   489,   492,   495,   498,   500,   503,
   505,   508,   510,   513,   516,   518,   522,   525,   526,   529,
   534,   537,   538,   540,   541,   549,   557,   562,   565,   567,
   569,   571,   582,   584,   586,   588,   606,   609,   610,   611,
   615,   617,   620,   623,   624,   626,   627,   643,   665,   668,
   669,   671,   674,   676,   678,   680,   682,   684,   685,   691,
   703,   705,   706,   711,   717,   723,   729,   732,   735,   739,
   742,   748,   760,   767,   780,   782,   787,   798,   803,   814,
   817,   820,   822,   829,   832,   833,   836,   839,   842,   843,
   847,   849,   850,   857,   862,   868,   880,   884,   886,   888,
   895,   899,   902,   903,   915,   923,   924,   925,   932,   935,
   938,   941,   942,   945,   950,   954,   959,   960,   984,   989,
   992,   995,   997,  1000,  1002,  1006,  1010,  1012,  1013,  1019,
  1021,  1023,  1025,  1027,  1029,  1032,  1035,  1037,  1039,  1041,
  1045,  1047,  1050,  1052,  1056,  1058,  1060,  1062,  1064,  1066,
  1068,  1070,  1072,  1076,  1079,  1082,  1083,  1085,  1089,  1099,
  1103,  1107,  1116,  1119,  1122,  1124,  1128,  1131,  1134,  1137,
  1139,  1142,  1143,  1145,  1147,  1149,  1152,  1155,  1158,  1161,
  1164,  1167,  1170,  1173,  1176,  1179,  1182,  1185,  1188,  1191,
  1194,  1197,  1200,  1203,  1206,  1209,  1212,  1215,  1218,  1221,
  1224,  1227,  1230,  1231,  1232,  1237,  1240,  1242,  1244,  1247,
  1249,  1251,  1253,  1260,  1263,  1270,  1273,  1275,  1277,  1280,
  1283,  1284,  1288,  1289,  1292,  1293,  1295,  1297,  1299,  1300,
  1303
};

static const short IgsYYrhs[] = {   199,
     0,     1,     0,   199,   205,   200,   207,    80,     0,     0,
    79,     0,     0,    78,   201,   205,   203,     0,     0,    19,
   202,   205,   203,     0,     0,    74,   204,   380,    72,     0,
     0,   205,   206,     0,     0,     3,     0,    24,     0,    77,
     0,    25,     0,   207,   209,     0,     0,    48,     0,    75,
     0,   210,   208,     0,   213,     0,   214,     0,   216,     0,
   217,     0,   220,     0,   221,     0,   222,     0,   218,     0,
   223,     0,   224,     0,   225,     0,   226,     0,   227,     0,
   228,     0,   229,     0,   230,     0,   231,     0,   232,     0,
   233,     0,   234,     0,   235,     0,   237,     0,   238,     0,
   239,     0,   240,     0,   241,     0,   242,     0,   243,     0,
   244,     0,   245,     0,   246,     0,   251,     0,   255,     0,
   211,     0,   212,     0,   256,     0,   259,     0,   261,     0,
   262,     0,   263,     0,   264,     0,   267,     0,   269,     0,
   275,     0,   276,     0,   278,     0,   277,     0,   279,     0,
   280,     0,   282,     0,   284,     0,   285,     0,   287,     0,
   291,     0,   292,     0,   293,     0,   294,     0,   295,     0,
   296,     0,   299,     0,   300,     0,   302,     0,   289,     0,
   290,     0,   305,     0,   306,     0,   307,     0,   308,     0,
   317,     0,   312,     0,   321,     0,   322,     0,   324,     0,
   325,     0,   326,     0,   327,     0,   328,     0,   330,     0,
   331,     0,   332,     0,   333,     0,   334,     0,   335,     0,
   219,     0,   337,     0,   336,     0,   338,     0,   339,     0,
   340,     0,   341,     0,   342,     0,   343,     0,   344,     0,
   345,     0,   346,     0,   347,     0,   348,     0,   349,     0,
   350,     0,   354,     0,   355,     0,   358,     0,   359,     0,
   360,     0,   361,     0,   362,     0,   369,     0,   370,     0,
   371,     0,   372,     0,   373,     0,   374,     0,   103,     0,
   375,     0,   376,     0,   377,     0,     1,     0,     0,    60,
   378,    72,     0,   131,     0,    81,     3,     0,   214,   215,
     0,   215,     0,   103,   215,     0,     4,     3,     0,    33,
     3,     0,    26,     3,     0,    76,     3,   189,     3,   190,
     3,     3,   191,    72,     0,    76,     3,     3,     3,   191,
    72,     0,    76,     3,     3,   192,   318,     3,   318,   191,
    72,     0,    76,     3,     3,   192,     3,     3,     3,   192,
   378,   191,    72,     0,    76,     3,     3,   192,     3,     3,
     3,   193,     3,     3,   191,    72,     0,    76,     3,     3,
   192,     3,     3,     3,     3,     3,   191,    72,     0,    29,
     3,     0,    88,    75,    29,     3,     0,    44,   303,     0,
    88,    43,     0,    43,     0,    47,     3,     0,    46,    70,
     0,    38,     3,     0,    39,     3,     0,    88,    75,    92,
   318,   192,     3,     3,     3,     3,   189,     3,   190,    72,
     3,     0,    92,   318,   192,     3,     3,     3,     3,   189,
     3,   190,    72,     3,     0,    54,     0,    57,    28,     3,
     0,    57,    35,     0,    57,    36,     0,    57,    37,     3,
     0,    58,     0,   116,     3,     0,    87,     0,    86,     0,
   117,   378,    72,     0,    15,   378,     0,   125,     0,   126,
     0,   127,     0,   128,     0,   236,   378,   194,     3,   195,
   378,   194,   378,    72,   303,     0,     8,     0,    31,     3,
     0,    93,     3,     0,    59,    88,     0,    59,     0,   120,
     3,     0,   121,     0,   138,   139,     0,    82,     0,     5,
     3,     0,   248,   247,     0,   247,     0,    90,   378,    72,
     0,   249,   249,     0,     0,   248,   250,     0,    42,    67,
   192,    67,     0,   253,   252,     0,     0,    27,     0,     0,
   177,   253,   178,   253,   179,   253,   254,     0,    83,     3,
     3,     3,     3,   382,    72,     0,    16,   378,    72,   379,
     0,   258,   257,     0,   257,     0,   258,     0,   379,     0,
   123,     3,   196,     3,     3,     3,   197,   192,    72,   260,
     0,   124,     0,   176,     0,   188,     0,    49,   318,     3,
   318,   196,     3,     3,     3,     3,     3,     3,   378,   197,
   196,     3,   197,    72,     0,   266,   265,     0,     0,     0,
    49,   268,   266,     0,    50,     0,    51,     3,     0,   271,
   270,     0,     0,   328,     0,     0,    52,     3,   196,     3,
     3,     3,   197,     3,     3,   196,     3,     3,     3,   197,
    72,     0,    52,     3,   196,     3,     3,     3,   197,     3,
     3,   196,     3,     3,     3,   197,    72,   183,     3,     3,
     3,     3,    72,     0,   130,     3,     0,     0,    91,     0,
   104,   282,     0,    17,     0,    18,     0,   140,     0,   141,
     0,   187,     0,     0,   273,   271,   274,   272,   281,     0,
   273,   271,   184,     3,     3,     3,     3,    72,   303,   274,
   272,     0,    61,     0,     0,   104,   283,    75,   286,     0,
   273,   271,   274,    88,   272,     0,   273,   271,   274,    88,
   272,     0,   273,   271,   274,    75,   272,     0,    68,   285,
     0,    68,   282,     0,   288,   102,     3,     0,   102,     3,
     0,   288,   273,   271,   274,   272,     0,   288,   273,   271,
   184,     3,     3,     3,     3,    72,   274,   272,     0,   102,
     3,    88,   273,   271,   272,     0,   102,     3,    88,   273,
   271,   184,     3,     3,     3,     3,    72,   272,     0,    98,
     0,    99,   273,   271,    88,     0,    99,   273,   271,   185,
     3,     3,     3,     3,    72,    88,     0,    99,   273,   271,
   274,     0,    99,   273,   271,   185,     3,     3,     3,     3,
    72,   274,     0,    88,   295,     0,   273,    55,     0,    34,
     0,    42,    63,     3,    64,    65,    72,     0,   298,   297,
     0,     0,   119,   298,     0,   118,   298,     0,   301,    66,
     0,     0,    32,   301,    72,     0,    88,     0,     0,   304,
   303,    71,     3,     3,     3,     0,    71,     3,     3,     3,
     0,   304,   303,   273,   271,   274,     0,   304,   303,   273,
   271,   184,     3,     3,     3,     3,    72,   274,     0,    89,
   378,    72,     0,    53,     0,   108,     0,    84,     3,     3,
     3,     3,    72,     0,    84,     3,    72,     0,   310,   309,
     0,     0,     3,   196,     3,   197,     3,     3,     3,     3,
     3,     3,    72,     0,     3,     3,     3,     3,     3,     3,
    72,     0,     0,     0,    84,   313,   310,    72,   314,   311,
     0,   378,    72,     0,   378,    73,     0,   316,   315,     0,
     0,   180,   316,     0,     3,   189,     3,   190,     0,     3,
   192,     3,     0,   196,     3,   197,     3,     0,     0,    95,
     3,   192,     3,    72,    95,     3,   196,     3,   197,   192,
   319,   320,    72,    95,     3,   196,     3,   197,   192,   319,
   320,    72,     0,   181,     3,   182,     3,     0,    94,     3,
     0,   324,   323,     0,   323,     0,    40,    12,     0,    41,
     0,    96,    97,   272,     0,   101,     3,   303,     0,    97,
     0,     0,    97,   329,   101,     3,   272,     0,   136,     0,
   137,     0,    97,     0,   142,     0,    30,     0,    30,    30,
     0,    10,    10,     0,    10,     0,     9,     0,   109,     0,
   133,     3,    72,     0,    20,     0,   134,   378,     0,     6,
     0,     6,    88,   303,     0,     7,     0,   100,     0,   135,
     0,   129,     0,   107,     0,    11,     0,   105,     0,   132,
     0,   106,   378,    72,     0,    56,     3,     0,   353,   352,
     0,     0,    13,     0,   351,   351,   353,     0,     3,     3,
     3,     3,   192,     3,   192,     3,     3,     0,   110,   356,
    72,     0,   111,   356,    72,     0,   112,     3,     3,     3,
     3,     3,     3,    72,     0,   113,     3,     0,   358,   357,
     0,   357,     0,   174,   378,    72,     0,   174,   175,     0,
   143,   378,     0,   172,   378,     0,    21,     0,   363,    22,
     0,     0,   196,     0,   197,     0,   145,     0,    23,   363,
     0,   165,    22,     0,   146,    22,     0,   152,    22,     0,
   170,    22,     0,   150,    22,     0,   169,    22,     0,   149,
    22,     0,   151,    22,     0,   148,    22,     0,   147,    22,
     0,   153,    22,     0,   154,    22,     0,   155,    22,     0,
   156,    22,     0,   157,    22,     0,   158,    22,     0,   159,
    22,     0,   161,    22,     0,   160,    22,     0,   171,   363,
     0,   168,    22,     0,   167,    22,     0,   162,   363,     0,
   163,   363,     0,   164,   363,     0,   365,   364,     0,     0,
     0,    62,   367,   365,    72,     0,   368,   366,     0,   366,
     0,   368,     0,   368,   166,     0,   144,     0,   173,     0,
   114,     0,    45,    69,    69,     3,     3,     3,     0,    45,
    72,     0,   186,    69,    69,     3,     3,     3,     0,   186,
    72,     0,   115,     0,    14,     0,    85,   383,     0,   378,
     3,     0,     0,   379,   378,    72,     0,     0,   380,   381,
     0,     0,    74,     0,     3,     0,     3,     0,     0,   383,
     3,     0,     0
};

#endif

#if YYDEBUG != 0
static const short IgsYYrline[] = { 0,
   137,   138,   141,   155,   161,   168,   175,   176,   181,   189,
   197,   198,   201,   202,   205,   210,   216,   222,   230,   231,
   234,   240,   247,   260,   261,   262,   263,   264,   265,   266,
   267,   268,   269,   270,   271,   272,   273,   274,   275,   276,
   277,   278,   279,   280,   281,   282,   283,   284,   285,   286,
   287,   288,   289,   290,   291,   292,   293,   294,   295,   296,
   297,   298,   299,   300,   301,   302,   303,   304,   305,   306,
   307,   308,   309,   310,   311,   312,   313,   314,   315,   316,
   317,   318,   319,   320,   321,   322,   323,   324,   325,   326,
   327,   328,   329,   330,   331,   332,   333,   334,   335,   336,
   337,   338,   339,   340,   341,   342,   343,   344,   345,   346,
   347,   348,   349,   350,   351,   352,   353,   354,   355,   356,
   357,   358,   359,   360,   361,   367,   368,   369,   370,   371,
   372,   373,   374,   375,   376,   377,   378,   379,   380,   381,
   386,   389,   412,   418,   435,   436,   437,   440,   447,   452,
   461,   472,   482,   491,   502,   516,   532,   537,   544,   551,
   555,   561,   568,   574,   579,   586,   598,   612,   618,   625,
   631,   637,   644,   650,   657,   663,   669,   676,   684,   686,
   687,   688,   691,   703,   710,   723,   736,   743,   750,   758,
   764,   770,   781,   792,   799,   802,   805,   831,   842,   855,
   864,   869,   875,   876,   879,   900,   912,   920,   933,   948,
   955,   957,   971,   988,   997,  1006,  1035,  1036,  1039,  1044,
  1049,  1052,  1069,  1076,  1087,  1088,  1091,  1117,  1152,  1153,
  1156,  1159,  1165,  1172,  1179,  1190,  1199,  1205,  1208,  1229,
  1252,  1253,  1292,  1301,  1313,  1323,  1335,  1339,  1345,  1351,
  1358,  1366,  1376,  1389,  1404,  1407,  1415,  1432,  1444,  1461,
  1464,  1474,  1480,  1493,  1500,  1507,  1520,  1533,  1537,  1543,
  1549,  1552,  1556,  1563,  1572,  1579,  1589,  1596,  1602,  1608,
  1616,  1623,  1624,  1630,  1642,  1654,  1658,  1663,  1665,  1691,
  1708,  1720,  1728,  1735,  1743,  1755,  1761,  1764,  1780,  1801,
  1808,  1809,  1812,  1820,  1826,  1832,  1841,  1844,  1848,  1856,
  1861,  1869,  1875,  1881,  1886,  1893,  1900,  1907,  1914,  1920,
  1936,  1941,  1952,  1957,  1964,  1971,  1977,  1983,  1989,  1995,
  2002,  2008,  2014,  2020,  2028,  2035,  2044,  2052,  2061,  2090,
  2102,  2116,  2132,  2141,  2142,  2145,  2150,  2156,  2163,  2177,
  2184,  2195,  2206,  2207,  2208,  2212,  2223,  2232,  2241,  2246,
  2251,  2256,  2261,  2266,  2271,  2276,  2281,  2286,  2291,  2296,
  2301,  2306,  2311,  2316,  2325,  2334,  2339,  2348,  2357,  2362,
  2367,  2374,  2375,  2378,  2382,  2385,  2386,  2389,  2393,  2399,
  2405,  2411,  2418,  2427,  2432,  2441,  2446,  2453,  2460,  2465,
  2476,  2486,  2495,  2505,  2506,  2509,  2510,  2517,  2518,  2521,
  2548
};
#endif


#if YYDEBUG != 0 || defined (YYERROR_VERBOSE)

static const char * const IgsYYtname[] = {   "$","error","$undefined.","NAME","SERVERMESSAGE",
"STATSENTRY","ILLEGALMOVE","ILLEGALUNDO","REQUESTINGMATCH","REMOVEGAMEFILE",
"MAILED","REMOVEGROUP","GIVEBYOYOMI","RESULTLINE","INVALID","AUTOMATCHDISPUTE",
"NEWCHANNEL","MUSTPASS","OPPMUSTPASS","GUEST","TELLDONE","REVIEWSTART","REVLITERAL",
"REVUNKNOWN","WELCOME","SERVERFULL","XSHOUT2","MYBET","YELL","TELL","RESIGN",
"KOMIREQUEST","DISPUTEMATCHTYPE","XSHOUT","DECLINE","JOIN","LEAVE","NEWTITLE",
"BROADCAST","ITBROADCAST","ENTERBYOYOMI","NOTIME","PERSON","BEEPING","PLAYERON",
"PROBA","STORED","IDLE","PROMPT","GAMES","REMOVE","MOVE","GAME","OVEROBSERVE",
"MESSAGES","NEWMATCH","STATUSLINE","CHANNEL","CHANGECHANNEL","FREE","TEXTFILE",
"FIRSTREMOVE","REVIEWTYPE","GAMECOLOR","GAMESECONDS","BYOYOMI","MATCHTYPE","NATURAL",
"BETRESULT","RATING","STOREDNUM","UNDO","END","FAIL","OLDPROMPT","SEMIPROMPT",
"INFOMESSAGE","LUSER","OLDPASSWORD","PASSWORD","INVALIDPASSWORD","IGSENTRY",
"TITLESET","TOGGLE","PLAYERS","UNKNOWNANSWER","MATCHCLOSED","MATCHOPEN","OBSERVE",
"WATCHING","EXTSTATSENTRY","ADD","KIBITZ","KOMISET","TRANSLATION","GAMETIME",
"LOSTCONNECTION","MYADJOURN","RESTORE","RESTART","NOTURN","GAMESAVED","UNDID",
"EMPTY","DONE","RESTORESCORING","STATUSHEADER","REMOVELIBERTY","OBSERVEWHILEPLAY",
"NOTELLTARGET","GMTTIME","LOCALTIME","SERVERUP","UPTIMEENTRY","THROWCOPY","SORRY",
"WRONGCHANNEL","AUTOMATCHREQUEST","DISPUTE","OPPONENTDISPUTE","LATEFREE","NOPLAY",
"CHANNELHEADER","OBSERVERS","GAMENOTFOUND","MATCHREQUEST","GOEMATCHREQUEST",
"TOURNAMENTMATCHREQUEST","TOURNAMENTGOEMATCHREQUEST","USERESIGN","GAMETITLE",
"ERASE","PLEASEREDONE","TELLTARGET","TELLOFF","NOREMOVETURN","ADJOURNSENTREQUEST",
"ADJOURNREQUEST","OPPONENTNOTON","NOLOAD","DISAGREEREMOVE","OPPDISAGREEREMOVE",
"DECLINEADJOURN","REVIEWLIST","REVIEWSTOP","REVNODE","REVCOMMENT","REVEVENT",
"REVRESULT","REVPLACE","REVUSER","REVDATE","REVKOMI","REVGAMENAME","REVWHITERANK",
"REVBLACKRANK","REVWHITENAME","REVBLACKNAME","REVSIZE","REVGAME","REVBLACK",
"REVWHITE","REVADDBLACK","REVADDWHITE","REVADDEMPTY","REVNODENAME","REVIEWEND",
"REVBLACKTIME","REVWHITETIME","REVCOPYRIGHT","REVHANDICAP","REVLETTERS","REVIEWVARIATIONS",
"NOREVIEW","SGFLIST","NOSGF","NOMOREMOVES","BETWINNERS","BETEVEN","BETLOSERS",
"USER","CURRENTSCORE","FINALSCORE","TEAMGAME","OBSERVETEAM","RESTARTTEAMGAME",
"SETPROBA","NOTREVIEWING","NOTREQUESTGAME","'['","']'","'}'","':'","'@'","'>'",
"'<'","'('","')'","start","session","pass","@1","@2","enterorfail","@3","loginmessages",
"loginmessage","inputs","prompt","moreinput","input","textfile","erase","igsentry",
"servermessages","servermessage","xshout","infomessage","tell","playeron","beeping",
"idle","stored","broadcast","kibitz","messages","yell","join","leave","newtitle",
"changechannel","wrongchannel","matchopen","matchclosed","automatchrequest",
"automatchdispute","ruledmatchrequest","matchrequest","requestingmatch","komirequest",
"komiset","freemessage","freeconfirm","latefree","noplay","noload","titleset",
"statsentry","statsentries","extendstatsentry","optextend","stats","betentry",
"betentries","optmybet","bet","toggle","channelentry","channelentries","channels",
"observerentries","observers","gamenotfound","nomoremoves","notrequestgame",
"gamesline","gameslines","games","@4","remove","move","movelist","optgamesaved",
"gamedesc","optgametitle","add","doneobserve","mustpass","oppmustpass","disagreeremove",
"opponentdisagreeremove","optnotreviewing","observe","optfirst","doneopponentobserve",
"opponentobserve","opponentoptobserve","betresult","undidlist","undid","opponentundid",
"restore","opponentrestart","restart","newmatch1","newmatch2","decline","disputeline",
"disputelines","opponentdispute","dispute","matchtypes","disputematchtype","optobserve",
"undolist","undo","watching","overobserve","observewhileplay","playerline","playerlines",
"playersstatusline","players","@5","@6","userline","userlines","users","player",
"playertime","optbyo","gametime","gamescore","translation","translations","byoyomi",
"notime","lostconnection","gamesaved","optadjourn","adjourn","adjournsentrequest",
"adjournrequest","oppadjourn","declineadjourn","resign","mailed","removegamefile",
"notelltarget","telltarget","telldone","telloff","illegalmove","illegalundo",
"noturn","noremoveturn","useresign","removeliberty","removegroup","restorescoring",
"pleaseredone","statusheader","statusline","statuslines","resultline","status",
"date","uptimeentry","uptime","sgflist","reviewlist","reviewvariations","reviewstart",
"reviewliterals","reviewentry","reviewentries","review","@7","auxreviews","reviews",
"reviewstop","noreview","throwcopy","proba","setproba","sorry","invalid","unknown",
"names","namesset","promptnames","promptname","optname","literallines", NULL
};
#endif

static const short IgsYYr1[] = {     0,
   198,   198,   199,   199,   200,   201,   200,   202,   200,   204,
   203,   203,   205,   205,   206,   206,   206,   206,   207,   207,
   208,   208,   209,   210,   210,   210,   210,   210,   210,   210,
   210,   210,   210,   210,   210,   210,   210,   210,   210,   210,
   210,   210,   210,   210,   210,   210,   210,   210,   210,   210,
   210,   210,   210,   210,   210,   210,   210,   210,   210,   210,
   210,   210,   210,   210,   210,   210,   210,   210,   210,   210,
   210,   210,   210,   210,   210,   210,   210,   210,   210,   210,
   210,   210,   210,   210,   210,   210,   210,   210,   210,   210,
   210,   210,   210,   210,   210,   210,   210,   210,   210,   210,
   210,   210,   210,   210,   210,   210,   210,   210,   210,   210,
   210,   210,   210,   210,   210,   210,   210,   210,   210,   210,
   210,   210,   210,   210,   210,   210,   210,   210,   210,   210,
   210,   210,   210,   210,   210,   210,   210,   210,   210,   210,
   210,   211,   212,   213,   214,   214,   214,   215,   216,   216,
   217,   217,   217,   217,   217,   217,   218,   218,   219,   220,
   220,   221,   222,   223,   223,   224,   224,   225,   226,   227,
   228,   229,   230,   231,   232,   233,   234,   235,   236,   236,
   236,   236,   237,   238,   239,   240,   241,   242,   243,   244,
   245,   246,   247,   248,   248,   249,   250,   250,   251,   252,
   253,   253,   254,   254,   255,   256,   257,   258,   258,   259,
   260,   261,   262,   263,   264,   265,   266,   266,   268,   267,
   269,   270,   271,   271,   272,   272,   273,   273,   274,   274,
   275,   276,   277,   278,   279,   280,   281,   281,   282,   282,
   283,   283,   284,   285,   286,   286,   287,   287,   288,   288,
   289,   289,   290,   290,   291,   292,   292,   293,   293,   294,
   295,   296,   297,   298,   298,   299,   300,   301,   301,   302,
   303,   303,   304,   304,   305,   305,   306,   307,   308,   309,
   309,   310,   310,   311,   311,   313,   314,   312,   315,   315,
   316,   316,   317,   318,   319,   320,   320,   321,   322,   323,
   324,   324,   325,   326,   327,   328,   329,   329,   330,   331,
   332,   333,   334,   335,   335,   336,   336,   337,   338,   339,
   340,   341,   342,   342,   343,   344,   345,   346,   347,   348,
   349,   350,   351,   352,   353,   353,   354,   355,   356,   357,
   357,   357,   357,   358,   358,   359,   359,   360,   361,   362,
   363,   363,   364,   364,   364,   364,   364,   364,   364,   364,
   364,   364,   364,   364,   364,   364,   364,   364,   364,   364,
   364,   364,   364,   364,   364,   364,   364,   364,   364,   364,
   364,   365,   365,   367,   366,   368,   368,   369,   369,   370,
   371,   372,   373,   373,   374,   374,   375,   376,   377,   378,
   378,   379,   379,   380,   380,   381,   381,   382,   382,   383,
   383
};

static const short IgsYYr2[] = {     0,
     1,     1,     5,     0,     1,     0,     4,     0,     4,     0,
     4,     0,     2,     0,     1,     1,     1,     1,     2,     0,
     1,     1,     2,     1,     1,     1,     1,     1,     1,     1,
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
     0,     3,     1,     2,     2,     1,     2,     2,     2,     2,
     9,     6,     9,    11,    12,    11,     2,     4,     2,     2,
     1,     2,     2,     2,     2,    14,    12,     1,     3,     2,
     2,     3,     1,     2,     1,     1,     3,     2,     1,     1,
     1,     1,    10,     1,     2,     2,     2,     1,     2,     1,
     2,     1,     2,     2,     1,     3,     2,     0,     2,     4,
     2,     0,     1,     0,     7,     7,     4,     2,     1,     1,
     1,    10,     1,     1,     1,    17,     2,     0,     0,     3,
     1,     2,     2,     0,     1,     0,    15,    21,     2,     0,
     1,     2,     1,     1,     1,     1,     1,     0,     5,    11,
     1,     0,     4,     5,     5,     5,     2,     2,     3,     2,
     5,    11,     6,    12,     1,     4,    10,     4,    10,     2,
     2,     1,     6,     2,     0,     2,     2,     2,     0,     3,
     1,     0,     6,     4,     5,    11,     3,     1,     1,     6,
     3,     2,     0,    11,     7,     0,     0,     6,     2,     2,
     2,     0,     2,     4,     3,     4,     0,    23,     4,     2,
     2,     1,     2,     1,     3,     3,     1,     0,     5,     1,
     1,     1,     1,     1,     2,     2,     1,     1,     1,     3,
     1,     2,     1,     3,     1,     1,     1,     1,     1,     1,
     1,     1,     3,     2,     2,     0,     1,     3,     9,     3,
     3,     8,     2,     2,     1,     3,     2,     2,     2,     1,
     2,     0,     1,     1,     1,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     0,     0,     4,     2,     1,     1,     2,     1,
     1,     1,     6,     2,     6,     2,     1,     1,     2,     2,
     0,     3,     0,     2,     0,     1,     1,     1,     0,     2,
     0
};

static const short IgsYYdefact[] = {     0,
     2,    14,     0,    15,     8,    16,    18,    17,     6,     5,
    20,    13,    14,    14,     0,    12,    12,   140,     0,     0,
   323,   325,   184,   318,   317,   330,   337,   398,   401,   401,
   233,   234,   321,   350,     0,     0,   314,     0,   269,     0,
   262,     0,     0,     0,   304,   161,   272,     0,     0,     0,
   219,   221,     0,   278,   168,     0,   173,   188,   401,   384,
     0,     0,     0,     3,     0,   192,     0,   286,   411,   176,
   175,     0,   401,   231,     0,     0,     0,     0,     0,   312,
   255,     0,   326,     0,     0,   136,   242,   331,   401,   329,
   279,   319,     0,     0,     0,     0,   392,   397,     0,   401,
   265,   265,     0,   190,     0,   213,   179,   180,   181,   182,
   328,   143,   332,     0,   401,   327,   310,   311,     0,   235,
   236,   313,   401,   390,   401,   391,   401,   214,   202,   292,
     0,     0,   215,    19,     0,    57,    58,    24,    25,   146,
    26,    27,    31,   107,    28,    29,    30,    32,    33,    34,
    35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
   401,    45,    46,    47,    48,    49,    50,    51,    52,    53,
    54,   195,   198,    55,    56,    59,   209,   210,    60,    61,
    62,    63,    64,    65,    66,   224,    67,    68,    70,    69,
    71,    72,    73,    74,    75,    76,     0,    86,    87,    77,
    78,    79,    80,    81,    82,    83,    84,    85,   272,    88,
    89,    90,    91,    93,    92,    94,    95,   302,    96,    97,
    98,    99,   100,   101,   102,   103,   104,   105,   106,   109,
   108,   110,   111,   112,   113,   114,   115,   116,   117,   118,
   119,   120,   121,   122,     0,   123,   124,   345,   125,   126,
   127,   128,   129,   387,   388,   130,   131,   132,   133,   134,
   135,   137,   138,   139,    10,     9,     7,   148,   193,   272,
   316,   178,     0,   150,   157,   315,   185,     0,   149,   164,
   165,   303,   271,   159,     0,   394,   163,   162,   218,     0,
     0,   170,   171,     0,   187,     0,   383,   224,   248,   247,
     0,     0,   144,     0,   283,   399,   160,     0,     0,   260,
     0,     0,     0,   186,   300,     0,   226,   307,     0,   224,
   272,   250,   147,   241,   224,   232,     0,     0,     0,     0,
     0,     0,   343,   174,     0,   267,   266,   189,     0,     0,
   322,   191,   348,   349,   347,     0,     0,   401,     0,     0,
   396,    21,    22,    23,   145,     0,   401,   194,     0,   199,
   208,   261,   230,     0,   224,     0,   301,   336,   344,   389,
   386,   405,   324,   400,   403,   268,   270,     0,   220,     0,
   169,   172,   142,     0,     0,     0,     0,     0,     0,   410,
     0,     0,   277,     0,     0,     0,   305,   225,     0,   230,
   306,     0,   230,     0,   333,     0,   340,   341,     0,   177,
     0,   264,     0,   320,   346,     0,   202,   201,   291,     0,
     0,     0,     0,     0,   197,     0,     0,     0,   223,   226,
   249,   230,     0,   224,   338,     0,   207,     0,     0,   217,
     0,   352,   385,   355,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
   352,   352,   352,     0,     0,     0,     0,     0,   352,   353,
   354,   382,   274,     0,     0,     0,     0,   287,     0,   282,
   158,     0,     0,     0,     0,   226,   256,     0,   258,   224,
   226,   224,   243,     0,     0,     0,     0,     0,     0,   289,
   290,   299,     0,     0,   196,   222,   229,     0,   226,   238,
     0,   226,     0,   230,     0,   335,   407,    11,   406,   404,
     0,     0,     0,     0,   356,   358,   366,   365,   363,   361,
   364,   359,   367,   368,   369,   370,   371,   372,   373,   375,
   374,   379,   380,   381,   357,   378,   377,   362,   360,   376,
     0,     0,     0,     0,   409,     0,     0,     0,   294,     0,
     0,   309,     0,   226,   230,     0,     0,     0,     0,     0,
   202,     0,   401,     0,   244,   237,   239,     0,   251,     0,
     0,   275,   334,   402,   393,     0,     0,   351,   152,     0,
     0,     0,   408,     0,     0,   288,     0,   281,     0,     0,
     0,     0,     0,   253,     0,     0,     0,     0,     0,   200,
   204,   395,     0,     0,     0,   273,     0,     0,     0,     0,
     0,     0,   206,     0,     0,     0,     0,     0,     0,     0,
     0,   226,   226,     0,     0,     0,     0,   203,   205,   401,
     0,     0,     0,     0,     0,     0,   401,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,   246,   245,
     0,   342,   263,     0,     0,   272,     0,     0,     0,     0,
     0,     0,     0,   153,   151,     0,     0,   280,     0,     0,
     0,   230,     0,     0,   403,   272,   230,   230,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,   257,
   259,     0,   339,   212,   211,   183,   226,   226,   230,     0,
     0,   156,   154,     0,     0,     0,     0,     0,     0,   226,
   240,   252,   276,     0,     0,   155,   285,     0,     0,   167,
     0,   297,   254,     0,     0,     0,     0,     0,     0,     0,
   401,     0,     0,   166,   295,     0,     0,     0,   227,     0,
     0,     0,     0,     0,   284,   296,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,   216,     0,     0,   228,
   297,     0,   298,     0,     0,     0
};

static const short IgsYYdefgoto[] = {   774,
     2,    11,    14,    13,   266,   372,     3,    12,    15,   354,
   134,   135,   136,   137,   138,   139,   140,   141,   142,   143,
   144,   145,   146,   147,   148,   149,   150,   151,   152,   153,
   154,   155,   156,   157,   158,   159,   160,   161,   162,   163,
   164,   165,   166,   167,   168,   169,   170,   171,   172,   173,
   359,   360,   174,   418,   347,   639,   175,   176,   177,   178,
   179,   704,   180,   181,   182,   183,   440,   379,   184,   289,
   185,   429,   363,   510,   186,   430,   187,   188,   189,   190,
   191,   192,   577,   193,   327,   194,   195,   493,   196,   197,
   198,   199,   200,   201,   202,   203,   204,   205,   412,   336,
   206,   207,   278,   208,   284,   209,   210,   211,   212,   213,
   480,   389,   596,   214,   305,   556,   419,   348,   215,   313,
   732,   740,   216,   217,   218,   219,   220,   221,   222,   398,
   319,   224,   225,   226,   227,   228,   229,   230,   231,   232,
   233,   234,   235,   236,   237,   238,   239,   240,   241,   242,
   243,   244,   245,   516,   435,   246,   247,   330,   248,   249,
   250,   251,   252,   253,   525,   472,   384,   254,   297,   255,
   256,   257,   258,   259,   260,   261,   262,   263,   264,   521,
   437,   436,   520,   594,   306
};

static const short IgsYYpact[] = {    32,
-32768,    59,    49,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,   541,    23,    23,-32768,    55,    72,
    41,-32768,-32768,-32768,    67,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,    79,   133,   146,   178,-32768,   185,
-32768,   192,   193,   214,-32768,-32768,   139,   114,   159,   227,
-32768,-32768,   228,-32768,-32768,   182,-32768,   144,-32768,-32768,
   181,   231,   232,-32768,   234,-32768,   235,-32768,-32768,-32768,
-32768,    27,-32768,-32768,   236,   237,   238,   239,   147,    36,
-32768,   181,-32768,   240,   242,   243,    26,-32768,-32768,-32768,
-32768,-32768,   245,   245,   246,   247,-32768,-32768,   248,-32768,
-32768,-32768,   250,-32768,   251,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,   252,-32768,-32768,-32768,-32768,   107,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,    81,-32768,-32768,-32768,
   254,   156,-32768,-32768,    33,-32768,-32768,-32768,   243,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,    15,-32768,-32768,-32768,-32768,   244,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,   203,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,    -3,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,   139,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,   165,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,   155,-32768,-32768,-32768,   111,-32768,
-32768,-32768,-32768,-32768,   -51,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,   139,
-32768,   259,    40,-32768,-32768,-32768,-32768,   118,-32768,-32768,
-32768,-32768,-32768,-32768,   194,-32768,-32768,-32768,-32768,    68,
   262,-32768,-32768,   263,-32768,    47,-32768,-32768,-32768,-32768,
   264,     9,-32768,   265,-32768,   266,-32768,    11,   203,-32768,
    50,    82,    78,-32768,-32768,    80,   172,-32768,   173,-32768,
   139,   187,-32768,-32768,-32768,-32768,   201,    58,   274,   206,
   207,   277,-32768,-32768,    62,   241,   241,-32768,    85,   210,
   259,-32768,   259,   259,-32768,    63,    -8,    45,   102,   216,
-32768,-32768,-32768,-32768,-32768,     7,-32768,-32768,   197,-32768,
-32768,-32768,   -13,   285,-32768,    24,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,   286,   249,   287,
-32768,-32768,-32768,    -5,   290,    16,   291,   292,     8,-32768,
   293,   236,-32768,   294,   296,   297,-32768,-32768,   298,   -24,
-32768,   181,   -13,   181,-32768,   299,-32768,-32768,   300,-32768,
   253,-32768,   301,-32768,-32768,   255,-32768,-32768,-32768,    12,
   302,   303,   304,    66,-32768,   306,   307,   308,-32768,     0,
-32768,    -9,   309,-32768,   258,    51,   101,   312,   236,-32768,
   314,-32768,-32768,-32768,   305,   310,   311,   313,   315,   316,
   317,   318,   319,   320,   321,   323,   325,   326,   327,   328,
-32768,-32768,-32768,   329,   330,   331,   332,   333,-32768,-32768,
-32768,-32768,-32768,   127,   322,   134,   353,-32768,   354,-32768,
-32768,   131,   136,   355,   257,   172,-32768,   356,-32768,-32768,
   172,-32768,-32768,   357,   358,   359,   360,   138,   -11,-32768,
-32768,-32768,   361,   141,-32768,-32768,-32768,   362,   172,   179,
   364,   172,   365,    -6,   366,-32768,-32768,-32768,-32768,-32768,
   110,   367,   368,   369,   351,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,   351,   351,   351,-32768,-32768,-32768,-32768,-32768,   351,
   334,    22,   371,   372,   373,   374,   113,   375,-32768,   376,
   288,-32768,   377,   -12,   -23,   142,   378,   267,   379,   324,
-32768,   381,-32768,   382,-32768,-32768,-32768,   383,-32768,   384,
   385,-32768,-32768,-32768,-32768,   236,   195,-32768,-32768,   386,
   236,   387,-32768,   335,     3,-32768,   390,-32768,   391,   394,
   395,   396,   397,-32768,   105,   398,   399,   338,   208,-32768,
   152,-32768,    10,   401,   406,-32768,   407,   215,   409,    13,
   222,   223,-32768,   412,   413,   414,   415,   230,   224,   418,
   419,   172,   172,   233,   352,   363,   256,-32768,-32768,-32768,
   370,   420,   423,   424,   425,   426,-32768,   427,   380,   388,
   428,   260,   389,   429,   430,   431,   392,   433,-32768,-32768,
   434,-32768,-32768,   393,   164,   139,   400,   435,   436,   270,
   271,    18,   437,-32768,-32768,   438,   440,-32768,   261,   268,
   272,   -47,   441,   442,-32768,   139,   337,   337,   402,   443,
   444,   403,   404,   279,   446,   448,   450,   405,   276,-32768,
-32768,   408,-32768,-32768,   101,-32768,   172,   172,   337,   451,
   452,-32768,-32768,   410,   411,   453,   273,   468,   470,   172,
-32768,-32768,-32768,   475,   478,-32768,-32768,   481,   416,-32768,
   295,   289,-32768,   483,   336,   486,   487,   488,   489,   421,
-32768,   422,   492,-32768,-32768,   339,   417,     6,   340,   432,
   493,   494,   341,   495,-32768,-32768,   342,   496,   497,   498,
   343,   499,   344,   439,   500,   347,-32768,   445,   470,-32768,
   289,   447,-32768,   505,   506,-32768
};

static const short IgsYYpgoto[] = {-32768,
-32768,-32768,-32768,-32768,   490,-32768,   202,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,   -49,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,   345,-32768,
   149,-32768,-32768,-32768,  -394,-32768,-32768,-32768,   346,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,  -303,  -312,   -58,  -396,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,   -15,-32768,-32768,   449,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,   454,-32768,-32768,   458,
-32768,-32768,-32768,-32768,  -207,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,  -384,
  -260,  -258,-32768,-32768,   349,-32768,-32768,-32768,-32768,   501,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,   269,-32768,-32768,-32768,-32768,   459,   278,-32768,
-32768,-32768,-32768,-32768,  -249,-32768,-32768,   275,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,   -29,
  -170,-32768,-32768,-32768,-32768
};


#define	YYLAST		729


static const short IgsYYtable[] = {   272,
   273,   366,   298,   489,   397,   624,   491,   482,   374,   374,
    60,   386,   374,   309,   374,   646,   400,   442,   474,    20,
   374,   403,   499,   320,   590,     4,   426,   426,   325,   296,
   416,    -4,     1,   416,    -4,   512,   323,   426,   426,   391,
   700,   426,   374,   311,   426,   299,     6,     7,    53,   374,
    -4,     4,   374,   517,   523,    -4,    -4,   268,    -1,   328,
   374,   432,   373,   487,   374,   374,   443,     5,   374,   307,
   335,   326,     6,     7,   269,    53,   271,    53,    53,   478,
   352,   274,   427,   500,   501,   341,   324,   509,    84,   355,
   553,   479,  -293,   343,   433,   344,   265,   346,   364,     8,
    84,   308,   392,  -401,   357,   427,   427,   353,    -4,    -4,
    -4,   375,   374,   401,   370,   597,   427,   582,   383,  -293,
   427,   393,   518,   427,   519,     8,     9,    10,   270,   405,
   514,   356,   318,   410,   415,   275,  -308,   505,   365,   444,
   445,   446,   447,   448,   449,   450,   451,   452,   453,   454,
   455,   456,   457,   458,   459,   460,   461,   462,   463,   464,
   488,   465,   466,   467,   468,   469,   374,   571,   605,   417,
   428,   603,  -401,   562,   511,   276,   611,   581,   638,   632,
   277,   584,   285,   376,   598,   286,   564,   279,   565,   377,
   470,   471,   633,   416,   280,   281,   575,   387,   625,   579,
   423,   618,   753,   640,   647,   648,   621,   475,   693,   291,
   394,   542,   543,   544,    16,    17,   292,   293,   294,   550,
    93,    94,    95,    96,   350,   282,   283,   351,   287,   288,
   290,   295,    53,   301,   302,   686,   303,   304,   312,   314,
   315,   316,   321,   317,   322,   342,    19,   329,   332,   333,
   334,   604,   338,   339,   340,   345,   349,   362,    77,    30,
    89,   374,   378,   380,   381,   382,   385,   388,   390,   395,
   394,   396,    84,   399,   402,   404,   406,   407,   408,   409,
   413,   414,   411,   421,   422,   701,   357,   431,   438,   441,
   707,   708,   473,   476,   477,   481,   483,   439,   484,   485,
   486,   494,   495,   497,   502,   503,   504,   434,   506,   507,
   508,   513,   723,   515,   522,   496,   524,   551,   420,   659,
   660,   498,   558,   554,   552,   559,   526,   424,   561,   570,
   608,   527,   528,   606,   529,   573,   530,   531,   532,   533,
   534,   535,   536,   490,   537,   492,   538,   539,   540,   541,
   545,   546,   547,   548,   549,   555,   557,   560,   563,   566,
   567,   568,   569,   572,   574,   576,   578,   580,   583,   585,
   586,   587,   588,   591,   592,   593,   595,   599,   600,   602,
   607,   609,   601,   612,   614,   615,   616,   617,   620,   622,
   610,   619,   626,   627,   721,   722,   628,   629,   630,   631,
   634,   635,   636,   641,   637,   589,   623,   733,   642,   643,
   644,   645,   649,   650,   651,   652,   653,   654,   655,   656,
   657,   658,   667,   662,   661,   668,   669,   670,   671,   673,
   676,   679,   680,   681,   663,   683,   684,   689,   690,   694,
   695,   666,   696,   702,   703,   710,   711,   664,   715,   697,
   716,   674,   717,   724,   725,   728,   677,   698,   687,   675,
   678,   692,   729,   682,   685,   691,   427,   719,   699,   714,
   730,   688,   731,   709,   712,   713,   718,   734,   706,   720,
   735,   726,   727,   736,   739,   741,   738,   737,   743,   744,
   745,   746,   747,   749,   750,   756,   757,   759,   761,   762,
   763,   765,   768,   755,   775,   776,   267,   425,   771,   300,
   767,   752,   772,   368,   705,   223,   770,   358,   773,     0,
     0,     0,   754,   361,     0,   310,   369,     0,     0,   371,
     0,     0,   742,     0,     0,   751,   758,   760,   769,   764,
   766,    18,     0,   613,    19,    20,    21,    22,    23,    24,
    25,    26,   331,    27,    28,    29,    30,    31,    32,   337,
    33,    34,     0,     0,     0,     0,    35,   367,     0,    36,
    37,    38,    39,    40,    41,     0,     0,     0,    42,    43,
    44,    45,     0,    46,    47,    48,    49,    50,  -141,    51,
    52,     0,    53,    54,    55,     0,     0,    56,    57,    58,
    59,     0,    60,     0,     0,     0,     0,     0,    61,     0,
   665,    62,     0,     0,     0,  -141,    63,   672,     0,     0,
    64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
     0,    74,    75,    76,    77,    78,    79,    80,    81,    82,
    83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
    93,    94,    95,    96,    97,    98,    99,   100,   101,   102,
   103,   104,     0,   105,   106,   107,   108,   109,   110,   111,
     0,   112,   113,   114,   115,   116,   117,   118,   119,     0,
   120,   121,   122,   123,   124,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,   748,   125,   126,   127,     0,   128,   129,     0,     0,
   130,   131,     0,     0,     0,     0,   132,     0,   133
};

static const short IgsYYcheck[] = {    29,
    30,   209,    61,   400,   317,     3,   403,   392,     3,     3,
    62,     3,     3,    72,     3,     3,   320,    23,     3,     5,
     3,   325,   417,    82,     3,     3,    51,    51,    87,    59,
    42,     0,     1,    42,     3,   432,    86,    51,    51,    29,
    88,    51,     3,    73,    51,    61,    24,    25,    52,     3,
    19,     3,     3,     3,   439,    24,    25,     3,     0,    89,
     3,   365,   270,    88,     3,     3,    72,    19,     3,    43,
   100,    87,    24,    25,     3,    52,    10,    52,    52,    72,
    48,     3,   130,    72,    73,   115,    61,    88,   101,   139,
   475,    84,    48,   123,    71,   125,    74,   127,   102,    77,
   101,    75,    92,     3,    90,   130,   130,    75,    77,    78,
    79,    72,     3,   321,   166,     3,   130,   514,    72,    75,
   130,    72,    72,   130,    74,    77,    78,    79,    88,    72,
   434,   161,    97,    72,    72,     3,   101,    72,   197,   145,
   146,   147,   148,   149,   150,   151,   152,   153,   154,   155,
   156,   157,   158,   159,   160,   161,   162,   163,   164,   165,
   185,   167,   168,   169,   170,   171,     3,   179,   565,   178,
   184,   184,    72,   486,   184,    30,   571,   184,    27,    75,
     3,    72,    69,    66,    72,    72,   490,     3,   492,    72,
   196,   197,    88,    42,     3,     3,   509,   189,   196,   512,
   194,   586,   197,   194,   192,   193,   591,   192,   191,    28,
   189,   461,   462,   463,    13,    14,    35,    36,    37,   469,
   110,   111,   112,   113,    69,    12,    88,    72,    70,     3,
     3,    88,    52,     3,     3,    72,     3,     3,     3,     3,
     3,     3,     3,    97,     3,   139,     4,     3,     3,     3,
     3,   564,     3,     3,     3,   175,     3,    55,    94,    16,
   106,     3,    69,   196,     3,     3,     3,     3,     3,   192,
   189,   192,   101,   101,    88,    75,     3,    72,    72,     3,
   196,    72,    42,   182,    69,   682,    90,     3,     3,     3,
   687,   688,     3,     3,     3,     3,     3,    49,     3,     3,
     3,     3,     3,     3,     3,     3,     3,   366,     3,     3,
     3,     3,   709,    56,     3,    63,     3,   191,   348,   632,
   633,    67,   192,   190,     3,   190,    22,   357,    72,   192,
    64,    22,    22,   192,    22,   195,    22,    22,    22,    22,
    22,    22,    22,   402,    22,   404,    22,    22,    22,    22,
    22,    22,    22,    22,    22,     3,     3,     3,     3,     3,
     3,     3,     3,     3,     3,   187,     3,     3,     3,     3,
     3,     3,    22,     3,     3,     3,     3,     3,     3,     3,
     3,     3,    95,     3,     3,     3,     3,     3,     3,     3,
    67,   197,     3,     3,   707,   708,     3,     3,     3,     3,
     3,     3,    65,     3,   197,    72,    72,   720,     3,     3,
   196,     3,   191,   191,     3,     3,     3,     3,   189,   196,
     3,     3,     3,    72,   192,     3,     3,     3,     3,     3,
     3,     3,     3,     3,    72,     3,     3,     3,     3,     3,
     3,    72,     3,     3,     3,     3,     3,   192,     3,   189,
     3,    72,     3,     3,     3,     3,   197,   190,   666,    72,
    72,   191,   190,    72,    72,   196,   130,   192,   197,   191,
     3,    72,     3,    72,    72,    72,    72,     3,   686,    72,
     3,    72,    72,     3,   196,     3,   192,    72,     3,     3,
     3,     3,    72,    72,     3,     3,     3,     3,     3,     3,
     3,     3,     3,    72,     0,     0,    17,   359,   769,    61,
    72,    95,   771,   245,   685,    15,    72,   173,    72,    -1,
    -1,    -1,   183,   178,    -1,    72,   249,    -1,    -1,   255,
    -1,    -1,   197,    -1,    -1,   197,   196,   196,   192,   197,
   197,     1,    -1,   573,     4,     5,     6,     7,     8,     9,
    10,    11,    94,    13,    14,    15,    16,    17,    18,   102,
    20,    21,    -1,    -1,    -1,    -1,    26,   219,    -1,    29,
    30,    31,    32,    33,    34,    -1,    -1,    -1,    38,    39,
    40,    41,    -1,    43,    44,    45,    46,    47,    48,    49,
    50,    -1,    52,    53,    54,    -1,    -1,    57,    58,    59,
    60,    -1,    62,    -1,    -1,    -1,    -1,    -1,    68,    -1,
   640,    71,    -1,    -1,    -1,    75,    76,   647,    -1,    -1,
    80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
    -1,    91,    92,    93,    94,    95,    96,    97,    98,    99,
   100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
   110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
   120,   121,    -1,   123,   124,   125,   126,   127,   128,   129,
    -1,   131,   132,   133,   134,   135,   136,   137,   138,    -1,
   140,   141,   142,   143,   144,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,   741,   172,   173,   174,    -1,   176,   177,    -1,    -1,
   180,   181,    -1,    -1,    -1,    -1,   186,    -1,   188
};
/* -*-C-*-  Note some compilers choke on comments on `#line' lines.  */
#line 3 "/usr/share/bison.simple"

/* Skeleton output parser for bison,
   Copyright (C) 1984, 1989, 1990 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

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

#define IgsYYerrok		(IgsYYerrstatus = 0)
#define IgsYYclearin	(IgsYYchar = YYEMPTY)
#define YYEMPTY		-2
#define YYEOF		0
#define YYACCEPT	return(0)
#define YYABORT 	return(1)
#define YYERROR		goto IgsYYerrlab1
/* Like YYERROR except do call IgsYYerror.
   This remains here temporarily to ease the
   transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */
#define YYFAIL		goto IgsYYerrlab
#define YYRECOVERING()  (!!IgsYYerrstatus)
#define YYBACKUP(token, value) \
do								\
  if (IgsYYchar == YYEMPTY && IgsYYlen == 1)				\
    { IgsYYchar = (token), IgsYYlval = (value);			\
      IgsYYchar1 = YYTRANSLATE (IgsYYchar);				\
      YYPOPSTACK;						\
      goto IgsYYbackup;						\
    }								\
  else								\
    { IgsYYerror ("syntax error: cannot back up"); YYERROR; }	\
while (0)

#define YYTERROR	1
#define YYERRCODE	256

#ifndef YYPURE
#define YYLEX		IgsYYlex()
#endif

#ifdef YYPURE
#ifdef YYLSP_NEEDED
#ifdef YYLEX_PARAM
#define YYLEX		IgsYYlex(&IgsYYlval, &IgsYYlloc, YYLEX_PARAM)
#else
#define YYLEX		IgsYYlex(&IgsYYlval, &IgsYYlloc)
#endif
#else /* not YYLSP_NEEDED */
#ifdef YYLEX_PARAM
#define YYLEX		IgsYYlex(&IgsYYlval, YYLEX_PARAM)
#else
#define YYLEX		IgsYYlex(&IgsYYlval)
#endif
#endif /* not YYLSP_NEEDED */
#endif

/* If nonreentrant, generate the variables here */

#ifndef YYPURE

int	IgsYYchar;			/*  the lookahead symbol		*/
YYSTYPE	IgsYYlval;			/*  the semantic value of the		*/
				/*  lookahead symbol			*/

#ifdef YYLSP_NEEDED
YYLTYPE IgsYYlloc;			/*  location data for the lookahead	*/
				/*  symbol				*/
#endif

int IgsYYnerrs;			/*  number of parse errors so far       */
#endif  /* not YYPURE */

#if YYDEBUG != 0
int IgsYYdebug;			/*  nonzero means print parse trace	*/
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

#ifndef YYPARSE_RETURN_TYPE
#define YYPARSE_RETURN_TYPE int
#endif

/* Prevent warning if -Wstrict-prototypes.  */
#ifdef __GNUC__
YYPARSE_RETURN_TYPE IgsYYparse (void);
#endif

#if __GNUC__ > 1		/* GNU C and GNU C++ define this.  */
#define __IgsYY_memcpy(TO,FROM,COUNT)	__builtin_memcpy(TO,FROM,COUNT)
#else				/* not GNU C or C++ */
#ifndef __cplusplus

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */
static void
__IgsYY_memcpy (to, from, count)
     char *to;
     char *from;
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
__IgsYY_memcpy (char *to, char *from, int count)
{
  register char *f = from;
  register char *t = to;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}

#endif
#endif

#line 196 "/usr/share/bison.simple"

/* The user can define YYPARSE_PARAM as the name of an argument to be passed
   into IgsYYparse.  The argument should have type void *.
   It should actually point to an object.
   Grammar actions can access the variable by casting it
   to the proper pointer type.  */

#ifdef YYPARSE_PARAM
#ifdef __cplusplus
#define YYPARSE_PARAM_ARG void *YYPARSE_PARAM
#define YYPARSE_PARAM_DECL
#else /* not __cplusplus */
#define YYPARSE_PARAM_ARG YYPARSE_PARAM
#define YYPARSE_PARAM_DECL void *YYPARSE_PARAM;
#endif /* not __cplusplus */
#else /* not YYPARSE_PARAM */
#define YYPARSE_PARAM_ARG
#define YYPARSE_PARAM_DECL
#endif /* not YYPARSE_PARAM */

YYPARSE_RETURN_TYPE
IgsYYparse(YYPARSE_PARAM_ARG)
     YYPARSE_PARAM_DECL
{
  register int IgsYYstate;
  register int IgsYYn;
  register short *IgsYYssp;
  register YYSTYPE *IgsYYvsp;
  int IgsYYerrstatus;	/*  number of tokens to shift before error messages enabled */
  int IgsYYchar1 = 0;		/*  lookahead token as an internal (translated) token number */

  short	IgsYYssa[YYINITDEPTH];	/*  the state stack			*/
  YYSTYPE IgsYYvsa[YYINITDEPTH];	/*  the semantic value stack		*/

  short *IgsYYss = IgsYYssa;		/*  refer to the stacks thru separate pointers */
  YYSTYPE *IgsYYvs = IgsYYvsa;	/*  to allow IgsYYoverflow to reallocate them elsewhere */

#ifdef YYLSP_NEEDED
  YYLTYPE IgsYYlsa[YYINITDEPTH];	/*  the location stack			*/
  YYLTYPE *IgsYYls = IgsYYlsa;
  YYLTYPE *IgsYYlsp;

#define YYPOPSTACK   (IgsYYvsp--, IgsYYssp--, IgsYYlsp--)
#else
#define YYPOPSTACK   (IgsYYvsp--, IgsYYssp--)
#endif

  int IgsYYstacksize = YYINITDEPTH;

#ifdef YYPURE
  int IgsYYchar;
  YYSTYPE IgsYYlval;
  int IgsYYnerrs;
#ifdef YYLSP_NEEDED
  YYLTYPE IgsYYlloc;
#endif
#endif

  YYSTYPE IgsYYval;		/*  the variable used to return		*/
				/*  semantic values from the action	*/
				/*  routines				*/

  int IgsYYlen;

#if YYDEBUG != 0
  if (IgsYYdebug)
    fprintf(stderr, "Starting parse\n");
#endif

  IgsYYstate = 0;
  IgsYYerrstatus = 0;
  IgsYYnerrs = 0;
  IgsYYchar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  IgsYYssp = IgsYYss - 1;
  IgsYYvsp = IgsYYvs;
#ifdef YYLSP_NEEDED
  IgsYYlsp = IgsYYls;
#endif

/* Push a new state, which is found in  IgsYYstate  .  */
/* In all cases, when you get here, the value and location stacks
   have just been pushed. so pushing a state here evens the stacks.  */
IgsYYnewstate:

  *++IgsYYssp = IgsYYstate;

  if (IgsYYssp >= IgsYYss + IgsYYstacksize - 1)
    {
      /* Give user a chance to reallocate the stack */
      /* Use copies of these so that the &'s don't force the real ones into memory. */
      YYSTYPE *IgsYYvs1 = IgsYYvs;
      short *IgsYYss1 = IgsYYss;
#ifdef YYLSP_NEEDED
      YYLTYPE *IgsYYls1 = IgsYYls;
#endif

      /* Get the current used size of the three stacks, in elements.  */
      int size = IgsYYssp - IgsYYss + 1;

#ifdef IgsYYoverflow
      /* Each stack pointer address is followed by the size of
	 the data in use in that stack, in bytes.  */
#ifdef YYLSP_NEEDED
      /* This used to be a conditional around just the two extra args,
	 but that might be undefined if IgsYYoverflow is a macro.  */
      IgsYYoverflow("parser stack overflow",
		 &IgsYYss1, size * sizeof (*IgsYYssp),
		 &IgsYYvs1, size * sizeof (*IgsYYvsp),
		 &IgsYYls1, size * sizeof (*IgsYYlsp),
		 &IgsYYstacksize);
#else
      IgsYYoverflow("parser stack overflow",
		 &IgsYYss1, size * sizeof (*IgsYYssp),
		 &IgsYYvs1, size * sizeof (*IgsYYvsp),
		 &IgsYYstacksize);
#endif

      IgsYYss = IgsYYss1; IgsYYvs = IgsYYvs1;
#ifdef YYLSP_NEEDED
      IgsYYls = IgsYYls1;
#endif
#else /* no IgsYYoverflow */
      /* Extend the stack our own way.  */
      if (IgsYYstacksize >= YYMAXDEPTH)
	{
	  IgsYYerror("parser stack overflow");
	  return 2;
	}
      IgsYYstacksize *= 2;
      if (IgsYYstacksize > YYMAXDEPTH)
	IgsYYstacksize = YYMAXDEPTH;
      IgsYYss = (short *) alloca (IgsYYstacksize * sizeof (*IgsYYssp));
      __IgsYY_memcpy ((char *)IgsYYss, (char *)IgsYYss1, size * sizeof (*IgsYYssp));
      IgsYYvs = (YYSTYPE *) alloca (IgsYYstacksize * sizeof (*IgsYYvsp));
      __IgsYY_memcpy ((char *)IgsYYvs, (char *)IgsYYvs1, size * sizeof (*IgsYYvsp));
#ifdef YYLSP_NEEDED
      IgsYYls = (YYLTYPE *) alloca (IgsYYstacksize * sizeof (*IgsYYlsp));
      __IgsYY_memcpy ((char *)IgsYYls, (char *)IgsYYls1, size * sizeof (*IgsYYlsp));
#endif
#endif /* no IgsYYoverflow */

      IgsYYssp = IgsYYss + size - 1;
      IgsYYvsp = IgsYYvs + size - 1;
#ifdef YYLSP_NEEDED
      IgsYYlsp = IgsYYls + size - 1;
#endif

#if YYDEBUG != 0
      if (IgsYYdebug)
	fprintf(stderr, "Stack size increased to %d\n", IgsYYstacksize);
#endif

      if (IgsYYssp >= IgsYYss + IgsYYstacksize - 1)
	YYABORT;
    }

#if YYDEBUG != 0
  if (IgsYYdebug)
    fprintf(stderr, "Entering state %d\n", IgsYYstate);
#endif

  goto IgsYYbackup;
 IgsYYbackup:

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
/* IgsYYresume: */

  /* First try to decide what to do without reference to lookahead token.  */

  IgsYYn = IgsYYpact[IgsYYstate];
  if (IgsYYn == YYFLAG)
    goto IgsYYdefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* IgsYYchar is either YYEMPTY or YYEOF
     or a valid token in external form.  */

  if (IgsYYchar == YYEMPTY)
    {
#if YYDEBUG != 0
      if (IgsYYdebug)
	fprintf(stderr, "Reading a token: ");
#endif
      IgsYYchar = YYLEX;
    }

  /* Convert token to internal form (in IgsYYchar1) for indexing tables with */

  if (IgsYYchar <= 0)		/* This means end of input. */
    {
      IgsYYchar1 = 0;
      IgsYYchar = YYEOF;		/* Don't call YYLEX any more */

#if YYDEBUG != 0
      if (IgsYYdebug)
	fprintf(stderr, "Now at end of input.\n");
#endif
    }
  else
    {
      IgsYYchar1 = YYTRANSLATE(IgsYYchar);

#if YYDEBUG != 0
      if (IgsYYdebug)
	{
	  fprintf (stderr, "Next token is %d (%s", IgsYYchar, IgsYYtname[IgsYYchar1]);
	  /* Give the individual parser a way to print the precise meaning
	     of a token, for further debugging info.  */
#ifdef YYPRINT
	  YYPRINT (stderr, IgsYYchar, IgsYYlval);
#endif
	  fprintf (stderr, ")\n");
	}
#endif
    }

  IgsYYn += IgsYYchar1;
  if (IgsYYn < 0 || IgsYYn > YYLAST || IgsYYcheck[IgsYYn] != IgsYYchar1)
    goto IgsYYdefault;

  IgsYYn = IgsYYtable[IgsYYn];

  /* IgsYYn is what to do for this token type in this state.
     Negative => reduce, -IgsYYn is rule number.
     Positive => shift, IgsYYn is new state.
       New state is final state => don't bother to shift,
       just return success.
     0, or most negative number => error.  */

  if (IgsYYn < 0)
    {
      if (IgsYYn == YYFLAG)
	goto IgsYYerrlab;
      IgsYYn = -IgsYYn;
      goto IgsYYreduce;
    }
  else if (IgsYYn == 0)
    goto IgsYYerrlab;

  if (IgsYYn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */

#if YYDEBUG != 0
  if (IgsYYdebug)
    fprintf(stderr, "Shifting token %d (%s), ", IgsYYchar, IgsYYtname[IgsYYchar1]);
#endif

  /* Discard the token being shifted unless it is eof.  */
  if (IgsYYchar != YYEOF)
    IgsYYchar = YYEMPTY;

  *++IgsYYvsp = IgsYYlval;
#ifdef YYLSP_NEEDED
  *++IgsYYlsp = IgsYYlloc;
#endif

  /* count tokens shifted since error; after three, turn off error status.  */
  if (IgsYYerrstatus) IgsYYerrstatus--;

  IgsYYstate = IgsYYn;
  goto IgsYYnewstate;

/* Do the default action for the current state.  */
IgsYYdefault:

  IgsYYn = IgsYYdefact[IgsYYstate];
  if (IgsYYn == 0)
    goto IgsYYerrlab;

/* Do a reduction.  IgsYYn is the number of a rule to reduce with.  */
IgsYYreduce:
  IgsYYlen = IgsYYr2[IgsYYn];
  if (IgsYYlen > 0)
    IgsYYval = IgsYYvsp[1-IgsYYlen]; /* implement default value of the action */

#if YYDEBUG != 0
  if (IgsYYdebug)
    {
      int i;

      fprintf (stderr, "Reducing via rule %d (line %d), ",
	       IgsYYn, IgsYYrline[IgsYYn]);

      /* Print the symbols being reduced, and their result.  */
      for (i = IgsYYprhs[IgsYYn]; IgsYYrhs[i] > 0; i++)
	fprintf (stderr, "%s ", IgsYYtname[IgsYYrhs[i]]);
      fprintf (stderr, " -> %s\n", IgsYYtname[IgsYYr1[IgsYYn]]);
    }
#endif


  switch (IgsYYn) {

case 3:
#line 142 "./gointer.y"
{
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
                ;
    break;}
case 4:
#line 156 "./gointer.y"
{
                    Passed = 0;
                ;
    break;}
case 5:
#line 162 "./gointer.y"
{
                    if (MyPassword) ForceCommand(NULL, MyPassword);
                    else AskString(toplevel, EnterString,
                                   (XtPointer) &MyPassword, "Enter password",
                                   "password", &MyPassword, NULL, NULL);
                ;
    break;}
case 6:
#line 169 "./gointer.y"
{
                    if (MyPassword) ForceCommand(NULL, MyPassword);
                    else AskString(toplevel, EnterString,
                                   (XtPointer) &MyPassword, "Enter password",
                                   "password", &MyPassword, NULL, NULL);
                ;
    break;}
case 8:
#line 177 "./gointer.y"
{
                    myfree(MyName);
                    MyName = IgsYYvsp[0].Name;
                ;
    break;}
case 9:
#line 182 "./gointer.y"
{
                    Outputf("This is a guest account. Please see "
                            "'help register' to register.\n"
                            "Your account name is %s\n", IgsYYvsp[-3].Name);
                ;
    break;}
case 10:
#line 190 "./gointer.y"
{
                    if (!Passed) {
                        Passed = 1;
                        PlayerPasses(MyName);
                    }
                    ForceCommand(NULL, "toggle client on");
                ;
    break;}
case 15:
#line 206 "./gointer.y"
{
                    Outputf("%s\n", IgsYYvsp[0].Name);
                    myfree(IgsYYvsp[0].Name);
                ;
    break;}
case 16:
#line 211 "./gointer.y"
{
                    Outputf("          Welcome to IGS at %s ", IgsYYvsp[0].Name);
                    SiteLogon(NULL, IgsYYvsp[0].Name);
                    myfree(IgsYYvsp[0].Name);
                ;
    break;}
case 17:
#line 217 "./gointer.y"
{
                    if (MyName) ForceCommand(NULL, MyName);
                    else AskString(toplevel, EnterString, (XtPointer) &MyName,
                                   "Enter user", "user", &MyName, NULL, NULL);
                ;
    break;}
case 18:
#line 223 "./gointer.y"
{
                    ServerMessage("%s\n", IgsYYvsp[0].Name);
                    Outputf("%s\n", IgsYYvsp[0].Name);
                    myfree(IgsYYvsp[0].Name);
                ;
    break;}
case 19:
#line 230 "./gointer.y"
{ eEmpty = 0; ;
    break;}
case 20:
#line 231 "./gointer.y"
{ eEmpty = PreEmpty = 0; ;
    break;}
case 21:
#line 235 "./gointer.y"
{
                    PreEmpty = 0;
                    SeenAdd = 0;
                    IgsYYval.Value = IgsYYvsp[0].Value;
                ;
    break;}
case 22:
#line 241 "./gointer.y"
{
                    PreEmpty = eEmpty;
                    IgsYYval.Value = 0;
                ;
    break;}
case 23:
#line 248 "./gointer.y"
{
                    if (!Passed && IgsYYvsp[0].Value) {
                        Passed = 1;
                        PlayerPasses(MyName);
                    }
                    if (IgsYYvsp[0].Value) {
                        ChangeCommand(NULL, 1);
                        ResyncCommand(NULL);
                    }
                ;
    break;}
case 25:
#line 261 "./gointer.y"
{ ChangeCommand(NULL, -1); ;
    break;}
case 26:
#line 262 "./gointer.y"
{ ChangeCommand(NULL, -1); ;
    break;}
case 27:
#line 263 "./gointer.y"
{ ChangeCommand(NULL, -1); ;
    break;}
case 28:
#line 264 "./gointer.y"
{ ChangeCommand(NULL, -1); ;
    break;}
case 31:
#line 267 "./gointer.y"
{ ChangeCommand(NULL, -1); ;
    break;}
case 32:
#line 268 "./gointer.y"
{ ChangeCommand(NULL, -1); ;
    break;}
case 33:
#line 269 "./gointer.y"
{ ChangeCommand(NULL, -1); ;
    break;}
case 35:
#line 271 "./gointer.y"
{ ChangeCommand(NULL, -1); ;
    break;}
case 36:
#line 272 "./gointer.y"
{ ChangeCommand(NULL, -1); ;
    break;}
case 37:
#line 273 "./gointer.y"
{ ChangeCommand(NULL, -1); ;
    break;}
case 38:
#line 274 "./gointer.y"
{ ChangeCommand(NULL, -1); ;
    break;}
case 45:
#line 281 "./gointer.y"
{ ChangeCommand(NULL, -1); ;
    break;}
case 49:
#line 285 "./gointer.y"
{ ChangeCommand(NULL, -1); ;
    break;}
case 70:
#line 306 "./gointer.y"
{ ChangeCommand(NULL, -1); ;
    break;}
case 72:
#line 308 "./gointer.y"
{ ChangeCommand(NULL, -1); ;
    break;}
case 74:
#line 310 "./gointer.y"
{ ChangeCommand(NULL, -1); ;
    break;}
case 75:
#line 311 "./gointer.y"
{ ChangeCommand(NULL, -1); ;
    break;}
case 76:
#line 312 "./gointer.y"
{ ChangeCommand(NULL, -1); ;
    break;}
case 78:
#line 314 "./gointer.y"
{ ChangeCommand(NULL, -1); ;
    break;}
case 80:
#line 316 "./gointer.y"
{ ChangeCommand(NULL, -1); ;
    break;}
case 83:
#line 319 "./gointer.y"
{ ChangeCommand(NULL, -1); ;
    break;}
case 87:
#line 323 "./gointer.y"
{ ChangeCommand(NULL, -1); ;
    break;}
case 88:
#line 324 "./gointer.y"
{ ChangeCommand(NULL, -1); ;
    break;}
case 99:
#line 335 "./gointer.y"
{ ChangeCommand(NULL, -1); ;
    break;}
case 102:
#line 338 "./gointer.y"
{ ChangeCommand(NULL, -1); ;
    break;}
case 103:
#line 339 "./gointer.y"
{ ChangeCommand(NULL, -1); ;
    break;}
case 104:
#line 340 "./gointer.y"
{ ChangeCommand(NULL, -1); ;
    break;}
case 105:
#line 341 "./gointer.y"
{ ChangeCommand(NULL, -1); ;
    break;}
case 106:
#line 342 "./gointer.y"
{ ChangeCommand(NULL, -1); ;
    break;}
case 107:
#line 343 "./gointer.y"
{ ChangeCommand(NULL, -1); ;
    break;}
case 108:
#line 344 "./gointer.y"
{ ChangeCommand(NULL, -1); ;
    break;}
case 125:
#line 362 "./gointer.y"
{
                    if (!Entered) {
                        Entering();
                    }
                ;
    break;}
case 136:
#line 377 "./gointer.y"
{ ChangeCommand(NULL, -1); ;
    break;}
case 140:
#line 382 "./gointer.y"
{
                    /* 1 in case next token is SEMIPROMPT */
                    SetCommand(NULL, 1);
                ;
    break;}
case 141:
#line 386 "./gointer.y"
{ eEmpty = 1; ;
    break;}
case 142:
#line 390 "./gointer.y"
{
                    NameList   *Names;
                    const char *User;

                    switch(IgsYYvsp[-2].Value) {
                      case 25: /* Results */
                        if (UserCommandP(NULL)) goto user;
                        User = StripFirstArgCommand(NULL, "results");
                        if (!User) goto user;
			if (*User == '-') User++;
                        /* AddResults keeps $2 */
                        AddResults(User, IgsYYvsp[-1].Namelist);
                        break;
                      default:
                      user:
                        for (Names = IgsYYvsp[-1].Namelist->Next;Names != IgsYYvsp[-1].Namelist; Names = Names->Next)
                            Outputf("%s\n", Names->Name);
                        FreeNameList(IgsYYvsp[-1].Namelist);
                    }
                ;
    break;}
case 143:
#line 413 "./gointer.y"
{
                    Output("Please erase your messages (see help erase)\n");
                ;
    break;}
case 144:
#line 419 "./gointer.y"
{
                    char *ptr;

                    Outputf("Logging into %s %s\n", IgsYYvsp[-1].Dummy, IgsYYvsp[0].Name);
                    ptr = mystrdup(IgsYYvsp[-1].Dummy);
                    myfree(ServerName);
                    ServerName = ptr;
                    switch(ptr[0]) {
                      case 'N': ServerType = NNGS; break;
                      default:  ServerType = IGS;  break;
                    }
                    myfree(IgsYYvsp[-1].Dummy);
                    myfree(IgsYYvsp[0].Name);
                ;
    break;}
case 147:
#line 437 "./gointer.y"
{;
    break;}
case 148:
#line 441 "./gointer.y"
{
                    ServerMessage("%s\n", IgsYYvsp[0].Name);
                    myfree(IgsYYvsp[0].Name);
                ;
    break;}
case 149:
#line 448 "./gointer.y"
{
                    ServerMessage("%s: %s\n", PlayerString(IgsYYvsp[-1].Person), IgsYYvsp[0].Name);
                    myfree(IgsYYvsp[0].Name);
                ;
    break;}
case 150:
#line 453 "./gointer.y"
{
		    /* dummy player name such as "*8^)*" */
                    ServerMessage("%s: %s\n", IgsYYvsp[-1].Name, IgsYYvsp[0].Name);
                    myfree(IgsYYvsp[-1].Name);
                    myfree(IgsYYvsp[0].Name);
                ;
    break;}
case 151:
#line 462 "./gointer.y"
{
                    /* A Connect */
                    if (strcmp(IgsYYvsp[-3].Name, "has") || strcmp(IgsYYvsp[-2].Name, "connected."))
                        YYFAIL;
                    PlayerConnect(IgsYYvsp[-7].Name, IgsYYvsp[-5].Name);
                    myfree(IgsYYvsp[-7].Name);
                    myfree(IgsYYvsp[-5].Name);
                    myfree(IgsYYvsp[-3].Name);
                    myfree(IgsYYvsp[-2].Name);
                ;
    break;}
case 152:
#line 473 "./gointer.y"
{
                    /* A disconnect */
                    if (strcmp(IgsYYvsp[-3].Name, "has") || strcmp(IgsYYvsp[-2].Name, "disconnected"))
                        YYFAIL;
                    PlayerDisconnect(IgsYYvsp[-4].Name);
                    myfree(IgsYYvsp[-4].Name);
                    myfree(IgsYYvsp[-3].Name);
                    myfree(IgsYYvsp[-2].Name);
                ;
    break;}
case 153:
#line 483 "./gointer.y"
{
                    /* A new match, format with game number */
                    if (strcmp(IgsYYvsp[-3].Name, "vs.") || strcmp(IgsYYvsp[-7].Name, "Match")) YYFAIL;
                    NewMatch(atoi(IgsYYvsp[-6].Name), IgsYYvsp[-4].Person, IgsYYvsp[-2].Person);
                    myfree(IgsYYvsp[-7].Name);
                    myfree(IgsYYvsp[-6].Name);
                    myfree(IgsYYvsp[-3].Name);
                ;
    break;}
case 154:
#line 492 "./gointer.y"
{
                    if (strcmp(IgsYYvsp[-5].Name, "vs") || strcmp(IgsYYvsp[-9].Name, "Game")) YYFAIL;
                    GameInfo(atoi(IgsYYvsp[-8].Name), IgsYYvsp[-4].Name, IgsYYvsp[-6].Name, IgsYYvsp[-2].Namelist);
                    myfree(IgsYYvsp[-9].Name);
                    myfree(IgsYYvsp[-8].Name);
                    myfree(IgsYYvsp[-6].Name);
                    myfree(IgsYYvsp[-5].Name);
                    myfree(IgsYYvsp[-4].Name);
                    FreeNameList(IgsYYvsp[-2].Namelist);
                ;
    break;}
case 155:
#line 503 "./gointer.y"
{
                    /* Resume */
                    if (strcmp(IgsYYvsp[-10].Name, "Game") || strcmp(IgsYYvsp[-6].Name, "vs") ||
                        strcmp(IgsYYvsp[-3].Name, "Move")) YYFAIL;
                    Resume(atoi(IgsYYvsp[-9].Name), IgsYYvsp[-5].Name, IgsYYvsp[-7].Name, atoi(IgsYYvsp[-2].Name));
                    myfree(IgsYYvsp[-10].Name);
                    myfree(IgsYYvsp[-9].Name);
                    myfree(IgsYYvsp[-7].Name);
                    myfree(IgsYYvsp[-6].Name);
                    myfree(IgsYYvsp[-5].Name);
                    myfree(IgsYYvsp[-3].Name);
                    myfree(IgsYYvsp[-2].Name);
                ;
    break;}
case 156:
#line 517 "./gointer.y"
{
                    /* Adjourn */
                    if (strcmp(IgsYYvsp[-3].Name, "has") || strcmp(IgsYYvsp[-2].Name, "adjourned.") ||
                        strcmp(IgsYYvsp[-5].Name, "vs")) YYFAIL;
                    Adjourn(atoi(IgsYYvsp[-8].Name), IgsYYvsp[-4].Name, IgsYYvsp[-6].Name);
                    myfree(IgsYYvsp[-9].Name);
                    myfree(IgsYYvsp[-8].Name);
                    myfree(IgsYYvsp[-6].Name);
                    myfree(IgsYYvsp[-5].Name);
                    myfree(IgsYYvsp[-4].Name);
                    myfree(IgsYYvsp[-3].Name);
                    myfree(IgsYYvsp[-2].Name);
                ;
    break;}
case 157:
#line 533 "./gointer.y"
{
                    ReceivedTell(IgsYYvsp[-1].Person, IgsYYvsp[0].Name);
                    myfree(IgsYYvsp[0].Name);
                ;
    break;}
case 158:
#line 538 "./gointer.y"
{
                    ReceivedTell(IgsYYvsp[-1].Person, IgsYYvsp[0].Name);
                    myfree(IgsYYvsp[0].Name);
                ;
    break;}
case 159:
#line 545 "./gointer.y"
{
                    ReceivedTell(IgsYYvsp[-1].Person, "is now on.");
                ;
    break;}
case 160:
#line 552 "./gointer.y"
{
                    Beeping(IgsYYvsp[0].Person);
                ;
    break;}
case 161:
#line 556 "./gointer.y"
{
                    Beeping(IgsYYvsp[0].Person);
                ;
    break;}
case 162:
#line 562 "./gointer.y"
{
		    Idle(IgsYYvsp[-1].Person, IgsYYvsp[0].Name);
                    myfree(IgsYYvsp[0].Name);
                ;
    break;}
case 163:
#line 569 "./gointer.y"
{
		    StoredNum(IgsYYvsp[-1].Person, IgsYYvsp[0].Value);
                ;
    break;}
case 164:
#line 575 "./gointer.y"
{
                    ShowBroadcast(IgsYYvsp[-1].Person, ":", IgsYYvsp[0].Name);
                    myfree(IgsYYvsp[0].Name);
                ;
    break;}
case 165:
#line 580 "./gointer.y"
{
                    ShowBroadcast(IgsYYvsp[-1].Person, "", IgsYYvsp[0].Name);
                    myfree(IgsYYvsp[0].Name);
                ;
    break;}
case 166:
#line 588 "./gointer.y"
{
                    if (strcmp(IgsYYvsp[-8].Name, "Game") || strcmp(IgsYYvsp[-6].Name, "vs")) YYFAIL;
                    ReceivedKibitz(IgsYYvsp[-10].Person, atoi(IgsYYvsp[-3].Name), IgsYYvsp[-5].Name, IgsYYvsp[-7].Name, IgsYYvsp[0].Name, strlen(IgsYYvsp[0].Name));
                    myfree(IgsYYvsp[-8].Name);
                    myfree(IgsYYvsp[-7].Name);
                    myfree(IgsYYvsp[-6].Name);
                    myfree(IgsYYvsp[-5].Name);
                    myfree(IgsYYvsp[-3].Name);
                    myfree(IgsYYvsp[0].Name);
                ;
    break;}
case 167:
#line 600 "./gointer.y"
{
                    if (strcmp(IgsYYvsp[-8].Name, "Game") || strcmp(IgsYYvsp[-6].Name, "vs")) YYFAIL;
                    ReceivedKibitz(IgsYYvsp[-10].Person, atoi(IgsYYvsp[-3].Name), IgsYYvsp[-5].Name, IgsYYvsp[-7].Name, IgsYYvsp[0].Name, strlen(IgsYYvsp[0].Name));
                    myfree(IgsYYvsp[-8].Name);
                    myfree(IgsYYvsp[-7].Name);
                    myfree(IgsYYvsp[-6].Name);
                    myfree(IgsYYvsp[-5].Name);
                    myfree(IgsYYvsp[-3].Name);
                    myfree(IgsYYvsp[0].Name);
                  ;
    break;}
case 168:
#line 613 "./gointer.y"
{
                    Outputf("You have %d line%s of messages\n",
                            IgsYYvsp[0].Value, IgsYYvsp[0].Value==1 ? "" : "s");
                ;
    break;}
case 169:
#line 619 "./gointer.y"
{
                    ShowYell(IgsYYvsp[-2].Value, IgsYYvsp[-1].Person, IgsYYvsp[0].Name);
                    myfree(IgsYYvsp[0].Name);
                ;
    break;}
case 170:
#line 626 "./gointer.y"
{
                    ChannelJoin(IgsYYvsp[-1].Value, IgsYYvsp[0].Person);
                ;
    break;}
case 171:
#line 632 "./gointer.y"
{
                    ChannelLeave(IgsYYvsp[-1].Value, IgsYYvsp[0].Person);
                ;
    break;}
case 172:
#line 638 "./gointer.y"
{
                    ChannelTitle(IgsYYvsp[-2].Value, IgsYYvsp[-1].Person, IgsYYvsp[0].Name);
                    myfree(IgsYYvsp[0].Name);
                ;
    break;}
case 173:
#line 645 "./gointer.y"
{
                    JoinChannel(IgsYYvsp[0].Value);
                ;
    break;}
case 174:
#line 651 "./gointer.y"
{
                    WrongChannel(IgsYYvsp[0].Name);
                    myfree(IgsYYvsp[0].Name);
                ;
    break;}
case 175:
#line 658 "./gointer.y"
{
                    Output("Setting you open for matches\n");
                ;
    break;}
case 176:
#line 664 "./gointer.y"
{
                    Output("You are not open for matches\n");
                ;
    break;}
case 177:
#line 670 "./gointer.y"
{
                    AutoMatchRequest(IgsYYvsp[-1].Namelist);
                    FreeNameList(IgsYYvsp[-1].Namelist);
                ;
    break;}
case 178:
#line 677 "./gointer.y"
{
                    AutoMatchDispute(IgsYYvsp[-1].Name, IgsYYvsp[0].Namelist);
                    myfree(IgsYYvsp[-1].Name);
                    FreeNameList(IgsYYvsp[0].Namelist);
                ;
    break;}
case 179:
#line 685 "./gointer.y"
{ IgsYYval.Value = 'I'; ;
    break;}
case 180:
#line 686 "./gointer.y"
{ IgsYYval.Value = 'G'; ;
    break;}
case 181:
#line 687 "./gointer.y"
{ IgsYYval.Value = 'i'; ;
    break;}
case 182:
#line 688 "./gointer.y"
{ IgsYYval.Value = 'g'; ;
    break;}
case 183:
#line 693 "./gointer.y"
{
                    if (strcmp(IgsYYvsp[-6].Name, "or")) YYFAIL;
                    MatchRequest(IgsYYvsp[-9].Value, IgsYYvsp[-8].Namelist);
                    FreeNameList(IgsYYvsp[-8].Namelist);
                    myfree(IgsYYvsp[-6].Name);
                    FreeNameList(IgsYYvsp[-4].Namelist);
                    FreeNameList(IgsYYvsp[-2].Namelist);
                ;
    break;}
case 184:
#line 704 "./gointer.y"
{
                    /* Outputf("%s\n", $1); */
                    myfree(IgsYYvsp[0].Name);
                ;
    break;}
case 185:
#line 711 "./gointer.y"
{
                    char *Ptr;

                    Ptr = strchr(IgsYYvsp[0].Name, 0)-1;
                    if (*Ptr == '.') *Ptr = 0;
                    MyGameMessage("%s wants the komi to be %s",
	                          PlayerString(IgsYYvsp[-1].Person), IgsYYvsp[0].Name);
                    if (WhatCommand(NULL, "komi") < 0) ChangeCommand(NULL, -1);
                    myfree(IgsYYvsp[0].Name);
                ;
    break;}
case 186:
#line 724 "./gointer.y"
{
                    char *Ptr;

                    Ptr = strchr(IgsYYvsp[0].Name, 0)-1;
                    if (*Ptr == '.') *Ptr = 0;
                    MyGameMessage("The komi has been set to %s", IgsYYvsp[0].Name);
                    CheckMyKomi(IgsYYvsp[0].Name);
                    if (WhatCommand(NULL, "komi") < 0) ChangeCommand(NULL, -1);
                    myfree(IgsYYvsp[0].Name);
                ;
    break;}
case 187:
#line 737 "./gointer.y"
{
                    Outputf("Game will %scount towards ratings\n",
                            IgsYYvsp[-1].Value ? "not" : "");
                ;
    break;}
case 188:
#line 744 "./gointer.y"
{
                    Outputf("Game will %scount towards ratings\n",
                            IgsYYvsp[0].Value ? "not " : "");
                ;
    break;}
case 189:
#line 751 "./gointer.y"
{
                    Outputf("You cannot change into a free game after %s\n",
                            IgsYYvsp[0].Name);
                    myfree(IgsYYvsp[0].Name);
                ;
    break;}
case 190:
#line 759 "./gointer.y"
{
                    Output("You are not playing a game\n");
                ;
    break;}
case 191:
#line 765 "./gointer.y"
{
                    Output("Your opponent is not on currently. "
                           "Game failed to load\n");
                ;
    break;}
case 192:
#line 771 "./gointer.y"
{
                    const char *title;

                    title = ArgsCommand(NULL, "title");
                    if (title) SetMyGameTitle(title);
                    else Warning("Title set, but I "
                                 "can't remember to what....\n");
                ;
    break;}
case 193:
#line 782 "./gointer.y"
{
                    NameVal *nameval;

                    IgsYYval.Nameval = nameval = mynew(NameVal);
                    nameval->Next  = nameval->Previous = nameval;
                    nameval->Name  = IgsYYvsp[-1].Name;
                    nameval->Value = IgsYYvsp[0].Name;
                ;
    break;}
case 194:
#line 793 "./gointer.y"
{
                    IgsYYval.Nameval = IgsYYvsp[-1].Nameval;
                    IgsYYvsp[0].Nameval->Previous = IgsYYvsp[-1].Nameval->Previous;
                    IgsYYvsp[0].Nameval->Next     = IgsYYvsp[-1].Nameval;
                    IgsYYvsp[0].Nameval->Previous->Next = IgsYYvsp[0].Nameval->Next->Previous = IgsYYvsp[0].Nameval;
                ;
    break;}
case 195:
#line 799 "./gointer.y"
{ IgsYYval.Nameval = IgsYYvsp[0].Nameval; ;
    break;}
case 196:
#line 802 "./gointer.y"
{ IgsYYval.Namelist = IgsYYvsp[-1].Namelist; ;
    break;}
case 197:
#line 806 "./gointer.y"
{
                    NameVal *nameval;
                    NameList *Pos1, *Pos2;

                    nameval = mynew(NameVal);
                    nameval->Previous = nameval->Next = nameval;
                    nameval->Name = nameval->Value = NULL;
                    IgsYYval.Nameval = nameval;

                    for (Pos1=IgsYYvsp[-1].Namelist->Next, Pos2=IgsYYvsp[0].Namelist->Next;
                         Pos1 != IgsYYvsp[-1].Namelist && Pos2 != IgsYYvsp[0].Namelist;
                         Pos1 = Pos1->Next, Pos2 = Pos2->Next) {
                        nameval = mynew(NameVal);
                        nameval->Name  = Pos1->Name; Pos1->Name = NULL;
                        nameval->Value = Pos2->Name; Pos2->Name = NULL;
                        nameval->Next = IgsYYval.Nameval;
                        nameval->Previous = IgsYYval.Nameval->Previous;
                        nameval->Previous->Next =
                            nameval->Next->Previous = nameval;
                    }
                    if (Pos1 != IgsYYvsp[-1].Namelist || Pos2 != IgsYYvsp[0].Namelist)
                        Warning("Name value lists have different length\n");
                    FreeNameList(Pos1);
                    FreeNameList(Pos2);
                ;
    break;}
case 198:
#line 832 "./gointer.y"
{
                    NameVal *nameval;

                    nameval = mynew(NameVal);
                    nameval->Previous = nameval->Next = nameval;
                    nameval->Name = nameval->Value = NULL;
                    IgsYYval.Nameval = nameval;
                ;
    break;}
case 199:
#line 843 "./gointer.y"
{
                    NameVal *ext;

                    IgsYYvsp[0].Nameval->Next->Previous = IgsYYvsp[-1].Nameval->Previous;
                    ext = IgsYYvsp[-1].Nameval->Previous->Next = IgsYYvsp[0].Nameval->Next;
                    IgsYYvsp[0].Nameval->Next = IgsYYvsp[-1].Nameval;
                    IgsYYvsp[-1].Nameval->Previous = IgsYYvsp[0].Nameval;
                    ShowStats(IgsYYvsp[0].Nameval, ext);
                    FreeNameValList(IgsYYvsp[0].Nameval);
                ;
    break;}
case 200:
#line 856 "./gointer.y"
{
                    IgsYYval.Bet = mynew(BetDesc);
                    IgsYYval.Bet->Who  = IgsYYvsp[-3].Person;
                    IgsYYval.Bet->Wins = IgsYYvsp[-2].Value;
                    IgsYYval.Bet->Bets = IgsYYvsp[0].Value;
                ;
    break;}
case 201:
#line 865 "./gointer.y"
{
                    IgsYYvsp[0].Bet->Next = IgsYYvsp[-1].Bet;
                    IgsYYval.Bet = IgsYYvsp[0].Bet;
                ;
    break;}
case 202:
#line 870 "./gointer.y"
{
                    IgsYYval.Bet = NULL;
                ;
    break;}
case 203:
#line 875 "./gointer.y"
{ IgsYYval.Name = IgsYYvsp[0].Name;   ;
    break;}
case 204:
#line 876 "./gointer.y"
{ IgsYYval.Name = NULL; ;
    break;}
case 205:
#line 881 "./gointer.y"
{
                    BetDesc *Here, *Next;

                    BetResults(IgsYYvsp[-5].Bet, IgsYYvsp[-3].Bet, IgsYYvsp[-1].Bet, IgsYYvsp[0].Name);
                    for (Here = IgsYYvsp[-5].Bet; Here; Here = Next) {
                        Next = Here->Next;
                        myfree(Here);
                    }
                    for (Here = IgsYYvsp[-3].Bet; Here; Here = Next) {
                        Next = Here->Next;
                        myfree(Here);
                    }
                    for (Here = IgsYYvsp[-1].Bet; Here; Here = Next) {
                        Next = Here->Next;
                        myfree(Here);
                    }
                ;
    break;}
case 206:
#line 901 "./gointer.y"
{
                    /* -Ton remove the optname */
                    /* eg: Set | verbose to be True. */
                    SetStat(IgsYYvsp[-5].Name, strcmp(IgsYYvsp[-2].Name+1, "alse."));
                    myfree(IgsYYvsp[-5].Name);
                    myfree(IgsYYvsp[-4].Name);
                    myfree(IgsYYvsp[-3].Name);
                    myfree(IgsYYvsp[-2].Name);
                ;
    break;}
case 207:
#line 913 "./gointer.y"
{
                    IgsYYvsp[-2].Namelist->Name = (char *) IgsYYvsp[0].Namelist;
                    IgsYYvsp[0].Namelist->Name = IgsYYvsp[-3].Name;
                    IgsYYval.Namelist = IgsYYvsp[-2].Namelist;
                ;
    break;}
case 208:
#line 921 "./gointer.y"
{
                    NameList *Names;

                    IgsYYval.Channeldata = IgsYYvsp[-1].Channeldata;
                    Names = (NameList *) IgsYYvsp[0].Namelist->Name;
                    IgsYYvsp[0].Namelist->Name = NULL;
                    AddChannelData(IgsYYval.Channeldata, Names->Name, IgsYYvsp[0].Namelist->Next->Name,
                                   IgsYYvsp[0].Namelist->Next->Next->Name,
                                   IgsYYvsp[0].Namelist->Next->Next->Next->Name, Names);
                    Names->Name = NULL;
                    FreeNameList(IgsYYvsp[0].Namelist);
                ;
    break;}
case 209:
#line 934 "./gointer.y"
{
                    NameList *Names;

                    IgsYYval.Channeldata = OpenChannelData();
                    Names = (NameList *) IgsYYvsp[0].Namelist->Name;
                    IgsYYvsp[0].Namelist->Name = NULL;
                    AddChannelData(IgsYYval.Channeldata, Names->Name, IgsYYvsp[0].Namelist->Next->Name,
                                   IgsYYvsp[0].Namelist->Next->Next->Name,
                                   IgsYYvsp[0].Namelist->Next->Next->Next->Name, Names);
                    Names->Name = NULL;
                    FreeNameList(IgsYYvsp[0].Namelist);
                ;
    break;}
case 210:
#line 949 "./gointer.y"
{
                    ChannelList(IgsYYvsp[0].Channeldata);
                    CloseChannelData(IgsYYvsp[0].Channeldata);
                ;
    break;}
case 211:
#line 955 "./gointer.y"
{ IgsYYval.Namelist = IgsYYvsp[0].Namelist; ;
    break;}
case 212:
#line 959 "./gointer.y"
{
                    if (strcmp(IgsYYvsp[-5].Name, "vs.")) YYFAIL;
                    ShowObservers(atoi(IgsYYvsp[-8].Name), IgsYYvsp[-4].Name, IgsYYvsp[-6].Name, IgsYYvsp[0].Namelist);

                    myfree(IgsYYvsp[-8].Name);
                    myfree(IgsYYvsp[-6].Name);
                    myfree(IgsYYvsp[-5].Name);
                    myfree(IgsYYvsp[-4].Name);
                    FreeNameList(IgsYYvsp[0].Namelist);
                ;
    break;}
case 213:
#line 972 "./gointer.y"
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
                ;
    break;}
case 214:
#line 989 "./gointer.y"
{
		    const char *msg = "There are no more moves";
		    StopMyGameForward(msg);
                    MyGameMessage(msg);
                ;
    break;}
case 215:
#line 998 "./gointer.y"
{
		    const char *msg = "This teach game is not a request game";
		    StopMyGameForward(msg);
                    MyGameMessage(msg);
                ;
    break;}
case 216:
#line 1008 "./gointer.y"
{
                    int    Mode, Rules;
                    size_t size;
                    char  *ptr;

                    ptr = IgsYYvsp[-6].Name;
                    if (ptr[1]) Mode = *ptr++;
                    else Mode = ' ';
                    Rules = *ptr++;
                    if (*ptr) YYFAIL;

                    size = atoi(IgsYYvsp[-10].Name);
                    IgsYYval.Game = FindGame(IgsYYvsp[-16].Value, IgsYYvsp[-13].Person, IgsYYvsp[-15].Person,
                                  atoi(IgsYYvsp[-11].Name), size, size, atoi(IgsYYvsp[-9].Name), IgsYYvsp[-8].Name,
                                  atoi(IgsYYvsp[-7].Name), Mode, Rules, atoi(IgsYYvsp[-2].Name));
                    myfree(IgsYYvsp[-14].Name);
                    myfree(IgsYYvsp[-11].Name);
                    myfree(IgsYYvsp[-10].Name);
                    myfree(IgsYYvsp[-9].Name);
                    myfree(IgsYYvsp[-8].Name);
                    myfree(IgsYYvsp[-7].Name);
                    myfree(IgsYYvsp[-6].Name);
                    FreeNameList(IgsYYvsp[-5].Namelist);
                    myfree(IgsYYvsp[-2].Name);
                ;
    break;}
case 217:
#line 1035 "./gointer.y"
{ IgsYYval.Value = IgsYYvsp[-1].Value+1; gamesSeen++; ;
    break;}
case 218:
#line 1036 "./gointer.y"
{ IgsYYval.Value = 0; gamesSeen = 0; ;
    break;}
case 219:
#line 1040 "./gointer.y"
{
                    AssertGamesDeleted();
                ;
    break;}
case 220:
#line 1044 "./gointer.y"
{
                    TestGamesDeleted(gamesSeen);
                ;
    break;}
case 221:
#line 1049 "./gointer.y"
{ UnObserve(IgsYYvsp[0].Value); ;
    break;}
case 222:
#line 1053 "./gointer.y"
{
                    char     Num[20], *ptr;
                    NameVal *nameval;
                    IgsYYval.Nameval = nameval = mynew(NameVal);
                    sprintf(Num, "%d", IgsYYvsp[-1].Value);
                    nameval->Next  = nameval->Previous = nameval;
                    nameval->Name  = mystrdup(Num);
                    ptr = IgsYYvsp[0].Name;
                    /* Get rid of extra `removed stones' entries.
                       Maybe I ought to compare them with what I work out.. */
                    while (*ptr && !isspace(*ptr)) ptr++;
                    if (ptr-IgsYYvsp[0].Name <= 3) *ptr = 0;
                    nameval->Value = IgsYYvsp[0].Name;
                ;
    break;}
case 223:
#line 1070 "./gointer.y"
{
                    IgsYYval.Nameval = IgsYYvsp[-1].Nameval;
                    IgsYYvsp[0].Nameval->Previous = IgsYYvsp[-1].Nameval->Previous;
                    IgsYYvsp[0].Nameval->Next     = IgsYYvsp[-1].Nameval;
                    IgsYYvsp[0].Nameval->Previous->Next = IgsYYvsp[0].Nameval->Next->Previous = IgsYYvsp[0].Nameval;
                ;
    break;}
case 224:
#line 1077 "./gointer.y"
{
                    NameVal *nameval;

                    IgsYYval.Nameval = nameval = mynew(NameVal);
                    nameval->Next  = nameval->Previous = nameval;
                    nameval->Name  = NULL;
                    nameval->Value = NULL;
                ;
    break;}
case 225:
#line 1087 "./gointer.y"
{;
    break;}
case 226:
#line 1088 "./gointer.y"
{;
    break;}
case 227:
#line 1093 "./gointer.y"
{
                    if (strcmp(IgsYYvsp[-7].Name, "vs")) YYFAIL;

                    IgsYYval.Gamedesc = mynew(GameDesc);
                    IgsYYval.Gamedesc->Id            = IgsYYvsp[-14].Value;
                    IgsYYval.Gamedesc->BlackName     = IgsYYvsp[-6].Name;
                    IgsYYval.Gamedesc->BlackName2    = 0;
                    IgsYYval.Gamedesc->BlackCaptures = atoi(IgsYYvsp[-4].Name);
                    IgsYYval.Gamedesc->BlackTime     = atoi(IgsYYvsp[-3].Name);
                    IgsYYval.Gamedesc->BlackByo      = atoi(IgsYYvsp[-2].Name);
                    IgsYYval.Gamedesc->WhiteName     = IgsYYvsp[-13].Name;
                    IgsYYval.Gamedesc->WhiteName2    = 0;
                    IgsYYval.Gamedesc->WhiteCaptures = atoi(IgsYYvsp[-11].Name);
                    IgsYYval.Gamedesc->WhiteTime     = atoi(IgsYYvsp[-10].Name);
                    IgsYYval.Gamedesc->WhiteByo      = atoi(IgsYYvsp[-9].Name);
                    myfree(IgsYYvsp[-11].Name);
                    myfree(IgsYYvsp[-10].Name);
                    myfree(IgsYYvsp[-9].Name);
                    myfree(IgsYYvsp[-7].Name);
                    myfree(IgsYYvsp[-4].Name);
                    myfree(IgsYYvsp[-3].Name);
                    myfree(IgsYYvsp[-2].Name);
                ;
    break;}
case 228:
#line 1120 "./gointer.y"
{
		    Game *game;
                    if (strcmp(IgsYYvsp[-13].Name, "vs")) YYFAIL;

                    IgsYYval.Gamedesc = mynew(GameDesc);
                    IgsYYval.Gamedesc->Id            = IgsYYvsp[-20].Value;
                    IgsYYval.Gamedesc->BlackName     = IgsYYvsp[-12].Name;
                    IgsYYval.Gamedesc->BlackName2    = IgsYYvsp[-2].Name;
                    IgsYYval.Gamedesc->BlackCaptures = atoi(IgsYYvsp[-10].Name);
                    IgsYYval.Gamedesc->BlackTime     = atoi(IgsYYvsp[-9].Name);
                    IgsYYval.Gamedesc->BlackByo      = atoi(IgsYYvsp[-8].Name);
                    IgsYYval.Gamedesc->WhiteName     = IgsYYvsp[-19].Name;
                    IgsYYval.Gamedesc->WhiteName2    = IgsYYvsp[-1].Name;
                    IgsYYval.Gamedesc->WhiteCaptures = atoi(IgsYYvsp[-17].Name);
                    IgsYYval.Gamedesc->WhiteTime     = atoi(IgsYYvsp[-16].Name);
                    IgsYYval.Gamedesc->WhiteByo      = atoi(IgsYYvsp[-15].Name);
		    /* We must create the game now, since the "games Id"
                     * command will not give the 3rd and 4th players:
                     */
                    TeamGame(IgsYYval.Gamedesc->Id, IgsYYvsp[-4].Name, IgsYYvsp[-3].Name, IgsYYvsp[-2].Name, IgsYYvsp[-1].Name, 0);
                    myfree(IgsYYvsp[-17].Name);
                    myfree(IgsYYvsp[-16].Name);
                    myfree(IgsYYvsp[-15].Name);
                    myfree(IgsYYvsp[-13].Name);
                    myfree(IgsYYvsp[-10].Name);
                    myfree(IgsYYvsp[-9].Name);
                    myfree(IgsYYvsp[-8].Name);
                    myfree(IgsYYvsp[-4].Name);
                    myfree(IgsYYvsp[-3].Name);
                ;
    break;}
case 229:
#line 1152 "./gointer.y"
{ IgsYYval.Name = IgsYYvsp[0].Name; ;
    break;}
case 230:
#line 1153 "./gointer.y"
{ IgsYYval.Name = 0;  ;
    break;}
case 231:
#line 1156 "./gointer.y"
{ SeenAdd = 1; ;
    break;}
case 232:
#line 1160 "./gointer.y"
{
                    Done();
                ;
    break;}
case 233:
#line 1166 "./gointer.y"
{
                    MyGameMessage(IgsYYvsp[0].Name);
                    myfree(IgsYYvsp[0].Name);
                ;
    break;}
case 234:
#line 1173 "./gointer.y"
{
                    MyGameMessage(IgsYYvsp[0].Name);
                    myfree(IgsYYvsp[0].Name);
                ;
    break;}
case 235:
#line 1180 "./gointer.y"
{
                    MyGameMessage("There is a disagreement about the "
                                  "life/death of that stone. "
                                  "The game will resume.");
                    RestoreFromScoring();
                    MyGameMessage("Board is restored to what it was"
                                  " before you started scoring");
                ;
    break;}
case 236:
#line 1191 "./gointer.y"
{
                    MyGameMessage("There is a disagreement about the "
                                  "life/death of that stone. "
                                  "The game will resume.");
                    RestoreFromScoring();
                ;
    break;}
case 237:
#line 1200 "./gointer.y"
{ /* teach invalid-game. The gamdesc will force "games"
                       * since teaching games are not announced.
                       */
		      Output("Game not found\n");
                    ;
    break;}
case 239:
#line 1209 "./gointer.y"
{
                    Game *game;
                    int   Nr;

		    /* Do not rely on WhatCommand(NULL, "moves"), this creates
                     * a race condition. Instead assume that the movelist
		     * is a result of "moves <id>" if it starts with move 0.
                     */
		    if (SeenAdd || IgsYYvsp[-3].Nameval->Next == IgsYYvsp[-3].Nameval ||
			strcmp(IgsYYvsp[-3].Nameval->Next->Name, "0")) {
                        game = AddMove(SeenAdd, IgsYYvsp[-4].Gamedesc, IgsYYvsp[-3].Nameval);
/*                      if (game && !MyGameP(game)) ChangeCommand(NULL, -1); */
                        if (game) SetGameTitle(game, IgsYYvsp[-2].Name);
                        myfree(IgsYYvsp[-2].Name);
                    } else {
		        Moves(IgsYYvsp[-4].Gamedesc, IgsYYvsp[-3].Nameval);
		    }
                    FreeGameDesc(IgsYYvsp[-4].Gamedesc);
                    FreeNameValList(IgsYYvsp[-3].Nameval);
                ;
    break;}
case 240:
#line 1232 "./gointer.y"
{
                    Game *game;
                    int   Nr;

		    IgsYYvsp[-10].Gamedesc->BlackName2 = strcmp(IgsYYvsp[-10].Gamedesc->BlackName, IgsYYvsp[-7].Name) ? IgsYYvsp[-7].Name : IgsYYvsp[-5].Name;
		    IgsYYvsp[-10].Gamedesc->WhiteName2 = strcmp(IgsYYvsp[-10].Gamedesc->WhiteName, IgsYYvsp[-6].Name) ? IgsYYvsp[-6].Name : IgsYYvsp[-4].Name;
		    if (SeenAdd || IgsYYvsp[-9].Nameval->Next == IgsYYvsp[-9].Nameval ||
			strcmp(IgsYYvsp[-9].Nameval->Next->Name, "0")) {
                        game = AddMove(SeenAdd, IgsYYvsp[-10].Gamedesc, IgsYYvsp[-9].Nameval);
/*                      if (game && !MyGameP(game)) ChangeCommand(NULL, -1); */
                        if (game) SetGameTitle(game, IgsYYvsp[-1].Name);
                        myfree(IgsYYvsp[-1].Name);
                    } else {
		        Moves(IgsYYvsp[-10].Gamedesc, IgsYYvsp[-9].Nameval);
		    }
                    FreeGameDesc(IgsYYvsp[-10].Gamedesc);
                    FreeNameValList(IgsYYvsp[-9].Nameval);
                ;
    break;}
case 241:
#line 1252 "./gointer.y"
{ IgsYYval.Value = IgsYYvsp[0].Value; ;
    break;}
case 242:
#line 1253 "./gointer.y"
{ IgsYYval.Value = Empty; ;
    break;}
case 243:
#line 1293 "./gointer.y"
{
                    Done();
                    if (IgsYYvsp[-2].Value != Empty)
                        MyGameMessage("%s needs to remove a group first.",
                                      IgsYYvsp[-2].Value == White ? "White" : "Black");
                ;
    break;}
case 244:
#line 1302 "./gointer.y"
{
                    Game *game;

                    game = AddMove(0, IgsYYvsp[-4].Gamedesc, IgsYYvsp[-3].Nameval);
                    FreeGameDesc(IgsYYvsp[-4].Gamedesc);
                    FreeNameValList(IgsYYvsp[-3].Nameval);
                    if (game) SetGameTitle(game, IgsYYvsp[-2].Name);
                    myfree(IgsYYvsp[-2].Name);
                ;
    break;}
case 245:
#line 1314 "./gointer.y"
{
                    Game *game;

                    game = AddMove(0, IgsYYvsp[-4].Gamedesc, IgsYYvsp[-3].Nameval);
                    FreeGameDesc(IgsYYvsp[-4].Gamedesc);
                    FreeNameValList(IgsYYvsp[-3].Nameval);
                    if (game) SetGameTitle(game, IgsYYvsp[-2].Name);
                    myfree(IgsYYvsp[-2].Name);
                ;
    break;}
case 246:
#line 1324 "./gointer.y"
{
                    Game *game;

                    game = AddMove(0, IgsYYvsp[-4].Gamedesc, IgsYYvsp[-3].Nameval);
                    FreeGameDesc(IgsYYvsp[-4].Gamedesc);
                    FreeNameValList(IgsYYvsp[-3].Nameval);
                    if (game) SetGameTitle(game, IgsYYvsp[-2].Name);
                    myfree(IgsYYvsp[-2].Name);
                ;
    break;}
case 247:
#line 1336 "./gointer.y"
{
                    AutoCommand(NULL, "%%bet bet");
                ;
    break;}
case 248:
#line 1340 "./gointer.y"
{
                    AutoCommand(NULL, "%%bet bet");
                ;
    break;}
case 249:
#line 1346 "./gointer.y"
{
                    /* undo of multiple moves allowed in a teaching game */
		    MyGameUndo(IgsYYvsp[0].Name);
                    myfree(IgsYYvsp[0].Name);
                ;
    break;}
case 250:
#line 1352 "./gointer.y"
{
		    MyGameUndo(IgsYYvsp[0].Name);
                    myfree(IgsYYvsp[0].Name);
                ;
    break;}
case 251:
#line 1360 "./gointer.y"
{
                    FreeGameDesc(IgsYYvsp[-3].Gamedesc);
                    FreeNameValList(IgsYYvsp[-2].Nameval);
                    /* if (game) SetGameTitle(game, $4); */
                    myfree(IgsYYvsp[-1].Name);
                ;
    break;}
case 252:
#line 1368 "./gointer.y"
{
                    FreeGameDesc(IgsYYvsp[-9].Gamedesc);
                    FreeNameValList(IgsYYvsp[-8].Nameval);
                    /* if (game) SetGameTitle(game, $10); */
                    myfree(IgsYYvsp[-1].Name);
                ;
    break;}
case 253:
#line 1378 "./gointer.y"
{
                    char *ptr;

                    ptr = strchr(IgsYYvsp[-4].Name, ')');
                    if (ptr) *ptr = 0;
                    else YYFAIL;
                    Undo(0, IgsYYvsp[-2].Gamedesc->Id, IgsYYvsp[-2].Gamedesc->BlackName, IgsYYvsp[-2].Gamedesc->WhiteName, IgsYYvsp[-4].Name);
                    myfree(IgsYYvsp[-4].Name);
                    FreeGameDesc(IgsYYvsp[-2].Gamedesc);
                    FreeNameValList(IgsYYvsp[-1].Nameval);
                ;
    break;}
case 254:
#line 1391 "./gointer.y"
{
                    char *ptr;

                    ptr = strchr(IgsYYvsp[-10].Name, ')');
                    if (ptr) *ptr = 0;
                    else YYFAIL;
                    Undo(0, IgsYYvsp[-8].Gamedesc->Id, IgsYYvsp[-8].Gamedesc->BlackName, IgsYYvsp[-8].Gamedesc->WhiteName, IgsYYvsp[-10].Name);
                    myfree(IgsYYvsp[-10].Name);
                    FreeGameDesc(IgsYYvsp[-8].Gamedesc);
                    FreeNameValList(IgsYYvsp[-7].Nameval);
                ;
    break;}
case 255:
#line 1404 "./gointer.y"
{;
    break;}
case 256:
#line 1408 "./gointer.y"
{
                    Resume(IgsYYvsp[-2].Gamedesc->Id, IgsYYvsp[-2].Gamedesc->BlackName, IgsYYvsp[-2].Gamedesc->WhiteName,
                           IgsYYvsp[-1].Nameval->Previous->Name ?
                           1+atoi(IgsYYvsp[-1].Nameval->Previous->Name) : 0);
                    FreeGameDesc(IgsYYvsp[-2].Gamedesc);
                    FreeNameValList(IgsYYvsp[-1].Nameval);
                ;
    break;}
case 257:
#line 1417 "./gointer.y"
{
                    Game *game;
		    /* We must create the game now, since the "games Id"
                     * command will not give the 3rd and 4th players:
		     */
                    game = ResumeTeam(IgsYYvsp[-8].Gamedesc->Id, IgsYYvsp[-5].Name, IgsYYvsp[-4].Name, IgsYYvsp[-3].Name, IgsYYvsp[-2].Name,
                                  IgsYYvsp[-7].Nameval->Previous->Name ?
                                  1+atoi(IgsYYvsp[-7].Nameval->Previous->Name) : 0);
                    if (game) SetGameTitle(game, IgsYYvsp[0].Dummy);
                    FreeGameDesc(IgsYYvsp[-8].Gamedesc);
                    FreeNameValList(IgsYYvsp[-7].Nameval);
                    myfree(IgsYYvsp[0].Dummy);
                ;
    break;}
case 258:
#line 1433 "./gointer.y"
{
                    Game *game;

                    game = Resume(IgsYYvsp[-2].Gamedesc->Id, IgsYYvsp[-2].Gamedesc->BlackName, IgsYYvsp[-2].Gamedesc->WhiteName,
                                  IgsYYvsp[-1].Nameval->Previous->Name ?
                                  1+atoi(IgsYYvsp[-1].Nameval->Previous->Name) : 0);
                    if (game) SetGameTitle(game, IgsYYvsp[0].Name);
                    FreeGameDesc(IgsYYvsp[-2].Gamedesc);
                    FreeNameValList(IgsYYvsp[-1].Nameval);
                    myfree(IgsYYvsp[0].Name);
                ;
    break;}
case 259:
#line 1446 "./gointer.y"
{
                    Game *game;
		    /* We must create the game now, since the "games Id"
                     * command will not give the 3rd and 4th players:
		     */
                    game = ResumeTeam(IgsYYvsp[-8].Gamedesc->Id, IgsYYvsp[-5].Name, IgsYYvsp[-4].Name, IgsYYvsp[-3].Name, IgsYYvsp[-2].Name,
                                  IgsYYvsp[-7].Nameval->Previous->Name ?
                                  1+atoi(IgsYYvsp[-7].Nameval->Previous->Name) : 0);
                    if (game) SetGameTitle(game, IgsYYvsp[0].Name);
                    FreeGameDesc(IgsYYvsp[-8].Gamedesc);
                    FreeNameValList(IgsYYvsp[-7].Nameval);
                    myfree(IgsYYvsp[0].Name);
                ;
    break;}
case 260:
#line 1461 "./gointer.y"
{;
    break;}
case 261:
#line 1465 "./gointer.y"
{
                    SendCommand(NULL, INT_TO_XTPOINTER(IgsYYvsp[-1].Gamedesc->Id+1),
				"games %d", IgsYYvsp[-1].Gamedesc->Id);
                    /* INT_TO_XTPOINTER(Id+1) will set ForceNew = Id
                     in AssertGamesDeleted() */
                    FreeGameDesc(IgsYYvsp[-1].Gamedesc);
                ;
    break;}
case 262:
#line 1475 "./gointer.y"
{
                    Decline(IgsYYvsp[0].Person);
                ;
    break;}
case 263:
#line 1481 "./gointer.y"
{
                    IgsYYval.Disputedesc = mynew(DisputeDesc);
                    IgsYYval.Disputedesc->Player = IgsYYvsp[-5].Person;
                    IgsYYval.Disputedesc->Color  = IgsYYvsp[-4].Value;
                    IgsYYval.Disputedesc->SizeX  = atoi(IgsYYvsp[-3].Name);
                    IgsYYval.Disputedesc->SizeY  = atoi(strchr(IgsYYvsp[-3].Name, 'x')+1);
                    IgsYYval.Disputedesc->Tim    = IgsYYvsp[-2].Value;
                    IgsYYval.Disputedesc->ByoYomi= IgsYYvsp[-1].Value;
                    myfree(IgsYYvsp[-3].Name);
                ;
    break;}
case 264:
#line 1494 "./gointer.y"
{
                    IgsYYvsp[0].Disputedesc->Next     = IgsYYvsp[-1].Disputedesc;
                    IgsYYvsp[0].Disputedesc->Previous = IgsYYvsp[-1].Disputedesc->Previous;
                    IgsYYvsp[0].Disputedesc->Next->Previous = IgsYYvsp[0].Disputedesc->Previous->Next = IgsYYvsp[0].Disputedesc;
                    IgsYYval.Disputedesc = IgsYYvsp[-1].Disputedesc;
                ;
    break;}
case 265:
#line 1501 "./gointer.y"
{
                    IgsYYval.Disputedesc = mynew(DisputeDesc);
                    IgsYYval.Disputedesc->Next = IgsYYval.Disputedesc->Previous = IgsYYval.Disputedesc;
                ;
    break;}
case 266:
#line 1508 "./gointer.y"
{
                    DisputeDesc *Here, *Next;

                    Dispute(IgsYYvsp[0].Disputedesc, 1);
                    for (Here = IgsYYvsp[0].Disputedesc->Next; Here != IgsYYvsp[0].Disputedesc; Here = Next) {
                        Next = Here->Next;
                        myfree(Here);
                    }
                    myfree(IgsYYvsp[0].Disputedesc);
                ;
    break;}
case 267:
#line 1521 "./gointer.y"
{
                    DisputeDesc *Here, *Next;

                    Dispute(IgsYYvsp[0].Disputedesc, 0);
                    for (Here = IgsYYvsp[0].Disputedesc->Next; Here != IgsYYvsp[0].Disputedesc; Here = Next) {
                        Next = Here->Next;
                        myfree(Here);
                    }
                    myfree(IgsYYvsp[0].Disputedesc);
                ;
    break;}
case 268:
#line 1534 "./gointer.y"
{
                    IgsYYval.Value = IgsYYvsp[-1].Value | IgsYYvsp[0].Value;
                ;
    break;}
case 269:
#line 1538 "./gointer.y"
{
                    IgsYYval.Value = 0;
                ;
    break;}
case 270:
#line 1544 "./gointer.y"
{
                    WantMatchType(IgsYYvsp[-2].Person, IgsYYvsp[-1].Value);
                ;
    break;}
case 271:
#line 1550 "./gointer.y"
{
                ;
    break;}
case 273:
#line 1557 "./gointer.y"
{             /* gameid white black move */
		    Undo(0, IgsYYvsp[-3].Value, IgsYYvsp[-1].Name, IgsYYvsp[-2].Name, IgsYYvsp[0].Name);
                    myfree(IgsYYvsp[0].Name);
                    myfree(IgsYYvsp[-1].Name);
                    myfree(IgsYYvsp[-2].Name);
                ;
    break;}
case 274:
#line 1564 "./gointer.y"
{
		    Undo(0, IgsYYvsp[-3].Value, IgsYYvsp[-1].Name, IgsYYvsp[-2].Name, IgsYYvsp[0].Name);
                    myfree(IgsYYvsp[0].Name);
                    myfree(IgsYYvsp[-1].Name);
                    myfree(IgsYYvsp[-2].Name);
                ;
    break;}
case 275:
#line 1573 "./gointer.y"
{
                    FreeGameDesc(IgsYYvsp[-2].Gamedesc);
                    FreeNameValList(IgsYYvsp[-1].Nameval);
                    /* if (game && $5) SetGameTitle(game, $5); */
                    myfree(IgsYYvsp[0].Name);
                ;
    break;}
case 276:
#line 1581 "./gointer.y"
{
                    FreeGameDesc(IgsYYvsp[-8].Gamedesc);
                    FreeNameValList(IgsYYvsp[-7].Nameval);
                    /* if (game && $11) SetGameTitle(game, $11); */
                    myfree(IgsYYvsp[0].Name);
                ;
    break;}
case 277:
#line 1590 "./gointer.y"
{
                    Watching(IgsYYvsp[-1].Namelist);
                    FreeNameList(IgsYYvsp[-1].Namelist);
                ;
    break;}
case 278:
#line 1597 "./gointer.y"
{
                    OverObserve(IgsYYvsp[0].Value);
                ;
    break;}
case 279:
#line 1603 "./gointer.y"
{
                    ObserveWhilePlaying();
                ;
    break;}
case 280:
#line 1609 "./gointer.y"
{
                    FindPlayer(IgsYYvsp[-3].Name, IgsYYvsp[-1].Name, IgsYYvsp[-4].Name, IgsYYvsp[-2].Name);
                    myfree(IgsYYvsp[-4].Name);
                    myfree(IgsYYvsp[-3].Name);
                    myfree(IgsYYvsp[-2].Name);
                    myfree(IgsYYvsp[-1].Name);
                ;
    break;}
case 281:
#line 1617 "./gointer.y"
{
                    FindPlayer(IgsYYvsp[-1].Name, "???", "?????  ???", UNKNOWN);
                    myfree(IgsYYvsp[-1].Name);
                ;
    break;}
case 283:
#line 1625 "./gointer.y"
{

                ;
    break;}
case 284:
#line 1631 "./gointer.y"
{
                    PlayerStatusLine(atoi(IgsYYvsp[-10].Name), atoi(IgsYYvsp[-8].Name), atoi(IgsYYvsp[-4].Name));
                    myfree(IgsYYvsp[-10].Name);
                    myfree(IgsYYvsp[-8].Name);
                    myfree(IgsYYvsp[-6].Name);
                    myfree(IgsYYvsp[-5].Name);
                    myfree(IgsYYvsp[-4].Name);
                    myfree(IgsYYvsp[-3].Name);
                    myfree(IgsYYvsp[-2].Name);
                    myfree(IgsYYvsp[-1].Name);
                ;
    break;}
case 285:
#line 1643 "./gointer.y"
{
                    PlayerStatusLine(atoi(IgsYYvsp[-6].Name), -1, atoi(IgsYYvsp[-4].Name));
                    myfree(IgsYYvsp[-6].Name);
                    myfree(IgsYYvsp[-5].Name);
                    myfree(IgsYYvsp[-4].Name);
                    myfree(IgsYYvsp[-3].Name);
                    myfree(IgsYYvsp[-2].Name);
                    myfree(IgsYYvsp[-1].Name);
                ;
    break;}
case 286:
#line 1655 "./gointer.y"
{
                    AssertPlayersDeleted();
                ;
    break;}
case 287:
#line 1659 "./gointer.y"
{
                    TestPlayersDeleted();
                ;
    break;}
case 289:
#line 1666 "./gointer.y"
{
                    NameList *Here;
                    int n;

                    n =0;
                    for (Here = IgsYYvsp[-1].Namelist->Next; Here != IgsYYvsp[-1].Namelist; Here = Here->Next) n++;
                    if (n == 11) {
		      IgsYYval.Namelist = IgsYYvsp[-1].Namelist;
		    } else if (n == 0) { /* list header */
		      IgsYYval.Namelist = NULL;
                    } else {
                        /* Don't call YYFAIL. user command leads to easily
                           to parse errors */
		        Output("Got the expected parse error following a "
                               "\"user\" command:\n");
                        for (Here = IgsYYvsp[-1].Namelist->Next; Here != IgsYYvsp[-1].Namelist; Here = Here->Next) {
                            Output(Here->Name);
                            Output(" ");
                        }
                        Output("\n");
                        _IgsDefaultParse();
                        FreeNameList(IgsYYvsp[-1].Namelist);
                        IgsYYval.Namelist = NULL;
                    }
                ;
    break;}
case 290:
#line 1692 "./gointer.y"
{
                    NameList *Here;

                    Output("Got the expected parse failure following a "
                           "\"user\" command:\n");
                    for (Here = IgsYYvsp[-1].Namelist->Next; Here != IgsYYvsp[-1].Namelist; Here = Here->Next) {
                        Output(Here->Name);
                        Output(" ");
                    }
                    Output("\n");
                    _IgsDefaultParse();
                    FreeNameList(IgsYYvsp[-1].Namelist);
                    IgsYYval.Namelist = NULL;
                ;
    break;}
case 291:
#line 1709 "./gointer.y"
{
                    if (IgsYYvsp[0].Namelist) {
                        NameListList *Last;
                        Last = mynew(NameListList);
                        Last->Names    = IgsYYvsp[0].Namelist;
                        Last->Previous = IgsYYvsp[-1].NameListlist->Previous;
                        Last->Next     = IgsYYvsp[-1].NameListlist;
                        Last->Next->Previous = Last->Previous->Next = Last;
                    }
                    IgsYYval.NameListlist = IgsYYvsp[-1].NameListlist;
                ;
    break;}
case 292:
#line 1721 "./gointer.y"
{
                    IgsYYval.NameListlist = mynew(NameListList);
                    IgsYYval.NameListlist->Previous = IgsYYval.NameListlist->Next = IgsYYval.NameListlist;
                    IgsYYval.NameListlist->Names = NULL;
                ;
    break;}
case 293:
#line 1729 "./gointer.y"
{
                    UserData(IgsYYvsp[0].NameListlist);
                    FreeNameListList(IgsYYvsp[0].NameListlist);
                ;
    break;}
case 294:
#line 1736 "./gointer.y"
{
                    IgsYYval.Person = FindPlayerByNameAndStrength(IgsYYvsp[-3].Name, IgsYYvsp[-1].Name);
                    myfree(IgsYYvsp[-3].Name);
                    myfree(IgsYYvsp[-1].Name);
                ;
    break;}
case 295:
#line 1744 "./gointer.y"
{
                    int sec;

                    sec = atoi(IgsYYvsp[0].Name);
                    if (IgsYYvsp[-2].Name[0] == '-') IgsYYval.Value = -60 * atoi(IgsYYvsp[-2].Name+1)-sec;
                    else              IgsYYval.Value =  60 * atoi(IgsYYvsp[-2].Name)  +sec;
                    myfree(IgsYYvsp[-2].Name);
                    myfree(IgsYYvsp[0].Name);
                ;
    break;}
case 296:
#line 1756 "./gointer.y"
{
                    IgsYYval.Value = atoi(IgsYYvsp[0].Name);
                    myfree(IgsYYvsp[-2].Name);
                    myfree(IgsYYvsp[0].Name);
                ;
    break;}
case 297:
#line 1761 "./gointer.y"
{ IgsYYval.Value = -1; ;
    break;}
case 298:
#line 1767 "./gointer.y"
{
                    if (strcmp(IgsYYvsp[-21].Name, "Game") ||
                        strcmp(IgsYYvsp[-16].Name, "White") || strcmp(IgsYYvsp[-7].Name, "Black")) YYFAIL;
                    GameTime(atoi(IgsYYvsp[-19].Name), IgsYYvsp[-5].Name, IgsYYvsp[-2].Value, IgsYYvsp[-1].Value, IgsYYvsp[-14].Name, IgsYYvsp[-11].Value, IgsYYvsp[-10].Value);
                    myfree(IgsYYvsp[-21].Name);
                    myfree(IgsYYvsp[-19].Name);
                    myfree(IgsYYvsp[-16].Name);
                    myfree(IgsYYvsp[-14].Name);
                    myfree(IgsYYvsp[-7].Name);
                    myfree(IgsYYvsp[-5].Name);
                ;
    break;}
case 299:
#line 1781 "./gointer.y"
{
                    int Nr;
                    Game     *game;

                    if (!UserCommandP(NULL) &&
                        (Nr = WhatCommand(NULL, "score")) >= 0 &&
                        (game = ServerIdToGame(Nr)) != NULL) {
                        GameMessage(game, "..........", "Current score:");
                        GameMessage(game, "..........", "%s", IgsYYvsp[-2].Name);
                        GameMessage(game, "..........", "Final score:");
                        GameMessage(game, "..........", "%s", IgsYYvsp[0].Name);
                    } else {
                        Outputf("Current score:\n %s\n", IgsYYvsp[-2].Name);
                        Outputf("Final score:\n %s\n", IgsYYvsp[0].Name);
                    }
                    myfree(IgsYYvsp[-2].Name);
                    myfree(IgsYYvsp[0].Name);
                ;
    break;}
case 300:
#line 1802 "./gointer.y"
{
                    Outputf("%s\n", IgsYYvsp[0].Name);
                    myfree(IgsYYvsp[0].Name);
                ;
    break;}
case 303:
#line 1813 "./gointer.y"
{
                    MyGameMessage("%s is now in byo-yomi, having %s",
                                  PlayerString(IgsYYvsp[-1].Person), IgsYYvsp[0].Name);
                    myfree(IgsYYvsp[0].Name);
                ;
    break;}
case 304:
#line 1821 "./gointer.y"
{
                    MyGameMessage("%s has run out of time.", PlayerString(IgsYYvsp[0].Person));
                ;
    break;}
case 305:
#line 1827 "./gointer.y"
{
                    MyGameMessage("Your opponent has lost his connection.");
                ;
    break;}
case 306:
#line 1833 "./gointer.y"
{
		    if (appdata.WantVerbose) {
                        MyGameMessage("Game saved.%s", IgsYYvsp[-1].Name);
		    }
                    myfree(IgsYYvsp[-1].Name);
                ;
    break;}
case 307:
#line 1842 "./gointer.y"
{
                ;
    break;}
case 309:
#line 1849 "./gointer.y"
{
                    MyGameMessage("Game has been adjourned.");
                    MyGameMessage("Game saved.%s", IgsYYvsp[-1].Name);
                    myfree(IgsYYvsp[-1].Name);
                ;
    break;}
case 310:
#line 1857 "./gointer.y"
{
                ;
    break;}
case 311:
#line 1862 "./gointer.y"
{
                    MyGameMessage("Your opponent requests an adjournment");
                    MyGameMessage("Use the <adjourn> or <decline adjourn> "
                                  "entries in the commands menu.");
                ;
    break;}
case 312:
#line 1870 "./gointer.y"
{
                    MyGameMessage("Game has been adjourned.");
                ;
    break;}
case 313:
#line 1876 "./gointer.y"
{
                    MyGameMessage("Your opponent declines to adjourn.");
                ;
    break;}
case 314:
#line 1882 "./gointer.y"
{
                    MyGameMessage("%s has resigned the game.",
                                  PlayerString(IgsYYvsp[0].Person));
                ;
    break;}
case 315:
#line 1887 "./gointer.y"
{ /* Double message in teaching game --Ton */
                    MyGameMessage("%s has resigned the game.",
                                  PlayerString(IgsYYvsp[-1].Person));
                ;
    break;}
case 316:
#line 1894 "./gointer.y"
{
                   Mailed(IgsYYvsp[-1].Name);
                   Mailed(IgsYYvsp[0].Name);
                   myfree(IgsYYvsp[-1].Name);
                   myfree(IgsYYvsp[0].Name);
               ;
    break;}
case 317:
#line 1901 "./gointer.y"
{
                   Mailed(IgsYYvsp[0].Name);
                   myfree(IgsYYvsp[0].Name);
               ;
    break;}
case 318:
#line 1908 "./gointer.y"
{
                   RemoveGameFile(IgsYYvsp[0].Name);
                   myfree(IgsYYvsp[0].Name);
               ;
    break;}
case 319:
#line 1915 "./gointer.y"
{
                    NoTell();
                ;
    break;}
case 320:
#line 1921 "./gointer.y"
{
                    /* Kludge to stop bell/raise at telltarget change --Ton */
                    int OldEntered;

                    OldEntered = Entered;
                    Entered = 0;
		    if (appdata.WantVerbose) {
                        Outputf("Setting your '.' to %16s\n",
                                PlayerNameToString(IgsYYvsp[-1].Name));
		    }
                    Entered = OldEntered;
                    myfree(IgsYYvsp[-1].Name);
                ;
    break;}
case 321:
#line 1937 "./gointer.y"
{
                ;
    break;}
case 322:
#line 1942 "./gointer.y"
{
                    NameList *Here;

                    Output("User is not accepting tells.\n");
                    for (Here = IgsYYvsp[0].Namelist->Next; Here != IgsYYvsp[0].Namelist; Here = Here->Next)
                        Outputf("%s\n", Here->Name);
                    FreeNameList(IgsYYvsp[0].Namelist);
                ;
    break;}
case 323:
#line 1953 "./gointer.y"
{
                    MyGameMessage("Illegal move: %s", IgsYYvsp[0].Name);
                    myfree(IgsYYvsp[0].Name);
                ;
    break;}
case 324:
#line 1958 "./gointer.y"
{
                    MyGameMessage("Illegal move: %s", IgsYYvsp[-2].Name);
                    myfree(IgsYYvsp[-2].Name);
                ;
    break;}
case 325:
#line 1965 "./gointer.y"
{
                    MyGameMessage("Cannot undo: %s", IgsYYvsp[0].Name);
                    myfree(IgsYYvsp[0].Name);
                ;
    break;}
case 326:
#line 1972 "./gointer.y"
{
                    MyGameMessage("It isn't your turn");
                ;
    break;}
case 327:
#line 1978 "./gointer.y"
{
                    MyGameMessage("It is not your turn to remove a group");
                ;
    break;}
case 328:
#line 1984 "./gointer.y"
{
                    MyGameMessage("To resign, please use 'resign'");
                ;
    break;}
case 329:
#line 1990 "./gointer.y"
{
                    MyGameMessage("You cannot remove liberties.");
                ;
    break;}
case 330:
#line 1996 "./gointer.y"
{
                    RemoveGroup(IgsYYvsp[0].Name);
                    myfree(IgsYYvsp[0].Name);
                ;
    break;}
case 331:
#line 2003 "./gointer.y"
{
                    RestoreScoring();
                ;
    break;}
case 332:
#line 2009 "./gointer.y"
{
                    MyGameMessage("Please repeat 'done'");
                ;
    break;}
case 333:
#line 2015 "./gointer.y"
{
                    IgsYYval.Namelist = IgsYYvsp[-1].Namelist;
                ;
    break;}
case 334:
#line 2021 "./gointer.y"
{
                    IgsYYval.Numval = mynew(NumVal);
                    IgsYYval.Numval->Num   = IgsYYvsp[-1].Value;
                    IgsYYval.Numval->Value = IgsYYvsp[0].Name;
                ;
    break;}
case 335:
#line 2029 "./gointer.y"
{
                    IgsYYvsp[0].Numval->Next = IgsYYvsp[-1].Numval;
                    IgsYYvsp[0].Numval->Previous = IgsYYvsp[-1].Numval->Previous;
                    IgsYYvsp[0].Numval->Next->Previous = IgsYYvsp[0].Numval->Previous->Next = IgsYYvsp[0].Numval;
                    IgsYYval.Numval = IgsYYvsp[-1].Numval;
                ;
    break;}
case 336:
#line 2036 "./gointer.y"
{
                    IgsYYval.Numval = mynew(NumVal);
                    IgsYYval.Numval->Next  = IgsYYval.Numval->Previous = IgsYYval.Numval;
                    IgsYYval.Numval->Num   = -1;
                    IgsYYval.Numval->Value = NULL;
                ;
    break;}
case 337:
#line 2045 "./gointer.y"
{
	            /* 20 jl (W:O):  2.5 to jloup (B:#):  3.0 */
                    MyGameMessage("%s", IgsYYvsp[0].Name);
                    myfree(IgsYYvsp[0].Name);
                ;
    break;}
case 338:
#line 2053 "./gointer.y"
{
                    if (GamePosition(IgsYYvsp[-1].Namelist, IgsYYvsp[-2].Namelist, IgsYYvsp[0].Numval)) ChangeCommand(NULL, 1);
                    FreeNameList(IgsYYvsp[-2].Namelist);
                    FreeNameList(IgsYYvsp[-1].Namelist);
                    FreeNumValList(IgsYYvsp[0].Numval);
                ;
    break;}
case 339:
#line 2062 "./gointer.y"
{
                    struct tm *FullTime;
                    int        i;
                    static const char *Month[] = {
                        "Jan", "Feb", "Mar", "Apr", "May", "Jun",
                        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

                    FullTime = mynew(struct tm);
                    for (i=0; i<12; i++) if (strcmp(IgsYYvsp[-7].Name, Month[i]) == 0) break;
                    FullTime->tm_year  = atoi(IgsYYvsp[0].Name)-1900;
                    FullTime->tm_mon   = i;
                    FullTime->tm_mday  = atoi(IgsYYvsp[-6].Name);
                    FullTime->tm_hour  = atoi(IgsYYvsp[-5].Name);
                    FullTime->tm_min   = atoi(IgsYYvsp[-3].Name);
                    FullTime->tm_sec   = atoi(IgsYYvsp[-1].Name);
                    FullTime->tm_isdst = LocalTime.tm_isdst;
                    mktime(FullTime);
                    IgsYYval.Dummy = FullTime;
                    myfree(IgsYYvsp[-8].Name);
                    myfree(IgsYYvsp[-7].Name);
                    myfree(IgsYYvsp[-6].Name);
                    myfree(IgsYYvsp[-5].Name);
                    myfree(IgsYYvsp[-3].Name);
                    myfree(IgsYYvsp[-1].Name);
                    myfree(IgsYYvsp[0].Name);
                ;
    break;}
case 340:
#line 2091 "./gointer.y"
{
                    char *ptr;
                    int   Length;

		    if (appdata.WantVerbose) {
			ptr = asctime(IgsYYvsp[-1].Dummy);
			Length = strlen(ptr);
			Outputf("Gmt    time: %.*s\n", Length-1, ptr);
		    }
                    myfree(IgsYYvsp[-1].Dummy);
                ;
    break;}
case 341:
#line 2103 "./gointer.y"
{
                    char *ptr;
                    int   Length;

                    ServerTime = *(struct tm *) IgsYYvsp[-1].Dummy;
                    SetServerTime = 1;
		    if (appdata.WantVerbose) {
			ptr = asctime(IgsYYvsp[-1].Dummy);
			Length = strlen(ptr);
			Outputf("Server time: %.*s\n", Length-1, ptr);
		    }
                    myfree(IgsYYvsp[-1].Dummy);
                ;
    break;}
case 342:
#line 2117 "./gointer.y"
{
                    long Uptime;

		    if (appdata.WantVerbose) {
			Uptime = ((24*atoi(IgsYYvsp[-6].Name) + atoi(IgsYYvsp[-4].Name))*60 + atoi(IgsYYvsp[-2].Name)) * 60;
			Outputf("Uptime: %s %s %s %s %s %s (%ld seconds)\n",
				IgsYYvsp[-6].Name, IgsYYvsp[-5].Name, IgsYYvsp[-4].Name, IgsYYvsp[-3].Name, IgsYYvsp[-2].Name, IgsYYvsp[-1].Name, Uptime);
		    }
                    myfree(IgsYYvsp[-6].Name);
                    myfree(IgsYYvsp[-5].Name);
                    myfree(IgsYYvsp[-4].Name);
                    myfree(IgsYYvsp[-3].Name);
                    myfree(IgsYYvsp[-2].Name);
                    myfree(IgsYYvsp[-1].Name);
                ;
    break;}
case 343:
#line 2133 "./gointer.y"
{
		    if (appdata.WantVerbose) {
			Outputf("%s\n", IgsYYvsp[0].Name);
		    }
                    myfree(IgsYYvsp[0].Name);
                ;
    break;}
case 346:
#line 2146 "./gointer.y"
{
                    SgfList(IgsYYvsp[-1].Namelist);
                    FreeNameList(IgsYYvsp[-1].Namelist);
                ;
    break;}
case 347:
#line 2151 "./gointer.y"
{
                    Output("sgf needs arguments\n");
                ;
    break;}
case 348:
#line 2157 "./gointer.y"
{
                    ReviewList(IgsYYvsp[0].Namelist);
                    FreeNameList(IgsYYvsp[0].Namelist);
                ;
    break;}
case 349:
#line 2164 "./gointer.y"
{
/* For the moment we just ignore the variations list
                    NameList *Here;

                    Output("Variations:");
                    for (Here = $2->Next; Here != $2; Here = Here->Next)
                        Outputf(" %s", Here->Name);
                    Output("\n");
*/
                    FreeNameList(IgsYYvsp[0].Namelist);
                ;
    break;}
case 350:
#line 2178 "./gointer.y"
{
                    ReviewStart(IgsYYvsp[0].Name);
                    myfree(IgsYYvsp[0].Name);
                ;
    break;}
case 351:
#line 2185 "./gointer.y"
{
                    NameList *names;

                    names = mynew(NameList);
                    names->Name     = IgsYYvsp[0].Name;
                    names->Next     = IgsYYvsp[-1].Namelist;
                    names->Previous = IgsYYvsp[-1].Namelist->Previous;
                    names->Next->Previous = names->Previous->Next = names;
                    IgsYYval.Namelist = IgsYYvsp[-1].Namelist;
                ;
    break;}
case 352:
#line 2196 "./gointer.y"
{
                    NameList *header;

                    header = mynew(NameList);
                    header->Name = NULL;
                    header->Next = header->Previous = header;
                    IgsYYval.Namelist = header;
                ;
    break;}
case 353:
#line 2206 "./gointer.y"
{ ReviewOpenVariation(); ;
    break;}
case 354:
#line 2207 "./gointer.y"
{ ReviewCloseVariation(); ;
    break;}
case 355:
#line 2209 "./gointer.y"
{
                    ReviewNewNode();
                ;
    break;}
case 356:
#line 2213 "./gointer.y"
{
                    NameList *Here;

                    Outputf("Unknown: %s:", IgsYYvsp[-1].Name);
                    for (Here = IgsYYvsp[0].Namelist->Next; Here != IgsYYvsp[0].Namelist; Here = Here->Next)
                        Outputf(" %s", Here->Name);
                    Output("\n");
                    myfree(IgsYYvsp[-1].Name);
                    FreeNameList(IgsYYvsp[0].Namelist);
                ;
    break;}
case 357:
#line 2224 "./gointer.y"
{
                    NameList Entry;

                    Entry.Name = IgsYYvsp[0].Name;
                    Entry.Next = Entry.Previous = &Entry;
                    ReviewLocalProperty(retNODENAME, &Entry);
                    myfree(IgsYYvsp[0].Name);
                ;
    break;}
case 358:
#line 2233 "./gointer.y"
{
                    NameList Entry;

                    Entry.Name = IgsYYvsp[0].Name;
                    Entry.Next = Entry.Previous = &Entry;
                    ReviewLocalProperty(retCOMMENT, &Entry);
                    myfree(IgsYYvsp[0].Name);
                ;
    break;}
case 359:
#line 2242 "./gointer.y"
{
                    ReviewGlobalProperty(retKOMI, IgsYYvsp[0].Name);
                    myfree(IgsYYvsp[0].Name);
                ;
    break;}
case 360:
#line 2247 "./gointer.y"
{
                    ReviewGlobalProperty(retHANDICAP, IgsYYvsp[0].Name);
                    myfree(IgsYYvsp[0].Name);
                ;
    break;}
case 361:
#line 2252 "./gointer.y"
{
                    ReviewGlobalProperty(retENTEREDBY, IgsYYvsp[0].Name);
                    myfree(IgsYYvsp[0].Name);
                ;
    break;}
case 362:
#line 2257 "./gointer.y"
{
                    ReviewGlobalProperty(retCOPYRIGHT, IgsYYvsp[0].Name);
                    myfree(IgsYYvsp[0].Name);
                ;
    break;}
case 363:
#line 2262 "./gointer.y"
{
                    ReviewGlobalProperty(retPLACE, IgsYYvsp[0].Name);
                    myfree(IgsYYvsp[0].Name);
                ;
    break;}
case 364:
#line 2267 "./gointer.y"
{
                    ReviewGlobalProperty(retDATE, IgsYYvsp[0].Name);
                    myfree(IgsYYvsp[0].Name);
                ;
    break;}
case 365:
#line 2272 "./gointer.y"
{
                    ReviewGlobalProperty(retRESULT, IgsYYvsp[0].Name);
                    myfree(IgsYYvsp[0].Name);
                ;
    break;}
case 366:
#line 2277 "./gointer.y"
{
                    ReviewGlobalProperty(retTOURNAMENT, IgsYYvsp[0].Name);
                    myfree(IgsYYvsp[0].Name);
                ;
    break;}
case 367:
#line 2282 "./gointer.y"
{
                    ReviewGlobalProperty(retNAME, IgsYYvsp[0].Name);
                    myfree(IgsYYvsp[0].Name);
                ;
    break;}
case 368:
#line 2287 "./gointer.y"
{
                    ReviewGlobalProperty(retWHITESTRENGTH, IgsYYvsp[0].Name);
                    myfree(IgsYYvsp[0].Name);
                ;
    break;}
case 369:
#line 2292 "./gointer.y"
{
                    ReviewGlobalProperty(retBLACKSTRENGTH, IgsYYvsp[0].Name);
                    myfree(IgsYYvsp[0].Name);
                ;
    break;}
case 370:
#line 2297 "./gointer.y"
{
                    ReviewGlobalProperty(retWHITENAME, IgsYYvsp[0].Name);
                    myfree(IgsYYvsp[0].Name);
                ;
    break;}
case 371:
#line 2302 "./gointer.y"
{
                    ReviewGlobalProperty(retBLACKNAME, IgsYYvsp[0].Name);
                    myfree(IgsYYvsp[0].Name);
                ;
    break;}
case 372:
#line 2307 "./gointer.y"
{
                    ReviewGlobalProperty(retSIZE, IgsYYvsp[0].Name);
                    myfree(IgsYYvsp[0].Name);
                ;
    break;}
case 373:
#line 2312 "./gointer.y"
{
                    ReviewGlobalProperty(retGAME, IgsYYvsp[0].Name);
                    myfree(IgsYYvsp[0].Name);
                ;
    break;}
case 374:
#line 2317 "./gointer.y"
{
                    NameList Entry;

                    Entry.Name = IgsYYvsp[0].Name;
                    Entry.Next = Entry.Previous = &Entry;
                    ReviewLocalProperty(retWHITE, &Entry);
                    myfree(IgsYYvsp[0].Name);
                ;
    break;}
case 375:
#line 2326 "./gointer.y"
{
                    NameList Entry;

                    Entry.Name = IgsYYvsp[0].Name;
                    Entry.Next = Entry.Previous = &Entry;
                    ReviewLocalProperty(retBLACK, &Entry);
                    myfree(IgsYYvsp[0].Name);
                ;
    break;}
case 376:
#line 2335 "./gointer.y"
{
                    ReviewLocalProperty(retLETTERS, IgsYYvsp[0].Namelist);
                    FreeNameList(IgsYYvsp[0].Namelist);
                ;
    break;}
case 377:
#line 2340 "./gointer.y"
{
                    NameList Entry;

                    Entry.Name = IgsYYvsp[0].Name;
                    Entry.Next = Entry.Previous = &Entry;
                    ReviewLocalProperty(retWHITETIME, &Entry);
                    myfree(IgsYYvsp[0].Name);
                ;
    break;}
case 378:
#line 2349 "./gointer.y"
{
                    NameList Entry;

                    Entry.Name = IgsYYvsp[0].Name;
                    Entry.Next = Entry.Previous = &Entry;
                    ReviewLocalProperty(retBLACKTIME, &Entry);
                    myfree(IgsYYvsp[0].Name);
                ;
    break;}
case 379:
#line 2358 "./gointer.y"
{
                    ReviewLocalProperty(retBLACKSET, IgsYYvsp[0].Namelist);
                    FreeNameList(IgsYYvsp[0].Namelist);
                ;
    break;}
case 380:
#line 2363 "./gointer.y"
{
                    ReviewLocalProperty(retWHITESET, IgsYYvsp[0].Namelist);
                    FreeNameList(IgsYYvsp[0].Namelist);
                ;
    break;}
case 381:
#line 2368 "./gointer.y"
{
                    ReviewLocalProperty(retEMPTYSET, IgsYYvsp[0].Namelist);
                    FreeNameList(IgsYYvsp[0].Namelist);
                ;
    break;}
case 384:
#line 2379 "./gointer.y"
{
                    ReviewEntryBegin(IgsYYvsp[0].Value);
                ;
    break;}
case 388:
#line 2390 "./gointer.y"
{
                    ReviewEnd(0);
                ;
    break;}
case 389:
#line 2394 "./gointer.y"
{
                    ReviewEnd(1);
                ;
    break;}
case 390:
#line 2400 "./gointer.y"
{
                    ReviewStop();
                ;
    break;}
case 391:
#line 2406 "./gointer.y"
{
                    ReviewNotFound();
                ;
    break;}
case 392:
#line 2412 "./gointer.y"
{
                    Output("You are already logged on. "
                           "Throwing other copy out\n");
                ;
    break;}
case 393:
#line 2419 "./gointer.y"
{ /* my rating, their rating, handicap,
                   * proba lose as white, proba lose as black
                   */
                    MyLoseProbas(IgsYYvsp[-5].Person, IgsYYvsp[-4].Value, IgsYYvsp[-3].Value, IgsYYvsp[-2].Name, IgsYYvsp[-1].Name, IgsYYvsp[0].Name);
                    myfree(IgsYYvsp[-2].Name);
                    myfree(IgsYYvsp[-1].Name);
                    myfree(IgsYYvsp[0].Name);
                ;
    break;}
case 394:
#line 2428 "./gointer.y"
{ /* other player does not have a rating */
                ;
    break;}
case 395:
#line 2433 "./gointer.y"
{ /* my rating, their rating, handicap,
                   * proba lose as white, proba lose as black
                   */
                    MyLoseProbas(NULL, IgsYYvsp[-4].Value, IgsYYvsp[-3].Value, IgsYYvsp[-2].Name, IgsYYvsp[-1].Name, IgsYYvsp[0].Name);
                    myfree(IgsYYvsp[-2].Name);
                    myfree(IgsYYvsp[-1].Name);
                    myfree(IgsYYvsp[0].Name);
                ;
    break;}
case 396:
#line 2442 "./gointer.y"
{ /* I do not have a rating */
                ;
    break;}
case 397:
#line 2447 "./gointer.y"
{
                    if (ArgsCommand(NULL, ";")) ChannelDisallowed();
                    else Output("Sorry.\n");
                ;
    break;}
case 398:
#line 2454 "./gointer.y"
{
                    Outputf("Unknown command %s\n", IgsYYvsp[0].Name);
                    myfree(IgsYYvsp[0].Name);
                ;
    break;}
case 399:
#line 2461 "./gointer.y"
{
                ;
    break;}
case 400:
#line 2466 "./gointer.y"
{
                    NameList *names;

                    names = mynew(NameList);
                    names->Name     = IgsYYvsp[0].Name;
                    names->Next     = IgsYYvsp[-1].Namelist;
                    names->Previous = IgsYYvsp[-1].Namelist->Previous;
                    names->Next->Previous = names->Previous->Next = names;
                    IgsYYval.Namelist = IgsYYvsp[-1].Namelist;
                ;
    break;}
case 401:
#line 2476 "./gointer.y"
{
                    NameList *header;

                    header = mynew(NameList);
                    header->Name = NULL;
                    header->Next = header->Previous = header;
                    IgsYYval.Namelist = header;
                ;
    break;}
case 402:
#line 2487 "./gointer.y"
{
                    IgsYYvsp[-2].Namelist->Previous->Next = IgsYYvsp[-1].Namelist->Next;
                    IgsYYvsp[-1].Namelist->Next->Previous = IgsYYvsp[-2].Namelist->Previous;
                    IgsYYvsp[-1].Namelist->Previous->Next = IgsYYvsp[-2].Namelist;
                    IgsYYvsp[-2].Namelist->Previous       = IgsYYvsp[-1].Namelist->Previous;
                    myfree(IgsYYvsp[-1].Namelist);
                    IgsYYval.Namelist = IgsYYvsp[-2].Namelist;
                ;
    break;}
case 403:
#line 2495 "./gointer.y"
{
                    NameList *header;

                    header = mynew(NameList);
                    header->Name = NULL;
                    header->Next = header->Previous = header;
                    IgsYYval.Namelist = header;
                ;
    break;}
case 406:
#line 2509 "./gointer.y"
{;
    break;}
case 407:
#line 2511 "./gointer.y"
{
                    Outputf("%s\n", IgsYYvsp[0].Name);
                    myfree(IgsYYvsp[0].Name);
                ;
    break;}
case 408:
#line 2517 "./gointer.y"
{ IgsYYval.Name = IgsYYvsp[0].Name;   ;
    break;}
case 409:
#line 2518 "./gointer.y"
{ IgsYYval.Name = NULL; ;
    break;}
case 410:
#line 2522 "./gointer.y"
{
                    char *ptr;

                    ptr = IgsYYvsp[0].Name;
                    if (ptr[0] == '\r') ptr++;
                    if (isdigit(ptr[0])) {
                        ptr++;
                        while (isdigit(ptr[0])) ptr++;
                        if (ptr[0] == ' ') ptr++;
                    }
                    Outputf("%s\n", ptr);
                    myfree(IgsYYvsp[0].Name);
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
#line 498 "/usr/share/bison.simple"

  IgsYYvsp -= IgsYYlen;
  IgsYYssp -= IgsYYlen;
#ifdef YYLSP_NEEDED
  IgsYYlsp -= IgsYYlen;
#endif

#if YYDEBUG != 0
  if (IgsYYdebug)
    {
      short *ssp1 = IgsYYss - 1;
      fprintf (stderr, "state stack now");
      while (ssp1 != IgsYYssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

  *++IgsYYvsp = IgsYYval;

#ifdef YYLSP_NEEDED
  IgsYYlsp++;
  if (IgsYYlen == 0)
    {
      IgsYYlsp->first_line = IgsYYlloc.first_line;
      IgsYYlsp->first_column = IgsYYlloc.first_column;
      IgsYYlsp->last_line = (IgsYYlsp-1)->last_line;
      IgsYYlsp->last_column = (IgsYYlsp-1)->last_column;
      IgsYYlsp->text = 0;
    }
  else
    {
      IgsYYlsp->last_line = (IgsYYlsp+IgsYYlen-1)->last_line;
      IgsYYlsp->last_column = (IgsYYlsp+IgsYYlen-1)->last_column;
    }
#endif

  /* Now "shift" the result of the reduction.
     Determine what state that goes to,
     based on the state we popped back to
     and the rule number reduced by.  */

  IgsYYn = IgsYYr1[IgsYYn];

  IgsYYstate = IgsYYpgoto[IgsYYn - YYNTBASE] + *IgsYYssp;
  if (IgsYYstate >= 0 && IgsYYstate <= YYLAST && IgsYYcheck[IgsYYstate] == *IgsYYssp)
    IgsYYstate = IgsYYtable[IgsYYstate];
  else
    IgsYYstate = IgsYYdefgoto[IgsYYn - YYNTBASE];

  goto IgsYYnewstate;

IgsYYerrlab:   /* here on detecting error */

  if (! IgsYYerrstatus)
    /* If not already recovering from an error, report this error.  */
    {
      ++IgsYYnerrs;

#ifdef YYERROR_VERBOSE
      IgsYYn = IgsYYpact[IgsYYstate];

      if (IgsYYn > YYFLAG && IgsYYn < YYLAST)
	{
	  int size = 0;
	  char *msg;
	  int x, count;

	  count = 0;
	  /* Start X at -IgsYYn if nec to avoid negative indexes in IgsYYcheck.  */
	  for (x = (IgsYYn < 0 ? -IgsYYn : 0);
	       x < (sizeof(IgsYYtname) / sizeof(char *)); x++)
	    if (IgsYYcheck[x + IgsYYn] == x)
	      size += strlen(IgsYYtname[x]) + 15, count++;
	  msg = (char *) malloc(size + 15);
	  if (msg != 0)
	    {
	      strcpy(msg, "parse error");

	      if (count < 5)
		{
		  count = 0;
		  for (x = (IgsYYn < 0 ? -IgsYYn : 0);
		       x < (sizeof(IgsYYtname) / sizeof(char *)); x++)
		    if (IgsYYcheck[x + IgsYYn] == x)
		      {
			strcat(msg, count == 0 ? ", expecting `" : " or `");
			strcat(msg, IgsYYtname[x]);
			strcat(msg, "'");
			count++;
		      }
		}
	      IgsYYerror(msg);
	      free(msg);
	    }
	  else
	    IgsYYerror ("parse error; also virtual memory exceeded");
	}
      else
#endif /* YYERROR_VERBOSE */
	IgsYYerror("parse error");
    }

  goto IgsYYerrlab1;
IgsYYerrlab1:   /* here on error raised explicitly by an action */

  if (IgsYYerrstatus == 3)
    {
      /* if just tried and failed to reuse lookahead token after an error, discard it.  */

      /* return failure if at end of input */
      if (IgsYYchar == YYEOF)
	YYABORT;

#if YYDEBUG != 0
      if (IgsYYdebug)
	fprintf(stderr, "Discarding token %d (%s).\n", IgsYYchar, IgsYYtname[IgsYYchar1]);
#endif

      IgsYYchar = YYEMPTY;
    }

  /* Else will try to reuse lookahead token
     after shifting the error token.  */

  IgsYYerrstatus = 3;		/* Each real token shifted decrements this */

  goto IgsYYerrhandle;

IgsYYerrdefault:  /* current state does not do anything special for the error token. */

#if 0
  /* This is wrong; only states that explicitly want error tokens
     should shift them.  */
  IgsYYn = IgsYYdefact[IgsYYstate];  /* If its default is to accept any token, ok.  Otherwise pop it.*/
  if (IgsYYn) goto IgsYYdefault;
#endif

IgsYYerrpop:   /* pop the current state because it cannot handle the error token */

  if (IgsYYssp == IgsYYss) YYABORT;
  IgsYYvsp--;
  IgsYYstate = *--IgsYYssp;
#ifdef YYLSP_NEEDED
  IgsYYlsp--;
#endif

#if YYDEBUG != 0
  if (IgsYYdebug)
    {
      short *ssp1 = IgsYYss - 1;
      fprintf (stderr, "Error: state stack now");
      while (ssp1 != IgsYYssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

IgsYYerrhandle:

  IgsYYn = IgsYYpact[IgsYYstate];
  if (IgsYYn == YYFLAG)
    goto IgsYYerrdefault;

  IgsYYn += YYTERROR;
  if (IgsYYn < 0 || IgsYYn > YYLAST || IgsYYcheck[IgsYYn] != YYTERROR)
    goto IgsYYerrdefault;

  IgsYYn = IgsYYtable[IgsYYn];
  if (IgsYYn < 0)
    {
      if (IgsYYn == YYFLAG)
	goto IgsYYerrpop;
      IgsYYn = -IgsYYn;
      goto IgsYYreduce;
    }
  else if (IgsYYn == 0)
    goto IgsYYerrpop;

  if (IgsYYn == YYFINAL)
    YYACCEPT;

#if YYDEBUG != 0
  if (IgsYYdebug)
    fprintf(stderr, "Shifting error token, ");
#endif

  *++IgsYYvsp = IgsYYlval;
#ifdef YYLSP_NEEDED
  *++IgsYYlsp = IgsYYlloc;
#endif

  IgsYYstate = IgsYYn;
  goto IgsYYnewstate;
}
#line 2550 "./gointer.y"

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
