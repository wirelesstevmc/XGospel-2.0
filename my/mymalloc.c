/*----------------------------------------------*/
/* Safe malloc : Test for NULL                  */
/* Some memory management debugging stuff       */
/*----------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <stddef.h>
#include "except.h"
#include "mymalloc.h"

static char Buffer[100];
#define ULONGSIZE (sizeof(unsigned long)*CHAR_BIT*3/10+1) /* log 2 = 0.30103 */
#define INTSIZE   (sizeof(int)          *CHAR_BIT*3/10+1) /* log 2 = 0.30103 */
#define PRTPTR    "#%p"                   /* Nice outputformat for a pointer */
#define NAMELEN   30                      /* Longest filename we will print  */

#ifndef CODE

void *mymalloc(size_t bytes)
{
    void           *memory;

    memory = malloc(bytes);
    if (!memory && bytes) {
        sprintf(Buffer, "malloc(%lu) failed", (unsigned long) bytes);
        Raise1(OutOfMemory, Buffer);
    }
    return memory;
}

char *mystrdup(const char *Model)
{
    size_t          bytes;
    char           *memory;
    static char ErrorMessage[] = "mystrdup(%.*s) (malloc(%lu)) failed)";

    if (Model == NULL) return NULL;
    bytes = strlen(Model)+1;
    memory = (char *) malloc(bytes);
    if (!memory) {
        sprintf(Buffer, ErrorMessage,
                sizeof(Buffer)-strlen(ErrorMessage)-ULONGSIZE,
                (unsigned long) bytes);
        Raise1(OutOfMemory, Buffer);
    }
    memcpy(memory, Model, bytes);
    return memory;
}

char *mystrndup(const char *Model, size_t Length)
{
    char           *memory;
    static char ErrorMessage[] = "mystrndup(%.*s, %lu) failed)";

    if (Model == NULL) return NULL;
    memory = (char *) malloc(Length+1);
    if (!memory) {
        sprintf(Buffer, ErrorMessage,
                sizeof(Buffer)-strlen(ErrorMessage)-ULONGSIZE,
                (unsigned long) Length);
        Raise1(OutOfMemory, Buffer);
    }
    memcpy(memory, Model, Length);
    memory[Length] = 0;
    return memory;
}

void *mycalloc(size_t elems, size_t bytes)
{
    void           *memory;

    memory = calloc(elems, bytes);
    if (!memory && elems && bytes) {
        sprintf(Buffer, "mycalloc(%lu, %lu) failed", 
                (unsigned long) elems, (unsigned long) bytes);
        Raise1(OutOfMemory, Buffer);
    }
    return memory;
}

void *myrealloc(void *ptr, size_t bytes)
{
    void           *memory;

    if (ptr) memory = realloc(ptr, bytes);
    else     memory = malloc(bytes);
    if (!memory && bytes) {
        sprintf(Buffer, "myrealloc(" PRTPTR ", %lu) failed",
                ptr, (unsigned long) bytes);
        Raise1(OutOfMemory, Buffer);
    }
    return memory;
}

void myfree(void *ptr)
{
# ifndef   __STDC__
    if (!ptr) return;
# endif /* __STDC__ */
    free(ptr);
}
#else /* CODE */

typedef struct test {
     struct test *Next, *Previous;
     const char  *File;
     int          Line;
     char        *EndCode;
     long         Code;       /* Often memory is attacked from below */
     /* This is where the allocated memory goes */
     char         ExtraCode[sizeof(long)]; /* Or an attack from above */
} Test;

static Test Base = { &Base, &Base, __FILE__, __LINE__, Base.ExtraCode, CODE };
#ifdef HAVE_ATEXIT
static int DoAt = 1;
#endif
static int    SpecialMalloc = -1;
static size_t Bytes;
static long   Blocks;

#define MEMORYBUGS 2
#define DUMPCORE   4
#define SHOWLEN   10
    
