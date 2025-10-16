/**
 *     AUTHOR: Jose Angel Caso Sanchez, 2022   ( altomaltes@gmail.com )
 *
 *     Copyright (C) EMASA, Empresa Municipal De Aguas de Gijon
 *
 *
 *     FILE: x11.cc
 *     DATE: ene 2022
 *
 *  DESCRIPCION:
 *
 *  SMS dialog
 *
 *  2009 Vincent Sanders <vince@simtec.co.uk>
 */

#include <string.h>


#include "../libnsfb_plot_util.h"
#include "../libnsfb_cursor.h"
#include "../plot.h"
#include "../surface.h"
#include "../cursor.h"
#include "../x11.h"


static int x11Finalise( Nsfb * nsfb )
{ /*if ( nsfb )
  { if ( nsfb->pan )
    { free( nsfb->pan );
      nsfb->pan= NULL;
    }
    if ( nsfb->loc )
    { free( nsfb->loc );
      nsfb->loc= NULL;
  } } */

  return( -1 );
}


static NsfbSurfaceRtns * initX11Display( struct x11Priv * drv )
{ int nColors= 254;                           // HP-UX only supports 254 colors
  int idx;

  char name[ 32 ]; strcpy( name, drv->rtns.name );

  char * mk= strchr( name, '@' );

  if ( mk )    /* Truncate non display info */
  { *mk++= 0;
  }

  if ( ! ( drv->theDisplay= XOpenDisplay( name )))                       // Can't open display, this is basic
  { fprintf( stderr, "Can't open X11 display %s"
          , name );
    return( NULL );
  }



//  drv->theScreen= DefaultScreen( drv->theDisplay  );
  drv->theScreen     = DefaultScreen  ( drv->theDisplay );    // feb 2020
  drv->theRoot       = RootWindow     ( drv->theDisplay, drv->theScreen ); // Abrir el ingenio
  drv->theVisual     = DefaultVisual  ( drv->theDisplay, drv->theScreen );
  drv->theCmap       = DefaultColormap( drv->theDisplay, drv->theScreen );
  drv->rtns.theWidth = XWidthOfScreen ( XDefaultScreenOfDisplay( drv->theDisplay ) );
  drv->rtns.theHeigth= XHeightOfScreen( XDefaultScreenOfDisplay( drv->theDisplay ) );
  drv->rtns.theDepth = DefaultDepth   ( drv->theDisplay, drv->theScreen );

/*
 * Now arrange the system it can work with all knonwn color planes
 */

  switch( drv->rtns.theDepth )
  { case 8:
      for( idx= 0                   // Prepare template
         ; idx < nColors
         ; idx ++ )
      { drv->palette[ idx ].pixel= idx;
      }

      XQueryColors( drv->theDisplay   // Ask for the current colors
                  , drv->theCmap
                  , drv->palette
                  , idx );

      if ( /*reserved */ 1 )
      { drv->theCmap= XCreateColormap( drv->theDisplay
                                     , drv->theRoot
                                     , (Visual *)drv->theVisual
                                     , AllocAll );
        for( idx= 39
           ; idx < nColors
           ; idx ++ )
        { drv->palette[ idx ].red=
          drv->palette[ idx ].blue=
          drv->palette[ idx ].green=
          drv->palette[ idx ].flags= DoRed | DoGreen | DoBlue;
        }

        XStoreColors( drv->theDisplay
                    , drv->theCmap
                    , drv->palette
                   , nColors );
        XInstallColormap( drv->theDisplay
                        , drv->theCmap );
  } }

  /*
   * OS native server  ççç
   */


  drv->rtns.fd= ConnectionNumber( drv->theDisplay );

  return( &drv->rtns );
}



/* We have a working display, create the window asked for
 */

/*
 */
