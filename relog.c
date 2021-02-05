#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <except.h>

#define EEA ErrnoExceptionAction
Exception Arguments    = { "program has exactly one argument: the filename" };
Exception Open         = { "Could not open file",         0, EEA };
Exception Error        = { "Error occured",               0, EEA };
Exception ReadError    = { "Error occured",               0, EEA };
Exception SockError    = { "Could not create socket",     0, EEA };
Exception BindError    = { "Could not bind socket",       0, EEA };
Exception GetNameError = { "Could not get socket name",   0, EEA };
Exception ListenError  = { "Could not set backlog",       0, EEA };
Exception AcceptError  = { "Could not accept connection", 0, EEA };
#undef EEA

#ifndef HAVE_NO_UNISTD_H
# include <unistd.h>
#endif /* HAVE_NO_UNISTD_H */
#ifndef _POSIX_SOURCE
extern int read( /* int fd,       char *buf, unsigned int n */);
extern int write(/* int fd, const char *buf, unsigned int n */);
#endif /* _POSIX_SOURCE */
extern int close(/* int fd */);

int main(int argc, char **argv)
{
    int   sock, length, msgsock, i;
    struct sockaddr_in server;
    FILE *fp;
    char  Buffer[1000], *ptr;

    ExceptionProgram = argv[0];
    if (argc != 2) Raise(Arguments);

    /* Create socket */
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) Raise(SockError);
    WITH_UNWIND {
        /* Name socket using wildcards */
        server.sin_family = AF_INET;
        server.sin_addr.s_addr = INADDR_ANY;
        server.sin_port = 0;
        if (bind(sock, (struct sockaddr *) &server, sizeof(server)))
            Raise(BindError);

        /* Find out assigned port number and print it out */
        length = sizeof(server);
        if (getsockname(sock, (struct sockaddr *) &server, (void*)&length))
            Raise(GetNameError);
        printf(isatty(1) ? "Socket has port #%d\n" : "%d",
               ntohs(server.sin_port));
        fclose(stdout);

        /* Start accepting connections */
        if (listen(sock, 5)) Raise(ListenError);
        msgsock = accept(sock, 0, 0);
        if (msgsock == -1) Raise(AcceptError);
        WITH_UNWIND {
            fp = fopen(argv[1], "rb");
            if (!fp) Raise2(Open, argv[1], "for read");
            WITH_UNWIND {
                for (i=1; fgets(Buffer, sizeof(Buffer), fp); i++) {
                    if (memcmp(Buffer, "Change from ", 12) == 0) continue;
                    if (memcmp(Buffer, "Forced from ", 12) == 0) continue;
                    ptr = strchr(Buffer, '*');
                    if (ptr &&
                        ptr[1] == ' ' && ptr[2] == '(' && ptr[4] == ')') {
                        ptr[0] = 0;
                        if (Buffer[0] &&
                            -1 == write(msgsock, Buffer, strlen(Buffer)))
                            Raise1(Error, "while writing to socket");
                        if (memcmp(ptr+6, "Warning", 7)) {
                            fprintf(stderr, "Waiting for %s", ptr+6);
                            fflush(stderr);
                            length = read(msgsock, Buffer, sizeof(Buffer));
                            if (length == -1) Raise(ReadError);
                            Buffer[length] = 0;
                        }
                    } else if (-1 == write(msgsock, Buffer, strlen(Buffer)))
                        Raise1(Error, "while writing to socket");
                }
                if (!feof(fp)) {
                    sprintf(Buffer, "while reading line %d of file %s", i, argv[1]);
                    Raise1(Error, Buffer);
                }
            } ON_UNWIND {
                fclose(fp);
            } END_UNWIND;
        } ON_UNWIND {
            close(msgsock);
        } END_UNWIND;
    } ON_UNWIND {
        close(sock);
    } END_UNWIND;
    return 0;
}
