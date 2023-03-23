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

struct NsfbCursorSt
{ DeviceImageRec * ico; /* current cursor image  */

  int plotted;

  NsfbArea loc
         , last;

  int hotspot_x;
  int hotspot_y;

/* current saved image
 */
  NSFBCOLOUR * savDat;

};


ANSIC bool nsfbCursorDestroy( struct NsfbCursorSt    * );
ANSIC bool nsfbCursorPlot(           NsfbSurfaceRtns * );


/* Hardcoded vectors
 */

extern IcoRec cursorDefault;





#endif /* CURSOR_H */
