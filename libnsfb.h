/*
 * Copyright 2009 Vincent Sanders <vince@simtec.co.uk>
 *
 * This file is part of libnsfb, http://www.netsurf-browser.org/
 * Licenced under the MIT License,
 *                http://www.opensource.org/licenses/mit-license.php
 *
 * This is the exported interface for the libnsfb graphics library.
 */

#ifndef _LIBNSFB_H
#define _LIBNSFB_H 1

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef __cplusplus
  #define PUBLIC extern "C"
  #define ANSIC extern "C"
#else
  #define PUBLIC
  #define ANSIC
#endif

#define ARGUSED __attribute__((unused))
#define UNUSED(x) ((x) = (x))


typedef struct NsfbPalette nsfb_palette_t;
typedef struct nsfbCursor_s  nsfbCursor_t;
typedef struct nsfb_s        nsfb_t;
typedef struct nsfb_event_s  nsfb_event_t;

/** co-ordinate for plotting operations */
typedef struct nsfb_point_s
{ int x;
  int y;
} nsfb_point_t;

/** bounding box for plotting operations */
typedef struct nsfb_bbox_s
{ int x0; int y0;
  int x1; int y1;
} nsfb_bbox_t;

/** The type of framebuffer surface.
 */
enum nsfb_type_e
{ NSFB_SURFACE_NONE = 0x0000   /** < No surface                */
, NSFB_SURFACE_RAM  = 0x0001   /** < RAM surface               */
, NSFB_SURFACE_SDL  = 0x0002   /** < SDL surface               */
, NSFB_SURFACE_LINUX= 0x0004   /** < Linux framebuffer surface */
, NSFB_SURFACE_VNC  = 0x0008   /** < VNC surface               */
, NSFB_SURFACE_ABLE = 0x0010   /** < ABLE framebuffer surface  */
, NSFB_SURFACE_X    = 0x0020   /** < X windows surface         */
, NSFB_SURFACE_WL   = 0x0040   /** < Wayland surface           */
, NSFB_SURFACE_FBUFF= 0x0080   /** < Internal framebuffer      */
, NSFB_SURFACE_DRM  = 0x0100   /** borrowed from <david.rheinsberg@gmail.com>  */
, NSFB_SURFACE_EGL  = 0x0200   /** borrowed from <david.rheinsberg@gmail.com>   */

, NSFB_SURFACE_EOF  = 0x0400   /** Marks the end of the list   */

/* Free slots
 */


, NSFB_SURFACE_0800 = 0x0800   /** Future next one  */
, NSFB_SURFACE_1000 = 0x1000   /** Future next one  */
, NSFB_SURFACE_2000 = 0x2000   /** Future next one  */
, NSFB_SURFACE_4000 = 0x4000   /** Future next one  */
, NSFB_SURFACE_8000 = 0x8000   /** Future next one  */

/* like this till 32ª bit */

};

enum nsfb_panning_e
{ NSFB_PAN_START
, NSFB_PAN_SWITCH
, NSFB_PAN_BSTORE
, NSFB_PAN_DUMP
};

enum nsfb_rotate_e
{ NSFB_ROTATE_NORTH= 0x00
, NSFB_ROTATE_EAST = 0x10
, NSFB_ROTATE_SOUTH= 0x20
, NSFB_ROTATE_WEST = 0x30
};


#define GETFMITEM( l, o, i  ) (( l | ( o << 1 )) << ( i << 3 ))


enum nsfb_format_e
{ NSFB_FMT_ANY     = 0           /* No specific format - use surface default */
, NSFB_FMT_XBGR8888= 0x30081828  /* 32bpp Blue Green Red */
, NSFB_FMT_XRGB8888= 0x30281808  /* 32bpp Red Green Blue */
, NSFB_FMT_ABGR8888= 0x38081828  /* 32bpp Alpha Blue Green Red */
, NSFB_FMT_ARGB8888= 0x38281808  /* 32bpp Alpha Red Green Blue */
, NSFB_FMT_RGB888  = 0x00081828  /* 24 bpp Alpha Red Green Blue */
, NSFB_FMT_BRB888  = 0x00281808  /* 24 bpp Alpha Red Green Blue */
, NSFB_FMT_ARGB1555= 0x31251505  /* 16 bpp 555 */
, NSFB_FMT_RGB565  = 0x00251505  /* 16 bpp 565 */
, NSFB_FMT_I8      = 1           /* 8bpp indexed */
, NSFB_FMT_I4                    /* 4bpp indexed */
, NSFB_FMT_I1                    /* black and white */

, NSFB_FMT_MASK    = 0x3FFFFFFF  /* JACS, format mask */

, NSFB_FMT_ACTIVATE= 0x40000000  /* JACS, add flags */
};

