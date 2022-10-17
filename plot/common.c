/*
 * Copyright 2009 Vincent Sanders <vince@simtec.co.uk>
 * Copyright 2010 Michael Drake <tlsa@netsurf-browser.org>
 *
 * Plot code common to all bpp just with differing types
 *
 * This file is part of libnsfb, http://www.netsurf-browser.org/
 * Licenced under the MIT License,
 *                http://www.opensource.org/licenses/mit-license.php
 *
 *   MODIFIED BY: Jose Angel Caso Sanchez, 2004   ( altomaltes@yahoo.es )
 *                                                ( altomaltes@gmail.com )
 *
 */

#ifndef PLOT_TYPE
  #error PLOT_TYPE must be set to byte, word, or dword
#endif

#ifndef PLOT_LINELEN
  #error PLOT_LINELEN must be a macro to increment a line length
#endif

#include <string.h>

#include "../palette.h"
#include "../cursor.h"

#define SIGN(x)  ((x<0) ?  -1  :  ((x>0) ? 1 : 0))

static bool line( nsfb_t * nsfb
                , int linec, nsfb_bbox_t * ln
                , nsfbPlotpen_t * pen )
{ int w;
  PLOT_TYPE ent;
  PLOT_TYPE *pvideo;
  int x, y, i;
  int dx, dy, sdy;
  int dxabs, dyabs;

 // return(0); // !!!


  COLOR_TO_PIXEL( &ent, 0, pen->stroke_colour );

  for (;linec > 0; linec--)
  { nsfb_bbox_t line= *ln++;

    if ( line.y0 == line.y1)                       /* horizontal line special cased */
    { if (!nsfbPlotclip_ctx( nsfb, &line ))       /* line outside clipping */
      { continue;
      }
      pvideo = get_xy_loc(nsfb, line.x0, line.y0);
      w= line.x1 - line.x0;
      while (w-- > 0) *(pvideo + w) = ent;
    }
    else                          /* standard bresenham line */
    { if (!nsfbPlotclip_line_ctx(nsfb, &line))                            /* line outside clipping */
      { continue;
      }

      dx= line.x1 - line.x0; dxabs = abs (dx); /* the horizontal distance of the line */
      dy= line.y1 - line.y0; dyabs = abs (dy); /* the vertical distance of the line */

      sdy = dx ? SIGN(dy) * SIGN(dx) : SIGN(dy);

      if ( dx >= 0) pvideo= get_xy_loc( nsfb, line.x0, line.y0 );
              else  pvideo= get_xy_loc( nsfb, line.x1, line.y1 );

      x = dyabs >> 1;
      y = dxabs >> 1;

      if (dxabs >= dyabs)                           /* the line is more horizontal than vertical */
      { for (i = 0; i < dxabs; i++)
        { *pvideo = ent;
           pvideo++;
           y += dyabs;
           if (y >= dxabs)
           { y -= dxabs;
             pvideo += sdy * PLOT_LINELEN( nsfb->loclen );
       } } }
       else                                 /* the line is more vertical than horizontal */
       { for (i = 0; i < dyabs; i++)
         { *pvideo = ent;
            pvideo += sdy * PLOT_LINELEN( nsfb->loclen );
            x += dxabs;
            if (x >= dyabs)
            { x -= dyabs;
              pvideo++;
  } } } } }

  return( true );
}


static bool point(nsfb_t *nsfb, int x, int y, nsfb_colour_t c)
{ PLOT_TYPE *pvideo;

        /* check point lies within clipping region */
  if ( (x < nsfb->clip.x0)
    || (x >= nsfb->clip.x1)
    || (y < nsfb->clip.y0)
    || (y >= nsfb->clip.y1)) return true;

  pvideo = get_xy_loc(nsfb, x, y);

  if ((c & 0xFF000000) != 0)
  { if ((c & 0xFF000000) != 0xFF000000)
    { c = nsfbPlotAblend(c, PIXEL_TO_COLOR( pvideo, 0 ));
    }
    COLOR_TO_PIXEL( pvideo, 0, c );
  }
  return true;
}

