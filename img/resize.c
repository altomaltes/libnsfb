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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *     FILE: bfind.c
 *     DATE: jul 1004
 *
 *  DESCRIPCION: Integer arithmetic image resizing algorithms
 *
 */

volatile static char * ident="Kannon";

#define ON_RESIZE_C

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../libnsfb_plot_util.h"
#include "../img/images.h"

#define dword unsigned int
#define  word unsigned short


int sign( w ) { return( w == 0 ? 0 : w  < 0 ? -1 : 1 ); }

struct ChRec;

typedef void ( *  LineProc )( struct ChRec *
                            , const  unsigned char  * src
                            , unsigned       ratio );  /* File/column crossing */

typedef void ( * ColumnProc )( struct ChRec *
                             , const   unsigned char * src
                             ,     unsigned * line
                             ,          int * idx );


typedef struct ChRec
{ LineProc liner;
  int   planes;
  ImgResizeProc given;
  void * userData;
  unsigned wd;
  unsigned hd;
  unsigned ws;
  unsigned hs;
  unsigned cf;     /* reduction coeficient */
  dword wspace[ ]; /* Adders array         */
} ChRec;



/** ================================================= [ JACS, 10/01/2012 ] == *\
 *                                                                            *
 *   JASC 2012                                                                *
 *                                                                            *
 *  FUNCTION keepLines                                                        *
 *                                                                            *
 *  @brief                                                                    *
 *                                                                            *
\* ========================================================================= **/
static void   keepLines(       ChRec * info
                , const  unsigned char * src
                , unsigned  ratio )  /* File/column crossing */
{ int ws, image, shift;
  ratio *= info->ws;
  dword * dst= info->wspace;

  for( ws= 0
     ; ws < info->wd
     ; ws ++ )
  { for( image= 0, shift= 0
       ; image < info->planes
       ; image ++, shift+= info->wd )
    { dst[ shift ] += ratio * src[ image ];
    }
    src+= info->planes; dst++;
} }

/** ================================================= [ JACS, 10/01/2012 ] == *\
 *                                                                            *
 *   JASC 2012                                                                *
 *                                                                            *
 *  FUNCTION enlargeLines                                                     *
 *                                                                            *
 *  @brief                                                                    *
 *      Source points, contiguous                                             *
 *  Destination lines, contiguous                                             *
 *                                                                            *
\* ========================================================================= **/
static void enlargeLines(       ChRec * info
                        , const  unsigned char * src
                        , unsigned  ratio )  /* File/column crossing */
{ word  x, image, shift;
  word  idx= 0;
  dword * dst= info->wspace;

  for( x= 0
     ; x < info->wd
     ; x ++, dst++ )
  { idx += info->ws;

    for( image= 0, shift= 0
       ; image < info->planes
       ; image ++, shift+= info->wd )
    { dst[ shift ] += info->ws * src[ image ] * ratio ;         /* Completly into */

      if ( idx > info->wd )
      { dst[ shift ] -= (idx - info->wd)* src[ image              ] * ratio ;  /* Left part       */
        dst[ shift ] += (idx - info->wd)* src[ image+info->planes ] * ratio ;  /* Right part      */
    } }
    if ( idx > info->wd )
    { idx-= info->wd;      /* Always positive   */
      src+= info->planes;  /* Next source point */
} } }

/** ================================================= [ JACS, 10/01/2012 ] == *\
 *                                                                            *
 *   JASC 2012                                                                *
 *                                                                            *
 *  FUNCTION reduceLines                                                      *
 *                                                                            *
 *  @brief                                                                    *
 *                                                                            *
\* ========================================================================= **/
static void reduceLines(        ChRec * info
                , const   unsigned char * src
                , unsigned  ratio )  /* File/column crossing */
{ unsigned x, shift, image;
  unsigned idx= 0;
  dword * dst= info->wspace;
  dword accu;


  for( x= 0
     ; x < info->ws
     ; x ++ )
  { idx += info->wd;

    for( image= 0, shift= 0
       ; image < info->planes
       ; image ++, shift+= info->wd, src ++ )
    { accu= src[ 0 ] * ratio;
      dst[ shift ] += info->wd * accu;  /* Completly into */

      if ( idx > info->ws )
      { dst[ shift     ] -= ( idx - info->ws ) * accu;       /* Left part       */
        dst[ shift + 1 ] += ( idx - info->ws ) * accu;       /* Right part      */
    } }

    if ( idx >= info->ws )
    { idx-= info->ws;     /* Always positive        */
      dst++;              /* Next destination point */
} } }

static void clearAccu( ChRec * info, unsigned * line )
{ memset( info->wspace
        , 0
        , info->wd * info->planes * sizeof( dword )); /* Clear accus     */
  (*line)++;                                          /* Next line       */
}

