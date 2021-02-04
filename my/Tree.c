/*
 * $XConsortium: Tree.c,v 1.42 91/02/20 20:06:07 converse Exp $
 *
 * Copyright 1990 Massachusetts Institute of Technology
 * Copyright 1989 Prentice Hall
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose and without fee is hereby granted, provided that the above
 * copyright notice appear in all copies and that both the copyright notice
 * and this permission notice appear in supporting documentation.
 * 
 * M.I.T., Prentice Hall and the authors disclaim all warranties with regard
 * to this software, including all implied warranties of merchantability and
 * fitness.  In no event shall M.I.T., Prentice Hall or the authors be liable
 * for any special, indirect or cosequential damages or any damages whatsoever
 * resulting from loss of use, data or profits, whether in an action of
 * contract, negligence or other tortious action, arising out of or in
 * connection with the use or performance of this software.
 * 
 * Authors:  Jim Fulton, MIT X Consortium,
 *           based on a version by Douglas Young, Prentice Hall
 * 
 * This widget is based on the Tree widget described on pages 397-419 of
 * Douglas Young's book "The X Window System, Programming and Applications 
 * with Xt OSF/Motif Edition."  The layout code has been rewritten to use
 * additional blank space to make the structure of the graph easier to see
 * as well as to support vertical trees.
 *
 * Partial rewrite by Ton Hospel 1993
 */

#include <X11/Intrinsic.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/CoreP.h>
#include <X11/CompositeP.h>
#include <X11/ConstrainP.h>
#include <X11/Xaw/XawInit.h>
#include <X11/Xaw/Cardinals.h>

#include "TreeP.h"

#define IsHorizontal(tw) ((tw)->tree.gravity == WestGravity || \
			  (tw)->tree.gravity == EastGravity)


					/* widget class method */
static void             ClassInitialize(void);
static void             Initialize(Widget grequest, Widget gnew);
static void             ConstraintInitialize(Widget request, Widget new);
static void             ConstraintDestroy(Widget w);
static Boolean          ConstraintSetValues(Widget current, Widget request,
                                            Widget new,
                                            ArgList args, Cardinal *num_args);
static void             Destroy(Widget gw);
static Boolean          SetValues(Widget gcurrent, Widget grequest,
		                  Widget gnew);
static XtGeometryResult GeometryManager(Widget w, XtWidgetGeometry *request,
                                        XtWidgetGeometry *reply);
static void             ChangeManaged(Widget gw);
static void             Redisplay(TreeWidget tw, XEvent *event, Region region);
static XtGeometryResult	QueryGeometry(Widget w, XtWidgetGeometry *intended,
                                      XtWidgetGeometry *preferred);

					/* utility routines */
static void             insert_node(Widget parent, Widget node);
static void             delete_node(Widget parent, Widget node);
static void             layout_tree(TreeWidget tw, Boolean insetvalues);


/*
 * resources of the tree itself
 */
static XtResource resources[] = {
    { (String) XtNautoReconfigure, (String) XtCAutoReconfigure, XtRBoolean,
          sizeof (Boolean),
	XtOffsetOf(TreeRec, tree.auto_reconfigure), XtRImmediate,
	(XtPointer) FALSE },
    { (String) XtNhSpace, (String) XtCHSpace, XtRDimension, sizeof (Dimension),
	XtOffsetOf(TreeRec, tree.hpad), XtRImmediate, (XtPointer) 0 },
    { (String) XtNvSpace, (String) XtCVSpace, XtRDimension, sizeof (Dimension),
	XtOffsetOf(TreeRec, tree.vpad), XtRImmediate, (XtPointer) 0 },
    { (String) XtNforeground, (String) XtCForeground, XtRPixel,
          sizeof (Pixel),
	XtOffsetOf(TreeRec, tree.foreground), XtRString,
          (XtPointer) XtDefaultForeground},
    { (String) XtNlineWidth, (String) XtCLineWidth, XtRDimension,
          sizeof (Dimension),
	XtOffsetOf(TreeRec, tree.line_width), XtRImmediate, (XtPointer) 0 },
    { (String) XtNgravity, (String) XtCGravity, XtRGravity, sizeof (XtGravity),
	XtOffsetOf(TreeRec, tree.gravity), XtRImmediate,
          (XtPointer) WestGravity },
};

