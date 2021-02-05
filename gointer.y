%{
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
#define YYOVERFLOW MyOverflow(x1)
#define	xmalloc	mymalloc
/* Kludge to get rid of yy_bcopy warnings --Ton */
#ifndef __GNUC__
# define __GNUC__ 2
#endif /* __GNUC__ */

static void yyerror(const char *s);
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
extern       int   yylex(void);
extern const char *_GoText(void);
extern       void  _IgsDefaultParse(void);
extern const char *_FormatError(void);
extern const char *Parsing(Connection conn);

%}
%union {
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
}

%token   <Name>   NAME SERVERMESSAGE STATSENTRY ILLEGALMOVE ILLEGALUNDO
%token   <Name>   REQUESTINGMATCH REMOVEGAMEFILE MAILED REMOVEGROUP GIVEBYOYOMI
%token   <Name>   RESULTLINE INVALID AUTOMATCHDISPUTE NEWCHANNEL MUSTPASS
%token   <Name>   OPPMUSTPASS GUEST TELLDONE REVIEWSTART REVLITERAL REVUNKNOWN
%token   <Name>   WELCOME SERVERFULL XSHOUT2 MYBET
%token   <Person> YELL TELL RESIGN KOMIREQUEST DISPUTEMATCHTYPE XSHOUT DECLINE
%token   <Person> JOIN LEAVE NEWTITLE BROADCAST ITBROADCAST ENTERBYOYOMI
%token   <Person> NOTIME PERSON BEEPING PLAYERON PROBA STORED IDLE
%token   <Value>  PROMPT GAMES REMOVE MOVE GAME OVEROBSERVE MESSAGES
%token   <Value>  NEWMATCH STATUSLINE CHANNEL CHANGECHANNEL FREE TEXTFILE
%token   <Value>  FIRSTREMOVE REVIEWTYPE GAMECOLOR GAMESECONDS BYOYOMI
%token   <Value>  MATCHTYPE NATURAL BETRESULT RATING STOREDNUM UNDO 
%token   <Dummy>  END FAIL OLDPROMPT SEMIPROMPT INFOMESSAGE LUSER OLDPASSWORD
%token   <Dummy>  PASSWORD INVALIDPASSWORD IGSENTRY TITLESET
%token   <Dummy>  TOGGLE PLAYERS UNKNOWNANSWER MATCHCLOSED MATCHOPEN
%token   <Dummy>  OBSERVE WATCHING EXTSTATSENTRY ADD KIBITZ KOMISET
%token   <Dummy>  TRANSLATION GAMETIME LOSTCONNECTION MYADJOURN RESTORE RESTART
%token   <Dummy>  NOTURN GAMESAVED UNDID EMPTY DONE RESTORESCORING
%token   <Dummy>  STATUSHEADER REMOVELIBERTY OBSERVEWHILEPLAY NOTELLTARGET
%token   <Dummy>  GMTTIME LOCALTIME SERVERUP UPTIMEENTRY THROWCOPY SORRY
%token   <Dummy>  WRONGCHANNEL AUTOMATCHREQUEST DISPUTE OPPONENTDISPUTE
%token   <Dummy>  LATEFREE NOPLAY CHANNELHEADER OBSERVERS GAMENOTFOUND
%token   <Dummy>  MATCHREQUEST GOEMATCHREQUEST TOURNAMENTMATCHREQUEST
%token   <Dummy>  TOURNAMENTGOEMATCHREQUEST USERESIGN GAMETITLE
%token   <Dummy>  ERASE PLEASEREDONE TELLTARGET TELLOFF NOREMOVETURN
%token   <Dummy>  ADJOURNSENTREQUEST ADJOURNREQUEST OPPONENTNOTON NOLOAD
%token   <Dummy>  DISAGREEREMOVE OPPDISAGREEREMOVE DECLINEADJOURN REVIEWLIST
%token   <Dummy>  REVIEWSTOP REVNODE REVCOMMENT REVEVENT REVRESULT REVPLACE
%token   <Dummy>  REVUSER REVDATE REVKOMI REVGAMENAME REVWHITERANK REVBLACKRANK
%token   <Dummy>  REVWHITENAME REVBLACKNAME REVSIZE REVGAME REVBLACK REVWHITE
%token   <Dummy>  REVADDBLACK REVADDWHITE REVADDEMPTY REVNODENAME REVIEWEND
%token   <Dummy>  REVBLACKTIME REVWHITETIME REVCOPYRIGHT REVHANDICAP REVLETTERS
%token   <Dummy>  REVIEWVARIATIONS NOREVIEW SGFLIST NOSGF NOMOREMOVES
%token   <Dummy>  BETWINNERS BETEVEN BETLOSERS USER CURRENTSCORE FINALSCORE
%token   <Dummy>  TEAMGAME OBSERVETEAM RESTARTTEAMGAME SETPROBA NOTREVIEWING
%token   <Dummy>  NOTREQUESTGAME

%type    <Name>     optname optgamesaved optgametitle optmybet
%type    <Value>    prompt gameslines playertime matchtypes
%type    <Value>    optbyo ruledmatchrequest optfirst
%type    <Nameval>  statsentry statsentries optextend move movelist
%type    <Numval>   statusline statuslines
%type    <Namelist> names namesset extendstatsentry statusheader
%type    <Namelist> observerentries channelentry reviewliterals userline
%type    <NameListlist> userlines
%type    <Channeldata> channelentries
%type    <Gamedesc> gamedesc
%type    <Disputedesc> disputelines disputeline
%type    <Game>     gamesline
%type    <Person>   player
%type    <Bet>      betentry betentries
%type    <Dummy>    date

%%
start       : session
            | error
            ;

session     : session loginmessages pass inputs INVALIDPASSWORD
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
                }
            |
                {
                    Passed = 0;
                }
            ;

pass        : PASSWORD
                {
                    if (MyPassword) ForceCommand(NULL, MyPassword);
                    else AskString(toplevel, EnterString,
                                   (XtPointer) &MyPassword, "Enter password",
                                   "password", &MyPassword, NULL, NULL);
                }
            | OLDPASSWORD
                {
                    if (MyPassword) ForceCommand(NULL, MyPassword);
                    else AskString(toplevel, EnterString,
                                   (XtPointer) &MyPassword, "Enter password",
                                   "password", &MyPassword, NULL, NULL);
                }
              loginmessages enterorfail
            | GUEST
                {
                    myfree(MyName);
                    MyName = $1;
                }
              loginmessages enterorfail
                {
                    Outputf("This is a guest account. Please see "
                            "'help register' to register.\n"
                            "Your account name is %s\n", $1);
                }
            ;

enterorfail : OLDPROMPT
                {
                    if (!Passed) {
                        Passed = 1;
                        PlayerPasses(MyName);
                    }
                    ForceCommand(NULL, "toggle client on");
                }
              promptnames END
            |
            ;

loginmessages: loginmessages loginmessage
            |
            ;

loginmessage: NAME
                {
                    Outputf("%s\n", $1);
                    myfree($1);
                }
            | WELCOME
                {
                    Outputf("          Welcome to IGS at %s ", $1);
                    SiteLogon(NULL, $1);
                    myfree($1);
                }
            | LUSER
                {
                    if (MyName) ForceCommand(NULL, MyName);
                    else AskString(toplevel, EnterString, (XtPointer) &MyName,
                                   "Enter user", "user", &MyName, NULL, NULL);
                }
            | SERVERFULL
                {
                    ServerMessage("%s\n", $1);
                    Outputf("%s\n", $1);
                    myfree($1);
                }
            ;

inputs      : inputs moreinput  { eEmpty = 0; }
            |                   { eEmpty = PreEmpty = 0; }
            ;

prompt      : PROMPT
                {
                    PreEmpty = 0;
                    SeenAdd = 0;
                    $$ = $1;
                }
            | SEMIPROMPT
                {
                    PreEmpty = eEmpty;
                    $$ = 0;
                }
            ;

moreinput   : input prompt
                {
                    if (!Passed && $2) {
                        Passed = 1;
                        PlayerPasses(MyName);
                    }
                    if ($2) {
                        ChangeCommand(NULL, 1);
                        ResyncCommand(NULL);
                    }
                }
            ;

