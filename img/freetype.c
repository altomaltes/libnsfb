/**
 *     AUTHOR: Jose Angel Caso Sanchez, 2017   ( altomaltes@yahoo.es )
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
 *     FILE: freetype.c
 *     DATE: ene 2017
 *
 *  DESCRIPCION: font support
 *
 */


#include <freetype2/ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

#include "../libnsfb.h"
#include "../surface.h"
#include "../plot.h"

#define byte  unsigned char
#define dword unsigned int

typedef struct
{ int          fontIdx;
  const char * fontName;
  FT_Face      face;
  int          pitch;
} TypesRec;


#ifdef _WIN32

  #define NSFB_FONTCOURIER_FILE   "couri.ttf"
  #define NSFB_FONTLUCIDA_FILE    "lucon.ttf"
  #define NSFB_FONTANDALE_FILE    "MobaConsole2.ttf"
  #define NSFB_FONTTERMINAL_8x15  "vga850.fon"
  #define NSFB_FONTTERMINAL_10x20 "..\\SYSTEM32\\10x20.pcf"

#else

  #define NSFB_FONTCOURIER_FILE   "TTF/LiberationMono-Bold.ttf"
  #define NSFB_FONTLUCIDA_FILE    "TTF/LiberationMono-Bold.ttf"
  #define NSFB_FONTANDALE_FILE    "TTF/LiberationMono-Bold.ttf"
//  #define NSFB_FONTTERMINAL_FILE "misc/8x16.pcf.gz"
  #define NSFB_FONTTERMINAL_8x15  "75dpi/vga850.fon"
  #define NSFB_FONTTERMINAL_10x20 "misc/10x20.pcf.gz"


#endif



static TypesRec types[]=
{{ .fontIdx= NSFB_FONT_COURIER , .fontName= NSFB_FONTCOURIER_FILE  , .face= NULL, .pitch= 0 }
,{ .fontIdx= NSFB_FONT_LUCIDA  , .fontName= NSFB_FONTLUCIDA_FILE   , .face= NULL, .pitch= 0 }
,{ .fontIdx= NSFB_FONT_ANDALE  , .fontName= NSFB_FONTANDALE_FILE   , .face= NULL, .pitch= 0 }
,{ .fontIdx= NSFB_FONT_TERMINAL, .fontName= NSFB_FONTTERMINAL_8x15 , .face= NULL, .pitch= 0x08000F }
,{ .fontIdx= NSFB_FONT_TERMINAL, .fontName= NSFB_FONTTERMINAL_10x20, .face= NULL, .pitch= 0x0A0014 }
,{ .fontIdx=               0, .fontName= NULL                 , .face= NULL, .pitch= 0 }};


typedef struct FontColorRecSt
{ struct FontColorRecSt * next;

  FT_Face face;

  dword fore;
  dword back;

/* Precalculated value
 */

  int w; int h; int s;
  dword fr; dword fg; dword fb;
  dword br; dword bg; dword bb;

  dword * chars[ 256 ];   /* char bitmaps */

} FontColorRec;

typedef struct FontListRecSt
{ struct FontListRecSt * next;

  TypesRec           * font;   /* Static properties  */
  int                  pitch;
  FontColorRec       * list;

} FontListRec;

  static FT_Library ftLib= NULL;

