/**
 *     AUTHOR: Jose Angel Caso Sanchez, 1993   ( altomaltes@yahoo.es )
 *                                             ( altomaltes@gmail.com )
 *
 *     Copyright (C) 1993, 2012 JACS
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
 * along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *     FILE: icons.c
 *     DATE: jul 2003
 *
 *  DESCRIPCION: windows ico loader
 *
 */

#include <stdio.h>
#include <string.h>

#include "formats/rdsvg/svgtiny.h"

#include "../nsfb.h"
#include "../nsfbPlot.h"

#ifndef __WIN32
  #include <alloca.h>
#endif

#define N_SEG 30



/**
 * codlets
 */
static int nfsbMoveTo( Nsfb * nsfb
                     , int x,	int y )
{ puts("MOVETO");

  return( 0 );
}

int nsfbSetSourceRgb( Nsfb * nsfb
  		                 , int r, int g, int b )
{ puts("nfsbSetSourceRgb");

  return( 0 );
}

int nfsbShowText( Nsfb * nsfb
                 , const char * text )
{ puts("SHOW TEXT");

  return( 0 );
}

/**
 * Render an svgtiny path using nfsb.
 */
static void renderPath( Nsfb * nsfb
                      , int offx, int offy
                      , int orgx, int tgtx
                      , int orgy, int tgty
                      , struct svgtiny_shape * path )
{ int points, code, j;
  float * src;

  NsfbPoint * holder
          , * dst;

  for ( j = 0, points= 0, src= path->path
	     ; j < path->path_length
	     ; j++ , points ++ )
	 { int code= *src++;

	   switch ( code )
	   { case NFSB_PLOT_PATHOP_QUAD:
	       points += N_SEG;
	     break;
		}	}

               points+=10;
  holder= (NsfbPoint *)alloca( points * sizeof( *holder ));

  /* Xlate format
 */
	 for ( j = 0, src= path->path, dst= holder
	     ; j < path->path_length
	     ; j++ )
	 { int code= *src++;

	   switch ( code )
	   { case NFSB_PLOT_PATHOP_QUAD:
	     {  NsfbBbox  curve;
         NsfbPoint ctrla;

         curve.x0= ( *src++ * tgtx ) / orgx + offx; j++;
         curve.y0= ( *src++ * tgty ) / orgy + offy; j++;

         ctrla.x = ( *src++ * tgtx ) / orgx + offx; j++;
         ctrla.y = ( *src++ * tgty ) / orgy + offy; j++;

         curve.x1= ( *src++ * tgtx ) / orgx + offx; j++;
         curve.y1= ( *src++ * tgty ) / orgy + offy; j++;

         dst+= quadraticPoints( N_SEG, dst, &curve, &ctrla );
      }
      break;

	     case NFSB_PLOT_PATHOP_MOVE:
     	case NFSB_PLOT_PATHOP_LINE:
     	  dst->x= ( *src++ * tgtx ) / orgx + offx; j++;
     	  dst->y= ( *src++ * tgty ) / orgy + offy; j++; dst++;
    		break;

     	case NFSB_PLOT_PATHOP_CLOSE:
     	{ NsfbPlotpen pen;


        pen.strokeType=   NFSB_PLOT_OPTYPE_SOLID;
        pen.strokeColour= path->stroke;                /**< Colour of stroke */
        pen.strokeWidth=  path->stroke_width ; // * ( scalex + scaley ) / 2;  /**< Width of stroke, in pixels */
        pen.fillColour=   path->fill;
        pen.fillType=     NFSB_PLOT_OPTYPE_SOLID;

        nsfbPlotpolygon(   nsfb, (const int *)holder, dst-holder, path->fill );
        nsfbPlotpolylines( nsfb, dst-holder, holder, &pen );

     	  dst->x= ( holder->x * tgtx ) / orgx + offx; j++;
     	  dst->y= ( holder->y * tgty ) / orgy + offy; dst++;
     	}
    		break;

		    default:
			     printf( "error in __FUNCTION__ \n"  );
}	} }





/** ========================================= [ JACS, 10/01/2012 ] == *\
 *                                                                    *
 *   JASC 2012                                                        *
 *                                                                    *
 *  FUNCTION nsfbRenderDeviceVects                                    *
 *                                                                    *
 *  @brief                                                            *
 *                                                                    *
\* ================================================================= **/
ANSIC bool nsfbRenderDeviceVects( Nsfb      * nsfb
                                , VectorRec * diagram
                                , int offx, int offy
                                , int tgtw, int tgth  )
{ if ( diagram )
  { int i;

  	 for ( i =  0
        ; i != diagram->shape_count
	       ; i ++ )
  	 { if ( diagram->shape[ i ].path )
	     { renderPath( nsfb
	                 , offx, offy
	                 , diagram->width,  tgtw
	                 , diagram->height, tgth
	                 , diagram->shape + i );
    		}
    		else if ( diagram->shape[ i ].text )
  		  { nsfbSetSourceRgb( nsfb
  		                    , svgtiny_RED(   diagram->shape[i].stroke) / 255.0
  		                    , svgtiny_GREEN( diagram->shape[i].stroke) / 255.0
  		                    , svgtiny_BLUE(  diagram->shape[i].stroke) / 255.0 );

     			nfsbMoveTo( nsfb, ( diagram->shape[ i ].text_x * tgtw ) / diagram->width  + offx
      			               ,	( diagram->shape[ i ].text_y * tgth ) / diagram->height + offy );
   	  		nfsbShowText( nsfb, diagram->shape[ i ].text);
  }	} }
  else
  { fprintf( stderr, "Vector image not found\n" );
  }

  return( true );
}



typedef VectorRec *  (*LoadImgCode) ( const char * fname, int wtarget, int htarget  );


/** ========================================= [ JACS, 10/01/2012 ] == *\
 *                                                                    *
 *   JASC 2012                                                        *
 *                                                                    *
 *  FUNCTION LoadVecFile                                              *
 *                                                                    *
 *  @brief                                                            *
 *                                                                    *
\* ================================================================= **/
VectorRec * LoadVecFile( const char * fileName, int wtarget, int htarget )
{ typedef struct
  { const char * ext;   /* File extension */
    LoadImgCode codec;  /* Loader         */
  } VecCodecsRec;

  static VecCodecsRec codecs[]=
  {{ ".svg", loadImgSvgFile }
  ,{ NULL , NULL        }};

  VecCodecsRec * codec;

  for( codec= codecs
     ; codec->ext
     ; codec ++ )
  { if ( strstr( fileName
               , codec->ext ))
    { return( codec->codec( fileName, wtarget, htarget ));
  } }

  return( NULL );
}