input       : igsentry
            | servermessages      { ChangeCommand(NULL, -1); }
            | xshout              { ChangeCommand(NULL, -1); }
            | infomessage         { ChangeCommand(NULL, -1); }
            | beeping             { ChangeCommand(NULL, -1); }
            | idle
            | stored
            | tell                { ChangeCommand(NULL, -1); }
            | broadcast           { ChangeCommand(NULL, -1); }
            | kibitz              { ChangeCommand(NULL, -1); }
            | messages
            | yell                { ChangeCommand(NULL, -1); }
            | join                { ChangeCommand(NULL, -1); }
            | leave               { ChangeCommand(NULL, -1); }
            | newtitle            { ChangeCommand(NULL, -1); }
            | changechannel
            | wrongchannel
            | matchopen
            | matchclosed
            | automatchrequest
            | automatchdispute
            | matchrequest        { ChangeCommand(NULL, -1); }
            | requestingmatch
            | komirequest
            | komiset
            | freemessage         { ChangeCommand(NULL, -1); }
            | freeconfirm
            | latefree
            | noplay
            | noload
            | titleset
            | stats
            | bet
            | textfile
            | erase
            | toggle
            | channels
            | observers
            | gamenotfound
            | nomoremoves
	    | notrequestgame
            | games
            | remove
            | add
            | doneobserve
            | oppmustpass
            | mustpass            { ChangeCommand(NULL, -1); }
            | disagreeremove
            | opponentdisagreeremove { ChangeCommand(NULL, -1); }
            | observe
            | doneopponentobserve { ChangeCommand(NULL, -1); }
            | opponentobserve     { ChangeCommand(NULL, -1); }
            | betresult           { ChangeCommand(NULL, -1); }
            | restore
            | opponentrestart     { ChangeCommand(NULL, -1); }
            | restart
            | newmatch1           { ChangeCommand(NULL, -1); }
            | newmatch2
            | decline
            | opponentdispute     { ChangeCommand(NULL, -1); }
            | dispute
            | disputematchtype
            | undid
            | opponentundid       { ChangeCommand(NULL, -1); }
            | undo                { ChangeCommand(NULL, -1); }
            | watching
            | overobserve
            | observewhileplay
            | users
            | players
            | gametime
            | gamescore
            | translations
            | byoyomi
            | notime
            | lostconnection      { ChangeCommand(NULL, -1); }
            | gamesaved
            | adjourn
            | adjournsentrequest  { ChangeCommand(NULL, -1); }
            | adjournrequest      { ChangeCommand(NULL, -1); }
            | oppadjourn          { ChangeCommand(NULL, -1); }
            | declineadjourn      { ChangeCommand(NULL, -1); }
            | resign              { ChangeCommand(NULL, -1); }
            | playeron            { ChangeCommand(NULL, -1); }
            | removegamefile      { ChangeCommand(NULL, -1); }
            | mailed
            | notelltarget
            | telltarget
            | telldone
            | telloff
            | illegalmove
            | illegalundo
            | noturn
            | noremoveturn
            | useresign
            | removeliberty
            | removegroup
            | restorescoring
            | pleaseredone
            | resultline
            | status
            | uptime
                {
                    if (!Entered) {
                        Entering();
                    }
                }
            | sgflist
            | reviewlist
            | reviewvariations
            | reviewstart
            | reviews
            | reviewstop
            | noreview
            | throwcopy
	    | proba
	    | setproba
            | EMPTY               { ChangeCommand(NULL, -1); }
            | sorry
            | invalid
            | unknown
            | error
                {
                    /* 1 in case next token is SEMIPROMPT */
                    SetCommand(NULL, 1);
                }
            |                     { eEmpty = 1; }
            ;

textfile    : TEXTFILE names END
                {
                    NameList   *Names;
                    const char *User;

                    switch($1) {
                      case 25: /* Results */
                        if (UserCommandP(NULL)) goto user;
                        User = StripFirstArgCommand(NULL, "results");
                        if (!User) goto user;
			if (*User == '-') User++;
                        /* AddResults keeps $2 */
                        AddResults(User, $2);
                        break;
                      default:
                      user:
                        for (Names = $2->Next;Names != $2; Names = Names->Next)
                            Outputf("%s\n", Names->Name);
                        FreeNameList($2);
                    }
                }
            ;

erase       : ERASE
                {
                    Output("Please erase your messages (see help erase)\n");
                }
            ;

igsentry    : IGSENTRY NAME
                {
                    char *ptr;

                    Outputf("Logging into %s %s\n", $1, $2);
                    ptr = mystrdup($1);
                    myfree(ServerName);
                    ServerName = ptr;
                    switch(ptr[0]) {
                      case 'N': ServerType = NNGS; break;
                      default:  ServerType = IGS;  break;
                    }
                    myfree($1);
                    myfree($2);
                }
            ;

servermessages: servermessages servermessage
            | servermessage
            | EMPTY servermessage {}
            ;

servermessage: SERVERMESSAGE NAME
                {
                    ServerMessage("%s\n", $2);
                    myfree($2);
                }
            ;

xshout      : XSHOUT NAME
                {
                    ServerMessage("%s: %s\n", PlayerString($1), $2);
                    myfree($2);
                }
            | XSHOUT2 NAME
                {
		    /* dummy player name such as "*8^)*" */
                    ServerMessage("%s: %s\n", $1, $2);
                    myfree($1);
                    myfree($2);
                }
            ;

infomessage : INFOMESSAGE NAME '[' NAME ']' NAME NAME '}' END
                {
                    /* A Connect */
                    if (strcmp($6, "has") || strcmp($7, "connected."))
                        YYFAIL;
                    PlayerConnect($2, $4);
                    myfree($2);
                    myfree($4);
                    myfree($6);
                    myfree($7);
                }
            | INFOMESSAGE NAME NAME NAME '}' END
                {
                    /* A disconnect */
                    if (strcmp($3, "has") || strcmp($4, "disconnected"))
                        YYFAIL;
                    PlayerDisconnect($2);
                    myfree($2);
                    myfree($3);
                    myfree($4);
                }
            | INFOMESSAGE NAME NAME ':' player NAME player '}' END
                {
                    /* A new match, format with game number */
                    if (strcmp($6, "vs.") || strcmp($2, "Match")) YYFAIL;
                    NewMatch(atoi($3), $5, $7);
                    myfree($2);
                    myfree($3);
                    myfree($6);
                }
            | INFOMESSAGE NAME NAME ':' NAME NAME NAME ':' names '}' END
                {
                    if (strcmp($6, "vs") || strcmp($2, "Game")) YYFAIL;
                    GameInfo(atoi($3), $7, $5, $9);
                    myfree($2);
                    myfree($3);
                    myfree($5);
                    myfree($6);
                    myfree($7);
                    FreeNameList($9);
                }
            | INFOMESSAGE NAME NAME ':' NAME NAME NAME '@' NAME NAME '}' END
                {
                    /* Resume */
                    if (strcmp($2, "Game") || strcmp($6, "vs") ||
                        strcmp($9, "Move")) YYFAIL;
                    Resume(atoi($3), $7, $5, atoi($10));
                    myfree($2);
                    myfree($3);
                    myfree($5);
                    myfree($6);
                    myfree($7);
                    myfree($9);
                    myfree($10);
                }
            | INFOMESSAGE NAME NAME ':' NAME NAME NAME NAME NAME '}' END
                {
                    /* Adjourn */
                    if (strcmp($8, "has") || strcmp($9, "adjourned.") ||
                        strcmp($6, "vs")) YYFAIL;
                    Adjourn(atoi($3), $7, $5);
                    myfree($2);
                    myfree($3);
                    myfree($5);
                    myfree($6);
                    myfree($7);
                    myfree($8);
                    myfree($9);
                }
            ;

tell        : TELL NAME
                {
                    ReceivedTell($1, $2);
                    myfree($2);
                }
            | OBSERVE SEMIPROMPT TELL NAME
                {
                    ReceivedTell($3, $4);
                    myfree($4);
                }
            ;

playeron    : PLAYERON optobserve
                {
                    ReceivedTell($1, "is now on.");
                }
            ;


beeping     : OBSERVE BEEPING
                {
                    Beeping($2);
                }
            | BEEPING
                {
                    Beeping($1);
                }
            ;

idle        : IDLE NAME
                {
		    Idle($1, $2);
                    myfree($2);
                }
            ;

