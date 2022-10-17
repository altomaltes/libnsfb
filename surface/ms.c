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

#define dword unsigned int
#define byte  unsigned char

struct modeset_buf;
struct modeset_dev;


static void modesetDraw(int fd);

/*
 * main() also stays almost exactly the same as before. We only need to change
 * the way that we initially set the CRTCs. Instead of using the buffer
 * information from modeset_dev, we now use dev->bufs[iter->front_buf] to get
 * the current front-buffer and use this framebuffer for drmModeSetCrtc().
 */

int main(int argc, char **argv)
{ int ret, fd;
	 const char *card;
	 struct modeset_dev *iter;
	 struct modeset_buf *buf;

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


	 ret = modesetPrepare(fd); /* prepare all connectors and CRTCs */
	 if (ret)
		{ goto out_close;
  }

  
         modesetCrt(     fd );
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
	 struct modeset_dev *iter;
	 struct modeset_buf *buf;
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

 		for (iter = modeset_list; iter; iter = iter->next)
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

