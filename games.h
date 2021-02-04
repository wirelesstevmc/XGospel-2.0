#ifndef GAMES_H
# define GAMES_H

# include <X11/Intrinsic.h>
# include "gointer.h"
# include "gospel.h"
# include "connect.h"
typedef struct _Game Game;
# include "players.h"
# include "observe.h"

struct _BetDesc {
    BetDesc *Next;
    Player  *Who;
    long     Wins, Bets;
};

# ifdef	__GNUC__
/* Figure out how to declare functions that (1) depend only on their
   parameters and have no side effects, or (2) don't return.  */
#  if __GNUC__ < 2 || (__GNUC__ == 2 && __GNUC_MINOR__ < 5) /* Old GCC way. */
#   define       __PRINTF1
#   define       __PRINTF2
#   define       __PRINTF3
#  else                                                     /* New GCC way. */
#   define       __PRINTF1     __attribute__ ((format (printf, 1, 2)))
#   define       __PRINTF2     __attribute__ ((format (printf, 2, 3)))
#   define       __PRINTF3     __attribute__ ((format (printf, 3, 4)))
#  endif
# else	/* Not GCC.  */
#  define       __PRINTF1
#  define       __PRINTF2
#  define       __PRINTF3
# endif	/* GCC.  */

#define GAME_OBSERVE  "observe"
#define GAME_STATUS  "status"
#define GAME_OBSERVERS  "observers"
#define GAME_PLAYERS  "players"
#define GAME_PLAYERS_MAIN  "players_main"
#define DUMPGAME  "dumpgame"

# define GameToObservers(game) (*Gametoobservers((Game *)(game)))

extern Widget     GamesButton, gameinfo;
extern CommentFun LastMoveFun, MoveFun;
extern int        nrgames;
extern int        minImportantRating;

# ifndef   HAVE_NO_STDARG_H
extern int       MyGameMessage(const char *Format, ...)       __PRINTF1;
extern XtPointer AddGameComment(const Game *game,
                                const char *Format, ...)      __PRINTF2;
extern XtPointer AddMyGameComment(const char *Format, ...)    __PRINTF1;
# else  /* HAVE_NO_STDARG_H */
extern int       MyGameMessage();
extern XtPointer AddGameComment();
extern XtPointer AddMyGameComment();
# endif /* HAVE_NO_STDARG_H */

extern       void      InitGames(Widget Toplevel);
extern       void      InitHash(void);
extern       void      CleanGames(void);
extern       void      DumpGames(const char *args);
extern       void      ShowGamelogInfo(Gamelog *log);
extern       void      GamesResort(void);
extern       Game     *AddMove(int Add, GameDesc *Desc, NameVal *moves);
extern       Game     *Moves(GameDesc *Desc, NameVal *moves);
extern       Game     *Undo(int Add, int Id,
                            const char *black, const char *white,
                            const char *MoveDesc);
extern       Game     *MyGameUndo(const char *MoveDesc);
extern       void      GameTime(int Id,
                                const char *black, int BTime, int Bbyo,
                                const char *white, int WTime, int Wbyo);
extern       void      GameInfo(int Id, const char *black, const char *white,
                                NameList *Type);
extern       void      Watching(NameList *games);
extern       void      StartObserve(Game *game);
extern       void      StopObserve(Game *game);
extern       int       MyFirstObserved(void);
extern       void      UnObserve(int Id);
extern       void      UnobserveGames(Connection conn);
extern       void      ObserveWhilePlaying(void);
extern       void      AssertGamesDeleted(void);
extern       void      TestGamesDeleted(int gamesSeen);
extern       Game     *FindGame(int Id, Player *BLack, Player *WHite, int Move,
                                size_t XSize, size_t YSize, int Handicap,
                                const char *Komi, int ByoPeriod, int Mode,
                                int rules, int NrObservers);
extern       void      TeamGame(int Id, const char *black, const char *white,
				const char *black2, const char *white2,
				int move);

extern       Game     *IdPlayersToGame(int Id,
                                       const char *black, const char *white);
extern       Game     *ServerIdToGame(int Id);
extern       int       NewMatch(int Id,
                                const Player *Player1, const Player *Player2);
extern       Observe **Gametoobservers(Game *game);
extern       void      RestoreFromScoring(void);
extern       Game     *Resume(int Id, const char *black, const char *white,
                              int move);
extern       Game     *ResumeTeam(int Id, const char *black, const char *white,
                             const char *black2, const char *white2, int move);
extern       void      Adjourn(int Id, const char *black, const char *white);
extern       void      Done(void);
extern       void      Mailed(char *Names);
extern       void      RemoveGameFile(char *Names);
extern       void      RemoveGroup(const char *Stone);
extern       void      RestoreScoring(void);
extern       int       GamePosition(NameList *black, NameList *white,
                                    NumVal *Lines);
extern       void      CheckMyKomi(const char *Komi);

/* The returned pointers exist until called again */
extern const char     *GameDescription(const Game *game);
extern const char     *GameLongDescription(const Game *game);
extern       char     *GameTemplateDescription(const Game *game,
                                               const char *Template);
extern       void      SetGameHeaders(Widget w, const Game *game);
extern       void      SetGameTitle(Game *game, const char *Title);
extern       void      SetMyGameTitle(const char *title);
extern       void      StopMyGameForward(const char *message);
extern       void      ChangeGameDescription(Game *game);
extern const char     *GameTitle(const Game *game);
extern const char     *GameCaptures(const Game *game);
extern       int       GameMove(const Game *game);
extern       time_t    GameMoveTime(const Game *game);

extern       char     *GameName(const char *Template, const char *Type,
                                const Game *game);
extern const Player   *GameBlack(const Game *game);
extern const Player   *GameWhite(const Game *game);
extern       size_t    GameXSize(const Game *game);
extern       size_t    GameYSize(const Game *game);
extern       int       GameAllowSuicide(const Game *game);
extern       Gamelog  *GameGamelog(const Game *game);
extern const char     *GameKomi(const Game *game);
extern       int       GameHandicap(const Game *game);
extern       XtPointer LastFromXY(const Game *game, int x, int y);
extern       void      XYFromLast(const Game *game, int *x, int *y,
                                 XtPointer last);
extern       int       GameServerId(const Game *game);
extern       int       ReviewP(const Game *game);
extern       int       TeachingP(const Game *game);
extern       int       RequestP(const Game *game);
extern       int       PlayingP(const Game *game);
extern       int       TersePlay(const Game *game);
extern       int       MyGameP(const Game *game);
extern       int       MyTurnP(const Game *game);
extern       int       ScoringP(const Game *game);
extern       int       BusyP(const Game *game);
extern       Game     *ObservedGame(void);
extern       int       SayP(const Player *player);
extern const Player   *MyOpponent(const Player *player);
extern const char     *GetTime(const Game *game);
extern       void      TestDeleteGame(Game *game);
extern       void ShowObservers(int Id, const char *black, const char *white,
                                const NameList *observers);
extern       void      GamesTime(unsigned long diff);
extern       void      BetResults(BetDesc *Win, BetDesc *Even, BetDesc *Loose,
				  const char *mybet);
# undef __PRINTF1
# undef __PRINTF2
# undef __PRINTF3
#endif /* GAMES_H */
