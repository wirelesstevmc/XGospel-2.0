/*******************************************************/
/* rport.c: A program to pass on internet connections  */
/*          Comparable to term `tredir'                */
/*                                                     */
/* Author: Ton Hospel                                  */
/*         ton@linux.cc.kuleuven.ac.be                 */
/*         (1994, 1995)                                */
/*                                                     */
/* Copyright: GNU copyleft                             */
/*******************************************************/

#define VERSION "rport 1.2"

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
#include <stddef.h>
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
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
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
#define VOID_FROM_INT(x) ((void *) (long) (x))

#ifndef _POSIX_SOURCE
extern int read( /* int fd,       char *buf, unsigned int n */);
extern int write(/* int fd, const char *buf, unsigned int n */);
#endif /* _POSIX_SOURC */
extern int close(/* int fd */);
extern int gethostname(/* char *address, int addrlen */);
extern int select(/* int nfds,
                     fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
                     struct timeval *timeout */);
#ifdef    HAVE_NO_MEMCHR_PROTO
extern void *memchr(const void *s, int c, size_t n);
#endif /* HAVE_NO_MEMCHR_PROTO */

#ifdef    HAVE_NO_MEMMOVE
void bcopy(/* char *source, char *target, int n */);
# define memmove(to, from, n)   bcopy(from, to, n)
#endif /* HAVE_NO_MEMMOVE */

#define STATIC static 

#define EEA ErrnoExceptionAction
#define SE  STATIC Exception
SE InputLost    = { "Lost stdin. Bailing out"};
SE FcntlError   = { "Could not execute fcntl()",            0, EEA };
SE HostError    = { "Could not execute gethostbyname( "	};
SE SockError    = { "Could not create socket",              0, EEA };
SE BindError    = { "Could not bind socket",                0, EEA };
SE GetNameError = { "Could not get socket name",            0, EEA };
SE ConnectError = { "Could not connect socket",             0, EEA };
SE ListenError  = { "Could not set backlog",                0, EEA };
SE AcceptError  = { "Could not accept connection",          0, EEA };
SE SelectError  = { "Select() call failed",                 0, EEA };

SE SyntaxError  = { "Syntax error" };
SE SilentError  = { "Silent error (you are not seeing this)" };
SE GetServiceError  = { "Could not get service" };
#undef EEA
#undef SE

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

STATIC struct wantio WantIos = { &WantIos, &WantIos };
STATIC fd_set WantIn, WantOut, WantErr;
STATIC int    MaxFd, WantRedo;
STATIC WantIo NextWantIo;

#define FDSET(a, b)                             \
do {                                            \
/*  fprintf(stderr, "Set on fd %d\n", Fid); */  \
    FD_SET(a, b);                               \
} while(0)

STATIC WantIo AddInput(int Fid, int Type, IoFun *Fun, void *Closure)
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
        if (Type & IOREAD)  FDSET(Fid, &WantIn);
        if (Type & IOWRITE) FDSET(Fid, &WantOut);
        if (Type & IOERROR) FDSET(Fid, &WantErr);
        if (Fid > MaxFd)    MaxFd = Fid;
    }
    return Ptr;
}

STATIC void RemoveInput(WantIo Ptr)
{
    if (NextWantIo == Ptr) NextWantIo = Ptr->Next;
    Ptr->Previous->Next = Ptr->Next;
    Ptr->Next->Previous = Ptr->Previous;
    myfree(Ptr);
    WantRedo = 1;
}

STATIC void InitInput(void)
{
    WantIos.Next = WantIos.Previous = &WantIos;
    FD_ZERO(&WantIn);
    FD_ZERO(&WantOut);
    FD_ZERO(&WantErr);
    MaxFd = -1;
    WantRedo   = 0;
    NextWantIo = 0;
}

/* Wait is not reentrant due to the NextWantIo variable */
STATIC int Wait(void)
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
            if (Type & IOREAD)  FDSET(Fid, &WantIn);
            if (Type & IOWRITE) FDSET(Fid, &WantOut);
            if (Type & IOERROR) FDSET(Fid, &WantErr);
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

STATIC void CleanInput(void)
{
    WantIo Ptr;

    while (Ptr = WantIos.Next, Ptr != &WantIos) RemoveInput(Ptr);
}

STATIC size_t ComPos;
STATIC int    IgnoreIn;

extern int h_errno;
STATIC char NoError[]      = "No error";
STATIC char InvalidError[] = "Unknown error number to strherror()";

