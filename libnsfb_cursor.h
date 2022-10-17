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
PUBLIC bool nsfbCursorInit( nsfb_t * );

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
PUBLIC bool nsfbCursorSet( nsfb_t *nsfb, const nsfb_colour_t * pixel
                         , int bmp_width, int bmp_height, int bmp_stride
                         , int hotspot_x, int hotspot_y );

/** Set cursor location.
 *
 * @param nsfb The frambuffer context.
 * @param loc The location of the cursor
 */
PUBLIC bool nsfbCursorLocSet( nsfb_t *nsfb, int x, int y );

/** get the cursor location
 */
PUBLIC bool nsfbCursorLocGet( nsfb_t *nsfb, nsfb_bbox_t *loc);

/** Plot the cursor saving the image underneath.
 */
PUBLIC bool nsfbCursorPlot( nsfb_t  * nsfb );

/** Clear the cursor restoring the image underneath
 */
PUBLIC bool nsfbCursorClear( nsfb_t * nsfb );

/** Destroy the cursor
  */
PUBLIC bool nsfbCursorDestroy(  struct nsfbCursor_s *cursor );



#endif
