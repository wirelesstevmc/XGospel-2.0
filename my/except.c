/*
 * A general-purpose exception-handling system for C
 *      based on the Byte article by Jonathan Amsterdam, 1991
 *
 * Heavy modifications by Ton Hospel, 1992
 */

#include <stdio.h>
#include <stdlib.h>                    /* for void exit(int) */
#include <string.h>
#include <stddef.h>
#include "except.h"

#ifdef HAVE_NO_STRERROR_PROTO
extern char *strerror(int err);
#endif /* HAVE_NO_STRERROR_PROTO */

#ifdef HAVE_NO_STRERROR
extern char *sys_errlist[];
extern int   sys_nerr;

static char NoError[]      = "No error";
static char InvalidError[] = "Unknown error number to strerror()";

char *strerror(int nr)
{
    if (0 < nr && nr < sys_nerr) return sys_errlist[nr];
    else if (nr == 0)            return NoError;
    else                         return InvalidError;
}

const char *strerrno(void)
{
    if (0 < errno && errno < sys_nerr) return sys_errlist[errno];
    else if (errno == 0)               return NoError;
    else                               return InvalidError;
}

const char *StrExceptionErrno(void)
{
    if (0 < ExceptionErrno && ExceptionErrno < sys_nerr)
        return sys_errlist[ExceptionErrno];
    else if (ExceptionErrno == 0) return NoError;
    else                          return InvalidError;
}
#else  /* HAVE_NO_STRERROR */
const char *strerrno(void)
{
    return strerror(errno);
}

const char *StrExceptionErrno(void)
{
    return strerror(ExceptionErrno);
}
#endif /* HAVE_NO_STRERROR */

Exception *theException = 0;
void *ExceptionItem[4]; /* Saved arguments of Raise()                 */
int   ExceptionArgs = 0;
/* Number of arguments given to public Raise()*/
#ifndef   SIMPLE_EXCEPTIONS
const char *ExceptionFile = 0; /* Name of file containing the Raise() */
int ExceptionLine         = 0; /* Linenumber of Raise() in this file  */
#endif /* SIMPLE_EXCEPTIONS */

Exception OutOfMemory          = {"Out of memory (buy more ?!)"};
Exception ReRaiseException     = {"Applied ReRaise() without exception"};
Exception FatalException       = {"Fatal,"};
Exception ErrnoFatalException  = {"Fatal,", 0, ErrnoExceptionAction};
Exception AssertException      = {"Assertion failed"};
Exception ErrnoAssertException = {"Assertion failed", 0, ErrnoExceptionAction};

int AbortOnException = 0;
int ExceptionErrno   = 0;
/* Stores the name of the program causing the exception, 0 means unknown */
static char Buffer[2048], *Pos = Buffer;
const char *ExceptionProgram = 0;

JmpBufRec      *CurRec = NULL;

void CleanHandlers(void)
{
    CurRec = 0;
}

int TestHandlers(const char *Context)
{
    JmpBufRec *Here, *Next;

    for (Here = CurRec; Here; Here = Next) {
        Next = Here->Next;
        fflush(stderr);
        if (Here->Self != Here || (Next && Here >= Next)) {
            fprintf(stderr, "%s: Corrupted exception stack !!!!!\n", Context);
            fflush(stderr);
            return 1;
        }
    }
    return 0;
}

void            PushJbr(JmpBufRec * jbr)
{
    jbr->Next = CurRec;
    jbr->Self = jbr;
    CurRec = jbr;
}

void            PopJbr(void)
{
    if (CurRec)
        CurRec = CurRec->Next;
    else {
        fflush(stdout);
        fputs("Attemp to pop empty exception stack\n", stderr);
        fflush(stderr);
        exit(1);
    }
}

void            DefaultExceptionAction(const char *Name, int Args, void **Data)
{
    int             i;

    fflush(stdout);
    if (ExceptionProgram) {
        fputs(ExceptionProgram, stderr);
        fputs(": ", stderr);
    }
    fputs(Name, stderr);
    for (i = 0; i < Args; i++) {
        fputc(' ', stderr);
        fputs((const char *) Data[i], stderr);
    }
    fputc('\n', stderr);
}

void ErrnoExceptionAction(const char *Name, int Args, void **Data)
{
    int             i;
    const char     *message;

    message = strerror(ExceptionErrno);
    fflush(stdout);
    if (ExceptionProgram) {
        fputs(ExceptionProgram, stderr);
        fputs(": ", stderr);
    }
    fputs(Name, stderr);
    for (i = 0; i < Args; i++) {
        fputc(' ', stderr);
        fputs((const char *) Data[i], stderr);
    }
    fputs(": ", stderr);
    fputs(message, stderr);
    fputc('\n', stderr);
}

void _raise_()
{
    JmpBufRec      *jbr;

    Pos = Buffer;
    if (CurRec &&
        (AbortOnException >= 0 || (theException->Flags & EXCEPTIONNODUMP))) {
        jbr = CurRec;
        if (jbr->Self != jbr) {
            fflush(stdout);
            fputs("Corrupted exception stack\n", stderr);
            fflush(stderr);
            exit(3);
        }
        PopJbr();
        longjmp(jbr->jb, 1);
    } else {
        if (theException != &FatalException &&
            theException != &ErrnoFatalException) {
            fflush(stdout);
#ifdef    SIMPLE_EXCEPTIONS
            fputs("Unhandled exception: ", stderr);
#else  /* SIMPLE_EXCEPTIONS */
            fprintf(stderr, "Unhandled exception in file %s, line %d: ",
                    ExceptionFile, ExceptionLine);
#endif /* SIMPLE_EXCEPTIONS */
        }
        ActionException(theException);
        CleanupException(theException);
        fflush(stderr);
        if (AbortOnException) abort();
        exit(2);
    }
}

char *ExceptionCopy(const char *Text)
{
    size_t Length, Free;
    char  *Old;

    if (Text) {
        Old    = Pos;
        Length = strlen(Text);
        Free   = Buffer+sizeof(Buffer)-Pos;
        if (Length >= Free) Length = Free-1;
        memcpy(Pos, Text, Length);
        Pos[Length] = 0;
        Pos += Length+1;
    } else Old = NULL;
    return Old;
}
