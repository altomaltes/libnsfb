/* libnsfb plotter test program */

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "../libnsfb_plot.h"

#include <freetype2/ft2build.h>
#include FT_FREETYPE_H

const struct
{ unsigned int w;
  unsigned int h;
  unsigned char data[16];
}
Mglyph1 =
{ 8, 40,
  { 0x00, /* 00000000 */
0x00, /* 00000000 */
0xc6, /* 11000110 */
0xee, /* 11101110 */
0xfe, /* 11111110 */
0xfe, /* 11111110 */
0xd6, /* 11010110 */
0xc6, /* 11000110 */
0xc6, /* 11000110 */
0xc6, /* 11000110 */
0xc6, /* 11000110 */
0xc6, /* 11000110 */
0x00, /* 00000000 */
0x00, /* 00000000 */
0x00, /* 00000000 */
0x00, /* 00000000 */
} };





FT_Face loadFont( const char * font, int w, int h )
{ static FT_Library library= NULL;  /* handle to library */
  FT_Face face= NULL;               /* handle to face object */

  if ( !library )
  { if ( FT_Init_FreeType( &library ) )
    { puts( "Fretype error" );
      return( NULL );
  } }

  switch( FT_New_Face( library
                     , font
                     , 0
                     , &face ))
  { case FT_Err_Unknown_File_Format:
      fprintf( stderr, "Font file %s not found\n"
                     , font );
    return( NULL );

    default:
      fprintf( stderr, "Format error on %s\n"
                     , font );
    return( NULL );

    case 0: break;
  }

//   error = FT_Set_Char_Size( face   /* handle to face object */
//                           , 0      /* char_width in 1/64th of points */
//                           , 16*64  /* char_height in 1/64th of points */
//                           , 300    /* horizontal device resolution */
//                           , 300 ); /* vertical device resolution */

  if ( FT_Set_Pixel_Sizes( face     /* handle to face object */
                          , w, h ))  /* pixel_width and  pixel_height */
  { return( NULL );
  }

  return( face );
}

int textOut( nsfb_t     * nsfb
           , FT_FaceRec * face
           , int x, int y
           , const unsigned char * str )
{ int wide= 0;
  FT_GlyphSlot slot= face->glyph;

  while( *str )
  { if ( !FT_Load_Char( face, *str++, FT_LOAD_RENDER ))  /* load glyph image into the slot (erase previous one) */
    { nsfb_bbox_t box;
      box.x1 = ( box.x0 = x - slot->bitmap_left + wide ) + slot->bitmap.width;
      box.y1 = ( box.y0 = y - slot->bitmap_top         ) + slot->bitmap.rows;
      nsfbPlotglyph8( nsfb
                      , &box
                      , slot->bitmap.buffer
                      , slot->bitmap.pitch, 0xff000000, 0x00000000 );
      wide+= slot->advance.x >> 6;
  } }

  return( wide );  /* increment pen position */
}



int main(int argc, char **argv)
{ const char *fename;
  enum nsfb_type_e fetype;
  nsfb_t *nsfb;

  nsfb_bbox_t box;
  byte *fbptr;
  int fbstride;
  int i;
  unsigned int x, y;

  fename="linux";


  FT_Face face= loadFont( "/usr/share/fonts/TTF/LiberationMono-Bold.ttf", 14, 0 );

  fetype = nsfbTypeFromName(fename);
  if (fetype == NSFB_SURFACE_NONE)
  { printf("Unable to convert \"%s\" to nsfb surface type\n",
				fename);
	   return EXIT_FAILURE;
  }

  nsfb = nsfbNew(fetype);
  if (nsfb == NULL)
  { printf("Unable to allocate \"%s\" nsfb surface\n", fename);
	   return EXIT_FAILURE;
  }

  if (nsfbInit(nsfb) == -1)
  { printf("Unable to initialise nsfb surface\n");
	   nsfbFree(nsfb);
	   return EXIT_FAILURE;
  }

	/* get the geometry of the whole screen */
  box.x0 = box.y0 = 0;
  nsfbGetGeometry(nsfb, &box.x1, &box.y1, NULL);

  nsfb_get_buffer(nsfb, &fbptr, &fbstride);
  nsfbClaim(nsfb, &box);

	/* Clear to white */
  nsfbPlotclg(nsfb, 0xffffffff);
  nsfb_update(nsfb, &box);

	/* test glyph plotting */
 // for (i = 0; i < 1000; i++)
  { for (y = 0; y < box.y1 - Mglyph1.h; y += Mglyph1.h )
    { for (x = 0; x < box.x1 - Mglyph1.w; x += Mglyph1.w )
      { x += textOut( nsfb, face
                    , x, y
                    , "Hola" );
        nsfb_update(nsfb, &box);

  } } }

  nsfb_update(nsfb, &box);
  nsfbFree(nsfb);

  return 0;
}