static bool glyph1( nsfb_t      * nsfb
                  , nsfb_bbox_t * loc
                  , const byte  * pixel
                  , int pitch
                  , nsfb_colour_t c )
{ PLOT_TYPE *pvideo;
  PLOT_TYPE const *pvideo_limit;
  PLOT_TYPE fgcol;
  int xloop;
  int xoff, yoff; /* x and y offset into image */
  int x = loc->x0;
  int y = loc->y0;
  int width; int height;
  const size_t line_len = PLOT_LINELEN( nsfb->loclen );
  const byte *row;

  if ( !nsfbPlotclip_ctx(nsfb, loc) )
  { return true;
  }

  height = loc->y1 - y;
  width = loc->x1 - x;

  xoff = loc->x0 - x;
  yoff = loc->y0 - y;

  COLOR_TO_PIXEL( &fgcol, 0, c );

  pitch >>= 3; /* bits to bytes */

  pvideo = get_xy_loc(nsfb, x, loc->y0);
  pvideo_limit = pvideo + line_len * (height - yoff);
  row = pixel + yoff * pitch;

  for (; pvideo < pvideo_limit; pvideo += line_len)
  { for (xloop = xoff; xloop < width; xloop++)
    { if (row[xloop / 8] & ((1<<7) >> (xloop % 8)))
      { *(pvideo + xloop) = fgcol;
    } }
    row += pitch;
  }

  return true;
}

static bool glyph8( nsfb_t      * nsfb
                  , nsfb_bbox_t * loc
                  , const byte  * pixel
                  , int pitch
                  , nsfb_colour_t c, nsfb_colour_t b )
{ PLOT_TYPE *pvideo;
  nsfb_colour_t fgcol;
  nsfb_colour_t abpixel; /* alphablended pixel */
  int xloop, yloop;
  int xoff, yoff;        /* x and y offset into image */
  int x = loc->x0;
  int y = loc->y0;
  int width;
  int height;

  if ( !nsfbPlotclip_ctx(nsfb, loc) )
  { return true;
  }

  height = (loc->y1 - loc->y0);
  width = (loc->x1 - loc->x0);

  xoff = loc->x0 - x;
  yoff = loc->y0 - y;

  pvideo = get_xy_loc(nsfb, loc->x0, loc->y0);

  fgcol = c & 0xFFFFFF;

  for (yloop = 0; yloop < height; yloop++)
  { for (xloop = 0; xloop < width; xloop++)
    { abpixel = (pixel[((yoff + yloop) * pitch) + xloop + xoff] << 24) | fgcol;

      if ((abpixel & 0xFF000000) != 0)                     /* pixel is not transparent */
      { if ((abpixel & 0xFF000000) != 0xFF000000)
        { abpixel = nsfbPlotAblend( abpixel
                                    , PIXEL_TO_COLOR( pvideo, xloop ));
        }
        COLOR_TO_PIXEL( pvideo, xloop,  abpixel);
    } }
    pvideo += PLOT_LINELEN(nsfb->loclen );
  }

  return true;
}

