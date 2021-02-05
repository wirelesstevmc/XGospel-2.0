#ifndef GOINTER_H
# define GOINTER_H

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

# ifndef HAVE_NO_STDARG_H
extern void Warning(const char *Comment, ...) __PRINTF1;
# else  /* HAVE_NO_STDARG_H */
extern void Warning();
# endif /* HAVE_NO_STDARG_H */

extern int IgsYYparse(void);

typedef struct _NameVal {
    struct _NameVal *Next, *Previous;
    char            *Name;
    char            *Value;
} NameVal;

typedef struct _NumVal {
    struct _NumVal  *Next, *Previous;
    int              Num;
    char            *Value;
} NumVal;

typedef struct _NameList {
    struct _NameList *Next, *Previous;
    char             *Name;
} NameList;

typedef struct _NameListList {
    struct _NameListList *Next, *Previous;
    NameList             *Names;
} NameListList;

typedef struct _NumNameListList {
    struct _NumNameListList *Next, *Previous;
    int                      Num;
    NameList                *Value;
} NumNameListList;

typedef struct _GameDesc {
    int   Id;
    char *BlackName,    *WhiteName;
    char *BlackName2,   *WhiteName2; /* team game only */
    int   BlackCaptures, WhiteCaptures;
    int   BlackTime,     WhiteTime;
    int   BlackByo,      WhiteByo;
} GameDesc;

typedef struct  ChannelData  ChannelData;
typedef struct _DisputeDesc  DisputeDesc;
typedef struct _BetDesc      BetDesc;

extern NameList *NameListDup(const NameList *Model);
extern void FreeNameList(NameList *Header);
extern void FreeNameValList(NameVal *Header);
extern void FreeNumValList(NumVal *Header);
extern void FreeNameListList(NameListList*Header);
extern void FreeNumNameListList(NumNameListList *Header);
extern void FreeGameDesc(GameDesc *Desc);
# undef __PRINTF1
#endif /* GOINTER_H */
