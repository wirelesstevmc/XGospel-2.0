/*
 * From a message sent by Tim Casey to clients@panda-igs.joyjoy.net 25 Nov 1998
 *
  0,	UNKNOWN	 unknown
  1,    PROMPT	 A Prompt (never)
  2,	BEEP	 \7 telnet
  3,	BOARD	 Board being drawn
  4,	DOWN	 The server is going down
  5,	ERROR	 An error reported
  6,    FIL	 File being sent      (not used)
  7,	GAMES	 Games listing
  8,    HELP	 Help file
  9,	INFO	 Generic info
 10,	LAST	 Last command
    10 File
    jl @ Thu Nov 26 02:54:29 1998
    10 File
 11,    KIBITZ	 Kibitz strings
 12,	LOAD	 Loading a game
 13,	LOOK_M	 Look
    13 The date on the game is Tue Nov 24 02:06:20 1998
    13 Black adjourned from this game.
 14,    MESSAGE	 Message lising
 15,    MOVE	 Move #:(B) A1
 16,	OBSERVE	 Observe report
 17,	REFRESH	 Refresh of a board
 18,    SAVED	 Stored command
    18 jl-jloup             
    18 Found 1 stored games.
 19,    SAY	 Say string
    51 Say in game 10

    19 *jloup*: test 4
 20,    SCORE_M	 Score report
 21,    SHOUT	 Shout string
 22,    STATUS	 Current Game status
    22 jl 30k  0 880 9 F 0.5 0
    22 jloup NR  0 835 9 F 0.5 0
    22  0: 413333330
    22  1: 133333333
    22  2: 333333333
    22  3: 333333333
    22  4: 333333333
    22  5: 333333333
    22  6: 333333333
    22  7: 333333330
    22  8: 333333305
    20 jl (W:O):  1.5 to jloup (B:#):  1.0
    
    9 game completed.
    1 7
    
    21 {Game 57: jl vs jloup : W  1.5 B  1.0}
    1 5
    
    9 Removed game file jl-jloup from database.
 23,	STORED	 Stored games   (not used)
 24,    TELL	 Tell string
    2 

    24 *jloup*:  test
 25,	THIST	 Thist report
    * (1) results -kido
    25 File
    kido       [ 4d*](W) : wildwolf   [ 4d ](B) H 0 K  5.5 19x19 W+Resign 26-00-34 R
    kido       [ 4d*](B) : niubi      [ 4d*](W) H 0 K  5.5 19x19 B+Time 26-01-24 R
    25 File
 26,	TIM	 times command
    * (1) time 16
    26 Game : 16
    26 White(tw) : -0:01 (B) 6
    26 Black(agneau) : 0:50 (B) 8
    9 White 5 Black 5
    1 8
 27,	WHO	 who command
 28,	UNDO	 Undo report
    (undo 139 3)
    28 jl undid the last move (D4) .
    28 jl undid the last move (D16) .
    28 jl undid the last move (R4) .

    15 Game 139 I: jl (0 0 -1) vs jl (0 50 -1)
    15   0(B): Q16
    9 Game is titled: cafard-freeheart-27-20-34
    1 6
 29,    SHOW 	 Shout string       (super user command)
 30,	TRANS	 Translation info
 31,    not used
 32,	YELL	 Channel yelling
    1 5

    32 70:jl:  test 2
    1 5
 33,    TEACH	 teaching game      (not used)
 34,	SGF_M	 SGF file
 35,	AUTOMAT	 Automatch anouncement
 36,	AUTOASK	 Automatch accept
 37,   TTT_BOARD tic tac toe
 38,	CHOICES  game choices
	choice
	38  2:* tic-tac-toe
	38  1:  go
	38  3:  shogi
	38  4:  chinese-chess
	1 5
      The one with a * is not public.
 39,	VERSION  IGS VERSION: IGS entry on %02d - %02d - %04d\r\n
 40,    DOT	 your . string
 41,	CLIVRFY  Client verify message
 42,	USER	 Long user report
 43,	SGF_F	 SGF command output
 44,	REGLOG	 The reregister reg log.
 45,	FOLLOWS	 A follow pl messages.
 46,	IGS	 multi listing of players.
    * (1) igs
    46 NR   zz         26k* rockthing  10k  mats       6k*  tim        5k*  yslll   

    46 1d   xray       1d*  Ucanrun    7k*  jyz        NR   guest4944  2k*  salem   
 47,	CAPTURE  Capture messages    (after command "cmatch" = Capture match)
    15 Game 17 I: jl (0 894 -1) vs jloup (2 884 -1)
    15   4(B): C1 B1 A1
    2 Game saved.
    ... much later when W plays:
    * (1) A1 115
    47 Black has captured 2 stones.
    15 Game 17 I: jl (0 778 -1) vs jloup (2 884 -1)
    15   5(W): A1
    1 6
 48,	ADJOURN  Multigame adjourn
    1 6
    9 Your opponent requests an adjournment.
    48 Game 36 jloup requests an adjournment
    9 Use <adjourn> to adjourn, or <decline adjourn> to decline.
    1 6

    1 6
    9 Your opponent has lost his/her connection.
    48 Game 9 has been adjourned by xgospel
    9 Game has been adjourned.
    9 Game saved.
    1 5

 49,	REMOVING Multigame stone removal
    9 Removing @ A1
    49 Game 57 jloup is removing @ A1
 50,	GOERR	 An error in a move
    5 Illegal move.
    50 Game #11
    5 Cannot place a stone in an occupied place
    1 6
 51,	SAY_MG	 Say multigame	<-- last
    51 Say in game 10

    19 *jloup*: test 4
*/

%x LOGIN ENTEREDLOGIN ENTEREDPASSWORD CHANNELS NEWCHANNELS CHANNELTITLE
%x NAMELIST THENAMES THELINE THELINES ALLLINE ALLNAMES NUMLINES NUMNAMES
%x FILETEXT PLAYERENTRY STATS STATUS UPTIME EATNL NONCLIENT REVIEWL REVIEW
%x REVIEWLITERAL DISPUTING DISPUTINGMATCHTYPE BET USERENTRY USERENTRYINFO
%x USERENTRYREST UNDIDLINES UNDOWHITE UNDOBLACK TEAMNAMES PROB STOR INFOLINE
%x UNDOMOVE

%{
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <mymalloc.h>

#include "connect.h"
#include "match.h"
#include "players.h"
#include "stats.h"
#include "xgospel.h"
#include "gointer.tab.h"

extern int   isatty(int fd);
static int   NextState;
static int   RevBracks;
#define BUFSTEP 200
static char  *TextBuf = NULL;
static size_t TextLength, TextFill;
static void  unsend(const char *Text);
static void  TextExtend(const char *Text, size_t Length);
static Player *IdToPlayer(const char *From);

extern       int   yylex(void);

#define YYTEXT  ((char *) yytext)
#define YYLENG  ((size_t) yyleng)

#undef YY_INPUT
#define YY_INPUT(buf,result,max_size)                                      \
        result = GetInput(NULL, (char *) buf, (size_t) max_size);

#define YY_USER_INIT                                                       \
                BEGIN(LOGIN);
#define YY_BREAK
#define ECHO                                                                 \
    Warning("Parse error: %d unexpected character '0x%02x' = \"%.*s\"[0]\n", \
            (int) YYLENG, YYTEXT[0], (int) YYLENG, YYTEXT);                  \
    break;
%}

