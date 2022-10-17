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
#include "static_gl.h"
#include "utermDrm_shared_internal.h"
#include "utermDrm3d_internal.h"
#include "utermVideo.h"
#include "utermVideo_internal.h"

#define LOG_SUBSYSTEM "utermDrm3d_render"

extern const char * gl_staticFill_vert;
extern const char * gl_staticFill_frag;
extern const char * gl_staticBlend_vert;
extern const char * gl_staticBlend_frag;
extern const char * gl_staticBlit_vert;
extern const char * gl_staticBlit_frag;

static int initShaders( struct utermVideo *video )
{ struct utermDrm3d_video *v3d = utermDrmVideoGetData(video);
  int ret;

  char *fill_attr[] = { "position", "color" };
  char *blend_attr[]= { "position", "texture_position" };
  char *blit_attr[] = { "position", "texture_position" };

  if ( v3d->sinit == 1 )
  { return( -EFAULT );
  }
  else if (v3d->sinit == 2)
  { return 0;
  }

  v3d->sinit = 1;

  ret= gl_shader_new( &v3d->fill_shader
                    , gl_staticFill_vert
                    , gl_staticFill_frag
                    , fill_attr, 2, log_llog, NULL );
  if (ret)
  { return ret;
  }

  v3d->uniFill_proj = gl_shaderGet_uniform( v3d->fill_shader
                                          , "projection" );
  ret= gl_shader_new( &v3d->blend_shader
                    , gl_staticBlend_vert
                    , gl_staticBlend_frag
                    , blend_attr, 2, log_llog,   NULL);
  if (ret)
  { return ret;
  }

  v3d->uniBlend_proj = gl_shaderGet_uniform( v3d->blend_shader, "projection" );
  v3d->uniBlend_tex  = gl_shaderGet_uniform( v3d->blend_shader, "texture"    );
  v3d->uniBlend_fgcol= gl_shaderGet_uniform( v3d->blend_shader, "fgcolor"    );
  v3d->uniBlend_bgcol= gl_shaderGet_uniform( v3d->blend_shader, "bgcolor"    );

  ret= gl_shader_new( &v3d->blit_shader
                    , gl_staticBlit_vert
                    , gl_staticBlit_frag
                    , blit_attr
                    , 2, log_llog, NULL );
  if (ret)
  { return ret;
  }

  v3d->uniBlit_proj= gl_shaderGet_uniform( v3d->blit_shader, "projection");
  v3d->uniBlit_tex = gl_shaderGet_uniform( v3d->blit_shader, "texture");

  gl_tex_new(&v3d->tex, 1);
  v3d->sinit = 2;

  return( 0 );
}

void utermDrm3d_deinit_shaders(struct utermVideo *video)
{ struct utermDrm3d_video *v3d = utermDrmVideoGetData(video);

  if (v3d->sinit == 0)
		{ return;
  }

  v3d->sinit = 0;
  gl_tex_free(&v3d->tex, 1);
  gl_shader_unref(v3d->blit_shader);
  gl_shader_unref(v3d->blend_shader);
  gl_shader_unref(v3d->fill_shader);
}

