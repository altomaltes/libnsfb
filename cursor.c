/*
 * Copyright 2010 Vincent Sanders <vince@kyllikki.org>
 *
 * This file is part of libnsfb, http://www.netsurf-browser.org/
 * Licenced under the MIT License,
 *                http://www.opensource.org/licenses/mit-license.php
 */

/** \file
 * cursor (implementation).
 */

#include <stdlib.h>

#include "nsfb.h"
#include "nsfbPlot.h"
#include "libnsfb_cursor.h"

#include "cursor.h"
#include "plot.h"
#include "surface.h"

bool nsfbCursorInit( NsfbSurfaceRtns * surf )
{ if ( !surf->pointer )
  { if (surf->pointer = CALLOC( sizeof( NsfbCursor )))
    { surf->pointer->loc.posi.x= surf->theWidth  / 2 + 1;
      surf->pointer->loc.posi.y= surf->theHeigth / 2 + 1;
      surf->pointer->plotted= -2; /* Mark as hidded */

      return( true );
  } }

  return( false );
}

/*
*/

ANSIC bool nsfbCursorSet( NsfbSurfaceRtns * surf
                        , IcoRec          * cursor )
{ if ( surf )
  { nsfbCursorInit( surf );

    if ( !surf->pointer->ico )     /* Create the bitmap */
    { surf->pointer->ico= openIco( cursor );

      if ( !surf->pointer->savDat )
      { surf->pointer->savDat= calloc( surf->pointer->ico->width
                                     * surf->pointer->ico->height
                                     , sizeof( unsigned ));
      }

      surf->pixmap( surf
                  , &surf->pointer->ico->map
                  ,  surf->pointer->ico->image
                  ,  NULL
                  , cursor->wNat
                  , cursor->hNat  );      /* Register new image    */

    //  if ( surf->pointer->loc.posi.x    /* Yet initialized surface */
    //    && surf->pointer->loc.posi.y )
    //  { nsfbCursorLocSet( surf
    //                    , 0, 0 );
    //  }
    //  else                              /* Display once initialized */
     // { surf->pointer->plotted= 0;
    }// }

    surf->pointer->plotted= -1;         /* Mark as uninitialized */
    return( true );
  }

  return( false );
}


bool nsfbCursorClear( NsfbSurfaceRtns * surf )
{ if ( surf )
  { if ( surf->pointer )
    { if ( surf->pointer->plotted < -1 ) /* Keep  inactive */
      { return( true );
      }
      if ( surf->pointer->plotted > 0 )  /* Onscreen */
      { if ( surf->cursor )
        { surf->pointer->plotted= -1;
          surf->cursor( surf );
      } }
      surf->pointer->plotted= 0;   /* Mark as retired */
      return( true );
  } }

  return( false );
}

int nsfbCursorHide( NsfbSurfaceRtns * surf )
{ if ( nsfbCursorClear( surf ) )
  { surf->pointer->plotted= -2; /* Mark as hidded */
  }
  return( 0 );
}

/**
 */
ANSIC NsfbPoint * nsfbCursorLocSet( NsfbSurfaceRtns * surf
                                  , int x, int y )
{ if ( surf )
  { struct NsfbCursorSt * cursor= surf->pointer;

    if ( cursor  )
    { if ( cursor->plotted > -2 )   /* active */
      { int flag= x | y;  /* No restore needed, for previous  */

        x= cursor->loc.posi.x - cursor->hotspot_x + x;
        y= cursor->loc.posi.y - cursor->hotspot_y - y;

        if ( x < 0            ) { x= 0;  }
        if ( y < 0            ) { y= 0; }
        if ( x > surf->theWidth  ) { x= surf->theWidth;  }
        if ( y > surf->theHeigth ) { y= surf->theHeigth; }

        cursor->loc.posi.x= x;
        cursor->loc.posi.y= y;

        cursor->loc.size.w= cursor->ico->width;
        if ( x + cursor->ico->width > surf->theWidth )
        { cursor->loc.size.w= surf->theWidth - x;
        }

        cursor->loc.size.h= cursor->ico->height;
        if ( y + cursor->ico->height > surf->theHeigth )
        { cursor->loc.size.h= surf->theHeigth - y;
        }

        if ( flag )
        { //struct NsfbEventStruct evt; memset( &evt, 0, sizeof( evt ));

          //evt.type = NSFB_EVT_MOVE  ;
          //evt.x    = cursor->loc.posi.x;
          //evt.y    = cursor->loc.posi.y;
          //evt.state= 0;

          //nsfbSpreadEvent( surf, &evt );
        }
        else
        { cursor->plotted= -2;    /* Do not restore previous */
        }

        surf->cursor( surf );

        cursor->loc.posi.x += cursor->hotspot_x;
        cursor->loc.posi.y += cursor->hotspot_y;

        return( &surf->pointer->loc.posi );
  } } }

  return( NULL );
}

ANSIC NsfbPoint * nsfbCursorLocGet( NsfbSurfaceRtns * surf )
{ if ( surf )
  { if ( surf->cursor )
    { return( &surf->pointer->loc.posi );
  } }

  return( NULL );
}

/* documented in cursor.h
 */