#ifdef HAVE_H_ERRLIST
extern char *h_errlist[];
extern int   h_nerr;
#else  /* HAVE_H_ERRLIST */
STATIC const char *h_errlist[] = {
    NoError,
    "Authoritive Answer Host not found",
    "Non-Authoritive Host not found, or SERVERFAIL",
    "Non recoverable errors, FORMERR, REFUSED, NOTIMP",
    "Valid host, no address, look for MX record"
};
STATIC int h_nerr = sizeof(h_errlist)/sizeof(*h_errlist);
#endif /* HAVE_H_ERRLIST */

typedef struct _Accept *Accept;
typedef struct _Pass   *Pass;

struct _Pass {
    Accept Parent;
    Pass   Previous, Next;
    struct timeval Start;
    int    Hosted, PeerPort, Local, Log;
    size_t HostToLocalLength, LocalToHostLength;
    int    HostToLocalOffset, LocalToHostOffset;
    unsigned long HostToLocalRead, HostToLocalWritten;
    unsigned long LocalToHostRead, LocalToHostWritten;
    WantIo OnHosted, OnLocal;
    char HostToLocal[1000], LocalToHost[1000];
    struct in_addr  PeerAddr;
    char           *PeerName;
};

struct _Accept {
    Accept        Previous, Next;
    WantIo        Id;
    int           Local, LocalPort, HostPort, Log;
    int		  LocalProtocol, HostProtocol;
    struct _Pass  Passes;
    struct sockaddr_in Template;
    char         *HostSite;
};

STATIC struct _Accept AcceptBase = { &AcceptBase, &AcceptBase };

STATIC const char *strherrno(void)
{
    if (0 < h_errno && h_errno < h_nerr) return h_errlist[h_errno];
    else if (h_errno == 0)               return NoError;
    else                                 return InvalidError;
}

STATIC const char *ProtoName(int Protocol)
{
    char Fail[80];

    switch(Protocol) {
      case SOCK_STREAM: return "tcp";
      case SOCK_DGRAM:  return "udp";
    }
    sprintf(Fail, "invalid protocol number %d", Protocol);
    Raise1(AssertException, Fail);
    return NULL;
}

STATIC void FreePass(Pass pass)
{
    Accept acc;
    struct timeval now;
    unsigned long millisecs;

    gettimeofday(&now, NULL);
    millisecs = 1000 * (now.tv_sec - pass->Start.tv_sec) +
        (1000500 + now.tv_usec - pass->Start.tv_usec) / 1000 - 1000;
    if (!millisecs) millisecs = 1; /* Avoid division by zero */

    acc = pass->Parent;
    fprintf(stderr,
            "Closing connection on %d: %s(%s) %d translated to port %d\n",
            acc->Local, acc->HostSite, inet_ntoa(acc->Template.sin_addr),
            acc->HostPort, acc->LocalPort);
    fprintf(stderr,
            "  Connection socket %d (host) to %d (local): %s %d"
            " (%lu.%03lu s)\n",
            pass->Hosted, pass->Local, pass->PeerName, pass->PeerPort,
            millisecs/1000, millisecs%1000);
    fprintf(stderr, "           HostToLocal: "
            "read %08lu written %08lu",
            pass->HostToLocalRead, pass->HostToLocalWritten);
    fprintf(stderr, " (%lu.%03lu Kbytes/sec)\n",
            pass->HostToLocalRead/millisecs,
            (2000*(pass->HostToLocalRead-
                   pass->HostToLocalRead/millisecs*millisecs)+millisecs)/
            (2*millisecs));
    fprintf(stderr, "           LocalToHost: "
            "read %08lu written %08lu",
                    pass->LocalToHostRead, pass->LocalToHostWritten);
    fprintf(stderr, " (%lu.%03lu Kbytes/sec)\n",
            pass->LocalToHostRead/millisecs,
            (2000*(pass->LocalToHostRead-
                   pass->LocalToHostRead/millisecs*millisecs)+millisecs)/
            (2*millisecs));
    if (pass->OnLocal)  RemoveInput(pass->OnLocal);
    if (pass->OnHosted) RemoveInput(pass->OnHosted);
    if (pass->Hosted >= 0) close(pass->Hosted);
    if (pass->Local  >= 0) close(pass->Local);
    if (pass->Log    >= 0) close(pass->Log);
    myfree(pass->PeerName);
    pass->Previous->Next = pass->Next;
    pass->Next->Previous = pass->Previous;
    myfree(pass);
}

