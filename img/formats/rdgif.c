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


#include <stdio.h>
#include <stdlib.h>
#include <gif_lib.h>   
#include <malloc.h>
#include <string.h>

#include "nsfb.h"
#include "../images.h"



static int InterlacedOffset[] = { 0, 4, 2, 1 }; /* The way Interlaced image should. */
static int InterlacedJumps[]  = { 8, 8, 4, 2 }; /* be read - offsets and jumps... */

typedef struct imageList
{ struct imageList * next;    /* Simple linked list */
  word delay;                 /* Image delay        */
  unsigned char pixels[1];    /* Stored image       */
} imageList;


/** ====================================================[ JACS 1997-11-11 ]== *\
 *                                                                            *
 *   JACS 2011                                                                *
 *                                                                            *
 *  FUNCTION: loadImgFile                                                     *
 *                                                                            *
 *  @brief Loads a gif image from disk.                                       *
 *                                                                            *
\* ========================================================================= **/
ANSIC DeviceImageRec * loadImgFile( const char * fname, int wtarget, int htarget  )
{ GifRecordType   recordType;
  ColorMapObject *ColorMap;

  int i, j, ExtCode, Count;
  int Row, Col, width, Height /*, ColorMapSize */;
  GifByteType *Extension;
  int imageSize;
  int loadErr;

  word      * delay;            /* To build the delay list  */

  imageList * subImage;         /* Subimages working        */
  imageList * subImages= NULL;  /* Subimages mounting       */

  GifFileType * gifFile= DGifOpenFileName( fname, &loadErr );

  if ( !gifFile )
  { puts( fname );
   // PrintGifError();
    return( NULL );
  }


  int wOrg= gifFile->SWidth;
  int hOrg= gifFile->SHeight;

  int wDst= gifFile->SWidth;
  int hDst= gifFile->SHeight;

  int pics= 1;

  ChangerRec     * changerAlpha;         /* Mask can change size    */
  DeviceImageRec * image= initAlphaMap( changerAlpha= allocChanger( 4, wDst ) /* RGB color     */
                                      , pics                                  /* Planes        */
                                      , wDst, hDst                            /* Final size    */
                                      , wOrg, hOrg );                         /* Original size */

  imageSize= gifFile->SWidth * gifFile->SHeight;

  while( DGifGetRecordType( gifFile
                         , &recordType ) != GIF_ERROR )
  { switch( recordType )
    { case IMAGE_DESC_RECORD_TYPE:
        if ( DGifGetImageDesc( gifFile ) == GIF_ERROR)
        { free( image ); return( NULL );
        }

        ColorMap= gifFile->Image.ColorMap
                ? gifFile->Image.ColorMap
                : gifFile->SColorMap;

    
        Col    = gifFile->Image.Left;
        Height = gifFile->Image.Height;


        if ( gifFile->Image.Left + gifFile->Image.Width  > gifFile->SWidth
          || gifFile->Image.Top  + gifFile->Image.Height > gifFile->SHeight )
        { free( image ); return( NULL );  /* Image %d is not confined to screen dimension, aborted ( ImageNum ) */
        }

        if ( gifFile->Image.Interlace ) /* Need to perform 4 passes on the images: */
        { for ( Count = i = 0
              ;         i < 4
              ;         i ++ )
            for ( j = gifFile->Image.Top + InterlacedOffset[i]
                ; j < gifFile->Image.Top + Height
                ; j += InterlacedJumps[i])
          { char * line= (unsigned char *)alloca( width= gifFile->Image.Width ); /* Holds a line of data */

            if ( DGifGetLine( gifFile
                            //, line + j * gifFile->Image.Width + gifFile->Image.Left
                            , line + gifFile->Image.Left
                            , width ) == GIF_ERROR)
            { free( image ); return( NULL );
        } } }
        else 
        { for ( i = 0
              ; i < Height
              ; i++)
          { char * line= (unsigned char *)alloca( width= gifFile->Image.Width ); /* Holds a line of data */

            if ( DGifGetLine( gifFile
//                            , line + Row++ * gifFile->Image.Width + gifFile->Image.Left
                            , line + gifFile->Image.Left
                            , width ) == GIF_ERROR)
            { free( image ); return( NULL );
            } 


            unsigned char * linePtr= changerLine( changerAlpha  );

            while( width-- )
            { unsigned char pal= *line ++;

             *linePtr++= ColorMap->Colors[ pal ].Red;
             *linePtr++= ColorMap->Colors[ pal ].Green;
             *linePtr++= ColorMap->Colors[ pal ].Blue;
             *linePtr++= 0xFF; // ( palPtr[ pal ].alpha & 0x0F ) ? 0xFF : 0x00;
            }

            changeImageAddLine( changerAlpha  ); /* Line */
          } }

//        icon->pics += 0x100;             /* A new frame (Overloaded) */
      break;

      case EXTENSION_RECORD_TYPE:      /* Skip any extension blocks in file: */

        if ( DGifGetExtension( gifFile
                            , &ExtCode
                            , &Extension ) == GIF_ERROR )
        { free( image ); return( NULL );
        }

        switch( ExtCode )
        { case GRAPHICS_EXT_FUNC_CODE:

        //    if ( Extension[ 4 ]< ColorMap->ColorCount )  // Mark the transparent color
        //    { icon->pal[ Extension[ 4 ] ].alpha= 0x0F;
        //    }

            if ( Extension[ 1 ] & 0x1C )              /* New image     */
            { subImage= alloca( sizeof( imageList )   /* Image control */
                              +         imageSize  ); /* Image data    */

               subImage->next= subImages;             /* Link list             */
               subImages= subImage;

               subImage->delay = Extension[ 3 ];      /* Load word             */
               subImage->delay<<= 8;
               subImage->delay|= Extension[ 2 ];

               char * line= subImage->pixels;             /* Load next image here  */
            }
      break;

      case APPLICATION_EXT_FUNC_CODE:
      case COMMENT_EXT_FUNC_CODE:
      break;
    }

    while ( Extension )
    { if ( DGifGetExtensionNext( gifFile
                               , &Extension ) == GIF_ERROR)
      { free( image ); return( NULL );
    } }
    break;

    case TERMINATE_RECORD_TYPE:
 //     if ( icon->pics > 0x100 && 0 )      /* Mount subimages (OVERLOADED) */
 //     { //int images= icon->pics >> 8;      /* !!! */

//          icon= ( IcoRec * )realloc( icon
//                                   , sizeof( IcoRec )               /* Static size       */
//                                   + colorSize                      /* Holds colormap    */
//                                   + ( imageSize + sizeof( word )) * images * 2 );  /* Holds image data !!! */
//
//          picture= icon->pic= (unsigned char *)( &icon->pal[ ColorMap->ColorCount ] ); /* Points to image data */
//          delay= icon->frm= (word *)( picture + imageSize * images );          /* Points to delays     */

//          for( subImage= subImages
//             ; subImage
//             ; subImage= subImage->next )
//          { memcpy( picture
//                  , subImage->pixels
//                  , imageSize );
//            *delay= subImage->delay;
//            picture+= imageSize;  /* next image */
//            delay ++;
///        } }

 //       image->mask= GETALPHA( image->width, image->height, image->image );


        DGifCloseFile( gifFile, &loadErr );
      return( image );


      default:                /* Should be traps by DGifGetRecordType. */
      break;
  } }

  free( image ); return( NULL );
}




