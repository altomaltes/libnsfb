/*
 * Copyright 2009 Vincent Sanders <vince@simtec.co.uk>
 *
 * This file is part of libnsfb, http://www.netsurf-browser.org/
 * Licenced under the MIT License,
 *                http://www.opensource.org/licenses/mit-license.php
 */

#include <stdio.h>
#include <stdlib.h>

#include "../nsfb.h"
#include "../nsfbPlot.h"
#include "../surface.h"
#include "../plot.h"

static int ramInitialise( Nsfb *nsfb )
{ size_t size = (nsfb->width * nsfb->height * nsfb->bpp) / 8;

  if ( size )
  { if ( nsfb->loc )
    { nsfb->loc= realloc( nsfb->loc, size);
    }
    else
    { nsfb->loc= CALLOC( size );
  } }
  else
  { FREE( nsfb->loc );
  }

  nsfb->panlen= (nsfb->width * nsfb->bpp) / 8;

  return 0;
}

static int ramSetGeometry( Nsfb * nsfb
                         , int width, int height
                         , enum NsfbFormat format )
{ int startsize;
  int endsize;
  format &= NSFB_FMT_MASK; // JACS

  startsize = (nsfb->width * nsfb->height * nsfb->bpp) / 8;

  if ( width  > 0 )  { nsfb->width = width;  }
  if ( height > 0 )  { nsfb->height= height;  }

  if (format != NSFB_FMT_ANY)
  { nsfb->format= format;
  }

  selectPlotters( nsfb );     /* select soft plotters appropriate for format */

  endsize = (nsfb->width * nsfb->height * nsfb->bpp) / 8;

  if ( startsize != endsize )
  { nsfb->loc= nsfb->loc ? CALLOC( endsize )
                         : realloc( nsfb->loc, endsize );
  }
  nsfb->panlen = (nsfb->width * nsfb->bpp) / 8;

  return( 0 );
}


static int ramFinalise(Nsfb *nsfb)
{ FREE( nsfb->loc );

  return 0;
}

NsfbSurfaceRtns ramRtns =
{ .type=  NSFB_SURFACE_RAM
, .name= "ram"

, .initialise= ramInitialise
, .finalise  = ramFinalise
, .geometry  = ramSetGeometry
, .cursor    = NULL



, .dataSize  = sizeof( ramRtns )
};

NSFB_SURFACE_DEF( ramRtns )

/*
 * Local variables:
 *  c-basic-offset: 4
 *  tab-width: 8
 * End:
 */
