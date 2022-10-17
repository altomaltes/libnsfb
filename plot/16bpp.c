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

static inline void * get_xy_loc( nsfb_t * nsfb, int x, int y)
{ return( nsfb->loc + (y * nsfb->loclen) + ( x << 1 ) );
}

static inline void * get_xy_pan( nsfb_t * nsfb, int x, int y)
{ return( nsfb->pan + (y * nsfb->panlen) + ( x << 1 ) );
}

static inline nsfb_colour_t PIXEL_TO_COLOR( void * p, int idx )
{ word * ptr= (word *)p;
  word   pixel= ptr[ idx ];

  return((( pixel & 0x001F ) << 19 )
        |(( pixel & 0x07E0 ) <<  5 )
        |(( pixel & 0xF800 ) >>  8 ));
}

static inline void COLOR_TO_PIXEL( void * p, int idx, nsfb_colour_t c )
{ word * ptr= ( word *)p;

  ptr[ idx ]=(( c & 0x0000F8) <<  8 )
            |(( c & 0x00FC00) >>  5 )
            |(( c & 0xF80000) >> 19 );
}


#define PLOT_TYPE word
#define PLOT_LINELEN( ll ) ((ll) >> 1)

#include "common.c"


static bool fill( nsfb_t      *nsfb
                , nsfb_bbox_t *rect
                , nsfb_colour_t c )
{ int w;
  word *pvid16;
  word ent16;
  dword *pvid32;
  dword ent32;
  dword llen;
  dword width;
  dword height;

  if (!nsfbPlotclip_ctx(nsfb, rect))
  { return true; /* fill lies outside current clipping region */
  }

  COLOR_TO_PIXEL( &ent16, 0, c );
  width = rect->x1 - rect->x0;
  height = rect->y1 - rect->y0;

  pvid16= get_xy_loc( nsfb, rect->x0, rect->y0 );

  if (((rect->x0 & 1) == 0) && ((width & 1) == 0))   /* aligned to 32bit value and width is even */
  { width = width >> 1;
    llen = (nsfb->loclen >> 2) - width;
    ent32 = ent16 | (ent16 << 16);
    pvid32 = (void *)pvid16;

    while (height-- > 0)
    { w = width;

      while (w >= 16)
      { *pvid32++ = ent32; *pvid32++ = ent32;
        *pvid32++ = ent32; *pvid32++ = ent32;
        *pvid32++ = ent32; *pvid32++ = ent32;
        *pvid32++ = ent32; *pvid32++ = ent32;
        *pvid32++ = ent32; *pvid32++ = ent32;
        *pvid32++ = ent32; *pvid32++ = ent32;
        *pvid32++ = ent32; *pvid32++ = ent32;
        *pvid32++ = ent32; *pvid32++ = ent32;
         w-=16;
      }

      while (w >= 4)
      { *pvid32++ = ent32; *pvid32++ = ent32;
        *pvid32++ = ent32; *pvid32++ = ent32;
         w-=4;
      }

      while (w > 0)
      { *pvid32++ = ent32;
         w--;
      }
       // for (w = width; w > 0; w--) *pvid32++ = ent32;
    pvid32 += llen;
  } }
  else
  { llen = (nsfb->loclen >> 1) - width;


    while (height-- > 0)
    { for (w = width; w > 0; w--) *pvid16++ = ent16;
        pvid16 += llen;
  } }
  return true;
}

const nsfb_plotter_fns_t _nsfb_16bpp_plotters =
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
