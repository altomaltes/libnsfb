/* public plotter interface */


#include "../libnsfb_plot.h"

#include "../nsfb.h"
#include "../plot.h"

/** Sets a clip rectangle for subsequent plots.
 *
 * Sets a clipping area which constrains all subsequent plotting operations.
 * The clipping area must lie within the framebuffer visible screen or false
 * will be returned and the new clipping area not set.
 */
PUBLIC bool nsfbPlotset_clip(nsfb_t *nsfb, nsfb_bbox_t *clip)
{ return nsfb->plotter_fns->set_clip(nsfb, clip);
}

/** Get the previously set clipping region.
 */
PUBLIC bool nsfbPlotget_clip(nsfb_t *nsfb, nsfb_bbox_t *clip)
{ return nsfb->plotter_fns->get_clip(nsfb, clip);
}

/** Clears plotting area to a flat colour.
 */
PUBLIC bool nsfbPlotclg( nsfb_t *nsfb, nsfb_colour_t c )
{ return nsfb->plotter_fns->clg( nsfb, c );
}

/** Plots a rectangle outline.
 *
 * The line can be solid, dotted or dashed. Top left corner at (x0,y0) and
 * rectangle has given width and height.
 */
PUBLIC bool nsfbPlotrectangle( nsfb_t *nsfb
                        , nsfb_bbox_t *rect
                        , int line_width
                        , nsfb_colour_t c
                        , bool dotted, bool dashed)
{ return nsfb->plotter_fns->rectangle(nsfb, rect, line_width, c, dotted, dashed);
}

/** Plots a filled rectangle. Top left corner at (x0,y0), bottom
 *		  right corner at (x1,y1). Note: (x0,y0) is inside filled area,
 *		  but (x1,y1) is below and to the right. See diagram below.
 */
PUBLIC bool nsfbPlotrectangle_fill(nsfb_t *nsfb, nsfb_bbox_t *rect, nsfb_colour_t c)
{ return nsfb->plotter_fns->fill(nsfb, rect, c);
}

/** Plots a line.
 *
 * Draw a line from (x0,y0) to (x1,y1). Coordinates are at centre of line
 * width/thickness.
 */
PUBLIC bool nsfbPlotline(nsfb_t *nsfb, nsfb_bbox_t *line, nsfbPlotpen_t *pen)
{ return nsfb->plotter_fns->line(nsfb, 1, line, pen);
}

/** Plots more than one line.
 *
 * Draw a line from (x0,y0) to (x1,y1). Coordinates are at centre of line
 * width/thickness.
 */
PUBLIC bool nsfbPlotlines(nsfb_t *nsfb, int linec, nsfb_bbox_t *line, nsfbPlotpen_t *pen)
{ return nsfb->plotter_fns->line(nsfb, linec, line, pen);
}

PUBLIC bool nsfbPlotpolylines(nsfb_t *nsfb, int pointc, const nsfb_point_t *points, nsfbPlotpen_t *pen)
{ return nsfb->plotter_fns->polylines(nsfb, pointc, points, pen);
}

/** Plots a filled polygon.
 *
 * Plots a filled polygon with straight lines between points. The lines around
 * the edge of the ploygon are not plotted. The polygon is filled with a
 * non-zero winding rule.
 *
 *
 */
PUBLIC bool nsfbPlotpolygon(nsfb_t *nsfb, const int *p, unsigned int n, nsfb_colour_t fill)
{ return nsfb->plotter_fns->polygon(nsfb, p, n, fill);
}

/** Plots an arc.
 *
 * around (x,y), from anticlockwise from angle1 to angle2. Angles are measured
 * anticlockwise from horizontal, in degrees.
 */
PUBLIC bool nsfbPlotarc( nsfb_t *nsfb
                         , int x, int y, int radius
                         , int angle1, int angle2
                         , nsfb_colour_t c)
{ return nsfb->plotter_fns->arc(nsfb, x, y, radius, angle1, angle2, c);
}

/** Plots an alpha blended pixel.
 *
 * plots an alpha blended pixel.
 */
PUBLIC bool nsfbPlotpoint(nsfb_t *nsfb, int x, int y, nsfb_colour_t c)
{ return nsfb->plotter_fns->point(nsfb, x, y, c);
}

PUBLIC bool nsfbPlotellipse(nsfb_t *nsfb, nsfb_bbox_t *ellipse, nsfb_colour_t c)
{ return nsfb->plotter_fns->ellipse(nsfb, ellipse, c);
}

PUBLIC bool nsfbPlotellipse_fill(nsfb_t *nsfb, nsfb_bbox_t *ellipse, nsfb_colour_t c)
{ return nsfb->plotter_fns->ellipse_fill(nsfb, ellipse, c);
}

/* copy an area of surface from one location to another.
 *
 * @warning This implementation is woefully incomplete!
 */
