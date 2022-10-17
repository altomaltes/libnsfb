/*
 * uterm - Linux User-Space Terminal
 *
 * Copyright (c) 2011-2013 David Herrmann <dh.herrmann@googlemail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/*
 * DRM Video backend
 */

#define EGL_EGLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <errno.h>
#include <fcntl.h>
#include <gbm.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
#include "eloop.h"
#include "shl_log.h"
#include "shl_misc.h"
#include "static_gl.h"
#include "utermDrm_shared_internal.h"
#include "utermDrm3d_internal.h"
#include "utermVideo.h"
#include "utermVideo_internal.h"

#define LOG_SUBSYSTEM "utermDrm3d_video"

static int displayInit( struct utermDisplay *disp )
{ int ret;

  struct utermDrm3dDisplay *d3d= calloc(sizeof(*d3d),1);

  if ( !d3d )
  { return( -ENOMEM );
  }

  if ((ret = utermDrmDisplayInit(disp, d3d)))
  { free(d3d);
    return ret;
  }

  return 0;
}

static void displayDestroy(struct utermDisplay *disp)
{ free( utermDrmDisplayGetData(disp) );
  utermDrmDisplayDestroy( disp );
}

static void boDestroyEvent( struct gbm_bo *bo, void *data )
{ struct utermDrm3dRb  * rb = data;
  struct utermDrmVideo * vdrm;

  if ( rb )
  { vdrm = rb->disp->video->data;
    drmModeRmFB(vdrm->fd, rb->fb);
    free(rb);
} }

static struct utermDrm3dRb *bo_to_rb( struct utermDisplay *disp
                                    , struct gbm_bo *bo )
{ struct utermDrm3dRb *rb = gbm_boGet_user_data(bo);
  struct utermVideo *video = disp->video;
  struct utermDrmVideo *vdrm = video->data;
  int ret;
  unsigned int stride, handle, width, height;;

  if ( !rb )
  { rb = calloc(sizeof(*rb),1);
    if ( !rb )
    { logError("cannot allocate memory for render buffer (%d): %m",  errno);
      return( NULL );
    }
   rb->disp = disp;
    rb->bo = bo;

#ifdef BUILD_HAVE_GBM_BOGet_PITCH
    stride = gbm_boGet_pitch(rb->bo);
#else
    stride = gbm_boGet_stride(rb->bo);
#endif
    handle = gbm_boGet_handle(rb->bo).u32;
    width = gbm_boGetWidth(rb->bo);
    height = gbm_boGetHeight(rb->bo);

    if (( ret = drmModeAddFB( vdrm->fd
                            , width, height
                            , 24, 32, stride
                            , handle, &rb->fb)))
    { logErr("cannot add drm-fb (%d): %m", errno);
      free(rb);
      return NULL;
    }

    gbm_bo_set_user_data(bo, rb, boDestroyEvent);
  }
  return( rb );
}

