#ifndef TELL_H
# define TELL_H

# include <X11/Intrinsic.h>
# include "players.h"

typedef struct _Tell Tell;
extern void  SetTellDescription(Tell *tell);
extern void  Idle(Player *player, const char *time);
extern void  Beeping(Player *player);
extern void  NoTell(void);
extern void  InitTell(Widget Toplevel);
extern void  CleanTell(void);
extern void  CoupleTell(Player *player, Widget w);
extern void  CoupleTellChallenge(Player *player, Widget Root,
                                 Widget (*InitFun)(XtPointer Closure));
extern void  ExplicitTell(char *tellCommand);
extern void  ReceivedTell(Player *player, const char *Message);
extern void  CallGetTell(Widget w, XtPointer clientdata, XtPointer calldata);
extern void  TellMessage(Tell *tell, const char *Message);
#endif /* TELL_H */
