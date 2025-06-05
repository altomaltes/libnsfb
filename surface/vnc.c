/*
 * Copyright 2009 Vincent Sanders <vince@simtec.co.uk>
 *
 * This file is part of libnsfb, http://www.netsurf-browser.org/
 * Licenced under the MIT License,
 *                http://www.opensource.org/licenses/mit-license.php
 */

#include "../libnsfb_plot.h"
#include "../surface.h"
#include "../plot.h"
#include "../cursor.h"

static nsfb _event_t *gevent;

/* vnc special set codes
 */
static enum nsfb _key_code_e vncNsfbMap[ 256 ] =
{ NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN
, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN
, NSFB_KEY_BACKSPACE, NSFB_KEY_TAB    , NSFB_KEY_LF     , NSFB_KEY_CLEAR    /* 0x08 */
, NSFB_KEY_UNKNOWN, NSFB_KEY_RETURN , NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN

/* 0x10 */
, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN, NSFB_KEY_RETURN
, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN

/* 0x18 */
, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN, NSFB_KEY_ESCAPE
, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN

/* 0x20 */
, NSFB_KEY_COMPOSE, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN
, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN

/* 0x28 */
, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN
, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN

 /* 0x30 */
, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN
, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN

/* 0x38 */
, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN
, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN

 /* 0x40 */
, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN
, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN

/* 0x48 */
, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN
, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN

/* 0x50 */
, NSFB_KEY_HOME   ,  NSFB_KEY_LEFT  ,      NSFB_KEY_UP, NSFB_KEY_RIGHT
, NSFB_KEY_DOWN   ,  NSFB_KEY_PAGEUP,NSFB_KEY_PAGEDOWN, NSFB_KEY_END

/* 0x58 */
, NSFB_KEY_HOME   , NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN
, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN
/* 0x60 */
, NSFB_KEY_UNKNOWN,   NSFB_KEY_PRINT,    NSFB_KEY_HELP, NSFB_KEY_UNKNOWN
, NSFB_KEY_UNKNOWN,    NSFB_KEY_UNDO, NSFB_KEY_UNKNOWN, NSFB_KEY_MENU

/* 0x68 */
, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN, NSFB_KEY_BREAK
, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN
, NSFB_KEY_UNKNOWN

/* 0x70 */
, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN
, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN,  NSFB_KEY_UNKNOWN


 /* 0x78 */
, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN
, NSFB_KEY_UNKNOWN, NSFB_KEY_MODE   , NSFB_KEY_NUMLOCK, NSFB_KEY_UNKNOWN

 /* 0x80 */
, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN
, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN

/* 0x88 */
, NSFB_KEY_UNKNOWN , NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN
, NSFB_KEY_KP_ENTER, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN

/* 0x90 */
, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN
, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN

/* 0x98 */
, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN
, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN

/* 0xA0 */
, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN
, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN

/* 0xA8 */
, NSFB_KEY_UNKNOWN , NSFB_KEY_KP_MULTIPLY, NSFB_KEY_KP_PLUS, NSFB_KEY_UNKNOWN
, NSFB_KEY_KP_MINUS, NSFB_KEY_KP_PERIOD, NSFB_KEY_KP_DIVIDE, NSFB_KEY_KP0

/* 0xB0 */
,  NSFB_KEY_KP1,    NSFB_KEY_KP2,    NSFB_KEY_KP3,    NSFB_KEY_KP4
,  NSFB_KEY_KP5,    NSFB_KEY_KP6,    NSFB_KEY_KP7,    NSFB_KEY_KP8

/* 0xB8 */
, NSFB_KEY_KP9      , NSFB_KEY_UNKNOWN,    NSFB_KEY_UNKNOWN,    NSFB_KEY_UNKNOWN
, NSFB_KEY_KP_EQUALS

, NSFB_KEY_F1, NSFB_KEY_F2, NSFB_KEY_F3

 /* 0xC0 */
, NSFB_KEY_F4, NSFB_KEY_F5, NSFB_KEY_F6,  NSFB_KEY_F7
, NSFB_KEY_F8, NSFB_KEY_F9, NSFB_KEY_F10, NSFB_KEY_F11

/* 0xC8 */
, NSFB_KEY_F12,    NSFB_KEY_F13,    NSFB_KEY_F14,    NSFB_KEY_F15
, NSFB_KEY_UNKNOWN,    NSFB_KEY_UNKNOWN,    NSFB_KEY_UNKNOWN
, NSFB_KEY_UNKNOWN

, /* 0xD0 */
, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN
, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN /* 0xD8 */
, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN
, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN /* 0xE0 */

, NSFB_KEY_LSHIFT,  NSFB_KEY_RSHIFT,  NSFB_KEY_LCTRL  , NSFB_KEY_RCTRL
, NSFB_KEY_CAPSLOCK
, NSFB_KEY_SCROLLOCK
, NSFB_KEY_LMETA
, NSFB_KEY_RMETA /* 0xE8 */
, NSFB_KEY_LALT
, NSFB_KEY_RALT
, NSFB_KEY_LSUPER
, NSFB_KEY_RSUPER

, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN /* 0xF0 */
, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN
, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN /* 0xF8 */
, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN
, NSFB_KEY_UNKNOWN, NSFB_KEY_UNKNOWN

, NSFB_KEY_DELETE
};