STATIC void FreeAcc(Accept acc)
{
    fprintf(stderr,
            "Closing acceptance on %d: %s(%s) %d translated to port %d\n",
            acc->Local, acc->HostSite, inet_ntoa(acc->Template.sin_addr),
            acc->HostPort, acc->LocalPort);
    while (acc->Passes.Next != &acc->Passes) FreePass(acc->Passes.Next);
    RemoveInput(acc->Id);
    close(acc->Local);
    myfree(acc->HostSite);
    if (acc->Log >= 0) close(acc->Log);
    acc->Previous->Next = acc->Next;
    acc->Next->Previous = acc->Previous;
    myfree(acc);
}

STATIC void GotLocal(WantIo Id, int Fid, void *Closure);
STATIC void GotHosted(WantIo Id, int Fid, void *Closure);

STATIC void WriteLocal(WantIo Id, int Fid, void *Closure)
{
    int rc;
    Pass pass;

    pass = Closure;
    rc = write(Fid, pass->HostToLocal+pass->HostToLocalOffset,
               pass->HostToLocalLength);
    if (rc <= 0)
        if (rc == 0) FreePass(pass);
        else fprintf(stderr, "Got error on output: %d, %s\n", rc, strerrno());
    else {
        pass->HostToLocalLength -= rc;
        pass->HostToLocalOffset += rc;
        pass->HostToLocalWritten += rc;
        if (pass->HostToLocalLength == 0) {
            RemoveInput(Id);
            pass->OnHosted = 0;
            pass->OnHosted =
                AddInput(pass->Hosted, IOREAD, GotHosted, Closure);
        }
/*      fprintf(stderr, "wrote output on %d: '%.*s'\n",
                Fid, rc, pass->HostToLocal); */
    }
}

STATIC void WriteHosted(WantIo Id, int Fid, void *Closure)
{
    int rc;
    Pass pass;

    pass = Closure;
    rc = write(Fid, pass->LocalToHost+pass->LocalToHostOffset,
               pass->LocalToHostLength);
    if (rc <= 0)
        if (rc == 0) FreePass(pass);
        else fprintf(stderr, "Got error on output: %d, %s\n", rc, strerrno());
    else {
        pass->LocalToHostLength  -= rc;
        pass->LocalToHostOffset  += rc;
        pass->LocalToHostWritten += rc;
        if (pass->LocalToHostLength == 0) {
            RemoveInput(Id);
            pass->OnLocal = 0;
            pass->OnLocal =
                AddInput(pass->Local, IOREAD, GotLocal, Closure);
        }
/*        fprintf(stderr, "wrote output on %d: '%.*s'\n", 
                  Fid, rc, pass->LocalToHost); */
    }
}

STATIC void GotLocal(WantIo Id, int Fid, void *Closure)
{
    int rc;
    Pass pass;

    pass = Closure;
    rc = read(Fid, pass->LocalToHost, sizeof(pass->LocalToHost));
    if (rc <= 0)
        if (rc == 0) FreePass(pass);
        else switch(errno) {
	  default:
	    fprintf(stderr, "Got error on input: %d, %s\n", rc, strerrno());
	    break;
#ifdef ECONNRESET
          case ECONNRESET:
#endif /* ECONNRESET */
#ifdef ETIMEDOUT
          case ETIMEDOUT:
#endif /* ETIMEDOUT */
#ifdef ECONNREFUSED
          case ECONNREFUSED:
#endif /* ECONNREFUSED */
#ifdef EHOSTDOWN
          case EHOSTDOWN:
#endif /* EHOSTDOWN */
#ifdef EHOSTUNREACH
          case EHOSTUNREACH:
#endif /* EHOSTUNREACH */
	    fprintf(stderr, "Got bad error on input: %d, %s\n", rc, strerrno());
	    FreePass(pass);
	    break;
	}
    else {
	if (pass->Log >= 0) {
	    write(pass->Log, "Local-> ", 8);
	    write(pass->Log, pass->LocalToHost, (size_t) rc);
	    if (write(pass->Log, "\n", 1) < 0) {
		fprintf(stderr, "Could not write to log file: %s. Closing\n",
		        strerrno());
		close(pass->Log);
		pass->Log = -1;
	    }
	}
        pass->LocalToHostLength = rc;
        pass->LocalToHostRead += rc;
        RemoveInput(Id);
        pass->OnLocal = 0;
        pass->OnLocal = AddInput(pass->Hosted, IOWRITE, WriteHosted, Closure);
        pass->LocalToHostOffset = 0;
/*      fprintf(stderr, "Read input on %d: '%.*s'\n",
                Fid, rc, pass->LocalToHost); */
    }
}

