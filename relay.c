/*******************************************************/
/* relay.c: A program to pass on gateway connections   */
/*                                                     */
/* Author: Ton Hospel                                  */
/*         ton@linux.cc.kuleuven.ac.be                 */
/*         (1994, 1995)                                */
/*                                                     */
/* Copyright: GNU copyleft                             */
/*******************************************************/

#ifdef HAVE_TERMNET
# include <termnet.h>
#endif /* HAVE_TERMNET */

#ifdef HAVE_SOCKS
# define connect	Rconnect
# define getsockname	Rgetsockname
# define bind		Rbind
# define accept		Raccept
# define listen		Rlisten
# define select		Rselect
#endif /* HAVE_SOCKS */

#include <mymalloc.h>
#include <except.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if !STDC_HEADERS && HAVE_MEMORY_H
# include <memory.h>
#endif /* not STDC_HEADERS and HAVE_MEMORY_H */

#include <errno.h>
#include <signal.h>
#ifndef RETSIGTYPE
# define RETSIGTYPE void
#endif /* RETSIGTYPE */
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <fcntl.h>

#ifndef HAVE_NO_UNISTD_H
# include <unistd.h>
#endif /* HAVE_NO_UNISTD_H */

#ifdef HAVE_SYS_SELECT_H
# include <sys/select.h>
#endif

#define INT_FROM_VOID(x) ((int)(long) (x))
#define VOID_FROM_INT(x) ((void *)(long) (x))

extern int   isatty(int fd);
extern pid_t fork(/* void */);
extern pid_t wait(/* int *stat_loc */ );
extern pid_t getpid(/* void */);
extern pid_t getppid(/* void */);
extern int execvp(/* char *file, char *argv[] */);
extern int pause(/* void */);
extern unsigned int alarm(/* unsigned int secs */);

#ifndef _POSIX_SOURCE
extern int read( /* int fd,       char *buf, unsigned int n */);
extern int write(/* int fd, const char *buf, unsigned int n */);
#endif /* _POSIX_SOURC */
extern int close(/* int fd */);
extern int dup(  /* int fd */);
extern int pipe(/* int fd[2] */);
extern int gethostname(/* char *address, int addrlen */);
extern int select(/* int nfds,
                     fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
                     struct timeval *timeout */);
#ifdef    HAVE_NO_MEMCHR_PROTO
extern void *memchr(const void *s, int c, size_t n);
#endif /* HAVE_NO_MEMCHR_PROTO */

static int    Talk[6];
static char  *RelayProgram;
static size_t RelayLen;

#define EEA ErrnoExceptionAction
#define SE  static Exception
#ifdef PSEUDOBSD
SE PseudoMasterError = { "Could not open master pseudoterminal" };
SE PseudoSlaveError  = { "Could not open slave pseudoterminal" };
SE IoctlError   = { "ioctl() call failed",                  0, EEA };
#endif /* PSEUDOBSD */
SE HostError    = { "Could not execute gethostname()",      0, EEA };
SE ReadError    = { "Read error occured",                   0, EEA };
SE WriteError   = { "Write error occured",                  0, EEA };
SE SockError    = { "Could not create socket",              0, EEA };
SE BindError    = { "Could not bind socket",                0, EEA };
SE GetNameError = { "Could not get socket name",            0, EEA };
SE ListenError  = { "Could not set backlog",                0, EEA };
SE AcceptError  = { "Could not accept connection",          0, EEA };
SE PipeError    = { "Could not open pipe",                  0, EEA };
SE ForkError    = { "Could not execute fork",               0, EEA };
SE CloseError   = { "Failed to close",                      0, EEA };
SE DupError     = { "Failed to dup",                        0, EEA };
SE SelectError  = { "Select() call failed",                 0, EEA };
SE ExecError    = { "Could not exec command",               0, EEA };
SE WaitError    = { "wait() for child failed",              0, EEA };
#undef EEA
#undef SE