/** ================================================= [ JACS, 10/01/2012 ] == *\
 *                                                                            *
 *   JASC 2012                                                                *
 *                                                                            *
 *  FUNCTION getFreeFont                                                      *
 *                                                                            *
 *  @brief gets an styled font renderer                                       *
 *                                                                            *
\* ========================================================================= **/
FontListRec * getFreeFont( int fontIdx
                         , int w, int h )
{ static FontListRec * seed= NULL;

  TypesRec * type;
  dword pitch=  h & 0xFFFF | ( w << 16 );

  if ( !ftLib )
  { if ( FT_Init_FreeType( &ftLib ) )
    { return( NULL );
    }
    {// stroker= NULL;
  } }

  for( type= types
     ; type->fontName
     ; type ++ )
  { if ( type->fontIdx == fontIdx )
    { FontListRec * ptr;

      if ( type->pitch )
      { if ( type->pitch != pitch )
        { continue;
      } }

      if ( !type->face )
      { char path[ 256 ];

        #ifdef _WIN32
          strcpy( path, getenv( "SystemRoot" ));
          strcat( path, "\\fonts\\" );
        #else
          strcpy( path, "/usr/share/fonts/" );
        #endif

        strcat( path, type->fontName );

        if (( FT_New_Face( ftLib, path, 0, &type->face )))
        { fprintf( stderr, "Cant find %s\n", path );
          return( NULL );
      } }

      for( ptr= seed
         ; ptr
         ; ptr= (FontListRec*)ptr->next )
      { if ( ptr->font->fontIdx == fontIdx )
        { if ( ptr->pitch == pitch )
          { return( ptr );               /* found */
      } } }

/* Create and link list
 */
      ptr= (FontListRec*) calloc( 10, sizeof(*ptr));
      ptr->next= (struct FontListRec*)seed;
      seed= ptr;

      ptr->pitch = pitch;
      ptr->font  = type;

      if (( FT_Set_Pixel_Sizes( ptr->font->face
                              , ( 5*w ) / 3
                              , ( 7*h ) / 6 )))
      { //return( NULL );
      }

      return( ptr );
  } }

  return( NULL );
}

/** ================================================= [ JACS, 10/01/2012 ] == *\
 *                                                                            *
 *   JASC 2012                                                                *
 *                                                                            *
 *  FUNCTION getFreeFont                                                      *
 *                                                                            *
 *  @brief gets an stored colored version of the render                       *
 *                                                                            *
\* ========================================================================= **/
FontColorRec * getFreeRender( FontListRec * seed
                            , dword fore, dword back /*, dword outline */ )
{ if ( !seed )
  { return( NULL );
  }

#ifdef _WIN32123

  fore= (( fore & 0x000000FF ) << 16 )
      | (( fore & 0x0000FF00 )      )
      | (( fore & 0x00FF0000 ) >> 16 );

  back= (( back & 0x000000FF ) << 16 )
      | (( back & 0x0000FF00 )      )
      | (( back & 0x00FF0000 ) >> 16 );

#endif


  FontColorRec * item= seed->list;

  while( item )
  { if ( item->fore == fore )
    { if ( item->back == back )
      { return( item );          /* Yet creaetd */
    } }

    item= (FontColorRec *)item->next;
  }


/*  Not found, create one of them
 */
  item= (FontColorRec *) calloc( sizeof( *item ), 1 );

  if ( item )
  { item->face= seed->font->face;
    item->fore= fore;
    item->back= back;
    item->h= seed->pitch &  0xFFFF;

    if (( item->w= seed->pitch >> 16 )); else
    { item->w= item->h;
    }

    item->s= item->w * item->h;

    item->fr= ( fore >>  0 ) & 0xFF;
    item->fg= ( fore >>  8 ) & 0xFF;
    item->fb= ( fore >> 16 ) & 0xFF;

    item->br= ( back >>  0 ) & 0xFF;
    item->bg= ( back >>  8 ) & 0xFF;
    item->bb= ( back >> 16 ) & 0xFF;

    item->next= ( FontColorRec *)seed->list;
    seed->list= item;
  }

  return( item );
}


