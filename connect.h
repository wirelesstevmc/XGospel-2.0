#ifndef CONNECT_H
# define CONNECT_H

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

# include <X11/Intrinsic.h>
# include <stddef.h>

typedef struct _Connection *Connection;
extern Connection Conn;

# ifndef   HAVE_NO_STDARG_H
extern       void SendCommand(Connection conn, XtPointer Context,
                              const char *Command, ...)       __PRINTF3;
extern       void MoveCommand(Connection conn,
                              const char *Command, ...)       __PRINTF2;
extern       void UserSendCommand(Connection conn, XtPointer Context,
                                  const char *Command, ...)   __PRINTF3;
extern       void AutoCommand(Connection conn,
                              const char *Command, ...)       __PRINTF2;
extern       void LastCommand(Connection conn,
                              const char *Command, ...)       __PRINTF2;
extern       void SharedLastCommand(Connection conn,
                                    const char *Command, ...) __PRINTF2;
extern       void UserCommand(Connection conn,
                              const char *Command, ...)       __PRINTF2;
extern       void FirstCommand(Connection conn,
                               const char *Command, ...)      __PRINTF2;
extern       void ForceCommand(Connection conn,
                               const char *Command, ...)      __PRINTF2;
# else  /* HAVE_NO_STDARG_H */
extern       void SendCommand();
extern       void MoveCommand();
extern       void UserSendCommand();
extern       void AutoCommand();
extern       void LastCommand();
extern       void SharedLastCommand();
extern       void UserCommand();
extern       void FirstCommand();
extern       void ForceCommand();
# endif /* HAVE_NO_STDARG_H */

extern       void  SiteLogon(Connection conn, const char *Host);
extern const char *LogonSite(Connection conn);
extern const char *GetHostname(void);
extern const char *GetUserId(void);
extern const char *strherror(int nr);
extern const char *strherrno(void);
extern       void  ForceQuit(void);
extern        int  ConnectedP(Connection conn);
extern     size_t  GetInput(Connection conn, char *buffer, size_t size);
extern       void  InitConnect(Widget toplevel);
extern       void  CleanConnect(void);
extern       void  ReConnect(Connection conn);
extern Connection  Connect(const char *Site, int Port);
extern const char *Parsing(Connection conn);
extern       void  RemoveMoves(Connection conn);
extern       void  UserActive(Connection conn);
extern       void  SetCommand(Connection conn, int value);
extern       void  ChangeCommand(Connection conn, int value);
extern       void  ResyncCommand(Connection conn);
extern       void  ConnectTime(unsigned long Diff);
extern       void  ShowConnectCounters(Connection conn);
extern        int  WhatCommand(Connection conn, const char *Command);
extern const char *ArgsCommand(Connection conn, const char *Command);
extern const char *StripArgsCommand(Connection conn, const char *Command);
extern const char *StripFirstArgCommand(Connection conn, const char *Command);
extern        int UserCommandP(Connection conn);
extern XtPointer  CommandClosure(Connection conn);
extern       void CallSendCommand(Widget w,
                                  XtPointer clientdata, XtPointer calldata);
# undef __PRINTF1
# undef __PRINTF2
# undef __PRINTF3

#endif /* CONNECT_H */
