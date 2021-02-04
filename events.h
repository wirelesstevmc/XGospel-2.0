#ifndef EVENTS_H
# define EVENTS_H

# include <X11/Intrinsic.h>
# include "players.h"

extern Widget EventsButton;
extern int maintainerRating;

typedef struct _Event  *Event;
typedef struct _Action *Action;
typedef void FreeFun(XtPointer);
typedef int (*EventFunPtr)(/* Anything goes */);
typedef void ActionFunction(Action action, EventFunPtr Fun, XtPointer FunArgs,
                            XtPointer Match, XtPointer Closure);

extern Event  AddEvent(EventFunPtr Fun, XtPointer Match, FreeFun *FreeMatch);
extern Action AddAction(Event event, unsigned long Delay, ActionFunction *Fun,
                        XtPointer Closure, FreeFun *FreeClosure);
extern void DeleteEvent(Event event);
extern void DeleteAction(Action action);
extern void InitEvents(Widget TopLevel);
extern void CleanEvents(void);

/* Builtin actions */
extern int EnterServer(const Player *player);
extern int Logon (const Player *player);
extern int Logoff(const Player *player);
extern int GotTell(Player *player, const char *Message);
extern int GotNoTell(Player *player, const char *Message);
extern int GotStats(const Player *player, const NameVal *stats);
#endif /* EVENTS_H */

