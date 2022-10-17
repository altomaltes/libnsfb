/*
 * Copyright 2009 Vincent Sanders <vince@simtec.co.uk>
 * Copyright 2010 Michael Drake <tlsa@netsurf-browser.org>
 *
 * This file is part of libnsfb, http://www.netsurf-browser.org/
 * Licenced under the MIT License,
 *                http://www.opensource.org/licenses/mit-license.php
 */


#define PLOT_TYPE dword
#define PLOT_LINELEN(ll) ((ll) >> 2)



static inline void *get_xy_loc( nsfb_t *nsfb, int x, int y)
{ return( nsfb->loc + ( y * nsfb->loclen ) + (x << 2));
}

static inline void *get_xy_pan( nsfb_t *nsfb, int x, int y)
{ return( nsfb->pan + ( y * nsfb->panlen ) + (x << 2));
}


#include "common.c"


static bool xorFill( nsfb_t *nsfb, nsfb_bbox_t *rect, dword ent )
{ int w;
  dword *pvid;
  dword llen;
  dword width;
  dword height;


  width = rect->x1 - rect->x0;
  height= rect->y1 - rect->y0;
  llen  = (nsfb->loclen >> 2) - width;

  pvid = get_xy_loc( nsfb, rect->x0, rect->y0 );

  while (height-- > 0)
  { w = width;
    while (w > 0)
    { *pvid++ ^= ent; w--;
    }
    pvid += llen;
  }

  return true;
}

static bool fill( nsfb_t *nsfb
                , nsfb_bbox_t * ln
                , nsfb_colour_t c )
{ int   w;
  dword *pvid;
  dword ent;
  dword llen;
  dword width;
  dword height;
  nsfb_bbox_t rect= *ln;

  if ( !nsfbPlotclip_ctx( nsfb, &rect ) )
  { return( true ); /* fill lies outside current clipping region */
  }

  COLOR_TO_PIXEL( &ent, 0, c );

  if ( c & 0x80000000 )
  { return( xorFill( nsfb, &rect, ent ));
  }

  width = rect.x1 - rect.x0;
  height= rect.y1 - rect.y0;
  llen  = ( nsfb->loclen >> 2 ) - width;

  pvid= get_xy_loc( nsfb, rect.x0, rect.y0 );

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

    while (w > 0)
    { *pvid++ = ent; w--;
    }
    pvid += llen;
  }

  return true;
}

/*
 * Local Variables:
 * c-basic-offset:8
 * End:
 */
