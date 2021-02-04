#include "myxlib.h"
#include "mymalloc.h"
#include "except.h"

/* To get around a problem with the SMT extension on alpha OSF1 */
#if NeedFunctionPrototypes      /* prototypes require event type definitions */
# define NEED_EVENTS
#endif
#include <X11/Xlibint.h>

/****************************/
/* Special context function */
/****************************/

#define MAGICCONTEXT  13  /* Some number to get interesting bits at the end */

static Exception NoContext = { "Could not find given context-name pair" };

MyContext MyAllocContext(void)
{
    Display *dpy;

    dpy = mynew(Display);
    WITH_HANDLING {
        dpy->free_funcs = mynew(struct _XFreeFuncs);
        dpy->free_funcs->context_db = 0;
        dpy->context_db = NULL;
    } ON_EXCEPTION {
        myfree(dpy);
    } END_HANDLING;
    return (MyContext) dpy;
}

void MyFreeContext(MyContext Context)
{
    Display *dpy;

    dpy = (Display *) Context;
    if (dpy->free_funcs->context_db) (*dpy->free_funcs->context_db)(dpy);
    myfree(dpy->free_funcs);
    myfree(dpy);
}

/* XTHREATS really, but who needs them ? */
/*#define XDREADS */

#ifdef XDREADS
# include "locking.h"
#else
# ifdef LockDisplay
#  undef LockDisplay
# endif /* LockDisplay */
# ifdef UnlockDisplay
#  undef UnlockDisplay
# endif /* UnlockDisplay */
# define LockDisplay(display)
# define UnlockDisplay(display)
#endif

#define INITHASHMASK 63 /* Number of entries originally in the hash table. */

typedef struct _TableEntryRec {	/* Stores one entry. */
    XID 			rid;
    XContext			context;
    XPointer			data;
    struct _TableEntryRec	*next;
} TableEntryRec, *TableEntry;

typedef struct _XContextDB {	/* Stores hash table for one display. */
    TableEntry *table;		/* Pointer to array of hash entries. */
    int mask;			/* Current size of hash table minus 1. */
    int numentries;		/* Number of entries currently in table. */
#ifdef XDREADS
    LockInfoRec linfo;
#endif
} DBRec, *DB;

#define Hash(db,rid,context) \
    (db)->table[(((rid) << 1) + context) & (db)->mask]

static void MyResizeTable(DB db)
{
    TableEntry *otable;
    register TableEntry entry, next, *pold, *head;
    register int i, j;

    otable = db->table;
    for (i = INITHASHMASK+1; (i + i) < db->numentries; )
	i += i;
    db->table = (TableEntry *) Xcalloc((unsigned)i, sizeof(TableEntry));
    if (!db->table) {
	db->table = otable;
	return;
    }
    j = db->mask + 1;
    db->mask = i - 1;
    for (pold = otable ; --j >= 0; pold++) {
	for (entry = *pold; entry; entry = next) {
	    next = entry->next;
	    head = &Hash(db, entry->rid, entry->context);
	    entry->next = *head;
	    *head = entry;
	}
    }
    Xfree((char *) otable);
}

static void _MyXFreeContextDB(Display *display)
{
    register DB db;
    register int i;
    register TableEntry *pentry, entry, next;

    db = display->context_db;
    if (db) {
	for (i = db->mask + 1, pentry = db->table ; --i >= 0; pentry++) {
	    for (entry = *pentry; entry; entry = next) {
		next = entry->next;
		Xfree((char *)entry);
	    }
	}
	Xfree((char *) db->table);
#ifdef XDREADS
	_XFreeMutex(&db->linfo);
#endif /* XDREADS */
	Xfree((char *) db);
    }
}

static int MyXSaveContext(Display *display, XID rid, XContext context,
                          char* data)
{
    DB *pdb;
    register DB db;
    TableEntry *head;
    register TableEntry entry;

    {
	LockDisplay(display);
	pdb = &display->context_db;
	db = *pdb;
	UnlockDisplay(display);
    }
    if (!db) {
	db = (DB) Xmalloc(sizeof(DBRec));
	if (!db)
	    return XCNOMEM;
	db->mask = INITHASHMASK;
	db->table = (TableEntry *)Xcalloc(db->mask + 1, sizeof(TableEntry));
	if (!db->table) {
	    Xfree((char *)db);
	    return XCNOMEM;
	}
	db->numentries = 0;
#ifdef XDREADS
	_XCreateMutex(&db->linfo);
#endif /* XDREADS */
	{
	    LockDisplay(display);
	    *pdb = db;
	    display->free_funcs->context_db = _MyXFreeContextDB;
	    UnlockDisplay(display);
	}
    }
#ifdef XDREADS
    _XLockMutex(&db->linfo);
#endif /* XDREADS */
    head = &Hash(db, rid, context);
#ifdef XDREADS
    _XUnlockMutex(&db->linfo);
#endif /* XDREADS */
    for (entry = *head; entry; entry = entry->next) {
	if (entry->rid == rid && entry->context == context) {
	    entry->data = (XPointer)data;
	    return 0;
	}
    }
    entry = (TableEntry) Xmalloc(sizeof(TableEntryRec));
    if (!entry)
	return XCNOMEM;
    entry->rid = rid;
    entry->context = context;
    entry->data = (XPointer)data;
    entry->next = *head;
    *head = entry;
#ifdef XDREADS
    _XLockMutex(&db->linfo);
#endif /* XDREADS */
    db->numentries++;
    if (db->numentries > (db->mask << 2))
	MyResizeTable(db);
#ifdef XDREADS
    _XUnlockMutex(&db->linfo);
#endif /* XDREADS */
    return 0;
}

