/*
 * Copyright 2009 Vincent Sanders <vince@simtec.co.uk>
 *
 * This file is part of libnsfb, http://www.netsurf-browser.org/
 * Licenced under the MIT License,
 *                http://www.opensource.org/licenses/mit-license.php
 */

#define __BYTE_ORDER __BYTE_ORDER__
#define __BIG_ENDIAN __ORDER_BIG_ENDIAN__

#include <string.h>
#include "../libnsfb_plot_util.h"
#include "../nsfb.h"
#include "../plot.h"

#define COLOR_TO_PIXEL( p, idx, c ) \
p[ idx*3+2 ]= c & 0xFF; c >>= 8; \
p[ idx*3+1 ]= c & 0xFF; c >>= 8; \
p[ idx*3+0 ]= c & 0xFF; c >>= 8

#define PIXEL_TO_COLOR( p, idx ) \
((dword)p[ idx*3+2 ] << 0  )| \
((dword)p[ idx*3+1 ] << 8  )| \
((dword)p[ idx*3+0 ] << 16 )


static inline void * get_xy_loc( nsfb_t *nsfb
                               , int x, int y )
{ return (byte *)(nsfb->loc + (y * nsfb->loclen) + (x * 3));
}

static inline void * get_xy_pan( nsfb_t *nsfb
                               , int x, int y )
{ return (byte *)(nsfb->pan + (y * nsfb->panlen) + (x * 3));
}

#define SIGN(x)  ((x<0) ?  -1  :  ((x>0) ? 1 : 0))

static bool line( nsfb_t *nsfb, int linec, nsfb_bbox_t *line, nsfbPlotpen_t *pen )
{ int w;
  dword c;
  byte * pvideo;
  int x, y, i;
  int dx, dy, sdy;
  int dxabs, dyabs;

  c=pen->stroke_colour;


  for (;linec > 0; linec--)
  { if (line->y0 == line->y1)                      /* horizontal line special cased */
    { if (!nsfbPlotclip_ctx(nsfb, line))         /* line outside clipping */
      { line++;
        continue;
      }
      pvideo = get_xy_loc(nsfb, line->x0, line->y0);

      w = line->x1 - line->x0;

      while ( w-- )
      { *pvideo++ =  c          & 0xFF;
        *pvideo++ = ( c >> 8  ) & 0xFF;
        *pvideo++ = ( c >> 16 ) & 0xFF;
    } }
    else                    /* standard bresenham line */
    { if (!nsfbPlotclip_line_ctx(nsfb, line))
      { line++;                        /* line outside clipping */
        continue;
      }

/* the horizontal distance of the line */
      dx = line->x1 - line->x0;
      dxabs = abs (dx);

/* the vertical distance of the line */
      dy = line->y1 - line->y0;
      dyabs = abs (dy);

      sdy = dx ? SIGN(dy) * SIGN(dx) : SIGN(dy);

      if (dx >= 0)  pvideo = get_xy_loc(nsfb, line->x0, line->y0);
               else pvideo = get_xy_loc(nsfb, line->x1, line->y1);

      x = dyabs >> 1;
      y = dxabs >> 1;

      if (dxabs >= dyabs)                           /* the line is more horizontal than vertical */
      { for (i = 0; i < dxabs; i++)
        { *pvideo++ =   c         & 0xFF;
          *pvideo++ = ( c >> 8  ) & 0xFF;
          *pvideo++ = ( c >> 16 ) & 0xFF;

           y += dyabs;
           if (y >= dxabs)
           { y -= dxabs;
             pvideo += sdy * nsfb->loclen;
      } } }
      else                          /* the line is more vertical than horizontal */
      { for (i = 0; i < dyabs; i++)
        { pvideo[0] =   c         & 0xFF;
          pvideo[1] = ( c >> 8  ) & 0xFF;
          pvideo[2] = ( c >> 16 ) & 0xFF;

          pvideo += sdy * nsfb->loclen;
          x += dxabs;
          if (x >= dyabs)
          { x -= dyabs;
            pvideo +=  3;
    } } } }

    line++;
  }
  return true;
}



