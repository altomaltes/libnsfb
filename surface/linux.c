/*
 * Copyright 2012 Vincent Sanders <vince@simtec.co.uk>
 *
 * This file is part of libnsfb, http://www.netsurf-browser.org/
 * Licenced under the MIT License,
 *                http://www.opensource.org/licenses/mit-license.php
 */

int openVt( int flags, void * user );

#ifndef _WIN32

  #include <sys/types.h>
  #include <sys/stat.h>
  #include <sys/ioctl.h>
  #include <fcntl.h>
  #include <sys/mman.h>
  #include <linux/fb.h>
  #include <unistd.h>

  #define FB_NAME "/dev/fb0"
  #define INT_NAN 0x80000000

struct LnxPriv
{ struct fb_fix_screeninfo FixInfo;
  struct fb_var_screeninfo VarInfo;
  int noPan; int size; int flags;
  void * mem;
};


#endif

#include <string.h>
#include <errno.h>


#include "../libnsfb_plot_util.h"
#include "../plot.h"
#include "../surface.h"
#include "../cursor.h"


static int linux_set_geometry( nsfb_t * nsfb
                             , int width, int height
                             , enum nsfb_format_e format )
{ if ( nsfb->surface_priv )
  { return( -1 ); /* if we are already initialised fail */
  }

  printf( "SET GEO %d %d \n", width, height );

  nsfb->width = width;
  nsfb->height= height;
  nsfb->format= format;

/* select default sw plotters for bpp
 */
  if ( select_plotters( nsfb ))
  { return( -1 );
  }

  return 0;
}

typedef struct
{ int x;
  int y;
} vc;


/* Init functions
 */

static int fbuff_finalise( nsfb_t * nsfb )
{ if ( nsfb )
  { if ( nsfb->pan )
    { free( nsfb->pan );
      nsfb->pan= NULL;
    }
    if ( nsfb->loc )
    { free( nsfb->loc );
      nsfb->loc= NULL;
  } }

  return( -1 );
}

static int fbuff_initialise( nsfb_t * nsfb )
{ int frameSize;

  if ( nsfb->surface_priv )
  { return( -1 );
  }

  nsfb->loclen=
  nsfb->panlen= nsfb->width  * sizeof(int);
  nsfb->buflen= nsfb->panlen * nsfb->height;

  fprintf( stderr, "MEMES %d\n",  nsfb->buflen );

  nsfb->loc= calloc( nsfb->buflen, 1 );
  nsfb->pan= calloc( nsfb->buflen, 1 );


#ifdef NSFB_RAM_FORMAT_CALC

  nsfb->format= GETFMITEM( 8,  0, 0 )  /* R len, off */
              | GETFMITEM( 8,  8, 1 )  /* G len, off */
              | GETFMITEM( 8, 16, 2 )  /* B len, off */
              | GETFMITEM( 8, 24, 3 ); /* Not alpha. as saying */

#else

  nsfb->format= NSFB_FMT_ABGR8888;

#endif


  if ( select_plotters( nsfb ) )  /* select default sw plotters for format */
  { fbuff_finalise( nsfb );
    return( -1 );
  }

  return( 0 );
}


static int fbuff_pan( nsfb_t * nsfb, int type )
{ switch( type & 0x0F )
  { case NSFB_PAN_START: puts("PAN START"); return( 4 );  /* Preparation         */
    case NSFB_PAN_DUMP:  puts("PAN DUMP" ); return( 3 );  /* do backing store    */

    case NSFB_PAN_BSTORE:              /* Start backing store */
      puts("PAN BSTORE");
      memcpy( nsfb->pan                /* now, working area   */
            , nsfb->loc                /* now, visible        */
            , nsfb->buflen );
    break;

    case NSFB_PAN_SWITCH:              /* Dump */
      puts("PAN SWITCH");
      memcpy( nsfb->pan                /* now, working area   */
            , nsfb->loc                /* now, visible        */
            , nsfb->buflen );
    break;

    default: puts("PAN UNKNOWN");
    
  }
  return( 0 );
}