/*
 * resources that are attached to all children of the tree
 */
static XtResource treeConstraintResources[] = {
    { (String) XtNtreeParent, (String) XtCTreeParent, XtRWidget,
          sizeof (Widget),
      XtOffsetOf(TreeConstraintsRec, tree.parent), XtRImmediate, NULL },
    { (String) XtNtreeGC, (String) XtCTreeGC, (String) XtRGC, sizeof(GC),
      XtOffsetOf(TreeConstraintsRec, tree.gc), XtRImmediate, NULL },
};

TreeClassRec treeClassRec = {
  {
					/* core_class fields  */
    (WidgetClass) &constraintClassRec,	/* superclass         */
    (String) "Tree",			/* class_name         */
    sizeof(TreeRec),			/* widget_size        */
    ClassInitialize,			/* class_init         */
    NULL,				/* class_part_init    */
    FALSE,				/* class_inited       */	
    (XtInitProc) Initialize,		/* initialize         */
    NULL,				/* initialize_hook    */	
    XtInheritRealize,			/* realize            */
    NULL,				/* actions            */
    0,					/* num_actions        */	
    resources,				/* resources          */
    XtNumber(resources),		/* num_resources      */
    NULLQUARK,				/* xrm_class          */
    TRUE,				/* compress_motion    */	
    TRUE,				/* compress_exposure  */	
    TRUE,				/* compress_enterleave*/	
    TRUE,				/* visible_interest   */
    Destroy,				/* destroy            */
    NULL,				/* resize             */
    (XtExposeProc) Redisplay,		/* expose             */
    (XtSetValuesFunc) SetValues,	/* set_values         */
    NULL,				/* set_values_hook    */	
    XtInheritSetValuesAlmost,		/* set_values_almost  */
    NULL,				/* get_values_hook    */	
    NULL,				/* accept_focus       */
    XtVersion,				/* version            */	
    NULL,				/* callback_private   */
    NULL,				/* tm_table           */
    QueryGeometry,			/* query_geometry     */	
    NULL,				/* display_accelerator*/
    NULL,				/* extension          */
  },
  {
					/* composite_class fields */
    GeometryManager,			/* geometry_manager    */
    ChangeManaged,			/* change_managed      */
    XtInheritInsertChild,		/* insert_child        */	
    XtInheritDeleteChild,		/* delete_child        */	
    NULL,				/* extension           */
  },
  { 
					/* constraint_class fields */
   treeConstraintResources,		/* subresources        */
   XtNumber(treeConstraintResources),	/* subresource_count   */
   sizeof(TreeConstraintsRec),		/* constraint_size     */
   (XtInitProc) ConstraintInitialize,	/* initialize          */
   ConstraintDestroy,			/* destroy             */
   ConstraintSetValues,			/* set_values          */
   NULL,				/* extension           */
   },
  {
					/* Tree class fields   */
    0,					/* ignore              */	
  }
};

WidgetClass treeWidgetClass = (WidgetClass) &treeClassRec;


/*****************************************************************************
 *                                                                           *
 *			     tree utility routines                           *
 *                                                                           *
 *****************************************************************************/

static void initialize_dimensions(Dimension **listp, int *sizep, int n)
{
    register int i;
    register Dimension *l;

    if (!*listp) {
	*listp = (Dimension *) XtCalloc ((unsigned int) n,
					 (unsigned int) sizeof(Dimension));
	*sizep = ((*listp) ? n : 0);
	return;
    }
    if (n > *sizep) {
	*listp = (Dimension *) XtRealloc((char *) *listp,
					 (unsigned int) (n*sizeof(Dimension)));
	if (!*listp) {
	    *sizep = 0;
	    return;
	}
	for (i = *sizep, l = (*listp) + i; i < n; i++, l++) *l = 0;
	*sizep = n;
    }
    return;
}