static bool fill( nsfb_t      * nsfb
                , nsfb_bbox_t * rect
                , nsfb_colour_t c )
{ int w;
  byte *pvid;
  dword llen;
  dword width;
  dword height;

  if (!nsfbPlotclip_ctx(nsfb, rect))
                return true; /* fill lies outside current clipping region */

  width = rect->x1 - rect->x0;
  height= rect->y1 - rect->y0;
  llen  = nsfb->loclen - width * 3;

  pvid = get_xy_loc(nsfb, rect->x0, rect->y0);

  while ( height-- > 0)
  { w = width;

    while ( w-- )
    { *pvid++ =   c         & 0xFF;
      *pvid++ = ( c >> 8  ) & 0xFF;
      *pvid++ = ( c >> 16 ) & 0xFF;
    }

    pvid += llen;
  }

  return true;
}


static bool point(nsfb_t *nsfb, int x, int y, nsfb_colour_t c)
{ byte * pvideo;

        /* check point lies within clipping region */
  if ( ( x <  nsfb->clip.x0 )
    || ( x >= nsfb->clip.x1 )
    || ( y <  nsfb->clip.y0 )
    || ( y >= nsfb->clip.y1 ))
  { return true;
  }

  pvideo = get_xy_loc(nsfb, x, y);

  if ((c & 0xFF000000) != 0)
  { if ((c & 0xFF000000) != 0xFF000000)
    { c= nsfbPlotAblend( c, PIXEL_TO_COLOR( pvideo, 0 ) );
    }

   COLOR_TO_PIXEL( pvideo, 0, c );
  }

  return( true );
}

static bool glyph1( nsfb_t      * nsfb
                  , nsfb_bbox_t * loc
                  , const byte  * pixel
                  , int pitch
                  , nsfb_colour_t c )
{ byte *pvideo;
  int xloop, yloop;
  int xoff, yoff; /* x and y offset into image */
  int x = loc->x0;
  int y = loc->y0;
  int width = loc->x1 - loc->x0;
  int height= loc->y1 - loc->y0;
  const byte *fntd;
  byte row;

  if ( !nsfbPlotclip_ctx(nsfb, loc)) return true;
  if ( height > (loc->y1 - loc->y0  )) height= (loc->y1 - loc->y0);
  if ( width  > (loc->x1 - loc->x0  )) width = (loc->x1 - loc->x0);

  xoff= loc->x0 - x;
  yoff= loc->y0 - y;

  pvideo= get_xy_loc( nsfb, loc->x0, loc->y0 );

  for (yloop = yoff; yloop < height; yloop++)
  { fntd = pixel + (yloop * (pitch>>3)) + (xoff>>3);
    row = (*fntd++) << (xoff & 3);
    for (xloop = xoff; xloop < width ; xloop++)
    { if (((xloop % 8) == 0) && (xloop != 0))
      { row = *fntd++;
      }

      if ( row & 0x80 )
      { COLOR_TO_PIXEL( pvideo, xloop, c );
      }
      row = row << 1;
    }
    pvideo += nsfb->loclen;
  }

  return true;
}

static bool glyph8( nsfb_t        * nsfb
                  , nsfb_bbox_t   * loc
                  , const byte    * pixel
                  , int pitch
                  , nsfb_colour_t color, nsfb_colour_t back )
{ byte *pvideo;
  nsfb_colour_t abpixel; /* alphablended pixel */
  int xloop, yloop;
  int  xoff, yoff;       /* x and y offset into image */
  int x = loc->x0;
  int y = loc->y0;
  int width = loc->x1 - loc->x0;
  int height= loc->y1 - loc->y0;

  if ( !nsfbPlotclip_ctx(nsfb, loc)) return true;
  if ( height > (loc->y1 - loc->y0  )) height = (loc->y1 - loc->y0);
  if ( width  > (loc->x1 - loc->x0  )) width  = (loc->x1 - loc->x0);

  xoff = loc->x0 - x;
  yoff = loc->y0 - y;

  pvideo = get_xy_loc( nsfb, loc->x0, loc->y0 );

  for (yloop = 0; yloop < height; yloop++)
  { byte * ptr = pixel + ( yoff + yloop ) * pitch;
           ptr+= xoff;

    for (xloop = 0; xloop < width; xloop++)
    { dword pix= *ptr++ << 24;
      abpixel= color & 0xFFFFFF;

      switch( pix )
      { case 0x00000000: break;             /* Transparent */
        default        : abpixel= nsfbPlotAblend( abpixel | pix , PIXEL_TO_COLOR( pvideo, xloop )); /* opaque */
        case 0xFF000000: COLOR_TO_PIXEL( pvideo, xloop, abpixel ); /* Mixed */
    } }
    pvideo += nsfb->loclen;
  }
  return true;
}


