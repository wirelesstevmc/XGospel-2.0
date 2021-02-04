/*------------------------------------------*/
/* Safe malloc function : Test for NULL     */
/* Also some elementary debugging functions */
/*------------------------------------------*/

#ifndef MYMALLOC_H
#define MYMALLOC_H
#include <stddef.h>

#define CODE    (0x12345678) /*  Undefine if you want no free memory checks */

#ifndef CODE

#define         testmalloc()
#define         mallocstats()
extern char  *mystrdup(const char *Model);
extern char  *mystrndup(const char *Model, size_t bytes);
extern void  *mymalloc(size_t bytes);
extern void  *mycalloc(size_t elems, size_t bytes);
extern void  *myrealloc(void *ptr, size_t bytes);
extern void   myfree(void *ptr);

#else  /* CODE */

#define         testmalloc()            TestMalloc(__FILE__, __LINE__)
#define         mallocstats()           MallocStats(__FILE__, __LINE__)
#define         mystrdup(Model)         myStrdup(Model, __FILE__, __LINE__)
#define         mystrndup(Model, bytes) myStrndup(Model, bytes, __FILE__, __LINE__)
#define         mymalloc(bytes)         myMalloc(bytes, __FILE__, __LINE__)
#define         mycalloc(elems, bytes)  myCalloc(elems, bytes, __FILE__, __LINE__)
#define         myrealloc(ptr, bytes)   myRealloc(ptr, bytes, __FILE__, __LINE__)
#define         myfree(ptr)             myFree(ptr, __FILE__, __LINE__)

extern void   MallocStats(const char *File, int Line);
extern int    TestMalloc(const char *File, int Line);
extern char  *myStrdup(const char *Model, const char *File, int Line);
extern char  *myStrndup(const char *Model, size_t bytes, const char *File, int Line);
extern void  *myMalloc(size_t bytes, const char *File, int Line);
extern void  *myCalloc(size_t elems, size_t bytes, const char *File, int Line);
extern void  *myRealloc(void *ptr, size_t bytes, const char *File, int Line);
extern void   myFree(void *ptr, const char *File, int Line);

#endif /* CODE */

extern void **allocMatrix(size_t size, size_t sizeY, size_t sizeX);
extern void   freeMatrix(void **Matrix, size_t sizeY);

# define mynew(object)     ((object *) mymalloc(sizeof(object)))
# define mynews(object, n) ((object *) mymalloc(sizeof(object) * (n)))
# define myrenews(ptr, object, n)       \
                ((object *) myrealloc(ptr, sizeof(object)*(n)))
# define AllocMatrix(object, y, x)   \
    ((object **) allocMatrix(sizeof(object), y, x))
# define FreeMatrix(matrix, y) freeMatrix((void **) matrix, y)

#endif /* MYMALLOC_H */