static GC get_tree_gc(TreeWidget w)
{
    XtGCMask valuemask = GCBackground | GCForeground;
    XGCValues values;

    values.background = w->core.background_pixel;
    values.foreground = w->tree.foreground;
    if (w->tree.line_width != 0) {
	valuemask |= GCLineWidth;
	values.line_width = w->tree.line_width;
    }

    return XtGetGC ((Widget) w, valuemask, &values);
}

static void insert_node(Widget parent, Widget node)
{
    TreeConstraints pc;
    TreeConstraints nc = TREE_CONSTRAINT(node);
    int nindex;
  
    nc->tree.parent = parent;

    if (parent == NULL) return;

    /*
     * If there isn't more room in the children array, 
     * allocate additional space.
     */  
    pc = TREE_CONSTRAINT(parent);
    nindex = pc->tree.n_children;
  
    if (pc->tree.n_children == pc->tree.max_children) {
	pc->tree.max_children += (pc->tree.max_children / 2) + 2;
	pc->tree.children = (WidgetList) XtRealloc ((char *)pc->tree.children, 
						    (unsigned int)
						    ((pc->tree.max_children) *
						    sizeof(Widget)));
    } 

    /*
     * Add the sub_node in the next available slot and 
     * increment the counter.
     */
    pc->tree.children[nindex] = node;
    pc->tree.n_children++;
}

static void delete_node(Widget parent, Widget node)
{
    TreeConstraints pc;
    int pos, i;

    /*
     * Make sure the parent exists.
     */
    if (!parent) return;  
  
    pc = TREE_CONSTRAINT(parent);

    /*
     * Find the sub_node on its parent's list.
     */
    for (pos = 0; pos < pc->tree.n_children; pos++)
      if (pc->tree.children[pos] == node) break;

    if (pos == pc->tree.n_children) return;

    /*
     * Decrement the number of children
     */  
    pc->tree.n_children--;

    /*
     * Fill in the gap left by the sub_node.
     * Zero the last slot for good luck.
     */
    for (i = pos; i < pc->tree.n_children; i++) 
      pc->tree.children[i] = pc->tree.children[i+1];

    pc->tree.children[pc->tree.n_children]=0;
}

static void check_gravity(TreeWidget tw, XtGravity grav)
{
    switch (tw->tree.gravity) {
      case WestGravity: case NorthGravity: case EastGravity: case SouthGravity:
	break;
      default:
	tw->tree.gravity = grav;
	break;
    }
}


/*****************************************************************************
 *                                                                           *
 * 			      tree class methods                             *
 *                                                                           *
 *****************************************************************************/

static void ClassInitialize(void)
{
    XawInitializeWidgetSet();
#ifndef   R4
    XtAddConverter (XtRString, XtRGravity, XmuCvtStringToGravity,
		    (XtConvertArgList) NULL, (Cardinal) 0);
#endif /* R4 */
}


static void Initialize(Widget grequest, Widget gnew)
{
    TreeWidget request = (TreeWidget) grequest, new = (TreeWidget) gnew;
    Arg args[2];

    /*
     * Make sure the widget's width and height are 
     * greater than zero.
     */
    if (request->core.width <= 0) new->core.width = 5;
    if (request->core.height <= 0) new->core.height = 5;

    /*
     * Set the padding according to the orientation
     */
    if (request->tree.hpad == 0 && request->tree.vpad == 0) {
	if (IsHorizontal (request)) {
	    new->tree.hpad = TREE_HORIZONTAL_DEFAULT_SPACING;
	    new->tree.vpad = TREE_VERTICAL_DEFAULT_SPACING;
	} else {
	    new->tree.hpad = TREE_VERTICAL_DEFAULT_SPACING;
	    new->tree.vpad = TREE_HORIZONTAL_DEFAULT_SPACING;
	}
    }

    /*
     * Create a graphics context for the connecting lines.
     */
    new->tree.gc = get_tree_gc (new);

    /*
     * Create the hidden root widget.
     */
    new->tree.tree_root = (Widget) NULL;
    XtSetArg(args[0], XtNwidth, 1);
    XtSetArg(args[1], XtNheight, 1);
    new->tree.tree_root = XtCreateWidget ("root", widgetClass, gnew, args,TWO);

    /*
     * Allocate the array used to hold the widest values per depth
     */
    new->tree.largest = NULL;
    new->tree.n_largest = 0;
    initialize_dimensions (&new->tree.largest, &new->tree.n_largest, 
			   TREE_INITIAL_DEPTH);

    /*
     * make sure that our gravity is one of the acceptable values
     */
    check_gravity (new, WestGravity);
} 


