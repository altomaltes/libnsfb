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

struct ModesetBuf;
struct ModesetDev;


/*
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

struct ModesetBuf
{ dword width;
  dword height;
  dword stride;
  dword size;
  dword handle;
  byte *map;
  dword fb;
};

struct ModesetDev
{ struct ModesetDev *next;

 unsigned int front_buf;
 struct ModesetBuf bufs[2];

 drmModeModeInfo mode;
 dword conn;
 dword crtc;
 drmModeCrtc *saved_crtc;
};

static struct ModesetDev *modesetList = NULL;

/*
 * modeset_prepare() stays the same.
 */

static int modeset_prepare(int fd)
{ drmModeRes *res;
	 drmModeConnector *conn;
	 unsigned int i;
	 struct ModesetDev *dev;
	 int ret;

/* retrieve resources
 */
 	res = drmModeGetResources(fd);
 	if (!res)
  { fprintf(stderr, "cannot retrieve DRM resources (%d): %m\n",	errno );
	  	return -errno;
 	}


	 for ( i = 0                       	/* iterate all connectors */
      ; i < res->count_connectors
      ; ++i )
  { conn = drmModeGetConnector(fd, res->connectors[i]); /* get information for each connector */

		  if (!conn)
    { fprintf(stderr, "cannot retrieve DRM connector %u:%u (%d): %m\n",				i, res->connectors[i], errno);
  		 	continue;
 		 }

	  	dev = malloc(sizeof(*dev)); /* create a device structure */
	  	memset(dev, 0, sizeof(*dev));
	  	dev->conn = conn->connector_id;

 		ret = modeset_setup_dev(fd, res, conn, dev); /* call helper function to prepare this connector */
	 	if (ret)
   { if (ret != -ENOENT)
     { errno = -ret;
  		 		fprintf(stderr, "cannot setup device for connector %u:%u (%d): %m\n",		 			i, res->connectors[i], errno);
		 	 }
 			 free(dev);
	 		 drmModeFreeConnector(conn);
		 	 continue;
		 }

		 drmModeFreeConnector(conn); /* free connector data and link device into global list */
		 dev->next = modesetList;
		 modesetList = dev;
	 }


	 drmModeFreeResources( res ); /* free resources again */
	 return 0;
}

/*
 * modeset_setup_dev() sets up all resources for a single device. It mostly
 * stays the same, but one thing changes: We allocate two framebuffers instead
 * of one. That is, we call modesetCreateFb() twice.
 * We also copy the width/height information into both framebuffers so
 * modesetCreateFb() can use them without requiring a pointer to ModesetDev.
 */
static int modeset_setup_dev( int fd
                            , drmModeRes *res
                            , drmModeConnector *conn
                            ,  struct ModesetDev *dev )
{ int ret;


	 if (conn->connection != DRM_MODE_CONNECTED)  /* check if a monitor is connected */
  { fprintf(stderr, "ignoring unused connector %u\n",
			 conn->connector_id);
		  return -ENOENT;
	 }


	 if (conn->count_modes == 0)  	/* check if there is at least one valid mode */
  { fprintf(stderr, "no valid mode for connector %u\n",
			 conn->connector_id);
	  	return -EFAULT;
 	}



	 memcpy( &dev->mode /* copy the mode information into our device structure and into both	 buffers  */
        , &conn->modes[0]
        , sizeof(dev->mode));
 	dev->bufs[0].width = conn->modes[0].hdisplay;
	 dev->bufs[0].height= conn->modes[0].vdisplay;
	 dev->bufs[1].width = conn->modes[0].hdisplay;
	 dev->bufs[1].height= conn->modes[0].vdisplay;

	 fprintf(stderr, "mode for connector %u is %ux%u\n",
		conn->connector_id, dev->bufs[0].width, dev->bufs[0].height);



	 if (( ret= modesetFindCrtc(fd, res, conn, dev)))  	/* find a crtc for this connector */
  { fprintf(stderr, "no valid crtc for connector %u\n",
			 conn->connector_id);
		  return ret;
 	}

 	if ((ret= modesetCreateFb(fd, &dev->bufs[0])))  /* create framebuffer #1 for this CRTC */
  { fprintf(stderr, "cannot create framebuffer for connector %u\n",
			 conn->connector_id);
		  return ret;
  }

	 if ((ret= modesetCreateFb(fd, &dev->bufs[1]))) 	/* create framebuffer #2 for this CRTC */
  { fprintf(stderr, "cannot create framebuffer for connector %u\n",
		 	conn->connector_id);
	 	 modeset_destroy_fb(fd, &dev->bufs[0]);
	 	 return ret;
	 }

	return 0;
}