static bool bitmap_scaled( nsfb_t * nsfb
                         , const nsfb_bbox_t * loc
                         , const nsfb_colour_t * pixel
                         , int bmp_width, int bmp_height
                         ,	int bmp_stride, int alpha )
{ PLOT_TYPE *pvideo;
  PLOT_TYPE *pvideo_limit;
  nsfb_colour_t abpixel;         /* alphablended pixel */
  int xloop;
  int xoff, yoff, xoffs;         /* x and y offsets into image */
  int rheight, rwidth;           /* post-clipping render area dimensions */
  int dx, dy;                    /* scale factor (integer part) */
  int dxr, dyr;                  /* scale factor (remainder) */
  int rx, ry, rxs;               /* remainder trackers */
  nsfb_bbox_t clipped;           /* clipped display */
  bool set_dither = false;       /* true iff we enabled dithering here */

  int x     = loc->x0;
  int y     = loc->y0;
  int width = loc->x1 - loc->x0; /* size to scale to */
  int height= loc->y1 - loc->y0; /* size to scale to */

	/* The part of the scaled image actually displayed is cropped to the
	 * current context. */
  clipped.x0 = x;
  clipped.y0 = y;
  clipped.x1 = x + width;
  clipped.y1 = y + height;

  if ( !nsfbPlotclip_ctx(nsfb, &clipped) )
		{ return( true );
		}

	/* get height of rendering region, after clipping
	 */

	 if ( height > (clipped.y1 - clipped.y0) ) 		rheight = (clipped.y1 - clipped.y0);
	 else		rheight = height;

	/* get width of rendering region, after clipping */
	 if (width > (clipped.x1 - clipped.x0))		rwidth = (clipped.x1 - clipped.x0);
	 else		rwidth = width;

	/* Enable error diffusion for paletted screens, if not already on */
	 if ( nsfb->palette != NULL
	   &&	nsfb_palette_dithering_on(nsfb->palette) == false)
 	{ nsfb_palette_dither_init(nsfb->palette, rwidth);
	  	set_dither = true;
  }

	/* get veritcal (y) and horizontal (x) scale factors; both integer
	 * part and remainder */
	 dx = bmp_width / width;
	 dy = (bmp_height / height) * bmp_stride;
	 dxr= bmp_width % width;
	 dyr= bmp_height % height;

	/* get start offsets to part of image being scaled, after clipping and
	 * set remainder trackers to correct starting value */
	 if (clipped.x0 - x != 0)
  { xoffs = ((clipped.x0 - x) * bmp_width) / width;
      rxs = ((clipped.x0 - x) * bmp_width) % width;
	 }
  else
  { xoffs = 0;
    rxs = 0;
  }

  if (clipped.y0 - y != 0)
  { yoff = (((clipped.y0 - y) * bmp_height) / height) * bmp_stride;
      ry = ((clipped.y0 - y) * bmp_height) % height;
  }
  else
  { yoff = 0;
	   ry = 0;
	 }

	/* plot the image
  */

  pvideo= ( alpha & DO_FRONT_RENDER ) ? get_xy_pan( nsfb, clipped.x0, clipped.y0 )
                                      : get_xy_loc( nsfb, clipped.x0, clipped.y0 );
	 pvideo_limit = pvideo + PLOT_LINELEN(nsfb->loclen) * rheight;

  if ( alpha & DO_ALPHA_BLEND )
  { for (; pvideo < pvideo_limit  /* looping through render area vertically */
         ; pvideo += PLOT_LINELEN(nsfb->loclen))
    { xoff = xoffs;
      rx = rxs;

      for (xloop = 0; xloop < rwidth; xloop++)  /* looping through render area horizontally */
      { abpixel = pixel[yoff + xoff]; /* get value of source pixel in question */
		      if ((abpixel & 0xFF000000) != 0)  /* pixel is not transparent; have to plot something */
        { if ((abpixel & 0xFF000000) != 0xFF000000)  /* pixel is not opaque; need to			 * blend */
          { abpixel = nsfbPlotAblend(	abpixel, PIXEL_TO_COLOR( pvideo, xloop ));
          }
          COLOR_TO_PIXEL( pvideo, xloop, abpixel ); /* plot pixel */
        }

				    xoff += dx; /* handle horizontal interpolation */
				    rx += dxr;
				    if (rx >= width)
        { xoff++;
     					rx -= width;
				  }	}

/* handle vertical interpolation
 */
			yoff += dy;
			ry += dyr;
			if (ry >= height)
   { yoff += bmp_stride;
				 ry -= height;
			}	}	}
			else
   {	for (; pvideo < pvideo_limit 	/* looping through render area vertically */
          ;	pvideo += PLOT_LINELEN( nsfb->loclen ))
     { xoff = xoffs;
    			rx = rxs;

				/* looping through render area horizontally */
				/* get value of source pixel in question */
    			for (xloop = 0; xloop < rwidth; xloop++)
    			{ abpixel = pixel[yoff + xoff];
    			  COLOR_TO_PIXEL( pvideo, xloop, abpixel ); 	/* plot pixel */


     				xoff += dx; /* handle horizontal interpolation */
     				rx += dxr;

     				if (rx >= width)
     				{ xoff++;
      					rx -= width;
  				}	 }

			/* handle vertical interpolation */
			   yoff += dy;
	   		ry += dyr;
			   if (ry >= height)
      { yoff += bmp_stride;
				    ry -= height;
		}	}	}

	 if (set_dither)
  { nsfb_palette_dither_fini(nsfb->palette);
	 }

	 return true;
}

