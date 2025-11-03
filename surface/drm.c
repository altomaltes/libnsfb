/*
 * modeset - DRM Double-Buffered Modesetting Example
 *
 * Written 2012 by David Rheinsberg <david.rheinsberg@gmail.com>
 * Dedicated to the Public Domain.
 */

/*
 * DRM Double-Buffered Modesetting Howto
 * This example extends the modeset.c howto and introduces double-buffering.
 * When drawing a new frame into a framebuffer, we should always draw into an
 * unused buffer and not into the front buffer. If we draw into the front
 * buffer, we might have drawn half the frame when the display-controller starts
 * scanning out the next frame. Hence, we see flickering on the screen.
 * The technique to avoid this is called double-buffering. We have two
 * framebuffers, the front buffer which is currently used for scanout and a
 * back-buffer that is used for drawing operations. When a frame is done, we
 * simply swap both buffers.
 * Swapping does not mean copying data, instead, only the pointers to the
 * buffers are swapped.
 *
 * Please read modeset.c before reading this file as most of the functions stay
 * the same. Only the differences are highlighted here.
 * Also note that triple-buffering or any other number of buffers can be easily
 * implemented by following the scheme here. However, in this example we limit
 * the number of buffers to 2 so it is easier to follow.
 */

#define byte unsigned char
#define dword unsigned int


#ifndef _WIN32
  #define DRM_NAME "/dev/dri/card0"
#endif


#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>
#include <xf86drm.h>
#include <xf86drmMode.h>


#include "../libnsfb_plot_util.h"
#include "../plot.h"
#include "../surface.h"
#include "../cursor.h"


struct ModesetBuf
{ dword width;
  dword height;
  dword stride;
  dword size;
  dword handle;
  byte * map;
  dword fb;
};

struct ModesetDev
{ struct ModesetDev *next;

  drmModeModeInfo mode;
  dword conn;
  dword crtc;
  drmModeCrtc * savedCrtc;

  struct ModesetBuf bufs[ 2 ];
};


/**
 * modeset_draw() is the place where things change. The render-logic is the same
 * and we still draw a solid-color on the whole screen. However, we now have two
 * buffers and need to flip between them.
 *
 * So before drawing into a framebuffer, we need to find the back-buffer.
 * Remember, dev->font_buf is the index of the front buffer, so
 * dev->front_buf ^ 1 is the index of the back buffer. We simply use
 * dev->bufs[dev->front_buf ^ 1] to get the back-buffer and draw into it.
 *
 * After we finished drawing, we need to flip the buffers. We do this with the
 * same call as we initially set the CRTC: drmModeSetCrtc(). However, we now
 * pass the back-buffer as new framebuffer as we want to flip them.
 * The only thing left to do is to change the dev->front_buf index to point to
 * the new back-buffer (which was previously the front buffer).
 * We then sleep for a short time period and start drawing again.
 *
 * If you run this example, you will notice that there is almost no flickering,
 * anymore. The buffers are now swapped as a whole so each new frame shows
 * always the whole new image. If you look carefully, you will notice that the
 * modeset.c example showed many screen corruptions during redraw-cycles.
 *
 * However, this example is still not perfect. Imagine the display-controller is
 * currently scanning out a new image and we call drmModeSetCrtc()
 * simultaneously. It will then have the same effect as if we used a single
 * buffer and we get some tearing. But, the chance that this happens is a lot
 * less likely as with a single-buffer. This is because there is a long period
 * between each frame called vertical-blank where the display-controller does
 * not perform a scanout. If we swap the buffers in this period, we have the
 * guarantee that there will be no tearing. See the modeset-vsync.c example if
 * you want to know how you can guarantee that the swap takes place at a
 * vertical-sync.
 */

static struct ModesetDev *modesetList = NULL;

/**
 * modesetOpen() stays the same as before.
 */