static void vncDoptr( int buttonMask
                    , int x,int y
                    , rfbClientPtr cl )
{ static int prevbuttonMask = 0;

  UNUSED(cl);

  if (prevbuttonMask != buttonMask) /* button click */
  { if (((prevbuttonMask ^ buttonMask) & 0x01) == 0x01)
    { if ((buttonMask & 0x01) == 0x01)
      { gevent->type = NSFB_EVENT_KEY_DOWN;
	  }
	  else
	  { gevent->type = NSFB_EVENT_KEY_UP;
	  }
	  gevent->value.keycode = NSFB_KEY_MOUSE_1;
	}

	else if (((prevbuttonMask ^ buttonMask) & 0x02) == 0x02)
	{ if ((buttonMask & 0x01) == 0x01)
	  { gevent->type = NSFB_EVENT_KEY_DOWN;
	  }
	  else
	  { gevent->type = NSFB_EVENT_KEY_UP;
	  }
	  gevent->value.keycode = NSFB_KEY_MOUSE_2;
	}
	else if (((prevbuttonMask ^ buttonMask) & 0x04) == 0x04)
	{ if ((buttonMask & 0x01) == 0x01)
	  { gevent->type = NSFB_EVENT_KEY_DOWN;
    }
    else
    { gevent->type = NSFB_EVENT_KEY_UP;
	  }
	  gevent->value.keycode = NSFB_KEY_MOUSE_3;
	 }
  else if (((prevbuttonMask ^ buttonMask) & 0x08) == 0x08)
  { if ((buttonMask & 0x01) == 0x01)
    { gevent->type = NSFB_EVENT_KEY_DOWN;
	   }
    else
    { gevent->type = NSFB_EVENT_KEY_UP;
	   }
	  gevent->value.keycode = NSFB_KEY_MOUSE_4;
	}
  else if (((prevbuttonMask ^ buttonMask) & 0x10) == 0x10)
  { if ((buttonMask & 0x01) == 0x01)
    { gevent->type = NSFB_EVENT_KEY_DOWN;
	  }
    else
    { gevent->type = NSFB_EVENT_KEY_UP;
	   }
	    gevent->value.keycode = NSFB_KEY_MOUSE_5;
	   }
	   prevbuttonMask = buttonMask;
  }
  else
  { gevent->type = NSFB_EVENT_MOVE_ABSOLUTE;
  	gevent->value.vector.x = x;
	  gevent->value.vector.y = y;
  	gevent->value.vector.z = 0;
} }


static void vncDokey(rfbBool down, rfbKeySym key, rfbClientPtr cl)
{ enum nsfb_ key_code_e keycode = NSFB_KEY_UNKNOWN;

  UNUSED(cl);

  if ((key >= XK_space)
   && (key <= XK_asciitilde))
  {	keycode = key; /* ascii codes line up */
  }
  else if ((key & 0xff00) == 0xff00) /* bottom 8bits of keysyms in this range map via table */
  { keycode = vncNsfbMap[(key & 0xff)];
  }

  if (down == 0)/* key up */
  { gevent->type = NSFB_EVENT_KEY_UP;
  }
  else  	/* key down */
  { gevent->type = NSFB_EVENT_KEY_DOWN;
  }
  gevent->value.keycode = keycode;
}


static int vncSetGeometry( Nsfb *nsfb
                           , int width, int height
                           , enum NsfbFormat format )
{ format &= NSFB_FMT_MASK;

  if ( nsfb->surfacePriv )
  { return -1; /* fail if surface already initialised */
  }

  if (width > 0)
  { nsfb->width = width;
  }

  if (height > 0)
  { nsfb->height = height;
  }

  if (format != NSFB_FMT_ANY)
  { nsfb->format = format;
  }

    /* select soft plotters appropriate for format */
  selectPlotters( nsfb );

  return 0;
}