static int MakeConnection(int Port)
{
    int sock, length;
    struct sockaddr_in server;

    /* Create socket */
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) Raise(SockError);
    WITH_HANDLING {
        /* Name socket using wildcards */
        server.sin_family = AF_INET;
        server.sin_addr.s_addr = INADDR_ANY;
        server.sin_port = htons((short) Port);
        if (bind(sock, (struct sockaddr *) &server, sizeof(server)))
            Raise(BindError);

        /* Find out assigned port number and print it out */
        length = sizeof(server);
        if (getsockname(sock, (struct sockaddr *) &server, (void*)&length))
            Raise(GetNameError);
        printf("Socket has port %d\r\n", ntohs(server.sin_port));
    } ON_EXCEPTION {
        close(sock);
    } END_HANDLING;
    return sock;
}

#ifdef PSEUDOBSD
static char PtyName[] = "/dev/ptyXY";

int MasterPseudoTerminal(char *Name)
{
    char       *nptr, *lptr, *end, ch;
    int         fd;
    struct stat statbuf;

    if (!Name) Raise(PseudoMasterError);
    end  = &Name[strlen(Name)-2];
    for (lptr = "pqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"; ch = *lptr; lptr++) {
        end[0] = ch;
        end[1] = '0';
        if (stat(Name, &statbuf) < 0) Raise(PseudoMasterError);
        for (nptr = "0123456789abcdef"; ch = *nptr; nptr++) {
            end[1] = ch;
            fd = open(Name, O_RDWR);
            if (fd >= 0) return fd;
        }
    }
    Raise(PseudoMasterError);
    return -1;
}

int SlavePseudoTerminal(char *Name)
{
    int fd;

    if (!Name) Raise(PseudoSlaveError);
    Name[strlen(Name)-5] = 't';
    fd = open(Name, O_RDWR);
    if (fd < 0) Raise(PseudoSlaveError);
    return fd;
}

typedef struct {
    struct sgttyb  BasicModes;
    struct tchars  BasicControlchars;
    struct ltchars LineControlchars;
    struct winsize WindowSize;
    int            LocalMode, LineDiscipline;
} TTYMode;

static void TTYGetMode(int fd, TTYMode *Mode)
{
    if (ioctl(fd, TIOCGETP,   (char *) &Mode->BasicModes) < 0)
        Raise1(IoctlError, "while getting tty basic modes");
    if (ioctl(fd, TIOCGETC,   (char *) &Mode->BasicControlchars) < 0)
        Raise1(IoctlError, "while getting tty basic control chars");
    if (ioctl(fd, TIOCGLTC,   (char *) &Mode->LineControlchars) < 0)
        Raise1(IoctlError, "while getting tty line discipline control chars");
    if (ioctl(fd, TIOCLGET,   (char *) &Mode->LocalMode) < 0)
        Raise1(IoctlError, "while getting tty local mode word");
    if (ioctl(fd, TIOCGETD,   (char *) &Mode->LineDiscipline) < 0)
        Raise1(IoctlError, "while getting tty line discipline word");
    if (ioctl(fd, TIOCGWINSZ, (char *) &Mode->WindowSize) < 0)
        Raise1(IoctlError, "while getting tty terminal and window sizes");
}

static void TTYSetMode(int fd, TTYMode *Mode)
{
    if (ioctl(fd, TIOCSETP,   (char *) &Mode->BasicModes) < 0)
        Raise1(IoctlError, "while setting tty basic modes");
    if (ioctl(fd, TIOCSETC,   (char *) &Mode->BasicControlchars) < 0)
        Raise1(IoctlError, "while setting tty basic control chars");
    if (ioctl(fd, TIOCSLTC,   (char *) &Mode->LineControlchars) < 0)
        Raise1(IoctlError, "while setting tty line discipline control chars");
    if (ioctl(fd, TIOCLSET,   (char *) &Mode->LocalMode) < 0)
        Raise1(IoctlError, "while setting tty local mode word");
    if (ioctl(fd, TIOCSETD,   (char *) &Mode->LineDiscipline) < 0)
        Raise1(IoctlError, "while setting tty line discipline word");
    if (ioctl(fd, TIOCSWINSZ, (char *) &Mode->WindowSize) < 0)
        Raise1(IoctlError, "while setting tty terminal and window sizes");
}

