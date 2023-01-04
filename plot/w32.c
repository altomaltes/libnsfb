/**
 *     AUTHOR: Jose Angel Caso Sanchez, 2006   ( altomaltes@yahoo.es )
 *                                             ( altomaltes@gmail.com )
 *
 *     Copyright (C) 2006, 2012 JACS
 *
 * This program is free software; you can redisgibute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is disgibuted in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *     FILE: w32.c
 *     DATE: ene 2006
 *
 *  DESCRIPCION: OS dependent code.
 *               windows version
 *
 */

#include "../w32.h"
#include "../img/images.h"


#ifdef LOG

  CodeRec * ptr;

  for( ptr= codes
     ; ptr->code
     ; ptr++  )
  { if ( ptr->code == code )
    { printf( "WINDOW EVENT (%05X) %4d -> %10s\n", h, ptr->code, ptr->explain );
      return;
  } }

  printf(     "WINDOW EVENT (%05X) UNKNOWN %4d\n", h, code );

}


#endif

/*
 * JASC, may 2007, reserve an internal message
 */


/** ================================================= [ JACS, 10/01/2012 ] == *\
 *                                                                            *
 *   JASC 2012                                                                *
 *                                                                            *
 *  OBJECT setForeColor                                                       *
 *         setBackColor                                                       *
 *         setTextColor                                                       *
 *                                                                            *
 *  @brief Win32 window driver internal elements.                             *
 *                                                                            *
\* ========================================================================= **/
static int setForeColor( struct w32List * win32, COLORREF clr )
{ //clr &=  0x00FFFFFF;           // Strip alpha channel info

 /*byte width= GET_BYTE( clr, 3 );           // Strip off the width

  if ( width )
  { LOGPEN data=
    {( width & 0x80 ) ? PS_DOT : PS_SOLID
    ,{ width & 0x7F, 0 }
    , clr & 0x00FFFFFF };

    CreatePenIndirect( &data );
  }
  */

  byte r= clr; clr >>= 8;
  byte g= clr; clr >>= 8;
  byte b= clr; clr >>= 8;

              clr  = r;
  clr <<= 8; clr |= g;
  clr <<= 8; clr |= b;


  return( SetDCPenColor( win32->theGC, clr & 0x00FFFFFF ));
}

static int setBackColor(  struct w32List * win32, COLORREF clr )
{  byte r= clr; clr >>= 8;
  byte g= clr; clr >>= 8;
  byte b= clr; clr >>= 8;

              clr  = r;
  clr <<= 8; clr |= g;
  clr <<= 8; clr |= b;

 return( SetDCBrushColor( win32->theGC, clr & 0x00FFFFFF ));
}


static int setTextColor( struct w32List * win32, COLORREF fg, COLORREF bg )
{ return(( ( fg != NOCOLOR ) ? SetTextColor( win32->theGC, fg ) == CLR_INVALID : 0 )
        |( ( bg != NOCOLOR ) ? SetBkColor(   win32->theGC, bg ) == CLR_INVALID : 0 ));
}


/*
 *  JASC, may 2006
 */

/** ================================================= [ JACS, 10/01/2012 ] == *\
 *                                                                            *
 *   JASC 2012                                                                *
 *                                                                            *
 *  FUNCTION win32SetClip                                                     *
 *           win32GetClip                                                     *
 *                                                                            *
 *  @brief Clipping operations                                                *
 *                                                                            *
\* ========================================================================= **/
static bool win32SetClip( struct w32List  * nsfb
                        , NsfbBbox        * ask )
{ HRGN clip= CreateRectRgn( ask->x0, ask->y0
                          , ask->x1, ask->y1 );
  return( SelectClipRgn( nsfb->theGC
                       , clip ) != ERROR );

  return( SelectClipRgn( nsfb->theGC
                       , NULL ) != ERROR );
}

