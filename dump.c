/*
 * Copyright 2009 Vincent Sanders <vince@simtec.co.uk>
 *
 * This file is part of libnsfb, http://www.netsurf-browser.org/
 * Licenced under the MIT License,
 *                http://www.opensource.org/licenses/mit-license.php
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "nsfb.h"
#include "nsfbPlot.h"
#include "libnsfb_plot_util.h"
#include "nsfb.h"
#include "surface.h"
#include "img/images.h"

/* exported interface documented in libnsfb.h
 */
ANSIC bool nsfbDump(Nsfb *nsfb, int fd)
{ FILE *outf;
  int x, y;

  outf = fdopen(dup(fd), "w");
  if (!outf )
  { return false;
  }

  fprintf( outf
         , "P3\n#libnsfb buffer dump\n%d %d\n255\n"
         , nsfb->width, nsfb->height );


/* loc for actual, pan for buffered
 */

  for (y=0; y < nsfb->height; y++)
  { for (x=0; x < nsfb->width; x++)
    { fprintf( outf,"%d %d %d "
             , *(nsfb->loc + (((nsfb->width * y) + x) * 4) + 2)
             , *(nsfb->loc + (((nsfb->width * y) + x) * 4) + 1)
             , *(nsfb->loc + (((nsfb->width * y) + x) * 4) + 0));
   }
	   fprintf(outf,"\n");
  }

  fclose(outf);

  return true;
}

/*
 *
ImgPalette * find( NSFBCOLOUR hue )
{ static HuePalette * first;

  HuePalette * ptr;

  for( ptr= first
     ; ptr
     ; ptr= ptr->next )
  { if ( ptr->hue == hue )
    { return( ptr->palette );
  } }

  ptr= new HuePalette( hue );


  return( ptr->palette );
}
 */

/* -------- */

/*
 * Local variables:
 *  c-basic-offset: 4
 *  tab-width: 8
 * End:
 */