static void TTYRaw(int fd)
{
    struct sgttyb mode;

    if (ioctl(fd, TIOCGETP, (char *) &mode) < 0)
        Raise1(IoctlError, "while getting tty basic modes");
    mode.sg_flags |= RAW;
    mode.sg_flags &= ~ECHO;
    if (ioctl(fd, TIOCSETP, (char *) &mode) < 0)
        Raise1(IoctlError, "while setting tty basic modes");
}

static int     master;
static char    TermName[11];
static TTYMode Full;

static RETSIGTYPE ChangeSize(int sig)
{
    if (ioctl(0,      TIOCGWINSZ, (char *) &Full.WindowSize) < 0)
        Raise1(IoctlError, "while getting tty terminal and window sizes");
    if (ioctl(master, TIOCSWINSZ, (char *) &Full.WindowSize) < 0)
        Raise1(IoctlError, "while setting tty terminal and window sizes");
    signal(SIGWINCH, ChangeSize);
}
#endif /* PSEUDOBSD */

#define IOREAD   1
#define IOWRITE  2
#define IOERROR  4

typedef struct wantio *WantIo;
typedef void IoFun(WantIo Id, int Fid, void *Closure);
struct wantio {
    struct wantio *Next, *Previous;
    int            Fid, Type;
    IoFun         *Fun;
    void          *Closure;
};

static struct wantio WantIos = { &WantIos, &WantIos };
static fd_set WantIn, WantOut, WantErr;
static int    MaxFd, WantRedo;
static WantIo NextWantIo;

static WantIo AddInput(int Fid, int Type, IoFun *Fun, void *Closure)
{
    WantIo Ptr;

    Ptr = mynew(struct wantio);
    Ptr->Fid  = Fid;
    Ptr->Fun  = Fun;
    Ptr->Type = Type;
    Ptr->Closure = Closure;
    Ptr->Next = WantIos.Next;
    Ptr->Previous = &WantIos;
    Ptr->Next->Previous = Ptr->Previous->Next = Ptr;
    if (!WantRedo) {
        if (Type & IOREAD)  FD_SET(Fid, &WantIn);
        if (Type & IOWRITE) FD_SET(Fid, &WantOut);
        if (Type & IOERROR) FD_SET(Fid, &WantErr);
        if (Fid > MaxFd)    MaxFd = Fid;
    }
    return Ptr;
}

static void RemoveInput(WantIo Ptr)
{
    if (NextWantIo == Ptr) NextWantIo = Ptr->Next;
    Ptr->Previous->Next = Ptr->Next;
    Ptr->Next->Previous = Ptr->Previous;
    myfree(Ptr);
    WantRedo = 1;
}

static void InitInput(void)
{
    WantIos.Next = WantIos.Previous = &WantIos;
    FD_ZERO(&WantIn);
    FD_ZERO(&WantOut);
    FD_ZERO(&WantErr);
    MaxFd = -1;
    WantRedo = 0;
    NextWantIo = 0;
}

/* Wait is not reentrant due to the NextWantIo variable */
static int Wait(void)
{
    WantIo Ptr;
    int    Type, Fid, rc;
    fd_set in, out, err;

    if (WantRedo) {
        MaxFd = -1;
        FD_ZERO(&WantIn);
        FD_ZERO(&WantOut);
        FD_ZERO(&WantErr);
        for (Ptr = WantIos.Next; Ptr != &WantIos; Ptr = Ptr->Next) {
            Type = Ptr->Type;
            Fid  = Ptr->Fid;
            if (Fid > MaxFd)    MaxFd = Fid;
            if (Type & IOREAD)  FD_SET(Fid, &WantIn);
            if (Type & IOWRITE) FD_SET(Fid, &WantOut);
            if (Type & IOERROR) FD_SET(Fid, &WantErr);
            WantRedo = 0;
        }
    }
    memcpy(&in,  &WantIn,  sizeof(in));
    memcpy(&out, &WantOut, sizeof(out));
    memcpy(&err, &WantErr, sizeof(err));
    do {
        rc = select(MaxFd+1, &in, &out, &err, NULL);
    } while (rc < 0 && errno == EINTR);
    if (rc < 0) Raise(SelectError);

    for (Ptr = WantIos.Next; Ptr != &WantIos; Ptr = NextWantIo) {
        NextWantIo = Ptr->Next;
        Type = Ptr->Type;
        Fid  = Ptr->Fid;
        if (((Type & IOREAD)  && FD_ISSET(Fid, &in))  || 
            ((Type & IOWRITE) && FD_ISSET(Fid, &out)) ||
            ((Type & IOERROR) && FD_ISSET(Fid, &err))) {
            (*Ptr->Fun)(Ptr, Ptr->Fid, Ptr->Closure);
            break;
        }    
    }
    return rc;
}