/* ARGSUSED */
static void ConstraintInitialize(Widget request, Widget new)
{
    TreeConstraints tc = TREE_CONSTRAINT(new);
    TreeWidget tw = (TreeWidget) new->core.parent;

    /*
     * Initialize the widget to have no sub-nodes.
     */
    tc->tree.n_children = 0;
    tc->tree.max_children = 0;
    tc->tree.children = (Widget *) NULL;
    tc->tree.x = tc->tree.y = 0; 
    tc->tree.bbsubwidth = 0;
    tc->tree.bbsubheight = 0;

    /*
     * If this widget has a super-node, add it to that 
     * widget' sub-nodes list. Otherwise make it a sub-node of 
     * the tree_root widget.
     */
    if (tc->tree.parent)
        insert_node (tc->tree.parent, new);
    else if (tw->tree.tree_root)
        insert_node (tw->tree.tree_root, new);
} 

/* ARGSUSED */
static Boolean SetValues(Widget gcurrent, Widget grequest, Widget gnew)
{
    TreeWidget current = (TreeWidget) gcurrent, new = (TreeWidget) gnew;
    Boolean redraw = FALSE;

    /*
     * If the foreground color has changed, redo the GC's
     * and indicate a redraw.
     */
    if (new->tree.foreground != current->tree.foreground ||
	new->core.background_pixel != current->core.background_pixel ||
	new->tree.line_width != current->tree.line_width) {
	XtReleaseGC (gnew, new->tree.gc);
	new->tree.gc = get_tree_gc (new);
	redraw = TRUE;     
    }

    /*
     * If the minimum spacing has changed, recalculate the
     * tree layout. layout_tree() does a redraw, so we don't
     * need SetValues to do another one.
     */
    if (new->tree.gravity != current->tree.gravity) {
	check_gravity (new, current->tree.gravity);
    }

    if (IsHorizontal(new) != IsHorizontal(current)) {
	if (new->tree.vpad == current->tree.vpad &&
	    new->tree.hpad == current->tree.hpad) {
	    new->tree.vpad = current->tree.hpad;
	    new->tree.hpad = current->tree.vpad;
	}
    }

    if (new->tree.vpad != current->tree.vpad ||
	new->tree.hpad != current->tree.hpad ||
	new->tree.gravity != current->tree.gravity) {
	layout_tree(new, TRUE);
	redraw = FALSE;
    }
    return redraw;
}

/* ARGSUSED */
static Boolean ConstraintSetValues(Widget current, Widget request, Widget new,
                                   ArgList args, Cardinal *num_args)
{
    TreeConstraints newc = TREE_CONSTRAINT(new);
    TreeConstraints curc = TREE_CONSTRAINT(current);
    TreeWidget tw = (TreeWidget) new->core.parent;
    int RealChanges;

    RealChanges = 0;
    /*
     * If the parent field has changed, remove the widget
     * from the old widget's children list and add it to the
     * new one.
     */
    if (curc->tree.parent != newc->tree.parent){
	if (curc->tree.parent) {
            delete_node(curc->tree.parent, new);
            if (current->core.managed != False) RealChanges = 1;
        }
	if (newc->tree.parent) {
            insert_node(newc->tree.parent, new);
            if (new->core.managed != False) RealChanges = 1;
        }

	/*
         * If the Tree widget has been realized, 
         * compute new layout.
         */
	if (RealChanges && XtIsRealized((Widget)tw))
            layout_tree(tw, FALSE);
    }
    return False;
}


