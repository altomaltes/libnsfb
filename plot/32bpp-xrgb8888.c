/*
 * Copyright 2009 Vincent Sanders <vince@simtec.co.uk>
 * Copyright 2010 Michael Drake <tlsa@netsurf-browser.org>
 *
 * This file is part of libnsfb, http://www.netsurf-browser.org/
 * Licenced under the MIT License,
 *                http://www.opensource.org/licenses/mit-license.php
 */

#include "../libnsfb_plot_util.h"

#include "../nsfb.h"
#include "../plot.h"

static inline nsfb_colour_t PIXEL_TO_COLOR( void * p, int idx )
{ dword * ptr= (dword *)p;

  return( ptr[ idx ] );
}

static inline void COLOR_TO_PIXEL( void * p, int idx, nsfb_colour_t c )
{ dword * ptr= (dword *)p;

  ptr[ idx ]= c;
}


#define PLOT_TYPE dword
#define PLOT_LINELEN(ll) ((ll) >> 2)

#include "32bpp-common.c"

const nsfb_plotter_fns_t _nsfb_32bpp_xrgb8888_plotters =
{ .line        = line
, .fill        = fill
, .point       = point
, .bitmap      = bitmap
, .bitmap_tiles= bitmap_tiles
, .glyph8      = glyph8
, .glyph1      = glyph1
, .readrect    = readrect
, .moverect    = moverect
};

/*
 * Local Variables:
 * c-basic-offset:8
 * End:
 */