int TestMalloc(const char *File, int Line)
{
    Test *Here;
    char *Env, *ptr;

    if (SpecialMalloc == -1) {
        Env = getenv("MYMALLOC");
        if (!Env || !*Env ||
            (SpecialMalloc = strtol(Env, &ptr, 0)) == -1 || *ptr) {
            SpecialMalloc = 0;
            return 0;
        }
        if (!SpecialMalloc) return 0;
#ifdef HAVE_ATEXIT
        if (DoAt) {
            atexit(EndFun);
            DoAt = 0;
        }
#endif
    }
    if (SpecialMalloc & MEMORYBUGS) {
        fflush(stdout);
        fprintf(stderr, "File %s, line %d: ", File, Line);
        fflush(stderr);
    }

    for (Here = Base.Next; Here != &Base; Here = Here->Next)
        if (Here->Code != CODE ||
            memcmp(Here->EndCode, &Here->Code, sizeof(long)) ||
            Here->Next->Previous != Here || Here->Previous->Next != Here)
            break;

    if (Here != &Base) {
        if (!(SpecialMalloc & MEMORYBUGS)) {
            fflush(stdout);
            fprintf(stderr, "File %s, line %d: ", File, Line);
        }
        fprintf(stderr, "memory management damaged\n");
        if (SpecialMalloc & DUMPCORE) {
            fprintf(stderr, "Dumping core as requested\n");
	    fflush(stderr);
	    abort();
        }
        fflush(stderr);
    } else if (SpecialMalloc & MEMORYBUGS) {
        fprintf(stderr, "memory management ok\n");
	fflush(stderr);
    }
    return SpecialMalloc;
}

void MallocStats(const char *File, int Line)
{
    Test *Here;
    int   i, n;
    const char *ptr;

    if (SpecialMalloc && TestMalloc(File, Line)) {
        fflush(stdout);
        fprintf(stderr, "%ld bytes allocated in %ld blocks\n",
                (long) Bytes, (long) Blocks);
        for (Here = Base.Next; Here != &Base; Here = Here->Next) {
            fprintf(stderr, "Memory not freed, allocated by %12s, line %4d 0x",
                    Here->File, Here->Line);
            ptr = Here->ExtraCode;
            n = Here->EndCode - ptr;
            if (n > SHOWLEN) n = SHOWLEN;
            for (i=0; i<n; i++) fprintf(stderr, "%02x", ptr[i]);
            fputs(" = '", stderr);
            for (i=0; i<n; i++, ptr++)
                fputc(isprint(*ptr) ? *ptr : '.', stderr);
            fputs("'\n", stderr);
        }
        fflush(stderr);
    }
}

# undef __File__
# undef __Line__
# define __File__       File
# define __Line__       Line

void *myMalloc(size_t bytes, const char *File, int Line)
{
    if (SpecialMalloc && TestMalloc(File, Line)) {
        Test           *memory;

        memory = (Test *) malloc(bytes+sizeof(Test));
        if (!memory) {
            sprintf(Buffer, "myMalloc(%lu, %.*s, %d) (malloc(%lu)) failed",
                    (unsigned long) bytes, NAMELEN, File, Line,
                    (unsigned long) (bytes+sizeof(Test)));
            Raise1(OutOfMemory, Buffer);
        }

        Bytes += bytes;
        Blocks++;
        memory->EndCode = memory->ExtraCode + bytes;
        memory->Code    = CODE;
        memcpy(memory->EndCode, &memory->Code, sizeof(long));
        memory->Next = Base.Next;
        memory->Previous = &Base;
        memory->File = File;
        memory->Line = Line;
        memory->Next->Previous = memory->Previous->Next = memory;

        return memory->ExtraCode;
    } else {
        void *memory;
        
        memory = malloc(bytes);
        if (!memory && bytes) {
            sprintf(Buffer, "malloc(%lu) failed", (unsigned long) bytes);
            Raise1(OutOfMemory, Buffer);
        }
        return memory;
    }
}

