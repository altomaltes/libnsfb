/*
 * Copyright 2009 Vincent Sanders <vince@simtec.co.uk>
 *
 * This file is part of libnsfb, http://www.netsurf-browser.org/
 * Licenced under the MIT License,
 *                http://www.opensource.org/licenses/mit-license.php
 */

#include <stdlib.h>
#include <string.h>

#include "libnsfb_plot.h"
#include "cursor.h"
#include "palette.h"
#include "surface.h"


PUBLIC nsfb_t * nsfbNew( const enum nsfb_type_e surface_type )
{ nsfb_t * newfb;
  newfb = calloc(1, sizeof(nsfb_t));

  if ( newfb )
  { newfb->surface_rtns= nsfbSurfaceGetRtns( surface_type );   /* obtain surface routines */

    if ( newfb->surface_rtns )
    { newfb->surface_rtns->defaults(newfb);
      return( newfb );
    }
    free( newfb );
  }

  return( NULL );
}


/*
 * altomaltes, config friendly initializer
 *
 *  example
 *
 * 1280x1024x32#N@egl,drm,fb
 *
 *   Try resolution of 1280x24 32 bit depth north orientation on driver list eql, drm, fb
 * fists one sucessfull
 */

nsfb_t * nsfbOpen( const char * mode )
{ nsfb_t * newfb= calloc(1, sizeof(nsfb_t));

  if ( newfb )
  { newfb->width=
    newfb->height= -1;
    newfb->bpp= 32;  /* bpp has default value */
    newfb->rotate= NSFB_ROTATE_NORTH;
    char * mk;

    char * driver= (char *)alloca( strlen(mode)+1); strcpy( driver, mode ); /* variabilize string */

    if (( mk= strchr( driver, '#' )))     /* Resolution specified */
    { *mk++= 0;                           /* Terminate driver     */
      mode= "*";

       sscanf( mk, "%dx%dx%d"
             , &newfb->width, &newfb->height, &newfb->bpp );
    }
    else
    { mk= driver;
    }

    if (( mk= strchr( mk, '.' )))
    { *mk++= 0; switch( *mk )     /* Resolution specified */
      { case 'N': case 'n': newfb->rotate= NSFB_ROTATE_NORTH; break;
        case 'S': case 's': newfb->rotate= NSFB_ROTATE_SOUTH; break;
        case 'E': case 'e': newfb->rotate= NSFB_ROTATE_EAST;  break;
        case 'W': case 'w': newfb->rotate= NSFB_ROTATE_WEST;  break;
    } }

    mk= driver; while( *driver )
    { while( *mk != ',' && *mk ) { mk++; }  /* tap next item */
      *mk++= 0;                                 /* Close item    */

      enum nsfb_type_e type= nsfbTypeFromName( driver ); driver= mk;

      if ( type != NSFB_SURFACE_NONE )
      { newfb->surface_rtns;   /* obtain surface routines */

        if (( newfb->surface_rtns= nsfbSurfaceGetRtns( type )))
        { //newfb->surface_rtns->defaults( newfb );
          if ( newfb->surface_rtns->initialise( newfb ) >= 0 )
          { return( newfb );
    } } } }

    free( newfb );
  }

  return( NULL );
}

/* exported interface documented in libnsfb.h
 */
PUBLIC int nsfbInit( nsfb_t * nsfb )
{ int code= nsfb->surface_rtns->initialise( nsfb );
  nsfb->rotate= NSFB_ROTATE_NORTH;

  return( code );
}

/* exported interface documented in libnsfb.h */
PUBLIC int nsfbFree( nsfb_t * nsfb )
{ int ret;

  if ( nsfb->palette     )   nsfb_palette_free( nsfb->palette     );
  if ( nsfb->plotter_fns )                free( nsfb->plotter_fns );
  if ( nsfb->cursor      ) nsfbCursor_destroy( nsfb->cursor      );

  ret= nsfb->surface_rtns->finalise( nsfb );

  free( nsfb->surface_rtns );
  free( nsfb );

  return ret;
}

/* exported interface documented in libnsfb.h
 */
PUBLIC bool nsfb_event( nsfb_t *nsfb
                      , nsfb_event_t *event
                      , int timeout )
{ return nsfb->surface_rtns->input(nsfb, event, timeout);
}

/* exported interface documented in libnsfb.h
 */
PUBLIC int nsfbClaim(nsfb_t *nsfb, nsfb_bbox_t *box)
{ return nsfb->surface_rtns->claim(nsfb, box);
}

/* exported interface documented in libnsfb.h
 */
PUBLIC int nsfb_update( nsfb_t *nsfb, nsfb_bbox_t *box )
{ return nsfb->surface_rtns->update( nsfb, box );
}

/* exported interface documented in libnsfb.h
 */
PUBLIC int  nsfbSetGeometry( nsfb_t * nsfb
                           , int width, int height
                           , enum nsfb_format_e format )
{ int fmt= format & NSFB_FMT_MASK;

  if ( width  <= 0         ) { width = nsfb->width;  }
  if ( height <= 0         ) { height= nsfb->height; }
  if ( fmt == NSFB_FMT_ANY ) { format= nsfb->format; }

  return( nsfb->surface_rtns->geometry( nsfb, width, height, format ));
}

PUBLIC int  nsfb_set_pan( nsfb_t * nsfb, int type )
{ return( nsfb->surface_rtns->pan( nsfb, type ) );
}



/* exported interface documented in libnsfb.h
 */
PUBLIC int nsfb_set_parameters(nsfb_t *nsfb, const char *parameters)
{ if ( parameters )
  { if (*parameters )
    { if ( nsfb->parameters )
     { free(nsfb->parameters);
     }

     nsfb->parameters= strdup( parameters );

    return( nsfb->surface_rtns->parameters(nsfb, parameters ));
 } }

 return( -1 );
}

/* exported interface documented in libnsfb.h
*/
PUBLIC void * nsfbGetGeometry( nsfb_t * nsfb
                             , int    * width
                             , int    * height
                             , enum nsfb_format_e * format)

{ if ( width  ) { *width = nsfb->width;  }
  if ( height ) { *height= nsfb->height; }
  if ( format ) { *format= nsfb->format; }

  return( nsfb->loc );
}

/* exported interface documented in libnsfb.h
 */
PUBLIC int nsfb_get_buffer( nsfb_t * nsfb
                          , byte  ** ptr
                          , int    * linelen )
{ if (ptr)
  { *ptr = nsfb->loc;
  }

  if (linelen )
  { *linelen = nsfb->loclen;
  }

  return 0;
}


/*
 * Local variables:
 *  c-basic-offset: 4
 *  tab-width: 8
 * End:
 */