static int frameBuffFd[]={ INT_NAN, INT_NAN,  INT_NAN,  INT_NAN };     /* Represents hardware, so static */

static int linux_initialise( nsfb_t * nsfb )
{ struct LnxPriv * lstate;
  enum   nsfb_format_e lformat;
  int    code, format;

  if ( frameBuffFd[ 0 ] < 0 )
  { frameBuffFd[ 0 ]= open( FB_NAME, O_RDWR | O_NOCTTY );
  }

  if ( !nsfb )                              /* Test for availability */
  { return( frameBuffFd[ 0 ] );
  }

  if ( frameBuffFd[ 0 ] < 0 )
  { fprintf( stderr, "Unable to open %s.\n", FB_NAME);
    free(lstate);
    return( -3 );
  }

  if ( nsfb->surface_priv )  /* Avoid reentrance */
  { return( -1 );
  }

  lstate= (struct LnxPriv *)calloc( sizeof( struct LnxPriv ), 1 );

  if ( ! lstate )
  { return( -2 );
  }

/* Split flags
 */
  format=        nsfb->format &  NSFB_FMT_MASK;
  lstate->flags= nsfb->format & ~NSFB_FMT_MASK;

/* Open the framebuffer device in read write
 */

  ioctl( frameBuffFd[ 0 ], FBIOBLANK, FB_BLANK_UNBLANK );

/* Do Ioctl. Retrieve fixed screen info.
 */
  if ( ioctl( frameBuffFd[ 0 ], FBIOGET_FSCREENINFO, &lstate->FixInfo ) < 0 )
  { fprintf( stderr, "get fixed screen info failed: %s\n"
           , strerror(errno));
    close( frameBuffFd[ 0 ] ); frameBuffFd[ 0 ]= INT_NAN;
	   free( lstate     );
    return( -4 );
  }

/* Do Ioctl. Get the variable screen info.
 */
  if ( ioctl( frameBuffFd[ 0 ], FBIOGET_VSCREENINFO, &lstate->VarInfo ) < 0)
  { fprintf( stderr,  "Unable to retrieve variable screen info: %s\n"
           , strerror(errno));
    close( frameBuffFd[ 0 ] ); frameBuffFd[ 0 ]= INT_NAN;
	   free(lstate);
	   return( -5 );
  }


/* Do Ioctl. Put desired scrren info
 */
//  lstate->VarInfo.rotate= FB_ROTATE_CCW;
 // lstate->VarInfo.nonstd= 0;
 // lstate->VarInfo.grayscale= 0;
 // lstate->VarInfo.accel_flags= 0; /* No autoscroll */
 // lstate->VarInfo.bits_per_pixel= 16; /* No autoscroll */
 // lstate->VarInfo.vmode=    FB_VMODE_YWRAP;

  lstate->VarInfo.rotate= 0;

  if ( ioctl( frameBuffFd[ 0 ], FBIOPUT_VSCREENINFO, &lstate->VarInfo) < 0)
  { perror("display-fbdev: FBIOPUT_VSCREENINFO");
    return( -1 );
  }

  if (ioctl( frameBuffFd[ 0 ], FBIOGET_VSCREENINFO, &lstate->VarInfo) < 0)
  { fprintf( stderr, "Unable to retrieve variable screen info: %s\n"
           , strerror(errno) );
    close( frameBuffFd[ 0 ] ); frameBuffFd[ 0 ]= INT_NAN;
    free(lstate);
    return( -5 );
  }


/* Calculate the size to mmap */
  lstate->size= lstate->FixInfo.line_length
              * lstate->VarInfo.yres;
//  fprintf( stderr, "LineLen %d %d\n", lstate->FixInfo.line_length, lstate->size );

/* Now mmap the framebuffer. */

  lstate->mem= nsfb->loc= mmap( NULL, lstate->FixInfo.smem_len                   /* Support doublebuffer */
                              , PROT_READ | PROT_WRITE, MAP_SHARED
                              , frameBuffFd[ 0 ], 0 );
  if ( !nsfb->loc )
  { fprintf( stderr, "mmap failed:\n");
    close( frameBuffFd[ 0 ] ); frameBuffFd[ 0 ]= INT_NAN;
    free(lstate);
    return( -6 );
  }

  nsfb->pan= ( lstate->FixInfo.smem_len < (lstate->size << 1 ))
            ? calloc( lstate->size, 1 )    /* Double buffer not suported */
            : ((byte*)lstate->mem) + lstate->size;  /* Double buffer OK */

  memset( lstate->mem, 0, lstate->FixInfo.smem_len );

  nsfb->loclen =
  nsfb->panlen = lstate->FixInfo.line_length;
  nsfb->width  = lstate->VarInfo.width = lstate->VarInfo.xres;
  nsfb->height = lstate->VarInfo.height= lstate->VarInfo.yres;
//  lformat = format_from_lstate( lstate );

  printf( "Resolution %d %d\n",  nsfb->width,  nsfb->height );

  lformat= GETFMITEM( lstate->VarInfo.red.length,   lstate->VarInfo.red.offset,    0 )
         | GETFMITEM( lstate->VarInfo.green.length, lstate->VarInfo.green.offset,  1 )
         | GETFMITEM( lstate->VarInfo.blue.length,  lstate->VarInfo.blue.offset ,  2 )
         | GETFMITEM(                           0,  lstate->VarInfo.transp.offset, 3 ); // Not alpha. as saying

  fprintf( stderr, " %d %d \n"
          " %d %d \n"
          " %d %d \n"
          " %d %d \n"
          " %X \n"
         , lstate->VarInfo.red.length,    lstate->VarInfo.red.offset
         , lstate->VarInfo.green.length,  lstate->VarInfo.green.offset
         , lstate->VarInfo.blue.length,   lstate->VarInfo.blue.offset
         , lstate->VarInfo.transp.length, lstate->VarInfo.transp.offset
         , lformat );

  if ( format != lformat )
  { nsfb->format= lformat;

    if ( select_plotters( nsfb ))  /* select default sw plotters for format */
    { munmap( lstate->mem, 0);
      close( frameBuffFd[ 0 ] ); frameBuffFd[ 0 ]= INT_NAN;
      free( lstate );
      return( -1 );
  } }

  lstate->VarInfo.xoffset=
  lstate->VarInfo.yoffset= 0;

  lstate->VarInfo.activate= FB_ACTIVATE_VBL
                          | FB_ACTIVATE_FORCE;

  lstate->VarInfo.xoffset= 0;
  lstate->noPan= 1;             /* Mark as not ready */
  nsfb->surface_priv= lstate;

  code= ioctl( frameBuffFd[ 0 ]
             , FBIOPAN_DISPLAY
             , &lstate->VarInfo );
  nsfb->buflen= lstate->size;
  return( 0 );
}