#define BITMAPPIXEL( x, y ) pixel[ ]

static bool bitmap( nsfb_t              * nsfb
                  , const nsfb_bbox_t   * loc
                  , const nsfb_colour_t * pixel
                  , int bmp_width, int bmp_height, int bmp_stride
                  , int alpha )
{ PLOT_TYPE *pvideo;
  nsfb_colour_t abpixel; /* alphablended pixel */
  int xloop, yloop;
  int xoff, yoff;        /* x and y offset into image */

  int x= loc->x0;
  int y= loc->y0;

  int width = loc->x1 - loc->x0;
  int height= loc->y1 - loc->y0;
  nsfb_bbox_t clipped;     /* clipped display */
  bool set_dither = false; /* true iff we enabled dithering here */

 // x=y=0;

  if (width == 0 || height == 0)
  { return true;
  }

/* Scaled bitmaps are handled by a separate function
 */
 /* if (width != bmp_width || height != bmp_height)
  { return bitmap_scaled( nsfb, loc, pixel
                        , bmp_width, bmp_height
                        , bmp_stride, alpha );
  }
*/
        /* The part of the image actually displayed is cropped to the
         * current context. */
  clipped.x0 = x;
  clipped.y0 = y;
  clipped.x1 = x + width;
  clipped.y1 = y + height;

  if (!nsfbPlotclip_ctx(nsfb, &clipped))
  { return true;
  }

  if (height > (clipped.y1 - clipped.y0))
  { height = (clipped.y1 - clipped.y0);
  }

  if (width > (clipped.x1 - clipped.x0))
  { width = (clipped.x1 - clipped.x0);
  }

/* Enable error diffusion for paletted screens, if not already on
 */
  if ( nsfb->palette != NULL
  && nsfb_palette_dithering_on(nsfb->palette) == false )
  { nsfb_palette_dither_init(nsfb->palette, width);
    set_dither = true;
  }

  xoff = clipped.x0 - x;
  yoff = (clipped.y0 - y) * bmp_stride;
  height = height * bmp_stride + yoff;

/* plot the image
 */
  pvideo= ( alpha & DO_FRONT_RENDER ) ? get_xy_pan( nsfb, clipped.x0, clipped.y0 )
                                      : get_xy_loc( nsfb, clipped.x0, clipped.y0 );

  if ( alpha & DO_ALPHA_BLEND )
  { for (yloop = yoff; yloop < height; yloop += bmp_stride)
    { for (xloop = 0; xloop < width; xloop++)
      { abpixel = pixel[yloop + xloop + xoff];

        switch( abpixel & 0xFF000000 )
        {         default: abpixel= nsfbPlotAblend( abpixel
                                                    , PIXEL_TO_COLOR( pvideo, xloop )); /* opaque */
          case 0x00000000: COLOR_TO_PIXEL( pvideo, xloop, abpixel ); /* Mixed */
          case 0xFF000000: break;             /* Transparent */
    } }
    pvideo += PLOT_LINELEN( nsfb->loclen );
  } }
  else
  { for ( yloop = yoff; yloop < height; yloop += bmp_stride )
    { for (xloop = 0; xloop < width; xloop++)
      { COLOR_TO_PIXEL( pvideo, xloop, pixel[ yloop + xloop + xoff ] );
      }
      pvideo += PLOT_LINELEN( nsfb->loclen );
  } }

  if ( set_dither )
  { nsfb_palette_dither_fini(nsfb->palette);
  }

  return true;
}

