/*
 * Copyright 2009 Vincent Sanders <vince@simtec.co.uk>
 *
 * This file is part of libnsfb, http://www.netsurf-browser.org/
 * Licenced under the MIT License,
 *                http://www.opensource.org/licenses/mit-license.php
 *
 * This is the internal interface for the libnsfb graphics library.
 */

#ifndef _NSFB_H
#define _NSFB_H 1

#include <stdint.h>

#ifdef HAVE_CONFIG_H
  #include "nsfbcfg.h"
#endif

#define word  unsigned short
#define dword unsigned int
#define byte  unsigned char

#include "libnsfb.h"


#define ENABLE_24_BPP

#define DO_ALPHA_BLEND   1
#define DO_FRONT_RENDER  2

/* input related
 */
typedef int (*ToClientFun)( struct NomadEvent * evt, void * userData );


/** NS Framebuffer context
 */
struct NsfbSt
{ struct NsfbSt            * next;        /** Next window (altomaltes) */

  struct NsfbSurfaceRtnsSt * surfaceRtns; /**< surface routines. ( inherited, up )  */
  struct NsfbPlotterFns    * plotterFns;  /** Plotter methods   */

  const char * theTitle;

  int offx;                           /** Panel offset */
  int offy;                           /** Panel offset */

  int width;                          /** Visible width.  */
  int height;                         /** Visible height. */

  enum NsfbFormat format;             /** Framebuffer format */
  enum NsfbRotate theGeo;             /** Rotate canvas    */

  int    bpp;                         /** Bits per pixel - distinct from format */

  byte * loc;                         /** Base of working video memory. */
  byte * pan;                         /** Panned video memory. */

  int    loclen;                      /** length of a video line. */
  int    panlen;                      /** length of a video line. */
  int    buflen;                      /** frame buffer size       */
  int    panCount;                    /** doublebuffer switch counter */

  struct NsfbPalette * palette;       /** palette for index modes */

  NsfbBbox clip;                      /** current clipping rectangle for plotters */

  ToClientFun     toClient;           /** client sourcer          */
  void          * toData;             /** client sourcer data     */


};



#endif

/*
 * Local variables:
 *  c-basic-offset: 4
 *  tab-width: 8
 * End:
 */