static int displayActivate( struct utermDisplay * disp
                          , struct UtermMode    * mode )
{ struct utermVideo        * video = disp->video;
  struct utermDrmVideo     * vdrm;
  struct utermDrm3d_video  * v3d;
  struct utermDrmDisplay   * ddrm = disp->data;
  struct utermDrm3dDisplay * d3d = utermDrmDisplayGetData(disp);
  int ret;
  struct gbm_bo *bo;
  drmModeModeInfo *minfo;

  if ( !mode )
  { return ( -EINVAL );
  }

  vdrm = video->data;
  v3d = utermDrmVideoGetData(video);
  minfo = utermdrmModeGet_info(mode);
  logInfo("activating display %p to %ux%u", disp,minfo->hdisplay, minfo->vdisplay);

  if (( ret = utermDrmDisplayActivate(disp, vdrm->fd)))
  { return( ret );
  }

  d3d->current = NULL;
  d3d->next = NULL;
  disp->current_mode = mode;

  d3d->gbm = gbm_surfaceCreate( v3d->gbm, minfo->hdisplay
                              , minfo->vdisplay
                              , GBM_FORMAT_XRGB8888
                              , GBM_BO_USE_SCANOUT | GBM_BO_USE_RENDERING );
  if ( !d3d->gbm )
  { logError("cannot create gbm surface (%d): %m", errno);
    ret = -EFAULT;
    goto err_saved;
  }

  d3d->surface= eglCreateWindowSurface( v3d->disp, v3d->conf
                                     , (EGLNativeWindowType)d3d->gbm
                                     , NULL);
  if (d3d->surface == EGL_NO_SURFACE)
  { logError("cannot create EGL window surface");
    ret = -EFAULT;
    goto err_gbm;
  }

  if (!eglMakeCurrent(v3d->disp, d3d->surface, d3d->surface,
			    v3d->ctx))
  { logError("cannot activate EGL context");
    ret = -EFAULT;
    goto err_surface;
  }

  glClearColor(0, 0, 0, 0);
  glClear(GL_COLORBuffer_BIT);
  if (!eglSwapBuffers(v3d->disp, d3d->surface))
  { logError("cannot swap buffers");
    ret = -EFAULT;
    goto err_noctx;
   }

  bo = gbm_surface_lock_frontBuffer(d3d->gbm);
  if (!bo)
  { logError("cannot lock front buffer during creation");
    ret = -EFAULT;
    goto err_noctx;
  }

  d3d->current = bo_to_rb(disp, bo);
  if (!d3d->current)
  { logError("cannot lock front buffer");
    ret = -EFAULT;
    goto err_bo;
  }

  ret= drmModeSetCrtc(vdrm->fd, ddrm->crtcId, d3d->current->fb,
			     0, 0, &ddrm->connId, 1, minfo);
  if (ret)
  { logErr("cannot set drm-crtc");
    ret = -EFAULT;
    goto err_bo;
  }

  disp->flags |= DISPLAY_ONLINE;
  return 0;

err_bo:
	gbm_surface_releaseBuffer(d3d->gbm, bo);
err_noctx:
	eglMakeCurrent(v3d->disp, EGL_NO_SURFACE, EGL_NO_SURFACE,
		       v3d->ctx);
err_surface:
	eglDestroySurface(v3d->disp, d3d->surface);
err_gbm:
	gbm_surfaceDestroy(d3d->gbm);
err_saved:
	disp->current_mode = NULL;
	utermDrmDisplayDeactivate(disp, vdrm->fd);
	return ret;
}

static void displayDeactivate( struct utermDisplay *disp )
{ struct utermDrm3dDisplay *d3d = utermDrmDisplayGetData(disp);
  struct utermVideo *video = disp->video;
  struct utermDrmVideo *vdrm;
  struct utermDrm3d_video *v3d;

  logInfo("deactivating display %p", disp);

  vdrm = video->data;
  v3d = utermDrmVideoGetData(video);
  utermDrmDisplayDeactivate(disp, vdrm->fd);

  eglMakeCurrent( v3d->disp
                , EGL_NO_SURFACE, EGL_NO_SURFACE
                , v3d->ctx);

  eglDestroySurface( v3d->disp, d3d->surface );

  if (d3d->current)
  { gbm_surface_releaseBuffer( d3d->gbm
                             , d3d->current->bo );
    d3d->current = NULL;
  }
  if ( d3d->next )
  { gbm_surface_releaseBuffer(d3d->gbm,  d3d->next->bo);
    d3d->next = NULL;
  }

  gbm_surfaceDestroy(d3d->gbm);
  disp->current_mode = NULL;
}

int utermDrm3ddisplayUse( struct utermDisplay *disp, bool *opengl )
{ struct utermDrm3dDisplay *d3d = utermDrmDisplayGetData(disp);
  struct utermDrm3d_video *v3d;

  v3d = utermDrmVideoGetData(disp->video);

  if (!eglMakeCurrent(v3d->disp, d3d->surface
                     ,d3d->surface, v3d->ctx))
  { logError("cannot activate EGL context");
	return( -EFAULT );
  }

  if ( opengl )
  { *opengl = true;
  }

  return 0; 	/* TODO: lets find a way how to retrieve the current front buffer */
}