static bool bitmap1( nsfb_t             * nsfb
                  , const nsfb_bbox_t   * loc
                  , const nsfb_colour_t * pixel
                  , int   bmp_width
                  , int   bmp_height
                  , int   bmp_stride
                  , int alpha )
{ PLOT_TYPE * porg;
  nsfb_colour_t abpixel; /* alphablended pixel */

  int xloop , yloop;
  int xoff  , yoff;   /* x and y offset into image */
  int xshift, yshift; /* x and y offset into image */

  int x     = loc->x0;
  int y     = loc->y0;
  int width = loc->x1 - loc->x0;
  int height= loc->y1 - loc->y0;

  int xstep= 1;
  int ystep= PLOT_LINELEN( nsfb->loclen ); /* x and y offset into image */

  nsfb_bbox_t clipped; /* clipped display */
  bool set_dither = false; /* true iff we enabled dithering here */

  if (width == 0 || height == 0)
  { return true;
  }



/* Scaled bitmaps are handled by a separate function
 */
 /* if (width != bmp_width || height != bmp_height)
  { return bitmap_scaled( nsfb, loc, pixel
                        , bmp_width, bmp_height
                        , bmp_stride, alpha );
  }
*/
        /* The part of the image actually displayed is cropped to the
         * current context. */
  clipped.x0= x;
  clipped.y0= y;
  clipped.x1= x + width;
  clipped.y1= y + height;

  if (!nsfbPlotclip_ctx(nsfb, &clipped))
  { return true;
  }

  if (height > (clipped.y1 - clipped.y0))
  { height = (clipped.y1 - clipped.y0);
  }

  if (width > (clipped.x1 - clipped.x0))
  { width = (clipped.x1 - clipped.x0);
  }

/* Enable error diffusion for paletted screens, if not already on
 */
  if (nsfb->palette != NULL
  && nsfb_palette_dithering_on(nsfb->palette) == false )
  { nsfb_palette_dither_init(nsfb->palette, width);
    set_dither = true;
  }

  xoff =  clipped.x0 - x;
  yoff = (clipped.y0 - y) * bmp_stride;
  height = height * bmp_stride + yoff;

/* plot the image
 */
  porg= ( alpha & DO_FRONT_RENDER ) ? get_xy_pan( nsfb, clipped.x0, clipped.y0 )
                                    : get_xy_loc( nsfb, clipped.x0, clipped.y0 );

  if ( alpha & DO_ALPHA_BLEND )
  { for ( yloop= yshift= yoff
        ; yloop < height
        ; yloop += bmp_stride, yshift+= ystep )
    { for ( xloop= xshift= 0
          ; xloop < width
          ; xloop++, xshift+= xstep )
      { PLOT_TYPE * pvideo= porg + xshift + yshift;

        abpixel= pixel[ yloop + xloop + xoff ];

        switch( abpixel & 0xFF000000 )
        {         default: abpixel= nsfbPlotAblend( abpixel
                                                    , PIXEL_TO_COLOR( pvideo, 0 )); /* opaque */
          case 0x00000000: COLOR_TO_PIXEL( pvideo, 0, abpixel );                    /* Mixed */
          case 0xFF000000: break;                                                   /* Transparent */
  } } } }

  else

  { for ( yloop= yshift= yoff
        ; yloop < height
        ; yloop += bmp_stride, yshift+= ystep )
    { for ( xloop= xshift= 0
          ; xloop < width
          ; xloop++, xshift+= xstep )
      { PLOT_TYPE *pvideo= porg + xshift + yshift;

        COLOR_TO_PIXEL( pvideo, 0, pixel[ yloop + xloop + xoff ] );
  } } }

  if ( set_dither )
  { nsfb_palette_dither_fini( nsfb->palette );
  }

  return true;
}

