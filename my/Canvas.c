/*

    Canvas.c - a widget that allows programmer-specified refresh procedures.
    Copyright (C) 1990 Robert H. Forsman Jr.

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

 */

#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>

#include <stdio.h>

#include "CanvasP.h"

#define offset(field) XtOffset(CanvasWidget, canvas.field)

static XtResource resources[] = {
    {(String) XtNexposeCallback, (String) XtCCallback, XtRCallback,
     sizeof(XtCallbackList), offset(redraw), XtRPointer, NULL},
    {(String) XtNrealizeCallback, (String) XtCCallback, XtRCallback,
     sizeof(XtCallbackList), offset(realize), XtRPointer, NULL},
    {(String) XtNresizeCallback, (String) XtCCallback, XtRCallback,
      sizeof(XtCallbackList), offset(resize), XtRPointer, NULL},
#ifdef    XAW3D
# undef offset /* field */
# define offset(field) XtOffsetOf(RectObjRec, rectangle.field)
    {(String) XtNborderWidth, (String) XtCBorderWidth, XtRDimension,
     sizeof(Dimension), offset(border_width), XtRImmediate, (XtPointer) 1},
#endif /* XAW3D */
};

static void Initialize(CanvasWidget req, CanvasWidget new);
static void Realize(Widget w, Mask*valueMask, XSetWindowAttributes*attributes);
static void Redisplay(Widget w, XExposeEvent *event, Region region);
static void Resize(Widget w);

CanvasClassRec canvasClassRec = {
    {
/* core_class fields	 */
#ifndef XAW3D
# define SuperClass               ((SimpleWidgetClass)&simpleClassRec)
#else  /* XAW3D */
# define SuperClass               ((ThreeDWidgetClass)&threeDClassRec)
#endif /* XAW3D */
    /* superclass                */ (WidgetClass) SuperClass,
    /* class_name	  	 */ (String) "Canvas",
    /* widget_size	  	 */ sizeof(CanvasRec),
    /* class_initialize   	 */ NULL,
    /* class_part_initialize	 */ NULL,
    /* class_inited       	 */ False,
    /* initialize	  	 */ (XtInitProc) Initialize,
    /* initialize_hook		 */ NULL,
    /* realize		  	 */ Realize,
    /* actions		  	 */ NULL,
    /* num_actions	  	 */ 0,
    /* resources	  	 */ resources,
    /* num_resources	  	 */ XtNumber(resources),
    /* xrm_class	  	 */ NULLQUARK,
    /* compress_motion	  	 */ True,
    /* compress_exposure  	 */ XtExposeCompressMultiple,
    /* compress_enterleave	 */ True,
    /* visible_interest	  	 */ True,
    /* destroy		  	 */ NULL,
    /* resize		  	 */ Resize,
    /* expose		  	 */ (XtExposeProc) Redisplay,
    /* set_values	  	 */ NULL,
    /* set_values_hook		 */ NULL,
    /* set_values_almost	 */ XtInheritSetValuesAlmost,
    /* get_values_hook		 */ NULL,
    /* accept_focus	 	 */ NULL,
    /* version			 */ XtVersion,
    /* callback_private   	 */ NULL,
    /* tm_table		   	 */ NULL,
    /* query_geometry		 */ NULL,
    /* display_accelerator       */ XtInheritDisplayAccelerator,
    /* extension                 */ NULL
    },
/* Simple class fields initialization */
    {
        /* change_sensitive      */  XtInheritChangeSensitive
    },
#ifdef  XAW3D
/* threeD class fields initialization */
    {
        /* ignore		 */  0
    },
#endif /* XAW3D */
};

WidgetClass canvasWidgetClass = (WidgetClass) & canvasClassRec;

static void Initialize(CanvasWidget req, CanvasWidget new)
{
    if (req->core.height <= 0)
        new->core.height = 200; /* a resonable default */

    if (req->core.width <= 0)
        new->core.width = 200;
}

static void Realize(Widget w, Mask *valueMask, XSetWindowAttributes*attributes)
{
    (*SuperClass->core_class.realize) (w, valueMask, attributes);
    XtCallCallbacks(w, XtNrealizeCallback, NULL);
} /* Realize */

static void Redisplay(Widget w, XExposeEvent *event, Region region)
{
    if (SuperClass->core_class.expose)
        (*SuperClass->core_class.expose)(w, (XEvent *) event, region);

/*
    if (!XtIsRealized(w))
        return;
*/

    XtCallCallbacks(w, XtNexposeCallback, region);
}

static void Resize(Widget w)
{
    XtCallCallbacks(w, XtNresizeCallback, NULL);
}