/** ====================================================[ JACS 1997-11-11 ]== *\
 *                                                                            *
 *   JACS 2011                                                                *
 *                                                                            *
 *  FUNCTION: loadIcoFile                                                     *
 *                                                                            *
 *  @brief Loads a gif image from disk.                                       *
 *                                                                            *
\* ========================================================================= **/
ANSIC IcoRec * loadIcoFile( const char * fname, int wtarget, int htarget  )
{ GifRecordType recordType;
  ColorMapObject *ColorMap;

  int i, j, ExtCode, Count;
  int Row, Col, Width, Height /*, ColorMapSize */;
  GifByteType *Extension;
  int imageSize;
  int colorSize;
  int loadErr;

  unsigned char * picture;
  ImgPalette * palette;

  word      * delay;            /* To build the delay list  */
  IcoRec    * icon= NULL;       /* Mark not loaded          */
  imageList * subImage;         /* Subimages working        */
  imageList * subImages= NULL;  /* Subimages mounting       */

  GifFileType * gifFile= DGifOpenFileName( fname, &loadErr );

  if ( !gifFile )
  { puts( fname );
   // PrintGifError();
    return( NULL );
  }

  ColorMap= gifFile->Image.ColorMap
          ? gifFile->Image.ColorMap
          : gifFile->SColorMap;

  imageSize= gifFile->SWidth * gifFile->SHeight;
  colorSize= ( ColorMap->ColorCount + 2 ) * sizeof( *palette );

  icon= ( IcoRec * ) calloc( sizeof( IcoRec ) /* Static size       */
                           + colorSize        /* Holds colormap    */
                           + imageSize        /* Holds image data  */
                           , 2  );
  icon->nCol= 255;                              /* One palette       */
//  icon->n0= 0;
  icon->wNat= gifFile->SWidth;
  icon->hNat= gifFile->SHeight;
  icon->frm= NULL;

  palette= icon->pal= (ImgPalette * )(icon+1); /* Skip fixed data */
  picture= icon->pic= (unsigned char *) ( palette + ColorMap->ColorCount ); /* Points to image data */

  for ( i = 0
      ; i < ColorMap->ColorCount
      ; i++ )
  { icon->pal[ i ].red=   ColorMap->Colors[i].Red;
    icon->pal[ i ].green= ColorMap->Colors[i].Green;
    icon->pal[ i ].blue=  ColorMap->Colors[i].Blue;
    icon->pal[ i ].alpha= 0;
  }
  icon->pal[ i-1 ].alpha |= 0x80;


  while( DGifGetRecordType( gifFile
                         , &recordType ) != GIF_ERROR )
  { switch( recordType )
    { case IMAGE_DESC_RECORD_TYPE:
        if ( DGifGetImageDesc( gifFile ) == GIF_ERROR)
        { return( NULL );
        }

        Row    = gifFile->Image.Top; /* Image Position relative to Screen. */
        Col    = gifFile->Image.Left;
        Width  = gifFile->Image.Width;
        Height = gifFile->Image.Height;


        if ( gifFile->Image.Left + gifFile->Image.Width  > gifFile->SWidth
          || gifFile->Image.Top  + gifFile->Image.Height > gifFile->SHeight )
        { return( NULL );  /* Image %d is not confined to screen dimension, aborted ( ImageNum ) */
        }

        if ( gifFile->Image.Interlace ) /* Need to perform 4 passes on the images: */
        { for ( Count = i = 0
              ; i < 4
              ; i++ )
            for ( j = Row + InterlacedOffset[i]
                ; j < Row + Height
                ; j += InterlacedJumps[i])
          { if ( DGifGetLine( gifFile
                            , picture + j * gifFile->Image.Width + Col
                            , Width ) == GIF_ERROR)
            { return( NULL );
        } } }
        else 
        { for ( i = 0
              ; i < Height
              ; i++)
          { if ( DGifGetLine( gifFile
                            , picture + Row++ * gifFile->Image.Width + Col
                            , Width) == GIF_ERROR)
            { return( NULL );
        } } }

        icon->pics += 0x1;             /* A new frame (Overloaded) */
      break;

      case EXTENSION_RECORD_TYPE:      /* Skip any extension blocks in file: */

        if ( DGifGetExtension( gifFile
                            , &ExtCode
                            , &Extension ) == GIF_ERROR )
        { return( NULL );
        }

        switch( ExtCode )
        { case GRAPHICS_EXT_FUNC_CODE:

            if ( Extension[ 4 ]< ColorMap->ColorCount )  // Mark the transparent color
            { icon->pal[ Extension[ 4 ] ].alpha= 0x0F;
            }

            if ( Extension[ 1 ] & 0x1C )              /* New image     */
            { subImage= alloca( sizeof( imageList )   /* Image control */
                              +         imageSize  ); /* Image data    */

               subImage->next= subImages;             /* Link list             */
               subImages= subImage;

               subImage->delay = Extension[ 3 ];      /* Load word             */
               subImage->delay<<= 8;
               subImage->delay|= Extension[ 2 ];

               picture= subImage->pixels;             /* Load next image here  */
            }
          break;

          case APPLICATION_EXT_FUNC_CODE:
          case COMMENT_EXT_FUNC_CODE:
          break;
        }

        while ( Extension )
        { if ( DGifGetExtensionNext( gifFile
                                  , &Extension ) == GIF_ERROR)
          { return( NULL );
        } }
      break;

      case TERMINATE_RECORD_TYPE:
        if ( icon->pics > 0x100 && 0 )      /* Mount subimages (OVERLOADED) */
        { int images= icon->pics >> 8;      /* !!! */

          icon= ( IcoRec * )realloc( icon
                                   , sizeof( IcoRec )               /* Static size       */
                                   + colorSize                      /* Holds colormap    */
                                   + ( imageSize + sizeof( word )) * images * 2 );  /* Holds image data !!! */

          picture= icon->pic= (unsigned char *)( &icon->pal[ ColorMap->ColorCount ] ); /* Points to image data */
          delay= icon->frm= (word *)( picture + imageSize * images );          /* Points to delays     */

          for( subImage= subImages
             ; subImage
             ; subImage= subImage->next )
          { memcpy( picture
                  , subImage->pixels
                  , imageSize );
            *delay= subImage->delay;
            picture+= imageSize;  /* next image */
            delay ++;
        } }

        DGifCloseFile( gifFile, &loadErr );
      return( icon );


      default:                /* Should be traps by DGifGetRecordType. */
      break;
  } }

  return( NULL );
}













