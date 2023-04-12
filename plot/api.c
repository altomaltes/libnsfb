/* public plotter interface */


#include "../nsfb.h"
#include "../nsfbPlot.h"
#include "../plot.h"
#include "../surface.h"

/** Sets a clip rectangle for subsequent plots.
 *
 * Sets a clipping area which constrains all subsequent plotting operations.
 * The clipping area must lie within the framebuffer visible screen or false
 * will be returned and the new clipping area not set.
 */
ANSIC bool nsfbPlotsetClip(Nsfb *nsfb, NsfbBbox *clip)
{ return nsfb->plotterFns->setClip(nsfb, clip);
}

/** Get the previously set clipping region.
 */
ANSIC bool nsfbPlotgetClip(Nsfb *nsfb, NsfbBbox *clip)
{ return nsfb->plotterFns->getClip(nsfb, clip);
}

/** Clears plotting area to a flat colour.
 */
ANSIC bool nsfbPlotclg( Nsfb *nsfb, NSFBCOLOUR c )
{ return( nsfb->plotterFns->clg( nsfb, c ) );
}

/** Plots a rectangle outline.
 *
 * The line can be solid, dotted or dashed. Top left corner at (x0,y0) and
 * rectangle has given width and height.
 */
ANSIC bool nsfbPlotrectangle( Nsfb *nsfb
                        , NsfbBbox *rect
                        , int line_width
                        , NSFBCOLOUR c
                        , bool dotted, bool dashed)
{ return( nsfb->plotterFns->rectangle(nsfb, rect, line_width, c, dotted, dashed ));
}

/** Plots a filled rectangle. Top left corner at (x0,y0), bottom
 *		  right corner at (x1,y1). Note: (x0,y0) is inside filled area,
 *		  but (x1,y1) is below and to the right. See diagram below.
 */
ANSIC bool nsfbPlotrectangleFill( Nsfb *nsfb, NsfbBbox *rect, NSFBCOLOUR c )
{ return( nsfb->plotterFns->fill(nsfb, rect, c ));
}

/** Plots a line.
 *
 * Draw a line from (x0,y0) to (x1,y1). Coordinates are at centre of line
 * width/thickness.
 */
ANSIC bool nsfbPlotline(Nsfb *nsfb, NsfbBbox *line, NsfbPlotpen *pen)
{ return( nsfb->plotterFns->line(nsfb, 1, line, pen ));
}

/** Plots more than one line.
 *
 * Draw a line from (x0,y0) to (x1,y1). Coordinates are at centre of line
 * width/thickness.
 */
ANSIC bool nsfbPlotlines(Nsfb *nsfb, int linec, NsfbBbox *line, NsfbPlotpen *pen)
{ return nsfb->plotterFns->line(nsfb, linec, line, pen);
}

ANSIC bool nsfbPlotpolylines(Nsfb *nsfb, int pointc, const NsfbPoint *points, NsfbPlotpen *pen)
{ return nsfb->plotterFns->polylines(nsfb, pointc, points, pen);
}

/** Plots a filled polygon.
 *
 * Plots a filled polygon with straight lines between points. The lines around
 * the edge of the ploygon are not plotted. The polygon is filled with a
 * non-zero winding rule.
 *
 *
 */
ANSIC bool nsfbPlotpolygon(Nsfb *nsfb, const int *p, unsigned int n, NSFBCOLOUR fill)
{ return nsfb->plotterFns->polygon(nsfb, p, n, fill);
}

/** Plots an arc.
 *
 * around (x,y), from anticlockwise from angle1 to angle2. Angles are measured
 * anticlockwise from horizontal, in degrees.
 */
ANSIC bool nsfbPlotarc( Nsfb *nsfb
                         , int x, int y, int radius
                         , int angle1, int angle2
                         , NSFBCOLOUR c)
{ return nsfb->plotterFns->arc(nsfb, x, y, radius, angle1, angle2, c);
}

/** Plots an alpha blended pixel.
 *
 * plots an alpha blended pixel.
 */
ANSIC bool nsfbPlotpoint(Nsfb *nsfb, int x, int y, NSFBCOLOUR c)
{ return nsfb->plotterFns->point(nsfb, x, y, c);
}

ANSIC bool nsfbPlotellipse(Nsfb *nsfb, NsfbBbox *ellipse, NSFBCOLOUR c)
{ return nsfb->plotterFns->ellipse(nsfb, ellipse, c);
}

ANSIC bool nsfbPlotellipseFill(      Nsfb * nsfb, NsfbBbox *ellipse, NSFBCOLOUR c)
{ return( nsfb->plotterFns->ellipseFill(nsfb, ellipse, c ));
}


/* copy an area of surface from one location to another.
 *
 * @warning This implementation is woefully incomplete!
 */
