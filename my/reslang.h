#ifndef _RESLANG_H
# define _RESLANG_H
#include <stdio.h>
#include <stddef.h>

typedef struct _ResParse_ ResParse;

struct _ResParse_ {
    int       NrArgs;
    char     *Name;
    ResParse *Next;
    ResParse *Arg[1];
};

extern void      fprintParse(FILE *fp, const ResParse *Parse);
extern void      FreeParse(ResParse *Parse);
extern ResParse *resParse(const char *Str, size_t Len);
#endif /* _RESLANG_H */
