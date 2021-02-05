#ifndef MESSAGES_H
# define MESSAGES_H

# ifdef	__GNUC__
/* Figure out how to declare functions that (1) depend only on their
   parameters and have no side effects, or (2) don't return.  */
#  if __GNUC__ < 2 || (__GNUC__ == 2 && __GNUC_MINOR__ < 5) /* Old GCC way. */
#   define       __PRINTF1
#  else                                                     /* New GCC way. */
#   define       __PRINTF1     __attribute__ ((format (printf, 1, 2)))
#  endif
# else	/* Not GCC.  */
#  define        __PRINTF1
# endif	/* GCC.  */

# include <X11/Intrinsic.h>

extern Widget ServerButton;

extern void InitMessages(Widget toplevel);
# ifndef   HAVE_NO_STDARG_H
extern void ServerMessage(const char *Message, ...)       __PRINTF1;
# else  /* HAVE_NO_STDARG_H */
extern void ServerMessage();
# endif /* HAVE_NO_STDARG_H */
#endif /* MESSAGES_H */