stored      : STORED STOREDNUM
                {
		    StoredNum($1, $2);
                }
            ;

broadcast   : BROADCAST NAME
                {
                    ShowBroadcast($1, ":", $2);
                    myfree($2);
                }
            | ITBROADCAST NAME
		{
                    ShowBroadcast($1, "", $2);
                    myfree($2);
                }
            ;

kibitz      : OBSERVE SEMIPROMPT KIBITZ player ':' NAME NAME NAME NAME
              '[' NAME ']' END NAME
                {
                    if (strcmp($6, "Game") || strcmp($8, "vs")) YYFAIL;
                    ReceivedKibitz($4, atoi($11), $9, $7, $14, strlen($14));
                    myfree($6);
                    myfree($7);
                    myfree($8);
                    myfree($9);
                    myfree($11);
                    myfree($14);
                }
            |  KIBITZ player ':' NAME NAME NAME NAME
                '[' NAME ']' END NAME
                  {
                    if (strcmp($4, "Game") || strcmp($6, "vs")) YYFAIL;
                    ReceivedKibitz($2, atoi($9), $7, $5, $12, strlen($12));
                    myfree($4);
                    myfree($5);
                    myfree($6);
                    myfree($7);
                    myfree($9);
                    myfree($12);
                  }
            ;

messages    : MESSAGES
                {
                    Outputf("You have %d line%s of messages\n",
                            $1, $1==1 ? "" : "s");
                }

yell        : CHANNEL YELL NAME
                {
                    ShowYell($1, $2, $3);
                    myfree($3);
                }
            ;

join        : CHANNEL JOIN
                {
                    ChannelJoin($1, $2);
                }
            ;

leave       : CHANNEL LEAVE
                {
                    ChannelLeave($1, $2);
                }
            ;

newtitle    : CHANNEL NEWTITLE NAME
                {
                    ChannelTitle($1, $2, $3);
                    myfree($3);
                }
            ;

changechannel: CHANGECHANNEL
                {
                    JoinChannel($1);
                }
            ;

wrongchannel: WRONGCHANNEL NAME
                {
                    WrongChannel($2);
                    myfree($2);
                }
            ;

matchopen   : MATCHOPEN
                {
                    Output("Setting you open for matches\n");
                }
            ;

matchclosed : MATCHCLOSED
                {
                    Output("You are not open for matches\n");
                }
            ;

automatchrequest: AUTOMATCHREQUEST names END
                {
                    AutoMatchRequest($2);
                    FreeNameList($2);
                }
            ;

automatchdispute: AUTOMATCHDISPUTE names
                {
                    AutoMatchDispute($1, $2);
                    myfree($1);
                    FreeNameList($2);
                }
            ;

ruledmatchrequest:
              MATCHREQUEST              { $$ = 'I'; }
            | GOEMATCHREQUEST           { $$ = 'G'; }
            | TOURNAMENTMATCHREQUEST    { $$ = 'i'; }
            | TOURNAMENTGOEMATCHREQUEST { $$ = 'g'; }
            ;

matchrequest: ruledmatchrequest names '>' NAME '<' names '>' names END
              optobserve
                {
                    if (strcmp($4, "or")) YYFAIL;
                    MatchRequest($1, $2);
                    FreeNameList($2);
                    myfree($4);
                    FreeNameList($6);
                    FreeNameList($8);
                }
            ;

requestingmatch: REQUESTINGMATCH
                {
                    /* Outputf("%s\n", $1); */
                    myfree($1);
                }
            ;

komirequest : KOMIREQUEST NAME
                {
                    char *Ptr;

                    Ptr = strchr($2, 0)-1;
                    if (*Ptr == '.') *Ptr = 0;
                    MyGameMessage("%s wants the komi to be %s",
	                          PlayerString($1), $2);
                    if (WhatCommand(NULL, "komi") < 0) ChangeCommand(NULL, -1);
                    myfree($2);
                }
            ;

komiset     : KOMISET NAME
                {
                    char *Ptr;

                    Ptr = strchr($2, 0)-1;
                    if (*Ptr == '.') *Ptr = 0;
                    MyGameMessage("The komi has been set to %s", $2);
                    CheckMyKomi($2);
                    if (WhatCommand(NULL, "komi") < 0) ChangeCommand(NULL, -1);
                    myfree($2);
                }
            ;

freemessage : FREE OBSERVE
                {
                    Outputf("Game will %scount towards ratings\n",
                            $1 ? "not" : "");
                }
            ;

freeconfirm : FREE
                {
                    Outputf("Game will %scount towards ratings\n",
                            $1 ? "not " : "");
                }
            ;

latefree    : LATEFREE NAME
                {
                    Outputf("You cannot change into a free game after %s\n",
                            $2);
                    myfree($2);
                }
            ;

noplay      : NOPLAY
                {
                    Output("You are not playing a game\n");
                }
            ;

noload      : OPPONENTNOTON NOLOAD
                {
                    Output("Your opponent is not on currently. "
                           "Game failed to load\n");
                }

titleset    : TITLESET
                {
                    const char *title;

                    title = ArgsCommand(NULL, "title");
                    if (title) SetMyGameTitle(title);
                    else Warning("Title set, but I "
                                 "can't remember to what....\n");
                }
            ;

statsentry  : STATSENTRY NAME
                {
                    NameVal *nameval;

                    $$ = nameval = mynew(NameVal);
                    nameval->Next  = nameval->Previous = nameval;
                    nameval->Name  = $1;
                    nameval->Value = $2;
                }
            ;

statsentries: statsentries statsentry
                {
                    $$ = $1;
                    $2->Previous = $1->Previous;
                    $2->Next     = $1;
                    $2->Previous->Next = $2->Next->Previous = $2;
                }
            | statsentry { $$ = $1; }
            ;

extendstatsentry: EXTSTATSENTRY names END { $$ = $2; }
            ;

optextend   : extendstatsentry extendstatsentry
                {
                    NameVal *nameval;
                    NameList *Pos1, *Pos2;

                    nameval = mynew(NameVal);
                    nameval->Previous = nameval->Next = nameval;
                    nameval->Name = nameval->Value = NULL;
                    $$ = nameval;

                    for (Pos1=$1->Next, Pos2=$2->Next;
                         Pos1 != $1 && Pos2 != $2;
                         Pos1 = Pos1->Next, Pos2 = Pos2->Next) {
                        nameval = mynew(NameVal);
                        nameval->Name  = Pos1->Name; Pos1->Name = NULL;
                        nameval->Value = Pos2->Name; Pos2->Name = NULL;
                        nameval->Next = $$;
                        nameval->Previous = $$->Previous;
                        nameval->Previous->Next =
                            nameval->Next->Previous = nameval;
                    }
                    if (Pos1 != $1 || Pos2 != $2)
                        Warning("Name value lists have different length\n");
                    FreeNameList(Pos1);
                    FreeNameList(Pos2);
                }
            |
                {
                    NameVal *nameval;

                    nameval = mynew(NameVal);
                    nameval->Previous = nameval->Next = nameval;
                    nameval->Name = nameval->Value = NULL;
                    $$ = nameval;
                }
            ;

stats       : statsentries optextend
                {
                    NameVal *ext;

                    $2->Next->Previous = $1->Previous;
                    ext = $1->Previous->Next = $2->Next;
                    $2->Next = $1;
                    $1->Previous = $2;
                    ShowStats($2, ext);
                    FreeNameValList($2);
                }
            ;

betentry    : PERSON NATURAL ':' NATURAL
                {
                    $$ = mynew(BetDesc);
                    $$->Who  = $1;
                    $$->Wins = $2;
                    $$->Bets = $4;
                }
            ;

betentries  : betentries betentry
                {
                    $2->Next = $1;
                    $$ = $2;
                }
            |
                {
                    $$ = NULL;
                }
            ;

optmybet    : MYBET { $$ = $1;   }
            |       { $$ = NULL; }
            ;

bet         : BETWINNERS betentries BETEVEN betentries BETLOSERS betentries
              optmybet
                {
                    BetDesc *Here, *Next;

                    BetResults($2, $4, $6, $7);
                    for (Here = $2; Here; Here = Next) {
                        Next = Here->Next;
                        myfree(Here);
                    }
                    for (Here = $4; Here; Here = Next) {
                        Next = Here->Next;
                        myfree(Here);
                    }
                    for (Here = $6; Here; Here = Next) {
                        Next = Here->Next;
                        myfree(Here);
                    }
                }
            ;

