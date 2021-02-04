#include <stddef.h>
#include <except.h>

extern Exception MemAddressNotFound;
typedef struct _memchunk *MemChunk;

typedef struct {
    const char *File;
    char       *Name;
    int         Line, NameLength;
    MemChunk    Begin, End, Extra;
    size_t      More;
    char       *First1, *From, *Last1, *First2, *To, *Last2;
    char        TheName[1];
} *MemQueue;

/* You can use negative lengths if you know what you are doing */
#define MemAllocAfter(m, len)                                      \
    (((m)->To += (len)) > (m)->Last2 ?  _MemAllocAfter(m, len) \
                                     : (void *) (m)->To - (len))

#define MemAllocBefore(m, len)                                         \
    (((m)->From -= (len)) < (m)->First1 ?  _MemAllocBefore((m), (len)) \
                                       : (void *) (m)->From)

/* You can use this to allocate memory if you know what you are doing */
#define MemFreeAfter(m, pos)                                         \
    ((m)->First2 <= (pos) && (pos) <= (m)->Last2 ? (m)->To = (pos)   \
                                                 : _MemFreeAfter(m, pos))

#define MemFreeBefore(m, pos)                                        \
    ((m)->First1 <= (pos) && (pos) <= (m)->Last1 ? (m)->From = (pos) \
                                                 : _MemFreeBefore(m, pos))

#define AllocQueue(More, Name, NameLength) \
    _AllocQueue(More, Name, __FILE__, __LINE__)
#define 
/* These do NOT support negative lengths */
extern void *_MemAllocAfter(MemQueue m, size_t len);
extern void *_MemAllocBefore(MemQueue m, size_t len);
/* These do NOT support a position out of range */
extern void  _MemFreeAfter(MemQueue m, void *pos);
extern void  _MemFreeBefore(MemQueue m, void *pos);

extern void FreeQueue(MemQueue queue);
extern MemQueue _AllocQueue(size_t More, const char *Name, int NameLength,
                            const char *File, int Line);