static int x11Initialise( struct x11List * drv )
{ if ( drv )
  { struct x11Priv * x11= drv->seed.surfaceRtns;;

    drv->seed.loclen=
    drv->seed.panlen= drv->seed.width  * sizeof(int);
    drv->seed.buflen= drv->seed.panlen * drv->seed.height;
    drv->seed.format= NSFB_FMT_ABGR8888;
    drv->theDisplay= x11->theDisplay;

/* Hardware initialization
 */


    XSetWindowAttributes attr;


    attr.background_pixel= /*drv->background*/ 0xFFF;
    attr.backing_store   = WhenMapped;
    attr.event_mask      = EVENTMASK;

/* Test for window manager JACS 07/07/2016
*/
    x11->rtns.theDepth= DefaultDepth( x11->theDisplay, x11->theScreen );;   // çç

    if ( XInternAtom( drv->theDisplay, "_NET_SUPPORTING_WM_CHECK", true ) )
    { XMapWindow( drv->theDisplay
                , drv->theWindow= XCreateWindow( x11->theDisplay
                                               , x11->theRoot
                                               , 0, 0                  // X and Y positions
                                               , drv->seed.width, drv->seed.height                  // Window size
                                               , 0                     // Border width
                                               , x11->rtns.theDepth, InputOutput
                                               , (Visual*)x11->theVisual
                                               , CWBackPixel | CWBackingStore | CWEventMask, &attr )); // Background );
    }
    else
    { drv->theWindow= x11->theRoot;
    }

    if (( drv->seed.pan= CALLOC( drv->seed.buflen ))) //  stride= w;
    { drv->theSurface= XCreatePixmap /*FromBitmapData*/ ( drv->theDisplay
                                                  , x11->theRoot
                                                 // , (char*)drv->seed.pan
                                                  , drv->seed.width, drv->seed.height  /* Suggested size     */
                                                //  , 1, 0
                                                  , x11->rtns.theDepth  );
      XImage * theImage= XGetImage( drv->theDisplay
                                  , drv->theSurface
                                  , 0, 0
                                  , drv->seed.width, drv->seed.height
                                  , AllPlanes   /* plane mask    */
                                  , ZPixmap );
      drv->seed.pan= theImage->data;

      XGCValues gcValues=
      { .graphics_exposures= 0
      };

      drv->theGC= XCreateGC( drv->theDisplay
                           , drv->theSurface
                           , GCGraphicsExposures, &gcValues );  /* 2022 */

/*
 * Not used yet events
 * EnterWindowMask     PointerMotionMask PointerMotionHintMask
 * Button1MotionMask   Button2MotionMask Button3MotionMask  Button4MotionMask
 * Button5MotionMask   ButtonMotionMask  KeymapStateMask
 * ResizeRedirectMask  SubstructureRedirectMask
 * PropertyChangeMask  ColormapChangeMask OwnerGrabButtonMask
 */

     XStoreName( drv->theDisplay
               , drv->theWindow
               , drv->seed.theTitle );


      drv->seed.plotterFns= &x11PlottersAddr;
      XFlush( drv->theDisplay );
  } }

  return( 0 );
}

static int x11Claim( Nsfb *nsfb, NsfbBbox * box )
{ //struct NsfbCursorSt *cursor= nsfb->cursor;
       /*
     */
  return( 0 );
}


/*
 *
 *
 */
static int x11Update( Nsfb      * nsfb
                    , NsfbBbox  * box )
{ struct x11Priv * x11= nsfb->surfaceRtns;
  struct x11List * win= nsfb;

  int x=0, y=0;

 // XCopyArea( x11->theDisplay
   //        , win->theSurface, win->theWindow
     //      , win->theGC
       //    , x, y
         //  , nsfb->width, nsfb->height
           //, x, y ) ;
  XFlush( x11->theDisplay );

  return( 0 );
}

/*
 *
 *
 */
static int x11Pan( Nsfb * nsfb, int type )
{ struct x11Priv * x11= nsfb->surfaceRtns;
  struct x11List * win= nsfb;

  switch ( type )
  { case NSFB_PAN_START : puts( "PAN START" );  break;

    case NSFB_PAN_SWITCH: return( x11Update( nsfb, NULL ));
    case NSFB_PAN_BSTORE: puts( "PAN BSTORE" ); break;
    case NSFB_PAN_DUMP  : puts( "PAN DUMP" );   break;

    default: puts( "PAN OTHER" );   return( -1 );
  }


  return( 0 );
}



/*
 * Main entry point for sourcer/sinker
 */