toggle      : TOGGLE NAME NAME NAME NAME optname END
                {
                    /* -Ton remove the optname */
                    /* eg: Set | verbose to be True. */
                    SetStat($2, strcmp($5+1, "alse."));
                    myfree($2);
                    myfree($3);
                    myfree($4);
                    myfree($5);
                }
            ;

channelentry: NEWCHANNEL names END namesset
                {
                    $2->Name = (char *) $4;
                    $4->Name = $1;
                    $$ = $2;
                }
            ;

channelentries: channelentries channelentry
                {
                    NameList *Names;

                    $$ = $1;
                    Names = (NameList *) $2->Name;
                    $2->Name = NULL;
                    AddChannelData($$, Names->Name, $2->Next->Name,
                                   $2->Next->Next->Name,
                                   $2->Next->Next->Next->Name, Names);
                    Names->Name = NULL;
                    FreeNameList($2);
                }
            | channelentry
                {
                    NameList *Names;

                    $$ = OpenChannelData();
                    Names = (NameList *) $1->Name;
                    $1->Name = NULL;
                    AddChannelData($$, Names->Name, $1->Next->Name,
                                   $1->Next->Next->Name,
                                   $1->Next->Next->Next->Name, Names);
                    Names->Name = NULL;
                    FreeNameList($1);
                }
            ;

channels    : channelentries
                {
                    ChannelList($1);
                    CloseChannelData($1);
                }
            ;

observerentries: namesset { $$ = $1; }

observers   : OBSERVERS NAME '(' NAME NAME NAME ')' ':' END
              observerentries
                {
                    if (strcmp($5, "vs.")) YYFAIL;
                    ShowObservers(atoi($2), $6, $4, $10);

                    myfree($2);
                    myfree($4);
                    myfree($5);
                    myfree($6);
                    FreeNameList($10);
                }
            ;

gamenotfound: GAMENOTFOUND
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
            ;

nomoremoves : NOMOREMOVES
                {
		    const char *msg = "There are no more moves";
		    StopMyGameForward(msg);
                    MyGameMessage(msg);
                }
            ;


notrequestgame : NOTREQUESTGAME
                {
		    const char *msg = "This teach game is not a request game";
		    StopMyGameForward(msg);
                    MyGameMessage(msg);
                }
            ;


gamesline   : GAMES player NAME player
              '(' NAME NAME NAME NAME NAME NAME names ')' '(' NAME ')' END
                {
                    int    Mode, Rules;
                    size_t size;
                    char  *ptr;

                    ptr = $11;
                    if (ptr[1]) Mode = *ptr++;
                    else Mode = ' ';
                    Rules = *ptr++;
                    if (*ptr) YYFAIL;

                    size = atoi($7);
                    $$ = FindGame($1, $4, $2,
                                  atoi($6), size, size, atoi($8), $9,
                                  atoi($10), Mode, Rules, atoi($15));
                    myfree($3);
                    myfree($6);
                    myfree($7);
                    myfree($8);
                    myfree($9);
                    myfree($10);
                    myfree($11);
                    FreeNameList($12);
                    myfree($15);
                }
            ;

gameslines  : gameslines gamesline { $$ = $1+1; gamesSeen++; }
            |                      { $$ = 0; gamesSeen = 0; }
            ;

games       : GAMES
                {
                    AssertGamesDeleted();
                }
              gameslines
                {
                    TestGamesDeleted(gamesSeen);
                }
            ;

remove      : REMOVE { UnObserve($1); }
            ;

move        : MOVE NAME
                {
                    char     Num[20], *ptr;
                    NameVal *nameval;
                    $$ = nameval = mynew(NameVal);
                    sprintf(Num, "%d", $1);
                    nameval->Next  = nameval->Previous = nameval;
                    nameval->Name  = mystrdup(Num);
                    ptr = $2;
                    /* Get rid of extra `removed stones' entries.
                       Maybe I ought to compare them with what I work out.. */
                    while (*ptr && !isspace(*ptr)) ptr++;
                    if (ptr-$2 <= 3) *ptr = 0;
                    nameval->Value = $2;
                }
            ;

movelist    : movelist move
                {
                    $$ = $1;
                    $2->Previous = $1->Previous;
                    $2->Next     = $1;
                    $2->Previous->Next = $2->Next->Previous = $2;
                }
            |
                {
                    NameVal *nameval;

                    $$ = nameval = mynew(NameVal);
                    nameval->Next  = nameval->Previous = nameval;
                    nameval->Name  = NULL;
                    nameval->Value = NULL;
                }
            ;

optgamesaved: gamesaved {}
            | {}
            ;

gamedesc    : GAME NAME '(' NAME NAME NAME ')' NAME
                   NAME '(' NAME NAME NAME ')' END
                {
                    if (strcmp($8, "vs")) YYFAIL;

                    $$ = mynew(GameDesc);
                    $$->Id            = $1;
                    $$->BlackName     = $9;
                    $$->BlackName2    = 0;
                    $$->BlackCaptures = atoi($11);
                    $$->BlackTime     = atoi($12);
                    $$->BlackByo      = atoi($13);
                    $$->WhiteName     = $2;
                    $$->WhiteName2    = 0;
                    $$->WhiteCaptures = atoi($4);
                    $$->WhiteTime     = atoi($5);
                    $$->WhiteByo      = atoi($6);
                    myfree($4);
                    myfree($5);
                    myfree($6);
                    myfree($8);
                    myfree($11);
                    myfree($12);
                    myfree($13);
                }

            | GAME NAME '(' NAME NAME NAME ')' NAME
                   NAME '(' NAME NAME NAME ')' END
                   TEAMGAME NAME NAME NAME NAME END
                {
		    Game *game;
                    if (strcmp($8, "vs")) YYFAIL;

                    $$ = mynew(GameDesc);
                    $$->Id            = $1;
                    $$->BlackName     = $9;
                    $$->BlackName2    = $19;
                    $$->BlackCaptures = atoi($11);
                    $$->BlackTime     = atoi($12);
                    $$->BlackByo      = atoi($13);
                    $$->WhiteName     = $2;
                    $$->WhiteName2    = $20;
                    $$->WhiteCaptures = atoi($4);
                    $$->WhiteTime     = atoi($5);
                    $$->WhiteByo      = atoi($6);
		    /* We must create the game now, since the "games Id"
                     * command will not give the 3rd and 4th players:
                     */
                    TeamGame($$->Id, $17, $18, $19, $20, 0);
                    myfree($4);
                    myfree($5);
                    myfree($6);
                    myfree($8);
                    myfree($11);
                    myfree($12);
                    myfree($13);
                    myfree($17);
                    myfree($18);
                }
            ;

optgametitle: GAMETITLE NAME { $$ = $2; }
            |                { $$ = 0;  }
            ;

add         : ADD { SeenAdd = 1; }
            ;

doneobserve : DONE observe
                {
                    Done();
                }
            ;

mustpass    : MUSTPASS
                {
                    MyGameMessage($1);
                    myfree($1);
                }
            ;

oppmustpass : OPPMUSTPASS
                {
                    MyGameMessage($1);
                    myfree($1);
                }
            ;

disagreeremove: DISAGREEREMOVE
                {
                    MyGameMessage("There is a disagreement about the "
                                  "life/death of that stone. "
                                  "The game will resume.");
                    RestoreFromScoring();
                    MyGameMessage("Board is restored to what it was"
                                  " before you started scoring");
                }
            ;

opponentdisagreeremove: OPPDISAGREEREMOVE
                {
                    MyGameMessage("There is a disagreement about the "
                                  "life/death of that stone. "
                                  "The game will resume.");
                    RestoreFromScoring();
                }
            ;

optnotreviewing : NOTREVIEWING
                    { /* teach invalid-game. The gamdesc will force "games"
                       * since teaching games are not announced.
                       */
		      Output("Game not found\n");
                    }
                |
                ;