STATIC void GotHosted(WantIo Id, int Fid, void *Closure)
{
    int rc;
    Pass pass;

    pass = Closure;
    rc = read(Fid, pass->HostToLocal, sizeof(pass->HostToLocal));
    if (rc <= 0)
        if (rc == 0) FreePass(pass);
        else switch(errno) {
	  default:
	    fprintf(stderr, "Got error on input: %d, %s\n", rc, strerrno());
	    break;
#ifdef ETIMEDOUT
          case ETIMEDOUT:
#endif /* ETIMEDOUT */
#ifdef ECONNREFUSED
          case ECONNREFUSED:
#endif /* ECONNREFUSED */
#ifdef EHOSTDOWN
          case EHOSTDOWN:
#endif /* EHOSTDOWN */
#ifdef EHOSTUNREACH
          case EHOSTUNREACH:
#endif /* EHOSTUNREACH */
	    fprintf(stderr, "Got bad error on input: %d, %s\n", rc, strerrno());
	    FreePass(pass);
	    break;
	}
    else {
	if (pass->Log >= 0) {
	    write(pass->Log, "Host -> ", 8);
	    write(pass->Log, pass->HostToLocal, (size_t) rc);
	    if (write(pass->Log, "\n", 1) < 0) {
		fprintf(stderr, "Could not write to log file: %s. Closing\n",
		        strerrno());
		close(pass->Log);
		pass->Log = -1;
	    }
	}
        pass->HostToLocalLength = rc;
        pass->HostToLocalRead += rc;
        RemoveInput(Id);
        pass->OnHosted = 0;
        pass->OnHosted = AddInput(pass->Local, IOWRITE, WriteLocal, Closure);
        pass->HostToLocalOffset = 0;
/*      fprintf(stderr, "Read input on %d: '%.*s'\n",
                Fid, rc, pass->HostToLocal); */
    }
}

STATIC void DoneConnect(WantIo Id, int Fid, void *Closure)
{
    Pass pass;

    pass = Closure;
    fprintf(stderr, "Done connect\n");
    RemoveInput(Id);
    pass->OnHosted = 0;
    pass->OnHosted = AddInput(pass->Hosted, IOREAD, GotHosted, Closure);
}

STATIC void GotDgram(WantIo Id, int Fid, void *Closure)
{
    fprintf(stderr, "I just received a DGRAM\n");
}

STATIC void GotAttempt(WantIo Id, int Fid, void *Closure)
{
    int local, hosted, len;
    size_t length;
    unsigned long addr;
    Accept acc;
    Pass   pass;
    struct sockaddr_in peer;
    struct hostent *hp;
    char **hosts, *host;

    acc = Closure;
    fprintf(stderr, "Got attempt");

    len = sizeof(peer);
    local = accept(acc->Local, (struct sockaddr *) &peer, (void*)&len);
    if (local < 0) Raise(AcceptError);
    WITH_HANDLING {
        hosted = socket(AF_INET, SOCK_STREAM, 0);
        if (hosted < 0) Raise(SockError);
        WITH_HANDLING {
            if (fcntl(hosted, F_SETFL, O_NONBLOCK) < 0) Raise(FcntlError);
            pass = mynew(struct _Pass);
            WITH_HANDLING {
                pass->Next     = &acc->Passes;
                pass->Previous =  acc->Passes.Previous;
                pass->Previous->Next = pass->Next->Previous = pass;
                pass->Parent   = acc;
                pass->Hosted   = hosted;
                pass->Local    = local;
		pass->Log      = acc->Log; acc->Log = -1;
                pass->HostToLocalLength = pass->LocalToHostLength = 0;
                pass->HostToLocalRead = pass->HostToLocalWritten = 0;
                pass->LocalToHostRead = pass->LocalToHostWritten = 0;
                pass->OnHosted = pass->OnLocal = 0;
                pass->PeerPort = ntohs(peer.sin_port);
                /* Next two lines are for alpha. Do they work ? --Ton */
                memcpy(&addr, &peer.sin_addr, sizeof(addr)); /* align long */
                pass->PeerAddr.s_addr = addr;
                hp = gethostbyaddr((char *) &pass->PeerAddr.s_addr,
                                   sizeof(pass->PeerAddr.s_addr), AF_INET);
                if (hp) {
                    if (!strchr(hp->h_name, '.')) {
                        length = strlen(hp->h_name);
                        for (hosts = hp->h_aliases; (host = *hosts) != NULL;
                             hosts++)
                            if (strncmp(host, hp->h_name, length) == 0 &&
                                host[length] == '.') {
                                pass->PeerName = mystrdup(host);
                                goto done;
                            }
                    }
                    pass->PeerName = mystrdup(hp->h_name);
                } else pass->PeerName = mystrdup(inet_ntoa(pass->PeerAddr));
              done:
                WITH_HANDLING {
                    if (connect(hosted, (struct sockaddr *) &acc->Template,
                                sizeof(acc->Template)) >= 0)
                        pass->OnHosted = AddInput(hosted, IOREAD,
                                                  GotHosted, pass);
                    else if (errno != EINPROGRESS) Raise(ConnectError);
                    else pass->OnHosted =
                        AddInput(hosted, IOWRITE, DoneConnect, pass);
                    pass->OnLocal = AddInput(local, IOREAD, GotLocal, pass);
                    gettimeofday(&pass->Start, NULL);
                    fprintf(stderr, " from %s(%s) %d\n",
                            pass->PeerName, inet_ntoa(pass->PeerAddr),
                            pass->PeerPort);
                } ON_EXCEPTION {
                    putc('\n', stderr);
                    myfree(pass->PeerName);
                } END_HANDLING;
            } ON_EXCEPTION {
                pass->Previous->Next = pass->Next;
                pass->Next->Previous = pass->Previous;
                myfree(pass);
            } END_HANDLING;
        } ON_EXCEPTION {
            close(hosted);
        } END_HANDLING;
    } ON_EXCEPTION {
        close(local);
        ActionException(theException);
        CleanupException(theException);
        ClearException();
    } END_HANDLING;
}

