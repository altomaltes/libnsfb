/**
 *     AUTHOR: Jose Angel Caso Sanchez, 2022   ( altomaltes@gmail.com )
 *
 *     Copyright (C) EMASA, Empresa Municipal De Aguas de Gijon
 *
 *
 *     FILE: sms.cc
 *     DATE: ene 2017
 *
 *  DESCRIPCION:
 *
 *  SMS dialog
 *
 *  2009 Vincent Sanders <vince@simtec.co.uk>
 */

#include <limits.h>
#include <string.h>


#include "../palette.h"
#include "../libnsfb_plot_util.h"
#include "../nsfb.h"
#include "../plot.h"
#include "../surface.h"
#include "../x11.h"

/** ================================================= [ JACS, 10/01/2006 ] == *\
 *                                                                            *
 *   JASC 2006                                                                *
 *                                                                            *
 *  FUNCTION setForeColor                                                     *
 *           setBackColor                                                     *
 *           setTextColor                                                     *
 *                                                                            *
 *  @brief                                                                    *
 *                                                                            *
\* ========================================================================= **/
static int setLineWidth( struct x11List * x11, int width )
{ return( XSetLineAttributes( x11->theDisplay, x11->theGC
                            , width
                            , LineSolid, CapButt
                            , JoinBevel  ));
}


static int setBackColor( struct x11List * x11, NSFBCOLOUR c )
{ if ( c != NOCOLOR ) return( XSetBackground( x11->theDisplay, x11->theGC, c ));
  return( 1 );
}

static int setForeColor( struct x11List * x11, NSFBCOLOUR c )
{ if (( c & ALPHACOLOR ) == ALPHACOLOR )                       /* Absolute background, includes NOCOLOR */
  { return( 0 );
  }

  XSetForeground( x11->theDisplay, x11->theGC, c );
  return( 1 );
}


/** Plots a filled rectangle. Top left corner at (x0,y0), bottom
 *		  right corner at (x1,y1). Note: (x0,y0) is inside filled area,
 *		  but (x1,y1) is below and to the right. See diagram below.
 */
static bool x11Fill( struct x11List  * nsfb
                   , NsfbBbox * rect
                   , NSFBCOLOUR c)
{ if ( setForeColor( nsfb, c ) > 0 )
  { int x= rect->x0;
    int y= rect->y0;
    int w= rect->x1 - rect->x0;
    int h= rect->y1 - rect->y0;

    XFillRectangle( nsfb->theDisplay, nsfb->theSurface, nsfb->theGC
                  , x, y
                  , w + 1
                  , h + 1 );
  }

  return( true );
}

/** Plots a rectangle outline. The line can be solid, dotted or
 *		  dashed. Top left corner at (x0,y0) and rectangle has given
 *		  width and height.
 */
static bool x11Rectangle( struct x11List  * nsfb
                        , NsfbBbox * rect
                        , int line_width, NSFBCOLOUR c
                        , bool dotted, bool dashed)
{ puts("TO RECT");
  return( false );
}

/** Plots a line using a given pen.
 */
static bool x11Line( struct x11List  * nsfb, int linec
                   , NsfbBbox * line, NsfbPlotpen * pen )
{ if ( setForeColor( nsfb,  pen->strokeColour ) > 0 )
  { setLineWidth( nsfb, 1 );

    return( !XDrawLine( nsfb->theDisplay, nsfb->theSurface, nsfb->theGC
                      , line->x0, line->y0
                      , line->x1, line->y1 ));
  }

  return( true  );
}

/** Plots a filled polygon with straight lines between points.
 *		  The lines around the edge of the ploygon are not plotted. The
 *		  polygon is filled with the non-zero winding rule.
 */
static bool x11Polygon( struct x11List   * nsfb
                      , const int * points, unsigned int npt
                      , NSFBCOLOUR fill )
{ if ( setForeColor( nsfb, fill ) > 0 )
  { XPoint * poly;   // Beggining of poly to draw
    XPoint *   pt;   // polygon iterator

    poly= pt= (XPoint *) alloca( npt * sizeof( XPoint )); // Get workspace

    while( npt )
    { pt->x= *points++;
      pt->y= *points++;
      pt++; npt--;
    }

    XFillPolygon( nsfb->theDisplay, nsfb->theSurface, nsfb->theGC
                , poly
                , pt-poly
                , Nonconvex              // Simplest filling
                , CoordModeOrigin );
  }

  return( true );
}