WHITESPACE        [ \t\f\v\r]
WHITE             ({WHITESPACE}+)
WHITESTARCOMMA    [ \t\f\v\r*,]+
WNL               ([ \t\f\v\r]*\n)
BELL              (\x07)
NL                (\r*\n)
CHAR              (.|\n)
DIGIT             [0-9]
NUM               ({DIGIT}+)
TWONUM            ((" "{DIGIT})|({DIGIT}{DIGIT}))
GAMENUM           (" -"|" --"|({DIGIT}{DIGIT}{DIGIT})|(" "{DIGIT}{DIGIT})|("  "{DIGIT})|({DIGIT}{DIGIT})|(" "{DIGIT}))
PROMPT            ("1 "{NUM}{NL})
MODE              (\xFF.\x01)
ID                [^ \[\](){}<>:@\n\t\f\v\r]+
IDNOSLASH         [^ \[\](){}<>:@\n\t\f\v\r/]+
UID               [^ \[\](){}<>:@\n\t\f\v\r,*]+
BADID             [^ !\n\t\f\v\r]+
SPACES            (" "+)
SPECIAL           [\[\](){}]
NZ                ("0"|"9"|"36"|"35")
COLOR             ("white"|"White"|"black"|"Black")
LETTER            ([a-ij-rs-zA-IJ-RS-Z])
WORD              ({LETTER}+)

WRONGPASS         "Invalid password."
SETCLIENT         ("9 Set client to be True."{NL})
%%
<LOGIN>"1 0"{WNL}               |
<INITIAL,LOGIN>"Login: "        { BEGIN(ENTEREDLOGIN);
                                  return LUSER; }
<LOGIN>" "+"Welcome to "{ID}" at "{ID}" " { char *ptr;

                                  ptr = YYTEXT+YYLENG-2;
                                  while (*ptr != ' ') ptr--;
                                  ptr++;
                                  yylval.Name =
                                      mystrndup(ptr, (size_t)
                                                (YYTEXT+YYLENG-ptr-1));
                                  return WELCOME; }
<LOGIN>{CHAR}                   { NextState = LOGIN;
                                  yyless(0);
                                  BEGIN(ALLLINE);
                                  break; }

<ENTEREDLOGIN>"Login: "         { break; }
<ENTEREDLOGIN>"Sorry".*"erver".*"ull".*{WNL} { const char *ptr;

                                  ptr = YYTEXT+YYLENG-1;
                                  while (isspace(*ptr)) ptr--;
                                  yylval.Name =
                                      mystrndup(YYTEXT,
                                                (size_t) (ptr-YYTEXT)-1);
                                  return SERVERFULL; }
<ENTEREDLOGIN>"Password: "      { BEGIN(ENTEREDPASSWORD);
                                  return OLDPASSWORD; }
<ENTEREDLOGIN>"This is a guest account.".*{WNL}"Your account name is "{ID}"."{WNL} { char *ptr1, *ptr2;
                                  ptr2 = strrchr(YYTEXT, '.');
                                  ptr1 = ptr2-1;
                                  while (*ptr1 != ' ') ptr1--;
                                  ptr1++;
                                  yylval.Name =
                                      mystrndup(ptr1, (size_t) (ptr2-ptr1));
                                  BEGIN(ENTEREDPASSWORD);
                                  return GUEST; }
<ENTEREDLOGIN>"9 Player currently exists."{WNL} |
<ENTEREDLOGIN>"9"{WNL}          { break; }
<ENTEREDLOGIN>"1 1"{WHITESPACE}* { /* No \n, forces new line */
                                  BEGIN(0);
                                  return PASSWORD; }
<ENTEREDLOGIN>{CHAR}            { NextState = ENTEREDLOGIN;
                                  yyless(0);
                                  BEGIN(ALLLINE);
                                  break; }
<ENTEREDPASSWORD>"Enter Your Password Again: " {
                                  ForceCommand(NULL, MyPassword);
                                  break; }
<ENTEREDPASSWORD>"5 "?{WRONGPASS}{WNL} {
                                  BEGIN(LOGIN);
                                  return(INVALIDPASSWORD); }
<ENTEREDPASSWORD>{SETCLIENT}    { BEGIN(0);
                                  SetStat("client", 1);
                                  return END; }
<ENTEREDPASSWORD>"#> "          { return OLDPROMPT; }
<ENTEREDPASSWORD>{CHAR}         { NextState = ENTEREDPASSWORD;
                                  yyless(0);
                                  BEGIN(ALLLINE);
                                  break; }
<LOGIN,ENTEREDLOGIN,ENTEREDPASSWORD>{MODE} { break; }

<INITIAL,STATS,DISPUTING,BET,USERENTRY,UPTIME,NEWCHANNELS,CHANNELTITLE,NAMELIST,FILETEXT,REVIEW,REVIEWL,NUMLINES,NUMNAMES,THELINES,ALLNAMES,NONCLIENT>^{MODE}?{PROMPT} {
                                  if (YYTEXT[0] == (char) 0xff)
                                      yylval.Value = atoi(YYTEXT+5);
                                  else yylval.Value = atoi(YYTEXT+2);
                                  BEGIN(0); /* Recover from wrong state */
                                  return PROMPT; }
^"2 "{BELL}"2"?                 { unput('2');
                                  break; }
"2 B W B W...:"{WHITE}          { BEGIN(TEAMNAMES);
                                  return OBSERVETEAM; }
"2"{WNL}                        { return OBSERVE; }

^"3 B W B W...:"{WHITE}         { BEGIN(TEAMNAMES);
                                  return TEAMGAME; }

^"4 "                           { BEGIN(THELINE);
                                  return SERVERMESSAGE; }

^"5"{WNL}                       { break; }
^"5 Sorry."{WNL}                { return SORRY; }
^"5 "{WRONGPASS}{WNL}           { BEGIN(LOGIN);
                                  return INVALIDPASSWORD; }
^"5 Observing too many, limit is "{NUM}" games."{WNL} {
                                  yylval.Value = atoi(YYTEXT+31);
                                  return OVEROBSERVE; }
^"5 You cannot observe while playing."{WNL} { return OBSERVEWHILEPLAY; }
^\n?"5 There is a dispute regarding your match:"{WNL} {
                                  BEGIN(DISPUTING);
                                  return YYTEXT[0] == '\n' ?
                                      OPPONENTDISPUTE : DISPUTE; }
^"5 Illegal move.\n"("50 ".*\n)?"5 ".*\n { const char *ptr;
                                  ptr = strchr(YYTEXT+18, '\n');
                                  yylval.Name = mystrndup(ptr+3, (size_t)
                                                 (YYLENG-(ptr+3-YYTEXT)));
                                  return ILLEGALMOVE; }
^"5 ".*\n("50 ".*\n)?"5 Illegal move.\n"    { const char *ptr;
                                  ptr = strchr(YYTEXT+2, '\n');
                                  yylval.Name = mystrndup(YYTEXT+2, (size_t)
                                                          (ptr-YYTEXT-2));
                                  return ILLEGALMOVE; }
^"5 It is not your turn (while removing)."{WNL} { return NOREMOVETURN; }
^"5 It is not your turn"{WNL}   |
^"5 It isn't your turn."{WNL}   { return NOTURN; }
^"5 It isn't your turn.  To resign, please use the 'resign' command."{WNL} |
^"5 To resign, please use ".*{WNL} { return USERESIGN; }
^"5 You cannot change into a free game " {
                                  BEGIN(THELINE);
                                  return LATEFREE; }
^"5 You aren't playing a game."{WNL} |
^"5 You are not playing."       { return NOPLAY; }
^"5 The other player is not on currently""."?{WNL} { return OPPONENTNOTON; }
^"5 Game failed to load."{WNL}  { return NOLOAD; }
^"5 You are not open for matches." { /* No \n -> forces prompt */
                                  return MATCHCLOSED; }
^"5 Setting you open for matches." { /* No \n -> forces prompt */
                                  return MATCHOPEN; }
^"9 Setting you open for matches." { /* No \n -> forces prompt */
                                  return MATCHOPEN; }
^"5 Cannot Undo\n5 ".*\n        { yylval.Name = mystrndup(YYTEXT+16,YYLENG-17);
                                  return ILLEGALUNDO; }
^"5 You cannot remove a liberty."{WNL} |
^"5 You cannot remove Liberties"{WNL} {
                                  return REMOVELIBERTY; }
^"5 Channel must be "           { BEGIN(THELINE);
                                  return WRONGCHANNEL; }
^"5 Title set."{WNL}            { break; }
^"5 Cannot find recipient."{WNL} { return NOTELLTARGET; }
^"5 ".*": Unknown command."{WNL} { yylval.Name = mystrdup(YYTEXT+2);
                                   *strchr(yylval.Name, ':') = 0;
                                  return INVALID; }