observe     : gamedesc movelist optgametitle optgamesaved optnotreviewing
                {
                    Game *game;
                    int   Nr;

		    /* Do not rely on WhatCommand(NULL, "moves"), this creates
                     * a race condition. Instead assume that the movelist
		     * is a result of "moves <id>" if it starts with move 0.
                     */
		    if (SeenAdd || $2->Next == $2 ||
			strcmp($2->Next->Name, "0")) {
                        game = AddMove(SeenAdd, $1, $2);
/*                      if (game && !MyGameP(game)) ChangeCommand(NULL, -1); */
                        if (game) SetGameTitle(game, $3);
                        myfree($3);
                    } else {
		        Moves($1, $2);
		    }
                    FreeGameDesc($1);
                    FreeNameValList($2);
                }
            |
            gamedesc movelist OBSERVETEAM NAME NAME NAME NAME END optobserve
	    optgametitle optgamesaved
                {
                    Game *game;
                    int   Nr;

		    $1->BlackName2 = strcmp($1->BlackName, $4) ? $4 : $6;
		    $1->WhiteName2 = strcmp($1->WhiteName, $5) ? $5 : $7;
		    if (SeenAdd || $2->Next == $2 ||
			strcmp($2->Next->Name, "0")) {
                        game = AddMove(SeenAdd, $1, $2);
/*                      if (game && !MyGameP(game)) ChangeCommand(NULL, -1); */
                        if (game) SetGameTitle(game, $10);
                        myfree($10);
                    } else {
		        Moves($1, $2);
		    }
                    FreeGameDesc($1);
                    FreeNameValList($2);
                }
            ;

optfirst    : FIRSTREMOVE { $$ = $1; }
            |             { $$ = Empty; }
            ;

/*
With bell on, opponent types last pass, can have either:

  9 You can check your score with the score command, type 'done' when finished.

  15 Game 11 I: jl (0 285 -1) vs jloup (0 267 -1)
  15   4(B): Pass
  2 Game saved.
  2 
  1 7
or:
  9 You can check your score with the score command, type 'done' when finished.

  15 Game 14 I: jl (0 266 -1) vs jloup (0 216 -1)
  15  10(B): Pass
  2 
  1 7

With bell off, opponent types last pass, can have either:

  9 You can check your score with the score command, type 'done' when finished.

  15 Game 57 I: jl (0 174 -1) vs jloup (0 164 -1)
  15   4(B): Pass
  2 Game saved.

  1 7
or:
  9 You can check your score with the score command, type 'done' when finished.

  15 Game 75 I: jl (0 222 -1) vs jloup (0 182 -1)
  15   8(B): Pass

  1 7
*/

doneopponentobserve: DONE optfirst SEMIPROMPT opponentoptobserve
                {
                    Done();
                    if ($2 != Empty)
                        MyGameMessage("%s needs to remove a group first.",
                                      $2 == White ? "White" : "Black");
                }
            ;

opponentobserve: gamedesc movelist optgametitle OBSERVE optgamesaved
                {
                    Game *game;

                    game = AddMove(0, $1, $2);
                    FreeGameDesc($1);
                    FreeNameValList($2);
                    if (game) SetGameTitle(game, $3);
                    myfree($3);
                }
            ;

opponentoptobserve: gamedesc movelist optgametitle OBSERVE optgamesaved
                {
                    Game *game;

                    game = AddMove(0, $1, $2);
                    FreeGameDesc($1);
                    FreeNameValList($2);
                    if (game) SetGameTitle(game, $3);
                    myfree($3);
                }
            | gamedesc movelist optgametitle SEMIPROMPT optgamesaved
                {
                    Game *game;

                    game = AddMove(0, $1, $2);
                    FreeGameDesc($1);
                    FreeNameValList($2);
                    if (game) SetGameTitle(game, $3);
                    myfree($3);
                }
            ;

betresult   : BETRESULT opponentobserve
                {
                    AutoCommand(NULL, "%%bet bet");
                }
            |  BETRESULT observe
                {
                    AutoCommand(NULL, "%%bet bet");
                }
            ;

undidlist   : undidlist UNDID NAME
                {
                    /* undo of multiple moves allowed in a teaching game */
		    MyGameUndo($3);
                    myfree($3);
                }
            |  UNDID NAME
                {
		    MyGameUndo($2);
                    myfree($2);
                }
            ;

undid       : undidlist
              gamedesc movelist optgametitle optgamesaved
                {
                    FreeGameDesc($2);
                    FreeNameValList($3);
                    /* if (game) SetGameTitle(game, $4); */
                    myfree($4);
                }
            | undidlist gamedesc movelist
              OBSERVETEAM NAME NAME NAME NAME END optgametitle optgamesaved
                {
                    FreeGameDesc($2);
                    FreeNameValList($3);
                    /* if (game) SetGameTitle(game, $10); */
                    myfree($10);
                }
            ;

opponentundid: UNDID NAME OBSERVE
               gamedesc movelist optgamesaved
                {
                    char *ptr;

                    ptr = strchr($2, ')');
                    if (ptr) *ptr = 0;
                    else YYFAIL;
                    Undo(0, $4->Id, $4->BlackName, $4->WhiteName, $2);
                    myfree($2);
                    FreeGameDesc($4);
                    FreeNameValList($5);
                }
             | UNDID NAME OBSERVE gamedesc movelist
               OBSERVETEAM NAME NAME NAME NAME END optgamesaved
                {
                    char *ptr;

                    ptr = strchr($2, ')');
                    if (ptr) *ptr = 0;
                    else YYFAIL;
                    Undo(0, $4->Id, $4->BlackName, $4->WhiteName, $2);
                    myfree($2);
                    FreeGameDesc($4);
                    FreeNameValList($5);
                }
            ;

restore     : RESTORE {}
            ;

opponentrestart: RESTART gamedesc movelist OBSERVE
                {
                    Resume($2->Id, $2->BlackName, $2->WhiteName,
                           $3->Previous->Name ?
                           1+atoi($3->Previous->Name) : 0);
                    FreeGameDesc($2);
                    FreeNameValList($3);
                }
            | RESTART gamedesc movelist RESTARTTEAMGAME NAME NAME NAME NAME END
              OBSERVE
                {
                    Game *game;
		    /* We must create the game now, since the "games Id"
                     * command will not give the 3rd and 4th players:
		     */
                    game = ResumeTeam($2->Id, $5, $6, $7, $8,
                                  $3->Previous->Name ?
                                  1+atoi($3->Previous->Name) : 0);
                    if (game) SetGameTitle(game, $10);
                    FreeGameDesc($2);
                    FreeNameValList($3);
                    myfree($10);
                }
            ;

restart     : RESTART gamedesc movelist optgametitle
                {
                    Game *game;

                    game = Resume($2->Id, $2->BlackName, $2->WhiteName,
                                  $3->Previous->Name ?
                                  1+atoi($3->Previous->Name) : 0);
                    if (game) SetGameTitle(game, $4);
                    FreeGameDesc($2);
                    FreeNameValList($3);
                    myfree($4);
                }
            | RESTART gamedesc movelist RESTARTTEAMGAME NAME NAME NAME NAME END
              optgametitle
                {
                    Game *game;
		    /* We must create the game now, since the "games Id"
                     * command will not give the 3rd and 4th players:
		     */
                    game = ResumeTeam($2->Id, $5, $6, $7, $8,
                                  $3->Previous->Name ?
                                  1+atoi($3->Previous->Name) : 0);
                    if (game) SetGameTitle(game, $10);
                    FreeGameDesc($2);
                    FreeNameValList($3);
                    myfree($10);
                }
            ;

newmatch1   : OBSERVE newmatch2 {}
            ;

newmatch2   : gamedesc NEWMATCH
                {
                    SendCommand(NULL, INT_TO_XTPOINTER($1->Id+1),
				"games %d", $1->Id);
                    /* INT_TO_XTPOINTER(Id+1) will set ForceNew = Id
                     in AssertGamesDeleted() */
                    FreeGameDesc($1);
                }
            ;

decline     : DECLINE
                {
                    Decline($1);
                }
            ;

disputeline : PERSON GAMECOLOR NAME GAMESECONDS BYOYOMI END
                {
                    $$ = mynew(DisputeDesc);
                    $$->Player = $1;
                    $$->Color  = $2;
                    $$->SizeX  = atoi($3);
                    $$->SizeY  = atoi(strchr($3, 'x')+1);
                    $$->Tim    = $4;
                    $$->ByoYomi= $5;
                    myfree($3);
                }
            ;