static int modesetOpen( int * out
                      , const char * node )
{ int fd, ret;
  uint64_t has_dumb;

  node= "/dev/dri/card0";

  if (( fd= open( node, O_RDWR | O_CLOEXEC ) ) < 0)
  { ret = -errno;
    fprintf( stderr, "cannot open > '%s': %m\n"
           , node );
    return( ret );
  }

  ret= drmGetCap( fd, DRM_CAP_DUMB_BUFFER, &has_dumb ) < 0;

  if ( ret || !has_dumb )
  { fprintf( stderr
           , "drm device '%s' does not support dumb buffers\n"
           , node);
    close(fd);
    return( -EOPNOTSUPP );
  }

  *out = fd;
  return 0;
}

/**
 * modesetDestroyFb() is a new function. It does exactly the reverse of
 * modesetCreateFb() and destroys a single framebuffer. The modeset.c example
 * used to do this directly in modeset_cleanup().
 * We simply unmap the buffer, remove the drm-FB and destroy the memory buffer.
 */
static void modesetDestroyFb( int fd
                            , struct ModesetBuf * buf )
{ struct drm_mode_destroy_dumb dreq;

  munmap(buf->map, buf->size); /* unmap buffer */
  drmModeRmFB( fd, buf->fb );    /* delete framebuffer */


  memset( &dreq, 0, sizeof(dreq)); /* delete dumb buffer */
  dreq.handle = buf->handle;
  drmIoctl( fd, DRM_IOCTL_MODE_DESTROY_DUMB, &dreq );
}

/**
 * modesetCreateFb() is mostly the same as before. Buf instead of writing the
 * fields of a ModesetDev, we now require a buffer pointer passed as @buf.
 * Please note that buf->width and buf->height are initialized by
 * modeset_setup_dev() so we can use them here.
 */

static void * modesetCreateFb( int fd, struct ModesetBuf * buf )
{ struct drm_mode_create_dumb creq;
  struct drm_mode_destroy_dumb dreq;
  struct drm_mode_map_dumb mreq;
  int ret;

/* create dumb buffer
 */
  memset( &creq, 0, sizeof(creq) );
  creq.width = buf->width;
  creq.height= buf->height;
  creq.bpp   = 32;

  if ( drmIoctl( fd, DRM_IOCTL_MODE_CREATE_DUMB, &creq ) >= 0 )
  { buf->size  = creq.size;
    buf->stride= creq.pitch;
    buf->handle= creq.handle;

    if (! drmModeAddFB( fd                       /* create framebuffer object for the dumb-buffer */
                      , buf->width, buf->height
                      , 24, 32, buf->stride
                      , buf->handle, &buf->fb ))
    { memset( &mreq, 0, sizeof(mreq));            /* prepare buffer for memory mapping */
      mreq.handle = buf->handle;

      if ( drmIoctl( fd, DRM_IOCTL_MODE_MAP_DUMB, &mreq) >= 0 )  // fprintf(stderr, "cannot map dumb buffer (%d): %m\n",errno);
      { if (( buf->map= mmap( 0, buf->size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, mreq.offset )))  /* perform actual memory mapping */
        { memset( buf->map, 0x00, buf->size); /* clear the framebuffer to 0 */
          return ( buf->map );
      } }

      drmModeRmFB( fd, buf->fb );
    }

    memset( &dreq, 0, sizeof(dreq) );
    dreq.handle = buf->handle;
    drmIoctl( fd, DRM_IOCTL_MODE_DESTROY_DUMB, &dreq );
  }

  return( NULL );
}



/**
 * modeset_find_crtc() stays the same.
 */