^"5 No longer reviewing".*{WNL}"5 Could not find game."{WNL} {
                                  return NOTREVIEWING; }
^"5 Game not found."{WNL}       { return GAMENOTFOUND; }
^"5 There are no more moves."{WNL} { return NOMOREMOVES; }
^"5 This teach game is not a req".*{WNL} {
 /* two IGS bugs: "request" spelled "reqeust" and impossibility
  * of reloading teaching games.
  */                               return NOTREQUESTGAME; }

^"5 User is not accepting tells."{WNL} {
                                  BEGIN(NUMLINES);
                                  return TELLOFF; }
^"5 Neither a review file".*{WNL} { return NOREVIEW; }
^"5 "{ID}" wants a "            { yylval.Person = IdToPlayer(YYTEXT+2);
                                  BEGIN(DISPUTINGMATCHTYPE);
                                  return DISPUTEMATCHTYPE; }

^"7 [##""#"*"]".*\n             { return GAMES; }
^"7 ["{WHITESPACE}*{NUM}"]"     { BEGIN(THENAMES);
                                  yylval.Value = atoi(YYTEXT+3);
                                  return GAMES; }

^("9"{WNL})?"9 !!"{ID}"!!: "    { BEGIN(THELINE);
                                  yylval.Person =
                                      LengthNameToPlayer(YYTEXT+4,
                                                         (int) YYLENG-8);
                                  return XSHOUT; }
^"9 !!"{BADID}"!!: "            { BEGIN(THELINE);
                                  yylval.Name = mystrndup(YYTEXT+4, YYLENG-8);
                                  return XSHOUT2; }
^("21"{WNL})?"21 !!"{ID}"!!: "  { BEGIN(THELINE);
                                  yylval.Person =
                                      LengthNameToPlayer(YYTEXT+5,
                                                         (int) YYLENG-9);
                                  return XSHOUT; }
^"9"{WNL}                       { return EMPTY; }
^"9"{WNL}"1 1"{WNL}             { yylval.Value = 1;
                                  return PROMPT; }
^"9 Set "                       { BEGIN(THENAMES);
                                  return TOGGLE; }
^"9 Request for adjournment sent."{WNL} { return ADJOURNSENTREQUEST; }
^"9 Your opponent requests an adjournment."{WNL} { return ADJOURNREQUEST; }
^"9 Use <adjourn> to adjourn, or <decline adjourn> to decline."{WNL} { break; }
^"9 Opponent declines to adjourn."{WNL} { return DECLINEADJOURN; }
^"9 Setting your '.' to "       { BEGIN(THENAMES);
                                  return TELLTARGET; }
^"9 Channels:"{WNL}             { BEGIN(NAMELIST);
                                  return CHANNELHEADER; }
^"9 Observing game "            { NextState = NAMELIST;
                                  BEGIN(ALLNAMES);
                                  return OBSERVERS; }
^"9 Player: "                   { yylval.Name = mystrndup(YYTEXT+2, YYLENG-4);
                                  NextState = STATS;
                                  BEGIN(ALLLINE);
                                  return STATSENTRY; }
^"9 Throwing other copy out.".* { /* No \n -> forces prompt */
                                  return THROWCOPY; }
^"9 You have a message.".*      { /* No \n -> forces prompt */
                                  yylval.Value = 1;
                                  return MESSAGES; }
^"9 You have "{NUM}" lines of messages.".* { /* No \n -> forces prompt */
                                  yylval.Value = atoi(YYTEXT+11);
                                  return MESSAGES; }
^"9 Removing game "{NUM}" from observation list."{WNL} {
                                  yylval.Value = atoi(YYTEXT+16);
                                  return REMOVE; }
^"9 Adding game to observation list." { /* No \n -> forces prompt */
                                  return ADD; }
^"9 Games currently being observed:" {
                                  BEGIN(THENAMES);
                                  return WATCHING; }
^"9 Game "{NUM}": "{ID}" vs "{ID}" has adjourned."{WNL} { break; }
^"9 You must PASS during the handicap plays."{WNL} {
                                  MyGameMessage("You must PASS during the "
                                                "handicap plays.");
                                  break; }
^"9 White must now pass".*{WNL} |
^"9 Black must now pass".*{WNL} { yylval.Name = mystrndup(YYTEXT+2, YYLENG-3);
                                  return OPPMUSTPASS; }
^"9 Game is titled: "           { BEGIN(THELINE);
                                  return GAMETITLE; }
^"9 Title set."{WNL}            { return TITLESET; }
^\n"9 White must now pass".*{WNL} |
^\n"9 Black must now pass".*{WNL} {
                                  yylval.Name = mystrndup(YYTEXT+3, YYLENG-4);
                                  return MUSTPASS; }
^\n?"9 There is a disagreement about the life/death of the stone, the game will resume."{WNL} {
                                  if (YYTEXT[0] == '\n')
                                      return OPPDISAGREEREMOVE;
                                  else return DISAGREEREMOVE; }
^{NZ}" "{ID}" wants a match with you:"{WNL}{NZ}" " {
                                  BEGIN(THENAMES);
                                  return AUTOMATCHREQUEST; }
^{NZ}" To accept match type ".*\n    { break; } /* AUTOMATCHREQUEST */
^"35 "{ID}" wants "{NUM}"x"{NUM}.*\n { break; } /* AUTOMATCHDISPUTE */
^"35 Using "{ID}" 'defs'."\n         { break; } /* AUTOMATCHDISPUTE */
^"35 "{ID}\n                         { break; } /* AUTOMATCHDISPUTE */

^("9"{WNL})?"9 Match ["{NUM}"] with "{ID}.*\n {
                                  yylval.Value = atoi(YYTEXT+9);
                                  return NEWMATCH; }
^("9"{WNL})?"9 "("TOUR""N"?"AMENT ")?"GOE "?[M|m]"atch["{NUM}"x"{NUM}"]".*\n {
                                  break; }
^"9 B W B W...:".*\n            { break; }

^"9 Use <match "                { BEGIN(THENAMES);
                                  return MATCHREQUEST; }
^"9 Use <tmatch "               { BEGIN(THENAMES);
                                  return TOURNAMENTMATCHREQUEST; }
^"9 Use <tgmatch "              { BEGIN(THENAMES);
                                  return TOURNAMENTGOEMATCHREQUEST; }
^"9 Use <goematch "             { BEGIN(THENAMES);
                                  return GOEMATCHREQUEST; }
^"9 Requesting match in ".*\n   { yylval.Name = mystrdup(YYTEXT+2);
                                  yylval.Name[yyleng-3] = 0;
                                  return REQUESTINGMATCH; }
^"9 Creating match ["{NUM}"] with "{ID}{WNL} {
                                  yylval.Value = atoi(YYTEXT+18);
                                  return NEWMATCH; }
^"9 Please use say to talk to you""r"?" opponent -- help say."{WNL} {
                                  break; }
^"9 "{ID}" declines your request for a match."{WNL}"9 Match with "{ID}.* {
                                  yylval.Person = IdToPlayer(YYTEXT+2);
                                  return DECLINE; }
^("9 "{ID}" wants the komi to be ".*{WNL})?"9 Set the komi to"  {
                                  BEGIN(THELINE);
                                  return KOMISET; }
^"9 "{ID}" wants the komi to be " {
                                  yylval.Person = IdToPlayer(YYTEXT+2);
                                  BEGIN(THELINE);
                                  return KOMIREQUEST; }
^"9 Game will count towards ratings."{WNL} {
                                  yylval.Value = 0;
                                  return FREE; }
^"9 Game will not count towards ratings."{WNL} {
                                  yylval.Value = 1;
                                  return FREE; }
^"9 Your opponent has lost their connection."{WNL} {
                                  return LOSTCONNECTION; }
^"9 Game has been adjourned."{WNL} {
                                  return MYADJOURN; }
^"9 "{ID}" has restored your old game." { /* No \n -> forces prompt */
                                  return RESTORE; }