static void CleanInput(void)
{
    WantIo Ptr;

    while (Ptr = WantIos.Next, Ptr != &WantIos) RemoveInput(Ptr);
}


static int    togospel, monitor, consock, Running, Pipe[2];
static size_t match;

static RETSIGTYPE CatchChild(int sig)
{
    while (-1 == wait(NULL)) if (errno != EINTR) Raise(WaitError);
    write(Pipe[1], " ", 1);
    /* signal(SIGCLD, CatchChild); */
}

#define Close(fd, message)                              \
do {                                                    \
    cfd = fd; fd = -1;                                  \
    if (close(cfd)) Raise1(CloseError, message);        \
} while (0)

static pid_t Shell(const char *Command)
{
    char   *Text, *End, **curarg, *From, *To, *argv[10];
    pid_t   TalkPid;  
    int     cfd;

    switch(TalkPid = fork()) {
      case -1:	/* Fail */
        Raise(ForkError);
        break;
      case 0:	/* Child */
        /* CleanHandlers(); */
#ifdef    PSEUDOBSD
    {
        TTYMode Mode;
        int     slave;
        
        TTYGetMode(0, &Mode);
        cfd = open("/dev/tty", O_RDWR);
        if (cfd > 0) {
            if (ioctl(cfd, TIOCNOTTY, (char *) 0) < 0) Raise(IoctlError);
            close(cfd);
        }
        slave = SlavePseudoTerminal(TermName);
        TTYSetMode(slave, &Mode);

        if (close(0))         Raise1(CloseError,  "shell stdin");
        if (dup(slave) == -1) Raise1(DupError, "to shell stdin");

        if (close(1))         Raise1(CloseError,  "shell stdout");
        if (dup(slave) == -1) Raise1(DupError, "to shell stdout");

        if (close(2))         Raise1(CloseError,  "shell stderr");
        if (dup(slave) == -1) Raise1(DupError, "to shell stderr");

        if (close(slave))     Raise1(CloseError,  "slave pty");
        Close(master, "master pty");
    }
#else  /* PSEUDOBSD */
        if (close(0))           Raise1(CloseError,  "shell stdin");
        if (dup(Talk[0]) == -1) Raise1(DupError, "to shell stdin");
        Close(Talk[0], "sink stdin");
        Close(Talk[1], "source stdin");

        if (close(1))           Raise1(CloseError,  "shell stdout");
        if (dup(Talk[3]) == -1) Raise1(DupError, "to shell stdout");
        Close(Talk[3], "sink stdout");
        Close(Talk[2], "source stdout");

        if (close(2))           Raise1(CloseError,  "shell stderr");
        if (dup(Talk[5]) == -1) Raise1(DupError, "to shell stderr");
        Close(Talk[5], "sink stderr");
        Close(Talk[4], "source stderr");
#endif /* PSEUDOBSD */

        Text = mystrdup(Command);
        End  = strchr(Text, 0);
        curarg = argv;
        for (From = Text; (To = memchr(From, ' ', (size_t) (End-From))) != 0;
             From = To) {
            *To++     = 0; 
            while (*To == ' ') To++;
            *curarg++ = From;
        }
        *curarg++ = From;
        *curarg = NULL;

        if (!argv[0]) From = NULL;
        else if ((From = strrchr(argv[0], '/')) != NULL) From++;
        else From = argv[0];

        execvp(From, argv);
        Raise1(ExecError, Command);
        break;
      default:  /* Parent */
        Running = 1;
#ifndef SIGCHLD
# define SIGCHLD SIGCLD
#endif /* SIGCLD */
        signal(SIGCHLD, CatchChild);
#ifdef    PSEUDOBSD
        signal(SIGWINCH, ChangeSize);
#else  /* PSEUDOBSD */
/*      if (close(0)) Raise1(CloseError, "tty stdin"); */
        Close(Talk[0], "sink stdin");
        Close(Talk[3], "sink stdout");
        Close(Talk[5], "sink stderr");
#endif /* PSEUDOBSD */
        break;
    }
    return TalkPid;
}
#undef Close