static int modesetFindCrtc( int fd
                          , drmModeRes *res
                          , drmModeConnector *conn
                          , struct ModesetDev *dev )
{ drmModeEncoder *enc;
  unsigned int i, j;
  long crtc;
  struct ModesetDev *iter;

  if (conn->encoder_id) /* first try the currently conected encoder+crtc */
  { enc = drmModeGetEncoder( fd, conn->encoder_id );
  }
  else
  { enc = NULL;
  }

  if (enc)
  { if (enc->crtc_id)
    { crtc = enc->crtc_id;

      for ( iter = modesetList
          ; iter
          ; iter = iter->next )
      { if ( iter->crtc == crtc )
        { crtc = -1;
          break;
      } }

     if (crtc >= 0)
     { drmModeFreeEncoder(enc);
       dev->crtc = crtc;
       return 0;
   } }

    drmModeFreeEncoder(enc);
  }

/**
 *   If the connector is not currently bound to an encoder or if the
 * encoder+crtc is already used by another connector (actually unlikely
 * but lets be safe), iterate all other available encoders to find a
 * matching CRTC.
 */

  for ( i = 0
      ; i < conn->count_encoders
      ; ++i )
  { if (!(enc= drmModeGetEncoder(fd, conn->encoders[i])))
    { fprintf(stderr, "cannot retrieve encoder %u:%u (%d): %m\n",i, conn->encoders[i], errno);
       continue;
    }


    for( j = 0         /* iterate all global CRTCs */
       ; j < res->count_crtcs
       ; ++j )
    { if (!(enc->possible_crtcs & (1 << j))) /* check whether this CRTC works with the encoder */
      {	continue;
      }

      crtc = res->crtcs[j]; /* check that no other device already uses this CRTC */
      for ( iter = modesetList
          ; iter
          ; iter = iter->next )
      { if (iter->crtc == crtc)
        { crtc = -1;
          break;
      } }

      if (crtc >= 0)  	/* we have found a CRTC, so save it and return */
      { drmModeFreeEncoder(enc);
        dev->crtc = crtc;
        return 0;
    } }

    drmModeFreeEncoder(enc);
  }

  fprintf(stderr, "cannot find suitable CRTC for connector %u\n",
  conn->connector_id);
  return -ENOENT;
}



/**
 * modeset_setup_dev() sets up all resources for a single device. It mostly
 * stays the same, but one thing changes: We allocate two framebuffers instead
 * of one. That is, we call modesetCreateFb() twice.
 * We also copy the width/height information into both framebuffers so
 * modesetCreateFb() can use them without requiring a pointer to ModesetDev.
 */
static int modesetSetupDev( int fd
                          , drmModeRes        * res
                          , drmModeConnector  * conn
                          , struct ModesetDev * dev )
{ int ret;

  if ( conn->connection != DRM_MODE_CONNECTED )  /* check if a monitor is connected */
  { fprintf(stderr, "ignoring unused connector %u\n",
    conn->connector_id);
    return -ENOENT;
  }

  if (conn->count_modes == 0)  	             /* check if there is at least one valid mode */
  { fprintf(stderr, "no valid mode for connector %u\n",
    conn->connector_id);
    return -EFAULT;
  }

  memcpy( &dev->mode /* copy the mode information into our device structure and into both	 buffers  */
        , &conn->modes[0]
        , sizeof(dev->mode));

  dev->bufs[ 0 ].width = conn->modes[ 0 ].hdisplay;
  dev->bufs[ 0 ].height= conn->modes[ 0 ].vdisplay;
  dev->bufs[ 1 ].width = conn->modes[ 0 ].hdisplay;
  dev->bufs[ 1 ].height= conn->modes[ 0 ].vdisplay;

  fprintf( stderr, "mode for connector %u is %ux%u\n"
         , conn->connector_id
         , dev->bufs[0].width, dev->bufs[0].height );

  if (( ret= modesetFindCrtc( fd, res, conn, dev )))  /* find a crtc for this connector */
  { fprintf(stderr, "no valid crtc for connector %u\n", conn->connector_id);
    return( ret );
  }

  modesetCreateFb( fd, dev->bufs + 0 ); /* create framebuffer #1 for this CRTC */
  modesetCreateFb( fd, dev->bufs + 1 ); /* create framebuffer #2 for this CRTC */

  if ( dev->bufs[ 0 ].map  && dev->bufs[ 1 ].map  )
  { return( 0 );
  }

  fprintf( stderr, "cannot create framebuffer for connector %u\n", conn->connector_id );

  if ( dev->bufs[ 0 ].map )
  { modesetDestroyFb( fd, dev->bufs + 0 );
  }
  if ( dev->bufs[ 1 ].map )
  { modesetDestroyFb( fd, dev->bufs + 1 );
  }

  return( -1 );
}