^"9 "{ID}" ""h"?"as restarted your game."{WNL} {
                                  return RESTART; }
^"9 Game saved."                { BEGIN(THELINE);
                                  return GAMESAVED; }
^"2 Game saved."                { BEGIN(THELINE);
                                  return GAMESAVED; }

^"9 "{ID}" has resigned the game."{WNL} {
                                  yylval.Person = IdToPlayer(YYTEXT+2);
                                  return RESIGN; }
^"9 Removed game file "{ID}" from database." { /* No \n -> forces prompt */
                                  yylval.Name = mystrdup(YYTEXT+20);
                                  strchr(yylval.Name, ' ')[0] = 0;
                                  return REMOVEGAMEFILE; }
^"9 Game "{ID}" mailed"         { /* No \n -> forces prompt */
                                  yylval.Name = mystrdup(YYTEXT+7);
                                  strchr(yylval.Name, ' ')[0] = 0;
                                  return MAILED; }
^"9 ".*"ype 'done' when finished""."?{WNL} { /* no \n? */ return DONE; }
^"9 "{COLOR}" needs to remove a ""a "?"group first."{WNL} {
                                  yylval.Value =
                                      (YYTEXT[2] == 'w' || YYTEXT[2] == 'W') ?
                                          White : Black;
                                  return FIRSTREMOVE; }
^"9 Removing @ "{ID}            {  /* No \n -> forces prompt */
                                  yylval.Name = mystrdup(YYTEXT+13);
                                  return REMOVEGROUP; }
^"9 Board is restored to what it was when you started scoring" {
                                  /* No \n -> forces prompt */
                                  return RESTORESCORING; }
^"9 "{ID}" has typed done."   { /* No \n -> forces prompt */
                                  MyGameMessage(YYTEXT+2);
                                  break; }
^"9 Please type 'done again."   { /* No \n -> forces prompt */
                                  return PLEASEREDONE; }
^"9 The player "{ID}" is now in byo-yomi."{WNL} {
                                  yylval.Person = IdToPlayer(YYTEXT+13);
                                  return ENTERBYOYOMI; }
^"9 You have "{NUM}" penalty points and "{NUM}" min".* |
^"9 You have "{NUM}" stones and "{NUM}" min".* { /* No \n -> forces prompt */
                                  yylval.Name = mystrdup(YYTEXT+11);
                                  return GIVEBYOYOMI; }
^"9 "{ID}" has run out of time." { /* No \n -> forces prompt */
                                  yylval.Person = IdToPlayer(YYTEXT+2);
                                  return NOTIME; }
^"9 "{ID}" is beeping you."{WNL} {yylval.Person = IdToPlayer(YYTEXT+2);
                                  return BEEPING; }
^"9 "{ID}" has been idle for " {
                                  yylval.Person = IdToPlayer(YYTEXT+2);
                                  BEGIN(THELINE);
                                  return IDLE; }
^"9 The current time (GMT) is:" { NextState = UPTIME;
                                  BEGIN(ALLNAMES);
                                  return GMTTIME; }
^"9 Please erase your messages (help erase)."{WNL} {
                                  return ERASE; }
^"9 There are no currently loaded games".*{WNL} |
^"9 The currently loaded games:"{WNL} {
                                  BEGIN(REVIEWL);
                                  return REVIEWLIST; }
^"9 You need review".*{WNL}     |
^"9 Review with ".*{WNL}        { return REVIEWSTOP; }
^"9 Now reviewing ".*           { /* No \n -> forces prompt */
                                  yylval.Name =
                                      mystrndup(YYTEXT+16,
                                                YYTEXT[YYLENG-18] == '.' ?
                                                YYLENG-18 : YYLENG-17);
                                  return REVIEWSTART; }
^"9 Variations (help 'forward'):"{WNL} {
                                  BEGIN(NUMLINES);
                                  return REVIEWVARIATIONS; }
^"9 You are at the end of this review".*{WNL} {
                                  break; }
^"9 Cannot do it yet".*{WNL}    { return REVIEWEND; /* What does this mean ?? -- Ton */ }
^"9 Type 'review' to stop reviewing".*{WNL} {
                                  return REVIEWEND; }
^"9 The current score is:"{WNL} { return CURRENTSCORE; }
^"9 The final score will be:"{WNL} { return FINALSCORE; }
^"9 White:".*", Black ".*", Dame ".*{WNL} {
                                  yylval.Name = mystrndup(YYTEXT+2, YYLENG-3);
                                  return NAME; }
^"9 White "{NUM}" Black "{NUM}{WNL} { break;
/* given after GAMETIME for super user, nb of undos left */ }

^"9 You won ""-"?{NUM}.*{WNL}  { yylval.Value = atoi(YYTEXT+10);
                                  return BETRESULT; }
^"9 You lost ""-"?{NUM}.*{WNL} { yylval.Value = -atoi(YYTEXT+11);
                                  return BETRESULT; }
^"9 You remained even".*{WNL}  { yylval.Value = 0;
                                  return BETRESULT; }

^"9 Increase ".*" minute".*{WNL} { AutoCommand(NULL, "time");
                                  MyGameMessage(YYTEXT+2);
                                  break; }

^"9 "{ID}" is now on."{WNL}    { const char *id = YYTEXT+2;
                                 const char *ptr = strchr(id, ' ');
                                 /* do not use IdToPlayer to avoid warning if
                                  * message "has connected" not yet received.
                                  */
                                 yylval.Person = PlayerFromLengthName(id,
								      ptr-id);
                                 return PLAYERON; }

^"9 Status of people acc".*{WNL}"9"{SPACES}{ID}": yes "{ID}": yes "{ID}": yes "{ID}": yes"{WNL}  { break; }

^"9 There "{NUM}" entries found and only "{NUM}" reported.\n9 " {
  /* missing "are" is bug in IGS, not xgospel */
  /* Display only "You may wish to modify your search to get fewer entries." */
                                 BEGIN(THELINE);
                                 return TRANSLATION; }

<INITIAL,NEWCHANNELS>^"9"{WHITE}"#"{ID}(" "|"*")"Title: " { const char *ptr;
                                  BEGIN(CHANNELTITLE);
                                  ptr = strchr(YYTEXT, '#')+1;
                                  yylval.Name =
                                      mystrndup(ptr, (size_t)
                                         (YYTEXT+YYLENG-8-ptr));
                                         /* (strchr(ptr, ' ')-ptr)); */
                                  return NEWCHANNEL; }

^"11 Kibitz "                   { BEGIN(THENAMES);
                                  return KIBITZ; }
^"11  "" "{0,2}                 { BEGIN(THELINE);
                                  break; }

^"12 B W B W...:"{WHITE}        { BEGIN(TEAMNAMES);
                                  return RESTARTTEAMGAME; }

^"14 You have messages."{WNL}   { return MESSAGES; }

^"15 Game "{NUM}.*": "          { yylval.Value = atoi(YYTEXT+8);
                                  BEGIN(THENAMES);
                                  return GAME; }
^"15"{WHITE}{NUM}"("[WB]"): "   { yylval.Value = atoi(YYTEXT+3);
                                  if ((yylval.Value %2 == 0) ^
                                      (YYTEXT[7] == 'B'))
                                      Warning("Contact programmer. "
                                              "This should not happen !!!!\n");
                                  BEGIN(THELINE);
                                  return MOVE; }

"15 B W B W...:"{WHITE}         { BEGIN(TEAMNAMES);
                                  return OBSERVETEAM; }

"16 B W B W...:"{WHITE}         { BEGIN(TEAMNAMES);
                                  return OBSERVETEAM; }

"17 B W B W...:"{WHITE}         { BEGIN(TEAMNAMES);
                                  return OBSERVETEAM; }

^"19 *"{ID}"*: "                { BEGIN(THELINE);
/*
51 Say in game 10

19 *jloup*: test 4
*/
                                  yylval.Person =
                                      LengthNameToPlayer(YYTEXT+4,
                                                         (int) YYLENG-7);
                                  return TELL; }