static int x11Events( int theDisp, void * userData
                    , int      sz, void * list  )
{ struct x11Priv * x11= (struct x11Priv *)userData;

  XEvent e;

  if ( sz <= 0 )    // pass tru
  { return( 0 );
  }

  if ( XCheckMaskEvent( x11->theDisplay   // Flush all events in the library, because then doesn't awake the select()
                         , EVENTMASK         // Select all events
                         , &e ))
  { //    printf ("Type >> %d - %d\n", thisEvent.type, ButtonPress );

/*  Search for window affectees
 */
    if ( x11->theDisplay == e.xany.display )
    { struct x11List * ptr;

       for( ptr= x11->rtns.clients
          ; ptr
          ; ptr= ptr->seed.next )
       { if ( ptr->theWindow == e.xany.window )
         { ToClientFun toClient= ptr->seed.toClient;
           void      * toData=   ptr->seed.toData;
           struct NsfbEventStruct evt; memset( &evt, 0, sizeof( evt ));

           evt.serial= e.xany.serial;

           switch ( e.type )
           { case VisibilityNotify:
               if ( e.xvisibility.state != VisibilityFullyObscured )
               { evt.type= NSFB_EVT_SHOW;
               }
             break;

            case FocusIn         : evt.type= NSFB_EVT_FOCUSIN; break;
            case FocusOut        : evt.type= NSFB_EVT_FOCUSOT; break;
            case UnmapNotify     : evt.type= NSFB_EVT_UNMAP;   break;
            case MapNotify       : evt.type= NSFB_EVT_MAP;     break;
            case Expose          : evt.type= NSFB_EVT_EXPOSE;  break;
            case NoExpose        : evt.type= NSFB_EVT_NEXPOSE; break;
            case SelectionClear  : evt.type= NSFB_EVT_SCLEAR;  break;
            case SelectionRequest: evt.type= NSFB_EVT_SREQ;    break;
            case PropertyNotify  : evt.type= NSFB_EVT_PROPERT; break;
            case DestroyNotify   : evt.type= NSFB_EVT_DESTROY; break; // Throw away destroyes windows

            case ConfigureNotify :   // Throw away destroyes windows
              evt.type= NSFB_EVT_RESIZE;
              evt.a   = e.xconfigure.width;
              evt.b   = e.xconfigure.height;
            break;

            case LeaveNotify     :   // Throw away destroyes windows
              evt.type =  NSFB_EVT_LEAVE;
              evt.mod=    e.xcrossing.state;
              evt.key= e.xcrossing.detail;
            break;

            case MotionNotify    :
              evt.type = NSFB_EVT_MOVE  ;
              evt.x    = e.xbutton.x;
              evt.y    = e.xbutton.y;
              evt.mod= e.xbutton.state;
              evt.stamp= e.xbutton.time;
            break;

            case ButtonPress     :
              evt.type = NSFB_EVT_PRESS ;
              evt.x    = e.xbutton.x;
              evt.y    = e.xbutton.y;
              evt.b    = e.xbutton.button;
              evt.mod  = e.xbutton.state;
              evt.stamp= e.xbutton.time;
            break;

            case ButtonRelease   :
              evt.type= NSFB_EVT_REL;
              evt.x    = e.xbutton.x;
              evt.y    = e.xbutton.y;
              evt.b = e.xbutton.button;
              evt.mod= e.xbutton.state;
              evt.stamp= e.xbutton.time;
            break;


            case KeyPress        :
              evt.type= NSFB_EVT_KEYPRESS;
              evt.stamp= e.xkey.time;
              evt.key =  e.xkey.keycode;
              evt.mod= e.xkey.state;
            break;

            case KeyRelease      :
              evt.type= NSFB_EVT_KEYRELEASE;
              evt.stamp= e.xkey.time;
              evt.key =  e.xkey.keycode;
              evt.mod= e.xkey.state;
            break;

            case EnterNotify     : evt.type= NSFB_EVT_ENTER;         break;
            case KeymapNotify    : evt.type= NSFB_EVT_LEAVE;         break;
            case GraphicsExpose  : evt.type= NSFB_EVT_GRAPH_EXPOSE;  break;
            case CreateNotify    : evt.type= NSFB_EVT_CREATE_WIN;    break;
            case MapRequest      : evt.type= NSFB_EVT_MAP_REQ;       break;
            case ReparentNotify  : evt.type= NSFB_EVT_REPARENT_NOT;  break;
            case ConfigureRequest: evt.type= NSFB_EVT_COFIGURE_REQ;  break;
            case GravityNotify   : evt.type= NSFB_EVT_GRAVITY_NOT;   break;
            case ResizeRequest   : evt.type= NSFB_EVT_RESIZE_REQ;    break;
            case CirculateNotify : evt.type= NSFB_EVT_CIRCULATE_NOT; break;
            case CirculateRequest: evt.type= NSFB_EVT_CIRCULATE_REQ; break;
            case SelectionNotify : evt.type= NSFB_EVT_SELECTION_NOT; break;
            case ColormapNotify  : evt.type= NSFB_EVT_COLORMAP_NOT;  break;
            case ClientMessage   : evt.type= NSFB_EVT_CLIENT_MESS;   break;
            case MappingNotify   : evt.type= NSFB_MAP_NOT;           break;

            default: fprintf( stderr, "UNKNOWN EVENT in %s -> %d\r\n", __FUNCTION__, e.type );
         }
         toClient( &evt, toData);
  } } } }

  return( 0 );              // Not more active windows
}