static int displaySwap(struct utermDisplay *disp, bool immediate)
{ int ret;
  struct gbm_bo *bo;
  struct utermDrm3dRb *rb;
  struct utermDrm3dDisplay *d3d = utermDrmDisplayGetData(disp);
  struct utermVideo *video = disp->video;
  struct utermDrm3d_video *v3d = utermDrmVideoGetData(video);

   if (!gbm_surface_has_freeBuffers(d3d->gbm))
		return( -EBUSY);

  if (!eglSwapBuffers(v3d->disp, d3d->surface))
  { logError("cannot swap EGL buffers (%d): %m", errno);
    return( -EFAULT );
  }

  bo = gbm_surface_lock_frontBuffer(d3d->gbm);
  if (!bo)
  { logError("cannot lock front buffer");
    return( -EFAULT );
  }

  rb = bo_to_rb(disp, bo);
  if (!rb)
  { logError("cannot lock front gbm buffer (%d): %m", errno);
    gbm_surface_releaseBuffer(d3d->gbm, bo);
    return( -EFAULT );
  }

  if (( ret = utermDrmDisplaySwap(disp, rb->fb, immediate)))
  { gbm_surface_releaseBuffer(d3d->gbm, bo);
    return ret;
  }

  if (d3d->next)
  { gbm_surface_releaseBuffer(d3d->gbm, d3d->next->bo);
    d3d->next = NULL;
  }

  if (immediate)
  { if (d3d->current)
	gbm_surface_releaseBuffer(d3d->gbm, d3d->current->bo);
    d3d->current = rb;
  }
  else
  { d3d->next = rb;
  }

  return 0;
}

static const struct displayOps drmDisplayOps =
{ .init = displayInit
, .destroy    = displayDestroy
, .activate   = displayActivate
, .deactivate = displayDeactivate
, .set_dpms   = utermDrmDisplaySetDpms
, .use        = utermDrm3ddisplayUse
, .getBuffers = NULL
, .swap       = displaySwap
, .blit       = utermDrm3dDisplayBlit
, .fakeBlendv = utermDrm3dDisplayFakeBlendv
, .fill = utermDrm3dDisplayFill
};

static void showDisplays(struct utermVideo *video)
{ int ret;
  struct utermDisplay *iter;
  struct shl_dlist *i;

  if (!videoIsAwake(video))
		return;

  shl_dlist_for_each(i, &video->displays)
  { iter = shl_dlist_entry(i, struct utermDisplay, list);

    if (!displayIsOnline(iter))
	continue;
    if (iter->dpms != UTERM_DPMS_ON)
	continue;

    ret = utermDrm3ddisplayUse(iter, NULL);
    if (ret)
    continue;

    glClearColor(0, 0, 0, 1);
    glClear(GL_COLORBuffer_BIT);
    displaySwap(iter, true);
} }

static void page_flip_handler(struct utermDisplay *disp)
{ struct utermDrm3dDisplay *d3d = utermDrmDisplayGetData(disp);

  if (d3d->next)
  { if (d3d->current)
     gbm_surface_releaseBuffer(d3d->gbm,
						   d3d->current->bo);
		d3d->current = d3d->next;
		d3d->next = NULL;
	}
}