^"20 ".*                        { /* No \n -> forces prompt */
                                  yylval.Name = mystrdup(YYTEXT+3);
                                  return RESULTLINE; }

^"21 {"                         { BEGIN(THENAMES);
                                  return INFOMESSAGE; }
^"21 !"{ID}"!: "                { BEGIN(THELINE);
                                  yylval.Person =
                                      LengthNameToPlayer(YYTEXT+4,
                                                         (int) YYLENG-7);
                                  return BROADCAST; }
^"21 --> "{ID}" "		{ BEGIN(THELINE);
                                  yylval.Person =
                                      LengthNameToPlayer(YYTEXT+7,
                                                         (int) YYLENG-8);
                                  return ITBROADCAST; }

^"22 "                          { BEGIN(THENAMES);
                                  return STATUSHEADER; }
^"22 "{TWONUM}": "              { yylval.Value = atoi(YYTEXT+3);
                                  NextState = STATUS;
                                  BEGIN(ALLLINE);
                                  return STATUSLINE; }

^"24 *"{ID}"*: "(" ")?          { BEGIN(THELINE);
/*
2 

24 *jloup*:  test
*/
                                  yylval.Person =
                                      LengthNameToPlayer
                                          (YYTEXT+4,
                                           strchr(YYTEXT+4, '*')-YYTEXT-4);
                                  return TELL; }

^"26 "                          { BEGIN(THENAMES);
                                  return GAMETIME; }

^"27  Info".*\n                 |
^"27   FL ".*\n                 { return PLAYERS; }
^"27"{WHITE}"*"+                { BEGIN(THENAMES);
                                  return END; }
^"27  "                         { BEGIN(PLAYERENTRY);
                                  return PLAYERS; }
^"27"{WNL}                      { break; }

^"28"{WNL}                      { break; }

^("2 "{BELL}\n)?"28 Undo in game "{NUM} {
/* for multiple undo in teach game:
1 8

28 Undo in game 23: jl vs jl:  B8 
1 8

28 Undo in game 23: jl vs jl:  B2 
1 8

28 Undo in game 23: jl vs jl:  B9 
1 8

15 Game 23 I: jl (0 55 -1) vs jl (0 62 -1)
15   2(B): B1
1 8

with bell on:
1 8
2 
28 Undo in game 51: jl vs jl:  C9 
1 8
2 
28 Undo in game 51: jl vs jl:  B9 
1 8
2 
28 Undo in game 51: jl vs jl:  A9 
1 8
2 
15 Game 51 I: jl (0 0 -1) vs jl (0 52 -1)
15   0(B): A2
1 8
*/
				  char *ptr = strrchr(YYTEXT, ' ');
                                  BEGIN(UNDOWHITE);
	                          yylval.Value = atoi(ptr+1);
                                  return UNDO; }
^"28 "{ID}" undid the last move (" {
                                  BEGIN(UNDIDLINES);
                                  return UNDID; }
^"28 B W B W...:"{WHITE}         { BEGIN(TEAMNAMES);
                                  return OBSERVETEAM; }


^"30 "                          { BEGIN(THELINE);
                                  return TRANSLATION; }

^"32"" "+"-"?{NUM}[ :]          { yylval.Value = atoi(YYTEXT+3);
                                  BEGIN(CHANNELS);
                                  return CHANNEL; }
^"32 Changing into channel"" "+"-"?{NUM}.*\n {
                                  yylval.Value = atoi(YYTEXT+25);
                                  return CHANGECHANNEL; }
^"32 Welcome to cyberspace."{WNL} { break; }

^"34 File "{NUM}{WNL}           { yylval.Value = atoi(YYTEXT+8);
                                  BEGIN(REVIEW);
                                  return REVIEWTYPE; }

<INITIAL,ENTEREDPASSWORD>^"39 "{ID}" "[^ ]+" " {
    				  const char *ptr;

                                  ptr = YYTEXT+3;
                                  while (!isspace(*ptr)) ptr++;
                                  yylval.Name =
                                      mystrndup(YYTEXT+3,
                                                (size_t) (ptr-YYTEXT-3));
                                  BEGIN(THELINE);
                                  return IGSENTRY; }

^"40 "{ID}{WNL}                 { /* Here because otherwise File matches */
                                  /*
                                  const char *ptr;

                                  ptr = YYTEXT+3;
                                  while (!isspace(*ptr)) ptr++;
                                  yylval.Person =
                                      LengthNameToPlayer(YYTEXT+3,
                                                         ptr-YYTEXT-3);
                                  return TELLDONE;
                                  */
                                  break; }

^"41 sgf"{WNL}                  { BEGIN(NUMNAMES);
                                  return SGFLIST; }
^("41 bet"{WNL})?"9 Winners"":"?{WNL} { BEGIN(BET);
                                  return BETWINNERS; }
^"41 literal".*{WNL}            |
^"41 literal".*"41"{WNL}{WNL}?  { BEGIN(NONCLIENT);
                                  return UNKNOWNANSWER; }
^"41 setproba"{WNL}             { BEGIN(PROB); /* with no-* player only */
                                  return SETPROBA; }
^"41 proba"{ID}{NL}             { const char *id = YYTEXT+8;
                                  const char *ptr = strchr(id, '\n');
                                 /* do not use IdToPlayer to avoid warning if
                                  * player is not connected.
                                  */
                                  while (ptr[-1] == '\r') ptr--;
                                  yylval.Person = PlayerFromLengthName(id,
								       ptr-id);
                                  BEGIN(PROB);
                                  return PROBA; }

^"41 stored"{ID}{NL}            { const char *id = YYTEXT+9;
                                  const char *ptr = strchr(id, '\n');
                                 /* do not use IdToPlayer to avoid warning if
                                  * player is not connected.
                                  */
                                  while (ptr[-1] == '\r') ptr--;
                                  yylval.Person = PlayerFromLengthName(id,
								       ptr-id);
                                  BEGIN(STOR);
                                  return STORED; }

^"41 ".*{WNL}                   |
^"41 ".*{WNL}{WNL}?             { /* Ignore for the moment */ break; }

^"42 ".*{WNL}                   { BEGIN(USERENTRY);
                                  return USER; }

<NUMNAMES>^"43 There "("is"|"are")" "{NUM}" sgf game".*{WNL} {
                                  BEGIN(0);
                                  return END; }

^"45 You are now trailing "{ID}{WNL} { break; } /* game will popup anyway */

^"47 "{ID}" has captured ".*{WNL}    { break; } /* we see this anyway */

^"48 Game".*"requests an adjournment"{WNL} { break;
/* ??? support multiple play:
1 6
9 Your opponent requests an adjournment.
48 Game 36 jloup requests an adjournment
9 Use <adjourn> to adjourn, or <decline adjourn> to decline.
1 6
*/
                                }
^"48 Game ".*"has been adjourned by".*{WNL} { break;
/* ??? support multiple play:
    1 6
    9 Your opponent has lost his/her connection.
    48 Game 9 has been adjourned by xgospel
    9 Game has been adjourned.
    9 Game saved.
    1 5
*/
                                }

^"49 Game ".*"is removing".*{WNL}  { break; /* ??? support multiple play */ }

^"50 Game #"{NUM}{WNL}          { break; /* ??? support multiple play */ }

^"51 Say in game ".*{WNL}       { break; /* ??? support multiple play */
/*
51 Say in game 10
*/
                                }

^{NUM}" "+"File"{WNL}           { BEGIN(FILETEXT);
                                  yylval.Value = atoi(YYTEXT);
                                  return TEXTFILE; }


^"9 "                           { BEGIN(INFOLINE); /* avoid parse error */
                                  break; }
<INFOLINE>.*{WNL}               { BEGIN(0);
                                  Outputf("%s", YYTEXT);
                                  break; }

^"5"[2-9]" ".*{WNL}             { /* avoid parse error */
                                  Outputf("%s", YYTEXT+3);
                                  break; }


\n                              { BEGIN(0);
                                  return SEMIPROMPT; }
;                               { break; /* -- Doubt, Doubt --Ton */ }
.                               { yyless(0);
                                  BEGIN(NONCLIENT);
                                  return UNKNOWNANSWER; }