STATIC int ReUse;

/* supposes that the name survives raising (exists until catch or
   up to end of program)
   Returns the port in host byte order */
STATIC unsigned short PortFromName(const char *Name)
{
    char *ptr;
    int   Port;
    struct servent *sv;

    Port = strtol(Name, &ptr, 0);
    if (ptr == Name) {
        sv = getservbyname(Name, "tcp");
        if (!sv) Raise2(GetServiceError, Name, "of type tcp");
        Port = ntohs(sv->s_port);
    }
    return Port;
}

STATIC void Help(int argc, char **argv);

STATIC void Connect(int argc, char **argv)
{
    struct sockaddr_in server;
    struct hostent    *hp;
    int                local, length;
    unsigned long      addr;
    Accept             acc;
    const char        *Site;
    int                Port, LocalPort;
    char              *ptr;
    char	      *type;

    type = "tt";
    while (argc > 1 && argv[1][0] == '-') {
        type = &argv[1][1];
        argc--;
        argv++;
    }
    if ((type[0] != 't' && type[0] != 'u') || (type[1] != 't' && type[1] != 'u') ||
        type[2] != 0) Raise2(SyntaxError, type, "is not a valid protocolpair");

    if (argc < 3 || argc > 4) Raise(SyntaxError);
    Site = argv[1];
    Port = PortFromName(argv[2]);
    if (argc == 4) LocalPort = PortFromName(argv[3]);
    else LocalPort = 0;

    acc = mynew(struct _Accept);
    WITH_HANDLING {
        acc->LocalProtocol = type[1] == 't' ? SOCK_STREAM : SOCK_DGRAM;
        acc->HostProtocol  = type[0] == 't' ? SOCK_STREAM : SOCK_DGRAM;
        local = socket(AF_INET, acc->LocalProtocol, 0);
        if (local < 0)  Raise(SockError);
        if (ReUse)
            if (setsockopt(local, SOL_SOCKET, SO_REUSEADDR,
                           (char *) &ReUse, sizeof(ReUse)))
                fprintf(stderr,
                        "%s: Could not set socket to reuse address: %s\n",
                        strerror(errno));
        WITH_HANDLING {
            /* Prepare description of remote host */
            addr = inet_addr(Site);
            /* We really expect (unsigned long) -1, but alpha returns
               only part. So we should use INADDR_NONE, but often excluded
               from netinet/in.h so just use INADDR_BROADCAST which is the
               same */
       
            if (addr == (unsigned long) INADDR_BROADCAST) {
                hp = gethostbyname(Site);
                if (!hp) Raise3(HostError, Site, "):", strherrno());
                memcpy(&server.sin_addr, hp->h_addr, (size_t) hp->h_length);
                /* Portability trick for alpha (long = 8 bytes) 
                   Hope it doesn't break on other long=8 machines */
                addr = (unsigned long) server.sin_addr.s_addr;
            }
            memcpy(&acc->Template.sin_addr, &addr, sizeof(addr));
            acc->Template.sin_family = AF_INET;
            acc->Template.sin_port = htons(Port);

            /* Name socket using wildcards */
            server.sin_family = AF_INET;
            server.sin_addr.s_addr = INADDR_ANY;
            server.sin_port = htons(LocalPort);
            if (bind(local, (struct sockaddr *) &server, sizeof(server)))
                Raise(BindError);

            length = sizeof(server);
            if (getsockname(local, (struct sockaddr *) &server,(void*)&length))
                Raise(GetNameError);

	    acc->Log   = -1;
            acc->Local = local;
            acc->LocalPort = ntohs(server.sin_port);
            acc->HostPort  = Port;
            acc->Passes.Next = acc->Passes.Previous = &acc->Passes;
            acc->HostSite  = mystrdup(Site);
            WITH_HANDLING {
                if (acc->LocalProtocol == SOCK_STREAM) {
                    if (listen(local, 5) < 0) Raise(ListenError);
                    acc->Id	= AddInput(local, IOREAD, GotAttempt, acc);
                } else
                    acc->Id	= AddInput(local, IOREAD, GotDgram, acc);

                acc->Next     = &AcceptBase;
                acc->Previous =  AcceptBase.Previous;
                acc->Previous->Next = acc->Next->Previous = acc;
            } ON_EXCEPTION {
                myfree(acc->HostSite);
            } END_HANDLING;
        } ON_EXCEPTION {
            close(local);
        } END_HANDLING;
    } ON_EXCEPTION {
        myfree(acc);
        ActionException(theException);
        CleanupException(theException);
        ClearException();
    } END_HANDLING;
}