static int videoInit( struct utermVideo *video, const char *node )
{ static const EGLint conf_att[] =
  { EGL_SURFACE_TYPE   , EGL_WINDOW_BIT
  , EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT
  , EGL_RED_SIZE       , 1
  , EGL_GREEN_SIZE     , 1
  , EGL_BLUE_SIZE      , 1
  , EGL_ALPHA_SIZE     , 0
  , EGL_NONE };

  static const EGLint ctx_att[] =
  { EGL_CONTEXT_CLIENT_VERSION, 2
  , EGL_NONE };

  const char *ext;
  int ret;
  EGLint major, minor, n;
  EGLenum api;
  EGLBoolean b;
  struct utermDrmVideo *vdrm;
  struct utermDrm3d_video *v3d;

  v3d = calloc(sizeof(*v3d), 1);
  if (!v3d)
  { return -ENOMEM;
  }

  ret= utermDrmVideoInit( video
                        , node
                        , &drmDisplayOps
                        , page_flip_handler
                        , v3d );
  if (ret)
  { goto err_free;
  }
  vdrm = video->data;

  logDebug("initialize 3D layer on %p", video);

  v3d->gbm = gbmCreate_device(vdrm->fd);
  if (!v3d->gbm)
  { logErr("cannot create gbm device for %s (permission denied)",		node);
    ret = -EFAULT;
    goto err_video;
   }

  v3d->disp = eglGetDisplay((EGLNativeDisplayType) v3d->gbm);

  if (v3d->disp == EGL_NODisplay)
  { logErr("cannot retrieve egl display for %s", node);
    ret = -EFAULT;
    goto err_gbm;
  }

  b = eglInitialize(v3d->disp, &major, &minor);
  if (!b)
  { logErr("cannot init egl display for %s", node);
    ret = -EFAULT;
    goto err_gbm;
  }

  logDebug( "EGL Init %d.%d", major, minor);
  logDebug( "EGL Version %s", eglQueryString(v3d->disp, EGL_VERSION));
  logDebug( "EGL Vendor %s" , eglQueryString(v3d->disp, EGL_VENDOR));

  ext = eglQueryString(v3d->disp, EGL_EXTENSIONS);
  logDebug("EGL Extensions %s", ext);

  if (!ext || !strstr(ext, "EGL_KHR_surfaceless_context"))
  { logErr("surfaceless opengl not supported");
    ret = -EFAULT;
    goto err_disp;
  }

  api = EGL_OPENGL_ES_API;
  if (!eglBindAPI(api))
  { logErr("cannot bind opengl-es api");
    ret = -EFAULT;
    goto err_disp;
  }

  b = eglChooseConfig(v3d->disp, conf_att, &v3d->conf, 1, &n);
  if (!b || n != 1)
  { logError("cannot find a proper EGL framebuffer configuration");
    ret = -EFAULT;
    goto err_disp;
  }

  v3d->ctx = eglCreateContext(v3d->disp, v3d->conf, EGL_NO_CONTEXT,
				    ctx_att);
  if (v3d->ctx == EGL_NO_CONTEXT)
  { logError("cannot create egl context");
    ret = -EFAULT;
    goto err_disp;
  }

  if (!eglMakeCurrent( v3d->disp
                     , EGL_NO_SURFACE, EGL_NO_SURFACE
                     , v3d->ctx))
  { logError("cannot activate surfaceless EGL context");
    ret = -EFAULT;
    goto err_ctx;
  }

  ext = (const char*)glGetString(GL_EXTENSIONS);
  if (ext && strstr((const char*)ext, "GL_EXT_unpack_subimage"))
    v3d->supports_rowlen = true;
  else
   logWarn( "your GL implementation does not support GL_EXT_unpack_subimage, rendering may be slower than usual" );

  return 0;

err_ctx:   eglDestroyContext(v3d->disp, v3d->ctx);
err_disp:  eglTerminate(v3d->disp);
err_gbm:   gbm_deviceDestroy(v3d->gbm);
err_video: utermDrmVideoDestroy(video);
err_free:
  free(v3d);
  return ret;
}

static void videoDestroy(struct utermVideo *video)
{ struct utermDrm3d_video *v3d = utermDrmVideoGetData(video);

  logInfo("free drm video device %p", video);

  if (!eglMakeCurrent(v3d->disp, EGL_NO_SURFACE, EGL_NO_SURFACE,
			    v3d->ctx))
  logError("cannot activate GL context during destruction");
  utermDrm3d_deinit_shaders(video);

  eglMakeCurrent(v3d->disp, EGL_NO_SURFACE, EGL_NO_SURFACE,
		       EGL_NO_CONTEXT);
  eglDestroyContext(v3d->disp, v3d->ctx);
  eglTerminate(v3d->disp);
  gbm_deviceDestroy(v3d->gbm);
  free(v3d);
  utermDrmVideoDestroy(video);
}

static int videoPoll(struct utermVideo *video)
{ return utermDrmVideoPoll(video);
}

static void videoSleep(struct utermVideo *video)
{ showDisplays(video);
  utermDrmVideoSleep(video);
}

static int videoWakeUp(struct utermVideo *video)
{ int ret;

  if (( ret= utermDrmVideoWakeUp(video)))
  { utermDrmVideo_armVt_timer(video);
    return ret;
  }

  showDisplays(video);
  return 0;
}

static const struct videoOps drmVideoOps =
{ .init = videoInit
, .destroy = videoDestroy
, .segfault = NULL, /* TODO: reset all saved CRTCs on segfault */
  .poll = videoPoll
, .sleep = videoSleep
, .wakeUp = videoWakeUp
};

static const struct utermVideo_module drm3d_module =
{ .ops = &drmVideoOps
};

PUBLIC const struct utermVideo_module *utermVideo_DRM3D = &drm3d_module;
