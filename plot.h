

/** Clears plotting area to a flat colour (if needed)
 */
typedef bool ( *NsfbPlotfnClg)(Nsfb *, NSFBCOLOUR c);

/** Switchs doublebuffer
 */
typedef bool ( *NsfbPlotfnPan)( Nsfb *, int type );

/** Plots a rectangle outline. The line can be solid, dotted or
 *		  dashed. Top left corner at (x0,y0) and rectangle has given
 *		  width and height.
 */
typedef	bool ( *NsfbPlotfnRectangle)(Nsfb *, NsfbBbox *rect, int line_width, NSFBCOLOUR c, bool dotted, bool dashed);

/** Plots a line using a given pen.
 */
typedef bool ( *nsfbPlotfnLine)(Nsfb *, int linec, NsfbBbox *line, NsfbPlotpen *pen);

/** Plots a filled polygon with straight lines between points.
 *		  The lines around the edge of the ploygon are not plotted. The
 *		  polygon is filled with the non-zero winding rule.
 */
typedef	bool ( *NsfbPlotfnPolygon)( Nsfb *, const int *p, unsigned int n, NSFBCOLOUR fill );

/** Plots a filled rectangle. Top left corner at (x0,y0), bottom
 *		  right corner at (x1,y1). Note: (x0,y0) is inside filled area,
 *		  but (x1,y1) is below and to the right. See diagram below.
 */
typedef	bool ( *NsfbPlotfnFill)(Nsfb *, NsfbBbox *rect, NSFBCOLOUR c);

/** Clipping operations.
 */
typedef	bool ( *NsfbPlotfnClip)(Nsfb *, NsfbBbox *clip);

/** Plots an arc, around (x,y), from anticlockwise from angle1 to
 *		  angle2. Angles are measured anticlockwise from horizontal, in
 *		  degrees.
 */
typedef	bool ( *NsfbPlotfnArc)(Nsfb *, int x, int y, int radius, int angle1, int angle2, NSFBCOLOUR c);

/** Plots a point.
 *
 * Plot a single alpha blended pixel.
 */
typedef	bool ( *NsfbPlotfnPoint      )(Nsfb *, int x, int y, NSFBCOLOUR c);

/** Plot an ellipse.
 *
 * plot an ellipse outline, note if teh bounding box is square this will plot a
 * circle.
 */
typedef	bool ( *NsfbPlotfnEllipse    )(Nsfb *, NsfbBbox *ellipse, NSFBCOLOUR c);

/** Plot a filled ellipse.
 *
 * plot a filled ellipse, note if the bounding box is square this will plot a
 * circle.
 */
typedef	bool ( *NsfbPlotfnEllipseFill )(Nsfb *nsfb, NsfbBbox *ellipse, NSFBCOLOUR c);


/** Plot bitmap
 */
typedef bool ( *NsfbPlotfnBitmap     )( Nsfb *nsfb
                                     , const NsfbBbox *loc, const NSFBCOLOUR *pixel
                                     , int bmp_width, int bmp_height, int bmp_stride
                                     , int alpha);

/** Plot tiled bitmap
 */
typedef bool ( *NsfbPlotfnBitmapTiles)( Nsfb *nsfb
                                     , const NsfbBbox *loc
                                     , int tiles_x, int tiles_y
                                     , const NSFBCOLOUR *pixel
                                     , int bmp_width, int bmp_height, int bmp_stride
                                     , int alpha );


/** plot pixmapp
 */
typedef bool          ( *NsfbPlotfnPutPixmap   )( Nsfb *
                                                , DeviceImageRec * img
                                                , int x, int y
                                                , int h, int hei
                                                , NSFBCOLOUR back );

/** Copy an area of screen
 *
 * Copy an area of the display.
 */
typedef bool ( * NsfbPlotfnCopy         )(Nsfb *, NsfbBbox *srcbox, NsfbBbox *dstbox);


/** Plot an 8 bit per pixel glyph.
 */
typedef bool ( *NsfbPlotfnGlyph8        )(Nsfb *, NsfbBbox *loc, const byte *pixel, int pitch, NSFBCOLOUR c, NSFBCOLOUR b );


/** Plot an 1 bit per pixel glyph.
 */