char *myStrdup(const char *Model, const char *File, int Line)
{
    size_t bytes;

    if (Model == NULL) return NULL;
    bytes = strlen(Model)+1;
    if (SpecialMalloc && TestMalloc(File, Line)) {
        Test           *memory;
        static char ErrorMessage[] =
            "myStrdup(%.*s, %.*s, %d) (malloc(%lu)) failed)";

        memory = (Test *) malloc(bytes+sizeof(Test));
        if (!memory) {
            sprintf(Buffer, ErrorMessage,
                    sizeof(Buffer)-strlen(ErrorMessage)-ULONGSIZE-NAMELEN-INTSIZE,
                    NAMELEN, File, Line, (unsigned long) (bytes+sizeof(Test)));
            Raise1(OutOfMemory, Buffer);
        }

        Bytes += bytes;
        Blocks++;
        memory->EndCode = memory->ExtraCode + bytes;
        memory->Code    = CODE;
        memcpy(memory->EndCode, &memory->Code, sizeof(long));
        memory->Next = Base.Next;
        memory->Previous = &Base;
        memory->File = File;
        memory->Line = Line;
        memory->Next->Previous = memory->Previous->Next = memory;

        memcpy(memory->ExtraCode, Model, bytes);
        return memory->ExtraCode;
    } else {
        char           *memory;
        static char ErrorMessage[] = "mystrdup(%.*s) (malloc(%lu)) failed)";

        memory = (char *) malloc(bytes);
        if (!memory) {
            sprintf(Buffer, ErrorMessage,
                    sizeof(Buffer)-strlen(ErrorMessage)-ULONGSIZE,
                    (unsigned long) bytes);
            Raise1(OutOfMemory, Buffer);
        }
        memcpy(memory, Model, bytes);
        return memory;
    }
}

char *myStrndup(const char *Model, size_t Length, const char *File, int Line)
{
    if (Model == NULL) return NULL;
    if (SpecialMalloc && TestMalloc(File, Line)) {
        Test           *memory;
        static char ErrorMessage[] =
            "myStrndup(%.*s, %lu, %.*s, %d) (malloc(%lu)) failed)";

        memory = (Test *) malloc(Length+1+sizeof(Test));
        if (!memory) {
            sprintf(Buffer, ErrorMessage,
                    sizeof(Buffer)-strlen(ErrorMessage)-
                    2*ULONGSIZE-NAMELEN-INTSIZE, (unsigned long) Length, 
                    NAMELEN, File, Line, (unsigned long) (Length+1+sizeof(Test)));
            Raise1(OutOfMemory, Buffer);
        }

        Bytes += Length+1;
        Blocks++;
        memory->EndCode = memory->ExtraCode+Length+1;
        memory->Code    = CODE;
        memcpy(memory->EndCode, &memory->Code, sizeof(long));
        memory->Next = Base.Next;
        memory->Previous = &Base;
        memory->File = File;
        memory->Line = Line;
        memory->Next->Previous = memory->Previous->Next = memory;

        memcpy(memory->ExtraCode, Model, Length);
        memory->ExtraCode[Length] = 0;
        return memory->ExtraCode;
    } else {
        char *memory;
        static char ErrorMessage[] = "mystrndup(%.*s, %lu) failed)";

        memory = (char *) malloc(Length+1);
        if (!memory) {
            sprintf(Buffer, ErrorMessage,
                    sizeof(Buffer)-strlen(ErrorMessage)-ULONGSIZE,
                    (unsigned long) Length);
            Raise1(OutOfMemory, Buffer);
        }
        memcpy(memory, Model, Length);
        memory[Length] = 0;
        return memory;
    }
}

void *myCalloc(size_t elems, size_t bytes, const char *File, int Line)
{
    void           *memory;

    if (SpecialMalloc) {
        memory = myMalloc(elems * bytes, File, Line);
        memset(memory, 0, elems * bytes);
    } else {
        memory = calloc(elems, bytes);
        if (!memory && elems && bytes) {
            sprintf(Buffer, "mycalloc(%lu, %lu) failed", 
                    (unsigned long) elems, (unsigned long) bytes);
            Raise1(OutOfMemory, Buffer);
        }
    }
    return memory;
}

