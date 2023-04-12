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
#include "../surface.h"

#ifndef __WIN32
  #include <alloca.h>
#endif


/** ========================================= [ JACS, 10/01/2012 ] == *\
 *                                                                    *
 *   JASC 2012                                                        *
 *                                                                    *
 *  FUNCTION openIcoFromData222222                                          *
 *                                                                    *
 *  @brief                                                            *
 *                                                                    *
\* ================================================================= **/
DeviceImageRec * openIcoFromData222222
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


static DeviceImageRec * openIcoFromData
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
  { // printf( "NATUR: %d PALS: %d ROWS: %d COLD: %d\n" , pics, pals, hOrg, wOrg );

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
 *  @brief explodes a hardcoded icon                                  *
 *                                                                    *
\* ================================================================= **/
ANSIC DeviceImageRec * openIco( IcoRec * ico )
{ return( openIcoFromData( ico->pic             /* image data               */
                         , ico->wNat, ico->hNat /* desired width and height */
                         , ico->wNat, ico->hNat /* stored  width and height */
                         , ico->pics, ico->nCol /* Keep palette            */
                         , ico->pal ));         /* color list               */
}


typedef IcoRec         *  (*LoadIcoCode) ( const char * fname, int w, int h  );
typedef DeviceImageRec *  (*LoadImgCode) ( const char * fname, int w, int h  );

ANSIC IcoRec * loadImgVecFile( const char * fName, int wide, int height ) /* Vector to raster */
{
}



/** ========================================= [ JACS, 10/01/2012 ] == *\
 *                                                                    *
 *   JASC 2012                                                        *
 *                                                                    *
 *  FUNCTION LoadIcoFile                                              *
 *           LoadImgFile                                              *
 *                                                                    *
 *  @brief                                                            *
 *                                                                    *
\* ================================================================= **/
IcoRec * LoadIcoFile( const char * fileName, int w, int h )
{ typedef struct
  { const char * ext;   /* File extension */
    LoadIcoCode codec;  /* Loader         */
  } IcoCodecsRec;

  static IcoCodecsRec codecs[]=
  {{ ".gif", loadIcoGifFile }
  ,{ ".png", loadIcoPngFile }
  ,{ ".jpg", loadIcoJpgFile }
  ,{ ".xpm", loadIcoXpmFile }
  ,{ ".ico", loadIcoIcoFile }
  ,{ NULL , NULL        }};

  IcoCodecsRec * codec;

  for( codec= codecs
     ; codec->ext
     ; codec ++ )
  { if ( strstr( fileName
               , codec->ext ))
    { return( codec->codec( fileName, w, h ));
  } }

  return( NULL );
}

DeviceImageRec * LoadImgFile( const char * fileName, int w, int h )
{ typedef struct
  { const char * ext;   /* File extension */
    LoadImgCode codec;  /* Loader         */
  } IcoCodecsRec;

  static IcoCodecsRec codecs[]=
  {{ ".gif", loadImgGifFile }
  ,{ ".png", loadImgPngFile }
  ,{ ".jpg", loadImgJpgFile }
  ,{ ".xpm", loadImgXpmFile }
  ,{ ".ico", loadImgIcoFile }
  ,{ ".svg", loadImgVecFile }
  ,{ NULL , NULL        }};

  IcoCodecsRec * codec;

  for( codec= codecs
     ; codec->ext
     ; codec ++ )
  { if ( strstr( fileName
               , codec->ext ))
    { return( codec->codec( fileName, w, h ));
  } }

  return( NULL );
}

/** ================================================= [ JACS, 10/06/2004 ] == *\
 *                                                                            *
 *   JASC 2004                                                                *
 *                                                                            *
 *  FUNCTION getDeviceImage                                                   *
 *           getDeviceBitmap                                                  *
 *                                                                            *
 *  @brief Constructs and loads and caches a device dependent image           *
 *                                                                            *
\* ========================================================================= **/
ANSIC int getDeviceImage( NsfbSurfaceRtns * surf
                        , ImageMap        * map
                        , void * img
                        , void * msk
                        , int w, int h )
{ return( surf->pixmap( surf, map, img, msk, w, h ) ); /* Register new image */
}

ANSIC DeviceImageRec * getDeviceBitmap( Nsfb * nsfb
                                      , DeviceImageRec  * img )
{ NsfbSurfaceRtns * surf= nsfb->surfaceRtns;

  if( surf )
  { surf->pixmap( surf
                , &img->map
                , img->image, img->mask
                , img->width, img->height );
    return( img );
  }

  return( NULL );
}


int nsfbGetImageGeometry( DeviceImageRec * img
                        , int * w, int * h, int * p )
{ if ( img )
  { if ( w ) { *w= img->width;  }
    if ( h ) { *h= img->height; }
    if ( p ) { *p= img->pics;   }

    return( 0 );
  }
  return( -1 );
}


DeviceImageRec * getIconImage( NsfbSurfaceRtns * surf
                             , IcoRec          * ico
                             , NSFBCOLOUR        hue123 )
{ static DeviceImageRec * seed= NULL;                   /* Linked list storing */

  if ( ico )
  { DeviceImageRec * iter;

/*
 *  Not found, create a new one
 */

    if ( !ico->pal )
    { //ico->pal=  find( hue );
      //ico->cln= ICOREC_MIRROR_PALETTE | 0x0106;
    }

    for( iter= seed                      // Iterate list
       ; iter
       ; iter= iter->next )
    { if ( ico != iter->iden )           // Picture differs
      { continue;
      }

 //   if ( theHistogram != iter->hist )  // Histogran differs
 //   { continue;
 //   }

      if ( iter->iden->pal == ico->pal )
      { return( iter );  // Found !!!
    } }


    iter= openIcoFromData( ico->pic             /* image data               */
                         , ico->wNat, ico->hNat /* desired width and height */
                         , ico->wNat, ico->hNat /* stored  width and height */
                         , ico->pics, ico->nCol /* Keep palette            */
                         , ico->pal );          /* color list               */

    int bmpWidth=  iter->width
      , bmpHeight= iter->height * iter->pics;

    iter->mask= GETALPHA( bmpWidth, bmpHeight, iter->image );

    surf->pixmap( surf
                , &iter->map
                ,  iter->image
                ,  iter->mask
                , bmpWidth
                , bmpHeight  ); /* Register new image */

  //  iter->hist= theHistogram;
    iter->iden= ico;
    iter->next= seed; seed= iter;  // Link list

    return( iter );
  }

  return( NULL );
}