bool nsfbCursorPlot( NsfbSurfaceRtns * surf )
{ int savSize;

  struct NsfbCursorSt * cursor= surf->cursor;

  if ( cursor )
  {

/* offset cursor rect for hotspot
 */
 //   cursor->loc.x -= cursor->hotspot_x;
 //   cursor->loc.y -= cursor->hotspot_y;

//    if (cursor->savSize < savSize)
//    { cursor->savCol = realloc( cursor->savCol, savSize );
//      cursor->savSize= savSize;
//    }

/*    nsfb->plotterFns->readrect( nsfb, &cursor->savLoc, cursor->savCol );

    cursor->savWidth = cursor->savLoc.x1 - cursor->savLoc.x0;
    cursor->savHeight= cursor->savLoc.y1 - cursor->savLoc.y0;

    nsfb->plotterFns->setClip( nsfb, NULL );

    nsfb->plotterFns->pixmapFill( nsfb
                                , cursor->ico
                                , cursor->loc.x0
                                , cursor->loc.y0
                                ,  0, 0, 0 );

*/

/* undo hotspot offset
 */
 //   cursor->loc.x += cursor->hotspot_x;
//    cursor->loc.y += cursor->hotspot_y;


    return( true );
  }

  return( false );
}

bool nsfbCursorDestroy( struct NsfbCursorSt * cursor )
{ FREE( cursor );

  return( true );
}


/** ================================================= [ JACS, 10/01/2023 ] == *\
 *                                                                            *
 *   JACS 2023                                                                *
 *                                                                            *
 *  CALLBACK attInput                                                         *
 *                                                                            *
 *  @brief: evdev client                                                      *
 *                                                                            *
\* ========================================================================= **/
enum
{ DID_INPUT         =  ( 1 << 15 )                   /* All of them      */
, DID_GRAB_DEVICE   =  ( 0x80000000 | DID_INPUT  )   /* Grab this device */
, DID_RELEASE_DEVICE=  ( 0x80000000              )   /* Release this device */

, DID_INP_EVENT    =  ( 1 << 0  )
, DID_INP_KEY      =  ( 1 << 1  )
, DID_INP_KEYBOARD =  ( 1 << 2  )
, DID_INP_BUTTON   =  ( 1 << 3  )
, DID_INP_MOUSE    =  ( 1 << 4  )
, DID_INP_POINTER  =  ( 1 << 5  )
, DID_INP_REL      =  ( 1 << 6  )
, DID_INP_TABLET   =  ( 1 << 7  )
, DID_INP_TOUCHPAD =  ( 1 << 8  )
, DID_INP_TOUCH    =  ( 1 << 9  )
, DID_INP_JOYSTICK =  ( 1 << 10 )
, DID_INP_SWITCH   =  ( 1 << 11 )
, DID_INP_MULTI    =  ( 1 << 12 )
};


/**
 */
ANSIC int nsfbSpreadEvent( NsfbSurfaceRtns * surf
                         , NsfbEvent * evt )
{ int cnt= 0;
  Nsfb * client;

  if ( !evt->stamp )
  { evt->stamp= time( NULL );
  }

  if ( !evt->serial )
  { static int serial;

    evt->serial= ++serial;
  }

  for( client= surf->clients
     ; client
     ; client= client->next )
  { client->toClient( evt
                    , surf->clients->toData );
    cnt++;
  }

  return( cnt );
}


ANSIC int nsfbAttEvent( int sk, void * userData
                      , int sz, void * stream  )
{ NsfbEvent       * evt= (NsfbEvent       *)stream;
  NsfbSurfaceRtns * con= (NsfbSurfaceRtns *)userData;

//  printf( "ARR %08X %08X %08X \n", sz, DID_INP_TOUCH, DID_INP_TOUCH );

  if ( sz < 0 ) switch( sz ) /* Metadata */
  { case DID_INP_MOUSE    | DID_RELEASE_DEVICE:
      nsfbCursorHide( con );
    break;

    case DID_INP_MOUSE    | DID_GRAB_DEVICE   :
      if ( !con->cursor )                         /* The console manages the mouse and keyboard yet */
      { break;
      }
      nsfbCursorSet(  con, &cursorDefault );   /* Overloads nsfbCursorShow */

    case DID_INP_TABLET   | DID_GRAB_DEVICE :
    case DID_INP_TOUCH    | DID_GRAB_DEVICE :
    case DID_INP_TOUCHPAD | DID_GRAB_DEVICE :
    case DID_INP_JOYSTICK | DID_GRAB_DEVICE : return( DID_GRAB_DEVICE );
  }
  else
  { if ( evt->mod > 0 )
    { evt->type = NSFB_EVT_PRESS;// puts("press");
    }
    else if ( evt->mod < 0 )
    { evt->type = NSFB_EVT_REL;  // puts("release");
    }
    else
    { evt->type = NSFB_EVT_MOVE; // puts("move");
    }


  switch ( evt->guess )
  { case DID_INP_REL:
      puts("rel");
    break;

    case DID_INP_KEY:
      puts("rel");
    break;

    case DID_INP_MOUSE:
    { evt->but &= 0x7;

      NsfbPoint * get= nsfbCursorLocSet( con        /* graph libatry clips */
                                       , evt->a
                                       , evt->b );
      evt->x= get->x;                               /* Clip to screen */
      evt->y= get->y;
    }

    case DID_INP_POINTER:
      nsfbSpreadEvent( con, evt );
    break;

    case DID_INP_JOYSTICK:
      evt->type= evt->guess;
      nsfbSpreadEvent( con, evt );
    break;
  } }

  return( 0 );
}