/*
 * modesetFindCrtc() stays the same.
 */

static int modesetFindCrtc( int fd
                           , drmModeRes *res
                           , drmModeConnector *conn
                           , struct ModesetDev *dev )
{ drmModeEncoder *enc;
	 unsigned int i, j;
	 int32_t crtc;
	 struct ModesetDev *iter;


	 if (conn->encoder_id) /* first try the currently conected encoder+crtc */
  { enc = drmModeGetEncoder(fd, conn->encoder_id);
  }
	 else
  { enc = NULL;
  }

	 if (enc)
  { if (enc->crtc_id)
    { crtc = enc->crtc_id;

   			for (iter = modesetList; iter; iter = iter->next)
      { if (iter->crtc == crtc)
        { crtc = -1;
     					break;
			  	}	}

   			if (crtc >= 0)
      { drmModeFreeEncoder(enc);
    				dev->crtc = crtc;
				    return 0;
			 }	}

  		drmModeFreeEncoder(enc);
	 }

	/* If the connector is not currently bound to an encoder or if the
	 * encoder+crtc is already used by another connector (actually unlikely
	 * but lets be safe), iterate all other available encoders to find a
	 * matching CRTC.
  */

	 for (i = 0; i < conn->count_encoders; ++i)
  { enc= drmModeGetEncoder(fd, conn->encoders[i]);

  		if (!enc)
    { fprintf(stderr, "cannot retrieve encoder %u:%u (%d): %m\n",				i, conn->encoders[i], errno);
   			continue;
	  	}


		for( j = 0         /* iterate all global CRTCs */
     ; j < res->count_crtcs
     ; ++j )
  { if (!(enc->possible_crtcs & (1 << j))) /* check whether this CRTC works with the encoder */
			 {	continue;
    }


			crtc = res->crtcs[j]; /* check that no other device already uses this CRTC */
			for (iter = modesetList; iter; iter = iter->next)
   { if (iter->crtc == crtc)
     { crtc = -1;
  					break;
			}	}


		  if (crtc >= 0)  	/* we have found a CRTC, so save it and return */
    { drmModeFreeEncoder(enc);
				  dev->crtc = crtc;
			  	return 0;
	  	}	}

		  drmModeFreeEncoder(enc);
	 }

 	fprintf(stderr, "cannot find suitable CRTC for connector %u\n",
		conn->connector_id);
	 return -ENOENT;
}

/*
 * modesetCreateFb() is mostly the same as before. Buf instead of writing the
 * fields of a ModesetDev, we now require a buffer pointer passed as @buf.
 * Please note that buf->width and buf->height are initialized by
 * modeset_setup_dev() so we can use them here.
 */