/** ================================================= [ JACS, 10/01/2006 ] == *\
 *                                                                            *
 *   JASC 2006                                                                *
 *                                                                            *
 *  METHOD Win32Console::doEvent                                              *
 *                                                                            *
 *  @brief  Distribute events among windows                                   *
 *                                                                            *
\* ========================================================================= **/
static int x11Pixmap( struct x11Priv * x11
                    , ImageMap * map
                    , void     * data
                    , void     * mask
                    , int bmpWidth, int bmpHeight )
{ if ( x11 )
  { if ( data )
    { int planes= 32;

      int sz= bmpWidth * bmpHeight * sizeof(  NSFBCOLOUR );

      switch( planes )
      { case  1: break;
        case  8: break;

        case 16: map->image=
          XCreateImage( x11->theDisplay
                      , (Visual*)x11->theVisual
                      ,  x11->rtns.theDepth
                      , ZPixmap, 0
                      , (char *)GETCOL16( bmpWidth, bmpHeight, data )
                      , bmpWidth, bmpHeight
                      , 8, 0 );
    break;

    case 24:
    case 32:
      map->image= XCreateImage( x11->theDisplay
                              , (Visual *)x11->theVisual
                              , x11->rtns.theDepth
                              , ZPixmap, 0
                              , data // (char *)GETCOL32( w,h, data )
                              , bmpWidth, bmpHeight
                              , 8, 0 );

      map->mask= mask
               ? XCreatePixmapFromBitmapData( x11->theDisplay
                                            , x11->theRoot
                                            , (char*)GETALPHA( bmpWidth, bmpHeight, data )
                                            , bmpWidth, bmpHeight             /* Suggested size     */
                                            , 1, 0, 1 ) : NULL;


    break;
    default: return( -1 );
  }





  } }

  return( 0 );
}


/*
 *
 */
NsfbSurfaceRtns * newNode( const char * mode )
{ static int dispIdx= 0;

  struct x11Priv * node= ( struct x11Priv * ) CALLOC( sizeof( struct x11Priv ));

  if ( node )
  { node->rtns.type      = NSFB_SURFACE_X11 | ( dispIdx++ << 16 );
    node->rtns.name      = strdup( mode );
    node->rtns.pan       = x11Pan;
    node->rtns.events    = x11Events;
    node->rtns.claim     = x11Claim;
    node->rtns.update    = x11Update;
    node->rtns.finalise  = x11Finalise;
    node->rtns.pixmap    = x11Pixmap;
    node->rtns.initialise= x11Initialise;
    node->rtns.dataSize  = sizeof( struct x11List );

    node->rtns.cursor    = NULL;   /* OS takes care of cursor */

    return( initX11Display( node ) );  /* Unique identificator */
  }

  return( NULL );
}