<FILETEXT>^{NUM}" File"{WHITESPACE}* { /* No \n -> forces prompt */
                                  BEGIN(0);
                                  return END; }
<FILETEXT>{CHAR}                { NextState = FILETEXT;
                                  yyless(0);
                                  BEGIN(ALLLINE);
                                  break; }

<STATUS>^"22 "{TWONUM}": "      { yylval.Value = atoi(YYTEXT+3);
                                  NextState = STATUS;
                                  BEGIN(ALLLINE);
                                  return STATUSLINE; }
<STATUS>\n                      |
<STATUS>{PROMPT}                { yyless(YYLENG-1);
                                  BEGIN(0);
                                  break; }

<BET>{WNL}                      { break; }
<BET>^"9 Your bet is ".*{WNL}	{ BEGIN(0);
	                          yylval.Name = mystrndup(YYTEXT+2, YYLENG-3);
                                  return MYBET; }
<BET>"-"?{NUM}{WHITE}?          { yylval.Value = atoi(YYTEXT);
                                  return NATURAL; }
<BET>":"{WHITE}?                { return YYTEXT[0]; }
<BET>("9"{WHITE})?{ID}{WHITE}   { const char *ptr;

                                  ptr = YYTEXT;
                                  if (ptr[0] == '9') {
                                      ptr++;
                                      while (isspace(*ptr)) ptr++;
                                  }
                                  yylval.Person = IdToPlayer(ptr);
                                  return PERSON; }
<BET>^"9 Even"":"?{WNL}         { return BETEVEN;   }
<BET>^"9 Losers"":"?{WNL}       { return BETLOSERS; }

<DISPUTING>"."                  |
<DISPUTING>"("                  |
<DISPUTING>")"                  |
<DISPUTING>{WHITE}              |
<DISPUTING>^"5 "                { break; }
<DISPUTING>{WNL}                { return END; }
<DISPUTING>{ID}" wants "        { yylval.Person = IdToPlayer(YYTEXT);
                                  return PERSON; }
<DISPUTING>{COLOR}" on "        { yylval.Value =
                                      (YYTEXT[0] == 'w' || YYTEXT[0] == 'W') ?
                                          White : Black;
                                  return GAMECOLOR; }
<DISPUTING>"a "{NUM}"x"{NUM}" " { yylval.Name = mystrndup(YYTEXT+2, YYLENG-3);
                                  return NAME; }
<DISPUTING>"in "{NUM}" mins"    { yylval.Value = atoi(YYTEXT+3) * 60;
                                  return GAMESECONDS; }
<DISPUTING>{NUM}" byoyomi"      { yylval.Value = atoi(YYTEXT);
                                  return BYOYOMI; }

<DISPUTINGMATCHTYPE>"match"     |
<DISPUTINGMATCHTYPE>"."         |
<DISPUTINGMATCHTYPE>{WHITE}     { break; }
<DISPUTINGMATCHTYPE>"TOURNAMENT" { /* More types needed here --Ton */
                                  yylval.Value = TOURNAMENTTYPE;
                                  return MATCHTYPE; }
<DISPUTINGMATCHTYPE>"IGS"       { yylval.Value = IGSTYPE;
                                  return MATCHTYPE; }
<DISPUTINGMATCHTYPE>{WNL}       { BEGIN(0);
                                  return END; }

<REVIEWL>^"9 "[^ \r\n].*{WNL}   |
<REVIEWL>{WNL}                  |
<REVIEWL>^"9  "" "*"|"" "*      { break; }
<REVIEWL>[^| \r\n][^|\r\n]*" "+"|"{WHITE}? {
                                  const char *ptr;

                                  ptr = YYTEXT+YYLENG-1;
                                  while (*ptr == ' ') ptr--;
                                  ptr--;
                                  while (*ptr == ' ') ptr--;
                                  yylval.Name =
                                      mystrndup(YYTEXT,(size_t)(ptr-YYTEXT+1));
                                  return NAME; }
<REVIEWL>{CHAR}                 { yyless(0);
                                  unput('\n');
                                  BEGIN(0);
                                  break; }

<REVIEW>^{NUM}" "+"File "{NUM}{WNL} {
                                  BEGIN(0);
                                  return END; }
<REVIEW>;                       { return REVNODE; }
<REVIEW>No?d?e?n?a?m?e?         { return REVNODENAME; }
<REVIEW>Bl?a?c?k?               { return REVBLACK; }
<REVIEW>Wh?i?t?e?               { return REVWHITE; }
<REVIEW>Ad?d?Bl?a?c?k?          { return REVADDBLACK; }
<REVIEW>Ad?d?Wh?i?t?e?          { return REVADDWHITE; }
<REVIEW>Ad?d?Em?p?t?y?          { return REVADDEMPTY; }
<REVIEW>Bl?a?c?k?t?i?m?e?Le?f?t? { return REVBLACKTIME; }
<REVIEW>Wh?i?t?e?t?i?m?e?Le?f?t? { return REVWHITETIME; }
<REVIEW>Co?Py?r?i?g?h?t?        { return REVCOPYRIGHT; }
<REVIEW>Co?m?m?e?n?t?           { return REVCOMMENT; }
<REVIEW>Ko?Mi?                  { return REVKOMI; }
<REVIEW>HAn?d?i?c?a?p?          { return REVHANDICAP; }
<REVIEW>REs?u?l?t?              { return REVRESULT; }
<REVIEW>Pl?a?Ce?                { return REVPLACE; }
<REVIEW>Da?Te?                  { return REVDATE; }
<REVIEW>USe?r?                  { return REVUSER; }
<REVIEW>EVe?n?t?                { return REVEVENT; }
<REVIEW>Ga?m?e?Na?m?e?          { return REVGAMENAME; }
<REVIEW>Wh?i?t?e?Ra?n?k?        { return REVWHITERANK; }
<REVIEW>Bl?a?c?k?Ra?n?k?        { return REVBLACKRANK; }
<REVIEW>Pl?a?y?e?r?Wh?i?t?e?    { return REVWHITENAME; }
<REVIEW>Pl?a?y?e?r?Bl?a?c?k?    { return REVBLACKNAME; }
<REVIEW>Le?t?t?e?r?s?           { return REVLETTERS; }
<REVIEW>Si?Ze?                  { return REVSIZE; }
<REVIEW>Ga?Me?                  { return REVGAME; }
<REVIEW>{WORD}                  { yylval.Name = mystrndup(YYTEXT, YYLENG);
                                  return REVUNKNOWN; }
<REVIEW>"["\n?\n?               { /* Eat buggy newline --Ton */
                                  BEGIN(REVIEWLITERAL);
                                  RevBracks = 0;
                                  if (TextBuf) myfree(TextBuf);
                                  TextBuf    = mymalloc(BUFSTEP);
                                  TextLength = BUFSTEP;
                                  TextFill   = 0;
                                  break; }
<REVIEW>.                       { return YYTEXT[0]; }
<REVIEW>\n                      { break; }

<REVIEWLITERAL>" "{ID}" "({NUM}[pdk][ *]|"??? "|"NR ")": ".*\n\n {
                                  char *ptr, *from;
                                  size_t Len;

                                  ptr = YYTEXT+1;
                                  while (*ptr != ' ') ptr++;
                                  Len = 10-(ptr-YYTEXT-1);
                                  if (Len > 0) TextExtend("          ", Len);
                                  TextExtend(YYTEXT+1, (size_t)(ptr-YYTEXT-1));
                                  TextExtend("[", 1);
                                  ptr++;
                                  from = ptr;
                                  ptr += 2;
                                  while (*ptr != ':') ptr++;
                                  ptr--;
                                  Len = ptr-from;
                                  if (Len < 3) TextExtend("   ", 3-Len);
                                  TextExtend(from, Len);
                                  TextExtend("]", 1);
                                  TextExtend(ptr, (size_t)
                                             (YYTEXT+YYLENG-2-ptr));
                                  yyless(YYLENG-2);
                                  break; }