static const char Escape[] = "^xgospel ";
static size_t     EscapeLength = sizeof(Escape)-1;

static void StdoutInput(WantIo Id, int Fid, void *Closure)
{
    char Buffer[1000], *ptr, *from;
    int  rc, pos;
    size_t left, len;

    rc = read(Fid, Buffer, sizeof(Buffer));
    if (rc> 0)
        if (consock < 0) write(1, Buffer, (size_t) rc);
        else if (togospel) {
            write(consock, Buffer, (size_t) rc);
            if (monitor) write(1, Buffer, (size_t) rc);
        } else {
            from = Buffer;
            pos = 0;
            left = rc;
            do {
                if (match == 0) {
                    ptr = memchr(from, Escape[0], left);
                    if (!ptr) {
                        write(1, from, left);
                        break;
                    }
                    left = ptr-from;
                    if (left) write(1, from, left);
                    pos  = ptr-Buffer;
                    from = ptr;
                    left = rc-pos;
                }
                len = EscapeLength-match;
                if (left < len) len = left;
                if (memcmp(from, Escape+match, len)) {
                    if (match) {
                        write(1, Escape, match);
                        match = 0;
                    } else {
                        write(1, from, 1);
                        pos++;
                        from++;
                        left--;
                    }
                } else {
                    match += len;
                    if (match == EscapeLength) togospel = 1;
                    pos += len;
                    from += len;
                    left = rc-pos;
                    if (left)
                        if (togospel) {
                            write(consock, from, left);
                            if (monitor) write(1, from, left);
                        } else write(1, from, left);
                    break;
                }
            } while (left > 0);
        }
    else if (rc < 0) Raise(ReadError);
    else Running = 0;
}

static void ErrorInput(WantIo Id, int Fid, void *Closure)
{
    Running = 0;
    RemoveInput(Id);
}

static void UserInput(WantIo Id, int Fid, void *Closure)
{
    char Buffer[1000];
    int rc;

    rc = read(Fid, Buffer, sizeof(Buffer));
    if (rc > 0) write(INT_FROM_VOID(Closure), Buffer, (size_t) rc);
    else if (rc < 0) Raise(ReadError);
    else Running = 0;
}

static void ConsockInput(WantIo Id, int Fid, void *Closure)
{
    char Buffer[1000];
    int  rc;

    rc = read(Fid, Buffer, sizeof(Buffer));
    if (rc > 0) {
        if (monitor) {
            fprintf(stdout, "(%.*s)", rc, Buffer);
            fflush(stdout);
        }
        write(INT_FROM_VOID(Closure), Buffer, (size_t) rc);
    } else if (rc < 0) Raise(ReadError);
    else {
        togospel = 0;
        match = 0;
        RemoveInput(Id);
        if (close(Fid)) Raise(CloseError);
        consock = -1;
        puts("Connection with xgospel closed\r");
        fflush(stdout);
    }
}