/** ================================================= [ JACS, 10/05/2014 ] == *\
 *                                                                            *
 *   JASC 2014                                                                *
 *                                                                            *
 *  METHOD nsfbGetChar                                                        *
 *                                                                            *
 *  @brief returns a chas as a 32 bit aligned bitmap                          *
 *                                                                            *
\* ========================================================================= **/
dword * nsfbGetChar( FontColorRec * item, byte idx )
{ if ( item )
  { dword  * bmap= item->chars[ idx ];

    if ( ! bmap )     /* Not loaded, load it */
    { FT_Glyph glyph;
      int sz, x, y;
      byte   * row;
      FT_BitmapGlyph bitmapg;

      if ((  FT_Load_Char( item->face, idx, FT_LOAD_RENDER )))
      { return( NULL );
      }

      if (( FT_Get_Glyph( item->face->glyph, &glyph )))
      { return( NULL );
      }

      if ( FT_Glyph_To_Bitmap( &glyph
                             , FT_RENDER_MODE_NORMAL, NULL, 1 ))
      { FT_Done_Glyph( glyph );
        return( NULL );
      }
      bitmapg= (FT_BitmapGlyph)glyph;


/* Get and clear a work area
 */
      sz= item->s;

      if ( bmap= item->chars[ idx ]= (dword*)calloc( sz, sizeof( dword ) * 2 ) )  // Two extra pointers
      { while( sz-- )
        { bmap[ sz ]= item->back;           /* Clear background */
        }

        bmap +=            bitmapg->left;
        bmap += ( item->h- bitmapg->top -2 ) * item->w;
        row= bitmapg->bitmap.buffer;

        y= bitmapg->bitmap.rows;

        while( y-- )
        { for( x= 0
             ; x < bitmapg->bitmap.width
             ; x++ )
          { dword color,alpha;

            if ( bitmapg->bitmap.num_grays > 1 )
            { dword color= row[ x ];
              alpha= 255-color;

              dword r= item->fr * color + item->br * alpha; r >>= 8; r &= 0x0000FF;
              dword g= item->fg * color + item->bg * alpha;          g &= 0x00FF00;
              dword b= item->fb * color + item->bb * alpha; b <<= 8; b &= 0xFF0000;

              bmap[ x ]= r | g | b;
            }
            else
            { bmap[ x ]= ( row[ x >> 0x3 ] & ( 128 >> (x & 0x7 )))
                       ? item->fore : item->back;
          } }
          row  += bitmapg->bitmap.pitch;
          bmap += item->w;
      } }

    FT_Done_Glyph( glyph );
    }
    return( item->chars[ idx ] );
  }
  return( NULL );
}


/** ================================================= [ JACS, 10/05/2014 ] == *\
 *                                                                            *
 *   JASC 2014                                                                *
 *                                                                            *
 *  FUNCTION nsfbFillString                                                     *
 *                                                                            *
 *  @brief                                                                    *
 *                                                                            *
\* ========================================================================= **/
#define MAX_CHARS 256

typedef struct RenderListSt
{ struct RendeListSt * next;

  FontColorRec * render;
  ImageMap charImages[ MAX_CHARS ];
} RenderList;

ANSIC int nsfbFillString( Nsfb        * nsfb
                        , FontListRec * font
                        , const int x, const int y      // Draws "chess" text
                        , const char * str
                        , NSFBCOLOUR fore
                        , NSFBCOLOUR back
                        , NSFBCOLOUR outline  )
{ int step= x;
  int ok= 0;

  FontColorRec * render= getFreeRender( font
                                      , fore
                                      , back     /* , outline  */);
  if ( render )   /* Able to get a new or recicled rendor */
  { RenderList * next;

    for( next= nsfb->surfaceRtns->renderList /* Search for an old one */
       ; next
       ; next= next->next )
    { if ( next->render == render )          /* Found !! */
      { break;
    } }

    if ( !next )                                    /* A new one must be registered */
    { next= nsfb->surfaceRtns->renderList;  /* Old one */
      nsfb->surfaceRtns->renderList= (RenderList*)calloc( sizeof( *next), 1 ); /* Create a new one */
      nsfb->surfaceRtns->renderList->next= next;      /* linkit */
      nsfb->surfaceRtns->renderList->render= render;  /* assign hardware font */
      next= nsfb->surfaceRtns->renderList;            /* Use from now */
    }

    if ( next )
    { while( *str )
      { if ( !next->charImages[ *str ].image )     /* Not previosly registered */
        { nsfb->surfaceRtns->pixmap( nsfb->surfaceRtns
                                   , next->charImages + *str
                                   , nsfbGetChar( render, *str )
                                   , NULL
                                   , render->w, render->h );
        }

        if ( next->charImages[ *str ].image )  /* Created or found image */
        { nsfb->plotterFns->pixmapFill( nsfb
                                      , next->charImages + *str
                                      , step, y
                                      , 0,0, NOCOLOR );
          ok++;
        }
        step += render->w; str++;
  } } }

  return( 0 );
}