static int linux_pan( nsfb_t * nsfb, int type )
{ void modeData(  )
  { printf( "-----------------> %x %x %d\n", nsfb->pan, nsfb->loc, nsfb->buflen  );

    if ( nsfb->active1 ) switch( nsfb->rotate )
    { case NSFB_ROTATE_NORTH:
        memcpy( nsfb->pan       /* now, working area */
              , nsfb->loc       /* now, visible      */
              , nsfb->buflen );
      return;

     case NSFB_ROTATE_WEST:
     case NSFB_ROTATE_EAST :
     case NSFB_ROTATE_SOUTH:
       fbPlotMoverect( nsfb
                       , nsfb->width
                       , nsfb->height
                       , 0, 0);
  }  }

  if ( !nsfb )
  { return( -EINVAL );
  }

  struct LnxPriv *lstate= nsfb->surface_priv;

  lstate->VarInfo.xoffset= 0;

  puts("pan ---------------------------");

  switch( type & 0x0F )
  { case NSFB_PAN_START:                     /* Preparation      */
  	   return(4);
	     nsfb->loc= lstate->mem + 0;
	     nsfb->pan= lstate->mem + lstate->size;
    return(0);

  	 case NSFB_PAN_DUMP: return(3);            /* do backing store */

      if ( lstate->VarInfo.yoffset )           /* page 2           */
 	    { lstate->VarInfo.yoffset= 0;            /* Switch pages     */
	       nsfb->loc= lstate->mem + 0;
	       nsfb->pan= lstate->mem + lstate->size;
  	   }
  	   else                                     /* Page 1           */
  	   { lstate->VarInfo.yoffset= nsfb->height;
	       nsfb->pan= lstate->mem + 0;
	       nsfb->loc= lstate->mem + lstate->size;
   	  }

   	  modeData( lstate->size );
    break;

    case NSFB_PAN_BSTORE:                       /* Start backing store */
  	   lstate->VarInfo.yoffset= nsfb->height;

  	   lstate->noPan= ioctl( frameBuffFd[ 0 ]          /* Real kernel support */
                          , FBIOPAN_DISPLAY
                          , &lstate->VarInfo );
      lstate->noPan= -1;
      lstate->VarInfo.yoffset= 0;
//      nsfb->pan= lstate->mem;                  /* make current  as pan (visible) */
 //     nsfb->loc= lstate->mem + lstate->size;   /* draw on hidden page            */
  	   type &= 0x00F0;

      if ( nsfb->rotate != type ) switch( nsfb->rotate= type )
      { case NSFB_ROTATE_NORTH:
        case NSFB_ROTATE_SOUTH:
        break;

        case NSFB_ROTATE_EAST:
        case NSFB_ROTATE_WEST:  /* Swap dimension */
          nsfb->panlen= ( nsfb->width * nsfb->bpp ) / 8;
          type= nsfb->width;
          nsfb->width= nsfb->height;
          nsfb->height= type;
          nsfb->loclen= ( nsfb->width * nsfb->bpp ) / 8;
        break;
      }

      nsfb->clip.x0= 0;
      nsfb->clip.y0= 0;
      nsfb->clip.x1= nsfb->width;
      nsfb->clip.y1= nsfb->height;

      openVt( lstate->noPan, nsfb ); /* Manage virtual terminal switch */
    break;

    case NSFB_PAN_SWITCH:                     /* Dump          */
      if ( lstate->noPan )
      { modeData( );
      }
      return( 1 );

     if ( lstate->VarInfo.yoffset )           /* page 2           */
     { lstate->VarInfo.yoffset= 0;            /* Switch pages     */
  //     nsfb->loc= lstate->mem + 0;
   //    nsfb->pan= lstate->mem + lstate->size;
     }
     else                                     /* Page 1 */
  	 { lstate->VarInfo.yoffset= nsfb->height;
	   //  nsfb->pan= lstate->mem + 0;
	   //  nsfb->loc= lstate->mem + lstate->size;
  	 }
    break;

    default: return(2);
      lstate->VarInfo.yoffset= nsfb->height * type;    /* Switch screens */
	    nsfb->pan= lstate->mem + 0;
	    nsfb->loc= lstate->mem + lstate->size * type;
  }

  return( ioctl( frameBuffFd[ 0 ]
               , FBIOPAN_DISPLAY
               , &lstate->VarInfo ));
}


