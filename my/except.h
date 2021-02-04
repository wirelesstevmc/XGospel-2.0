/*
 * A general-purpose exception-handling system for C
 *      based on the Byte article by Jonathan Amsterdam, 1991
 *
 * Heavy modifications by Ton Hospel, 1992
 */

#ifndef __EXCEPTION_H
#define __EXCEPTION_H

#ifdef	__GNUC__
/* Figure out how to declare functions that (1) depend only on their
   parameters and have no side effects, or (2) don't return.  */
# if __GNUC__ < 2 || (__GNUC__ == 2 && __GNUC_MINOR__ < 5)   /* Old GCC way. */
#  ifndef	__NORETURN
#   define	__NORETURN	__volatile__
#   define	__NORETURN2
#  endif
# else                                                       /* New GCC way. */
#  ifndef	__NORETURN
#   define	__NORETURN
#   ifdef noreturn
#    define	__NORETURN2	/* We don't want empty __attribute__ (()). */
#   else
#    define	__NORETURN2	__attribute__ ((noreturn))
#   endif
#  endif
# endif
#else	/* Not GCC.  */
# define	__NORETURN		/* No functions-of-no-return.  */
# define	__NORETURN2
#endif	/* GCC.  */

#include <setjmp.h>
#include <errno.h>

#define ExceptionCopy           ExcCopy /* For linkers with few characters */
#define ExceptionErrno          ExcErrno
#define ExceptionItem           ExcItem
#define ExceptionArgs           ExcArgs
#define ExceptionFile           ExcFile
#define ExceptionLine           ExcLine

typedef void    ExceptionCleanup(                 int Args, void **Item);
typedef void    ExceptionAction(const char *Name, int Args, void **Item);

extern void *ExceptionItem[4]; /* Saved arguments of Raise()                 */
extern int   ExceptionArgs;    /* Number of arguments given to public Raise()*/
#ifndef   SIMPLE_EXCEPTIONS
extern const char *ExceptionFile; /* Name of file containing the Raise()     */
extern   int ExceptionLine;    /* Linenumber of Raise() in this file         */
#endif /* SIMPLE_EXCEPTIONS */

#define EXCEPTIONNODUMP 1      /* Don't dump core even if dump variable set */
#define EXCEPTIONERRNO  2      /* When this happens, errno will contain data*/
#define EXCEPTIONFATAL  4      /* No way we can recover from this           */
#define EXCEPTIONGOOD   8      /* This is a benign exception. Don't panic   */

typedef struct {
    const char       *Name;    /* Name of this exception (default errortext) */
    ExceptionCleanup *cleanup; /* Item cleanup code                          */
    ExceptionAction  *action;  /* Code to execute when no handler            */
    int               Flags;   /* Extra flags for exception behaviour        */
} Exception;

typedef struct jbr {
    jmp_buf         jb;
    struct jbr     *Next, *Self;
} JmpBufRec;

extern void DefaultExceptionAction(const char *Name, int Args, void **Data);
extern void ErrnoExceptionAction(const char *Name, int Args, void **Data);
extern int AbortOnException, ExceptionErrno;
extern const char *ExceptionProgram;
/*      Variable containing the current exception       */
extern Exception *theException;

/*      Some predefined exceptions                      */
extern Exception OutOfMemory, ReRaiseException,
                 AssertException, ErrnoAssertException,
                 FatalException, ErrnoFatalException;

extern void     CleanHandlers(void);
extern int      TestHandlers(const char *Context);
extern void     PushJbr(JmpBufRec *jbr);
extern void     PopJbr(void);
/* The real exception raiser */
extern void     __NORETURN _raise_(void) __NORETURN2;
extern char    *ExceptionCopy(const char *Text);
extern const char *strerrno(void);
extern const char *StrExceptionErrno(void);

#define ExceptionP(Exception)   (theException == &(Exception))
#define ExceptionPending()      (theException != NULL)
#define ExceptionName()         (theException->Name)
#define ClearException()        (theException = 0)
#define ExceptionArg(nr, type)  ((type) theException->Item[nr])

/* If you use arguments, make sure they are still in scope by the time the */
/* exceptionaction gets called                                             */

#ifndef    SIMPLE_EXCEPTIONS
# define __File__       __FILE__
# define __Line__       __LINE__
# define _Raise_(args)                                                       \
        ExceptionFile = __File__;                                            \
        ExceptionLine = __Line__;                                            \
        ExceptionArgs = args;                                                \
        _raise_()
#else /* SIMPLE_EXCEPTIONS */
# define _Raise_(args)                                                       \
        ExceptionArgs = args;                                                \
        _raise_()
#endif /* SIMPLE_EXCEPTIONS */