disputelines: disputelines disputeline
                {
                    $2->Next     = $1;
                    $2->Previous = $1->Previous;
                    $2->Next->Previous = $2->Previous->Next = $2;
                    $$ = $1;
                }
            |
                {
                    $$ = mynew(DisputeDesc);
                    $$->Next = $$->Previous = $$;
                }
            ;

opponentdispute: OPPONENTDISPUTE disputelines
                {
                    DisputeDesc *Here, *Next;

                    Dispute($2, 1);
                    for (Here = $2->Next; Here != $2; Here = Next) {
                        Next = Here->Next;
                        myfree(Here);
                    }
                    myfree($2);
                }
            ;

dispute     : DISPUTE disputelines
                {
                    DisputeDesc *Here, *Next;

                    Dispute($2, 0);
                    for (Here = $2->Next; Here != $2; Here = Next) {
                        Next = Here->Next;
                        myfree(Here);
                    }
                    myfree($2);
                }
            ;

matchtypes  : matchtypes MATCHTYPE
                {
                    $$ = $1 | $2;
                }
            |
                {
                    $$ = 0;
                }
            ;

disputematchtype: DISPUTEMATCHTYPE matchtypes END
                {
                    WantMatchType($1, $2);
                }
            ;

optobserve  : OBSERVE
                {
                }
            |
            ;


undolist   : undolist optobserve UNDO NAME NAME NAME
                {             /* gameid white black move */
		    Undo(0, $3, $5, $4, $6);
                    myfree($6);
                    myfree($5);
                    myfree($4);
                }
            |  UNDO NAME NAME NAME
                {
		    Undo(0, $1, $3, $2, $4);
                    myfree($4);
                    myfree($3);
                    myfree($2);
                }
            ;

undo        : undolist optobserve gamedesc movelist optgametitle
                {
                    FreeGameDesc($3);
                    FreeNameValList($4);
                    /* if (game && $5) SetGameTitle(game, $5); */
                    myfree($5);
                }
            | undolist optobserve gamedesc movelist
              OBSERVETEAM NAME NAME NAME NAME END optgametitle
                {
                    FreeGameDesc($3);
                    FreeNameValList($4);
                    /* if (game && $11) SetGameTitle(game, $11); */
                    myfree($11);
                }
            ;

watching    : WATCHING names END
                {
                    Watching($2);
                    FreeNameList($2);
                }
            ;

overobserve : OVEROBSERVE
                {
                    OverObserve($1);
                }
            ;

observewhileplay: OBSERVEWHILEPLAY
                {
                    ObserveWhilePlaying();
                }
            ;

playerline  : PLAYERS NAME NAME NAME NAME END
                {
                    FindPlayer($3, $5, $2, $4);
                    myfree($2);
                    myfree($3);
                    myfree($4);
                    myfree($5);
                }
            | PLAYERS NAME END
                {
                    FindPlayer($2, "???", "?????  ???", UNKNOWN);
                    myfree($2);
                }
            ;

playerlines : playerlines playerline
            |
                {

                }
            ;

playersstatusline: NAME '(' NAME ')' NAME NAME NAME NAME NAME NAME END
                {
                    PlayerStatusLine(atoi($1), atoi($3), atoi($7));
                    myfree($1);
                    myfree($3);
                    myfree($5);
                    myfree($6);
                    myfree($7);
                    myfree($8);
                    myfree($9);
                    myfree($10);
                }
            | NAME NAME NAME NAME NAME NAME END
                {
                    PlayerStatusLine(atoi($1), -1, atoi($3));
                    myfree($1);
                    myfree($2);
                    myfree($3);
                    myfree($4);
                    myfree($5);
                    myfree($6);
                }
            ;

players     : PLAYERS
                {
                    AssertPlayersDeleted();
                }
              playerlines END
                {
                    TestPlayersDeleted();
                }
              playersstatusline
            ;

userline    : names END
                {
                    NameList *Here;
                    int n;

                    n =0;
                    for (Here = $1->Next; Here != $1; Here = Here->Next) n++;
                    if (n == 11) {
		      $$ = $1;
		    } else if (n == 0) { /* list header */
		      $$ = NULL;
                    } else {
                        /* Don't call YYFAIL. user command leads to easily
                           to parse errors */
		        Output("Got the expected parse error following a "
                               "\"user\" command:\n");
                        for (Here = $1->Next; Here != $1; Here = Here->Next) {
                            Output(Here->Name);
                            Output(" ");
                        }
                        Output("\n");
                        _IgsDefaultParse();
                        FreeNameList($1);
                        $$ = NULL;
                    }
                }
            | names FAIL
                {
                    NameList *Here;

                    Output("Got the expected parse failure following a "
                           "\"user\" command:\n");
                    for (Here = $1->Next; Here != $1; Here = Here->Next) {
                        Output(Here->Name);
                        Output(" ");
                    }
                    Output("\n");
                    _IgsDefaultParse();
                    FreeNameList($1);
                    $$ = NULL;
                }
            ;

userlines   : userlines userline
                {
                    if ($2) {
                        NameListList *Last;
                        Last = mynew(NameListList);
                        Last->Names    = $2;
                        Last->Previous = $1->Previous;
                        Last->Next     = $1;
                        Last->Next->Previous = Last->Previous->Next = Last;
                    }
                    $$ = $1;
                }
            |
                {
                    $$ = mynew(NameListList);
                    $$->Previous = $$->Next = $$;
                    $$->Names = NULL;
                }
            ;

users       : USER userlines
                {
                    UserData($2);
                    FreeNameListList($2);
                }
            ;

player      : NAME '[' NAME ']'
                {
                    $$ = FindPlayerByNameAndStrength($1, $3);
                    myfree($1);
                    myfree($3);
                }
            ;

playertime  : NAME ':' NAME
                {
                    int sec;

                    sec = atoi($3);
                    if ($1[0] == '-') $$ = -60 * atoi($1+1)-sec;
                    else              $$ =  60 * atoi($1)  +sec;
                    myfree($1);
                    myfree($3);
                }
            ;

optbyo      : '(' NAME ')' NAME
                {
                    $$ = atoi($4);
                    myfree($2);
                    myfree($4);
                }
            |   { $$ = -1; }
            ;

gametime    : GAMETIME NAME ':' NAME END
              GAMETIME NAME '(' NAME ')' ':' playertime optbyo END
              GAMETIME NAME '(' NAME ')' ':' playertime optbyo END
                {
                    if (strcmp($2, "Game") ||
                        strcmp($7, "White") || strcmp($16, "Black")) YYFAIL;
                    GameTime(atoi($4), $18, $21, $22, $9, $12, $13);
                    myfree($2);
                    myfree($4);
                    myfree($7);
                    myfree($9);
                    myfree($16);
                    myfree($18);
                }
            ;

gamescore   : CURRENTSCORE NAME FINALSCORE NAME
		{
                    int Nr;
                    Game     *game;

                    if (!UserCommandP(NULL) &&
                        (Nr = WhatCommand(NULL, "score")) >= 0 &&
                        (game = ServerIdToGame(Nr)) != NULL) {
                        GameMessage(game, "..........", "Current score:");
                        GameMessage(game, "..........", "%s", $2);
                        GameMessage(game, "..........", "Final score:");
                        GameMessage(game, "..........", "%s", $4);
                    } else {
                        Outputf("Current score:\n %s\n", $2);
                        Outputf("Final score:\n %s\n", $4);
                    }
                    myfree($2);
                    myfree($4);
                }
            ;

translation : TRANSLATION NAME
                {
                    Outputf("%s\n", $2);
                    myfree($2);
                }
            ;

translations: translations translation
            | translation
            ;

byoyomi     : ENTERBYOYOMI GIVEBYOYOMI
                {
                    MyGameMessage("%s is now in byo-yomi, having %s",
                                  PlayerString($1), $2);
                    myfree($2);
                }
            ;

notime      : NOTIME
                {
                    MyGameMessage("%s has run out of time.", PlayerString($1));
                }
            ;

lostconnection: LOSTCONNECTION MYADJOURN optgamesaved
                {
                    MyGameMessage("Your opponent has lost his connection.");
                }
            ;

gamesaved   : GAMESAVED NAME optobserve
                {
		    if (appdata.WantVerbose) {
                        MyGameMessage("Game saved.%s", $2);
		    }
                    myfree($2);
                }
            ;

optadjourn : MYADJOURN
                {
                }
            |
            ;