void *myRealloc(void *ptr, size_t bytes, const char *File, int Line)
{
    if (SpecialMalloc && TestMalloc(File, Line)) { 
        Test *memory, *Here;

        if (ptr) {
            memory = (Test *) ((char *) ptr - offsetof(Test, ExtraCode[0]));
            for (Here = Base.Next; Here != &Base; Here = Here->Next)
                if (Here == memory) break;
            if (Here == &Base) {
                fflush(stdout);
                fprintf(stderr, "%s %d: "
                        "Attempt to realloc memory you have not allocated\n",
                        File, Line);
                if (SpecialMalloc & DUMPCORE) {
                    fprintf(stderr, "Dumping core as requested\n");
		    fflush(stderr);
		    abort();
                }
                fflush(stderr);
            }
            memory = (Test *) realloc(memory, bytes+sizeof(Test));
        } else memory = (Test *) malloc(bytes+sizeof(Test));
        if (!memory) {
            sprintf(Buffer, "myRealloc(" PRTPTR ", %lu, %.*s, %d) failed",
                    ptr, (unsigned long) bytes, NAMELEN, File, Line);
            Raise1(OutOfMemory, Buffer);
        }

        if (!ptr) {
            Bytes += bytes;
            Blocks++;
            memory->Code    = CODE;
            memory->Next = Base.Next;
            memory->Previous = &Base;
            memory->File = File;
            memory->Line = Line;
        } else Bytes = Bytes + bytes - (memory->EndCode-(char *) ptr);

        memory->EndCode = memory->ExtraCode + bytes;
        memcpy(memory->EndCode, &memory->Code, sizeof(long));
        memory->Next->Previous = memory->Previous->Next = memory;

        return memory->ExtraCode;
    } else {
        void           *memory;

        if (ptr) memory = realloc(ptr, bytes);
        else     memory = malloc(bytes);
        if (!memory && bytes) {
            sprintf(Buffer, "myrealloc(" PRTPTR ", %lu) failed",
                    ptr, (unsigned long) bytes);
            Raise1(OutOfMemory, Buffer);
        }
        return memory;
    }
}

void myFree(void *ptr, const char *File, int Line)
{
    Test *memory, *Here;

    if (SpecialMalloc && TestMalloc(File, Line)) {
        if (ptr == NULL) return;
        memory = (Test *) ((char *) ptr - offsetof(Test, ExtraCode[0]));
        for (Here = Base.Next; Here != &Base; Here = Here->Next)
            if (Here == memory) {
                Bytes -= memory->EndCode - memory->ExtraCode;
                Blocks--;
                memory->Next->Previous = memory->Previous;
                memory->Previous->Next = memory->Next;
                memory->Code = 0x46726565; /* 'Free' */
                free(memory);
                return;
            }
        fflush(stdout);
        fprintf(stderr,
                "%s %d: Attempt to free memory you have not allocated\n",
                File, Line);
        if (SpecialMalloc & DUMPCORE) {
            fprintf(stderr, "Dumping core as requested\n");
	    fflush(stderr);
	    abort();
        }
        fflush(stderr);
    } else {
# ifndef   __STDC__
        if (!ptr) return;
# endif /* __STDC__ */
        free(ptr);
    }
}

# undef __File__
# undef __Line__
# define __File__       __FILE__
# define __Line__       __LINE__
#endif /* CODE */

void **allocMatrix(size_t size, size_t sizeY, size_t sizeX)
{
    void **Matrix, ** volatile M;
    size_t y;

    Matrix = mynews(void *, sizeY);
    size *= sizeX;
    WITH_HANDLING {
        for (y=sizeY, M= Matrix; y>0; y--, M++)
            *M = mymalloc(size);
    } ON_EXCEPTION {
        for (M--; M >= Matrix; M--) myfree(*M);
        myfree(Matrix);
    } END_HANDLING;
    return Matrix;
}

void freeMatrix(void **Matrix, size_t sizeY)
{
    void **M;

    for (M=Matrix; sizeY>0; sizeY--, M++) myfree(*M);
    myfree(Matrix);
}
