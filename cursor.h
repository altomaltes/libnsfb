/*
 * Copyright 2009 Vincent Sanders <vince@simtec.co.uk>
 *
 * This file is part of libnsfb, http://www.netsurf-browser.org/
 * Licenced under the MIT License,
 *                http://www.opensource.org/licenses/mit-license.php
 *
 * This is the *internal* interface for the cursor.
 */

#ifndef CURSOR_H
#define CURSOR_H 1

#include "nsfb.h"

struct nsfbCursor_s
{ bool plotted;
  nsfb_bbox_t loc;

/* current cursor image 
 */

  const nsfb_colour_t *pixel;
  int bmp_width;
  int bmp_height;
  int bmp_stride;
  int hotspot_x;
  int hotspot_y;

/* current saved image 
 */
  nsfb_bbox_t     savLoc;
  nsfb_colour_t * savCol;
  int savSize;
  int savWidth;
  int savHeight;

};

bool nsfbCursorDestroy( struct nsfbCursor_s *cursor );
bool nsfbCursorPlot( nsfb_t * nsfb );




#endif /* CURSOR_H */
