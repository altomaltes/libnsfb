/*
 * Copyright 2009 Vincent Sanders <vince@simtec.co.uk>
 *
 * This file is part of libnsfb, http://www.netsurf-browser.org/
 * Licenced under the MIT License,
 *                http://www.opensource.org/licenses/mit-license.php
 */


#include <stdlib.h>
#include <string.h>

#include "nsfb.h"
#include "nsfbPlot.h"

#include "cursor.h"
#include "palette.h"
#include "surface.h"

#define PATH_SIZE 256


/*
 * Allocs a new surface
 */
static int voidClient( struct NomadEvent * evt, void * userData )
{ return( 0 );
}

ANSIC Nsfb * nsfbNew( const enum NsfbType surfaceType )
{ NsfbSurfaceRtns * ptr= nsfbFindSurface( surfaceType );

  if ( ptr )
  { Nsfb * newfb= CALLOC( ptr->dataSize*2 );

    if ( newfb )
    { newfb->next= ptr->clients; ptr->clients= newfb;  /* Link on list */

      nsfbSurfaceDefaultRtns( newfb->surfaceRtns= ptr );

      newfb->theGeo= newfb->surfaceRtns->theGeo;       /* Default rotation */
      newfb->toClient= voidClient;

      return( newfb );
  } }

  return( NULL );
}

/*
 * altomaltes, config friendly initializer
 */