static bool x11Polylines( struct x11List  * nsfb
                        , int npt, const NsfbPoint * points
                        , NsfbPlotpen * pen )
{ if ( setForeColor( nsfb, pen->strokeColour ) > 0 )
  { XPoint * poly;   // Beggining of poly to draw
    XPoint *   pt;   // polygon iterator

    poly= pt= (XPoint *) alloca( npt * sizeof( XPoint )); // Get workspace

    const NsfbPoint first= *points;

    while( npt )
    { pt->x= points->x;
      pt->y= points->y;
      pt++; npt--; points++;
    }
    pt->x= first.x; pt->y= first.y; pt++;

    setLineWidth( nsfb, pen->strokeWidth );
    XDrawLines( nsfb->theDisplay, nsfb->theSurface, nsfb->theGC
              , poly
              , pt-poly
              , CoordModeOrigin );
  }

  return( true );
}



/** Clipping operations.
 */
static bool x11GetClip( struct x11List  * nsfb
                      , NsfbBbox * clip )
{ puts("TOD GETCLIP");
  return( false );
}

static bool x11SetClip( struct x11List  * nsfb
                      , NsfbBbox * clip )
{ puts("TO SETCLIP");
  return( false );
}

/** Plots an arc, around (x,y), from anticlockwise from angle1 to
 *		  angle2. Angles are measured anticlockwise from horizontal, in
 *		  degrees.
 */
static bool x11Arc( struct x11List  * nsfb
                  , int x, int y, int radius
                  , int angle1, int angle2, NSFBCOLOUR c)
{ puts("TO ARC");
  return( false );
}

/** Plots a point.
 *
 * Plot a single alpha blended pixel.
 */
static bool x11Point( struct x11List  * nsfb
                    , int x, int y
                    , NSFBCOLOUR c )
{ if ( setForeColor( nsfb, c ) > 0 )
  { XDrawPoint( nsfb->theDisplay, nsfb->theSurface, nsfb->theGC
               , x, y );
  }

  return( true );
}

/** Plot an ellipse.
 *
 * plot an ellipse outline, note if teh bounding box is square this will plot a
 * circle.
 */
static bool x11Ellipse( struct x11List  * nsfb
                      , NsfbBbox *ellipse
                      , NSFBCOLOUR c)
{ if ( setForeColor( nsfb, c ) > 0 )
  {

    puts("TO ELLIP");
  }
  return( false );
}

/** Plot a filled ellipse.
 *
 * plot a filled ellipse, note if the bounding box is square this will plot a
 * circle.
 */
static bool x11EllipseFill( struct x11List  * nsfb
                          , NsfbBbox *ellipse
                          , NSFBCOLOUR c)
{ if ( setForeColor( nsfb, c ) > 0 )
  {
  puts("TO EFILL");
  }
  return( false );
}


