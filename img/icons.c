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

#include "../libnsfb_plot_util.h"
#include "images.h"

#ifndef __WIN32
  #include <alloca.h>
#endif


/** ========================================= [ JACS, 10/01/2012 ] == *\
 *                                                                    *
 *   JASC 2012                                                        *
 *                                                                    *
 *  FUNCTION openIcoFromData2                                          *
 *                                                                    *
 *  @brief                                                            *
 *                                                                    *
\* ================================================================= **/
DeviceImageRec * openIcoFromData2
                 ( int          deep            /* Histogram          */
                 , int pics, int cls            /* Numbero of images (pals)  */
                 , int wDst, int hDst           /* desired width and height */
                 , int wOrg, int hOrg           /* stored  width and height */
                 , unsigned char * picture      /* Image data         */
                 , ImgPalette    * palette )    /* Returns image mask */
{ DeviceImageRec  * image;
  unsigned char * linePtr, * maskPtr;

  ChangerRec * changerRGB;          /* Mask can change size    */
  ChangerRec * changerMask;         /* Mask can change size    */

  int pals= 1;                      /* Default values          */
  int cols= 1;

  if ( cls )    /* Multiicon */
  { cols= cls & 0xFFFF;
    pals= cls << 16;

    if ( !pals )
    { pals= 1;
  } }

  image= initImageMap( changerRGB=  allocChanger( 3, wDst ) /* RGB color     */
                     , changerMask= allocChanger( 1, wDst ) /* Mask          */
                     , deep, pics * pals                    /* Planes        */
                     , wDst, hDst                           /* Final size    */
                     , wOrg, hOrg );                        /* Original size */

  while ( pics-- )
  { int horz; ImgPalette * palPtr;        /* Run all avatars */

    for( horz= pals, palPtr= palette
       ; horz
       ; horz --   , palPtr += cols )
    { int vert;

      unsigned char * idx= picture;   /* Next entry */

      for( vert= 0
         ; vert < hOrg
         ; vert ++ )
      { int horz;

        linePtr= changerLine( changerRGB  );
        maskPtr= changerLine( changerMask );

        for( horz= 0
           ; horz < wOrg
           ; horz ++)
        { unsigned char pal= *idx ++;

          *maskPtr++= ( palPtr[ pal ].alpha & 0x0F ) ? 0xFF : 0x00;
          *linePtr++=   palPtr[ pal ].red;
          *linePtr++=   palPtr[ pal ].green;
          *linePtr++=   palPtr[ pal ].blue;
        }

        changeImageAddLine( changerRGB  ); /* Line */
        changeImageAddLine( changerMask ); /* Mask */
      }
      palPtr += cols;
    }
    while(( palPtr->red & palPtr->green & palPtr->blue & palPtr->alpha ) != 0xFF );
  }

  return( image );
}


