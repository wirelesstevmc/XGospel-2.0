#include <MemQueue.h>
#include <mymalloc.h>
#include <string.h>

Exception MemAddressNotFound  = { "Given address was not allocated within "
                                  "this group" };

struct _memchunk {
    MemChunk  Previous, Next;
    char     *First, *From, *To, *Last;
};

void *_MemAllocAfter(MemQueue queue, size_t size)
{
    MemChunk Consider;
    char    *Current,  *Ptr;

    queue->To -= len; /* Recover from MemAllocAfter macro */
    if (queue->From == queue->To) { /* Empty */
        queue->From = Ptr = queue->First1;
        Current = Ptr + size;
        if (Current <= queue->Last2) {
            queue->To = Current;
            return Ptr;
        }
        if (queue->Begin) {
            myfree(queue->Begin);
            queue->Begin = queue->End = NULL;
        }
    } else queue->End->To = queue->To; /* Flush in case we extend */
    Consider = queue->Extra;
    if (Consider) {
        queue->Extra = NULL;
        Ptr  = Consider->First;
        Current = Ptr + size;
        if (Current <= Consider->Last) goto done:
        myfree(Consider);
    }
    Consider = (MemChunk) mymalloc(sizeof(struct _MemChunk)+queue->More+size);
    Ptr = (char *) &Consider[1];
    Consider->First = Ptr;
    Current         = Ptr+size;
    Consider->Last  = Current+queue->More;
  done:
    Consider->Next     = NULL;
    Consider->Previous = queue->End;
    if (queue->End) queue->End->Next = Consider;
    else  {
        queue->Begin  = Consider;
        queue->First1 = queue->From = Ptr;
        queue->Last1  = Consider->Last;
    }
    queue->End = Consider;

    queue->First2 = Consider->From = Ptr;
    queue->To     = Consider->To   = Current;
    queue->Last2  = Consider->Last;

    return Ptr;
}

void *_MemAllocBefore(MemQueue queue, size_t size)
{
    MemChunk Consider;
    char    *Current;

    queue->From += len;         /* Recover from MemAllocBefore macro */
    if (queue->From == queue->To) { /* Empty */
        queue->To = Current = queue->Last2;
        Current -= size;
        if (Current >= queue->First1) {
            queue->From = Current;
            return Current;
        }
        if (queue->End) {
            myfree(queue->End);
            queue->Begin = queue->End = NULL;
        }
    } else queue->Begin->From = queue->From; /* Flush in case we extend */
    Consider = queue->Extra;
    if (Consider) {
        queue->Extra = NULL;
        Current = Consider->Last - size;
        if (Current >= Consider->First) goto done;
        myfree(Consider);
    }
    Consider = (MemChunk) mymalloc(sizeof(struct _MemChunk)+queue->More+size);
    Current = (char *) &Consider[1];
    Consider->First = Current;
    Current += queue->More;
    Consider->Last = Current+size;
  done:
    Consider->Previous  = NULL;
    Consider->Next = queue->Begin;
    if (queue->Begin) Queue->Begin->Previous = Consider;
    else {
        queue->End    = Consider;
        queue->Last2  = queue->To = Consider->Last;
        queue->First2 = Consider->First;
    }
    queue->Begin = Consider;

    queue->First1 = Consider->First;
    queue->From   = Consider->From = Current;
    queue->Last1  = Consider->To   = Consider->Last;
    return Current;
}

MemQueue _AllocQueue(size_t More, const char *Name, int NameLength,
                     const char *File, int Line)
{
    MemQueue queue;
    char    *Current;

    if (!Name) NameLength = 0;
    else if (NameLength < 0) NameLength = strlen(Name);
    queue = mymalloc(sizeof(struct _memqueue)+NameLength);
    if (Name) {
        queue->Name = queue->TheName;
        memcpy(queue->TheName, Name, NameLength);
    } else queue->Name = NULL;
    queue->TheName[NameLength] = 0;
    queue->NameLength = NameLength;
    queue->File = File;
    queue->Line = Line;
    queue->First1 = queue->From = queue->Last1 =
        queue->First2 = queue->To = queue->Last2 = queue->TheName;
    queue->Begin = queue->End = queue->Extra = NULL;
    return queue;
}

void FreeQueue(MemQueue queue)
{
    MemChunk *Here, *Next;

    for (Here = queue->Begin; Here; Here = Next) {
        Next = Here->Next;
        myfree(Here);
    }
    if (queue->Extra) myfree(queue->Extra);
    myfree(queue);
}

void  _MemFreeAfter(MemQueue queue, void *pos)
{
    MemChunk Consider;
    char    *pos;

    block = (char *) pos;
    queue->Begin->From = queue->From; /* In case we get to the beginning */
    if (queue->Extra) myfree(queue->Extra);
    queue->Extra = Consider = queue->End;
    
    do {
        Consider = Consider->Previous;
        if (Consider->From <= block && block <= Consider->To) {
            if (Consider->From == block && Consider->Previous) {
                myfree(queue->Extra);
                queue->Extra = Consider;
                Consider = Consider->Previous;
            }
            queue->End = Consider;
            Consider->Next = NULL;
            queue->First2 = Consider->First;
            queue->To     = block;
            queue->Last2  = Consider->Last;
            return;
        }
        myfree(queue->Extra);
        queue->Extra = Consider;
    } while (Consider);
    if (block != queue->TheName) Raise(MemAddressNotFound);
    Consider = queue->Extra;
    queue->Extra = 0;
    queue->Begin = queue->End = Consider;
    Consider->Next = NULL;
    queue->First1 = queue->First2 = queue->From = queue->To = Consider->First;
    queue->Last1  = queue->Last2  = Consider->Last;
}

void  _MemFreeBefore(MemQueue m, void *pos)
{
}