static void ConstraintDestroy(Widget w)
{ 
    TreeConstraints tc = TREE_CONSTRAINT(w);
    TreeWidget tw = (TreeWidget) XtParent(w);
    int i;

    /* 
     * Remove the widget from its parent's sub-nodes list and
     * make all this widget's sub-nodes sub-nodes of the parent.
     */
  
    if (tw->tree.tree_root == w) {
	if (tc->tree.n_children > 0)
            tw->tree.tree_root = tc->tree.children[0];
	else
            tw->tree.tree_root = NULL;
    }

    delete_node(tc->tree.parent, (Widget) w);
    for (i = 0; i< tc->tree.n_children; i++)
        insert_node (tc->tree.parent, tc->tree.children[i]);

    /* Do we really need this ? I think widget will be unmanaged already */
    if (w->core.managed != False)
        layout_tree((TreeWidget) (w->core.parent), FALSE);
}

/* ARGSUSED */
static XtGeometryResult GeometryManager(Widget w, XtWidgetGeometry *request,
                                        XtWidgetGeometry *reply)
{
    TreeWidget tw = (TreeWidget) w->core.parent;

    /*
     * No position changes allowed!.
     */
    if ((request->request_mode & CWX && request->x!=w->core.x)
	||(request->request_mode & CWY && request->y!=w->core.y))
        return (XtGeometryNo);

    /*
     * Allow all resize requests.
     */

    if (request->request_mode & CWWidth)
        w->core.width = request->width;
    if (request->request_mode & CWHeight)
        w->core.height = request->height;
    if (request->request_mode & CWBorderWidth)
        w->core.border_width = request->border_width;

    if (w->core.managed != False && tw->tree.auto_reconfigure != False)
        layout_tree(tw, FALSE);
    return (XtGeometryYes);
}

static void ChangeManaged(Widget gw)
{
    layout_tree((TreeWidget) gw, FALSE);
}


static void Destroy(Widget gw)
{
    TreeWidget w = (TreeWidget) gw;

    XtReleaseGC (gw, w->tree.gc);
    if (w->tree.largest) XtFree ((char *) w->tree.largest);
}

static void DrawLines(TreeWidget tw, Widget child, GC localgc,
                      int depth, int x, int y)
{
    TreeConstraints tc;
    int             i, srcx, srcy;
    Dimension       bw, corew, coreh;
    Display        *dpy;
    Window          w;

    tc = TREE_CONSTRAINT(child);
    if (child->core.managed == False)
        for (i = 0; i < tc->tree.n_children; i++)
            DrawLines(tw, tc->tree.children[i], localgc, depth, x, y);
    else {
        srcx  = child->core.x;
        srcy  = child->core.y;
        corew = child->core.width;
        coreh = child->core.height;
        bw    = child->core.border_width;
        /*
         * Don't draw lines from the fake tree_root.
         */
        if (depth) {
            dpy   = XtDisplay(tw);
            w     = XtWindow(tw);

            if (!localgc) localgc = tw->tree.gc;
            switch (tw->tree.gravity) {
              case WestGravity:
                /*
                 * right center to left center
                 */
                XDrawLine(dpy, w, localgc, x, y,
                          srcx, srcy + (int) bw + (int) coreh / 2);
                break;
              case NorthGravity:
                /*
                 * bottom center to top center
                 */
                XDrawLine(dpy, w, localgc, x, y,
                          srcx + (int) bw + (int) corew / 2, srcy);
                break;
              case EastGravity:
                /*
                 * left center to right center
                 */
                XDrawLine(dpy, w, localgc, x, y,
                          srcx + (((int) bw) << 1) + (int) corew,
                          srcy + (int) bw + (int) coreh / 2);
                break;
              case SouthGravity:
                /*
                 * top center to bottom center
                 */
                XDrawLine(dpy, w, localgc, x, y,
                          srcx + (int) bw + (int) corew / 2,
                          srcy + (((int) bw) << 1) + (int) coreh);
                break;
            }
        }
            
        switch (tw->tree.gravity) {
          case WestGravity:
            srcx += corew + 2 * bw;
            srcy += bw + coreh / 2;
            break;
          case EastGravity:
            srcy += bw + coreh / 2;
            break;
          case NorthGravity:
            srcx += bw + corew / 2;
            srcy += coreh + 2 * bw;
            break;
          case SouthGravity:
            srcx += bw + corew / 2;
            break;
        }
        for (i = 0; i < tc->tree.n_children; i++)
            DrawLines(tw, tc->tree.children[i], tc->tree.gc,
                      depth+1, srcx, srcy);
    }
}

