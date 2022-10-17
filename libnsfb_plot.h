/*
 * Copyright 2009 Vincent Sanders <vince@simtec.co.uk>
 *
 * This file is part of libnsfb, http://www.netsurf-browser.org/
 * Licenced under the MIT License,
 *                http://www.opensource.org/licenses/mit-license.php
 *
 * This is the exported plotter interface for the libnsfb graphics library.
 */

#ifndef _LIBNSFB_PLOT_H
#define _LIBNSFB_PLOT_H 1

#include "libnsfb.h"

#define word  unsigned short
#define dword unsigned int
#define byte  unsigned char

/** representation of a colour.
 *
 * The colour value comprises of four components arranged in the order ABGR:
 * bits 24-31 are the alpha value and represent the opacity. 0 is
 *   transparent i.e. there would be no change in the target surface if
 *   this colour were to be used and 0xFF is opaque.
 *
 * bits 16-23 are the Blue component of the colour.
 *
 * bits 8-15 are the Green component of the colour.
 *
 * bits 0-7 are the Red component of the colour.
 */
typedef dword nsfb_colour_t;

/**
 * Type of plot operation
 */
typedef enum nsfbPlotoptype_e
{ NFSB_PLOT_OPTYPE_NONE = 0  /**< No operation */
, NFSB_PLOT_OPTYPE_SOLID     /**< Solid colour */
, NFSB_PLOT_OPTYPE_PATTERN   /**< Pattern plot */
} nsfbPlotoptype_t;

/** pen colour and raster operation for plotting primatives. */
typedef struct nsfbPlotpen_s
{ nsfbPlotoptype_t stroke_type; /**< Stroke plot type */
  int stroke_width;               /**< Width of stroke, in pixels */
  nsfb_colour_t stroke_colour;    /**< Colour of stroke */
  dword stroke_pattern;
  nsfbPlotoptype_t fill_type;   /**< Fill plot type */
  nsfb_colour_t fill_colour;      /**< Colour of fill */
} nsfbPlotpen_t;

/** path operation type. */
typedef enum nsfbPlotpathop_type_e
{ NFSB_PLOT_PATHOP_MOVE
, NFSB_PLOT_PATHOP_LINE
, NFSB_PLOT_PATHOP_QUAD
, NFSB_PLOT_PATHOP_CUBIC
} nsfbPlotpathop_type_t;

/** path element */
typedef struct nsfbPlotpathop_s
{ nsfbPlotpathop_type_t operation;
  nsfb_point_t point;
} nsfbPlotpathop_t;

/** Sets a clip rectangle for subsequent plots.
 *
 * Sets a clipping area which constrains all subsequent plotting operations.
 * The clipping area must lie within the framebuffer visible screen or false
 * will be returned and the new clipping area not set.
 */
PUBLIC bool nsfbPlotset_clip( nsfb_t *, nsfb_bbox_t *clip);

/** Get the previously set clipping region.
 */
PUBLIC bool nsfbPlotget_clip( nsfb_t *, nsfb_bbox_t *clip);

/** Clears plotting area to a flat colour.
 */
PUBLIC bool nsfbPlotclg( nsfb_t *, nsfb_colour_t c );


/** Plots a rectangle outline.
 *
 * The line can be solid, dotted or dashed. Top left corner at (x0,y0) and
 * rectangle has given width and height.
 */
PUBLIC bool nsfbPlotrectangle( nsfb_t *, nsfb_bbox_t *rect, int line_width, nsfb_colour_t c, bool dotted, bool dashed);

/** Plots a filled rectangle. Top left corner at (x0,y0), bottom
 *		  right corner at (x1,y1). Note: (x0,y0) is inside filled area,
 *		  but (x1,y1) is below and to the right. See diagram below.
 */
PUBLIC bool nsfbPlotrectangle_fill( nsfb_t *, nsfb_bbox_t *rect, nsfb_colour_t c);

/** Plots a line.
 *
 * Draw a line from (x0,y0) to (x1,y1). Coordinates are at centre of line
 * width/thickness.
 */
PUBLIC bool nsfbPlotline( nsfb_t *, nsfb_bbox_t *line, nsfbPlotpen_t *pen);