static bool win32GetClip( struct w32List  * nsfb
                        , NsfbBbox * clip )
{ return( GetClipBox(  nsfb->theGC, (RECT * )clip  ));
}

/** ================================================= [ JACS, 10/01/2012 ] == *\
 *                                                                            *
 *   JASC 2012                                                                *
 *                                                                            *
 *  METHOD Win32::setTitle                                                    *
 *                                                                            *
 *  @brief sets the title of a window.                                        *
 *                                                                            *
\* ========================================================================= **/
//  const char * setTitle( const char * title )
//  { return( SetWindowText( theHandle
//                         , title ) ? title : NULL );
//  }


static bool win32Polygon( struct w32List   * win32
                        , const int * points, unsigned int npt
                        , NsfbColour fill)
{ if ( fill != NOCOLOR )        // Has speficied the color
  { setForeColor( win32, fill );
    setBackColor( win32, fill );
  }

  return( Polygon( win32->theGC
                 , (POINT*)points
                 , npt ));
}


static bool win32Polylines( struct w32List  * win32
                          , int npt, const NsfbPoint * points
                          , NsfbPlotpen * pen )
{ setForeColor( win32, pen->strokeColour );

  POINT* list= (POINT*)points;

  return( Polyline( win32->theGC
                  , list
                  , npt ));
}


/** Plots a point.
 *
 * @brief Plot a single alpha blended pixel.
 */
static bool win32Point( struct w32List  * win32
                      , int x, int y
                      , NsfbColour fore )
{ return( SetPixel
          ( win32->theGC
          , x, y
          , fore & 0x00FFFFFF ));
}

/** Plots an arc, around (x,y), from anticlockwise from angle1 to
 *		  angle2. Angles are measured anticlockwise from horizontal, in
 *		  degrees.
 */
static bool win32Arc( struct w32List * win32
                    , int x, int y, int radius
                    , int angle1, int angle2, NsfbColour c )
{ setForeColor( win32, c );

  return( AngleArc( win32->theGC
                  , x, y
                  , radius
                  , angle1, angle2 ));
}

/** Plot an ellipse.
 *
 * plot an ellipse outline, note if teh bounding box is square this will plot a
 * circle.
 */
static bool win32Ellipse( struct w32List * win32
                        , NsfbBbox       * ellipse
                        , NsfbColour fore )
{ setForeColor( win32, fore );

  return( Ellipse( win32->theGC
                 , ellipse->x0
                 , ellipse->y0
                 , ellipse->x1
                 , ellipse->y1 ));
}


/** Plot a filled ellipse.
 *
 * plot a filled ellipse, note if the bounding box is square this will plot a
 * circle.
 */
static bool win32EllipseFill( struct w32List * win32
                            , NsfbBbox       * ellipse
                            , NsfbColour fore )
{ setForeColor( win32, fore );

  return( Ellipse( win32->theGC
                 , ellipse->x0
                 , ellipse->y0
                 , ellipse->x1
                 , ellipse->y1 ));
}


static bool win32Rectangle( struct w32List * win32
                          , NsfbBbox       * rect
                          , int lineWidth, NsfbColour fore, bool dotted, bool dashed )
{ setForeColor( win32, fore );

  return( Rectangle( win32->theGC
                   , rect->x0, rect->y0
                   , rect->x1, rect->y1 ));
}

/** Plots a filled rectangle. Top left corner at (x0,y0), bottom
 *		  right corner at (x1,y1). Note: (x0,y0) is inside filled area,
 *		  but (x1,y1) is below and to the right. See diagram below.
 */
static bool win32Fill( struct w32List * win32
                     , NsfbBbox       * rect
                     , NsfbColour back )
{ byte r= back; back >>= 8;
  byte g= back; back >>= 8;
  byte b= back; back >>= 8;

              back  = r;
  back <<= 8; back |= g;
  back <<= 8; back |= b;

  HBRUSH brush= CreateSolidBrush( back );

  bool ret= FillRect( win32->theGC
                    , (RECT*)rect
                    , brush );
  DeleteObject( brush );
  return( ret );
}




