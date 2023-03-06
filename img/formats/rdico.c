/**
 *     AUTHOR: Jose Angel Caso Sanchez, 1997   ( altomaltes@yahoo.es )
 *                                             ( altomaltes@gmail.com )
 *
 *     Copyright (C) 2004, 2012 JACS
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
 *     FILE: rdico.c
 *     DATE: oct 1997
 *
 *  DESCRIPCION: 3.11 windows resources import
 *
 */



#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <malloc.h>

#include "nsfb.h"
#include "../images.h"

/*
typedef struct
{ short bfType;
  int   bfSize;
  int   reserverd;
  int   bfOffbits;
}  BitmapFileHeader;

typedef struct
{ int biSize;
  int biWidth;
  int biHeight;
  short biPlanes;
  short biBitCount;
  int biCompression;
  int biSizeImage;
  int biXPelsPerMeter;
  int biYPelsPerMeter;
  int biClrUsed;
  int biClrImportant;
}  BitmapInfoHeader;
*/


//#define TRANSPARENT 255

/** ====================================================[ JACS 1997-11-11 ]== *\
 *                                                                            *
 *   JACS 2011                                                                *
 *                                                                            *
 *  FUNCTION: GetBitMap    .                                                  *
 *                                                                            *
 *  @brief                                                                    *
 *                                                                            *
\* ========================================================================= **/
static unsigned char GetBitMap( unsigned char * dev, unsigned char * src
                              , int w, int h, unsigned char alpha )    /* Obtiene un bitmap  */

{ unsigned char * dst= dev + w*h;

  while( h -- )         /* for every line  */
  { int left= w;
    //unsigned char * dst= dev + w*(h+1);

    while( left )
    { int quant= 32;
      dword color;
      unsigned char * ac= (unsigned char*)&color;

      ac[ 3 ]= *src++;
      ac[ 2 ]= *src++;
      ac[ 1 ]= *src++;
      ac[ 0 ]= *src++;

      while( quant && left )
      { dst--; *dst= ( color & 0x80000000 ) ? alpha : 0 ;
        quant--; left--; color <<= 1;
  } } }

  return( alpha );   /* Transparent color */
}



/** ====================================================[ JACS 1997-11-11 ]== *\
 *                                                                            *
 *   JACS 2011                                                                *
 *                                                                            *
 *  FUNCTION: GetQibMap    .                                                  *
 *                                                                            *
 *  @brief Two color planes                                                   *
 *                                                                            *
\* ========================================================================= **/
static int GetQibMap( unsigned char * dst
                    , unsigned char * src
                    , int w, int h
                    , unsigned char alpha )         /* Obtiene un bitmap       */
{ unsigned char qibb;
  int sz= w*h;                        /* Total of image bits     */

  dword * mask= (dword *)alloca( sz ); /* Total of mask pixels    */
  GetBitMap( (unsigned char*)mask, src, w, h, alpha );       /* Temporary copy the mask */

  for( dst+= sz
     ; sz > 0
     ; sz -= 4 )
  { qibb= *src++;
    *dst-- = qibb & 3 ; qibb >>= 2;
    *dst-- = qibb & 3 ; qibb >>= 2;
    *dst-- = qibb & 3 ; qibb >>= 2;
    *dst-- = qibb & 3 ;
  }
  return( alpha );  /* Transparent color */
}

/** ====================================================[ JACS 1997-11-11 ]== *\
 *                                                                            *
 *   JACS 2011                                                                *
 *                                                                            *
 *  FUNCTION: GetPixMap    .                                                  *
 *                                                                            *
 *  @brief 16 bit icons                                                       *
 *                                                                            *
\* ========================================================================= **/
static int GetPixMap( unsigned char * dev
                     , unsigned char * src
                     , int w, int h
                     , int alpha )              /* Obtiene un bitmap  */
{ int size= w*h;
  int wide= (((( w-1 ) >> 4) + 1 ) << 3 );  /* Half quantize to 32 / 2 bit */
  unsigned char * mask= (unsigned char *)alloca( size );      /* Total of mask pixels    */

  GetBitMap( mask                            /* Destination mask */
           , src + wide * h                  /* Start of bitmap  */
           , w, h
           , alpha );                         /* Image size       */
  mask +=  size;

  while( h -- )         /* for every line  */
  { int left= w;
    unsigned char * dst= dev + w*h;

    while( left )
    { int quant= 8;
      dword color;
      unsigned char * ac= (unsigned char*)&color;

      ac[ 3 ]= *src++; ac[ 2 ]= *src++;
      ac[ 1 ]= *src++; ac[ 0 ]= *src++;

      while( quant && left )
      { mask--;
        *dst++ = *mask ?  alpha : ( ( ac[3] & 0xF0 ) >> 4 );
        quant--; left--; color <<= 4;
  } } }

  return( alpha );  /* Transparent color */
}

