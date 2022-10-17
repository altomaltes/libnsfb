/*
 * Copyright 2009 Vincent Sanders <vince@simtec.co.uk>
 * Copyright 2010 Michael Drake <tlsa@netsurf-browser.org>
 *
 * This file is part of libnsfb, http://www.netsurf-browser.org/
 * Licenced under the MIT License,
 *                http://www.opensource.org/licenses/mit-license.php
 */

#include <string.h>

#include "../libnsfb_plot_util.h"

#include "../nsfb.h"
#include "../palette.h"
#include "../plot.h"

static inline void *get_xy_loc(nsfb_t *nsfb, int x, int y)
{ return(nsfb->loc + (y * nsfb->loclen) + (x));
}
static inline void *get_xy_pan(nsfb_t *nsfb, int x, int y)
{ return(nsfb->pan + (y * nsfb->panlen) + (x));
}


static inline nsfb_colour_t PIXEL_TO_COLOR( void * p, int idx )
{// byte * ptr= (byte *)p;

//  return( nsfb->palette
  //      ? nsfb->palette->data[pixel]
    //    : 0 );
    return( 0 );
}

static inline void COLOR_TO_PIXEL( void * p, int idx, nsfb_colour_t c )
{// byte * ptr= (byte *)p;

//  p[ idx ]= nsfb->palette
  //        ? nsfb_palette_best_match_dither( nsfb->palette, c)
    //      : 0 ;
}



#define PLOT_TYPE byte
#define PLOT_LINELEN(ll) (ll)

#include "common.c"

static bool fill(nsfb_t *nsfb, nsfb_bbox_t *rect, nsfb_colour_t c)
{ int y;
  int8_t ent;
  byte *pvideo;

  if (!nsfbPlotclip_ctx(nsfb, rect))
   return true; /* fill lies outside current clipping region */

  pvideo= get_xy_loc(nsfb, rect->x0, rect->y0);

  COLOR_TO_PIXEL( &ent, 0, c );

  for (y = rect->y0; y < rect->y1; y++)
  { memset(pvideo, ent, rect->x1 - rect->x0);
    pvideo += nsfb->loclen;
  }

  return true;
}

const nsfb_plotter_fns_t _nsfb_8bpp_plotters =
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