static inline bool bitmap_tiles_x( nsfb_t * nsfb
                                 , const nsfb_bbox_t * loc
                                 , int tiles_x
                                 , const nsfb_colour_t *pixel
                                 , int bmp_stride
                                 , int alpha )
{ PLOT_TYPE * pvideo;
  PLOT_TYPE * pvideo_pos;
  PLOT_TYPE * pvideo_limit;

  nsfb_colour_t abpixel; /* alphablended pixel */
  int xloop;
  int xoff, yoff; /* x and y offset into image */
  int xlim;
  int t;
  int x = loc->x0;
  int y = loc->y0;
  int width = loc->x1 - loc->x0;
  int height = loc->y1 - loc->y0;
  nsfb_bbox_t clipped; /* clipped display */

  if (width == 0 || height == 0)
  { return true;
  }

	/* The part of the image actually displayed is cropped to the
	 * current context. */
	 clipped.x0 = x;
	 clipped.y0 = y;
	 clipped.x1 = x + width * tiles_x;
	 clipped.y1 = y + height;

	 if (!nsfbPlotclip_ctx(nsfb, &clipped))
		{ return true;
  }

	 if (height > (clipped.y1 - clipped.y0))
	 { height = (clipped.y1 - clipped.y0);
  }

  pvideo= ( alpha & DO_FRONT_RENDER ) ? get_xy_pan( nsfb, clipped.x0, clipped.y0 )
                                      : get_xy_loc( nsfb, clipped.x0, clipped.y0 );

	 pvideo_limit = pvideo + PLOT_LINELEN(nsfb->loclen) * height;

	 xoff = clipped.x0 - x;
	 xlim = width - (x + width * tiles_x - clipped.x1);
	 yoff = (clipped.y0 - y) * bmp_stride;

/* plot the image
 */
  if ( alpha & DO_ALPHA_BLEND  )
 	{ for(
	      ; pvideo < pvideo_limit
       ; pvideo += PLOT_LINELEN(nsfb->loclen))
   { pvideo_pos = pvideo;
			for (t = 0; t < 1; t++)
			{ for (xloop = xoff; xloop < width; xloop++)
			  { abpixel = pixel[yoff + xloop];
  					if ((abpixel & 0xFF000000) ) 		               /* pixel is not transparent; have to plot something */
		  			{ if ((abpixel & 0xFF000000) !=	0xFF000000)			/* pixel is not opaque;							 * need to blend */
		  			  { abpixel =
    							nsfbPlotAblend( abpixel
				   			                , PIXEL_TO_COLOR( pvideo, xloop -xoff ));
						}
			  COLOR_TO_PIXEL( pvideo, xloop-xoff, abpixel );
			} } }

			pvideo_pos += width - xoff;
			for (; t < tiles_x - 1; t++) {
				for (xloop = 0; xloop < width; xloop++) {
					abpixel = pixel[yoff + xloop];
						/* pixel is not transparent;						 * have to plot something */
					if ((abpixel & 0xFF000000) != 0)
     {
						if ((abpixel & 0xFF000000) !=
								0xFF000000) {
							/* pixel is not opaque; * need to blend */
							abpixel =
							nsfbPlotAblend(abpixel
							                , PIXEL_TO_COLOR( pvideo, xloop -xoff ));
						}
			  COLOR_TO_PIXEL( pvideo, xloop, abpixel );
			} }
			pvideo_pos += width;
			}

			for (; t < tiles_x; t++) 
   { for (xloop = 0; xloop < xlim; xloop++) 
     { abpixel = pixel[yoff + xloop];
			   	if ((abpixel & 0xFF000000) != 0)  /* pixel is not transparent;	 * have to plot something */
       { if ((abpixel & 0xFF000000) !=	0xFF000000)  /* pixel is not opaque; 	 * need to blend */
         { abpixel =
    							nsfbPlotAblend(	abpixel
				  			                , PIXEL_TO_COLOR( pvideo, xloop ));
						   }
			      COLOR_TO_PIXEL( pvideo, xloop, abpixel );
			 }	}	}
					yoff += bmp_stride;
		} } 
  else 
  { for (; pvideo < pvideo_limit;
				pvideo += PLOT_LINELEN(nsfb->loclen))
				{
			pvideo_pos = pvideo;
			for (t = 0; t < 1; t++)
			{ for (xloop = xoff; xloop < width; xloop++)
			  { abpixel = pixel[yoff + xloop];
 			  COLOR_TO_PIXEL( pvideo, xloop-xoff, abpixel );
			}	}
			pvideo_pos += width - xoff;

			for (; t < tiles_x - 1; t++)
   { for (xloop = 0; xloop < width; xloop++)
     {	abpixel = pixel[yoff + xloop];
			  COLOR_TO_PIXEL( pvideo, xloop, abpixel );
				}
				pvideo_pos += width;
			}
			for (; t < tiles_x; t++)
			{ for (xloop = 0; xloop < xlim; xloop++)
			  { abpixel = pixel[yoff + xloop];
  			  COLOR_TO_PIXEL( pvideo, xloop, abpixel );
			}	}
			yoff += bmp_stride;
  } }

	return true;
}