/* ARGSUSED */
static void Redisplay(TreeWidget tw, XEvent *event, Region region)
{
    Widget          root;
    TreeConstraints tc;
    int             i;
    /*
     * If the Tree widget is visible, visit each managed child.
     */
    if (tw->core.visible) {
        root = tw->tree.tree_root;
        tc = TREE_CONSTRAINT(root);
        for (i=0; i < tc->tree.n_children; i++)
            DrawLines(tw, tc->tree.children[i], tw->tree.gc, 0, 0, 0);
    }
}

static XtGeometryResult QueryGeometry(Widget w, XtWidgetGeometry *intended,
                                      XtWidgetGeometry *preferred)
{
    register TreeWidget tw = (TreeWidget) w;

    preferred->request_mode = (CWWidth | CWHeight);
    preferred->width = tw->tree.maxwidth;
    preferred->height = tw->tree.maxheight;

    if (((intended->request_mode & (CWWidth | CWHeight)) ==
	 (CWWidth | CWHeight)) &&
	intended->width == preferred->width &&
	intended->height == preferred->height)
        return XtGeometryYes;
    else if (preferred->width == w->core.width &&
             preferred->height == w->core.height)
        return XtGeometryNo;
    else
        return XtGeometryAlmost;
}


/*****************************************************************************
 *                                                                           *
 *			     tree layout algorithm                           *
 *                                                                           *
 * Each node in the tree is "shrink-wrapped" with a minimal bounding         *
 * rectangle, laid next to its siblings (with a small about of padding in    *
 * between) and then wrapped with their parent.  Parents are centered about  *
 * their children (or vice versa if the parent is larger than the children). *
 *                                                                           *
 *****************************************************************************/

static void compute_bounding_box_subtree(TreeWidget tree, Widget w, int depth)
{
    TreeConstraints tc = TREE_CONSTRAINT(w); /* info attached to all kids */
    register int i;
    Bool horiz = IsHorizontal(tree);
    Dimension newwidth, newheight;
    Dimension bw2;

    if (w->core.managed == False) {
        newwidth = newheight = 0;

        for (i = 0; i < tc->tree.n_children; i++) {
            Widget child = tc->tree.children[i];
            TreeConstraints cc = TREE_CONSTRAINT(child);
	    
            compute_bounding_box_subtree(tree, child, depth);
            if (horiz) {
                if (cc->tree.bbheight)
                    newheight += tree->tree.vpad + cc->tree.bbheight;
            } else {
                if (cc->tree.bbwidth)
                    newwidth += tree->tree.hpad + cc->tree.bbwidth;
            }
        }
        if (horiz) {
            if (newheight) newheight -= tree->tree.vpad;
        } else {
            if (newwidth)  newwidth  -= tree->tree.hpad;
        }
        tc->tree.bbsubwidth  = tc->tree.bbwidth  = newwidth;
        tc->tree.bbsubheight = tc->tree.bbheight = newheight;
    } else {
        bw2 = w->core.border_width * 2;

        /*
         * Set the max-size per level.
         */
        if (depth >= tree->tree.n_largest) {
            initialize_dimensions (&tree->tree.largest,
                                   &tree->tree.n_largest, depth + 1);
        }
        newwidth = ((horiz ? w->core.width : w->core.height) + bw2);
        if (tree->tree.largest[depth] < newwidth)
            tree->tree.largest[depth] = newwidth;

        /*
         * initialize
         */
        tc->tree.bbwidth  = w->core.width  + bw2;
        tc->tree.bbheight = w->core.height + bw2;

        if (tc->tree.n_children == 0) {
            tc->tree.bbsubwidth = tc->tree.bbsubheight = 0;
            return;
        }

        /*
         * Figure the size of the opposite dimension (vertical if tree is 
         * horizontal, else vice versa).  The other dimension will be set 
         * in the second pass once we know the maximum dimensions.
         */
        newwidth  = 0;
        newheight = 0;
        for (i = 0; i < tc->tree.n_children; i++) {
            Widget child = tc->tree.children[i];
            TreeConstraints cc = TREE_CONSTRAINT(child);
	    
            compute_bounding_box_subtree (tree, child, depth + 1);

            if (horiz) {
                if (cc->tree.bbheight)
                    newheight += tree->tree.vpad + cc->tree.bbheight;
            } else {
                if (cc->tree.bbwidth)
                    newwidth  += tree->tree.hpad + cc->tree.bbwidth;
            }
        }

        /*
         * Now fit parent onto side (or top) of bounding box and correct for
         * extra padding.
         */
        if (horiz) {
            if (newheight) {
                newheight -= tree->tree.vpad;
                if (newheight > tc->tree.bbheight)
                    tc->tree.bbheight = newheight;
            }
        } else {
            if (newwidth) {
                newwidth -= tree->tree.hpad;
                if (newwidth > tc->tree.bbwidth)
                    tc->tree.bbwidth = newwidth;
            }
        }

        tc->tree.bbsubwidth  = newwidth;
        tc->tree.bbsubheight = newheight;
    }
}