static bool x11Bitmap( struct x11List   * nsfb
                     , const NsfbBbox   * loc
                     , const NSFBCOLOUR * pixel
                     , int bmpWidth, int bmpHeight, int bmp_stride
                     , int alpha )
{ XImage * img;

  int setDither = false;
  int x= loc->x0;
  int y= loc->y0;

  int width = loc->x1 - loc->x0;
  int height= loc->y1 - loc->y0;

  NsfbBbox clipped;         /* clipped display */

  struct x11Priv * x11= nsfb->seed.surfaceRtns;


  if (width == 0 || height == 0)
  { return true;
  }

  if (width > bmpWidth || height > bmpHeight)
  {// return bitmapScaled( nsfb, loc, pixel
     //                   , bmpWidth, bmpHeight
       //                 , bmp_stride, alpha );
    return( 0 );
  }



  clipped.x0 = x;
  clipped.y0 = y;
  clipped.x1 = x + width;
  clipped.y1 = y + height;

  if (!nsfbPlotClipCtx(nsfb, &clipped))
  { return true;
  }

  if (height > (clipped.y1 - clipped.y0))
  { height= clipped.y1 - clipped.y0;
  }

  if (width > (clipped.x1 - clipped.x0))
  { width= clipped.x1 - clipped.x0;
  }

/* Enable error diffusion for paletted screens, if not already on
 */
  if ( nsfb->seed.palette
  && nsfbPaletteDitheringOn( nsfb->seed.palette ) == false )
  { nsfbPaletteDitherInit(nsfb->seed.palette, width);
    setDither = true;
  }

  int xoff =  clipped.x0 - x;
  int yoff = (clipped.y0 - y) * bmp_stride;

/*  Actually plot the image
 */



  int w= loc->x1 - loc->x0 + 1;
  int h= loc->y1 - loc->y0 + 1;


/*   XDestroyImage frees the given data even if is not in the heap, so we must choose
 *  between to expensibly copy the given data on heap and manage its conditional freeing
 *  or leave Ximage zombie vars on the heap.
 *
 *  Anybody thiks the "clevest" mandatory free of XDestroyImage is a good idea?
 */

  int sz= bmpWidth * height * sizeof(  NSFBCOLOUR );
  char * fakeMem= malloc( sz );
  memcpy( fakeMem, pixel, sz );

  void * mask;

  switch( x11->rtns.theDepth )
  { case  1: break;
    case  8: break;

    case 16: img=
      XCreateImage(          x11->theDisplay
                  , (Visual*)x11->theVisual
                  ,          x11->rtns.theDepth
                  , ZPixmap, 0
                  , (char *)GETCOL16( bmpWidth, height, fakeMem )
                  , bmpWidth, height
                  , 8, 0 );
    break;

    case 24:
    case 32:
      img= XCreateImage(           x11->theDisplay
                       , (Visual *)x11->theVisual
                       , x11->rtns.theDepth
                       , ZPixmap, 0
                       , fakeMem // (char *)GETCOL32( w,h, data )
                       , bmpWidth, height
                       , 8, 0 );

      mask= XCreatePixmapFromBitmapData( x11->theDisplay
                                       , x11->theRoot
                                       , (char*)GETALPHA( bmpWidth, height, fakeMem )
                                       , bmpWidth, height             /* Suggested size     */
                                       , 1, 0, 1 );


    break;
    default: return( -1 );
  }

  if ( img  )
  { XSetClipOrigin( x11->theDisplay, nsfb->theGC
                  , loc->x0, loc->y0 );

    XSetClipMask  ( x11->theDisplay, nsfb->theGC
                  , (Pixmap)(mask ));


    XPutImage(  x11->theDisplay, nsfb->theSurface, nsfb->theGC
             , (XImage*)img
             , 0  , 0
             , loc->x0, loc->y0
             , bmpWidth, height );

    XSetClipMask( x11->theDisplay, nsfb->theGC
                , None );

  }

  XDestroyImage( img );
  XFreePixmap( x11->theDisplay,  mask );

  return( true );
}

/** Plot tiled bitmap
 */
static bool x11BitmapTiles( struct x11List  * nsfb
                          , const NsfbBbox *box
                          , int tiles_x, int tiles_y
                          , const NSFBCOLOUR *pixel
                          , int bmpWidth, int bmpHeight, int bmp_stride
                          , int alpha )
{

  puts("TO TILES");
  return( false );
}


/** Copy an area of screen
 *
 * Copy an area of the display.
 */
static bool x11Copy( struct x11List  * nsfb
                   , NsfbBbox * srcbox, NsfbBbox *dstbox )
{
  puts(" COPY");

  return( false );
}


/** Plot an 8 bit per pixel glyph.
 */
static bool x11Glyph8( struct x11List  * nsfb
                     , NsfbBbox *loc
                     , const byte *pixel, int pitch
                     , NSFBCOLOUR c, NSFBCOLOUR b )
{

  puts("TO G8");
  return( false );
}


/** Plot an 1 bit per pixel glyph.
 */
static bool x11Glyph1( struct x11List  * nsfb
                     , NsfbBbox *loc, const byte *pixel
                     , int pitch, NSFBCOLOUR c )
{

  puts("TO G1");
  return( false );
}

/** Read rectangle of screen into buffer
 */
static bool x11Readrect( struct x11List    * nsfb
                       , NsfbBbox   * rect
                       , NSFBCOLOUR * buffer )
{

  puts("TO >RRE");
  return( false );
}

/** Move rectangle of screen
 */
static int x11Moverect( struct x11List  * drv
                      , int  w, int h
                      , int x, int y )
{ XCopyArea( drv->theDisplay
           , drv->theSurface, drv->theWindow
           , drv->theGC
           , x, y
           , w, h
           , x, y ) ;
  XFlush( drv->theDisplay );

  return( true );
}