/** Plots a number of lines.
 *
 * Draw a series of lines.
 */
PUBLIC bool nsfbPlotlines(nsfb_t *nsfb, int linec, nsfb_bbox_t *line, nsfbPlotpen_t *pen);

/** Plots a number of connected lines.
 *
 * Draw a series of connected lines.
 */
PUBLIC bool nsfbPlotpolylines(nsfb_t *nsfb, int pointc, const nsfb_point_t *points, nsfbPlotpen_t *pen);

/** Plots a filled polygon.
 *
 * Plots a filled polygon with straight lines between points. The lines around
 * the edge of the ploygon are not plotted. The polygon is filled with a
 * non-zero winding rule.
 *
 *
 */
PUBLIC bool nsfbPlotpolygon(nsfb_t *nsfb, const int *p, unsigned int n, nsfb_colour_t fill);

/** Plot an ellipse.
 */
PUBLIC bool nsfbPlotellipse(nsfb_t *nsfb, nsfb_bbox_t *ellipse, nsfb_colour_t c);

/** Plot a filled ellipse.
 */
PUBLIC bool nsfbPlotellipse_fill(nsfb_t *nsfb, nsfb_bbox_t *ellipse, nsfb_colour_t c);

/** Plots an arc.
 *
 * around (x,y), from anticlockwise from angle1 to angle2. Angles are measured
 * anticlockwise from horizontal, in degrees.
 */
PUBLIC bool nsfbPlotarc( nsfb_t *nsfb
                         , int x, int y, int radius
                         , int angle1, int angle2
                         , nsfb_colour_t c);

/** Plots an alpha blended pixel.
 *
 * plots an alpha blended pixel.
 */
PUBLIC bool nsfbPlotpoint(            nsfb_t *, int x, int y, nsfb_colour_t c );
PUBLIC bool nsfbPlotcubic_bezier(     nsfb_t *, nsfb_bbox_t *curve, nsfb_point_t *ctrla, nsfb_point_t *ctrlb, nsfbPlotpen_t *pen );
PUBLIC bool nsfbPlotquadratic_bezier( nsfb_t *, nsfb_bbox_t *curve, nsfb_point_t *ctrla, nsfbPlotpen_t *pen );
PUBLIC bool nsfbPlotpath(             nsfb_t *, int pathc, nsfbPlotpathop_t *pathop, nsfbPlotpen_t *pen );

/** copy an area of screen
 *
 * Copy an area of the display.
 */
PUBLIC bool nsfbPlotcopy(         nsfb_t *, nsfb_bbox_t *srcbox, nsfb_t *dstfb, nsfb_bbox_t *dstbox);

/** Plot bitmap.
 */
PUBLIC bool nsfbPlotbitmap( nsfb_t *
                            , const nsfb_bbox_t *loc, const nsfb_colour_t *pixel
                            , int bmp_width, int bmp_height, int bmp_stride
                            , int alpha );

/** Plot bitmap.
 */
PUBLIC bool nsfbPlotbitmap_tiles( nsfb_t *
                                  , const nsfb_bbox_t *
                                  , int tiles_x, int tiles_y
                                  , const nsfb_colour_t *
                                  , int bmp_width, int bmp_height, int bmp_stride
                                  , int alpha );

/** Plot an 8 bit glyph.
 */
PUBLIC bool nsfbPlotglyph8(       nsfb_t *, nsfb_bbox_t *, const byte *pixel, int pitch, nsfb_colour_t c, nsfb_colour_t b );

/** Plot an 1 bit glyph.
 */
PUBLIC bool nsfbPlotglyph1(       nsfb_t *, nsfb_bbox_t *loc, const byte *pixel, int pitch, nsfb_colour_t c);

/** read rectangle into buffer
 */
PUBLIC bool nsfbPlotreadrect(     nsfb_t *, nsfb_bbox_t *rect, nsfb_colour_t * buffer );

/** Move image area
  */
PUBLIC  int nsfbPlotMoverect(     nsfb_t *, int  w, int h, int x, int y );
PUBLIC  int   fbPlotMoverect(     nsfb_t *, int  w, int h, int x, int y );


#endif /* _LIBNSFB_PLOT_H */