STATIC void Kill(int argc, char **argv)
{
    int    sock;
    Accept acc;
    Pass   pass;
    char  *ptr;

    if (argc != 2) Raise(SyntaxError);
    sock = PortFromName(argv[1]);

    for (acc=AcceptBase.Next; acc != &AcceptBase; acc = acc->Next)
        if (acc->Local == sock) {
            FreeAcc(acc);
            return;
        } else
            for (pass = acc->Passes.Next; pass != &acc->Passes;
                 pass = pass->Next)
                if (pass->Hosted == sock || pass->Local == sock) {
                    FreePass(pass);
                    return;
                }
    fprintf(stderr, "Could not find socket %d\n", sock);
    Raise(SilentError);
}

STATIC void Log(int argc, char **argv)
{
    Accept acc;
    Pass   pass;
    int    fd, sock;
    char  *ptr;
    const char *Name;

    if (argc != 3) Raise(SyntaxError);
    sock = PortFromName(argv[1]);
    /* if (*ptr || ptr == argv[1]) Raise(SyntaxError); */
    Name = argv[2];

    for (acc=AcceptBase.Next; acc != &AcceptBase; acc = acc->Next)
        if (acc->Local == sock) {
            fd = creat(Name, 0644);
	    if (fd >= 0) {
	        acc->Log = fd;
	        return;
	    }
	    fprintf(stderr, "Could not create %s: %s\n", Name, strerrno());
            Raise(SilentError);
        } else
            for (pass = acc->Passes.Next;
                 pass != &acc->Passes;
                 pass = pass->Next)
                if (pass->Hosted == sock || pass->Local == sock) {
                    fd = creat(Name, 0644);
		    if (fd >= 0) {
		        pass->Log = fd;
		        return;
		    }
		    fprintf(stderr, "Could not create %s: %s\n",
		            Name, strerrno());
                    Raise(SilentError);
                }
}

STATIC void UnLog(int argc, char **argv)
{
    Accept acc;
    Pass   pass;
    int    sock;

    if (argc != 3) Raise(SyntaxError);

    sock = PortFromName(argv[1]);

    for (acc=AcceptBase.Next; acc != &AcceptBase; acc = acc->Next)
        for (pass = acc->Passes.Next; pass != &acc->Passes; pass = pass->Next)
            if (pass->Hosted == sock || pass->Local == sock) {
		if (pass->Log >= 0) {
		    close(pass->Log);
		    pass->Log = -1;
		    return;
		} else
		fprintf(stderr, "Socket %d was not logging.\n", sock);
                Raise(SilentError);
            }
}


