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

#define ENABLE_24_BPP

#define DO_ALPHA_BLEND   1
#define DO_FRONT_RENDER  2

/** NS Framebuffer context
 */
struct nsfb_s
{ int fd;                         /** device handler  */

  int width;                      /**< Visible width.  */
  int height;                     /**< Visible height. */

  enum nsfb_rotate_e rotate;      /** Rotate canvas    */

  char * parameters;

  enum nsfb_format_e format;       /** Framebuffer format */

  int  bpp;                        /** Bits per pixel - distinct from format */

  byte * loc;                      /** Base of working video memory. */
  byte * pan;                      /** Panned video memory. */

  int    loclen;                   /** length of a video line. */
  int    panlen;                   /** length of a video line. */
  int    buflen;                   /** frame buffer size       */
  int    panCount;                 /** doublebuffer switch counter */

  struct NsfbPalette * palette;              /** palette for index modes */
  nsfbCursor_t      * cursor;               /** cursor                  */

  struct  nsfb_surface_rtns_s *surface_rtns; /**< surface routines. */
  void  * surface_priv;                      /** surface opaque data. */

  nsfb_bbox_t clip;                          /** current clipping rectangle for plotters */
  int active1;                                /** Framebuffer outputing                   */
  struct nsfb_plotter_fns_s *plotter_fns;    /** Plotter methods                         */

};



int drmFinalise(   nsfb_t * );
int drmInitialize( nsfb_t * );
int drmPan(        nsfb_t * , int type );


#endif

/*
 * Local variables:
 *  c-basic-offset: 4
 *  tab-width: 8
 * End:
 */