/** Plot bitmap
 */
static bool win32Bitmap( struct w32List  * nsfb
                       , const NsfbBbox   * loc
                       , const NsfbColour * pixel
                       , int bmpWidth, int bmpHeight, int bmp_stride
                       , int alpha )

{
  return( true );
}

/** Plot tiled bitmap
 */
static bool win32BitmapTiles( struct w32List  * nsfb
                          , const NsfbBbox *box
                          , int tiles_x, int tiles_y
                          , const NsfbColour *pixel
                          , int bmpWidth, int bmpHeight, int bmp_stride
                          , int alpha )
{ puts("TO TILES");
  return( false );
}


/** Copy an area of screen
 *
 * Copy an area of the display.
 */
static bool win32Copy( struct w32List  * nsfb
                     , NsfbBbox * srcbox, NsfbBbox *dstbox )
{
  puts(" COPY");

  return( false );
}


/** Plot an 8 bit per pixel glyph.
 */
static bool win32Glyph8( struct w32List  * nsfb
                     , NsfbBbox *loc
                     , const byte *pixel, int pitch
                     , NsfbColour c, NsfbColour b )
{ puts("TO G8");
  return( false );
}


/** Plot an 1 bit per pixel glyph.
 */
static bool win32Glyph1( struct w32List  * nsfb
                       , NsfbBbox *loc, const byte *pixel
                       , int pitch, NsfbColour c )
{ puts("TO G1");
  return( false );
}

/** Read rectangle of screen into buffer
 */
static bool win32Readrect( struct w32List    * nsfb
                       , NsfbBbox   * rect
                       , NsfbColour * buffer )
{ puts("TO >RRE");
  return( false );
}

/** Move rectangle of screen
 */
static int win32Moverect( struct w32List  * drv
                        , int  w, int h
                        , int  x, int y )
{ RECT rgn={ x, w
           , y, h };

  rgn.left= rgn.right = x; rgn.right  += w+1;
  rgn.top = rgn.bottom= y; rgn.bottom += h+1;

  return( InvalidateRect( drv->theWindow, &rgn, 1 ));
}

 //XCopyArea( drv->theDisplay
    //       , drv->theSurface, drv->theWindow
      //     , drv->theGC
        //   , x, y
          // , w, h
         //  , x, y ) ;
 // XFlush( drv->theDisplay );



//  return( true );
//}

/** Plots a line using a given pen.
 */
static bool win32Line( struct w32List  * w32, int linec
                     , NsfbBbox * line, NsfbPlotpen * pen )
{ setForeColor( w32, pen->strokeColour );

  if ( MoveToEx( w32->theGC
               , line->x0, line->y0
               , NULL ))
  { if( LineTo( w32->theGC
              , line->x1, line->y1 ))
    { return( true );
  } }

  return( false );
}




static bool PixmapFill( struct w32List * nsfb
                      , ImageMap       * img
                      , int x, int y, int offy, int capy
                      , COLORREF back )
{ if ( !img )
  { return( false );
  }

  if ( ! SelectObject( nsfb->theMM
                     , img->image ))
  { return( false );
  }

  BITMAP info;

  if ( !GetObject( img->image                // Need be in memory
                 , sizeof( info )
                 ,        &info ))
  { return( 0 );
  }

  if ( capy )                                // Only redraw a section
  { info.bmHeight= capy;
  }


  if ( img->mask )
  { if ( back != NOCOLOR )
    { HBRUSH brush= CreateSolidBrush( back );

      HGDIOBJ old= SelectObject( nsfb->theGC, brush );

      MaskBlt( nsfb->theGC         // theDisplay + theWindow + thisGC
             , x, y
             , info.bmWidth
             , info.bmHeight
             , nsfb->theMM
             , 0, offy
             , img->mask // + image->mask
             , 0, offy
             , MAKEROP4( SRCCOPY, PATCOPY ));   //     , 0x00AA0029 ));

     SelectObject( nsfb->theGC, old );

     return( DeleteObject( brush ));
    }

    return( MaskBlt( nsfb->theGC         // theDisplay + theWindow + thisGC
                   , x, y
                   , info.bmWidth
                   , info.bmHeight
                   , nsfb->theMM
                   , 0, offy
                   , img->mask // + image->mask
                   , 0, offy
                   , MAKEROP4( SRCCOPY, DSTCOPY )));   //     , 0x00AA0029 ));
  }

  return( BitBlt( nsfb->theGC
                , x, y
                , info.bmWidth
                , info.bmHeight
                , nsfb->theMM
                , 0, offy
                , SRCCOPY ));
}