STATIC void PrintStatus(int argc, char **argv)
{
    Accept acc;
    Pass   pass;
    struct timeval now;
    unsigned long millisecs;

    gettimeofday(&now, NULL);
    for (acc=AcceptBase.Next; acc != &AcceptBase; acc = acc->Next) {
        fprintf(stderr,
                "Translations for %d: %s(%s) %d (%s) "
                "translated to port %d (%s)\n",
                acc->Local, acc->HostSite, inet_ntoa(acc->Template.sin_addr),
                acc->HostPort, ProtoName(acc->HostProtocol), acc->LocalPort,
                ProtoName(acc->LocalProtocol));
        for (pass = acc->Passes.Next; pass != &acc->Passes;pass = pass->Next) {
            millisecs = 1000 * (now.tv_sec - pass->Start.tv_sec) +
                (1000500 + now.tv_usec - pass->Start.tv_usec) / 1000 - 1000;
            if (!millisecs) millisecs = 1; /* Avoid division by zero */

            fprintf(stderr,
                    "  Connect: socket %d (host) to %d (local): %s(%s) %d"
                    " (%lu.%03lu s)\n",
                    pass->Hosted, pass->Local,
                    pass->PeerName, inet_ntoa(pass->PeerAddr), pass->PeerPort,
                    millisecs/1000, millisecs%1000);
            fprintf(stderr, "           HostToLocal: read %08lu written %08lu",
                    pass->HostToLocalRead, pass->HostToLocalWritten);
            fprintf(stderr, " (%lu.%03lu Kbytes/sec)\n",
                    pass->HostToLocalRead/millisecs,
                    (2000*(pass->HostToLocalRead-
                           pass->HostToLocalRead/millisecs*millisecs)+
                     millisecs)/
                    (2*millisecs));
            fprintf(stderr, "           LocalToHost: read %08lu written %08lu",
                    pass->LocalToHostRead, pass->LocalToHostWritten);
            fprintf(stderr, " (%lu.%03lu Kbytes/sec)\n",
                    pass->LocalToHostRead/millisecs,
                    (2000*(pass->LocalToHostRead-
                           pass->LocalToHostRead/millisecs*millisecs)+
                     millisecs)/
                    (2*millisecs));
            fprintf(stderr, "           log: %d\n", pass->Log);
        }
    }
}

typedef void Fun(int argc, char **argv);

typedef struct {
    char *Name;
    Fun  *Command;
    char *Help;
} CommandEntry;

STATIC CommandEntry CommandEntries[] = {
    { "status",  PrintStatus, "status" },
    { "connect", Connect,     "connect [-uu|-tu|-ut|tt] <site> <port> [<localport>]" },
    { "kill",    Kill,        "kill <socket>" },
    { "log",     Log,         "log <socket> <file>" },
    { "unlog",   UnLog,       "unlog <socket>" },
    { "help",    Help,        "help" },
};

STATIC const int NrCommandEntries =
    sizeof(CommandEntries) / sizeof(*CommandEntries);

STATIC void Help(int argc, char **argv)
{
    int i, j;

    if (argc <= 1) {
        fprintf(stderr, "Valid command are:\n");
        for (i=0; i<NrCommandEntries; i++)
            fprintf(stderr, "\t%s\n", CommandEntries[i].Help);
    } else {
        for (j=1; j<argc; j++) {
            for (i=0; i<NrCommandEntries; i++)
                if (strcmp(argv[j], CommandEntries[i].Name) == 0) {
                    fprintf(stderr, "Syntax: %s\n", CommandEntries[i].Help);
                    goto found;
                }
            fputs("The command ", stderr);
            fputs(argv[j], stderr);
            fputs(" does not exist", stderr);
            Raise(SyntaxError);
          found:;
        }
    }
}

#define EXTEND 20