DeviceImageRec * openIcoFromData1
                  ( unsigned char * picture  /* Image data               */
                  , int wDst, int hDst       /* desired width and height */
                  , int wOrg, int hOrg       /* stored  width and height */
                  , int pics, int pals       /* Numbero of palettes      */ /* Number of pictures       */
                  , ImgPalette * palette )   /* Returns image mask       */
{ DeviceImageRec * image;
  ChangerRec     * changerALPHA;   /* Mask can change size    */
  int cols, rows, loop, vert;
  ImgPalette * palPtr;
  unsigned char * pic;        /* Run all avatars */
//  int order= 0;

  if ( pals == 0x0905 )
  { puts("hola");
}

  if ( pals )                              /* Multiicon */
  { cols= pals;      cols &= 0xFF; 
    pals= pals >> 8; pals &= 0xFF;
  } 
  else
  { cols= pals= 1;
  }

  if ( !pals )
  { pals= 1;
  } 

  image= initAlphaMap( changerALPHA= allocChanger( 4, wDst ) /* RGB color     */
                     , pics * pals                           /* pictures      */
                     , wDst, hDst                            /* Final size    */
                     , wOrg, hOrg );

  /* Original size */

  if ( pics > 1 )
  {  printf( "ORDER: %d PALS: %d ROWS: %d COLD: %d colsxx: %d\n"
           , pals, pics, hOrg, wOrg, cols );


    for( rows= pals, palPtr= palette
       ; rows
       ; rows --   , palPtr += cols )
    { for( loop= 0, pic= picture
         ; loop< pics
         ; loop ++ )
      { for( vert= 0
           ; vert < hOrg
           ; vert ++ )
        { int horz;

          unsigned char * linePtr= changerLine( changerALPHA  );

          for( horz= 0
             ; horz < wOrg
             ; horz ++)
          { unsigned char pal= *pic ++;

            *linePtr++=  palPtr[ pal ].red;
            *linePtr++=  palPtr[ pal ].green;
            *linePtr++=  palPtr[ pal ].blue;
            *linePtr++=  palPtr[ pal ].alpha;
          }
          changeImageAddLine( changerALPHA  ); /* Line */
  } } } }

  else
  {  printf( "NATUR: %d PALS: %d ROWS: %d COLD: %d\n" , pics, pals, hOrg, wOrg );

    while ( pics-- )                    /* A image per palette */
    { int hight; ImgPalette * palPtr;

      for( hight= pals, palPtr= palette
         ; hight
         ; hight --   , palPtr += cols )
      { int vert;

        for( vert= 0, pic= picture
           ; vert < hOrg
           ; vert ++ )
        { int horz;

          unsigned char * linePtr= changerLine( changerALPHA  );

          for( horz= 0
             ; horz < wOrg
             ; horz ++)
          { unsigned char pal= *pic ++;

            *linePtr++=  palPtr[ pal ].red;
            *linePtr++=  palPtr[ pal ].green;
            *linePtr++=  palPtr[ pal ].blue;
            *linePtr++=  palPtr[ pal ].alpha;
          }
          changeImageAddLine( changerALPHA  ); /* Line */
  } } } }

  return( image );
}


/** ========================================= [ JACS, 10/01/2012 ] == *\
 *                                                                    *
 *   JASC 2012                                                        *
 *                                                                    *
 *  FUNCTION openIco                                                  *
 *                                                                    *
 *  @brief                                                            *
 *                                                                    *
\* ================================================================= **/
//DeviceImageRec * openIco( IcoRec * src, int deep   /* Histogram         */
//                        , int     wDst, int hDst ) /* IN ->  Icon size  */
//{ return( openIcoFromData2
//          ( deep
//          ,       1, src->cln       /* Pics and pals            */
//          , wDst   , hDst    /* desired width and height */
//          , src->w0, src->h0 /* stored  width and height */
//          , src->pic         /* image data               */
 //         , src->pal ));     /* color list               */
//}

typedef IcoRec *  (*LoadIcoCode) ( const char * fname  );


/** ========================================= [ JACS, 10/01/2012 ] == *\
 *                                                                    *
 *   JASC 2012                                                        *
 *                                                                    *
 *  FUNCTION LoadIcoFile                                              *
 *                                                                    *
 *  @brief                                                            *
 *                                                                    *
\* ================================================================= **/
IcoRec * LoadIcoFile( const char * fileName )
{ typedef struct
  { const char * ext;   /* File extension */
    LoadIcoCode codec;  /* Loader         */
  } IcoCodecsRec;

  static IcoCodecsRec codecs[]=
  {
//  { "xpm", LoadXpmFile }
//  ,{ "gif", LoadGifFile }
//  ,{ "ico", LoadICOFile }
//  ,{ "png", LoadPng }
  { NULL , NULL        }};

  IcoCodecsRec * codec;

  for( codec= codecs
     ; codec->ext
     ; codec ++ )
  { if ( strstr( fileName
               , codec->ext ))
    { return( codec->codec( fileName ));
  } }

  return( NULL );
}