/** ================================================= [ JACS, 10/01/2012 ] == *\
 *                                                                            *
 *   JASC 2012                                                                *
 *                                                                            *
 *  FUNCTION keepColumn                                                       *
 *                                                                            *
 *  @brief  This is a changer for bitmaps ( a bit per pixel )                 *
 *                                                                            *
\* ========================================================================= **/
static void keepColumn(      ChRec * info
               , const unsigned char * src
               , unsigned   * line
               ,        int * idx )
{ info->liner( info             /* Completly into */
             , src
             , info->hs );

  info->given( info->userData
             , info->wspace     /* Let user take line */
             , info->wd
             , * line
             , info->cf );
  clearAccu( info, line );
}

/** ================================================= [ JACS, 10/01/2012 ] == *\
 *                                                                            *
 *   JASC 2012                                                                *
 *                                                                            *
 *  FUNCTION enlargeColumn                                                    *
 *                                                                            *
 *  @brief  This is a changer for bitmaps ( a bit per pixel )                 *
 *                                                                            *
\* ========================================================================= **/
static void enlargeColumn(      ChRec * info
                  , const unsigned char * src
                  , unsigned   * line
                  ,      int   * idx )  // Index counter
{ if ( *idx > 0 )                   /* Previous line */
  { info->liner( info    /* Down part */
               , src
               , *idx );

    if ( info->given )
    { info->given( info->userData
                 , info->wspace       /* Let user take line */
                 , info->wd
                 , * line
                 , info->cf );
    }
    clearAccu( info, line );
  }


  while ( 1 )
  { idx[0] +=  info->hs;

    if ( idx[0] <= info->hd )
    { info->liner( info           /* Completly into */
                 , src
                 , info->hs );
    }
    else
    { idx[0] -= info->hd;                   /* Always positive */
      info->liner( info
                 , src
                 , info->hs - idx[ 0 ] );   /* Up part         */
      break;
    }

    if ( info->given )
    { info->given( info->userData
                 , info->wspace               /* Let user take line */
                 , info->wd
                 , * line
                 , info->cf );
    }
    clearAccu( info, line );
} }

/** ================================================= [ JACS, 10/01/2012 ] == *\
 *                                                                            *
 *   JASC 2012                                                                *
 *                                                                            *
 *  FUNCTION reduceColumn                                                     *
 *                                                                            *
 *  @brief  This is a changer for bitmaps ( a bit per pixel )                 *
 *                                                                            *
\* ========================================================================= **/
static void reduceColumn(        ChRec * info
                 , const   unsigned char * src
                 ,    unsigned  * line
                 ,         int  * idx )    /* Index counter */
{ idx[0] += info->hd;

  if ( idx[0] < info->hs )
  { info->liner( info              /* Up part  */
               , src
               , info->hd );
  }
  else
  { idx[0] -= info->hs;                 /* Always positive             */
    info->liner( info                   /* Up part   */
               , src
               , info->hd - idx[0] );
    info->given( info->userData
               , info->wspace           /* Let user take line */
               , info->wd
               , * line
               , info->cf );
    clearAccu( info, line );
    info->liner( info           /* Down part ( previous pass )  */
               , src
               , idx[0] );
} }


struct ChangerRecStruct
{ ColumnProc columns;
  int idx; int line;
  ChRec info;
};

/** ================================================= [ JACS, 10/01/2012 ] == *\
 *                                                                            *
 *   JASC 2012                                                                *
 *                                                                            *
 *  FUNCTION changerSize                                                      *
 *                                                                            *
 *  @brief                                                                    *
 *                                                                            *
\* ========================================================================= **/
int changerSize( int w, int channels )
{ return( sizeof( ChangerRec )
        + w * channels * ( sizeof( dword ) + 1 ));
}

/** ================================================= [ JACS, 10/01/2012 ] == *\
 *                                                                            *
 *   JASC 2012                                                                *
 *                                                                            *
 *  FUNCTION initChanger                                                      *
 *                                                                            *
 *  @brief                                                                    *
 *                                                                            *
\* ========================================================================= **/
ChangerRec * initChanger( ChangerRec * changer /* Process info        */
                        , int planes           /* Number of scanlines */
                        , ImgResizeProc given
                        , void * data
                        , int wDst, int hDst    /* Destination wide and height   */
                        , int wOrg, int hOrg )  /* Original wide  and height     */
{ if ( changer )
  { memset( changer
          , 0
          , changerSize( wDst , planes )); /* Clear all           */

    switch( sign( wDst - wOrg ) )
    { case -1: changer->info.liner= reduceLines;  break;
      case  1: changer->info.liner= enlargeLines; break;
      case  0: changer->info.liner= keepLines;    break;
    }

    switch( sign( hDst - hOrg ) )
    { case -1: changer->columns= reduceColumn;  break;
      case  1: changer->columns= enlargeColumn; break;
      case  0: changer->columns= keepColumn;    break;
    }

    changer->idx= 0;
    changer->line= 0;
    changer->info.wd    = wDst;
    changer->info.hd    = hDst;
    changer->info.ws    = wOrg;
    changer->info.hs    = hOrg;
    changer->info.cf    = wOrg;
    changer->info.cf   *= hOrg;
    changer->info.planes= planes;

    changer->info.given= given;
    changer->info.userData= data;
  }

  return( changer );
}