/**
 * Previously, we used the ModesetDev objects to hold buffer informations, too.
 * Technically, we could have split them but avoided this to make the
 * example simpler.
 * However, in this example we need 2 buffers. One back buffer and one front
 * buffer. So we introduce a new structure ModesetBuf which contains everything
 * related to a single buffer. Each device now gets an array of two of these
 * buffers.
 * Each buffer consists of width, height, stride, size, handle, map and fb-id.
 * They have the same meaning as before.
 *
 * Each device also gets a new integer field: front_buf. This field contains the
 * index of the buffer that is currently used as front buffer / scanout buffer.
 * In our example it can be 0 or 1. We flip it by using XOR:
 *   dev->front_buf ^= dev->front_buf
 *
 * Everything else stays the same.
 */

/**
 * modesetPrepare() stays the same.
 */
static int modesetPrepare( int fd )
{ drmModeRes *res;
  drmModeConnector *conn;
  unsigned int i;
  struct ModesetDev *dev;
  int ret;

/* retrieve resources
 */
  if (!(res = drmModeGetResources( fd )))
  { fprintf(stderr, "cannot retrieve DRM resources (%d): %m\n",	errno );
    return( -errno );
  }

  for ( i = 0                       	/* iterate all connectors */
      ; i < res->count_connectors
      ; i ++ )
  { conn= drmModeGetConnector( fd, res->connectors[i]); /* get information for each connector */

    if (!conn)
    { fprintf(stderr, "cannot retrieve DRM connector %u:%u (%d): %m\n",	i, res->connectors[i], errno);
      continue;
    }

    dev= CALLOC(sizeof(*dev)); /* create a device structure */
    dev->conn = conn->connector_id;

    if ((ret = modesetSetupDev( fd, res, conn, dev)))  /* call helper function to prepare this connector */
    {
      if (ret != -ENOENT)
      { errno = -ret;
        fprintf(stderr, "cannot setup device for connector %u:%u (%d): %m\n",	i, res->connectors[i], errno);
      }

      FREE( dev );
      drmModeFreeConnector( conn );
      continue;
    }

    drmModeFreeConnector( conn ); /* free connector data and link device into global list */
    dev->next = modesetList;
    modesetList = dev;
  }

  drmModeFreeResources( res ); /* free resources again */
  return( 0 );
}



/**
 * modeset_cleanup() stays the same as before. But it now calls
 * modesetDestroyFb() instead of accessing the framebuffers directly.
 */
static void modesetCleanup( int fd )
{ struct ModesetDev *iter;

  while (modesetList)    /* remove from global list */
  { iter = modesetList;
    modesetList = iter->next;

    drmModeSetCrtc( fd     /* restore saved CRTC configuration */
                  , iter->savedCrtc->crtc_id
                  , iter->savedCrtc->buffer_id
                  , iter->savedCrtc->x
                  , iter->savedCrtc->y
                  , &iter->conn
                  , 1
                  , &iter->savedCrtc->mode );

    drmModeFreeCrtc( iter->savedCrtc );

    modesetDestroyFb( fd, iter->bufs+1 );  /* destroy framebuffers */
    modesetDestroyFb( fd, iter->bufs+0 );
    FREE( iter );  /* free allocated memory */
} }


/** perform actual modesetting on each found connector+CRTC
 */