ANSIC bool nsfbPlotcopy( Nsfb * srcfb, NsfbBbox * srcbox
                       , Nsfb * dstfb, NsfbBbox * dstbox )
{ int trans = 0;
  NSFBCOLOUR srccol;

  if ( srcfb == dstfb )
  { return( dstfb->plotterFns->copy(srcfb, srcbox, dstbox));
  }

  if (srcfb->format == NSFB_FMT_ABGR8888 )
  { trans = DO_ALPHA_BLEND;
  }

  if ((srcfb->width == 1) && (srcfb->height == 1))
  { srccol = *(NSFBCOLOUR *)(void *)(srcfb->loc);  // JACS ptr -> pan


    if ((srccol & 0xff000000) == 0) 	/* check for completely transparent */
	   { return( true );
	   }

	/* completely opaque pixels can be replaced with fill
	 */
    if ((srccol & 0xff000000) == 0xff000000)
  	 { return dstfb->plotterFns->fill(dstfb, dstbox, srccol);
  } }

  return dstfb->plotterFns->bitmap( dstfb, dstbox
                                  , (const NSFBCOLOUR *)(void *)srcfb->loc + (srcbox->y0*srcbox->x1)// JACS ptr -> pan
                                  , srcfb->width, srcfb->height
                                  , (srcfb->panlen << 3) / srcfb->bpp
                                  , trans );
}

ANSIC bool nsfbPlotbitmap( Nsfb              * nsfb
                          , const NsfbBbox   * loc
                          , const NSFBCOLOUR * pixel
                          , int bmp_width, int bmp_height
                          , int bmp_stride, int alpha )
{ return nsfb->plotterFns->bitmap( nsfb, loc, pixel, bmp_width, bmp_height, bmp_stride, alpha);
}

ANSIC bool nsfbPlotbitmapTiles( Nsfb *nsfb, const NsfbBbox *loc
                               , int tiles_x, int tiles_y
                               , const NSFBCOLOUR *pixel
                               , int bmp_width, int bmp_height, int bmp_stride
                               , int alpha )
{ return nsfb->plotterFns->bitmapTiles( nsfb
                                      , loc, tiles_x, tiles_y
                                      , pixel, bmp_width, bmp_height
                                      , bmp_stride, alpha );
}

/** Plot an 8 bit glyph.
 */
ANSIC bool nsfbPlotglyph8( Nsfb      * nsfb
                            , NsfbBbox * loc
                            , const byte  * pixel
                            , int pitch
                            , NSFBCOLOUR c, NSFBCOLOUR b )
{ return nsfb->plotterFns->glyph8(nsfb, loc, pixel, pitch, c, b );
}


/** Plot an 1 bit glyph.
 */
ANSIC bool nsfbPlotglyph1( Nsfb      * nsfb
                          , NsfbBbox *loc
                          , const byte  * pixel
                          , int pitch, NSFBCOLOUR c )
{ return nsfb->plotterFns->glyph1(nsfb, loc, pixel, pitch, c);
}

/* read a rectangle from screen into buffer */
ANSIC bool nsfbPlotreadrect( Nsfb *nsfb, NsfbBbox *rect, NSFBCOLOUR * buffer  )
{ return nsfb->plotterFns->readrect(nsfb, rect, buffer );
}

ANSIC int nsfbPlotMoverect( Nsfb * nsfb, int  w, int h, int x, int y )
{ return( nsfb->surfaceRtns->panType
        ? nsfb->plotterFns->moverect( nsfb, w,h,x,y )
        : 0 );
}

ANSIC int fbPlotMoverect( Nsfb * nsfb, int  w, int h, int x, int y )
{ return( nsfb->plotterFns->moverect( nsfb, w,h,x,y ));
}



ANSIC bool nsfbPlotcubicBezier( Nsfb *nsfb
                               , NsfbBbox   *curve
                               , NsfbPoint  *ctrla, NsfbPoint *ctrlb
                               , NsfbPlotpen *pen )
{ return nsfb->plotterFns->cubic( nsfb, curve, ctrla, ctrlb, pen );
}

ANSIC bool nsfbPlotquadraticBezier( Nsfb *nsfb
                                  , NsfbBbox *curve, NsfbPoint *ctrla
                                  , NsfbPlotpen *pen )
{ return nsfb->plotterFns->quadratic(nsfb, curve, ctrla, pen);
}

ANSIC bool nsfbPlotpath( Nsfb *nsfb, int pathc, nsfbPlotpathop_t *pathop, NsfbPlotpen *pen)
{ return( nsfb->plotterFns->path( nsfb, pathc, pathop, pen ));
}

ANSIC  bool   nsfbPutPixmap( Nsfb * nsfb
                           , const ImageMap * hard
                           , int x, int y
                           , NSFBCOLOUR back )
{ return( nsfb->plotterFns->pixmapFill( nsfb, hard, x,y, 0, 0, back ));
}

ANSIC  bool nsfbPutIconImage( Nsfb           * nsfb
                            , DeviceImageRec * image
                            , int x, int y, int n
                            , NSFBCOLOUR back )
{ return( image ? nsfb->plotterFns->pixmapFill( nsfb
                                              , &image->map
                                              , x, y
                                              , n * image->height, image->height
                                              , back )
                : false );
}


/*
 * Local variables:
 *  c-basic-offset: 4
 *  tab-width: 8
 * End:
 */
