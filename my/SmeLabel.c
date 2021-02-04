#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/Xaw/XawInit.h>

#include "SmeLabelP.h"

static void DoNothing(Widget w);

#define superclass (&smeBSBClassRec)
SmeLabelClassRec smeLabelClassRec = {
  {
    /* superclass         */    (WidgetClass) superclass,
    /* class_name         */    (String) "SmeLabel",
    /* size               */    sizeof(SmeLabelRec),
    /* class_initializer  */	XawInitializeWidgetSet,
    /* class_part_initialize*/	NULL,
    /* Class init'ed      */	FALSE,
    /* initialize         */    NULL,
    /* initialize_hook    */	NULL,
    /* realize            */    NULL,
    /* actions            */    NULL,
    /* num_actions        */    0,
    /* resources          */    NULL,
    /* resource_count     */	0,
    /* xrm_class          */    NULLQUARK,
    /* compress_motion    */    FALSE, 
    /* compress_exposure  */    FALSE,
    /* compress_enterleave*/ 	FALSE,
    /* visible_interest   */    FALSE,
    /* destroy            */    NULL,
    /* resize             */    NULL,
    /* expose             */    XtInheritExpose,
    /* set_values         */    NULL,
    /* set_values_hook    */	NULL,
    /* set_values_almost  */	XtInheritSetValuesAlmost,  
    /* get_values_hook    */	NULL,			
    /* accept_focus       */    NULL,
    /* intrinsics version */	XtVersion,
    /* callback offsets   */    NULL,
    /* tm_table		  */    NULL,
    /* query_geometry	  */    XtInheritQueryGeometry,
    /* display_accelerator*/    NULL,
    /* extension	  */    NULL
  },{
    /* Menu Entry Fields */
      
    /* highlight */             DoNothing,
    /* unhighlight */           DoNothing,
    /* notify */		DoNothing,
    /* extension	  */    NULL
  }, {
#ifdef    XAW3D
    /* ThreeDClass Fields */
    /* shadowdraw         */    XtInheritXawSme3dShadowDraw
  }, {
#endif /* XAW3D */
    /* BSB Menu entry Fields */  

    /* extension	  */    NULL
  }, {
    /* Label Menu entry Fields */  

    /* extension	  */    NULL
  }
};

WidgetClass smeLabelObjectClass = (WidgetClass) &smeLabelClassRec;

static void DoNothing(Widget w)
{
}