int utermDrm3dDisplayBlit( struct utermDisplay *disp
                         , const struct utermVideoBuffer *buf
                         , unsigned int x, unsigned int y )
{ struct utermDrm3d_video *v3d;
  unsigned int sw, sh, tmp, width, height, i;
  float mat[16];
  float vertices[6 * 2], texpos[6 * 2];
  int ret;
  byte *packed, *src, *dst;

  if (!buf || buf->format != UTERM_FORMAT_XRGB32)
  { return ( -EINVAL );
  }

  v3d = utermDrmVideoGetData(disp->video);
  ret = utermDrm3ddisplayUse(disp, NULL);
  if (ret)
  { return ret;
  }
  ret = init_shaders(disp->video);
  if (ret)
  { return ret;
  }

  sw= utermdrmModeGetWidth(  disp->current_mode );
  sh= utermdrmModeGetHeight( disp->current_mode );

  vertices[ 0 ]= -1.0; vertices[ 1 ]= -1.0; vertices[2] = -1.0;
  vertices[ 3 ]= +1.0; vertices[ 4 ]= +1.0; vertices[5] = +1.0;

  vertices[ 6 ]= -1.0; vertices[ 7 ]= -1.0; vertices[8] = +1.0;
  vertices[ 9 ]= +1.0; vertices[10 ]= +1.0; vertices[11] = -1.0;

  texpos[ 0 ]= 0.0; texpos[ 1 ]= 1.0; texpos[2] = 0.0;
  texpos[ 3 ]= 0.0; texpos[ 4 ]= 1.0; texpos[5] = 0.0;

  texpos[ 6 ]= 0.0; texpos[ 7 ]= 1.0; texpos[8] = 1.0;
  texpos[ 9 ]= 0.0; texpos[10 ]= 1.0; texpos[11] = 1.0;

  tmp = x + buf->width;

  if (tmp < x || x >= sw)
	 { return ( -EINVAL );
  }

  if (tmp > sw)
	 { width = sw - x;
  }
  else
	 { width = buf->width;
  }

  tmp = y + buf->height;

  if (tmp < y || y >= sh)
 	{ return ( -EINVAL );
  }

  if (tmp > sh)
  { height = sh - y;
  }
  else
  { height = buf->height;
  }

  glViewport(x, sh - y - height, width, height);
  glDisable(GLBlend);

  gl_shader_use( v3d->blit_shader);

  gl_m4_identity( mat );
  glUniformMatrix4fv(v3d->uniBlit_proj, 1, GL_FALSE, mat);

  glActiveTexture( GL_TEXTURE0 );
  glBindTexture(   GL_TEXTURE_2D, v3d->tex);
  glPixelStorei(   GL_UNPACK_ALIGNMENT, 1);

  if (v3d->supports_rowlen)
  { glPixelStorei(GL_UNPACK_ROW_LENGTH, buf->stride / 4);
    glTexImage2D( GL_TEXTURE_2D, 0, GL_BGRA_EXT, width, height, 0
                , GL_BGRA_EXT, GL_UNSIGNED_BYTE
                , buf->data );
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
  }
  else if (buf->stride == width)
  { glTexImage2D(GL_TEXTURE_2D, 0, GL_BGRA_EXT, width, height, 0,
    GL_BGRA_EXT, GL_UNSIGNED_BYTE, buf->data);
  }
  else
  { packed = malloc(width * height);
    if (!packed)
      return -ENOMEM;

    src = buf->data;
    dst = packed;
    for (i = 0; i < height; ++i)
    { memcpy(dst, src, width * 4);
      dst += width * 4;
      src += buf->stride;
    }

    glTexImage2D( GL_TEXTURE_2D, 0, GL_BGRA_EXT
                , width, height, 0
                , GL_BGRA_EXT, GL_UNSIGNED_BYTE, packed );

    free(packed);
  }

  glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
  glUniform1i(v3d->uniBlit_tex, 0);

  glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, 0, vertices);
  glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, 0, texpos);
  glEnableVertexAttribArray( 0);
  glEnableVertexAttribArray( 1);
  glDrawArrays( GL_TRIANGLES, 0, 6 );
  glDisableVertexAttribArray( 0);
  glDisableVertexAttribArray( 1);

  if (gl_has_error(v3d->blit_shader))
  { logWarning("GL error");
     return( -EFAULT );
  }

  return 0;
}

static int displayBlend( struct utermDisplay *disp
                       , const struct utermVideoBuffer *buf
                       , unsigned int x, unsigned int y
                       , byte fr, byte fg, byte fb
                       , byte br, byte bg, byte bb )
{ struct utermDrm3d_video *v3d;
  unsigned int sw, sh, tmp, width, height, i;
  float mat[16];
  float vertices[6 * 2], texpos[6 * 2], fgcol[3], bgcol[3];
  int ret;
  byte *packed, *src, *dst;

 if (!buf || buf->format != UTERM_FORMAT_GREY)
    return ( -EINVAL );

  v3d= utermDrmVideoGetData(disp->video);
  ret= utermDrm3ddisplayUse(disp, NULL);

  if (ret)
    return ret;
  ret = init_shaders(disp->video);
  if (ret)
    return ret;

  sw = utermdrmModeGetWidth(disp->current_mode);
  sh = utermdrmModeGetHeight(disp->current_mode);

  vertices[ 0 ] = -1.0;	vertices[ 1 ] = -1.0; vertices[ 2 ] = -1.0;
  vertices[ 3 ] = +1.0;	vertices[ 4 ] = +1.0; vertices[ 5 ] = +1.0;

  vertices[ 6 ] = -1.0;	vertices[7] = -1.0;  vertices[8] = +1.0;
  vertices[ 9 ] = +1.0;	vertices[10] = +1.0; vertices[11] = -1.0;

  texpos[0] = 0.0; texpos[1] = 1.0;	texpos[2] = 0.0;
  texpos[3] = 0.0; texpos[4] = 1.0;	texpos[5] = 0.0;

  texpos[6] = 0.0; texpos[7 ]= 1.0;	texpos[8] = 1.0;
  texpos[9] = 0.0; texpos[10]= 1.0; texpos[11] = 1.0;

  fgcol[0] = fr / 255.0; fgcol[1] = fg / 255.0;	fgcol[2] = fb / 255.0;
  bgcol[0] = br / 255.0; bgcol[1] = bg / 255.0;	bgcol[2] = bb / 255.0;

  tmp = x + buf->width;
if (tmp < x || x >= sw)
	return ( -EINVAL );
if (tmp > sw)
	width = sw - x;
else
width = buf->width;

  tmp = y + buf->height;
  if (tmp < y || y >= sh)
	return ( -EINVAL );
if (tmp > sh)
	height = sh - y;
else
	height = buf->height;

  glViewport(x, sh - y - height, width, height);
  glDisable(GLBlend);

  gl_shader_use(v3d->blend_shader);

  gl_m4_identity(mat);
  glUniformMatrix4fv(v3d->uniBlend_proj, 1, GL_FALSE, mat);

  glUniform3fv(v3d->uniBlend_fgcol, 1, fgcol);
  glUniform3fv(v3d->uniBlend_bgcol, 1, bgcol);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, v3d->tex);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  if (v3d->supports_rowlen)
  { glPixelStorei( GL_UNPACK_ROW_LENGTH, buf->stride);
    glTexImage2D(  GL_TEXTURE_2D, 0, GL_ALPHA
                , width, height, 0
                , GL_ALPHA, GL_UNSIGNED_BYTE, buf->data );
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
  }
  else if (buf->stride == width)
  { glTexImage2D( GL_TEXTURE_2D, 0, GL_ALPHA
                , width, height, 0
                , GL_ALPHA, GL_UNSIGNED_BYTE, buf->data );
  }
  else
  { packed = malloc(width * height);
    if (!packed)
        return -ENOMEM;

    src = buf->data;
    dst = packed;
    for (i = 0; i < height; ++i)
    { memcpy(dst, src, width);
      dst += width;
      src += buf->stride;
    }

    glTexImage2D( GL_TEXTURE_2D, 0
                , GL_ALPHA, width, height, 0
                , GL_ALPHA, GL_UNSIGNED_BYTE
                , packed );
    free(packed);
  }

  glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
  glUniform1i(v3d->uniBlend_tex, 0);

  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, vertices);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, texpos);
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  glDrawArrays( GL_TRIANGLES, 0, 6 );
  glDisableVertexAttribArray( 0 );
  glDisableVertexAttribArray( 1 );

  if (gl_has_error(v3d->blend_shader))
  { logWarning("GL error");
    return( -EFAULT );
  }

  return 0;
}