/** ====================================================[ JACS 1997-11-11 ]== *\
 *                                                                            *
 *   JACS 2011                                                                *
 *                                                                            *
 *  FUNCTION: GetHicMap    .                                                  *
 *                                                                            *
 *  @brief 256 color pixmap                                                   *
 *                                                                            *
\* ========================================================================= **/
static int GetHicMap( unsigned char * dst
                    , unsigned char * src
                    , int w, int h
                    , unsigned char alpha )
{ int wide= (((( w-1 ) >> 2) + 1 ) << 2 );  /* Half quantize to 32 / 2 bit */
  unsigned char * pic;
  unsigned char usedCols[ 256 ];               /* Find transparency         */
  word j, size= w*h;
  unsigned char transparent;

  unsigned char * mask= (unsigned char *)alloca( size );         /* Total of mask pixels    */

  GetBitMap( mask             /* Destination mask */
           , src + wide * h   /* Start of bitmap  */
           , w, h, alpha );          /* Image size       */


  for( memset( usedCols          /* Reset finder          */
             , 0
             , sizeof(usedCols))
     , j=  0
     , pic= src

     ; j< size
     ; j++ )
  { usedCols[ *pic++ ]++;         /* Candidate                       */
  }

  for( transparent= 255
     ; transparent> 0
     ; transparent -- )
  { if ( ! usedCols[ transparent ] )
    { break;   /* Found a free color ! */
  } }

  dst  +=  size;
  mask +=  size;

  while( h -- )         /* for every line  */
  { int left= w;

    while( left )
    { int quant= 4;
      dword color;
      unsigned char * ac= (unsigned char*)&color;

      ac[ 3 ]= *src++;
      ac[ 2 ]= *src++;
      ac[ 1 ]= *src++;
      ac[ 0 ]= *src++;

      while( quant && left )
      {  dst--; mask--;
        *dst = *mask ?  transparent :  ac[3];
        quant--; left--; color <<= 8;
  } } }

  return( transparent );
}



#define getbyte(i, d) \
 i = *d++

#define getword(i, d) \
 i = d[1]; i<<=8; \
 i+= d[0]; d+= sizeof( word )

#define getlong(i, d) \
 i = d[3]; i<<=8; \
 i+= d[2]; i<<=8; \
 i+= d[1]; i<<=8; \
 i+= d[0]; d+= sizeof( long )

/** ====================================================[ JACS 1997-11-11 ]== *\
 *                                                                            *
 *   JACS 2011                                                                *
 *                                                                            *
 *  FUNCTION: LoadIco                                                         *
 *                                                                            *
 *  @brief Loads an ico file                                                  *
 *                                                                            *
\* ========================================================================= **/
static IcoRec * loadIco( unsigned char * icoData )