static int modesetCreateFb( int fd, struct ModesetBuf *buf )
{ struct drm_mode_create_dumb creq;
	 struct drm_mode_destroy_dumb dreq;
	 struct drm_mode_map_dumb mreq;
	 int ret;

/* create dumb buffer
 */
	 memset(&creq, 0, sizeof(creq));
 	creq.width = buf->width;
 	creq.height = buf->height;
 	creq.bpp = 32;
 	ret= drmIoctl( fd, DRM_IOCTL_MODE_CREATE_DUMB, &creq );

 	if (ret < 0)
  { fprintf(stderr, "cannot create dumb buffer (%d): %m\n",			errno);
  		return -errno;
	 }
 	buf->stride = creq.pitch;
 	buf->size = creq.size;
 	buf->handle = creq.handle;

	/* create framebuffer object for the dumb-buffer */
	 ret = drmModeAddFB(fd, buf->width, buf->height, 24, 32, buf->stride,
			   buf->handle, &buf->fb);
	 if (ret)
  { fprintf(stderr, "cannot create framebuffer (%d): %m\n",			errno);
  		ret = -errno;
		  goto err_destroy;
  }

	/* prepare buffer for memory mapping */
	 memset(&mreq, 0, sizeof(mreq));
	 mreq.handle = buf->handle;
	 ret = drmIoctl(fd, DRM_IOCTL_MODE_MAP_DUMB, &mreq);
	 if (ret)
  { fprintf(stderr, "cannot map dumb buffer (%d): %m\n",			errno);
  		ret = -errno;
		  goto err_fb;
	  }

	/* perform actual memory mapping */
	 buf->map= mmap( 0, buf->size, PROT_READ | PROT_WRITE, MAP_SHARED,  fd, mreq.offset);

 	if (buf->map == MAP_FAILED)
  { fprintf(stderr, "cannot mmap dumb buffer (%d): %m\n",	errno);
  		ret = -errno;
		  goto err_fb;
 	}


	 memset(buf->map, 0, buf->size); /* clear the framebuffer to 0 */
 	return 0;

err_fb:	drmModeRmFB(fd, buf->fb);

err_destroy:
	memset(&dreq, 0, sizeof(dreq));
	dreq.handle = buf->handle;
	drmIoctl(fd, DRM_IOCTL_MODE_DESTROY_DUMB, &dreq);
	return ret;
}

/*
 * modeset_destroy_fb() is a new function. It does exactly the reverse of
 * modesetCreateFb() and destroys a single framebuffer. The modeset.c example
 * used to do this directly in modesetCleanup().
 * We simply unmap the buffer, remove the drm-FB and destroy the memory buffer.
 */

static void modeset_destroy_fb( int fd
                              , struct ModesetBuf *buf )
{ struct drm_mode_destroy_dumb dreq;

	 munmap(buf->map, buf->size); /* unmap buffer */
	 drmModeRmFB(fd, buf->fb); 	/* delete framebuffer */


	 memset(&dreq, 0, sizeof(dreq)); /* delete dumb buffer */
	 dreq.handle = buf->handle;
	 drmIoctl(fd, DRM_IOCTL_MODE_DESTROY_DUMB, &dreq);
}

/*
 * modesetCleanup() stays the same as before. But it now calls
 * modeset_destroy_fb() instead of accessing the framebuffers directly.
 */

static void modesetCleanup(int fd)
{ struct ModesetDev *iter;

 	while (modesetList)  /* remove from global list */
  { iter = modesetList;
  		modesetList = iter->next;


		  drmModeSetCrtc( fd   		/* restore saved CRTC configuration */
                  , iter->saved_crtc->crtc_id
                  , iter->saved_crtc->buffer_id
                  , iter->saved_crtc->x
                  , iter->saved_crtc->y
                  , &iter->conn
                  , 1
                  , &iter->saved_crtc->mode );
  		drmModeFreeCrtc( iter->saved_crtc );

/* destroy framebuffers
 */
	  	modeset_destroy_fb( fd, iter->bufs + 1 );
	  	modeset_destroy_fb( fd, iter->bufs + 0 );

	 	 free(iter); 	/* free allocated memory */
} }


/*
 * main() also stays almost exactly the same as before. We only need to change
 * the way that we initially set the CRTCs. Instead of using the buffer
 * information from ModesetDev, we now use dev->bufs[iter->front_buf] to get
 * the current front-buffer and use this framebuffer for drmModeSetCrtc().
 */

