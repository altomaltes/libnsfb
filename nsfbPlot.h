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
typedef dword NsfbColour;

/**
 * Type of plot operation
 */
typedef enum nsfbPlotoptype_e
{ NFSB_PLOT_OPTYPE_NONE = 0  /**< No operation */
, NFSB_PLOT_OPTYPE_SOLID     /**< Solid colour */
, NFSB_PLOT_OPTYPE_PATTERN   /**< Pattern plot */
} nsfbPlotoptype_t;

/** pen colour and raster operation for plotting primatives.
 */
typedef struct nsfbPlotpen_s
{ nsfbPlotoptype_t strokeType;     /**< Stroke plot type */
  int              strokeWidth;    /**< Width of stroke, in pixels */
  NsfbColour       strokeColour;   /**< Colour of stroke */
  nsfbPlotoptype_t fillType;       /**< Fill plot type */
  NsfbColour       fillColour;     /**< Colour of fill */
} NsfbPlotpen;

/** path operation type.
 */
typedef enum nsfbPlotpathop_type_e
{ NFSB_PLOT_PATHOP_MOVE
, NFSB_PLOT_PATHOP_LINE
, NFSB_PLOT_PATHOP_QUAD
, NFSB_PLOT_PATHOP_CUBIC
} NsfbPlotpathopType;

/** path element
 */
typedef struct nsfbPlotpathop_s
{ NsfbPlotpathopType operation;
  NsfbPoint point;
} nsfbPlotpathop_t;

/** Sets a clip rectangle for subsequent plots.
 *
 * Sets a clipping area which constrains all subsequent plotting operations.
 * The clipping area must lie within the framebuffer visible screen or false
 * will be returned and the new clipping area not set.
 */
ANSIC bool nsfbPlotsetClip( Nsfb *, NsfbBbox *clip);

/** Get the previously set clipping region.
 */
ANSIC bool nsfbPlotgetClip( Nsfb *, NsfbBbox *clip);

/** Clears plotting area to a flat colour.
 */
ANSIC bool nsfbPlotclg( Nsfb *, NsfbColour c );


/** Plots a rectangle outline.
 *
 * The line can be solid, dotted or dashed. Top left corner at (x0,y0) and
 * rectangle has given width and height.
 */
ANSIC bool nsfbPlotrectangle( Nsfb *, NsfbBbox *rect, int line_width, NsfbColour c, bool dotted, bool dashed);

/** Plots a filled rectangle. Top left corner at (x0,y0), bottom
 *		  right corner at (x1,y1). Note: (x0,y0) is inside filled area,
 *		  but (x1,y1) is below and to the right. See diagram below.
 */
ANSIC bool nsfbPlotrectangleFill( Nsfb *, NsfbBbox *rect, NsfbColour c);

/** Plots a line.
 *
 * Draw a line from (x0,y0) to (x1,y1). Coordinates are at centre of line
 * width/thickness.
 */
ANSIC bool nsfbPlotline( Nsfb *, NsfbBbox *line, NsfbPlotpen *pen);

/** Plots a number of lines.
 *
 * Draw a series of lines.
 */
ANSIC bool nsfbPlotlines(Nsfb *nsfb, int linec, NsfbBbox *line, NsfbPlotpen *pen);

/** Plots a number of connected lines.
 *
 * Draw a series of connected lines.
 */
ANSIC bool nsfbPlotpolylines(Nsfb *nsfb, int pointc, const NsfbPoint *points, NsfbPlotpen *pen);

/** Plots a filled polygon.
 *
 * Plots a filled polygon with straight lines between points. The lines around
 * the edge of the ploygon are not plotted. The polygon is filled with a
 * non-zero winding rule.
 *
 *
 */
ANSIC bool nsfbPlotpolygon(Nsfb *nsfb, const int *p, unsigned int n, NsfbColour fill);

/** Plot an ellipse.
 */
ANSIC bool nsfbPlotellipse(Nsfb *nsfb, NsfbBbox *ellipse, NsfbColour c);

/** Plot a filled ellipse.
 */
ANSIC bool nsfbPlotellipseFill(Nsfb *nsfb, NsfbBbox *ellipse, NsfbColour c);

/** Plots an arc.
 *
 * around (x,y), from anticlockwise from angle1 to angle2. Angles are measured
 * anticlockwise from horizontal, in degrees.
 */
ANSIC bool nsfbPlotarc( Nsfb *
                       , int x, int y, int radius
                       , int angle1, int angle2
                       , NsfbColour c);

/** Plots an alpha blended pixel.
 *
 * plots an alpha blended pixel.
 */
ANSIC bool nsfbPlotpoint(           Nsfb *, int x, int y, NsfbColour c );
ANSIC bool nsfbPlotcubicBezier(     Nsfb *, NsfbBbox *curve, NsfbPoint *ctrla, NsfbPoint *ctrlb, NsfbPlotpen *pen );
ANSIC bool nsfbPlotquadraticBezier( Nsfb *, NsfbBbox *curve, NsfbPoint *ctrla, NsfbPlotpen *pen );
ANSIC bool nsfbPlotpath(            Nsfb *, int pathc, nsfbPlotpathop_t *pathop, NsfbPlotpen *pen );

/** copy an area of screen
 *
 * Copy an area of the display.
 */
ANSIC bool nsfbPlotcopy(         Nsfb *, NsfbBbox *srcbox, Nsfb *dstfb, NsfbBbox *dstbox);

/** Plot bitmap.
 */
ANSIC bool nsfbPlotbitmap( Nsfb *
                         , const NsfbBbox *loc, const NsfbColour *pixel
                         , int bmp_width, int bmp_height, int bmp_stride
                         , int alpha );

ANSIC  bool nsfbPutIconImage( Nsfb           * nsfb
                            , DeviceImageRec * image
                            , int x, int y, int n
                            , COLORREF1 back );

ANSIC  bool   nsfbPutPixmap( Nsfb * nsfb
                           , const ImageMap * hard
                           , int x, int y
                           , COLORREF1 back );

/** Plot bitmap.
 */
ANSIC bool nsfbPlotbitmapTiles( Nsfb *
                               , const NsfbBbox *
                               , int tiles_x, int tiles_y
                               , const NsfbColour *
                               , int bmp_width, int bmp_height, int bmp_stride
                               , int alpha );

/** Plot an 8 bit glyph.
 */
ANSIC bool nsfbPlotglyph8(    Nsfb *, NsfbBbox *, const unsigned char *pixel, int pitch, NsfbColour c, NsfbColour b );

/** Plot an 1 bit glyph.
 */
ANSIC bool nsfbPlotglyph1(    Nsfb *, NsfbBbox *loc, const unsigned char *pixel, int pitch, NsfbColour c);

/** read rectangle into buffer
 */
ANSIC bool nsfbPlotreadrect(  Nsfb *, NsfbBbox *rect, NsfbColour * buffer );

/** Move image area
  */
ANSIC  int nsfbPlotMoverect(  Nsfb *, int  w, int h, int x, int y );
ANSIC  int   fbPlotMoverect(  Nsfb *, int  w, int h, int x, int y );


#endif /* _LIBNSFB_PLOT_H */