typedef bool ( *NsfbPlotfnGlyph1         )(Nsfb *, NsfbBbox *loc, const byte *pixel, int pitch, NSFBCOLOUR c);

/** Read rectangle of screen into buffer
 */
typedef	bool ( *NsfbPlotfnReadrect       )( Nsfb *, NsfbBbox *rect, NSFBCOLOUR *buffer);

/** Move rectangle of screen
 */
typedef	int  ( *NsfbPlotfnMoverect       )( Nsfb *, int  w, int h, int x, int y );

/** Plot quadratic bezier spline
 */
typedef bool ( *NsfbPlotfnQuadratic)(Nsfb *, NsfbBbox *curve, NsfbPoint *ctrla, NsfbPlotpen *pen);

/** Plot cubic bezier spline
 */
typedef bool ( *NsfbPlotfnCubicBezier    )(Nsfb *, NsfbBbox *curve, NsfbPoint *ctrla, NsfbPoint *ctrlb, NsfbPlotpen *pen);

typedef bool ( *NsfbPlotfnPolylines      )(Nsfb *, int pointc, const NsfbPoint *points, NsfbPlotpen *pen);

/** plot path
 */
typedef bool ( *nsfbPlotfnPath           )(Nsfb *, int pathc, nsfbPlotpathop_t *pathop, NsfbPlotpen *pen);

/** plotter function table.
  */

bool setClip( Nsfb *, NsfbBbox *clip );
bool getClip( Nsfb *, NsfbBbox *clip );
bool clg(     Nsfb *, NSFBCOLOUR c);
bool polygon( Nsfb *
            , const int * p, unsigned int n
            , NSFBCOLOUR c );

bool rectangle( Nsfb     *
              , NsfbBbox * rect
              , int line_width, NSFBCOLOUR c
              , bool dotted, bool dashed );

bool ellipseFill(Nsfb *, NsfbBbox *ellipse, NSFBCOLOUR c);
bool ellipse(    Nsfb *, NsfbBbox *ellipse, NSFBCOLOUR c);

bool        copy( Nsfb     *
                , NsfbBbox * srcbox
                , NsfbBbox * dstbox );

bool        arc( Nsfb *
               , int x, int y, int radius
               , int angle1, int angle2
               , NSFBCOLOUR c );

bool plotCubic( Nsfb *
              , NsfbBbox   * curve
              , NsfbPoint  * ctrla
              , NsfbPoint  * ctrlb
              , NsfbPlotpen * pen );

bool plotQuadratic( Nsfb *
                  , NsfbBbox *curve
                  , NsfbPoint *ctrla
                  , NsfbPlotpen *pen );

bool plotPath( Nsfb *
             , int pathc, nsfbPlotpathop_t *pathop
             , NsfbPlotpen *pen );

bool plotClg( Nsfb *, NSFBCOLOUR c );
bool plotPan( Nsfb *, int type );


bool polylines( Nsfb *
              , int pointc
              , const NsfbPoint * points
              , NsfbPlotpen     * pen );

typedef struct NsfbPlotterFns
{ NsfbPlotfnClg          clg;
  NsfbPlotfnPan          pan;
  NsfbPlotfnRectangle    rectangle;
  nsfbPlotfnLine         line;
  NsfbPlotfnPolygon      polygon;
  NsfbPlotfnFill         fill;
  NsfbPlotfnClip         getClip;
  NsfbPlotfnClip         setClip;
  NsfbPlotfnEllipse      ellipse;
  NsfbPlotfnEllipseFill  ellipseFill;
  NsfbPlotfnPutPixmap    pixmapFill;
  NsfbPlotfnArc          arc;
  NsfbPlotfnBitmap       bitmap;
  NsfbPlotfnBitmapTiles  bitmapTiles;
  NsfbPlotfnPoint        point;
  NsfbPlotfnCopy         copy;
  NsfbPlotfnGlyph8       glyph8;
  NsfbPlotfnGlyph1       glyph1;
  NsfbPlotfnReadrect     readrect;
  NsfbPlotfnMoverect     moverect;
  NsfbPlotfnQuadratic    quadratic;
  NsfbPlotfnCubicBezier  cubic;
  nsfbPlotfnPath         path;
  NsfbPlotfnPolylines    polylines;
} nsfbPlotterFns;


bool selectPlotters(Nsfb *nsfb);