adjourn     : MYADJOURN optadjourn GAMESAVED NAME optgamesaved
                {
                    MyGameMessage("Game has been adjourned.");
                    MyGameMessage("Game saved.%s", $4);
                    myfree($4);
                }
            ;

adjournsentrequest: ADJOURNSENTREQUEST
                {
                }
            ;

adjournrequest: ADJOURNREQUEST
                {
                    MyGameMessage("Your opponent requests an adjournment");
                    MyGameMessage("Use the <adjourn> or <decline adjourn> "
                                  "entries in the commands menu.");
                }
            ;

oppadjourn  : MYADJOURN
                {
                    MyGameMessage("Game has been adjourned.");
                }
            ;

declineadjourn: DECLINEADJOURN
                {
                    MyGameMessage("Your opponent declines to adjourn.");
                }
            ;

resign      : RESIGN
                {
                    MyGameMessage("%s has resigned the game.",
                                  PlayerString($1));
                }
            | RESIGN RESIGN
                { /* Double message in teaching game --Ton */
                    MyGameMessage("%s has resigned the game.",
                                  PlayerString($1));
                }
            ;

mailed      : MAILED MAILED
               {
                   Mailed($1);
                   Mailed($2);
                   myfree($1);
                   myfree($2);
               }
            | MAILED
               {
                   Mailed($1);
                   myfree($1);
               }
            ;

removegamefile: REMOVEGAMEFILE
               {
                   RemoveGameFile($1);
                   myfree($1);
               }
            ;

notelltarget: NOTELLTARGET
                {
                    NoTell();
                }
            ;

telltarget  : TELLTARGET NAME END
                {
                    /* Kludge to stop bell/raise at telltarget change --Ton */
                    int OldEntered;

                    OldEntered = Entered;
                    Entered = 0;
		    if (appdata.WantVerbose) {
                        Outputf("Setting your '.' to %16s\n",
                                PlayerNameToString($2));
		    }
                    Entered = OldEntered;
                    myfree($2);
                }
            ;

telldone    : TELLDONE
                {
                }
            ;

telloff     : TELLOFF names
                {
                    NameList *Here;

                    Output("User is not accepting tells.\n");
                    for (Here = $2->Next; Here != $2; Here = Here->Next)
                        Outputf("%s\n", Here->Name);
                    FreeNameList($2);
                }
            ;

illegalmove : ILLEGALMOVE
                {
                    MyGameMessage("Illegal move: %s", $1);
                    myfree($1);
                }
            | ILLEGALMOVE OBSERVE optobserve
                {
                    MyGameMessage("Illegal move: %s", $1);
                    myfree($1);
                }
            ;

illegalundo : ILLEGALUNDO
                {
                    MyGameMessage("Cannot undo: %s", $1);
                    myfree($1);
                }
            ;

noturn      : NOTURN
                {
                    MyGameMessage("It isn't your turn");
                }
            ;

noremoveturn: NOREMOVETURN
                {
                    MyGameMessage("It is not your turn to remove a group");
                }
            ;

useresign   : USERESIGN
                {
                    MyGameMessage("To resign, please use 'resign'");
                }
            ;

removeliberty: REMOVELIBERTY
                {
                    MyGameMessage("You cannot remove liberties.");
                }
            ;

removegroup : REMOVEGROUP
                {
                    RemoveGroup($1);
                    myfree($1);
                }
            ;

restorescoring: RESTORESCORING
                {
                    RestoreScoring();
                }
            ;

pleaseredone: PLEASEREDONE
                {
                    MyGameMessage("Please repeat 'done'");
                }
            ;

statusheader: STATUSHEADER names END
                {
                    $$ = $2;
                }
            ;

statusline  : STATUSLINE NAME
                {
                    $$ = mynew(NumVal);
                    $$->Num   = $1;
                    $$->Value = $2;
                }
            ;

statuslines : statuslines statusline
                {
                    $2->Next = $1;
                    $2->Previous = $1->Previous;
                    $2->Next->Previous = $2->Previous->Next = $2;
                    $$ = $1;
                }
            |
                {
                    $$ = mynew(NumVal);
                    $$->Next  = $$->Previous = $$;
                    $$->Num   = -1;
                    $$->Value = NULL;
                }
            ;

resultline  : RESULTLINE
                {
	            /* 20 jl (W:O):  2.5 to jloup (B:#):  3.0 */
                    MyGameMessage("%s", $1);
                    myfree($1);
                }
            ;

status      : statusheader statusheader statuslines
                {
                    if (GamePosition($2, $1, $3)) ChangeCommand(NULL, 1);
                    FreeNameList($1);
                    FreeNameList($2);
                    FreeNumValList($3);
                }
            ;

