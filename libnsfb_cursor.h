/*
 * Copyright 2009 Vincent Sanders <vince@simtec.co.uk>
 *
 * This file is part of libnsfb, http://www.netsurf-browser.org/
 * Licenced under the MIT License,
 *                http://www.opensource.org/licenses/mit-license.php
 *
 * This is the exported interface for the libnsfb graphics library.
 */

#ifndef _LIBnsfbCursor_H
#define _LIBnsfbCursor_H 1

/** Initialise the cursor.
 */
ANSIC bool nsfbCursorInit( NsfbSurfaceRtns * );

/** Set cursor parameters.
 *
 * Set a cursor bitmap, the cursor will be shown at the location set by
 * nsfbCursor_loc_set. The pixel data may be referenced untill the cursor
 * is altered or cleared
 *
 * @param nsfb       The frambuffer context
 * @param pixel      The cursor bitmap data
 * @param bmp_width  The width of the cursor bitmap
 * @param bmp_height The height of the cursor bitmap
 * @param bmp_stride The cursor bitmap's row stride
 * @param hotspot_x  Coordinate within cursor image to place over cursor loc
 * @param hotspot_y  Coordinate within cursor image to place over cursor loc
 *
 * (hot_spot_x, hot_spot_y) is from top left.  (0, 0) means top left pixel of
 * cursor bitmap is to be rendered over the cursor location.
 */

 /** Input event, must match the head of nomad event (avoid dependencies)
 */
typedef struct NsfbEventStruct
{ int guess;             /* type of device */
  int type;              /* type of event */

  unsigned long stamp;
  unsigned long serial;
  int           mod;

  short x, y, z, but;    /* Linear              */
  short a, b, g, key;

} NsfbEvent;



typedef struct NsfbCursorSt NsfbCursor;

ANSIC bool nsfbCursorSet( NsfbSurfaceRtns * , IcoRec * );

ANSIC int nsfbSpreadEvent( NsfbSurfaceRtns *
                         , NsfbEvent       * );

/** Set cursor location.
 *
 * @param nsfb The frambuffer context.
 * @param loc The location of the cursor
 */
ANSIC NsfbPoint * nsfbCursorLocSet( NsfbSurfaceRtns *, int x, int y );

/** get the cursor location
 */
ANSIC NsfbPoint * nsfbCursorLocGet( NsfbSurfaceRtns * );

/** Plot the cursor saving the image underneath.
 */
ANSIC bool nsfbCursorPlot( NsfbSurfaceRtns * );

/** Clear the cursor restoring the image underneath
 */
ANSIC bool nsfbCursorClear( NsfbSurfaceRtns * );

/** Hide the corsor from screeen
 */
ANSIC int nsfbCursorHide( NsfbSurfaceRtns * );

/** Destroy the cursor
  */
ANSIC bool nsfbCursorDestroy( NsfbCursor * cursor );


/** Hardcoded cursors
  */
extern IcoRec cursorDefault;





#endif