/** ================================================= [ JACS, 10/01/2004 ] == *\
 *                                                                            *
 *   JASC 2004                                                                *
 *                                                                            *
 *  TABLE win32PlottersAddr.                                                    *
 *                                                                            *
 *  @brief set win32 plotters                                                 *
 *                                                                            *
\* ========================================================================= **/
/*
 */
struct NsfbPlotterFns w32PlottersAddr=
{ clg        : ( NsfbPlotfnClg         ) plotClg           // NsfbPlotfnClg
, pan        : ( NsfbPlotfnPan         ) plotPan           // NsfbPlotfnPan
, rectangle  : ( NsfbPlotfnRectangle   ) win32Rectangle    // NsfbPlotfnRectangle
, line       : ( nsfbPlotfnLine        ) win32Line         // nsfbPlotfnLine
, polygon    : ( NsfbPlotfnPolygon     ) win32Polygon      // NsfbPlotfnPolygon
, fill       : ( NsfbPlotfnFill        ) win32Fill         // NsfbPlotfnFill
, getClip    : ( NsfbPlotfnClip        ) win32GetClip      // NsfbPlotfnClip
, setClip    : ( NsfbPlotfnClip        ) win32SetClip      // NsfbPlotfnClip
, ellipse    : ( NsfbPlotfnEllipse     ) win32Ellipse      // NsfbPlotfnEllipse
, ellipseFill: ( NsfbPlotfnEllipseFill ) win32EllipseFill  // NsfbPlotfnEllipseFill
, pixmapFill : ( NsfbPlotfnPutPixmap   ) PixmapFill        // NsfbPlotfnBitmap
, arc        : ( NsfbPlotfnArc         ) win32Arc          // NsfbPlotfnArc
, bitmap     : ( NsfbPlotfnBitmap      ) win32Bitmap       // NsfbPlotfnBitmap
, bitmapTiles: ( NsfbPlotfnBitmapTiles ) win32BitmapTiles  // NsfbPlotfnBitmapTiles
, point      : ( NsfbPlotfnPoint       ) win32Point        // NsfbPlotfnPoint
, copy       : ( NsfbPlotfnCopy        ) win32Copy         // NsfbPlotfnCopy
, glyph8     : ( NsfbPlotfnGlyph8      ) win32Glyph8       // NsfbPlotfnGlyph8
, glyph1     : ( NsfbPlotfnGlyph1      ) win32Glyph1       // NsfbPlotfnGlyph1
, readrect   : ( NsfbPlotfnReadrect    ) win32Readrect     // NsfbPlotfnReadrect
, moverect   : ( NsfbPlotfnMoverect    ) win32Moverect     // NsfbPlotfnMoverect
, quadratic  : ( NsfbPlotfnQuadratic   ) plotQuadratic     // NsfbPlotfnQuadratic
, cubic      : ( NsfbPlotfnCubicBezier ) plotCubic         // NsfbPlotfnCubicBezier
, path       : ( nsfbPlotfnPath        ) plotPath          // nsfbPlotfnPath
, polylines  : ( NsfbPlotfnPolylines   ) win32Polylines    // NsfbPlotfnPolylines
};