static void ConnectAttempt(WantIo Id, int Fid, void *Closure)
{
    int tmp;

    if (consock < 0) {
        consock = accept(Fid, 0, 0);
        if (consock < 0) Raise(AcceptError);
        AddInput(consock, IOREAD, ConsockInput, VOID_FROM_INT(Talk[1]));
        puts("Connection with xgospel established.\r");
        if (RelayProgram && RelayLen != write(Talk[1], RelayProgram, RelayLen))
            Raise(WriteError);
    } else {
        puts("Connection attempt while already connected.\r");
        tmp = accept(Fid, 0, 0);
        if (tmp == -1) Raise(AcceptError);
        if (close(tmp)) Raise(CloseError);
        puts("Boarders have been repelled.\r");
    }
    fflush(stdout);
}

int main(int argc, char **argv)
{
    const char   *shell;
    int           sock;

    ExceptionProgram = argv[0];

    togospel = 0;
    monitor  = 1;

#ifdef    PSEUDOBSD
    TTYGetMode(0, &Full);
    strcpy(TermName, "/dev/ptyXY");
    master = MasterPseudoTerminal(TermName);
    Talk[1] = Talk[2] = Talk[4] = master;
#else  /* PSEUDOBSD */
    if (pipe(Talk+0)) Raise(PipeError);
    if (pipe(Talk+2)) Raise(PipeError);
    if (pipe(Talk+4)) Raise(PipeError);
#endif /* PSEUDOBSD */
    WITH_UNWIND {
#ifdef    PSEUDOBSD
        shell = getenv("SHELL");
        if (!shell || !*shell) shell = "/bin/sh";
        Shell(shell);
#else  /* PSEUDOBSD */
        RelayProgram = getenv("RELAYPROGRAM");
        if (RelayProgram) {
            RelayLen = strlen(RelayProgram);
            RelayProgram = mystrndup(RelayProgram, RelayLen+1);
            RelayProgram[RelayLen++] = '\n';
        }
        WITH_UNWIND {
            shell = getenv("RELAYSHELL");
            if (!shell || !*shell) {
                char Buffer[200];
                static char Command[] = "rlogin ";
            
                memcpy(Buffer, Command, sizeof(Command)-1);
                if (gethostname(Buffer+sizeof(Command)-1,
                                sizeof(Buffer)-sizeof(Command)))
                    Raise(HostError);
                Buffer[sizeof(Buffer)-1] = 0;
                Shell(Buffer);
            } else Shell(shell);
#endif /* PSEUDOBSD */
            if (pipe(Pipe)) Raise(PipeError);
            WITH_UNWIND {
#ifdef    PSEUDOBSD
                TTYRaw(0);
#endif /* PSEUDOBSD */
                sock = MakeConnection(0);
                WITH_UNWIND {
                    printf("Waiting for xgospel\r\n"); fflush(stdout);
                    if (listen(sock, 5)) Raise(ListenError);
                    consock = -1;
                    InitInput();
                    WITH_UNWIND {
                        AddInput(sock, IOREAD, ConnectAttempt, NULL);
                        AddInput(0, IOREAD, UserInput, VOID_FROM_INT(Talk[1]));
                        AddInput(Talk[2], IOREAD, StdoutInput, NULL);
                        AddInput(Talk[4], IOREAD, StdoutInput, NULL);
                        AddInput(Pipe[0], IOREAD, ErrorInput, NULL);
                        match = 0;
                        do {
                            Wait();
                        } while(Running);
                    } ON_UNWIND {
                        CleanInput();
                    } END_UNWIND;
                } ON_UNWIND {
                    close(sock);
                } END_UNWIND;
            } ON_UNWIND {
                close(Pipe[0]);
                close(Pipe[1]);
            } END_UNWIND;
        } ON_UNWIND {
            myfree(RelayProgram);
        } END_UNWIND;
    } ON_UNWIND {
#ifdef    PSEUDOBSD
        if (master >= 0) close(master);
        TTYSetMode(0, &Full);
#else  /* PSEUDOBSD */
        int i, *Ptr;

        for (i=sizeof(Talk)/sizeof(*Talk), Ptr = Talk; i>0; i--, Ptr++)
            if (*Ptr >= 0 && close(*Ptr)) Raise(CloseError);
#endif /* PSEUDOBSD */
        printf("%s done\n", argv[0]); fflush(stdout);
    } END_UNWIND;
    return 0;
}