/* Given an XID and context, returns the associated data.  Note that data 
   here is a pointer since it is a return value.  Returns nonzero error code
   if an error has occured, 0 otherwise.  Possible errors are Entry-not-found.
*/

static int MyXFindContext(Display *display, XID rid,
                          XContext context, XPointer *data)
{
    register DB db;
    register TableEntry entry;

    {
	LockDisplay(display);
	db = display->context_db;
	UnlockDisplay(display);
    }
    if (!db)
	return XCNOENT;
#ifdef XDREADS
    _XLockMutex(&db->linfo);
#endif /* XDREADS */
    for (entry = Hash(db, rid, context); entry; entry = entry->next)
    {
	if (entry->rid == rid && entry->context == context) {
	    *data = (XPointer)entry->data;
#ifdef XDREADS
	    _XUnlockMutex(&db->linfo);
#endif /* XDREADS */
	    return 0;
	}
    }
#ifdef XDREADS
    _XUnlockMutex(&db->linfo);
#endif /* XDREADS */
    return XCNOENT;
}

/* Deletes the entry for the given XID and context from the datastructure.
   This returns the same thing that FindContext would have returned if called
   with the same arguments.
*/

static int MyXDeleteContext(Display *display, XID rid, XContext context)
{
    register DB db;
    register TableEntry entry, *prev;

    {
	LockDisplay(display);
	db = display->context_db;
	UnlockDisplay(display);
    }
    if (!db)
	return XCNOENT;
#ifdef XDREADS
    _XLockMutex(&db->linfo);
#endif /* XDREADS */
    for (prev = &Hash(db, rid, context);
	 entry = *prev;
	 prev = &entry->next) {
	if (entry->rid == rid && entry->context == context) {
	    *prev = entry->next;
	    Xfree((char *) entry);
	    db->numentries--;
	    if (db->numentries < db->mask && db->mask > INITHASHMASK)
		MyResizeTable(db);
#ifdef XDREADS
	    _XUnlockMutex(&db->linfo);
#endif /* XDREADS */
	    return 0;
	}
    }
#ifdef XDREADS
    _XUnlockMutex(&db->linfo);
#endif /* XDREADS */
    return XCNOENT;
}

void MySaveContext(MyContext Context, Widget w, XrmQuark Name, XtPointer Value)
{
    if (MyXSaveContext((Display *) Context, ((XID) w)/MAGICCONTEXT,
                       Name, Value))
        Raise1(OutOfMemory, "while saving context");
}

/* rc 0 means found */
int MyFindContext(MyContext Context, Widget w, XrmQuark Name, XtPointer *Value)
{
    XPointer data;
    int      rc;
    
    rc = MyXFindContext((Display *) Context, ((XID) w)/MAGICCONTEXT,
                      Name, &data);
    if (rc == 0) *Value = data;
    return rc;
}

void MyDeleteContext(MyContext Context, Widget w, XrmQuark Name)
{
    if (MyXDeleteContext((Display *) Context, ((XID) w)/MAGICCONTEXT,
                       Name))
        Raise(NoContext);
}

void MyQSaveContext(MyContext Context, XtPointer q,
                    XrmQuark Name, XtPointer Value)
{
    if (MyXSaveContext((Display *) Context, (XID) q, Name, Value))
        Raise1(OutOfMemory, "while saving context");
}

int MyQFindContext(MyContext Context, XtPointer q,
                   XrmQuark Name, XtPointer *Value)
{
    XPointer data;
    int      rc;
    
    rc = MyXFindContext((Display *) Context, (XID) q, Name, &data);
    if (rc == 0) *Value = data;
    return rc;
}

void MyQDeleteContext(MyContext Context, XtPointer q, XrmQuark Name)
{
    if (MyXDeleteContext((Display *) Context, (XID) q, Name))
        Raise(NoContext);
}
