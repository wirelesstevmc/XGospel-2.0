#ifndef _TearofMenuP_h
#define _TearofMenuP_h

#include "TearofMenu.h"
#include <X11/IntrinsicP.h>
#include <X11/Xaw/SimpleMenP.h>

typedef struct {
    XtPointer extension;		/* For future needs. */
} TearofMenuClassPart;

typedef struct _TearofMenuClassRec {
  CoreClassPart	          core_class;
  CompositeClassPart      composite_class;
  ShellClassPart          shell_class;
  OverrideShellClassPart  override_shell_class;
  SimpleMenuClassPart	  simpleMenu_class;
  TearofMenuClassPart	  tearofMenu_class;
} TearofMenuClassRec;

extern TearofMenuClassRec tearofMenuClassRec;

typedef struct _TearofMenuPart {
    /* resources */
    int      state;
    Position x_root, y_root;
    /* private state */
} TearofMenuPart;

typedef struct _TearofMenuRec {
  CorePart		core;
  CompositePart 	composite;
  ShellPart 	        shell;
  OverrideShellPart     override;
  SimpleMenuPart	simple_menu;
  TearofMenuPart	tearof_menu;
} TearofMenuRec;

#endif /* _TearofMenuP_h */