Nsfb * nsfbOpenAscii( const char * fename )
{ enum NsfbType fetype= nsfbTypeFromName( fename );

  if ( fetype == NSFB_SURFACE_NONE )
  { fprintf(stderr, "Unable to convert \"%s\" to nsfb surface type\n", fename);
    return( NULL );
  }

  return( nsfbNewConsole( fetype, fename ));
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

/* exported interface documented in libnsfb.h
 */
ANSIC int nsfbInit( Nsfb * nsfb )
{ nsfb->surfaceRtns->panType= 1;   // Default copy pan

  return( nsfb->surfaceRtns->initialise( nsfb ));
}

/* exported interface documented in libnsfb.h
 */
ANSIC int nsfbFree( Nsfb * nsfb )
{ int ret;

  if ( nsfb->palette ) nsfbPaletteFree(   nsfb->palette );
 // !! if ( nsfb->cursor  ) nsfbCursorDestroy( nsfb->cursor  );

  ret= nsfb->surfaceRtns->finalise( nsfb );

  FREE( nsfb );

  return ret;
}

/* exported interface documented in libnsfb.h
 */
ANSIC int nsfbClaim(Nsfb *nsfb, NsfbBbox *box)
{ return nsfb->surfaceRtns->claim(nsfb, box);
}

/* exported interface documented in libnsfb.h
 */
ANSIC int nsfbUpdate( Nsfb *nsfb, NsfbBbox *box )
{ return nsfb->surfaceRtns->update( nsfb, box );
}

/* exported interface documented in libnsfb.h
 */
ANSIC int nsfbSetPosition( Nsfb * nsfb
                         , int x, int y, int geo )
{ switch( geo )     /* Resolution specified */
  { case 'S': case 's': nsfb->theGeo= NSFB_ROTATE_SOUTH; break;
    case 'E': case 'e': nsfb->theGeo= NSFB_ROTATE_EAST;  break;
    case 'W': case 'w': nsfb->theGeo= NSFB_ROTATE_WEST;  break;
    case 'N': case 'n': nsfb->theGeo= NSFB_ROTATE_NORTH; break;
    default:            nsfb->theGeo= nsfb->surfaceRtns->theGeo; break;
  }

  nsfb->offx= x;
  nsfb->offy= y;

  return( 0 );
}

ANSIC const char * nsfbSetAttrib( Nsfb * nsfb
                                 , const char * title )
{ const char * tmp= nsfb->theTitle;

  nsfb->theTitle= title;

  return( tmp );
}

/* exported interface documented in libnsfb.h
 */
ANSIC int  nsfbSetGeometry( Nsfb * nsfb
                          , int width, int height
                          , enum NsfbFormat format )
{ int fmt= format & NSFB_FMT_MASK;

  if ( width  <= 0         ) { width = nsfb->width;  }
  if ( height <= 0         ) { height= nsfb->height; }
  if ( fmt == NSFB_FMT_ANY ) { format= nsfb->format; }

/*  Default clip
 */
  nsfb->clip.x0= nsfb->clip.y0= 0;
  nsfb->clip.x1= width + 1;
  nsfb->clip.y1= height+ 1;

  return( nsfb->surfaceRtns->geometry( nsfb
                                     , width, height
                                     , format ));
}

/* exported interface documented in libnsfb.h
 */
ANSIC int  nsfbSetPan( Nsfb * nsfb, int type )
{ return( nsfb->surfaceRtns->pan( nsfb, type ) );
}



/* exported interface documented in libnsfb.h ( foreground )
 */
ANSIC void * nsfbGetGeometry( Nsfb * nsfb
                            , int  * width
                            , int  * height
                            , enum NsfbFormat * format)

{ if ( width  ) { *width = nsfb->width;  }
  if ( height ) { *height= nsfb->height; }
  if ( format ) { *format= nsfb->format; }

  return( nsfb->loc );
}

ANSIC int nsfbGetStride( Nsfb * nsfb )
{ return( nsfb ? nsfb->loclen >> 2 : -1 );
}


ANSIC int nsfbGetSurfGeo( Nsfb * nsfb )
{ return( nsfb->surfaceRtns->theGeo );
}

/* exported interface documented in libnsfb.h ( background )
 */
ANSIC void * nsfbGetBackStore( Nsfb * nsfb
                             , int  * width
                             , int  * height
                             , enum NsfbFormat * format)

{ if ( width  ) { *width = nsfb->width;  }
  if ( height ) { *height= nsfb->height; }
  if ( format ) { *format= nsfb->format; }

  return( nsfb->loc );
}


/* exported interface documented in libnsfb.h
 */
ANSIC int nsfbGetBuffer( Nsfb  * nsfb
                        , byte ** ptr
                        , int   * linelen )
{ if (ptr)
  { *ptr = nsfb->loc;
  }

  if (linelen )
  { *linelen = nsfb->loclen;
  }

  return( 0 );
}


/* To interface to multiplexer ( select or poll )
 */

ANSIC NsfbSurfacefnEvents nsfbGetEventCursor(  NsfbSurfaceRtns * surf )
{ return( surf ? surf->cursor : NULL );
}

ANSIC NsfbSurfacefnEvents nsfbGetEventSinker(  NsfbSurfaceRtns * surf )
{ return( surf ? surf->events : NULL );
}

ANSIC int                 nsfbGetEventHandler(  NsfbSurfaceRtns * surf )
{ return( surf ? surf->fd     : -1 );
}

ANSIC int                 nsfbGetDepth(       NsfbSurfaceRtns * surf )
{ return( surf ? surf->theDepth: -1 );
}

ANSIC int                 nsfbGetWidth(       NsfbSurfaceRtns * surf )
{ return( surf ? surf->width : -1 );
}

ANSIC int                 nsfbGetHard(        NsfbSurfaceRtns * surf )
{ return( surf ? surf->initialise( NULL ) : -1 );
}

ANSIC int                 nsfbGetHeight(      NsfbSurfaceRtns * surf )
{ return( surf ? surf->height : -1 );
}


ANSIC void *  nsfbSetEventSourcer( Nsfb * nsfb, void * code, void * data )
{ ToClientFun old= (ToClientFun)nsfb->toClient;

  nsfb->toData= data;
  nsfb->toClient= code;
  return( old );
}



/*
 * Local variables:
 *  c-basic-offset: 4
 *  tab-width: 8
 * End:
 */