int utermDrm3dDisplayFakeBlendv( struct utermDisplay *disp
                               , const struct utermVideoBlend_req *req
                               , size_t num )
{ int ret;
  unsigned int i;

 if (!disp || !req)
	return ( -EINVAL );

 for (i = 0; i < num; ++i, ++req)
 { if (!req->buf)
 { continue;
   }

  ret= displayBlend( disp, req->buf
                   , req->x, req->y
                   , req->fr, req->fg, req->fb
                   , req->br, req->bg, req->bb );
  if (ret)
			return ret;
	}

  return 0;
}

int utermDrm3dDisplayFill( struct utermDisplay *disp
                         , byte r, byte g, byte b
                         , unsigned int x, unsigned int y
                         , unsigned int width, unsigned int height )
{ struct utermDrm3d_video *v3d;
  unsigned int sw, sh, tmp, i;
  float mat[16];
  float vertices[6 * 2], colors[6 * 4];
  int ret;

  v3d= utermDrmVideoGetData(disp->video);
  ret= utermDrm3ddisplayUse(disp, NULL);

  if (ret)
    return ret;
  ret = init_shaders(disp->video);
    if (ret)
 	return ret;

  sw = utermdrmModeGetWidth(disp->current_mode);
  sh = utermdrmModeGetHeight(disp->current_mode);

  for ( i = 0
      ; i < 6
      ; ++i )
  { colors[ i * 4 + 0 ] = r / 255.0;
    colors[ i * 4 + 1 ] = g / 255.0;
    colors[ i * 4 + 2 ] = b / 255.0;
    colors[ i * 4 + 3 ] = 1.0;
  }

  vertices[0] = -1.0;vertices[1] = -1.0;	vertices[2] = -1.0;
  vertices[3] = +1.0;	vertices[4] = +1.0;	vertices[5] = +1.0;

  vertices[6] = -1.0;	vertices[ 7] = -1.0;	vertices[ 8] = +1.0;
  vertices[9] = +1.0;	vertices[10] = +1.0;	vertices[11] = -1.0;

  tmp = x + width;
  if (tmp < x || x >= sw)
 	return ( -EINVAL );
  if (tmp > sw)
  	width = sw - x;
  tmp = y + height;
  if (tmp < y || y >= sh)
	return ( -EINVAL );
  if (tmp > sh)
	height = sh - y;

  glViewport(x, y, width, height);
  glDisable(GLBlend);

  gl_shader_use(v3d->fill_shader);
  gl_m4_identity(mat);
  glUniformMatrix4fv(v3d->uniFill_proj, 1, GL_FALSE, mat);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, vertices );
  glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, colors   );
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  glDrawArrays( GL_TRIANGLES, 0, 6);
  glDisableVertexAttribArray( 0 );
  glDisableVertexAttribArray( 1 );

   if (gl_has_error(v3d->fill_shader))
   { logWarning("GL error");
    return( -EFAULT );
   }

  return( 0 );
}