<REVIEWLITERAL>\n\n             { /* Eat buggy newline --Ton */
                                  TextExtend(YYTEXT, YYLENG-1);
                                  break; }
<REVIEWLITERAL>\n               |
<REVIEWLITERAL>[^\]\[\n]*([:;]-[\[\]])? {
                                  TextExtend(YYTEXT, YYLENG);
                                  break; }
<REVIEWLITERAL>"["              { RevBracks++;
                                  TextExtend(YYTEXT, YYLENG);
                                  break; }
<REVIEWLITERAL>\n?\n?\n?"[*** END OF GAME ***]" |
<REVIEWLITERAL>\n?\n?\n?"[*** 'B'"[^\n\]]*".]"\n? {
                                  /* Eat mgt assumption --Ton */
                                  TextExtend("\n(see variation)", 16);
                                  break; }
<REVIEWLITERAL>\n?\n?"]"        { /* Eat buggy newline --Ton */
                                  if (--RevBracks < 0) {
                                      TextBuf[TextFill] = 0;
                                      yylval.Name = TextBuf;
                                      TextBuf = NULL;
                                      BEGIN(REVIEW);
                                      return REVLITERAL;
                                  }
                                  TextExtend(YYTEXT, YYLENG);
                                  break; }
<REVIEWLITERAL>^{NUM}" "+"File "{NUM}{WNL}{MODE}?{PROMPT} { /* Panic exit */
                                  const char *ptr;

                                  myfree(TextBuf);
                                  TextBuf = 0;
                                  ptr = YYTEXT+YYLENG-1;
                                  while (*ptr != ' ') ptr--;
                                  yylval.Value = atoi(ptr+1);
                                  BEGIN(0); /* Recover from wrong state */
                                  return PROMPT; }

<NAMELIST>"Found "{NUM}" ".*"."  { break; }
<NAMELIST>{ID}                  { yylval.Name = mystrndup(YYTEXT, YYLENG);
                                 return NAME; }
<NAMELIST>{WNL}                 { return END; }
<NAMELIST>^"9 "                 { break; }
<NAMELIST>{WHITE}               { break; }

<PLAYERENTRY>[ SQ][ X!]{GAMENUM}"  "{GAMENUM}" " {
                                  yylval.Name = mystrndup(YYTEXT, YYLENG-1);
                                  return NAME; }
<PLAYERENTRY>{ID}               { yylval.Name = mystrndup(YYTEXT, YYLENG);
                                  return NAME; }
<PLAYERENTRY>{WHITESPACE}       { break; }
<PLAYERENTRY>" |"               { unsend("\n27 ");
                                  break; }
<PLAYERENTRY>\n                 { BEGIN(0);
                                  return END; }
<PLAYERENTRY>{CHAR}             { yyless(0);
                                  BEGIN(THELINE);
                                  break; }

<USERENTRY>^"42"" "+{ID}"  "    { const char *ptr;
                                  BEGIN(USERENTRYINFO);
                                  ptr = YYTEXT+3;
                                  while (*ptr == ' ') ptr++;
                                  yylval.Name =
                                      mystrndup(ptr, (size_t)
                                                (YYTEXT+YYLENG-2-ptr));
                                  return NAME; }
<USERENTRY>^"9 ".*{WNL}         { BEGIN(0);
                                  return END; }     
<USERENTRY>{CHAR}               { yyless(0);
                                  return FAIL; }
<USERENTRYINFO>.{14}            { int len;

                                  for (len = 13; len>=0; len--)
                                      if (YYTEXT[len] != ' ') break;
                                  yylval.Name = mystrndup(YYTEXT,
                                                          (size_t) len+1);
                                  BEGIN(USERENTRYREST);
                                  return NAME; }
<USERENTRYINFO>{CHAR}           { yyless(0);
                                  BEGIN(THELINE);
                                  return FAIL; }
<USERENTRYREST>{WHITE}+         { break; }
<USERENTRYREST>{NUM}"/"?        { yylval.Name = mystrndup(YYTEXT, YYTEXT[YYLENG-1] == '/' ? YYLENG-1 : YYLENG);
                                  return NAME; }
<USERENTRYREST>"-"              { yylval.Name = mystrndup("-1", 2);;
                                  return NAME; }
<USERENTRYREST>{IDNOSLASH}      { yylval.Name = mystrndup(YYTEXT, YYLENG);
                                  return NAME; }
<USERENTRYREST>{WNL}            { BEGIN(USERENTRY);
                                  return END; }
<USERENTRYREST>{CHAR}           { yyless(0);
                                  BEGIN(THELINE);
                                  return FAIL; }

<STOR>^"18 Found "{NUM}" stored games."{WNL} {
				  BEGIN(0);
                                  yylval.Value = atoi(YYTEXT+9);
				  return STOREDNUM;
                                 }
<STOR>^"18 ".*{WNL}              { break; }
<STOR>{CHAR}                     { yyless(0);
                                   BEGIN(THELINE);
                                   break; }

<PROB>^"5 ".*"not have a rating."{WNL} {
				  BEGIN(0);
				  return END;
                                 }
<PROB>^"5 Player not found."{WNL} {
				  BEGIN(0);
				  return END;
                                 }
<PROB>^"9 The hand".*"\n9 A komi".*"\n9 Is the".*"\n9 Your rating:"{WHITE}*{ID}{WHITE}* {
	                          char *ptr = strrchr(YYTEXT, ':');
                                  yylval.Value = atoi(ptr+1);
                                  return RATING; }
<PROB>"Their rating:"{WHITE}*{ID}{WHITE}* {
                                  yylval.Value = atoi(YYTEXT+13);
                                  return RATING; }
<PROB>"The handicap:"{WHITE}*{ID}{WNL} {
				  yylval.Name = mystrdup(YYTEXT+13);
                                   *strchr(yylval.Name, '\n') = 0;
                                  return NAME; }
<PROB>^"9 Playing as WHITE win/lose:".*"/"{ID}{WNL} {
	                          char *ptr = strrchr(YYTEXT, '/');
				  yylval.Name = mystrdup(ptr+1);
                                   *strchr(yylval.Name, '\n') = 0;
                                  return NAME; }
<PROB>^"9 Playing as BLACK win/lose:".*"/"{ID}{WNL} {
	                          char *ptr = strrchr(YYTEXT, '/');
				  yylval.Name = mystrdup(ptr+1);
                                   *strchr(yylval.Name, '\n') = 0;
				   BEGIN(0);
                                  return NAME; }
<PROB>{CHAR}                    { yyless(0);
                                  BEGIN(THELINE);
                                  break; }

<STATS>^"9 Defaults (help defs): " {
                                  yylval.Name = mystrdup("Defaults");
                                  NextState = STATS;
                                  BEGIN(ALLLINE);
                                  return STATSENTRY; }
<STATS>^"9 "{ID}[^:\n]*": "     { yylval.Name = mystrdup(YYTEXT+2);
                                  yylval.Name[yyleng-4] = 0;
                                  NextState = STATS;
                                  BEGIN(ALLLINE);
                                  return STATSENTRY; }
<STATS>^"9 "                    { NextState = STATS;
                                  BEGIN(ALLNAMES);
                                  return EXTSTATSENTRY; }
<STATS>{CHAR}                   { yyless(0);
                                  BEGIN(THELINE);
                                  break; }

<UPTIME>^"9 The current IGS local time is:" {
                                  BEGIN(ALLNAMES);
                                  return LOCALTIME; }
<UPTIME>^"9 The server has been up" {
                                  BEGIN(ALLNAMES);
                                  return SERVERUP; }
<UPTIME>^"9 "                   { BEGIN(ALLLINE);
                                  return UPTIMEENTRY; }
<UPTIME>{CHAR}                  { yyless(0);
                                  BEGIN(THELINES);
                                  return UNKNOWNANSWER; }

<CHANNELS>{ID}": "" "?          { yylval.Person =
                                      LengthNameToPlayer
                                          (YYTEXT,
                                           (strchr(YYTEXT, ':')-YYTEXT));
                                  BEGIN(THELINE);
                                  return YELL; }