/* Very naive interpreter. Replace by something better --Ton */
STATIC void DoCommand(char *Line, size_t length)
{
    char        *ptr, *end;
    int          i, argc, margc;
    char       **argv, **nargv;
    STATIC char *Err[] = { "help", "kill", 0 };
    
    margc = EXTEND;
    argv = NULL;
    argv = mynews(char *, margc);
    WITH_UNWIND {
        ptr = Line;
        end = strchr(Line, 0);
        argc = 0;
        for (;;) {
            if (argc>=margc) {
                nargv = mynews(char *, margc+EXTEND);
                memcpy(nargv, argv, sizeof(char *) * margc);
                myfree(argv);
                nargv = argv;
                margc += EXTEND;
            }
            while(isspace(*ptr)) ptr++;
            if (ptr == end) break;
            argv[argc++] = ptr;
            *end = ' ';
            while(!isspace(*ptr)) ptr++;
            *ptr++ = 0;
            if (ptr > end) break;
            *end = 0;
        }
        if (!argc) goto done;
        WITH_HANDLING {
            for (i=0; i<NrCommandEntries; i++)
                if (strcmp(argv[0], CommandEntries[i].Name) == 0) {
                    (*CommandEntries[i].Command)(argc, argv);
                    goto done;
                }
            fputs("Unknown command ", stderr);
            fputs(argv[0], stderr);
            fputc('\n', stderr);
            Help(0, NULL);
            Raise(SilentError);
          done:
            printf("0\n");
        } ON_EXCEPTION {
            if (ExceptionP(SyntaxError)) {
                Err[1] = argv[0];
                Help(2, Err);
                ClearException();
            } else if (ExceptionP(GetServiceError)) {
                fputs("Could not get service ", stderr);
                fputs(ExceptionItem[0], stderr);
                fputs(" of  type ", stderr);
                fputs(ExceptionItem[1], stderr);
                fputc('\n', stderr);
                ClearException();
            } else if (ExceptionP(SilentError))
                ClearException();
            printf("1\n");
        } END_HANDLING;
    } ON_UNWIND {
        myfree(argv);
    } END_UNWIND;
}

STATIC void GotStdin(WantIo Id, int Fid, void *Closure)
{
    char Buffer[200], *ptr, *Base, *End;
    int rc;

    rc = read(Fid, Buffer+ComPos, sizeof(Buffer)-ComPos);
    if (rc <= 0)
        if (rc == 0) Raise(InputLost);
        else fprintf(stderr, "Error on stdin: %s\n", strerrno());
    else {
        ptr = memchr(Buffer+ComPos, '\n', (size_t) rc);
        ComPos += rc;
        if (ptr) {
            Base = Buffer;
            End  = Buffer+ComPos;
            do {
                if (IgnoreIn) IgnoreIn = 0;
                else {
                     *ptr = 0;
                    DoCommand(Base, (size_t) (ptr-Base));
                }
                Base = ++ptr;
            } while ((ptr = memchr(Base, '\n', (size_t) (End-Base))) != NULL);
            ComPos = End-Base;
            memmove(Buffer, Base, ComPos);
        } else if (IgnoreIn) ComPos = 0;
        if (ComPos == sizeof(Buffer)) {
            fprintf(stderr, "Command overflow. Ignoring upto next new line\n");
            ComPos = 0;
            IgnoreIn = 1;
        }
    }
}

STATIC void Syntax(FILE *fp)
{
    fprintf(fp,
            "Usage:\t%s [-h] [-V] [-R]\n"
            "\t%s is a program waits for a connection on a given socket.\n"
            "\tIf one arrives, it make a connection with a given site,\n"
            "\tand passes all traffic that arrives on one to the other.\n"
            "\toptions:\n"
            "\t\t-h:\tGive this help\n"
            "\t\t-V:\tVersion (" VERSION ")\n"
            "\t\t-R:\tWhen opening a socket, force reuse of the address\n",
            ExceptionProgram, ExceptionProgram);
}

extern char *optarg;
extern int   optind;

int main(int argc, char **argv)
{
    int Leave, ch, options;

    ExceptionProgram = argv[0];

    ComPos   = 0;
    IgnoreIn = 0;
    ReUse    = 0;
    Leave    = 0;

    for (options=0; (ch = getopt(argc, (void *) argv,
                                 "RVh")) != EOF; options++)
        switch(ch) {
          case 'R':
            ReUse = 1;
            break;
          case 'V':
            Leave = 1;
            puts(VERSION);
            break;
          case 'h':
            Syntax(stdout);
            Leave = 1;
            break;
          default:
            Syntax(stderr);
            return 1;
        }
    if (Leave) return 0;

    if (optind != argc)
        fprintf(stderr,
                "Extra arguments ignored. This program needs no arguments\n");

    InitInput();
    WITH_UNWIND {
        AddInput(0, IOREAD, GotStdin, NULL);
        if (ch == ch) for (;;) Wait();
    } ON_UNWIND {
        CleanInput();
        while (AcceptBase.Next != &AcceptBase) FreeAcc(AcceptBase.Next);
    } END_UNWIND;
    return 0;
}