static int linux_finalise( nsfb_t * nsfb )
{ struct LnxPriv * lstate = nsfb->surface_priv;

  if (lstate != NULL)
  { munmap( lstate->mem, 0) ;
    close(  frameBuffFd[ 0 ]   );frameBuffFd[ 0 ]= INT_NAN;
    free(   lstate       );
    return( 0 );
  }

  return( 1 );
}

/*
 *
 */
static bool linux_input( nsfb_t * nsfb
                       , nsfb_event_t *event
                       , int timeout )
{ UNUSED( nsfb    );
  UNUSED( event   );
  UNUSED( timeout );

  return false;
}

static int linux_claim( nsfb_t *nsfb, nsfb_bbox_t * box )
{ struct nsfbCursor_s *cursor= nsfb->cursor;
       /*
  if ( cursor )
  { if ( cursor->plotted )
    { if ( nsfbPlotbbox_intersect( box, &cursor->loc ))
      { nsfb->plotter_fns->bitmap( nsfb
                                 , &cursor->savLoc
                                 ,  cursor->sav
                                 ,  cursor->savWidth
                                 ,  cursor->savHeight
                                 ,  cursor->savWidth
                                 ,  cursor->plotted= false );
  } } }
     */
  return 0;
}


static int linux_cursor( nsfb_t               * nsfb
                       , struct nsfbCursor_s * cursor )
{ if ( cursor )
  { nsfb_bbox_t sclip= nsfb->clip;
    nsfb->plotter_fns->set_clip( nsfb, NULL );

/* Clear old area
 */
    nsfb->cursor= NULL;          /* lock backstore */
      fbPlotMoverect( nsfb
                    , cursor->savWidth
                    , cursor->savHeight
                    , cursor->savLoc.x0
                    , cursor->savLoc.y0 );
    nsfb->cursor= cursor;         /* lock backstore */

    nsfb->plotter_fns->bitmap( nsfb
                             , &cursor->loc
                             ,  cursor->pixel
                             ,  cursor->bmp_width
                             ,  cursor->bmp_height, cursor->bmp_stride
                             , DO_ALPHA_BLEND | DO_FRONT_RENDER );
    nsfb->clip= sclip;

    return( true );
  }

  return( false );
}


