/*
 * Copyright 2009 Vincent Sanders <vince@simtec.co.uk>
 * Copyright 2010 Michael Drake <tlsa@netsurf-browser.org>
 *
 * This file is part of libnsfb, http://www.netsurf-browser.org/
 * Licenced under the MIT License,
 *                http://www.opensource.org/licenses/mit-license.php
 */

#define PLOT_TYPE dword
#define PLOT_LINELEN(ll) ((ll) >> 2 )



static inline void *getXYloc( Nsfb *nsfb, int x, int y)
{ return( nsfb->loc + ( y * nsfb->loclen ) + (x << 2));
}

static inline void *getXYpan( Nsfb *nsfb, int x, int y)
{ return( nsfb->pan + ( y * nsfb->panlen ) + (x << 2));
}


#include "common.c"


static bool xorFill( Nsfb *nsfb, NsfbBbox *rect, dword ent )
{ int w;
  PLOT_TYPE *pvid;
  dword llen;
  dword width;
  dword height;


  width = rect->x1 - rect->x0;
  height= rect->y1 - rect->y0;
  llen  = (nsfb->loclen >> 2) - width;

  pvid = getXYloc( nsfb, rect->x0, rect->y0 );

  while (height-- > 0)
  { w = width;
    while (w > 0)
    { *pvid++ ^= ent; w--;
    }
    pvid += llen;
  }

  return true;
}

static bool fill( Nsfb     * nsfb
                , NsfbBbox * ln
                , NSFBCOLOUR c )
{ int   w;
  PLOT_TYPE *pvid;
  dword ent;
  dword llen;
  dword width;
  dword height;
  NsfbBbox rect= *ln;

  if ( !nsfbPlotClipCtx( nsfb, &rect ) )
  { return( true ); /* fill lies outside current clipping region */
  }

  COLOR_TO_PIXEL( &ent, 0, c );

  if ( c & 0x80000000 )
  { return( xorFill( nsfb, &rect, ent ));
  }

  width = rect.x1 - rect.x0;
  height= rect.y1 - rect.y0;
  llen  = PLOT_LINELEN( nsfb->loclen ) - width;

  pvid= getXYloc( nsfb, rect.x0, rect.y0 );

  PLOT_TYPE *last= (PLOT_TYPE *)nsfb->loc;

  while ( height-- > 0)
  { w = width;

   while (w >= 16)
    { *pvid++ = ent; *pvid++ = ent; *pvid++ = ent; *pvid++ = ent;
      *pvid++ = ent; *pvid++ = ent; *pvid++ = ent; *pvid++ = ent;
      *pvid++ = ent; *pvid++ = ent; *pvid++ = ent; *pvid++ = ent;
      *pvid++ = ent; *pvid++ = ent; *pvid++ = ent; *pvid++ = ent;
       w-=16;
    }

    while (w >= 4)
    { *pvid++ = ent; *pvid++ = ent; *pvid++ = ent; *pvid++ = ent;
       w-=4;
    }

    while ( w )
    { *pvid++ = ent; w--;
    }
    pvid += llen;
  }

  return true;
}

/*
 *
 */
static bool pixmapFill( Nsfb    * nsfb
                     , ImageMap * img
                     , int x, int y
                     , int offy, int capy
                     , NSFBCOLOUR back )
{ if ( img )
  { int overload= (int)img->mask;

    dword * image= img->image;

    int bmpWidth=  overload &  0xFFFF;    // Unproperly packed
    int bmpHeight= overload >> 16;

    if ( capy )                           // Only redraw a section
    { bmpHeight= capy;
    }

    NsfbBbox loc;

    loc.x0= x; loc.x1= loc.x0 + bmpWidth;
    loc.y0= y; loc.y1= loc.y0 + bmpHeight;

    image += offy * bmpWidth;            /* Aply offset on mosaic */

    return( bitmap( nsfb
                  , &loc
                  , image
                  , bmpWidth, bmpHeight, bmpWidth
                  , true ));
  }

  return( false );
}





/*
 * Local Variables:
 * c-basic-offset:8
 * End:
 */