static  bool x11Clg( struct x11List  * drv, NSFBCOLOUR c )
{ return( plotClg( drv, drv->theBackground= c & 0xFFFFFF ));   /* Strip alpha channel */

/*  XSetWindowBackground( drv->theDisplay
                      , drv->theWindow
                      , drv->theBackground= c & 0xFFFFFF );

  XClearWindow( drv->theDisplay
              , drv->theWindow );
  XFlush( drv->theDisplay );

  return( true ); */
}



/** Plot bitmap
 */
static bool x11PixmapFill( struct x11List * drv
                         , ImageMap       * img
                         , int x, int y, int offy, int capy
                         , NSFBCOLOUR back )
{ if ( img )
  { int bmpWidth=  ((XImage * )img->image)->width;
    int bmpHeight= ((XImage * )img->image)->height;

    if ( capy )                                // Only redraw a section
    { bmpHeight= capy;
    }

    if ( img->mask )
    { XSetClipOrigin( drv->theDisplay, drv->theGC
                    , x, y - offy );

      XSetClipMask  ( drv->theDisplay, drv->theGC
                    , (Pixmap)(img->mask));
    }
    else
    { XSetClipMask( drv->theDisplay, drv->theGC
                  , None );
    }
//    if (  bmpWidth > 1000 )
  //  printf("putimage %d %d %d size %d %d\n", offy, x, y, bmpWidth, bmpHeight );


    XPutImage(  drv->theDisplay, drv->theSurface, drv->theGC
             , (XImage*)img->image
             , 0, offy
             , x,    y
             , bmpWidth, bmpHeight );
    XFlush( drv->theDisplay );

    if ( img->mask )
    { XSetClipMask( drv->theDisplay, drv->theGC
                  , None );
  } }

  return( true );
}

/* set X11 plotters
 */
struct NsfbPlotterFns x11PlottersAddr=
{ clg        : ( NsfbPlotfnClg         ) x11Clg          // NsfbPlotfnClg
, pan        : ( NsfbPlotfnPan         ) plotPan         // NsfbPlotfnPan
, rectangle  : ( NsfbPlotfnRectangle   ) x11Rectangle    // NsfbPlotfnRectangle
, line       : ( nsfbPlotfnLine        ) x11Line         // nsfbPlotfnLine
, polygon    : ( NsfbPlotfnPolygon     ) x11Polygon      // NsfbPlotfnPolygon
, fill       : ( NsfbPlotfnFill        ) x11Fill         // NsfbPlotfnFill
, getClip    : ( NsfbPlotfnClip        ) x11GetClip      // NsfbPlotfnClip
, setClip    : ( NsfbPlotfnClip        ) x11SetClip      // NsfbPlotfnClip
, ellipse    : ( NsfbPlotfnEllipse     ) x11Ellipse      // NsfbPlotfnEllipse
, ellipseFill: ( NsfbPlotfnEllipseFill ) x11EllipseFill  // NsfbPlotfnEllipseFill
, pixmapFill : ( NsfbPlotfnPutPixmap   ) x11PixmapFill   // NsfbPlotfnBitmap
, arc        : ( NsfbPlotfnArc         ) x11Arc          // NsfbPlotfnArc
, bitmap     : ( NsfbPlotfnBitmap      ) x11Bitmap       // NsfbPlotfnBitmap
, bitmapTiles: ( NsfbPlotfnBitmapTiles ) x11BitmapTiles  // NsfbPlotfnBitmapTiles
, point      : ( NsfbPlotfnPoint       ) x11Point        // NsfbPlotfnPoint
, copy       : ( NsfbPlotfnCopy        ) x11Copy         // NsfbPlotfnCopy
, glyph8     : ( NsfbPlotfnGlyph8      ) x11Glyph8       // NsfbPlotfnGlyph8
, glyph1     : ( NsfbPlotfnGlyph1      ) x11Glyph1       // NsfbPlotfnGlyph1
, readrect   : ( NsfbPlotfnReadrect    ) x11Readrect     // NsfbPlotfnReadrect
, moverect   : ( NsfbPlotfnMoverect    ) x11Moverect     // NsfbPlotfnMoverect
, quadratic  : ( NsfbPlotfnQuadratic   ) plotQuadratic   // NsfbPlotfnQuadratic
, cubic      : ( NsfbPlotfnCubicBezier ) plotCubic       // NsfbPlotfnCubicBezier
, path       : ( nsfbPlotfnPath        ) plotPath        // nsfbPlotfnPath
, polylines  : ( NsfbPlotfnPolylines   ) x11Polylines    // NsfbPlotfnPolylines
};