static bool bitmap_tiles( nsfb_t            * nsfb
                        ,	const nsfb_bbox_t * loc
                        ,	int tiles_x, int tiles_y
                        ,	const nsfb_colour_t *pixel
                        ,	int bmp_width,	int bmp_height
                        ,	int bmp_stride, int alpha )
{ nsfb_bbox_t render_area;
  nsfb_bbox_t tloc;
  int tx, ty;
  int width = loc->x1 - loc->x0;
  int height = loc->y1 - loc->y0;
  int skip = 0;
  bool ok = true;
  bool scaled = (width != bmp_width) || (height != bmp_height);
  bool set_dither = false; /* true iff we enabled dithering here */

/* Avoid pointless rendering
 */
  if (width == 0 || height == 0)
  {	return true;
  }

  render_area.x0 = loc->x0;
  render_area.y0 = loc->y0;
  render_area.x1 = loc->x0 + width * tiles_x;
  render_area.y1 = loc->y0 + height * tiles_y;

	if (!nsfbPlotclip_ctx(nsfb, &render_area))
		return true;

/*   Enable error diffusion for paletted screens, if not already on,
 * if not scaled or if not repeating in x direction
 */
  if ((!scaled || tiles_x == 1)
    && nsfb->palette != NULL
    && nsfb_palette_dithering_on(nsfb->palette) == false)
  { nsfb_palette_dither_init( nsfb->palette
                            , render_area.x1 - render_area.x0 );
  	 set_dither = true;
  }

  /* Given tile location is top left; start with that one.
   */
  tloc = *loc;

  if (render_area.x0 - tloc.x0 > width)
  { skip = (render_area.x0 - tloc.x0) / width;
  	tiles_x -= skip;
  	skip *= width;
  	tloc.x0 += skip;
  	tloc.x1 += skip;
  }

  if (tloc.x1 - render_area.x1 > width)
  { tiles_x -= (tloc.x1 - render_area.x1) / width;
  }

  if (scaled)  	/* Scaled */
  { for (ty = 0; ty < tiles_y; ty++)
    { for (tx = 0; tx < tiles_x; tx++)
      { ok &= bitmap_scaled( nsfb, &tloc, pixel,
						  bmp_width, bmp_height,
						  bmp_stride, alpha);
				    tloc.x0 += width;
				    tloc.x1 += width;
			}
			tloc.x0 = loc->x0;
			tloc.y0 += height;
			tloc.x1 = loc->x1;
			tloc.y1 += height;
  }	}
  else  /* Unscaled */
  { if (tiles_x == 1 || !set_dither)
    { for (ty = 0; ty < tiles_y; ty++)
      { for (tx = 0; tx < tiles_x; tx++)
        { ok &= bitmap(nsfb, &tloc, pixel,
							   bmp_width, bmp_height,
   							bmp_stride, alpha);
		        tloc.x0 += width;
     		   tloc.x1 += width;
	       }
      		tloc.x0 = loc->x0 + skip;
       	tloc.y0 += height;
      		tloc.x1 = loc->x1 + skip;
      		tloc.y1 += height;
  } }
  else if (tiles_x > 1)
  { for (ty = 0; ty < tiles_y; ty++)
    { ok &= bitmap_tiles_x( nsfb, &tloc, tiles_x,
						pixel, bmp_stride, alpha);
		  		tloc.y0 += height;
				  tloc.y1 += height;
  } } }

  if (set_dither)
  { nsfb_palette_dither_fini(nsfb->palette);
  }

  return( ok );
}

static bool readrect( nsfb_t        * nsfb
                    , nsfb_bbox_t   * rect
                    , nsfb_colour_t * buffer )
{ PLOT_TYPE *pvideo;
  int xloop, yloop;
  int width;

  if (!nsfbPlotclip_ctx(nsfb, rect))
  { return true;
  }

  width = rect->x1 - rect->x0;
  pvideo= get_xy_loc(nsfb, rect->x0, rect->y0);

  for (yloop = rect->y0; yloop < rect->y1; yloop += 1)
  { for (xloop = 0; xloop < width; xloop++)
    { *buffer++ = PIXEL_TO_COLOR( pvideo, xloop );
    }
    pvideo += PLOT_LINELEN(nsfb->loclen);
  }
  return true;
}