{ unsigned char w, h;
  word dummy, tipo, items, colors;
  dword offset;
  IcoRec * icon;

 // unsigned char * subData;
  unsigned char * datos= icoData;

  getword( dummy, datos );  /* Leer dummy      */
  getword(  tipo, datos );  /* Leer tipo       */
  getword( items, datos );  /* Number of items */

  while( items -- )
  { dword imgsize;
    word  hotx;
    word  hoty;
    unsigned char palette;

    getbyte(       w, datos );  /* width        */
    getbyte(       h, datos );  /* Height       */
    getbyte( palette, datos );  /* Color planes */
    datos ++;                   /* Reserved     */
    getword(    hotx, datos );     /* Image size in bytes  */
    getword(    hoty, datos );     /* Image size in bytes  */
    getlong( imgsize, datos );  /* Image size in bytes  */
    getlong(  offset, datos );   /* Icon offset  */

    { int loop;

      int biSize     , biWidth, biHeight,  biPlanes, biBitCount, biCompress
        , biSizeImage, biXPels,  biYPels, biClrUsed, biClrImpr;

      dword * pic;                        /* Holds picture                    */
      unsigned char  * subData= icoData + offset;  /* Point to the particular subimage */

      getlong(      biSize, subData );
      getlong(     biWidth, subData );
      getlong(    biHeight, subData );
      getword(    biPlanes, subData );
      getword(  biBitCount, subData );
      getlong(  biCompress, subData );
      getlong( biSizeImage, subData );
      getlong(     biXPels, subData );
      getlong(     biYPels, subData );
      getlong(   biClrUsed, subData );
      getlong(   biClrImpr, subData );

      colors= 1 << biBitCount;
      biHeight /= 2;  /* Includes transparency mask */

      icon= ( IcoRec * ) calloc( sizeof( IcoRec )                 /* Static size       */
                               + colors  * sizeof( icon->pal ) /* Holds colormap    */
                               + biWidth * biHeight                /* Holds image data  */
                               , 1 );
      icon->pal= (ImgPalette *)(icon+1);    /* points to free space */

      if ( biHeight < 0 )
      { biHeight= -biHeight;  /* Top-Down icon */
      }

      icon->nCol= 255;
      icon->wNat= biWidth;
      icon->hNat= biHeight;
      icon->frm= NULL;
      pic= icon->pic= (dword *)( &icon->pal[colors+1]  );    /* Points to image data */

      for( loop= 0
         ; loop<colors
         ; loop ++ )
      { getbyte( icon->pal[ loop ].blue , subData );
        getbyte( icon->pal[ loop ].green, subData );
        getbyte( icon->pal[ loop ].red  , subData );
        subData ++ ;
      }

      icon->pal[ loop ].alpha= 0xCF;

      switch ( biBitCount )              // Depende de los planos de color
      { case  1: GetBitMap( (unsigned char*)pic, subData, biWidth, biHeight, 0xFF ); break; /* Un plano de color               */
        case  2: GetQibMap( (unsigned char*)pic, subData, biWidth, biHeight, loop ); break; /* Dos planos de color  (jun 2007) */
        case  4: GetPixMap( (unsigned char*)pic, subData, biWidth, biHeight, loop ); break; /* Cuatro planos de color          */
        case  8: GetHicMap( (unsigned char*)pic, subData, biWidth, biHeight, loop ); break; /* ocho planos de color            */

        default: free( icon ); return( NULL );
  } } }

  return( icon );        /* Retornar los elementos cargados */
}

/** ====================================================[ JACS 1997-11-11 ]== *\
 *                                                                            *
 *   JACS 2011                                                                *
 *                                                                            *
 *  FUNCTION: LoadICOFile                                                     *
 *                                                                            *
 *  @brief Loads an icon from disk                                            *
 *                                                                            *
\* ========================================================================= **/
ANSIC IcoRec * loadIcoIcoFile( const char * bname, int wtarget, int htarget )
{ unsigned char datos[ 10000 ];

  int handler= open( bname, 0 );
  if ( handler < 0 )
  { return(NULL);               /* Not present */
  }

  read( handler           /* Leerlo completamente en memoria */
      , datos
      , sizeof( datos ));
  close( handler );            /* Not needed yet */

  return( loadIco( datos ));  /* Retornar el icono encontrado */
}


/** ====================================================[ JACS 1997-11-11 ]== *\
 *                                                                            *
 *   JACS 2011                                                                *
 *                                                                            *
 *  FUNCTION: loadImgIcoFile                                                  *
 *                                                                            *
 *  @brief Loads an icon from disk                                            *
 *                                                                            *
\* ========================================================================= **/
DeviceImageRec * loadImgIcoFile( const char * bname, int wtarget, int htarget ) 
{ return( NULL );
}