PUBLIC bool nsfbPlotcopy( nsfb_t * srcfb, nsfb_bbox_t * srcbox
                          , nsfb_t * dstfb, nsfb_bbox_t * dstbox )
{ int trans = 0;
  nsfb_colour_t srccol;

  if ( srcfb == dstfb )
  { return dstfb->plotter_fns->copy(srcfb, srcbox, dstbox);
  }

  if (srcfb->format == NSFB_FMT_ABGR8888)
  { trans = DO_ALPHA_BLEND;
  }

  if ((srcfb->width == 1) && (srcfb->height == 1))
  { srccol = *(nsfb_colour_t *)(void *)(srcfb->loc);  // JACS ptr -> pan


    if ((srccol & 0xff000000) == 0) 	/* check for completely transparent */
	   { return true;
	   }

	/* completely opaque pixels can be replaced with fill
	 */
    if ((srccol & 0xff000000) == 0xff000000)
  	 { return dstfb->plotter_fns->fill(dstfb, dstbox, srccol);
  } }

  return dstfb->plotter_fns->bitmap( dstfb, dstbox
                                   , (const nsfb_colour_t *)(void *)srcfb->loc + (srcbox->y0*srcbox->x1)// JACS ptr -> pan
                                   , srcfb->width, srcfb->height
                                   , (srcfb->panlen << 3) / srcfb->bpp
                                   , trans );
}

PUBLIC bool nsfbPlotbitmap( nsfb_t              * nsfb
                            , const nsfb_bbox_t   * loc
                            , const nsfb_colour_t * pixel
                            , int bmp_width, int bmp_height
                            , int bmp_stride, int alpha )
{ return nsfb->plotter_fns->bitmap( nsfb, loc, pixel, bmp_width, bmp_height, bmp_stride, alpha);
}

PUBLIC bool nsfbPlotbitmap_tiles( nsfb_t *nsfb, const nsfb_bbox_t *loc
                                  , int tiles_x, int tiles_y
                                  , const nsfb_colour_t *pixel
                                  , int bmp_width, int bmp_height, int bmp_stride
                                  , int alpha )
{ return nsfb->plotter_fns->bitmap_tiles(nsfb, loc, tiles_x, tiles_y, pixel, bmp_width, bmp_height, bmp_stride, alpha );
}

/** Plot an 8 bit glyph.
 */
PUBLIC bool nsfbPlotglyph8( nsfb_t      * nsfb
                            , nsfb_bbox_t * loc
                            , const byte  * pixel
                            , int pitch
                            , nsfb_colour_t c, nsfb_colour_t b )
{ return nsfb->plotter_fns->glyph8(nsfb, loc, pixel, pitch, c, b );
}


/** Plot an 1 bit glyph.
 */
PUBLIC bool nsfbPlotglyph1( nsfb_t      * nsfb
                            , nsfb_bbox_t *loc
                            , const byte  * pixel
                            , int pitch, nsfb_colour_t c )
{ return nsfb->plotter_fns->glyph1(nsfb, loc, pixel, pitch, c);
}

/* read a rectangle from screen into buffer */
PUBLIC bool nsfbPlotreadrect(     nsfb_t *nsfb, nsfb_bbox_t *rect, nsfb_colour_t * buffer  )
{ return nsfb->plotter_fns->readrect(nsfb, rect, buffer );
}

PUBLIC int nsfbPlotMoverect( nsfb_t * nsfb, int  w, int h, int x, int y )
{ return( nsfb->plotter_fns->moverect( nsfb, w,h,x,y) );
}

PUBLIC int fbPlotMoverect( nsfb_t * nsfb, int  w, int h, int x, int y )
//{ return(( nsfb->active & 0x01 ) ? nsfb->plotter_fns->moverect( nsfb, w,h,x,y )
//                                 : 0 );
//}
{ return( nsfb->plotter_fns->moverect( nsfb, w,h,x,y ));
}



PUBLIC bool nsfbPlotcubic_bezier( nsfb_t *nsfb, nsfb_bbox_t *curve, nsfb_point_t *ctrla, nsfb_point_t *ctrlb, nsfbPlotpen_t *pen)
{ return nsfb->plotter_fns->cubic(nsfb, curve, ctrla, ctrlb, pen);
}

PUBLIC bool nsfbPlotquadratic_bezier(nsfb_t *nsfb, nsfb_bbox_t *curve, nsfb_point_t *ctrla, nsfbPlotpen_t *pen)
{ return nsfb->plotter_fns->quadratic(nsfb, curve, ctrla, pen);
}

PUBLIC bool nsfbPlotpath(nsfb_t *nsfb, int pathc, nsfbPlotpathop_t *pathop, nsfbPlotpen_t *pen)
{ return nsfb->plotter_fns->path(nsfb, pathc, pathop, pen);
}

/*
 * Local variables:
 *  c-basic-offset: 4
 *  tab-width: 8
 * End:
 */
