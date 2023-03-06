/*
 * Jose Angel SÄnchez Caso (C) 2003.  (JASC)
 *
 *  altomaltes@yahoo.es
 *
 *  Jul-2004
 *
 */

#include <stdio.h>
#include <setjmp.h>
#include <malloc.h>

#include <jerror.h>
#include <jpeglib.h>

#include "nsfb.h"
#include "../images.h"




struct jpeg_error_mgr* ( *jpegStdError ) ( struct jpeg_error_mgr * err );





#define jCreateDecompress( w ) jpegCreateDecompress( w, JPEG_LIB_VERSION, sizeof( *w ));


typedef struct
{ struct jpeg_error_mgr pub;
         jmp_buf setjmp_buffer;
} my_error_mgr;


/** ====================================================[ JACS 1997-11-11 ]== *\
 *                                                                            *
 *   JACS 2011                                                                *
 *                                                                            *
 *  FUNCTION: loadImgFile                                                     *
 *                                                                            *
 *  @brief Loads a jpeg image from disk.                                      *
 *                                                                            *
\* ========================================================================= **/
ANSIC DeviceImageRec * loadImgFile( const char * fname, int wtarget, int htarget  )
{ struct jpeg_decompress_struct info;
  int    w,h;
  my_error_mgr jerr;     /* Gestor de errores       */
  FILE * fp;

  if ( ! (fp= fopen( fname, "rb" )))
  { return( NULL );
  }

  info.err= jpeg_std_error( &jerr.pub );

  if ( setjmp(jerr.setjmp_buffer ))
  { goto Salir;
  }

  jpeg_create_decompress( &info       );
  jpeg_stdio_src    (     &info, fp   );
  jpeg_read_header  (     &info, TRUE );

  info.dct_method         = JDCT_FASTEST;
  info.do_fancy_upsampling= FALSE;
  info.out_color_space    = JCS_RGB;

  info.scale_num          = 1;
  info.scale_denom        = 1;
  jpeg_calc_output_dimensions( &info );   /* note colorspace changes... */

  int szW= info.output_width;
  int szH= info.output_height;

  { int west= szW;
    int hest= szH;

    while   (( west < ( info.output_width  / 2 ))
          && ( hest < ( info.output_height / 2 ))
          && ( info.scale_denom <= 8 ))
    { west <<= 1; hest <<= 1; info.scale_denom <<= 1;
  } }

  jpeg_calc_output_dimensions( &info );   /* note colorspace changes... */
  w= info.output_width;
  h= info.output_height;

//  initChangerSize( szW, szH    /* Destination wide & height */
//                 ,   w, h  );  /* Original    wide & height    */

//  pic= initImageMap( changer= allocChanger( info.output_components, *szW ) /* RGB color  */
//                   , NULL                                                  /* No mask    */
//                   , info.output_components, 1                             /* Planes        */
//                   , *szW, *szH
//                   , w, h );                                               /* workspace */

  int pics= 1;

  ChangerRec     * changerAlpha;         /* Mask can change size    */
  DeviceImageRec * image= initAlphaMap( changerAlpha= allocChanger( 4, szW ) /* RGB color     */
                                      , pics                                  /* Planes        */
                                      , szW, szH                            /* Final size    */
                                      , w, h );                         /* Original size */


  jpeg_start_decompress( &info );

  while (info.output_scanline < info.output_height )
  { char * row= changerLine( changerAlpha  );

    jpeg_read_scanlines( &info, &row, 1 );

    changeImageAddRGB( changerAlpha ); /* From RGB to 32 bit alpha */
  }

  jpeg_finish_decompress( &info );

Salir:
  jpeg_destroy_decompress( &info );
  fclose( fp    );
  return( image   );
}



