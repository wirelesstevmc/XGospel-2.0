#ifndef _YShellP_h
#define _YShellP_h
#include <X11/CoreP.h>
#include <X11/ShellP.h>

#include "YShell.h"
#include "widgettree.h"

typedef struct {
    XtPointer       extension;          /* pointer to extension record      */
} YShellClassPart;

typedef struct _YShellClassRec {
    CoreClassPart             core_class;
    CompositeClassPart        composite_class;
    ShellClassPart            shell_class;
    WMShellClassPart          wm_shell_class;
    VendorShellClassPart      vendor_shell_class;
    TopLevelShellClassPart    top_level_shell_class;
    ApplicationShellClassPart application_shell_class;
    YShellClassPart           y_shell_class;
} YShellClassRec;

extern YShellClassRec yShellClassRec;

typedef struct {
    /* resources */
    Visual             *visual;
    int	                depth;
    NameClassList       class_names;
    String              top_name, tree, res_tree;
    Boolean             print_tree, print_resources, dump_on_X_error;
    /* private state */
    TreeTemplate *tree_shape, *tree_use;
    MyContext           context;
} YShellPart;

typedef struct _YShellRec {
    CorePart 	         core;
    CompositePart 	 composite;
    ShellPart 	         shell;
    WMShellPart	         wm;
    VendorShellPart	 vendor;
    TopLevelShellPart    topLevel;
    ApplicationShellPart application;
    YShellPart	         yShell;
} YShellRec;

#endif /* _YShellP_h */
