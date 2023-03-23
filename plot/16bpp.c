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

static inline void * getXYloc( Nsfb * nsfb, int x, int y)
{ return( nsfb->loc + (y * nsfb->loclen) + ( x << 1 ) );
}

static inline void * getXYpan( Nsfb * nsfb, int x, int y)
{ return( nsfb->pan + (y * nsfb->panlen) + ( x << 1 ) );
}

static inline NSFBCOLOUR PIXEL_TO_COLOR( void * p, int idx )
{ word * ptr= (word *)p;
  word   pixel= ptr[ idx ];

  return((( pixel & 0x001F ) << 19 )
        |(( pixel & 0x07E0 ) <<  5 )
        |(( pixel & 0xF800 ) >>  8 ));
}

static inline void COLOR_TO_PIXEL( void * p, int idx, NSFBCOLOUR c )
{ word * ptr= ( word *)p;

  ptr[ idx ]=(( c & 0x0000F8) <<  8 )
            |(( c & 0x00FC00) >>  5 )
            |(( c & 0xF80000) >> 19 );
}


#define PLOT_TYPE word
#define PLOT_LINELEN( ll ) ((ll) >> 1)

#define surfaceAlphaBitmap surfaceAlphaBitmap16
#define surfaceBitmap surfaceBitmap16

#include "common.c"


static bool fill( Nsfb        * nsfb
                , NsfbBbox * rect
                , NSFBCOLOUR c )
{ int w;
  word *pvid16;
  word ent16;
  dword *pvid32;
  dword ent32;
  dword llen;
  dword width;
  dword height;

  if (!nsfbPlotClipCtx(nsfb, rect))
  { return true; /* fill lies outside current clipping region */
  }

  COLOR_TO_PIXEL( &ent16, 0, c );
  width = rect->x1 - rect->x0;
  height = rect->y1 - rect->y0;

  pvid16= getXYloc( nsfb, rect->x0, rect->y0 );

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

/*
 *
 */
static bool pixmapFill( Nsfb    * nsfb
                     , ImageMap * img
                     , int x, int y
                     , int offy, int capy
                     , NSFBCOLOUR back )
{ if ( img )
  { int overload= (int)img->mask;

    int bmpWidth=  overload &  0xFFFF;    // Unproperly packed
    int bmpHeight= overload >> 16;

    if ( capy )                            // Only redraw a section
    { bmpHeight= capy;
  } }



}

const nsfbPlotterFns _nsfb_16bpp_plotters =
{ .line       = line
, .fill       = fill
, .point      = point
, .bitmap     = bitmap
, .glyph8     = glyph8
, .glyph1     = glyph1
, .readrect   = readrect
, .moverect   = moverect
, .rectangle  = rectangle
, .ellipse    = ellipse
, .ellipseFill= ellipseFill
, .copy       = copy
, .arc        = arc
, .quadratic  = plotQuadratic
, .cubic      = plotCubic
, .path       = plotPath
, .polylines  = polylines
, .polygon    = polygon
, .bitmapTiles= bitmapTilesCommon
, .clg        = clg
, .setClip    = setClip
, .getClip    = getClip

, .pixmapFill = pixmapFill  // NsfbPlotfnBitmap

};
/*
 * Local Variables:
 * c-basic-offset:8
 * End:
 */
