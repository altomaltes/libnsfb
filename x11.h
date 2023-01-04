/**
 *     AUTHOR: Jose Angel Caso Sanchez, 2022   ( altomaltes@gmail.com )
 *
 *     FILE: x11.cc
 *     DATE: ene 2022
 *
 *  DESCRIPCION:
 *
 *   X11 driver
 *
 *  2009 Vincent Sanders <vince@simtec.co.uk>
 */


#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/xpm.h>
#include <X11/cursorfont.h>

#define DISP_MAX 256


struct x11Priv
{ NsfbSurfaceRtns rtns;  /* Includes next */

  Window    theRoot;
  Colormap  theCmap; XColor palette[ 256 ];
  Display * theDisplay;
     int    theScreen;
  Visual  * theVisual;
};

struct x11List
{ Nsfb      seed;        /* Includes up and next */

  Display * theDisplay;  /* catched */

  Window    theWindow;
  int       theBackground;
  GC        theGC;
  Pixmap    theSurface;
};


#define EVENTMASK ExposureMask           \
                | VisibilityChangeMask   \
                | StructureNotifyMask    \
                | FocusChangeMask        \
                | LeaveWindowMask        \
                                         \
                | KeyPressMask           \
                | KeyReleaseMask         \
                                         \
                | ButtonPressMask        \
                | ButtonReleaseMask      \
                | ButtonMotionMask       \
                | PointerMotionMask



extern struct NsfbPlotterFns x11PlottersAddr;