void * changerLine( ChangerRec * changer )           /* Process info               */
{ return( changer->info.wspace                       /* Pointer to line processing */
        + changer->info.wd * changer->info.planes ); /* Skip just a line           */
}


/*
 * This is a changer for bitmaps ( a bit per pixel )
 *
 *
static void recoef( void        * userData
                  , const dword * line
                  , unsigned      wide
                  , unsigned      row
                  , unsigned      coef )
{ byte * dst= (byte *) userData;
  dst+= row * wide;               // Point the correct line

  while( wide -- )
  { *dst ++ =  *line ++ / coef ;
  }
}
*/

/** ================================================= [ JACS, 10/01/2012 ] == *\
 *                                                                            *
 *   JASC 2012                                                                *
 *                                                                            *
 *  FUNCTION initChangerByte                                                  *
 *                                                                            *
 *  @brief  This is a changer for bitmaps ( a bit per pixel )                  *
 *                                                                            *
\* ========================================================================= **/
void * initChangerByte( ChangerRec * changer /* Process info       */
                      , int wDst             /* Destination wide   */
                      , int hDst             /* Destination height */
                      , int wOrg             /* Original wide      */
                      , int hOrg  )          /* Original height    */
{ //return( resetChanger( initChanger( changer
    //                               , 1
      //                             , wDst, hDst
        //                           , wOrg, hOrg )      /* Only a channel */
          //            , recoef
            //          , calloc( wDst, hDst ))); /* Standard image, almost always valid */
  return( NULL );
}


/** ================================================= [ JACS, 10/01/2012 ] == *\
 *                                                                            *
 *   JASC 2012                                                                *
 *                                                                            *
 *  FUNCTION giveBitmap                                                       *
 *                                                                            *
 *  @brief  This is a changer for bitmaps ( a bit per pixel )                 *
 *                                                                            *
\* ========================================================================= **/
void giveBitmap( void        * userData
               , const dword * line
               , unsigned      wide
               , unsigned      row
               , unsigned      coef )
{ unsigned char * dst= (unsigned char *) userData;
  dst += row *
#ifdef _WIN32
               QUANTIZE( wide , 4 ) * sizeof( word ); /* WIN32 Point to the correct line */
#else
               QUANTIZE( wide , 3 ) * sizeof( unsigned char ); /* X11   Point to the correct line */
#endif

  coef >>= 1;                 /* Adjust cut point                */

  int wp, words;

  for( wp= 0, dst --
     ; wp < wide
     ; wp ++ )
  { words= wp & 0x7;           /* Word counter                  */

    if ( !words )              /* Is a new bitmap word, zero it */
    { dst++; *dst= 0;
    }

    *dst |= (*line ++) < coef
#ifdef _WIN32
         ?  ( 0x80 >> words )  /* (  row > 20 ) ? 0x0F: 0xF0 */
#else
         ?  ( 0x01 << words )  /* (  row > 20 ) ? 0x0F: 0xF0 */
#endif
         : 0;                  /* The magic is here */
} }


void * getAlpha( unsigned char * mask
               , int w, int h, int coef
               , unsigned char * alpha )
{ if ( mask )
  { int hp; int wide=
#ifdef _WIN32
               QUANTIZE( w , 4 ) * sizeof( word          ); /* WIN32 Point to the correct line */
#else
               QUANTIZE( w , 3 ) * sizeof( unsigned char ); /* X11   Point to the correct line */
#endif

    for( hp= 0, alpha += 3                         /* Step vertically, point to alpha channel */
       ; hp < h
       ; hp ++ )
    { unsigned char * dst= mask + hp * wide;                /* WIN32 Point to the correct line */
      int words, wp;

      for( wp= 0, dst--                            /* Step horizontally */
         ; wp < w
         ; wp ++ )
      { words= wp & 0x7;                           /* Word counter                  */

        if ( !words )                              /* Is a new bitmap word, zero it */
        { dst++; *dst= 0;
        }

        *dst |= *alpha < coef
#ifdef _WIN32
         ?  ( 0x80 >> words )                      /* (  row > 20 ) ? 0x0F: 0xF0 */
#else
         ?  ( 0x01 << words )                      /* (  row > 20 ) ? 0x0F: 0xF0 */
#endif
             : 0;                                  /* The magic is here */
        alpha +=  4;                               /* Next alpha */
  } } }

  return( mask );
}


