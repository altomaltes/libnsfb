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


#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

#include "../libnsfb.h"

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


typedef struct
{ struct FontColorRec * next;

  FT_Face face;

  dword fore;
  dword back;

/* Precalculated value
 */

  int w; int h; int s;
  dword fr; dword fg; dword fb;
  dword br; dword bg; dword bb;

  byte * chars[ 256 ];   /* Chars */

} FontColorRec;

typedef struct
{ struct FontListRec * next;

  TypesRec           * font;   /* Static properties  */
  int                  pitch;
  FontColorRec       * list;

} FontListRec;

//  static FT_Stroker stroker;
  static FT_Library ftLib= NULL;

/** ================================================= [ JACS, 10/01/2012 ] == *\
 *                                                                            *
 *   JASC 2012                                                                *
 *                                                                            *
 *  METHOD Win32::fillString                                                  *
 *         Win32::drawString                                                  *
 *                                                                            *
 *  @brief draws a backgrounded string                                        *
 *                                                                            *
\* ========================================================================= **/
void * getFreeFont( int fontIdx
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
      ptr= (FontListRec*) calloc( 1, sizeof(*ptr));
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

void * getFreeRender( void * seed0
                    , dword fore, dword back /*, dword outline */ )
{ FontListRec * seed= (FontListRec *)seed0;

  if ( !seed )
  { return( NULL );
  }

  FontColorRec * item= seed->list;

  while( item )
  { if ( item->fore == fore )
    { if ( item->back == back )
      { return( item );          /* Yet creaetd */
    } }

    item= item->next;
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

    item->next= seed->list;
    seed->list= item;
  }
  return( item );
}


void ** getChar( void * item0, byte idx )
{ FT_Glyph glyph;
  int sz, x, y;
  byte  * row;
  void  ** alloc;
  FT_BitmapGlyph bitmapg;

  FontColorRec * item= (FontColorRec *) item0;

  if ( !item )
  { return( NULL );
  }

  dword * bmap=  item->chars[ idx ];

  if ( bmap )
  { return( bmap );
  }

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

  if ( alloc= (void**)calloc(( sz * sizeof( dword ))
                            +(  2 * sizeof( void* )), 50 ))                 // Two extra pointers
  { bmap= (dword*)(alloc+2);

    alloc[ 1 ]= bmap; // Enroll

    while( sz-- )
    { bmap[ sz ]= item->back;
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
  return( item->chars[ idx ]= alloc );
}