static int vncInitialise(Nsfb *nsfb)
{ rfbScreenInfoPtr vncscreen;
  int argc = 0;
  char **argv = NULL;

  if ( nsfb->surfacePriv  )  /* fail l if surface already initialised */
  { return -1;
  }


  if (nsfb->bpp != 32)     /* sanity checked depth. */
  { return -1;
  }

/*
 *   create vnc screen with 8bits per sample, three samples per
 * pixel and 4 bytes per pixel.
 */

  vncscreen= rfbGetScreen( &argc, argv
                         , nsfb->width, nsfb->height
                         , 8, 3, (nsfb->bpp / 8));

/* Note libvncserver does not check its own allocations/error
 * paths so the faliure mode of the rfbGetScreen is to segfault.
 */
  if ( vncscreen == NULL )
  { return -1;
  }

  vncscreen->frameBuffer= calloc( nsfb->width * nsfb->height, (nsfb->bpp / 8) );

  if (vncscreen->frameBuffer == NULL)
  { rfbScreenCleanup(vncscreen);
	  return -1;
  }


  switch (nsfb->bpp)
  { case 8:	break;

    case 16:
      vncscreen->serverFormat.trueColour=TRUE;
      vncscreen->serverFormat.redShift = 11;
      vncscreen->serverFormat.greenShift = 5;
      vncscreen->serverFormat.blueShift = 0;
      vncscreen->serverFormat.redMax = 31;
      vncscreen->serverFormat.greenMax = 63;
      vncscreen->serverFormat.blueMax = 31;
  	 break;

    case 32:
      vncscreen->serverFormat.trueColour=TRUE;
      vncscreen->serverFormat.redShift = 16;
      vncscreen->serverFormat.greenShift = 8;
      vncscreen->serverFormat.blueShift = 0;
   	break;
  }

  vncscreen->alwaysShared = TRUE;
  vncscreen->autoPort = 1;
  vncscreen->ptrAddEvent = vncDoptr;
  vncscreen->kbdAddEvent = vncDokey;

  rfbInitServer(vncscreen);

    /* keep parameters */
  nsfb->surfacePriv = vncscreen;
  nsfb->ptr = (byte *)vncscreen->frameBuffer;
  nsfb->linelen = (nsfb->width * nsfb->bpp) / 8;

  return 0;
}

static int vncFinalise(Nsfb *nsfb)
{ rfbScreenInfoPtr vncscreen= nsfb->surfacePriv;

  if ( vncscreen )
  { rfbScreenCleanup( vncscreen );
  }

  return 0;
}


static int vncUpdate(Nsfb *nsfb, NsfbBbox *box)
{ rfbScreenInfoPtr vncscreen= nsfb->surfacePriv;

  rfbMarkRectAsModified( vncscreen, box->x0, box->y0, box->x1, box->y1 );

  return 0;
}


//static bool vncInput( Nsfb *nsfb, nsfb/_event_t *event, int timeout )
//{ rfbScreenInfoPtr vncscreen = nsfb->surfacePriv;
//  int ret;
//
//  if ( vncscreen  )
//  { gevent = event; /* blergh - have to use global state to pass data */

/* set default to timeout
 */
//  	 event->type             = NSFB_EVENT_CONTROL;
//	   event->value.controlcode= NSFB_CONTROL_TIMEOUT;
//
//  	 ret = rfbProcessEvents(vncscreen, timeout * 1000);
//
//	   if (ret == 0)  /* valid event */
//  	 { return true;
//    }

	/* connection error occurred */
//  }

 // return false;
//}

static int vncCursor( Nsfb *nsfb
                    , struct nsfbCursor_s *cursor )
{ rfbScreenInfoPtr vncscreen= nsfb->surfacePriv;
  rfbCursorPtr     vnccursor= calloc(1,sizeof(rfbCursor));
  int rwidth; /* rounded width */
  int row;
  int col;
  const NsfbColour *pixel;
  byte bit;

  rwidth = (cursor->bmp_width + 7) / 8;

  vnccursor->cleanup = 1; /* rfb lib will free this allocation */
  vnccursor->width = cursor->bmp_width;
  vnccursor->height = cursor->bmp_height;
  vnccursor->foreRed = vnccursor->foreGreen = vnccursor->foreBlue = 0xffff;

  vnccursor->source = calloc(rwidth, vnccursor->height);
  vnccursor->cleanupSource = 1; /* rfb lib will free this allocation */
  vnccursor->mask = calloc(rwidth, vnccursor->height);
  vnccursor->cleanupMask = 1; /* rfb lib will free this allocation */

  for ( row = 0, pixel = cursor->pixel
      ; row < vnccursor->height
      ; row++)
  { for( col = 0, bit = 0x80
       ; col < vnccursor->width
       ; col++, bit = (bit & 1)? 0x80 : bit>>1, pixel++ ) 	    /* pixel luminance more than 50% */
	{ if ((((((*pixel) & 0x000000ff) *  77)
	    +  ((((*pixel) & 0x0000ff00) >>  8) * 151)
	    +  ((((*pixel) & 0x00ff0000) >> 16) * 28)) / 256) > 128)
	  { vnccursor->source[row * rwidth + col/8] |= bit;
	  }
	  if (((*pixel) & 0xff000000) != 0)
	  {	vnccursor->mask[row * rwidth + col/8] |= bit;
   }	} }

  rfbSetCursor(vncscreen, vnccursor);
  return true;
}

const NsfbSurfaceRtns vncRtns =
{ .type=     NSFB_SURFACE_VNC

, .initialise= vncInitialise
, .finalise  = vncFinalise
, .update    = vncUpdate
, .cursor    = vncCursor
, .geometry  = vncSetGeometry
};

NSFB_SURFACE_DEF( &vncRtns )

/*
 * Local variables:
 *  c-basic-offset: 4
 *  tab-width: 8
 * End:
 */