# define Raise(Exception)                                                    \
do {                                                                         \
        ExceptionErrno = errno;                                              \
        theException = &(Exception);                                         \
        _Raise_(0);                                                          \
} while (0)

# define Raise1(Exception, Arg0)                                             \
do {                                                                         \
        ExceptionErrno = errno;                                              \
        theException = &(Exception);                                         \
        ExceptionItem[0] = (void *) (Arg0);                                  \
        _Raise_(1);                                                          \
} while(0)

# define Raise2(Exception, Arg0, Arg1)                                       \
do {                                                                         \
        ExceptionErrno = errno;                                              \
        theException = &(Exception);                                         \
        ExceptionItem[0] = (void *) (Arg0);                                   \
        ExceptionItem[1] = (void *) (Arg1);                                  \
        _Raise_(2);                                                          \
} while (0)

# define Raise3(Exception, Arg0, Arg1, Arg2)                                 \
do {                                                                         \
        ExceptionErrno = errno;                                              \
        theException = &(Exception);                                         \
        ExceptionItem[0] = (void *) (Arg0);                                  \
        ExceptionItem[1] = (void *) (Arg1);                                  \
        ExceptionItem[2] = (void *) (Arg2);                                  \
        _Raise_(3);                                                          \
} while (0)

# define Raise4(Exception, Arg0, Arg1, Arg2, Arg3)                           \
do {                                                                         \
        ExceptionErrno = errno;                                              \
        theException = &(Exception);                                         \
        ExceptionItem[0] = (void *) (Arg0);                                  \
        ExceptionItem[1] = (void *) (Arg1);                                  \
        ExceptionItem[2] = (void *) (Arg2);                                  \
        ExceptionItem[3] = (void *) (Arg3);                                  \
        _Raise_(4);                                                          \
} while(0)

#define ActionException(Exception)                                           \
    if ((Exception)->action)                                                 \
        (*(Exception)->action)((Exception)->Name, ExceptionArgs,             \
                               ExceptionItem);                               \
    else DefaultExceptionAction((Exception)->Name, ExceptionArgs,            \
                                ExceptionItem)


#define CleanupException(Exception)                                          \
do {                                                                         \
    if ((Exception)->cleanup)                                                \
        (*(Exception)->cleanup)(ExceptionArgs, ExceptionItem);               \
} while (0)


/*      Reraise the current exception                   */
#define ReRaise()                                                       \
        if (theException) _raise_();                                    \
        else Raise(ReRaiseException);

/* So you don't use the raise() from <signal.h> by accident !!! */
/* notice that an exception (lower case first letter) structure */
/* exists in some compilers in math.h                           */

#ifndef raise		/* SCO defines raise(sig) as kill(getpid(), sig) */
extern int      raise(int sig);
#endif /* raise */

/* Start a piece of code in which you want exception handling       */
/* Code that executes the WITH_... must also execute the            */
/* corrsponding END_... (and they must be properly nested), so e.g. */
/* don't return from inside an exception level                      */
/* Also notice that registers can be saved and restored by setjmp   */
/* and longjmp, so throwing an exception can restore variables      */
/* that have been allocated to a register by the compiler (use      */
/* volatile variables in an ANSI C compiler if you don't want this) */

#ifndef   __STDC__
# define WITH_HANDLING {                                                \
      JmpBufRec jbr;                                                    \
                                                                        \
      PushJbr(&jbr);                                                    \
      if (setjmp(jbr.jb) == 0) {
#else  /* __STDC__ */
# ifndef CAT
#  define EXCAT(x, y) x##y
#  define CAT(x, y) EXCAT(x, y)
# endif /* CAT */
# define WITH_HANDLING {                                                \
      JmpBufRec CAT(jbr,__LINE__);                                      \
                                                                        \
      PushJbr(&CAT(jbr,__LINE__));                                      \
      if (setjmp(CAT(jbr,__LINE__).jb) == 0) {
#endif /* __STDC__ */

/*      In case of exception, start executing the following stuff       */
#define ON_EXCEPTION                                                    \
         PopJbr();                                                      \
      } else {

/*      Ends the part with exception handling                           */
#define END_HANDLING                                                    \
         if (theException) _raise_();                                   \
      }                                                                 \
   }

/*      Start a piece of code whose cleanup must be executed            */
#define WITH_UNWIND {                                                   \
   WITH_HANDLING

/*      Introduces the cleanup code                                     */
#define ON_UNWIND                                                       \
         PopJbr();                                                      \
      }                                                                 \
   }

/*      Ends the part with unwind handling                              */
#define END_UNWIND                                                      \
   if (theException) _raise_();                                         \
}
#endif  /* __EXCEPTION_H */