static bool bitmap( nsfb_t * nsfb
                  , const nsfb_bbox_t   * loc
                  , const nsfb_colour_t * pixel
                  , int bmp_width, int bmp_height
                  , int bmp_stride, int alpha )
{ byte *pvideo;
  nsfb_colour_t abpixel = 0; /* alphablended pixel */
  int xloop, yloop;
  int xoff, yoff;            /* x and y offset into image */
  int x = loc->x0;
  int y = loc->y0;
  int width = loc->x1 - loc->x0;
  int height = loc->y1 - loc->y0;
  nsfb_bbox_t clipped; /* clipped display */

/*   TODO here we should scale the image from bmp_width to width, for
 * now simply crop.
 */

  if ( width > bmp_width   ) width = bmp_width;
  if ( height > bmp_height ) height = bmp_height;

/*  The part of the scaled image actually displayed is cropped to the
 * current context.
 */

  clipped.x0 = x;
  clipped.y0 = y;
  clipped.x1 = x + width;
  clipped.y1 = y + height;

  if (!nsfbPlotclip_ctx(nsfb, &clipped))
  { return true;
  }

  if ( height > (clipped.y1 - clipped.y0)) height= (clipped.y1 - clipped.y0);
  if (  width > (clipped.x1 - clipped.x0))  width= (clipped.x1 - clipped.x0);

  xoff = clipped.x0 - x;
  yoff = (clipped.y0 - y) * bmp_width;
  height = height * bmp_stride + yoff;

        /* plot the image */
  pvideo= ( alpha & DO_FRONT_RENDER ) ? get_xy_pan( nsfb, clipped.x0, clipped.y0 )
                                      : get_xy_loc( nsfb, clipped.x0, clipped.y0 );

  if ( alpha & DO_ALPHA_BLEND )
  { for (yloop = yoff; yloop < height; yloop += bmp_stride)
    { for (xloop = 0; xloop < width; xloop++)
      { abpixel = pixel[ yloop + xloop + xoff ];

      switch( abpixel & 0xFF000000 )
      { case 0xFF000000: break;             /* Transparent */
                default: abpixel= nsfbPlotAblend( abpixel
                                                , PIXEL_TO_COLOR( pvideo, xloop )); /* opaque */
        case 0x00000000: COLOR_TO_PIXEL( pvideo, xloop, abpixel ); /* Mixed */
      } }
      pvideo += nsfb->loclen;
  } }
  else
  { for (yloop = yoff; yloop < height; yloop += bmp_stride)
    { for (xloop = 0; xloop < width; xloop++)
      { abpixel = pixel[ yloop + xloop + xoff ];
        COLOR_TO_PIXEL( pvideo, xloop, abpixel );
      }
      pvideo += nsfb->loclen;
  } }
  return true;
}

static bool readrect( nsfb_t *nsfb
                    , nsfb_bbox_t *rect
                    , nsfb_colour_t *buffer )
{ byte *pvideo;
  int xloop, yloop;
  int width;

  if (!nsfbPlotclip_ctx(nsfb, rect))
  { return true;
  }

  width = rect->x1 - rect->x0;

  pvideo= get_xy_loc(nsfb, rect->x0, rect->y0);

  for( yloop = rect->y0
     ; yloop < rect->y1
     ; yloop ++ )
  { for ( xloop = 0
        ; xloop < width
        ; xloop++ )
    { *buffer++ =  PIXEL_TO_COLOR( pvideo, xloop );
    }
    pvideo += nsfb->loclen;
  }

  return true;
}

static int moverect( nsfb_t * nsfb
                   , int w, int h
                   , int x, int y )
{ /*dword * src= (dword*)get_xy_pan( nsfb, x, y );
  dword * dst= (dword*)get_xy_loc( nsfb, x, y );

  int width= nsfb->linelen >> 2;

  while( h-- )
  { memcpy( dst, src, w * sizeof( dword ));
    dst += width; src += width;
  }

  return( 0 );   */

  puts("24ppp");
}


const nsfb_plotter_fns_t _nsfb_24bpp_plotters =
{ .line    = line
, .fill    = fill
, .point   = point
, .bitmap  = bitmap
, .glyph8  = glyph8
, .glyph1  = glyph1
, .readrect= readrect
, .moverect= moverect
};

/*
 * Local Variables:
 * c-basic-offset:8
 * End:
 */