static int moverect( nsfb_t * nsfb
                   , int  w, int h
                   , int  x, int y )
{ int xoff, yoff, sz, reverse= 0;
  int width= PLOT_LINELEN( nsfb->loclen );
  struct nsfbCursor_s * cursor;

  PLOT_TYPE * ptr;
  PLOT_TYPE * dst;

  PLOT_TYPE * off= nsfb->pan;
  PLOT_TYPE * src;


/*  Direction overloaded
 */
  if ( w < 0 ) { w= -w; reverse++; }
  if ( h < 0 ) { h= -h; reverse++; }

/* Clip
 */
  if ( x < 0 ) { w -= x; x= 0; }
  if ( y < 0 ) { h -= y; y= 0; }

  if ( w <= 0 || h <= 0 )   /* Alles done */
  { return( 0 );
  }

  if ( ( x + w ) > nsfb->width  ) { w= nsfb->width - x; }
  if ( ( y + h ) > nsfb->height ) { h= nsfb->height- y; }

  switch ( nsfb->rotate  )
  { case NSFB_ROTATE_NORTH:
      if ( reverse )
      { dst= get_xy_loc( nsfb, x, y );
        src= get_xy_pan( nsfb, x, y );
      }
      else
      { dst= get_xy_pan( nsfb, x, y );
        src= get_xy_loc( nsfb, x, y );
      }

      while( h-- )
      { memcpy( dst, src, w * sizeof( PLOT_TYPE ));
        dst += width; src += width;
      }
    break;

    case NSFB_ROTATE_WEST:
      w += x; h += y;         /* endpoint */
      width= nsfb->width - x;

      for( yoff= y
         ; yoff< h
         ; yoff++ )
      { if ( reverse )
        { src= get_xy_pan( nsfb,    x, yoff  );
          dst= get_xy_loc( nsfb, yoff, width );
        }
        else
        { src= get_xy_loc( nsfb,    x, yoff  );
          dst= get_xy_pan( nsfb, yoff, width );
        }

        for( xoff= x
           ; xoff< w
           ; xoff++, src++ )
        { dst-= nsfb->height;
         *dst= *src;
      } }
    break;

    case NSFB_ROTATE_EAST:
      w += x; h += y;         /* endpoint */
      width= nsfb->height - y;

      for( yoff= y
         ; yoff< h
         ; yoff++, width-- )
      { if ( reverse )
        { src= get_xy_pan( nsfb,     x, yoff );
          dst= get_xy_loc( nsfb, width, x    );
        }
        else
        { src= get_xy_loc( nsfb,     x, yoff );
          dst= get_xy_pan( nsfb, width, x    );
        }

        for( xoff= x
           ; xoff< w
           ; xoff++, src++, dst+= nsfb->height )
        { *dst= *src;
      } }
    break;

    case NSFB_ROTATE_SOUTH:
      w += x; h += y;         /* endpoint */
      width= nsfb->height - y;

      for( yoff= y
         ; yoff< h
         ; yoff++, width-- )
      { if ( reverse )
        { src= get_xy_pan( nsfb,               x,  yoff  );
          dst= get_xy_loc( nsfb, nsfb->width - x,  width );
        }
        else
        { src= get_xy_loc( nsfb,               x,  yoff  );
          dst= get_xy_pan( nsfb, nsfb->width - x,  width );
        }

        for( xoff= x
           ; xoff< w
           ; xoff++, src++, dst-- )
        { *dst= *src;
      } }
    break;
  }

/*  Test for cursor restamp
 */
  if (( cursor= nsfb->cursor ))
  { nsfb_bbox_t box= { x-w, y-h
                     , x+w, y+h };

   // if ( nsfbPlotbbox_intersect( &box, &cursor->savLoc ))
    { nsfb->plotter_fns->bitmap( nsfb
                               , &cursor->savLoc
                               ,  cursor->pixel
                               ,  cursor->bmp_width
                               ,  cursor->bmp_height, cursor->bmp_stride
                               , DO_ALPHA_BLEND | DO_FRONT_RENDER  );
  } }

  return( 0 );
}


/*
 * Local Variables:
 * c-basic-offset:8
 * End:
 */