/* why recursive ?                            */
static void set_positions(TreeWidget tw, Widget w, int level)
{
    int i;
  
    if (w) {
	TreeConstraints tc = TREE_CONSTRAINT(w);

	if (level > 0) {
	    /*
             * mirror if necessary
             */
	    switch (tw->tree.gravity) {
	      case EastGravity:
		tc->tree.x =
                    (((Position) tw->tree.maxwidth) -
                     ((Position) (w->core.width+2*w->core.border_width)) -
                     tc->tree.x);
		break;

	      case SouthGravity:
		tc->tree.y =
                    (((Position) tw->tree.maxheight) -
                     ((Position) (w->core.height+2*w->core.border_width)) -
                     tc->tree.y);
		break;
	    }
            
	    /*
             * Move the widget into position.
             */
	    XtMoveWidget (w, tc->tree.x, tc->tree.y);
	}
        
	/*
         * Set the positions of all children.
         */
	for (i = 0; i < tc->tree.n_children; i++)
            set_positions (tw, tc->tree.children[i], level + 1);
    }
}

static void arrange_subtree(TreeWidget tree, Widget w, int depth,
                            Position x, Position y)
{
    TreeConstraints tc = TREE_CONSTRAINT(w); /* info attached to all kids */
    register int i;
    int newx, newy, myh, myw;
    Bool horiz = IsHorizontal(tree);
    Widget child = NULL;
    Dimension bw2 = w->core.border_width * 2;

    tc->tree.x = newx = x;
    tc->tree.y = newy = y;

    if (w->core.managed == False) {
        /*
         * Walk down tree laying out children, then laying out parents.
         */
        for (i = 0; i < tc->tree.n_children; i++) {
            TreeConstraints cc;

            child = tc->tree.children[i];
            cc = TREE_CONSTRAINT(child);

            arrange_subtree(tree, child, depth, newx, newy);
            if (horiz) {
                if (cc->tree.bbheight)
                    newy += tree->tree.vpad + cc->tree.bbheight;
            } else {
                if (cc->tree.bbwidth)
                    newx += tree->tree.hpad + cc->tree.bbwidth;
            }
        }

        /* Layout as if width and height are 0 */
        if (horiz) tc->tree.y += tc->tree.bbsubheight / 2;
        else       tc->tree.x += tc->tree.bbsubwidth  / 2;
    } else {
        myw = myh = 0; /* against uninitialised variable compiler warnings */

        /* Be carefull to make sure connections to a single child are
           horizontal/vertical
        */
        if (horiz) {
            myh = tc->tree.bbsubheight/2 - (int)(w->core.height + bw2)/2;
            if (myh > 0) tc->tree.y += myh;
            else newy -= myh;
            newx += tree->tree.hpad;
            newx += tree->tree.largest[depth];
        } else {
            myw = tc->tree.bbsubwidth/2 - (int)(w->core.width + bw2)/2;
            if (myw > 0) tc->tree.x += myw;
            else newx -= myw;
            newy += tree->tree.vpad;
            newy += tree->tree.largest[depth];
        }

        for (i = 0; i < tc->tree.n_children; i++) {
            TreeConstraints cc;

            child = tc->tree.children[i];
            cc = TREE_CONSTRAINT(child);

            arrange_subtree (tree, child, depth + 1, newx, newy);
            if (horiz) {
                if (cc->tree.bbheight)
                    newy += tree->tree.vpad + cc->tree.bbheight;
            } else {
                if (cc->tree.bbwidth)
                    newx += tree->tree.hpad + cc->tree.bbwidth;
            }
        }
    }
}

