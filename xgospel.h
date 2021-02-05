#ifndef XGOSPEL_H
# define XGOSPEL_H

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

# include <stdio.h>
# include <X11/Intrinsic.h>

typedef struct {
    String  User, Password, Maintainer, Version, VersionMessage;
    String  Directory;
    String  SgfFilename, PsFilename, KibitzFilename, BroadcastFilename;
    String  YellFilename, IgsMessageFilename, TellFilename, MessageFilename;
    String  EventsFilename, AnalyzeFilename, GamesFilename, PlayersFilename;
    String  MainFilename, AutoReplyMessage;
    struct _StringPairList *PlayerToWidget, *GameToWidget, *ReviewToWidget;
    struct _StringPairList *PlayerImportance;
    struct _StringList     *Friends, *Notifies;
    struct _StringList *TellKill, *YellKill, *KibitzKill, *BroadcastKill;
    struct _StringList *TellPass, *YellPass, *KibitzPass, *BroadcastPass;
    String  Site;
    int     Port;
    int     DebugFun;
    String  DebugFile;
    int     TellSize, YellSize, KibitzSize, BroadcastSize;
    int     ReplayTimeout, WhoTimeout, GamesTimeout, ReviewsTimeout;
    int     ServerTimeout, QuitTimeout, ReconnectTimeout, PlayerUpdateTimeout;
    int     GameUpdateTimeout, ReviewUpdateTimeout, ClockTimeout;
    int     InactiveTimeout;
    int     PlayersScale, GamesScale, ScrollUnit;
    int     AnalyzeSize, DefaultTime, DefaultByoYomi;
    Boolean WantStdout, WantVerbose, AllowSuicide, TersePlay, UseTerm;
    Boolean SimpleNames, NumberKibitzes, UseSay, MarkTerritories, MarkDame;
    Boolean AutoScore;
    String  DateFormat, SgfDateFormat, MinProRank, MinImportantRank;
    Pixel   MyLowTimeBackground, MyLowTimeForeground;
    Pixel   LowTimeBackground, LowTimeForeground;
    int     LowTimeSet, MinSecPerMove, MinLagMargin;
    Boolean AutoReply;
    Time    MinButtonTime;
} AppData, *AppDataPtr;

typedef enum {
    IGS,
    NNGS
} ServerTYPE;

extern AppData appdata;
extern FILE   *DebugFile;
extern int     DebugFun, DebugPending, Entered;
extern int     ServerVersion, ServerSubVersion;
extern ServerTYPE ServerType;
extern Widget  toplevel;
extern char   *MyPassword, *MyName, *UserId, *ServerName;
extern char   *SgfDateFormat, *CompileTime;
extern struct tm LocalTime;

extern void IgsPrompt(void);
extern size_t Output(const char *Text);
extern void UserCommands(void);
# ifndef HAVE_NO_STDARG_H
extern size_t Outputf(const char *Comment, ...) __PRINTF1;
# else  /* HAVE_NO_STDARG_H */
extern size_t Outputf();
# endif /* HAVE_NO_STDARG_H */
# undef __PRINTF1
#endif /* XGOSPEL_H */