date        : NAME NAME NAME NAME ':' NAME ':' NAME NAME
                {
                    struct tm *FullTime;
                    int        i;
                    static const char *Month[] = {
                        "Jan", "Feb", "Mar", "Apr", "May", "Jun",
                        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

                    FullTime = mynew(struct tm);
                    for (i=0; i<12; i++) if (strcmp($2, Month[i]) == 0) break;
                    FullTime->tm_year  = atoi($9)-1900;
                    FullTime->tm_mon   = i;
                    FullTime->tm_mday  = atoi($3);
                    FullTime->tm_hour  = atoi($4);
                    FullTime->tm_min   = atoi($6);
                    FullTime->tm_sec   = atoi($8);
                    FullTime->tm_isdst = LocalTime.tm_isdst;
                    mktime(FullTime);
                    $$ = FullTime;
                    myfree($1);
                    myfree($2);
                    myfree($3);
                    myfree($4);
                    myfree($6);
                    myfree($8);
                    myfree($9);
                }
            ;

uptimeentry : GMTTIME date END
                {
                    char *ptr;
                    int   Length;

		    if (appdata.WantVerbose) {
			ptr = asctime($2);
			Length = strlen(ptr);
			Outputf("Gmt    time: %.*s\n", Length-1, ptr);
		    }
                    myfree($2);
                }
            | LOCALTIME date END
                {
                    char *ptr;
                    int   Length;

                    ServerTime = *(struct tm *) $2;
                    SetServerTime = 1;
		    if (appdata.WantVerbose) {
			ptr = asctime($2);
			Length = strlen(ptr);
			Outputf("Server time: %.*s\n", Length-1, ptr);
		    }
                    myfree($2);
                }
            | SERVERUP NAME NAME NAME NAME NAME NAME END
                {
                    long Uptime;

		    if (appdata.WantVerbose) {
			Uptime = ((24*atoi($2) + atoi($4))*60 + atoi($6)) * 60;
			Outputf("Uptime: %s %s %s %s %s %s (%ld seconds)\n",
				$2, $3, $4, $5, $6, $7, Uptime);
		    }
                    myfree($2);
                    myfree($3);
                    myfree($4);
                    myfree($5);
                    myfree($6);
                    myfree($7);
                }
            | UPTIMEENTRY NAME
                {
		    if (appdata.WantVerbose) {
			Outputf("%s\n", $2);
		    }
                    myfree($2);
                }
            ;

uptime      : uptime uptimeentry
            | uptimeentry
            ;

sgflist     : SGFLIST names END
                {
                    SgfList($2);
                    FreeNameList($2);
                }
            | SGFLIST NOSGF
                {
                    Output("sgf needs arguments\n");
                }
            ;

reviewlist  : REVIEWLIST names
                {
                    ReviewList($2);
                    FreeNameList($2);
                }
            ;

reviewvariations: REVIEWVARIATIONS names
                {
/* For the moment we just ignore the variations list
                    NameList *Here;

                    Output("Variations:");
                    for (Here = $2->Next; Here != $2; Here = Here->Next)
                        Outputf(" %s", Here->Name);
                    Output("\n");
*/
                    FreeNameList($2);
                }
            ;

reviewstart : REVIEWSTART
                {
                    ReviewStart($1);
                    myfree($1);
                }
            ;

reviewliterals: reviewliterals REVLITERAL
                {
                    NameList *names;

                    names = mynew(NameList);
                    names->Name     = $2;
                    names->Next     = $1;
                    names->Previous = $1->Previous;
                    names->Next->Previous = names->Previous->Next = names;
                    $$ = $1;
                }
            |
                {
                    NameList *header;

                    header = mynew(NameList);
                    header->Name = NULL;
                    header->Next = header->Previous = header;
                    $$ = header;
                }
            ;

reviewentry : '(' { ReviewOpenVariation(); }
            | ')' { ReviewCloseVariation(); }
            | REVNODE
                {
                    ReviewNewNode();
                }
            | REVUNKNOWN reviewliterals
                {
                    NameList *Here;

                    Outputf("Unknown: %s:", $1);
                    for (Here = $2->Next; Here != $2; Here = Here->Next)
                        Outputf(" %s", Here->Name);
                    Output("\n");
                    myfree($1);
                    FreeNameList($2);
                }
            | REVNODENAME REVLITERAL
                {
                    NameList Entry;

                    Entry.Name = $2;
                    Entry.Next = Entry.Previous = &Entry;
                    ReviewLocalProperty(retNODENAME, &Entry);
                    myfree($2);
                }
            | REVCOMMENT REVLITERAL
                {
                    NameList Entry;

                    Entry.Name = $2;
                    Entry.Next = Entry.Previous = &Entry;
                    ReviewLocalProperty(retCOMMENT, &Entry);
                    myfree($2);
                }
            | REVKOMI REVLITERAL
                {
                    ReviewGlobalProperty(retKOMI, $2);
                    myfree($2);
                }
            | REVHANDICAP REVLITERAL
                {
                    ReviewGlobalProperty(retHANDICAP, $2);
                    myfree($2);
                }
            | REVUSER REVLITERAL
                {
                    ReviewGlobalProperty(retENTEREDBY, $2);
                    myfree($2);
                }
            | REVCOPYRIGHT REVLITERAL
                {
                    ReviewGlobalProperty(retCOPYRIGHT, $2);
                    myfree($2);
                }
            | REVPLACE REVLITERAL
                {
                    ReviewGlobalProperty(retPLACE, $2);
                    myfree($2);
                }
            | REVDATE REVLITERAL
                {
                    ReviewGlobalProperty(retDATE, $2);
                    myfree($2);
                }
            | REVRESULT REVLITERAL
                {
                    ReviewGlobalProperty(retRESULT, $2);
                    myfree($2);
                }
            | REVEVENT REVLITERAL
                {
                    ReviewGlobalProperty(retTOURNAMENT, $2);
                    myfree($2);
                }
            | REVGAMENAME REVLITERAL
                {
                    ReviewGlobalProperty(retNAME, $2);
                    myfree($2);
                }
            | REVWHITERANK REVLITERAL
                {
                    ReviewGlobalProperty(retWHITESTRENGTH, $2);
                    myfree($2);
                }
            | REVBLACKRANK REVLITERAL
                {
                    ReviewGlobalProperty(retBLACKSTRENGTH, $2);
                    myfree($2);
                }
            | REVWHITENAME REVLITERAL
                {
                    ReviewGlobalProperty(retWHITENAME, $2);
                    myfree($2);
                }
            | REVBLACKNAME REVLITERAL
                {
                    ReviewGlobalProperty(retBLACKNAME, $2);
                    myfree($2);
                }
            | REVSIZE REVLITERAL
                {
                    ReviewGlobalProperty(retSIZE, $2);
                    myfree($2);
                }
            | REVGAME REVLITERAL
                {
                    ReviewGlobalProperty(retGAME, $2);
                    myfree($2);
                }
            | REVWHITE REVLITERAL
                {
                    NameList Entry;

                    Entry.Name = $2;
                    Entry.Next = Entry.Previous = &Entry;
                    ReviewLocalProperty(retWHITE, &Entry);
                    myfree($2);
                }
            | REVBLACK REVLITERAL
                {
                    NameList Entry;

                    Entry.Name = $2;
                    Entry.Next = Entry.Previous = &Entry;
                    ReviewLocalProperty(retBLACK, &Entry);
                    myfree($2);
                }
            | REVLETTERS reviewliterals
                {
                    ReviewLocalProperty(retLETTERS, $2);
                    FreeNameList($2);
                }
            | REVWHITETIME REVLITERAL
                {
                    NameList Entry;

                    Entry.Name = $2;
                    Entry.Next = Entry.Previous = &Entry;
                    ReviewLocalProperty(retWHITETIME, &Entry);
                    myfree($2);
                }
            | REVBLACKTIME REVLITERAL
                {
                    NameList Entry;

                    Entry.Name = $2;
                    Entry.Next = Entry.Previous = &Entry;
                    ReviewLocalProperty(retBLACKTIME, &Entry);
                    myfree($2);
                }
            | REVADDBLACK reviewliterals
                {
                    ReviewLocalProperty(retBLACKSET, $2);
                    FreeNameList($2);
                }
            | REVADDWHITE reviewliterals
                {
                    ReviewLocalProperty(retWHITESET, $2);
                    FreeNameList($2);
                }
            | REVADDEMPTY reviewliterals
                {
                    ReviewLocalProperty(retEMPTYSET, $2);
                    FreeNameList($2);
                }
            ;

reviewentries: reviewentries reviewentry
            |
            ;

review      : REVIEWTYPE
                {
                    ReviewEntryBegin($1);
                }
              reviewentries END
            ;

auxreviews  : auxreviews review
            | review
            ;

reviews     : auxreviews
                {
                    ReviewEnd(0);
                }
            | auxreviews REVIEWEND
                {
                    ReviewEnd(1);
                }
            ;

reviewstop  : REVIEWSTOP
                {
                    ReviewStop();
                }
            ;

noreview    : NOREVIEW
                {
                    ReviewNotFound();
                }
            ;

throwcopy   : THROWCOPY
                {
                    Output("You are already logged on. "
                           "Throwing other copy out\n");
                }
            ;

proba       : PROBA RATING RATING NAME NAME NAME
                { /* my rating, their rating, handicap,
                   * proba lose as white, proba lose as black
                   */
                    MyLoseProbas($1, $2, $3, $4, $5, $6);
                    myfree($4);
                    myfree($5);
                    myfree($6);
                }
	    | PROBA END
                { /* other player does not have a rating */
                }
            ;

setproba    : SETPROBA RATING RATING NAME NAME NAME
                { /* my rating, their rating, handicap,
                   * proba lose as white, proba lose as black
                   */
                    MyLoseProbas(NULL, $2, $3, $4, $5, $6);
                    myfree($4);
                    myfree($5);
                    myfree($6);
                }
	    | SETPROBA END
                { /* I do not have a rating */
                }
            ;

sorry       : SORRY
                {
                    if (ArgsCommand(NULL, ";")) ChannelDisallowed();
                    else Output("Sorry.\n");
                }
            ;

invalid     : INVALID
                {
                    Outputf("Unknown command %s\n", $1);
                    myfree($1);
                }
            ;

unknown     : UNKNOWNANSWER literallines
                {
                }
            ;

names       : names NAME
                {
                    NameList *names;

                    names = mynew(NameList);
                    names->Name     = $2;
                    names->Next     = $1;
                    names->Previous = $1->Previous;
                    names->Next->Previous = names->Previous->Next = names;
                    $$ = $1;
                }
            |   {
                    NameList *header;

                    header = mynew(NameList);
                    header->Name = NULL;
                    header->Next = header->Previous = header;
                    $$ = header;
                }
            ;

namesset    : namesset names END
                {
                    $1->Previous->Next = $2->Next;
                    $2->Next->Previous = $1->Previous;
                    $2->Previous->Next = $1;
                    $1->Previous       = $2->Previous;
                    myfree($2);
                    $$ = $1;
                }
            |   {
                    NameList *header;

                    header = mynew(NameList);
                    header->Name = NULL;
                    header->Next = header->Previous = header;
                    $$ = header;
                }
            ;

promptnames : promptnames promptname
            |
            ;

promptname  : OLDPROMPT       {}
            | NAME
                {
                    Outputf("%s\n", $1);
                    myfree($1);
                }
            ;

optname     : NAME { $$ = $1;   }
            |      { $$ = NULL; }
            ;

literallines: literallines NAME
                {
                    char *ptr;

                    ptr = $2;
                    if (ptr[0] == '\r') ptr++;
                    if (isdigit(ptr[0])) {
                        ptr++;
                        while (isdigit(ptr[0])) ptr++;
                        if (ptr[0] == ' ') ptr++;
                    }
                    Outputf("%s\n", ptr);
                    myfree($2);
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
            |
            ;
%%
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

static void yyerror(const char *s)
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
