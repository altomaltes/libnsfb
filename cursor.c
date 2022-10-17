/*
 * Copyright 2010 Vincent Sanders <vince@kyllikki.org>
 *
 * This file is part of libnsfb, http://www.netsurf-browser.org/
 * Licenced under the MIT License,
 *                http://www.opensource.org/licenses/mit-license.php
 */

/** \file
 * cursor (implementation).
 */

// #define nsfb_s ç

#include <stdlib.h>

#include "libnsfb_plot.h"
#include "libnsfb_cursor.h"

#include "cursor.h"
#include "plot.h"
#include "surface.h"

bool nsfbCursorInit( nsfb_t * nsfb )
{ if ( !nsfb->cursor )
  { nsfb->cursor = calloc(1, sizeof(struct nsfbCursor_s));
    if ( nsfb->cursor )
    { nsfb->cursor->loc.x0= nsfb->width  / 2;
      nsfb->cursor->loc.y0= nsfb->height / 2;
      return true;
  } }

  return( false );
}

bool nsfbCursorSet( nsfb_t * nsfb
                    , const nsfb_colour_t * pixel
                    , int bmp_width, int bmp_height, int bmp_stride
                    , int hotspot_x, int hotspot_y )
{ if ( nsfb->cursor )
  { nsfb->cursor->pixel     = pixel;
    nsfb->cursor->bmp_width = bmp_width;
    nsfb->cursor->bmp_height= bmp_height;
    nsfb->cursor->bmp_stride= bmp_stride;
    nsfb->cursor->loc.x1    = nsfb->cursor->loc.x0 + nsfb->cursor->bmp_width;
    nsfb->cursor->loc.y1    = nsfb->cursor->loc.y0 + nsfb->cursor->bmp_height;

    nsfb->cursor->hotspot_x = hotspot_x;
    nsfb->cursor->hotspot_y = hotspot_y;

    return( nsfb->surface_rtns->cursor( nsfb, nsfb->cursor ));
  }

  return( false );
}

bool nsfbCursorLocSet( nsfb_t * nsfb
                     , int x, int y )
{ if ( nsfb )
  { struct nsfbCursor_s * cursor= nsfb->cursor;

    if ( cursor )
    {  // cursor->loc   = *loc;

      cursor->loc.x0= x - cursor->hotspot_x;
      cursor->loc.y0= y - cursor->hotspot_y;
//      cursor->loc.x1 -= cursor->hotspot_x;
//      cursor->loc.y1 -= cursor->hotspot_y;
      cursor->loc.x1= cursor->loc.x0 + cursor->bmp_width;
      cursor->loc.y1= cursor->loc.y0 + cursor->bmp_height;

      nsfb->surface_rtns->cursor( nsfb, cursor );

      cursor->savLoc   = cursor->loc;  // After draw
      cursor->savWidth = cursor->savLoc.x1 - cursor->savLoc.x0;
      cursor->savHeight= cursor->savLoc.y1 - cursor->savLoc.y0;
      return( true );
  } }

  return( false );
}

bool nsfbCursor_loc_get(nsfb_t *nsfb, nsfb_bbox_t *loc)
{ if (nsfb->cursor == NULL)       return false;

  *loc = nsfb->cursor->loc;
  return true;
}

/* documented in cursor.h
 */

bool nsfbCursorPlot( nsfb_t * nsfb )
{ int savSize;
  nsfb_bbox_t sclip;               /* saved clipping area */

  struct nsfbCursor_s * cursor= nsfb->cursor;

  if ( cursor )
  { nsfb->plotter_fns->get_clip( nsfb, &sclip );
    nsfb->plotter_fns->set_clip( nsfb, NULL   );

/* offset cursor rect for hotspot
 */
    cursor->loc.x0 -= cursor->hotspot_x;
    cursor->loc.y0 -= cursor->hotspot_y;
    cursor->loc.x1 -= cursor->hotspot_x;
    cursor->loc.y1 -= cursor->hotspot_y;

    cursor->savLoc    = cursor->loc;
    cursor->savWidth = cursor->savLoc.x1 - cursor->savLoc.x0;
    cursor->savHeight= cursor->savLoc.y1 - cursor->savLoc.y0;

    savSize = cursor->savWidth * cursor->savHeight * sizeof(nsfb_colour_t);

    if (cursor->savSize < savSize)
    { cursor->savCol = realloc( cursor->savCol, savSize );
      cursor->savSize= savSize;
    }

    nsfb->plotter_fns->readrect( nsfb, &cursor->savLoc, cursor->savCol );

    cursor->savWidth = cursor->savLoc.x1 - cursor->savLoc.x0;
    cursor->savHeight= cursor->savLoc.y1 - cursor->savLoc.y0;

    nsfb->plotter_fns->set_clip( nsfb, NULL );

    nsfb->plotter_fns->bitmap( nsfb
                             , &cursor->loc
                             ,  cursor->pixel
                             ,  cursor->bmp_width
                             ,  cursor->bmp_height, cursor->bmp_stride
                             , DO_ALPHA_BLEND );

/* undo hotspot offset
 */
    cursor->loc.x0 += cursor->hotspot_x;
    cursor->loc.y0 += cursor->hotspot_y;
    cursor->loc.x1 += cursor->hotspot_x;
    cursor->loc.y1 += cursor->hotspot_y;

    nsfb->plotter_fns->set_clip(nsfb, &sclip);
    cursor->plotted = true;

    return( true );
  }
  return( false );
}

bool nsfbCursorClear( nsfb_t * nsfb )
{ if ( nsfb) 
  { if ( nsfb->cursor ) 
    { if ( nsfb->plotter_fns )
      { nsfb_bbox_t sclip; /* saved clipping area */

        nsfb->plotter_fns->get_clip( nsfb, &sclip );
        nsfb->plotter_fns->set_clip( nsfb, NULL   );

//    printf( "cursor clear %d %d\n", nsfb->cursor->savWidth, nsfb->cursor->savHeight );

         nsfb->plotter_fns->bitmap( nsfb
                                  , &nsfb->cursor->savLoc
                                  , nsfb->cursor->savCol
                                  , nsfb->cursor->savWidth
                                  , nsfb->cursor->savHeight
                                  , nsfb->cursor->savWidth
                                  , 0 );

         nsfb->plotter_fns->set_clip( nsfb, &sclip );
         nsfb->cursor->plotted = false;
         return( true );
  } } }

  return( false );
}

bool nsfbCursorDestroy( struct nsfbCursor_s * cursor )
{ free( cursor->savCol ); 	/* Note: cursor->pixel isn't owned by us */
  free( cursor );

  return( true );
}