<CHANNELS>{ID}": Person joining channel"{NL} {
                                  yylval.Person =
                                      LengthNameToPlayer
                                          (YYTEXT, strchr(YYTEXT, ':')-YYTEXT);
                                  BEGIN(0);
                                  return JOIN; }
<CHANNELS>{ID}" is jo".*{NL}    { yylval.Person = IdToPlayer(YYTEXT);
                                  BEGIN(0);
                                  return JOIN; }
<CHANNELS>{ID}": Person leaving channel"{NL} {
                                  yylval.Person =
                                      LengthNameToPlayer
                                          (YYTEXT, strchr(YYTEXT, ':')-YYTEXT);
                                  BEGIN(0);
                                  return LEAVE; }
<CHANNELS>{ID}" is le".*{NL}    { yylval.Person = IdToPlayer(YYTEXT);
                                  BEGIN(0);
                                  return LEAVE; }
<CHANNELS>{ID}": Title is now: ("{ID}") " {
                                  yylval.Person =
                                      LengthNameToPlayer
                                          (YYTEXT, strchr(YYTEXT, ':')-YYTEXT);
                                  BEGIN(THELINE);
                                  return NEWTITLE; }
<CHANNELS>{CHAR}                { yyless(0);
                                  BEGIN(THELINES);
                                  return UNKNOWNANSWER; }

<CHANNELTITLE>"("{ID}")"        { yylval.Name = mystrndup(YYTEXT+1, YYLENG-2);
                                  return NAME; }
<CHANNELTITLE>" ".*" -- "       { yylval.Name = mystrndup(YYTEXT+1, YYLENG-5);
                                  NextState = NEWCHANNELS;
                                  BEGIN(ALLNAMES);
                                  return NAME; }
<CHANNELTITLE>.                 { yyless(0);
                                  unput(' ');
                                  yylval.Name = NULL;
                                  return NAME; }

<NEWCHANNELS>^"9"               { NextState = NEWCHANNELS;
                                  BEGIN(ALLNAMES);
                                  break; }
<NEWCHANNELS>^{WNL}             { break; }

<THENAMES>{ID}                  { yylval.Name = mystrdup(YYTEXT);
                                  return NAME; }
<THENAMES>{WHITE}               { break; }
<THENAMES>\n                    { BEGIN(0);
                                  return END; }
<THENAMES>.                     { return YYTEXT[0]; }

<TEAMNAMES>{UID}                { yylval.Name = mystrdup(YYTEXT);
                                  return NAME; }
<TEAMNAMES>{WHITESTARCOMMA}     { break; }
<TEAMNAMES>\n                   { BEGIN(0);
                                  return END; }
<TEAMNAMES>.                    { return YYTEXT[0]; }

<THELINE>.*\n                   { yylval.Name = mystrndup(YYTEXT, YYLENG-1);
                                  BEGIN(0);
                                  return NAME; }

<UNDIDLINES>.*")".*\n\n?        { char *ptr = strchr(YYTEXT, ')');
                                  yylval.Name = mystrndup(YYTEXT, ptr-YYTEXT);
                                  BEGIN(0);
                                  return NAME; }

<UNDIDLINES>.*")".*\n"2 "{BELL}\n {char *ptr = strchr(YYTEXT, ')');
                                  yylval.Name = mystrndup(YYTEXT, ptr-YYTEXT);
                                  BEGIN(0);
                                  return NAME; }

<UNDOWHITE>": "{ID}              { yylval.Name = mystrdup(YYTEXT+2);
                                  BEGIN(UNDOBLACK);
                                  return NAME; }

<UNDOBLACK>" vs "{ID}            { yylval.Name = mystrdup(YYTEXT+4);
                                  BEGIN(UNDOMOVE);
                                  return NAME; }

<UNDOMOVE>":"{WHITE}             { break; }
<UNDOMOVE>{ID}                   { yylval.Name = mystrdup(YYTEXT);
		                   return NAME; }
<UNDOMOVE>{WNL}"1 8"\n\n?        { BEGIN(0);
	                           break; }

<THELINES>.*\n                  { yylval.Name = mystrndup(YYTEXT, YYLENG-1);
                                  return NAME; }

<NONCLIENT>"#> "                { yylval.Value = -1;
                                  BEGIN(0);
                                  return PROMPT; }
<NONCLIENT>{CHAR}               { yyless(0);
                                  NextState = NONCLIENT;
                                  BEGIN(ALLLINE);
                                  break; }

<NUMLINES>^{NUM}" ".*\n         { char *ptr;
                                  ptr = strchr(YYTEXT, ' ')+1;
                                  yylval.Name =
                                      mystrndup(ptr, YYLENG-1-(ptr-YYTEXT));
                                  return NAME; }
<NUMLINES>\n                    { yyless(0);
                                  BEGIN(0);
                                  break; }

<NUMNAMES>\n                    |
<NUMNAMES>{WHITE}               |
<NUMNAMES>^{NUM}                { break; }
<NUMNAMES>{ID}                  { yylval.Name = mystrdup(YYTEXT);
                                  return NAME; }
<NUMNAMES>.                     { yyless(0);
                                  BEGIN(0);
                                  break; }
<NUMNAMES>^"9 There "("is"|"are")" "{NUM}" sgf game".*{WNL} {
                                  BEGIN(0);
                                  return END; }
<NUMNAMES>^"5 sgf: Needs arguments."{WNL} {
                                  BEGIN(0);
                                  return NOSGF; }

<ALLLINE>.*{WNL}                { yylval.Name = mystrndup(YYTEXT, YYLENG-1);
                                  BEGIN(NextState);
                                  return NAME; }

<ALLNAMES>{ID}                  { yylval.Name = mystrdup(YYTEXT);
                                  return NAME; }
<ALLNAMES>{WHITE}               { break; }
<ALLNAMES>\n                    { BEGIN(NextState);
                                  return END; }
<ALLNAMES>.                     { return YYTEXT[0]; }

<BET,DISPUTING,STATUS,DISPUTINGMATCHTYPE,NAMELIST,NUMLINES>. {
                                  yyless(0);
                                  unput('\n');
                                  BEGIN(0);
                                  break; }
%%

static Player *IdToPlayer(const char *From)
{
    const char *ptr;

    ptr = strchr(From, ' ');
    return LengthNameToPlayer(From, ptr-From);
}

static void unsend(const char *Text)
{
    const char *ptr;
    char        ch;

    ptr = Text+strlen(Text);
    while (ptr > Text) {
        ch = *--ptr;
        unput(ch);
    }
}

extern const char *_GoText(void);
const char *_GoText(void)
{
    return (const char *) YYTEXT;
}

extern void _IgsRestartParse(void);
void _IgsRestartParse(void)
{
    yyrestart(stdin);
    BEGIN(LOGIN);
}

extern void _IgsDefaultParse(void);
void _IgsDefaultParse(void)
{
    SetCommand(NULL, 1);
    unput('\n');        /* Force semi prompt as separator       */
    BEGIN(0);
}

extern const char *_FormatError(void);
const char *_FormatError(void)
{
    static char Error[42];

    BEGIN(THELINE);
    yylex();
    if (YYLENG < sizeof(Error)) strcpy(Error, yylval.Name);
    else {
        memcpy(Error, yylval.Name, sizeof(Error)-1);
        Error[sizeof(Error)-1] = 0;
    }
    myfree(yylval.Name);
    yyless(0);
    return Error;
}

static void TextExtend(const char *Text, size_t Length)
{
    int Temp;

    Temp = TextFill + Length;
    if (Temp >= TextLength) {
        do {
            TextLength += BUFSTEP;
        } while (Temp >= TextLength);
        TextBuf = myrealloc(TextBuf, TextLength);
    }
    memcpy(TextBuf+TextFill, Text, Length);
    TextFill = Temp;
}

/****************************************** flex fun **********************/
/*
void yy_flex_free(void *Data)
{
    myfree(Data);
}

void *yy_flex_alloc(unsigned int size)
{
    return mymalloc(size);
}

void *yy_flex_realloc(void *ptr, unsigned int size)
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
    loopy();
}