int main(int argc, char **argv)
{ int ret, fd;
	 const char *card;
	 struct ModesetDev *iter;
	 struct ModesetBuf *buf;

	/* check which DRM device to open */
	 if (argc > 1)
		{ card = argv[1];
  }
 	else
		{ card = "/dev/dri/card0";
  }

	 fprintf(stderr, "using card '%s'\n", card);


 	ret = modesetOpen(&fd, card); /* open the DRM device */
	 if (ret)
		{ goto out_return;
  }


	 ret = modeset_prepare(fd); /* prepare all connectors and CRTCs */
	 if (ret)
		{ goto out_close;
  }

/* perform actual modesetting on each found connector+CRTC
 */

  for( iter = modesetList
     ; iter
     ; iter = iter->next )
  { iter->saved_crtc = drmModeGetCrtc(fd, iter->crtc);
  		buf = &iter->bufs[iter->front_buf];
		  ret = drmModeSetCrtc(fd, iter->crtc, buf->fb, 0, 0,  &iter->conn, 1, &iter->mode);
  		if (ret)
		 	{ fprintf(stderr, "cannot set CRTC for connector %u (%d): %m\n",				iter->conn, errno);
  }	}

	 modesetDraw(    fd );  /* draw some colors for 5seconds */
	 modesetCleanup( fd ); 	/* cleanup everything */

	 ret = 0;

  out_close:	close(fd);

  out_return: if (ret)
  { errno = -ret;
   	fprintf(stderr, "modeset failed with error %d: %m\n", errno);
  }
  else
  { fprintf(stderr, "exiting\n");
  }
	 return ret;
}

/*
 * A short helper function to compute a changing color value. No need to
 * understand it.
 */

static byte nextColor(bool *up, byte cur, unsigned int mod)
{ byte next;

	 next = cur + (*up ? 1 : -1) * (rand() % mod);
	 if ((*up && next < cur) || (!*up && next > cur))
  { *up = !*up;
  		next = cur;
	 }

	 return next;
}

/*
 * modesetDraw() is the place where things change. The render-logic is the same
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

static void modesetDraw(int fd)
{ byte r, g, b;
  bool r_up, g_up, b_up;
  unsigned int i, j, k, off;
  struct ModesetDev *iter;
  struct ModesetBuf *buf;
  int ret;

 srand(time(NULL));
 r = rand() % 0xff;
 g = rand() % 0xff;
 b = rand() % 0xff;
 r_up = g_up = b_up = true;

 for ( i = 0
      ; i < 50
      ; ++i)
  { r = nextColor( &r_up, r, 20 );
		  g = nextColor( &g_up, g, 10 );
	  	b = nextColor( &b_up, b,  5 );

 		for (iter = modesetList; iter; iter = iter->next)
   { buf = &iter->bufs[iter->front_buf ^ 1];
  			for (j = 0; j < buf->height; ++j)
     { for (k = 0; k < buf->width; ++k)
       { off = buf->stride * j + k * 4;
   					*(dword*)&buf->map[off] =
	     (r << 16) | (g << 8) | b;
		  	} }

			 ret = drmModeSetCrtc( fd
                        , iter->crtc
                       , buf->fb, 0, 0
                      ,  &iter->conn, 1, &iter->mode);
	   if (ret)
		   {	fprintf(stderr, "cannot flip CRTC for connector %u (%d): %m\n",					iter->conn, errno);
      }
  			 else
   {	iter->front_buf ^= 1;
   } }

		  usleep(100000);
} }

/*
 * This was a very short extension to the basic modesetting example that shows
 * how double-buffering is implemented. Double-buffering is the de-facto
 * standard in any graphics application so any other example will be based on
 * this. It is important to understand the ideas behind it as the code is pretty
 * easy and short compared to modeset.c.
 *
 * Double-buffering doesn't solve all problems. Vsync'ed page-flips solve most
 * of the problems that still occur, but has problems on it's own (see
 * modeset-vsync.c for a discussion).
 *
 * If you want more code, I can recommend reading the source-code of:
 *  - plymouth (which uses dumb-buffers like this example; very easy to understand)
 *  - kmscon (which uses libuterm to do this)
 *  - wayland (very sophisticated DRM renderer; hard to understand fully as it
 *             uses more complicated techniques like DRM planes)
 *  - xserver (very hard to understand as it is split across many files/projects)
 *
 * Any feedback is welcome. Feel free to use this code freely for your own
 * documentation or projects.
 *
 *  - Hosted on http://github.com/dvdhrm/docs
 *  - Written by David Rheinsberg <david.rheinsberg@gmail.com>
 */