#define APPLY_ROTATE1( holder, xd, yd, xs, ys ) \
{ switch( holder->rotate )                                      \
  { case NSFB_ROTATE_NORTH: xd=               xs; yd=                ys; break; \
    case NSFB_ROTATE_WEST:  xd=               ys; yd= holder->height- xs; break; \
    case NSFB_ROTATE_SOUTH: xd= holder->width-xs; yd= holder->height-ys; break; \
    case NSFB_ROTATE_EAST : xd= holder->width-ys; yd=                xs; break; \
} }



/** Select frontend type from a name.
 *
 * @param name The name to select a frontend.
 * @return The surface type or NSFB_SURFACE_NONE if frontend with specified
 *         name was not available
 */
PUBLIC enum nsfb_type_e nsfbTypeFromName(const char *name);

/** Create a nsfb context.
 *
 * This creates a framebuffer surface context.
 *
 * @param surface_type The type of surface to create a context for.
 */
PUBLIC nsfb_t *nsfbNew(const enum nsfb_type_e surface_type);

/** Initialise selected surface context.
 *
 * @param nsfb The context returned from ::nsfbInit
 */
PUBLIC    int   nsfbInit( nsfb_t * );
PUBLIC nsfb_t * nsfbOpen( const char * mode ); // JACS

/** Free nsfb context.
 *
 * This shuts down and releases all resources associated with an nsfb context.
 *
 * @param nsfb The context returned from ::nsfbNew to free
 */
PUBLIC int nsfbFree(nsfb_t *nsfb);

/** Claim an area of screen to be redrawn.
 *
 * Informs the nsfb library that an area of screen will be directly
 * updated by the user program. This is neccisarry so the library can
 * ensure the soft cursor plotting is correctly handled. After the
 * update has been perfomed ::nsfb_update should be called.
 *
 * @param box The bounding box of the area which might be altered.
 */
PUBLIC int nsfbClaim(nsfb_t *nsfb, nsfb_bbox_t *box);

/** Update an area of screen which has been redrawn.
 *
 * Informs the nsfb library that an area of screen has been directly
 * updated by the user program. Some surfaces only show the update on
 * notification. The area updated does not neccisarrily have to
 * corelate with a previous ::nsfbClaim bounding box, however if the
 * redrawn area is larger than the claimed area pointer plotting
 * artifacts may occour.
 *
 * @param box The bounding box of the area which has been altered.
 */
PUBLIC int nsfb_update(nsfb_t *nsfb, nsfb_bbox_t *box);

/** Obtain the geometry of a nsfb context.
 *
 * @param width a variable to store the framebuffer width in or NULL
 * @param height a variable to store the framebuffer height in or NULL
 * @param format a variable to store the framebuffer format in or NULL
 */
PUBLIC void * nsfbGetGeometry( nsfb_t *nsfb
                               , int *width, int    * height
                               , enum nsfb_format_e * format );

/** Alter the geometry of a surface
 *
 * @param nsfb The context to alter.
 * @param width The new display width.
 * @param height The new display height.
 * @param format The desired surface format.
 */
PUBLIC int nsfbSetGeometry( nsfb_t *nsfb
                            , int width, int height
                            , enum nsfb_format_e format );

PUBLIC int nsfb_set_pan( nsfb_t * nsfb, int type );


/** Set parameters a surface
 *
 * Some surface types can take additional parameters for
 * attributes. For example the linux surface uses this to allow the
 * setting of a different output device
 *
 * @param nsfb The surface to alter.
 * @param parameters The parameters for the surface.
 */
PUBLIC int nsfb_set_parameters(nsfb_t *nsfb, const char *parameters);

/** Obtain the buffer memory base and stride.
 *
 * @param nsfb The context to read.
 */
PUBLIC int nsfb_get_buffer(nsfb_t *nsfb, unsigned char **ptr, int *linelen);

/** Dump the surface to fd in PPM format
 */
PUBLIC bool nsfb_dump(nsfb_t *nsfb, int fd);

PUBLIC int vtswitchopen(  );





#endif

/*
 * Local variables:
 *  c-basic-offset: 4
 *  tab-width: 8
 * End:
 */
