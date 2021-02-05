#ifndef MATCH_H
# define MATCH_H
# include "players.h"
# include "gointer.h"

#define TOURNAMENTTYPE   1
#define NOTOURNAMENTTYPE 2
#define IGSTYPE          4
#define GOETYPE          8

typedef struct _Match Match;

struct _DisputeDesc {
    DisputeDesc *Next, *Previous;
    Player      *Player;
    int          Color;
    size_t       SizeX, SizeY;
    int          Tim, ByoYomi;
};

extern void InitMatch(Widget Toplevel);
extern void AutoMatchRequest(const NameList *Lines);
extern void AutoMatchDispute(const char *FirstLine, const NameList *Lines);
extern void Decline(Player *player);
extern void MatchRequest(int Rules, const NameList *names);
extern void Dispute(DisputeDesc *Lines, int FromOther);
extern void CoupleChallenge(Player *player, Widget Toggle);
extern void CoupleChallengeTell(Player *player, Widget Root,
                                Widget (*InitFun)(XtPointer Closure));
extern void SetMatchDescription(Match *match);
extern void CallGetChallenge(Widget w,
                             XtPointer clientdata, XtPointer calldata);
extern void WantMatchType(Player *player, int type);
#endif /* MATCH_H */
