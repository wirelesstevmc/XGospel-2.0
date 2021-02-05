#ifndef PLAYERS_H
# define PLAYERS_H

# include <X11/Intrinsic.h>
# include "gointer.h"

# define UNKNOWN        "--"

#define TELLPLAYER      "tell"
#define CHALLENGEPLAYER "challenge"
#define STATSPLAYER     "stats"
#define OBSERVEPLAYER   "observe"
#define DUMPPLAYER      "dumpplayer"
#define MARKPLAYER      "mark"

#define NameToPlayer(Name)   LengthNameToPlayer(Name, -1)
#define NameOnServer(Name)   LengthNameOnServer(Name, -1)
#define PlayerFromName(Name) PlayerFromLengthName(Name, -1)

typedef struct _Player Player;
/* Only drug dealers and the computer industry call their customers users.
   I think this must mean something */
typedef enum _UserType {
    NONXGOSPELUSER, EXXGOSPELUSER, XGOSPELUSER
} UserType;

# include "games.h"

typedef enum {
    UNCHANGED, CHANGED, NEW, DELETED,
    OBSERVED, UNOBSERVED
} State;

extern Widget  PlayersButton;
extern Player *Me;

extern     Player *MakeDummyPlayer(void);
extern       void  RenameDummyPlayer(Player *player, const char *Name);
extern       void  FreeDummyPlayer(Player *player);
extern       void  DumpPlayers(const char *args);
extern       void  UnassumePlayers(Connection conn);
extern       void  TellXgospelUsers(Connection conn, const char *message);
extern       void  NewXgospelUser(Connection conn, const Player *user);
extern        int  StrengthCompare(const Player *player1,
                                   const Player *player2);
extern        int  PlayerCompare(const Player *player1, const Player *player2);
extern        int  PlayersCompare(const void *player1, const void *player2);
extern       void  PlayersResort(void);
extern const char *PlayerString(const Player *player);
extern const char *PlayerNameToString(const char *Name);
extern       char *PlayerTemplateDescription(const Player *player,
                                             const char *Template);
extern       double PlayerExactRating(const Player *player);
extern       int   PlayerRating(const Player *player);
extern       int   StrengthToRating(const char *Strength);
extern const char *RatingToStength(int rating);
extern       void  SetPlayerTitles(Widget w, const Player *player);
extern       void  CheckPlayerStrength(Player *player, const char *Strength);
extern       void  ResetPlayersImportance(void);
extern       void  InitPlayers(Widget Toplevel);
extern       void  CleanPlayers(void);
extern     Player *PlayerConnect(const char *Name, const char *Strength);
extern       void  PlayerDisconnect(const char *Name);
extern       void  UserData(NameListList *Users);
extern       void  AssertPlayersDeleted(void);
extern       void  TestPlayersDeleted(void);
extern       void  PlayerStatusLine(int Players, int MaxPlayers, int Games);
extern       void  ShowPlayerStats(void);
extern     Player *PlayerFromLengthName(const char *Name, int Length);
extern     Player *LengthNameOnServer(  const char *Name, int Length);
extern     Player *LengthNameToPlayer(  const char *Name, int Length);
extern       void  PlayerInGame( Player *player, Game *game, int Id);
extern       void  PlayerOutGame(Player *player, Game *game, int Id);
extern     Player *FindPlayer(const char *Name,  const char *Strength,
                              const char *state, const char *Idle);
extern     Player *FindPlayerByNameAndStrength(const char *Name,
                                               const char *Strength);
extern const char     *GetPlayerType(const Player *player, const char *Prefix);
extern const char     *PlayerToName(const Player *player);
extern const char     *PlayerName(const Player *player);
extern const char     *PlayerToStrength(const Player *player);
extern       int       PlayerToImportance(const Player *player);
extern       int       PlayerToStored(const Player *player);
extern const char     *PlayerToAutoRated(const Player *player);
extern const char     *PlayerToIdle(const Player *player);
extern const NameList *PlayerToResults(const Player *player);
extern       void      AddResults(const char *Name, NameList *Results);
extern       void CallPlayerObserve(Widget w, XtPointer clientdata,
				    XtPointer calldata);
extern       void MyLoseProbas(Player *player, int myRating,
			       int theirRating, const char *handicap,
			       const char *probaLoseAsWhite,
			       const char *probaLoseAsBlack);
extern       void      StoredNum(Player *player, int stored);
extern       int       GuestP(const Player *player);
extern       int       OnServerP(const Player *player);
#endif /* PLAYERS_H */
