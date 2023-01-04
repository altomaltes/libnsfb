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


#define WINVER  0x0500

#include <limits.h>
#include <string.h>

#include "palette.h"
#include "libnsfb_plot_util.h"
#include "nsfb.h"
#include "plot.h"
#include "surface.h"

#undef byte

#include <windows.h>

#ifndef WINGDIAPI
  #define HBITMAP void *
#endif

/*
 * JASC, may 2007, reserve an internal message
 */

//const int HABANA_MESSAGE= WM_USER;
#undef  WM_USER
//const int WM_USER= HABANA_MESSAGE + 1;


#ifdef _WIN32_CE
  #define strdup _strdup
#endif


/*
 *
 */

#define DSTCOPY 0x00AA0029

struct w32Priv
{ NsfbSurfaceRtns rtns;  /* Includes next */

  int    theHue;

  struct HistRec * theHistogram;
  int theScreen;
};

struct w32List
{ Nsfb      seed;        /* Includes up and next */

  HANDLE    theWindow;

  WINDOWPOS theOrigin;   // Jul 2006, w indow screen position
//  int       theStride;   // Jul 2006, w indow screen position
  HDC       theMM;       // Image coping
  HDC       theGC;
};


extern struct NsfbPlotterFns w32PlottersAddr;










