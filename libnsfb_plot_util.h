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

#include "libnsfb_plot.h"

/* alpha blend two pixels together
 */
static inline nsfb_colour_t nsfbPlotAblend( nsfb_colour_t pixel
                                          , nsfb_colour_t scrpixel )
{ int opacity= pixel >> 24;
  int transp = 0x100 - opacity;

  dword rb, g;

  rb = ((pixel & 0xFF00FF) * opacity + (scrpixel & 0xFF00FF) * transp) >> 8;
  g  = ((pixel & 0x00FF00) * opacity + (scrpixel & 0x00FF00) * transp) >> 8;

  return ((rb & 0xFF00FF) | (g & 0xFF00));
}

#define restrict

PUBLIC bool nsfbPlot_clip(      const nsfb_bbox_t *, nsfb_bbox_t * restrict rect );
PUBLIC bool nsfbPlot_clip_ctx(             nsfb_t *, nsfb_bbox_t * restrict rect );
PUBLIC bool nsfbPlot_clip_line( const nsfb_bbox_t *, nsfb_bbox_t * restrict line );
PUBLIC bool nsfbPlot_clip_line_ctx(        nsfb_t *, nsfb_bbox_t * restrict line );

/** Obtain a bounding box which is the superset of two source boxes.
 *
 */
PUBLIC bool nsfbPlot_add_rect(const nsfb_bbox_t *box1, const nsfb_bbox_t *box2, nsfb_bbox_t *result);

/** Find if two boxes intersect. */
PUBLIC bool nsfbPlot_bbox_intersect(const nsfb_bbox_t *box1, const nsfb_bbox_t *box2);

#endif /* _LIBNSFB_PLOT_UTIL_H */