static int linux_update( nsfb_t      * nsfb
                       , nsfb_bbox_t * box )
{ struct nsfbCursor_s *cursor = nsfb->cursor;

  puts("update .................. ");

  UNUSED( box );

//  if ( cursor )
//  { if ( !cursor->plotted )
//    { nsfbCursor_plot( nsfb );
//  } }

  return( 0 );
}

/** ================================================= [ JACS, 10/01/2014 ] == *\
 *                                                                            *
 *   JASC 2012                                                                *
 *                                                                            *
 *  JUMPTABLE                                                                 *
 *                                                                            *
\* ========================================================================= **/
const nsfb_surface_rtns_t egl_rtns =
{ .initialise= drmInitialize
, .finalise  = drmFinalise
, .input     = linux_input
, .claim     = linux_claim
, .update    = linux_update
, .cursor    = linux_cursor
, .geometry  = linux_set_geometry
, .pan       = drmPan
};
NSFB_SURFACE_DEF( egl, NSFB_SURFACE_EGL, &egl_rtns    )

const nsfb_surface_rtns_t drm_rtns =
{ .initialise= drmInitialize
, .finalise  = drmFinalise
, .input     = linux_input
, .claim     = linux_claim
, .update    = linux_update
, .cursor    = linux_cursor
, .geometry  = linux_set_geometry
, .pan       = drmPan
};
NSFB_SURFACE_DEF( drm,    NSFB_SURFACE_DRM, &drm_rtns    )

const nsfb_surface_rtns_t fbuff_rtns =
{ .initialise= fbuff_initialise
, .finalise  = fbuff_finalise
, .input     = linux_input
, .claim     = linux_claim
, .update    = linux_update
, .cursor    = linux_cursor
, .geometry  = linux_set_geometry
, .pan       = fbuff_pan
};
NSFB_SURFACE_DEF( ram, NSFB_SURFACE_FBUFF, &fbuff_rtns )


const nsfb_surface_rtns_t linux_rtns=
{ .initialise= linux_initialise
, .finalise  = linux_finalise
, .input     = linux_input
, .claim     = linux_claim
, .update    = linux_update
, .cursor    = linux_cursor
, .geometry  = linux_set_geometry
, .pan       = linux_pan
};

NSFB_SURFACE_DEF( fb, NSFB_SURFACE_LINUX, &linux_rtns )


int linuxFbOutput( int active, nsfb_t * sf )
{ if (( sf->active1= active ))
  { linux_pan( sf, NSFB_PAN_SWITCH );
  }
  else
  {
} }


