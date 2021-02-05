#ifndef OBSERVE_H
# define OBSERVE_H

# include <X11/Intrinsic.h>

typedef struct _Observe Observe;
# include "players.h"
# include "games.h"

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

# ifndef   HAVE_NO_STDARG_H
extern int GameMessage(const Game *game, const char *Prefix, 
                       const char *Format, ...) __PRINTF3;
extern int NbGameMessage(const Game *game, const char *Prefix, 
                         const char *Format, ...) __PRINTF3;
# else  /* HAVE_NO_STDARG_H */
extern int GameMessage();
extern int NbGameMessage();
# endif /* HAVE_NO_STDARG_H */

extern Observe *OpenObserve(Game *game);
extern void     DestroyObservers(Observe **observers);
extern void     InitObserve(Widget toplevel);
extern void     CleanObserve(void);
extern void     GotoObserve(Observe *observe, size_t Pos);
extern void     PassToDone(const Game *game, int Pass, int done);
extern void     Kibitz(const Game *game, const Player *player,
                       const char *kibitz, size_t length);
extern void     ObserveMessage(const Observe *observers);
extern void     ObserveError(const Observe *observers);
extern int      ObserveMove(const Observe *observe);
extern float    GetWhiteAdvantage(const Observe *observe);
extern void     SetTitle(const Observe *observers, const char *Text);
extern void     SetKomi(const Observe *observers, const char *Text);
extern void     SetHandicap(const Observe *observers, int handicap);
extern void     SetObserveTime(const Observe *observers, const char *Text,
			       int low);
extern void     SetObserveDescriptions(Observe *observers, const Game *game);
extern void     ShowPosition(const Game *game);
extern void     SetCaptures(const Game *game);
extern void     ReplayTime(unsigned long diff, Observe *observers,
                           size_t nodes);
extern void     StopForward(Observe *observers, size_t nodes);

# undef __PRINTF1
# undef __PRINTF2
# undef __PRINTF3

#endif /* OBSERVE_H */
