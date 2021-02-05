#ifndef STATS_H
# define STATS_H

# include <X11/Intrinsic.h>
# include "gointer.h"
# include "players.h"

typedef struct _Stats Stats;

extern void InitStats(Widget Toplevel);
extern void SetStatsDescription(Stats *stats);
extern void ShowStats(NameVal *stats, NameVal *ExtStats);
extern void CallGetStats(Widget w, XtPointer clientdata, XtPointer calldata);
extern void GetExactRating(const Player *player);
extern void RefreshRating(const Player *player);
extern void RefreshStored(const Player *player);
extern void SetStat(const char *Name, int value);
#endif /* STATS_H */
