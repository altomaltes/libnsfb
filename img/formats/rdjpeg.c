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


/*
 *
 *
 */
DeviceImageRec * LoadJfif( const char * fName
                         , int        * szW, int * szH )
{ struct jpeg_decompress_struct info;
  DeviceImageRec * pic= NULL;
  unsigned char * rowptr;
  int    w,h;
  my_error_mgr jerr;     /* Gestor de errores       */
  ChangerRec * changer;  /* RGB changers */
  FILE * fp;

  if ( ! (fp= fopen( fName, "rb" )))
  { return( NULL );
  }

  info.err= jpeg_std_error( &jerr.pub );

  if ( setjmp(jerr.setjmp_buffer ))
  { if ( pic )
    { free(pic);
    }
    pic= NULL;
    goto Salir;
  }

  jpeg_create_decompress( &info       );
  jpeg_stdio_src    ( &info, fp   );
  jpeg_read_header  ( &info, TRUE );

  info.dct_method         = JDCT_FASTEST;
  info.do_fancy_upsampling= FALSE;
  info.out_color_space    = JCS_RGB;

  info.scale_num          = 1;
  info.scale_denom        = 1;
  jpeg_calc_output_dimensions( &info );   /* note colorspace changes... */

  if ( ! *szW )
  { *szW= info.output_width;
  }

  if ( ! *szH )
  { *szH= info.output_height;
  }

  { int west= *szW;
    int hest= *szH;

    while   (( west < ( info.output_width  / 2 ))
          && ( hest < ( info.output_height / 2 ))
          && ( info.scale_denom <= 8 ))
    { west <<= 1; hest <<= 1; info.scale_denom <<= 1;
  } }

  jpeg_calc_output_dimensions( &info );   /* note colorspace changes... */
  w= info.output_width;
  h= info.output_height;

  initChangerSize( szW, szH    /* Destination wide & height */
                 ,   w, h  );  /* Original    wide & height    */

  pic= initImageMap( changer=  allocChanger( info.output_components, *szW ) /* RGB color  */
                   , NULL                                                   /* No mask    */
                   , info.output_components, 1                           /* Planes        */
                   , *szW, *szH
                   , w, h );   /* workspace */

  rowptr= changerLine( changer );

  jpeg_start_decompress( &info );

  while (info.output_scanline < info.output_height )
  { jpeg_read_scanlines( &info, &rowptr, 1 );
    changeImageAddLine( changer ); /* Feed resizer */
  }

  jpeg_finish_decompress( &info );

Salir:
  jpeg_destroy_decompress( &info );
  fclose( fp    );
  return( pic   );
}