static int  modesetConnector( int fd )
{ struct ModesetDev * iter= modesetList;
  struct ModesetBuf * buf;
  int errors= 0;

  while( iter )
  { iter->savedCrtc= drmModeGetCrtc( fd, iter->crtc );
    if ( drmModeSetCrtc( fd
                       , iter->crtc
                       , iter->bufs->fb
                       , 0, 0
                       , &iter->conn, 1
                       , &iter->mode))
    { errors ++ ;
    }
    iter= iter->next;
  }

  return( errors );
}


static int drmInitialize( Nsfb * nsfb )
{ struct ModesetDev * iter;
  struct ModesetBuf * buf;
  int ret= -3;

  if ( modesetOpen( &nsfb->surfaceRtns->fd, DRM_NAME ))      /* open the DRM device */
  { return( -1 );
  }

  if ( modesetPrepare( nsfb->surfaceRtns->fd ))      /* prepare all connectors and CRTCs */
  { return( -2 );
  }

/* perform actual modesetting on each found connector+CRTC
 */
  for ( iter= modesetList
      ; iter
      ; iter= iter->next )
  { iter->savedCrtc= drmModeGetCrtc( nsfb->surfaceRtns->fd, iter->crtc );

    if ( drmModeSetCrtc( nsfb->surfaceRtns->fd, iter->crtc
                       , iter->bufs->fb
                       , 0, 0
                       , &iter->conn, 1, &iter->mode ) )
    { fprintf( stderr, "cannot set CRTC for connector %u (%d): %m\n"
             , iter->conn, errno);
      continue;
    }

    nsfb->panCount= 0;
    nsfb->pan=    iter->bufs[ 1 ].map;
    nsfb->loc=    iter->bufs[ 0 ].map;
    nsfb->width=  iter->bufs[ 0 ].width;
    nsfb->height= iter->bufs[ 0 ].height;
    nsfb->loclen=
    nsfb->panlen= iter->bufs[ 0 ].stride;
    nsfb->buflen= iter->bufs[ 0 ].size;
    ret= 0;

puts( "*****************************" );

  printf( "PAN %x %X %d %d %d %d \n"
        , nsfb->pan, nsfb->loc
        , iter->bufs[ 0 ].height
        , iter->bufs[ 0 ].width
        , iter->bufs[ 0 ].stride
        , iter->bufs[ 0 ].size
        , nsfb->panlen
        , nsfb->width
        , nsfb->height );

  }

  return( ret );
}

static int drmFinalise( Nsfb * nsfb )
{ modesetCleanup( nsfb->surfaceRtns->fd );
}


static int drmPan( Nsfb * nsfb, int mode )
{ struct ModesetDev * iter;

  printf( "DRM PAN %d panCount %d \n", nsfb->panCount, mode );

  for( iter= modesetList
     ; iter
     ; iter= iter->next )
  { if (( drmModeSetCrtc( nsfb->surfaceRtns->fd
                        , iter->crtc
                        , iter->bufs[ nsfb->panCount & 1 ].fb
                        , 0, 0
                        , &iter->conn, 1, &iter->mode ) ))
    { fprintf( stderr, "cannot flip CRTC for connector %u (%d): %m\n", iter->conn, errno);
    }

    nsfb->pan= iter->bufs[ nsfb->panCount & 1 ].map;
    nsfb->panCount++;
    nsfb->loc= iter->bufs[ nsfb->panCount & 1 ].map;
  }
  return( nsfb->panCount & 1 );
}


NsfbSurfaceRtns drmRtns=
{ .type= NSFB_SURFACE_DRM
, .name= "drm"

, .initialise= drmInitialize
, .finalise  = drmFinalise
, .claim     = NULL
, .update    = NULL
, .cursor    = NULL
, .geometry  = NULL
, .pan       = drmPan

, .dataSize  = sizeof( drmRtns )

};

NSFB_SURFACE_DEF( drmRtns  )