void * getCol32( dword * image
               , int w, int h
               , unsigned char * array )
{ unsigned char * dst= (unsigned char *)image;

  for( w *= h; w; w--)
  { *dst++ = *array++;
    *dst++ = *array++;
    *dst++ = *array++;
    *dst++ = 0; array++;
  }

 // printf( "offsize %d\n", org-image );

  return( image );
}

void * getCol16( word * image
               , int w, int h
               , unsigned char * array )
{ if ( image )
  { int hp, wp;
    word pix;

    for( hp= 0                                     /* Step vertically, point to alpha channel */
       ; hp < h
       ; hp ++ )
    { for( wp= 0                            /* Step horizontally */
         ; wp < w
         ; wp ++ )
      { pix  = ((*array++) >> 3)     ;
        pix |= ((*array++) >> 2) <<  5;
        pix |= ((*array++) >> 3) << 11;
        array++;                                   /* Skip alpha channel */
        image[ hp * w + wp ]= pix;   /* Reduce colors */
  } } }

  return( image );
}



/** ================================================= [ JACS, 10/01/2012 ] == *\
 *                                                                            *
 *   JASC 2012                                                                *
 *                                                                            *
 *  FUNCTION giveBitmap                                                       *
 *                                                                            *
 *  @brief                                                                    *
 *                                                                            *
\* ========================================================================= **/
void * initChangerBit( ChangerRec * changer /* Process info       */
                     , int wDst, int hDst   /* Destination size   */
                     , int wOrg, int hOrg ) /* Original size      */
{// return( resetChanger( initChanger( changer, 1  /* Only a channel */
   //                                , wDst, hDst
     //                              , wOrg, hOrg )
       //               , giveBitmap
         //             , calloc( hDst                     /* Destination space */
           //                   , QUANTIZE( wDst, 4 )
             //                 * sizeof( word ))));  /* WIN32 */
  return( NULL );
}

/** ================================================= [ JACS, 10/01/2004 ] == *\
 *                                                                            *
 *   JASC 2004                                                                *
 *                                                                            *
 *  FUNCTION changeImageSize                                                  *
 *                                                                            *
 *  @brief                                                                    *
 *                                                                            *
\* ========================================================================= **/
void * changeImageSize( ChangerRec * changer )
{ int loop;

  for ( loop= 0
      ; loop < changer->info.hs
      ; loop ++ )
  {// unsigned char * src= changerLine( changer );

//    changeImageAddLine( changer );
//    src += changer->info.ws
//         * changer->info.images;
  }

  return( changer->info.userData );
}

/** ================================================= [ JACS, 10/01/2004 ] == *\
 *                                                                            *
 *   JASC 2004                                                                *
 *                                                                            *
 *  FUNCTION initChangerSize                                                  *
 *                                                                            *
 *  @brief                                                                     *
 *                                                                            *
\* ========================================================================= **/
void * initChangerSize( int     * wDst     /* Destination wide   */
                      , int     * hDst     /* Destination height */
                      , int       wOrg     /* Original wide      */
                      , int       hOrg  )  /* Original height    */
{ if ( *wDst || *hDst )                       /* Se ha pedido un cambio de tama¤o */
  { if (! *hDst )                             /* Emular tama¤os */
    { *hDst= (hOrg * (*wDst)) / wOrg;
    }

    if (! *wDst )
    { *wDst= (wOrg * (*hDst)) / hOrg;
  } }
  else
  { *wDst= wOrg;
    *hDst= hOrg;
  }
  return( 0 );
}


/** ================================================= [ JACS, 10/01/2004 ] == *\
 *                                                                            *
 *   JASC 2004                                                                *
 *                                                                            *
 *  FUNCTION changeImageAddLine                                               *
 *                                                                            *
 *  @brief                                                                    *
 *                                                                            *
\* ========================================================================= **/
void changeImageAddLine( ChangerRec * changer )
{ changer->columns( &changer->info           /* Static data   */
                  , changerLine( changer )   /* Source data   */
                  , &changer->line           /* Current line  */
                  , &changer->idx );         /* Quantum       */
}

void changeImageAddRGB( ChangerRec * changer ) /* Line */
{ int sz= changer->info.ws;
  unsigned char * row= changerLine( changer  ); 
  unsigned char * dst= row; /* From RGB */
  
  row += sz * 3;     /* Larst RGB */
  dst += sz * 4;     /* Larst ALPHA */
  
  while( sz-- )
  { dst[3]= 0xFF;    // alpha
    dst[2]= row[2];  // r
    dst[1]= row[1];  // g
    dst[0]= row[0];  // b

    dst-= 4;
    row-= 3;
  }

  changeImageAddLine( changer  ); /* Line */
}


