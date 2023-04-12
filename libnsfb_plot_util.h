/*
 * Copyright 2009 Vincent Sanders <vince@simtec.co.uk>
 *
 * This file is part of libnsfb, http://www.netsurf-browser.org/
 * Licenced under the MIT License,
 *                http://www.opensource.org/licenses/mit-license.php
 *
 * This is the exported interface for the libnsfb graphics library.
 */

#ifndef _LIBNSFB_PLOT_UTIL_H
#define _LIBNSFB_PLOT_UTIL_H 1

#include "nsfb.h"
#include "nsfbPlot.h"

struct HistRecStruct;

/* alpha blend two pixels together
 */
static inline NSFBCOLOUR nsfbPlotAblend( NSFBCOLOUR pixel
                                       , NSFBCOLOUR scrpixel )
{ int opacity= pixel >> 24;
  int transp = 0x100 - opacity;

  dword rb, g;

  rb = ((pixel & 0xFF00FF) * opacity + (scrpixel & 0xFF00FF) * transp) >> 8;
  g  = ((pixel & 0x00FF00) * opacity + (scrpixel & 0x00FF00) * transp) >> 8;

  return ((rb & 0xFF00FF) | (g & 0xFF00));
}

#define restrict

ANSIC bool nsfbPlotClip(      const NsfbBbox *, NsfbBbox * restrict rect );
ANSIC bool nsfbPlotClipCtx(             Nsfb *, NsfbBbox * restrict rect );
ANSIC bool nsfbPlotClipLine(  const NsfbBbox *, NsfbBbox * restrict line );
ANSIC bool nsfbPlotClipLineCtx(         Nsfb *, NsfbBbox * restrict line );

/** Obtain a bounding box which is the superset of two source boxes.
 *
 */
//ANSIC bool nsfbPlot_add_rect(const NsfbBbox *box1, const NsfbBbox *box2, NsfbBbox *result);

/** Find if two boxes intersect. */
//ANSIC bool nsfbPlot_bbox_intersect(const NsfbBbox *box1, const NsfbBbox *box2);

/* JACS, (altomaltes)
 *
 * from resize.c
 */

typedef struct ChangerRecStruct ChangerRec;



#endif /* _LIBNSFB_PLOT_UTIL_H */