static void set_tree_size(TreeWidget tw, Boolean insetvalues,
                          Dimension width, Dimension height)
{
   if (insetvalues) {
	tw->core.width = width;
	tw->core.height = height;
    } else {
	Dimension replyWidth = 0, replyHeight = 0;
	XtGeometryResult result = XtMakeResizeRequest ((Widget) tw,
						       width, height,
						       &replyWidth,
						       &replyHeight);
	/*
	 * Accept any compromise.
	 */
	if (result == XtGeometryAlmost)
	  XtMakeResizeRequest ((Widget) tw, replyWidth, replyHeight,
			       (Dimension *) NULL, (Dimension *) NULL);
    }
    return;
}

static void layout_tree(TreeWidget tw, Boolean insetvalues)
{
    int i, used;
    Dimension *dp, size;

    /*
     * Do a depth-first search computing the width and height of the bounding
     * box for the tree at that position (and below).  Then, walk again using
     * this information to layout the children at each level.
     */

    if (tw->tree.tree_root == NULL) return;

    tw->tree.maxwidth = tw->tree.maxheight = 0;
    for (i = 0, dp = tw->tree.largest; i < tw->tree.n_largest; i++, dp++)
        *dp = 0;
    initialize_dimensions (&tw->tree.largest, &tw->tree.n_largest, 
			   tw->tree.n_largest);
    compute_bounding_box_subtree (tw, tw->tree.tree_root, 0);
    size =  0;
    used = -1;
    for (i = 0, dp = tw->tree.largest; i < tw->tree.n_largest; i++, dp++)
        if (*dp) {
            used  = i;
            size += *dp;
        }
    if (IsHorizontal(tw)) {
        if (size) tw->tree.maxwidth = size + used * tw->tree.hpad;
        else      tw->tree.maxwidth = 0;
        tw->tree.maxheight =
            TREE_CONSTRAINT(tw->tree.tree_root)->tree.bbheight;
    } else {
        if (size) tw->tree.maxheight = size + used * tw->tree.vpad;
        else      tw->tree.maxheight = 0;
        tw->tree.maxwidth =
            TREE_CONSTRAINT(tw->tree.tree_root)->tree.bbwidth;
    }
    /*
     * Second pass to do final layout.  Each child's bounding box is stacked
     * on top of (if horizontal, else next to) on top of its siblings.  The
     * parent is centered between the first and last children.
     */
    arrange_subtree (tw, tw->tree.tree_root, 0, 0, 0);

    if (used+11 < tw->tree.n_largest) {
        /* treedepth must have gone down quite a bit. Free memory */
        XtFree((char *) tw->tree.largest);
        tw->tree.n_largest = 0;
        tw->tree.largest = 0;
    }

    /*
     * Move each widget into place.
     */
    set_tree_size(tw, insetvalues, tw->tree.maxwidth, tw->tree.maxheight);
    set_positions(tw, tw->tree.tree_root, 0);

    /*
     * And redisplay.
     */
    if (XtIsRealized ((Widget) tw)) {
	XClearArea (XtDisplay(tw), XtWindow((Widget)tw), 0, 0, 0, 0, True);
    }
}

/*****************************************************************************
 *                                                                           *
 * 				Public Routines                              *
 *                                                                           *
 *****************************************************************************/

void
#if NeedFunctionPrototypes
XawTreeForceLayout (Widget tree)
#else
XawTreeForceLayout (tree)
    Widget tree;
#endif
{
    layout_tree ((TreeWidget) tree, FALSE);
}
