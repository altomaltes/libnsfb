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


#endif

#include <string.h>
#include <errno.h>


#include "../libnsfb_plot_util.h"
#include "../plot.h"
#include "../surface.h"
#include "../cursor.h"


static int linuxSetGeometry( Nsfb * nsfb
                           , int width, int height
                           , enum NsfbFormat format )
{ nsfb->width = width;
  nsfb->height= height;
  nsfb->format= format;

  if ( selectPlotters( nsfb ))
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
static int fbuffFinalise( Nsfb * nsfb )
{ if ( nsfb )
  { if ( nsfb->pan )
    { free( nsfb->pan );
      nsfb->pan= NULL;
    }
    if ( nsfb->loc )
    { free( nsfb->loc );
      nsfb->loc= NULL;
  } }

  return( 0 );
}

static int fbuffInitialise( Nsfb * nsfb )
{
  if ( nsfb )
  { nsfb->loclen=
    nsfb->panlen= nsfb->width * sizeof(int);
    nsfb->buflen= nsfb->width * nsfb->height * sizeof( int );

//  fprintf( stderr, "MEMES %d\n",  nsfb->buflen );

    nsfb->loc= CALLOC( nsfb->buflen );
    nsfb->pan= NULL;


#ifdef NSFB_RAM_FORMAT_CALC

    nsfb->format= GETFMITEM( 8,  0, 0 )  /* R len, off */
                | GETFMITEM( 8,  8, 1 )  /* G len, off */
                | GETFMITEM( 8, 16, 2 )  /* B len, off */
                | GETFMITEM( 8, 24, 3 ); /* Not alpha. as saying */

#else

    nsfb->format= NSFB_FMT_ABGR8888;

#endif

    if ( selectPlotters( nsfb ) )  /* select default sw plotters for format */
    { fbuffFinalise( nsfb );
      return( -1 );
  } }

  return( 0 );
}


static int fbuffPan( Nsfb * nsfb, int type )
{ switch( type & 0x0F )
  { case NSFB_PAN_START: puts("PAN START"); return( 4 );  /* Preparation         */
    case NSFB_PAN_DUMP:  puts("PAN DUMP" ); return( 3 );  /* do backing store    */

    case NSFB_PAN_BSTORE:              /* Start backing store */
      memcpy( nsfb->pan                /* now, working area   */
            , nsfb->loc                /* now, visible        */
            , nsfb->buflen );
    break;

    case NSFB_PAN_SWITCH:              /* Dump */
      memcpy( nsfb->pan                /* now, working area   */
            , nsfb->loc                /* now, visible        */
            , nsfb->buflen );
    break;

    default: puts("PAN UNKNOWN");

  }
  return( 0 );
}

/**
 *  identifies a framebuffer
 */
unsigned long getFbSize( )
{ struct fb_var_screeninfo VarInfo;
  int fd= open( FB_NAME, O_RDWR | O_NOCTTY );
  dword size= 0;

  if ( fd < 0 )
  { fprintf( stderr, "Unable to open %s.\n", FB_NAME );
  }

  else
  { if ( ioctl( fd, FBIOGET_VSCREENINFO, &VarInfo ) < 0)
    { fprintf( stderr,  "Unable to retrieve variable screen info: %s\n"
             , strerror(errno));
    }
    else
    { size = VarInfo.yres; size <<= 16;
      size|= VarInfo.xres;
    }

    close( fd );
  }

  fprintf( stderr, "SURFACE %s %d %d %X\n", FB_NAME, VarInfo.yres, VarInfo.xres, size );

  return( size );
}

/**
 *
 */
static NsfbSurfaceRtns * initLinux( NsfbSurfaceRtns * drv )
{ int    code, format;

  struct fb_fix_screeninfo FixInfo;
  struct fb_var_screeninfo VarInfo;

  drv->fd= open( FB_NAME, O_RDWR | O_NOCTTY );

  if ( drv->fd < 0 )
  { fprintf( stderr, "Unable to open %s.\n", FB_NAME);
    return( NULL);
  }

 /* Open the framebuffer device in read write
 */

  ioctl( drv->fd, FBIOBLANK, FB_BLANK_UNBLANK );

/* Do Ioctl. Retrieve fixed screen info.
 */
  if ( ioctl( drv->fd
            , FBIOGET_FSCREENINFO
            , &FixInfo ) < 0 )
  { fprintf( stderr, "get fixed screen info failed: %s\n"
           , strerror(errno));
    close( drv->fd );
    return( NULL );
  }

/* Do Ioctl. Get the variable screen info.
 */
  if ( ioctl( drv->fd, FBIOGET_VSCREENINFO, &VarInfo ) < 0)
  { fprintf( stderr,  "Unable to retrieve variable screen info: %s\n"
           , strerror(errno));
    close( drv->fd );
	   return( NULL );
  }


/* Do Ioctl. Put desired scrren info
 */
//  VarInfo.rotate= FB_ROTATE_CCW;
 // VarInfo.nonstd= 0;
 // VarInfo.grayscale= 0;
 // VarInfo.accel_flags= 0; /* No autoscroll */
 // VarInfo.bits_per_pixel= 16; /* No autoscroll */
 // VarInfo.vmode=    FB_VMODE_YWRAP;

  drv->theWidth = VarInfo.xres_virtual;
  drv->theHeigth= VarInfo.yres_virtual;
  drv->theDepth = VarInfo.bits_per_pixel;
//  drv->theGeo=   VarInfo.rotate;

  if ( ioctl( drv->fd, FBIOPUT_VSCREENINFO, &VarInfo) < 0)
  { perror("display-fbdev: FBIOPUT_VSCREENINFO");
    return( NULL );
  }

  if (ioctl( drv->fd, FBIOGET_VSCREENINFO, &VarInfo) < 0)
  { fprintf( stderr, "Unable to retrieve variable screen info: %s\n"
           , strerror(errno) );
    close( drv->fd ); drv->fd= INT_NAN;
    return( NULL );
  }


/* Calculate the size to mmap
 */
  drv->buffSize= FixInfo.line_length * VarInfo.yres;

/* Now mmap the framebuffer.
 */
  drv->buffStart= mmap( NULL, FixInfo.smem_len                   /* Support doublebuffer */
                      , PROT_READ | PROT_WRITE, MAP_SHARED
                      , drv->fd, 0 );
  if ( !drv->buffStart )
  { fprintf( stderr, "mmap failed:\n");
    close( drv->fd );
    return( NULL );
  }

//  nsfb->loc= ( panel->FixInfo.smem_len < (panel->size << 1 ))
//           ? CALLOC( lstate->size )                          /* Double buffer not suported */
// !!!           : ((byte*)lstate->mem) + lstate->size;               /* Double buffer OK */

//  nsfb->loclen= nsfb->width * VarInfo.bits_per_pixel >> 3;
//  nsfb->loc= CALLOC( nsfb->height * nsfb->loclen );   // Not double buffer schema

  memset( drv->buffStart, 0, FixInfo.smem_len );
// relay on virtual, conceptually correct drv->width=  VarInfo.xres;
//  drv->height= VarInfo.yres;
  drv->stride= FixInfo.line_length;



//  lstate->VarInfo.width = lstate->VarInfo.xres;
//  lstate->VarInfo.height= lstate->VarInfo.yres;
//  nsfb->width  = width;
//  nsfb->height = height;
//  lformat = format_from_lstate( lstate );


  fprintf( stderr, "Resolution %d %d\n"
         , drv->theWidth
         , drv->theHeigth );
//        , nsfb->width,  nsfb->height );

  drv->format= GETFMITEM( VarInfo.red.length,   VarInfo.red.offset,    0 )
             | GETFMITEM( VarInfo.green.length, VarInfo.green.offset,  1 )
             | GETFMITEM( VarInfo.blue.length,  VarInfo.blue.offset ,  2 )
             | GETFMITEM(                   0,  VarInfo.transp.offset, 3 ); // Not alpha. as saying

/*  fprintf( stderr, " %d %d \n"
          " %d %d \n"
          " %d %d \n"
          " %d %d \n"
          " %X \n"
         , VarInfo.red.length,    VarInfo.red.offset
         , VarInfo.green.length,  VarInfo.green.offset
         , VarInfo.blue.length,   VarInfo.blue.offset
         , VarInfo.transp.length, VarInfo.transp.offset
         , drv->format );
  */
  VarInfo.xoffset=
  VarInfo.yoffset= 0;

  VarInfo.activate= FB_ACTIVATE_VBL
                  | FB_ACTIVATE_FORCE;

  ioctl( drv->fd
       , FBIOPAN_DISPLAY
       , &VarInfo );
  openVt( 1, drv ); /* Manage virtual terminal switch */

  return( &drv );
}

static int linuxFinalise( Nsfb * nsfb )
{ if ( nsfb )
  { if ( nsfb->surfaceRtns->buffStart )
    { munmap( nsfb->surfaceRtns->buffStart, 0) ;
      nsfb->surfaceRtns->buffStart= NULL;
      close( nsfb->surfaceRtns->fd );
      nsfb->surfaceRtns->fd= INT_NAN;
    }
    if ( nsfb->loc )
    { FREE( nsfb->loc );
  } }

  return( 0 );
}

/** ================================================= [ JACS, 10/01/2022 ] == *\
 *                                                                            *
 *   JACS 2022                                                                *
 *                                                                            *
 *                                                                            *
\* ========================================================================= **/
struct linuxList
{ Nsfb      seed;        /* Includes up and next */

};

static int linuxInitialise( Nsfb * nsfb )
{ extern NsfbSurfaceRtns linuxRtns;

  if ( !linuxRtns.buffStart )   /* Initialize at first use */
  { if ( !initLinux( &linuxRtns ))
    { //return( -2 );
  } }

  if ( nsfb )
  { nsfb->surfaceRtns= &linuxRtns;
    nsfb->theGeo     = linuxRtns.theGeo;       /* Default rotation */
    nsfb->theGeo     = linuxRtns.theGeo;       /* Default rotation */

    nsfb->fbWidth = linuxRtns.theWidth;
    nsfb->fbHeigth= linuxRtns.theHeigth;

    nsfb->format= NSFB_FMT_ABGR8888;
    nsfb->loc   = CALLOC( nsfb->buflen= nsfb->width * nsfb->height * sizeof( int ) );
    nsfb->pan   = linuxRtns.buffStart;

/*  Apply window position
 */
    nsfb->pan += nsfb->offy * linuxRtns.stride;
    nsfb->pan += nsfb->offx * sizeof( int );

    nsfb->loclen= nsfb->width * sizeof( int );
    nsfb->panlen= linuxRtns.stride;

    if ( selectPlotters( nsfb ) )  /* select default sw plotters for format */
    { linuxFinalise( nsfb );
      return( -1 );
    }

    return( 0 );
  }
  return( -1 );
}

/** ================================================= [ JACS, 10/01/2022 ] == *\
 *                                                                            *
 *   JACS 2022                                                                *
 *                                                                            *
 *                                                                            *
\* ========================================================================= **/
static int linuxPan( Nsfb * nsfb, int type )
{ void modeData(  )
  {  printf( "-----------------> %x %x %d\n", nsfb->pan, nsfb->loc, nsfb->buflen  );

    switch( nsfb->theGeo )
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

//  struct LnxPriv *lstate= nsfb->surfacePriv;

 // lstate->VarInfo.xoffset= 0;


  switch( type & 0x0F )
  { case NSFB_PAN_START:                     /* Preparation      */
      puts("PAN START ---------------------------");
	 //    nsfb->loc= lstate->mem + 0;
	 //    nsfb->pan= lstate->mem + lstate->size;
    return(0);

  	 case NSFB_PAN_DUMP:
      puts("PAN DUMP ---------------------------");

      return(3);            /* do backing store */

//      if ( lstate->VarInfo.yoffset )           /* page 2           */
// 	    { lstate->VarInfo.yoffset= 0;            /* Switch pages     */
//	       nsfb->loc= lstate->mem + 0;
//	       nsfb->pan= lstate->mem + lstate->size;
//  	   }
//  	   else                                     /* Page 1           */
//  	   { lstate->VarInfo.yoffset= nsfb->height;
//	       nsfb->pan= lstate->mem + 0;
//	       nsfb->loc= lstate->mem + lstate->size;
//   	  }

   	//  modeData( lstate->size );
    break;

    case NSFB_PAN_BSTORE:                       /* Start backing store */
      printf( "PAN HARDWARE ---%d %d %d %d -----------------------\n", nsfb->width, nsfb->height, nsfb->loclen, nsfb->panlen );

   //   { lstate->VarInfo.yoffset= 0;
   //   }

      switch( nsfb->theGeo )
      { case NSFB_ROTATE_NORTH:
        case NSFB_ROTATE_SOUTH:

        case NSFB_ROTATE_EAST:
        case NSFB_ROTATE_WEST:  /* Swap dimension */
          nsfb->loclen= ( nsfb->width * nsfb->bpp ) / 8;
     //     type= nsfb->width;
     //     nsfb->width= nsfb->height;
     //     nsfb->height= type;
//          nsfb->panlen= ( nsfb->height * nsfb->bpp ) / 8;
        break;
      }

      nsfb->clip.x0= 0;
      nsfb->clip.y0= 0;
      nsfb->clip.x1= nsfb->width;
      nsfb->clip.y1= nsfb->height;
    break;

    case NSFB_PAN_SWITCH:                     /* Dump          */
      puts("PAN SWITCH ---------------------------");
      return( 1 );

  //   if ( lstate->VarInfo.yoffset )           /* page 2           */
  //   { lstate->VarInfo.yoffset= 0;            /* Switch pages     */
  //     nsfb->loc= lstate->mem + 0;
   //    nsfb->pan= lstate->mem + lstate->size;
  //   }
  //   else                                     /* Page 1 */
 // 	 { lstate->VarInfo.yoffset= nsfb->height;
	   //  nsfb->pan= lstate->mem + 0;
	   //  nsfb->loc= lstate->mem + lstate->size;
 // 	 }
    break;

    default:
     puts("PAN OTHER ---------------------------");
     return(2);
 //    lstate->VarInfo.yoffset= nsfb->height * type;    /* Switch screens */
	//    nsfb->pan= lstate->mem + 0;
	//    nsfb->loc= lstate->mem + lstate->size * type;
  }

  //return( ioctl( frameBuffFd[ 0 ]
    //           , FBIOPAN_DISPLAY
      //         , &lstate->VarInfo ));
}


/*
 *
 */
static int linuxClaim( Nsfb *nsfb, NsfbBbox * box )
{ //if ( cursor )

  return 0;
}


/**
 * plotted :  0 -> only write, status    -> 1
 *         :  1 -> recover and write     -> 1
 *         : -1 -> only recover          -> 0
 */

static int voidCursor( NsfbSurfaceRtns * surf )
{ return( -1 );
}

static int linuxCursor( NsfbSurfaceRtns * surf )
{ if ( surf )
  { if ( surf->pointer &&  surf->buffStart )
    { int plotted= surf->pointer->plotted;

      if ( plotted > -2 )   /* Not hidded */
      { surfaceBitmapXRGB( surf->buffStart, surf->stride   /* Restore previous ( also on clear ) */
                         , surf->pointer->savDat
                         , surf->pointer->last.posi.x
                         , surf->pointer->last.posi.y
                         , surf->pointer->last.size.w
                         , surf->pointer->last.size.h );   /* Mark as restored */
        surf->pointer->plotted= 0;
        if ( plotted < 0 )                                 /* First time init */
        { return( 0 );
      } }

    /* Draw new */
      surfaceAlphaBitmapXRGB( surf->buffStart, surf->stride
                            , surf->pointer->ico->map.image, surf->pointer->ico->width
                            , surf->pointer->savDat
                            , surf->pointer->loc.posi.x
                            , surf->pointer->loc.posi.y
                            , surf->pointer->loc.size.w
                            , surf->pointer->loc.size.h );
        surf->pointer->last= surf->pointer->loc;
        surf->pointer->plotted= 1;
    }

    return( true );
  }

  return( false );
}


/* No physical output, so no update needed
 */
static int voidUpdate( Nsfb     * nsfb
                     , NsfbBbox * box )
{ UNUSED( box );

//  if ( cursor )
//  { if ( !cursor->plotted )
//    { nsfbCursor_plot( nsfb );
//  } }

  return( 0 );
}

/** ================================================= [ JACS, 10/01/2014 ] == *\
 *                                                                            *
 *   JACS 2023                                                                *
 *                                                                            *
 *  JUMPTABLE                                                                 *
 *                                                                            *
\* ========================================================================= **/
static int fbuffPixmap( struct x11Priv * x11
                      , ImageMap * map
                      , void     * data
                      , void     * mask
                      , int bmpWidth, int bmpHeight )
{ if ( map )
  { unsigned int overload= bmpWidth  << 0
                         | bmpHeight << 16 ;

     map->image= data;
     map->mask=  (void*)overload;
  }


  return( 0 );
}

/** ================================================= [ JACS, 10/01/2014 ] == *\
 *                                                                            *
 *   JACS 2012                                                                *
 *                                                                            *
 *  JUMPTABLE                                                                 *
 *                                                                            *
\* ========================================================================= **/
NsfbSurfaceRtns fbuffRtns =
{ .type= NSFB_SURFACE_FBUFF
, .name= "ram@vga"
, .fd= -1

, .theDepth  = 32

, .initialise= fbuffInitialise
, .finalise  = fbuffFinalise
, .geometry  = linuxSetGeometry
, .pan       = fbuffPan
, .pixmap    = fbuffPixmap
, .update    = voidUpdate
, .cursor    = voidCursor

, .dataSize  = sizeof( Nsfb )
};
NSFB_SURFACE_DEF( fbuffRtns )


NsfbSurfaceRtns linuxRtns=
{ .type=  NSFB_SURFACE_LINUX
, .name= "fb@vga"
, .fd  = -1

, .theDepth  = 0

, .initialise= linuxInitialise
, .finalise  = linuxFinalise
, .claim     = linuxClaim
, .cursor    = linuxCursor
, .geometry  = linuxSetGeometry
, .pan       = linuxPan
, .pixmap    = fbuffPixmap
, .events    = nsfbAttEvent

, .dataSize  = sizeof( linuxRtns )

};

NSFB_SURFACE_DEF( linuxRtns )

/*
 *
 */
NsfbSurfaceRtns * newNode( const char * mode )
{ _nsfb_register_surface( &linuxRtns );
  _nsfb_register_surface( &fbuffRtns );

  return( NULL );
}



int linuxFbOutput( int active, NsfbSurfaceRtns * sf )
{ if ( sf->panType= active )  /* switch on */
  { if ( sf->clients->plotterFns )
    { sf->clients->plotterFns->moverect( sf->clients            /* Total pan*/
                                       , 0, 0
                                       , sf->clients->width
                                       , sf->clients->height );
  } }
  else           /* switch off */
  {
  }

  return( 0 );
}


