/**
 *     AUTHOR: Jose Angel Caso Sanchez, 2003   ( altomaltes@yahoo.es )
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *     FILE: rdgif.c
 *     DATE: jul 2003
 *
 *  DESCRIPCION: gif file type reader (plugin type)
 *
 */


#include <alloca.h>

#define PNG_SETJMP_SUPPORTED
#include <png.h>

#include "nsfb.h"
#include "../images.h"



/** ====================================================[ JACS 1997-11-11 ]== *\
 *                                                                            *
 *   JACS 2011                                                                *
 *                                                                            *
 *  FUNCTION: LoadGifFile                                                     *
 *                                                                            *
 *  @brief Loads a gif image from disk.                                       *
 *                                                                            *
\* ========================================================================= **/
DeviceImageRec * LoadPng( const char * fname
                        , int * szW, int * szH  )
{ png_structp pngPtr  = NULL;
  png_infop  infoPtr  = NULL;
  DeviceImageRec * dev= NULL;

/*    Create and initialize the png_struct with the desired error handler
 * functions.  If you want to use the default stderr and longjump method,
 * you can supply NULL for the last three parameters.  We also supply the
 * the compiler header file version, so that we know if the application
 * was compiled with a compatible version of the library.  REQUIRED
 */
/* Set error handling if you are using the setjmp/longjmp method (this is
 * the normal method of doing things with libpng).  REQUIRED unless you
 * set up your own error handlers in the png_create_read_struct() earlier.
 */

  if (( pngPtr= pmg_create_read_struct( PNG_LIBPNG_VER_STRING
                                   , NULL           /* png_voidp user_error_ptr */
                                   , NULL           /* user_error_fn            */
                                   , NULL )))       /* user_warning_fn          */
  { if (( infoPtr= pngCreateInfoStruct( pngPtr )))  /* Allocate/initialize the memory for image information.  REQUIRED. */
    { FILE * fp= fopen( fname, "rb");

      if ( fp )
      { ChangerRec * changerRGB;     /* Mask can change size    */

        int pics= 1, interlace
          , depth, ctype;
        png_uint_32 wOrg, hOrg;  /* Original wide and height    */
        unsigned char * row;


 //       if ( pngSetLongjmpFn( pngPtr
   //                         , longjmp, sizeof (jmp_buf)))
     //   { goto resume;
       // }

        pngInitIO  ( pngPtr, fp      );      /* Set up the input control if you are using standard C streams */
        pngReadInfo( pngPtr, infoPtr );
        pngGetIHDR ( pngPtr, infoPtr
                   , &wOrg, &hOrg
                   , &depth, &ctype
                   , &interlace
                   , NULL, NULL );


        if ( szW ) { *szW= wOrg; }
        if ( szH ) { *szH= hOrg; }

        pngSetStrip16( pngPtr ); /* Tell libpng to strip 16 bit/color files down to 8 bits/color */
//        png_set_strip_alpha( pngPtr ); /* Strip alpha bytes from the input data without combining background (not rec.) */
/*   png_set_packing(     pngPtr ); Pixels with bit depths of 1, 2, and 4 into separate bytes */
/*   png_set_packswap(    pngPtr ); Change the order of packed pixels  */
      //  png_set_bgr( pngPtr );

    /*    png_color_16 my_background, *image_background;

        if ( png_get_bKGD( pngPtr
                         , infoPtr
                         , &image_background ))
        { png_set_background( pngPtr
                            , image_background
                            , PNG_BACKGROUND_GAMMA_FILE, 1, 1.0);
        }
        else
        { png_set_background( pngPtr
                            , &my_background
                            , PNG_BACKGROUND_GAMMA_SCREEN, 0, 1.0);
         }
          */

       // png_set_bKGD( pngPtr, infoPtr, 0 );

        pngSetExpand( pngPtr );

        switch( ctype )
        { case PNG_COLOR_TYPE_PALETTE: /* Expand paletted colors into true RGB triplets */
            pngSetPaletteToRGB( pngPtr );
          break;

          case PNG_COLOR_TYPE_GRAY:    /* Expand grayscale images to the full 8 bits from 1, 2, or 4 bits/pixel */
            if ( depth < 8 )
            { pngSetExpandGray124to8( pngPtr );
            }
          break;

          case PNG_COLOR_TYPE_RGB:
//           puts("PNG_COLOR_TYPE_RGB");
          break;

          case PNG_COLOR_TYPE_RGB_ALPHA:
  //         puts("PNG_COLOR_TYPE_RGB_ALPHA");
          break;

          case PNG_COLOR_TYPE_GRAY_ALPHA:
    //       puts("PNG_COLOR_TYPE_GRAY_ALPHA");
          break;
        }


        dev= initImageMap
             ( changerRGB=  allocChanger( 3, wOrg ) // Add alpha channel
             , NULL /* changerMask= allocChanger( 1, wOrg ) */
             ,   24, pics    /* Number of planes             */
             , wOrg, hOrg    /* Destination wide and  height */
             , wOrg, hOrg ); /* Original wide and height     */

        row= alloca( wOrg * 4 );              /* three colors plus alpha channel */
 /* Read the image a single row at a time */
      /*  for( pass = 0
           ; pass < number_passes
           ; pass++ )   */
        { int vert;

          for( vert = 0
             ; vert < hOrg
             ; vert++ )
          { row= changerLine( changerRGB  );
            pngReadRow( pngPtr, row, NULL );
            changeImageAddLine( changerRGB  ); /* Line */
        } } }
        fclose(fp);
    }
    pngDestroyReadStruct( &pngPtr
                        , &infoPtr
                        , NULL ); /* Free all of the memory associated with the png_ptr and info_ptr */
  }

  return( dev );
}

PUBLIC unsigned fillPng(  )
{ return( 0 );
}













