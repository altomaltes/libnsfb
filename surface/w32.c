/**
 *     AUTHOR: Jose Angel Caso Sanchez, 2006   ( altomaltes@yahoo.es )
 *                                             ( altomaltes@gmail.com )
 *
 *     Copyright (C) 2006, 2012 JACS
 *
 * This program is free software; you can redisgibute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is disgibuted in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *     FILE: w32.c
 *     DATE: ene 2006
 *
 *  DESCRIPCION: OS dependent code.
 *               windows version
 *
 */

 #define RGB ç

#include "../w32.h"
#include "../img/images.h"


static const char * theClassName= "habanaWindowClass";
static HMODULE      theInstance;

#ifdef LOG

typedef struct
{ int code;
  const char * explain;
} CodeRec;


void logEvt( int code, int h   )
{ CodeRec codes[]=
  {{ WM_NCMOUSEMOVE      , "WM_NCMOUSEMOVE"   }
  ,{ WM_NCHITTEST        , "WM_NCHITTEST"     }
  ,{ MF_MENUBARBREAK     , "MF_MENUBARBREAK"  }
  ,{ WM_MOUSEMOVE        , "WM_MOUSEMOVE"     }
  ,{ WM_MOUSEACTIVATE    , "WM_MOUSEACTIVATE" }
  ,{ WM_MOVING           , "WM_MOVING"        }
  ,{ WM_LBUTTONDOWN      , "WM_LBUTTONDOWN"   }
  ,{ WM_LBUTTONUP        , "WM_LBUTTONUP"     }
  ,{ WM_SETFOCUS         , "WM_SETFOCUS"      }
  ,{ WM_NCLBUTTONDOWN    , "WM_NCLBUTTONDOWN" }


  ,{ WM_WINDOWPOSCHANGED , "WM_WINDOWPOSCHANGED"  }
  ,{ WM_WINDOWPOSCHANGING, "WM_WINDOWPOSCHANGING" }
  ,{ WM_MOVE             , "WM_MOVE"              }
  ,{ WM_GETTEXT          , "WM_GETTEXT"           }
  ,{ WM_GETMINMAXINFO    , "WM_GETMINMAXINFO"     }
  ,{ WM_ACTIVATE         , "WM_ACTIVATE"          }
  ,{ WM_NCPAINT          , "WM_NCPAINT"           }
  ,{ WM_SYNCPAINT        , "WM_SYNCPAINT"         }

  ,{ WM_PAINT            , "WM_PAINT" }
  ,{ WM_ERASEBKGND       , "WM_ERASEBKGND"     }
  ,{ WM_NCACTIVATE       , "WM_NCACTIVATE"     }
  ,{ WM_ACTIVATEAPP      , "WM_ACTIVATEAPP"    }
  ,{ WM_KILLFOCUS        , "WM_KILLFOCUS"      }
  ,{ WM_CAPTURECHANGED   , "WM_CAPTURECHANGED" }

  ,{              0, NULL              }};



  CodeRec * ptr;

  for( ptr= codes
     ; ptr->code
     ; ptr++  )
  { if ( ptr->code == code )
    { printf( "WINDOW EVENT (%05X) %4d -> %10s\n", h, ptr->code, ptr->explain );
      return;
  } }

  printf(     "WINDOW EVENT (%05X) UNKNOWN %4d\n", h, code );

}


#endif



#define TOGGLED_KEY 0x01 /* The key is toggled */
#define PRESSED_KEY 0x80 /* The key is pressed */

#ifndef IDC_ARROW
 #define IDC_ARROW    MAKEINTRESOURCE(32512)
#endif

#ifndef IDC_HAND
 #define IDC_HAND     MAKEINTRESOURCE(32649)
#endif

#ifndef IDC_SIZENWSE
 #define IDC_SIZENWSE MAKEINTRESOURCE(32642)
#endif


typedef struct
{ int system;
  int key;
} ConvKeyRec;

ConvKeyRec xlateTableKeys[]=
{{ VK_ESCAPE   , kbEsc        }
,{ VK_TAB      , kbTab        }
,{ VK_HOME     , kbHome       }
,{ VK_END      , kbEnd        }  /* EOL                     */
,{ VK_LEFT     , kbLeft       }  /* Move left, left arrow   */
,{ VK_RIGHT    , kbRight      }  /* Move right, right arrow */
,{ VK_UP       , kbUp         }  /* Move up, up arrow       */
,{ VK_DOWN     , kbDown       }  /* Move down, down arrow   */
,{ VK_PRIOR    , kbPgUp       }
,{ VK_NEXT     , kbPgDn       }
,{ VK_RETURN   , kbEnter      }  /* Return, enter           */
,{ VK_PAUSE    , kbPause      }  /* Pause, hold             */
,{ VK_DELETE   , kbDelete     }  /* Delete, rubout          */
,{ VK_INSERT   , kbInsert     }  /* Insert, insert here     */
,{ VK_PRINT    , kbPrnScr     }
,{ VK_BACK     , kbBackSpace  }  /* back space, back char   */

,{ VK_F1       , kbF1         }
,{ VK_F2       , kbF2         }
,{ VK_F3       , kbF3         }
,{ VK_F4       , kbF4         }
,{ VK_F5       , kbF5         }
,{ VK_F6       , kbF6         }
,{ VK_F7       , kbF7         }
,{ VK_F8       , kbF8         }
,{ VK_F9       , kbF9         }
,{ VK_F10      , kbF10        }
,{ VK_F11      , kbF11        }
,{ VK_F12      , kbF12        }
,{   0         , 0            }}; /* Terminator */


//static struct Win32Console * theOrigin;


static unsigned doMask( unsigned state )
{ return(( state & MK_SHIFT    ? kmShift    : 0 )
        |( state & MK_CONTROL  ? kmCtrl     : 0 )
        |( state & MK_LBUTTON  ? kmButton_1 : 0 )
        |( state & MK_MBUTTON  ? kmButton_2 : 0 )
        |( state & MK_RBUTTON  ? kmButton_3 : 0 )

#if(_WIN32_WINNT >= 0x0500)
        |( state & MK_XBUTTON1 ? kmButton_4 : 0 )
        |( state & MK_XBUTTON2 ? kmButton_5 : 0 )
#endif
        );
}

static int setKey( int vcode )
{ const ConvKeyRec * ptr;

  for( ptr= xlateTableKeys   /* Scan the special key table */
     ; ptr->system
     ; ptr++ )
  { if ( ptr->system == vcode )
    { return( ptr->key );
  } }
  return( 0 );
}




#ifndef _WIN32_WCE
//  WNDCLASSEX    classex;    // Datastructure for the windowclass
#endif

//  STARTUPINFO startinfo;

/** ================================================= [ JACS, 10/01/2006 ] == *\
 *                                                                            *
 *   JASC 2006                                                                *
 *                                                                            *
 *  FUNCTION getKeyMask                                                       *
 *                                                                            *
 *  @brief                                                                    *
 *                                                                            *
\* ========================================================================= **/
unsigned getKeyMask( )
{ return((( GetKeyState( VK_LSHIFT   ) & PRESSED_KEY ? kmLShift : 0 )
         |( GetKeyState( VK_RSHIFT   ) & PRESSED_KEY ? kmRShift : 0 )
         |( GetKeyState( VK_SHIFT    ) & PRESSED_KEY ? kmShift  : 0 )
         |( GetKeyState( VK_LCONTROL ) & PRESSED_KEY ? kmLCtrl  : 0 )
         |( GetKeyState( VK_RCONTROL ) & PRESSED_KEY ? kmRCtrl  : 0 )
         |( GetKeyState( VK_CONTROL  ) & PRESSED_KEY ? kmCtrl   : 0 )
         |( GetKeyState( VK_LMENU    ) & PRESSED_KEY ? kmAltL   : 0 )
         |( GetKeyState( VK_RMENU    ) & PRESSED_KEY ? kmAltR   : 0 )

         |( GetKeyState( VK_CAPITAL  ) & TOGGLED_KEY ? kmCaps   : 0 )
         |( GetKeyState( VK_INSERT   ) & TOGGLED_KEY ? kmInsert : 0 )
         |( GetKeyState( VK_SCROLL   ) & TOGGLED_KEY ? kmScroll : 0 )
         |( GetKeyState( VK_NUMLOCK  ) & TOGGLED_KEY ? kmNumLock: 0 )));
}


/** ================================================= [ JACS, 10/01/2004 ] == *\
 *                                                                            *
 *   JASC 2004                                                                *
 *                                                                            *
 *  METHOD Win32Console::doEvent.                                             *
 *                                                                            *
 *  @brief Main entry point for sourcer/sinker                                *
 *                                                                            *
\* ========================================================================= **/
static int w32Events( int theDisp, void * userData
                    , int      sz, void * list  )
{ struct w32Priv * w32= (struct w32Priv *)userData;

  return( 0 );
}


static int doEvent( HWND   hwnd
                  , UINT   message
                  , WPARAM wParam
                  , LPARAM lParam )
{ struct w32Priv * surface= (struct w32Priv *)nsfbFindSurface( NSFB_SURFACE_WIN32 );

  if ( surface )
  { struct w32List * win32;


/* Iterate node windows
 */
    for ( win32= surface->rtns.clients
        ; win32
        ; win32= win32->seed.next  )
    { if ( win32->theWindow == hwnd )
      { ToClientFun toClient= win32->seed.toClient;
        void      * toData  = win32->seed.toData;
        int button= 0;   // Sequencial
 // puts("NODE");

        switch ( message )
        { //case HABANA_MESSAGE:         // May 2007, automessage
          //  ((wobject *)thisEvent->lParam)->doMessage( NULL );    // say unknown originated message
          //return( HABANA_MESSAGE );

          case WM_ERASEBKGND:
          return( WM_ERASEBKGND );

          case WM_PAINT:
          { PAINTSTRUCT ps;
            /*HDC hdc=*/ BeginPaint( hwnd, &ps );

//            int rc;

          //        FillRect(win32->theGC, &ps.rcPaint, (HBRUSH) (COLOR_WINDOW+1));

            /*rc=*/ BitBlt( ps.hdc
                      , ps.rcPaint.left,  ps.rcPaint.top
                      , ps.rcPaint.right, ps.rcPaint.bottom
                      , win32->theGC
                      , ps.rcPaint.left,  ps.rcPaint.top
                      , SRCCOPY);

                //  FillRect(hdc, &ps.rcPaint, (HBRUSH) (COLOR_WINDOW+1));

            EndPaint( hwnd, &ps );
          }
          return( WM_PAINT );


          case WM_MOVE:
          return( 0 );

          case WM_SIZE:
          return( 0 );

          case WM_WINDOWPOSCHANGED:     // Keep track of window position & size
          return( WM_WINDOWPOSCHANGED );
    //  { WINDOWPOS newpos= * ((LPWINDOWPOS) thisEvent->lParam);

      //  int cx= ( newpos.cx - theWindow->origin.cx + theWindow->border.top  - theWindow->border.bottom ) / canvas->theFont->cell.w;
      //  int cy= ( newpos.cy - theWindow->origin.cy + theWindow->border.left - theWindow->border.right  ) / canvas->theFont->cell.h;

    //    if ( cx | cy )
    //    { cx *= canvas->theFont->cell.w; cx+= theWindow->origin.cx;
    //      cy *= canvas->theFont->cell.h; cy+= theWindow->origin.cy;

         // doResizeRequest( ResizeRequestEvent
           ///                ( canvas->theFont->cell.w
              //             , canvas->theFont->cell.h    // Reanchor widgets
                //           , cx, cy ));
     //     }

         // theWindow->origin= newpos;
//        }
          return( WM_WINDOWPOSCHANGED );

#if(_WIN32_WINNT >= 0x0500)
          case WM_XBUTTONDBLCLK: button++;
#endif
          case WM_MBUTTONDBLCLK: button++;
          case WM_RBUTTONDBLCLK: button++;
          case WM_LBUTTONDBLCLK: button++;
            toClient( toData
                    , NSFB_EVT_CLICK
                    , LOWORD( lParam ), HIWORD( lParam )
                    , button
                    , doMask( wParam ), 0 );
            break;

#if(_WIN32_WINNT >= 0x0500)
          case WM_XBUTTONDOWN: button++;
#endif
          case WM_MBUTTONDOWN: button++;
          case WM_RBUTTONDOWN: button++;
          case WM_LBUTTONDOWN: button++;
            toClient( toData
                    , NSFB_EVT_PRESS
                    , LOWORD( lParam ), HIWORD( lParam )
                    , button
                    , doMask( wParam ), 0 /* time */ );
            break;

            case WM_LBUTTONUP:
            case WM_MBUTTONUP:
            case WM_RBUTTONUP:
#if(_WIN32_WINNT >= 0x0500)
            case WM_XBUTTONUP:
#endif
              toClient( toData
                      , NSFB_EVT_REL
                      , LOWORD( lParam ), HIWORD( lParam )
                      ,   message == WM_LBUTTONDBLCLK ? 1
                        : message == WM_MBUTTONDBLCLK ? 2
                        : message == WM_RBUTTONDBLCLK ? 3
#if(_WIN32_WINNT >= 0x0500)
                        : message == WM_XBUTTONDBLCLK ? 4
#endif
                        : 0
                     , doMask( wParam ), 0 /* time */ );
            break;


            case WM_MOUSEMOVE://  puts("mousemove1");
              toClient( toData, NSFB_EVT_MOVE
                      , LOWORD( lParam ), HIWORD( lParam )
                      , doMask( wParam )
                      , 0, 0 );
            return( 1 );

            case WM_DESTROY:   // ago 2008, know closing window
    //    doClose();
            break;
  } } } }

  return( 0 );
}

/** ================================================= [ JACS, 10/01/2006 ] == *\
 *                                                                            *
 *   JASC 2006                                                                *
 *                                                                            *
 *  METHOD WConsole::atEvent                                                  *
 *                                                                            *
 *  @brief  Wraps the window loop to the standard console                     *
 *                                                                            *
\* ========================================================================= **/
LRESULT CALLBACK atEvent( HWND   hwnd
                        , UINT   message
                        , WPARAM wParam
                        , LPARAM lParam )
{ switch( message )           // Some messages need preprocess
  { //case WM_CREATE: return( 0;
    //case WM_SIZE:   return( 0;        // Set the size and position of the window.


    case WM_LBUTTONDOWN:       // Set the size and position of the window.
//      puts("butt");
    break;

    case WM_DESTROY:         // Clean up window-specific data objects.
      PostQuitMessage( 0 );
    return( 0 );
  }

  return( doEvent( hwnd      // Other events must be forwardwed
                 , message
                 , wParam
                 , lParam ) ? 0  // Distribute it among program windows
        : DefWindowProc( hwnd      // Other events must be forwardwed
                       , message
                       , wParam
                       , lParam ));
}


/** ================================================= [ JACS, 10/01/2012 ] == *\
 *                                                                            *
 *   JASC 2012                                                                *
 *                                                                            *
 *  FUNCTION initW32Display                                                   *
 *                                                                            *
 *  @brief Initializes a ne console                                           *
 *                                                                            *
\* ========================================================================= **/
static NsfbSurfaceRtns * initW32Display( struct w32Priv * drv )
{ int idx;
  PALETTEENTRY palette[ 256 ];

  drv->rtns.fd= -1;  /* Not use of, handlers, but event loop*/

  if ( ! theInstance )
  { static WNDCLASSEX theClass;

    theInstance= GetModuleHandle( NULL );

   // if ( !aModule )
   // { //FreeConsole();
   // }

    theClass.cbSize       = sizeof( theClass );
    theClass.style        = CS_DBLCLKS; // CS_HREDRAW | CS_VREDRAW | CS_PARENTDC ;       // no CS_DBLCLKS (Catch dbl-clicks)
    theClass.lpfnWndProc  = atEvent;
    theClass.cbClsExtra   = 0;
    theClass.cbWndExtra   = 0;
    theClass.hIcon        = LoadIcon( theInstance, IDI_APPLICATION );
    theClass.hIconSm      = LoadIcon( theInstance, IDI_APPLICATION );
    theClass.hCursor      = NULL; // LoadCursor( NULL, IDC_IBEAM );
    theClass.hbrBackground= NULL;
    theClass.lpszMenuName = NULL;
    theClass.lpszClassName= theClassName; // wclass
    theClass.hInstance    = theInstance;

/* Register the window class, if fail quit the program
 */
    if( !RegisterClassExA( &theClass ) )
    { return( NULL );
  } }

  drv->theScreen= CreateDC( "DISPLAY"            // Device capabilities
                          , (LPCSTR) NULL
                          , (LPCSTR) NULL
                          , (CONST DEVMODE *) NULL );

  if ( !drv->theScreen )                       // Can't open display, this is basic
  { return( NULL );
  }

//  drv->focused= NULL;
//  drv->pressed= NULL;

  drv->theHistogram= NewDeviceHistogram( drv->rtns.theDepth= GetDeviceCaps( drv->theScreen
                                                                          , BITSPIXEL )
                                       , drv->theScreen );

    switch( drv->rtns.theDepth ) // Now arrange the system it can work with all knonwn color planes
    { case 8:
        GetSystemPaletteEntries( drv->theScreen
                               , 0
                               , 256
                               , palette );

        for( idx= 0
           ; idx < 256
           ; idx ++ )
        { AddHistogram( drv->theHistogram
                      , palette[ idx ].peRed
                      , palette[ idx ].peGreen
                      , palette[ idx ].peBlue );
  } }

//    GetStartupInfoA ( &startinfo );   /* Get the command line passed to the process. */

  getKeyMask();
  drv->theHue= RGB( 0xFF, 0xDF, 0xCE );     // Default colorstyle

  drv->rtns.width = GetSystemMetrics( SM_CXSCREEN );
  drv->rtns.height= GetSystemMetrics( SM_CYSCREEN );

  return( &drv->rtns );
}


static int w32Initialise( struct w32List * drv )
{ if ( drv )
  { int styles= WS_OVERLAPPEDWINDOW
//              | WS_CHILD
              | WS_VISIBLE
              | WS_CLIPCHILDREN
              | WS_CLIPSIBLINGS;

    RECT  border=               // Jul 2006, w indow borders
    {    left: drv->seed.offx
    ,     top: drv->seed.offy
    ,   right: drv->seed.width
    ,  bottom: drv->seed.height
    };

    AdjustWindowRect( &border
                    , styles
                    , false );

    drv->theWindow= CreateWindowEx
    ( 0                            /* Possible styles   */
    , theClassName                 /* theClassName      */
    , drv->seed.theTitle

    , styles                       /* Window style      */

    , CW_USEDEFAULT, CW_USEDEFAULT /* Window dimensions */

    , drv->seed.width= ( border.right  - border.left )
    , drv->seed.height=( border.bottom - border.top  )

    , NULL                        // GetDesktopWindow() // HWND_DESKTOP                /* The window is a childwindow to desktop */
    , NULL                        /* No menu                                */
    , theInstance                 /* Program Instance handler               */
    , NULL );                     /* No Window Creation data                */

//    border.right -= w;
//    border.bottom-= h;

    if ( drv->theWindow )                 /* Fails to create (font creation assigns) */
    { ShowWindow  ( drv->theWindow          /* Make the window visible on the screen  */
                  , SW_SHOWNORMAL );

/*
 *    Windows annoys us by untransparently requesting display compatible
 *  memory buffer for bitmap operations
 *
 */
      HDC theWW= GetDC( drv->theWindow );

      drv->theGC= CreateCompatibleDC( theWW );
      drv->theMM= CreateCompatibleDC( theWW );

      HBITMAP thisBM;

     if ( 1 )
     { thisBM= CreateCompatibleBitmap( theWW
                                     , drv->seed.width
                                     , drv->seed.height );
     }
     else
     { BITMAPINFO bmi; memset(&bmi, 0, sizeof(BITMAPINFO));

       bmi.bmiHeader.biSize    =  sizeof(BITMAPINFOHEADER);
       bmi.bmiHeader.biWidth   =  drv->seed.width; // drv->theStride;
       bmi.bmiHeader.biHeight  =  drv->seed.height; // top-down
       bmi.bmiHeader.biPlanes  =  1;
       bmi.bmiHeader.biBitCount=  drv->seed.surfaceRtns->theDepth;
       bmi.bmiHeader.biCompression = BI_RGB;

      // thisBM= CreateDIBSection( theWW, &bmi, DIB_RGB_COLORS
        //                      , (void**)&drv->seed.theFrameBuffer
          //                    , NULL, NULL);
     }

      ReleaseDC( drv->theWindow, theWW );

      SelectObject( drv->theGC, thisBM );
      SelectObject( drv->theGC, GetStockObject( DC_PEN   ));
      SelectObject( drv->theGC, GetStockObject( DC_BRUSH ));

      HICON hicon = LoadIcon( theInstance
                            , MAKEINTRESOURCE( 101 ) );

      if ( hicon )
      { SendMessage( drv->theWindow, WM_SETICON, ICON_BIG,   (LPARAM)hicon );
        SendMessage( drv->theWindow, WM_SETICON, ICON_SMALL, (LPARAM)hicon );
      }

      UpdateWindow( drv->theWindow );
  } }

  drv->seed.plotterFns= &w32PlottersAddr;


  return( 0 );
}

/** ================================================= [ JACS, 10/11/2022 ] == *\
 *                                                                            *
 *   JASC 2012                                                                *
 *                                                                            *
 *  CALBACK w32Cursor                                                         *
 *                                                                            *
 *  @brief                                                                    *
 *                                                                            *
\* ========================================================================= **/
static int w32Cursor( Nsfb                * nsfb
                    , struct nsfbCursor_s * cursor )
{
  if ( cursor )
  {// LPCSTR WCursor;

//    if ( cursor0 != CURSOR_UNDEFINED )
//    { //if ( cursor0 == pointer )
//      {// return(0);
//    } }

/* IDC_APPSTARTING Standard arrow and small hourglass
   IDC_CROSS       Crosshair
   IDC_HAND        up point hand
   IDC_IBEAM       Text I-beam
   IDC_ICON        Windows NT only: Empty icon
   IDC_NO          Slashed circle
   IDC_SIZEALL     Same as

   IDC_SIZE        Double-pointed arrow pointing north and south
   IDC_SIZENS      Double-pointed arrow pointing north and south
   IDC_SIZEWE      Double-pointed arrow pointing west and east
   IDC_SIZENWSE
   IDC_SIZENESW
   IDC_UPARROW     Vertical arrow
   IDC_WAIT        Hourglass.
  */

   // pointer= cursor0;
   }
    return( 0 );
 }

/*  int setCaret( int what, int x, int y )
  { switch( what )
    { case HIDE_CARET:    HideCaret(   theHandle ); break;
      case CREATE_CARET:  CreateCaret( theHandle, 0, x, caretSize=y ); break;
      case DESTROY_CARET: DestroyCaret(); break;

      case MOVE_CARET:    SetCaretPos( x, y - caretSize );
      case SHOW_CARET:    ShowCaret( theHandle );
      break;
    }
    return( 0 );
   }
*/
/*
 *
 *
 */
static int w32Pan( Nsfb * nsfb, int type )
{ //struct win32Priv * w32= nsfb->surfaceRtns;
  //struct win32List * win= nsfb;

  switch ( type )
  { case NSFB_PAN_START : puts( "PAN START" );  break;

  //  case NSFB_PAN_SWITCH: return( w32Update( nsfb, NULL ));
    case NSFB_PAN_BSTORE: puts( "PAN BSTORE" ); break;
    case NSFB_PAN_DUMP  : puts( "PAN DUMP" );   break;

    default: puts( "PAN OTHER" );   return( -1 );
  }


  return( 0 );
}

//  { RECT rgn= { zone.posi.x, zone.size.w
 //             , zone.posi.y, zone.size.h };
//
//    rgn.left= rgn.right = zone.posi.x; rgn.right  += zone.size.w+1;
//    rgn.top = rgn.bottom= zone.posi.y; rgn.bottom += zone.size.h+1;
//
//    return( InvalidateRect( theHandle, &rgn, 1 ));
//  }
//
//  int pan( int type )
//  { switch( type )
//    { case 1:
//      { RECT rgn= {         0,         0
//                  , origin.cx, origin.cy };
//
 //       InvalidateRect( theHandle, &rgn, 1 );
//      }
//      break;
//
//      case 0: break;
//    }
//    return( 0 );
//  }


/** ================================================= [ JACS, 10/01/2012 ] == *\
 *                                                                            *
 *   JASC 2012                                                                *
 *                                                                            *
 *  METHOD Win32::setTitle                                                    *
 *                                                                            *
 *  @brief sets the title of a window.                                        *
 *                                                                            *
\* ========================================================================= **/
//  const char * setTitle( const char * title )
//  { return( SetWindowText( theHandle
//                         , title ) ? title : NULL );
//  }



/** ================================================= [ JACS, 10/01/2006 ] == *\
 *                                                                            *
 *   JASC 2006                                                                *
 *                                                                            *
 *  METHOD Win32Console::doEvent                                              *
 *                                                                            *
 *  @brief  Distribute events among windows                                   *
 *                                                                            *
\* ========================================================================= **/
int doEvent1( int type, int wParam )
{ dword keyCode, keyMask;

 // ToClientFun toClient= NULL;//= ptr->seed.toClient;
//  void      * toData  = NULL; //=   ptr->seed.toData;


/* Lock first for key events, not related to canvases
 */

  switch ( type )  // Key events belong to focused widget
  { case WM_CHAR:
    case WM_KEYDOWN:
    case WM_SYSCHAR:
    case WM_SYSKEYDOWN:
      keyMask= getKeyMask();               // Adapt keymask to TVISION

      switch ( type )   // Test again
      { case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
          if ( keyMask & kmAltered )
          { if ( wParam >= ' ' )         /* key + alt or ctrl */
            { if ( wParam <= 'Z' )
              { //return( doKeyPress( wParam
                  //                , keyMask ));
          } } }

          if (( keyCode= setKey( wParam )))
          { if ( keyMask & kmAltR )         // Emulate alt for system keys
            { keyMask &= ~kmAltR;
              keyMask |=  kmAltL;
            }
            else
            { if ( keyMask & kmAltL )       // Windows event ?
              { return( 0 );
            } }

        if (( keyCode > 0XFFFF ))
        { // return( doKeyPress( keyCode, keyMask ));
        } }
      return( 0 );

      case WM_SYSCHAR:  /* (fixme) Block windows system keys, fix: not all of then must be blocked */
      return( 1 );

      case WM_CHAR:
      break;
     // return( doKeyPress( thisEvent->wParam, keyMask ));
    } }


    switch( type )
    { case WM_ACTIVATE:          // The window becomes top level
//            theActive= canvasPtr;
  //          puts( "activate"               );
      return( 0 );

      case WM_MOUSEMOVE:
     //   toClient( toData, NSFB_EVT_MOVE
       //         , LOWORD( lParam ), HIWORD( lParam )
         //       , doMask( wParam )
           //     , 0, 0 );
      break;

    //  default: return( doEvent( canvasPtr, thisEvent ));
    }

    return( 0 );    // No program window found, bypass event
}

static int w32Finalise( Nsfb * nsfb )
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
/*
 *
 *
 */
static int w32Update( Nsfb      * nsfb
                    , NsfbBbox  * box )
{ struct w32List * win= (struct w32List *)nsfb;

  int i= InvalidateRect( win->theWindow, NULL, FALSE );

  MSG messages;            /* Here messages to the application are saved */

/* Run the message loop. It will run until GetMessage() returns 0
 */
  while (GetMessage (&messages, NULL, 0, 0))
  { TranslateMessage(&messages); /* Translate virtual-key messages into character messages */
    DispatchMessage(&messages); /* Send message to WindowProcedure */
  }

  return( 0 );
}

/*
 *
 */
static int w32Claim( Nsfb *nsfb, NsfbBbox * box )
{ //struct nsfbCursor_s *cursor= nsfb->cursor;
       /*
     */
  return( 0 );
}


static int w32Pixmap( NsfbSurfaceRtns * surf
                    , ImageMap * map
                    , void     * data
                    , void     * mask
                    , int bmpWidth, int bmpHeight )
{ if ( surf )
  { if ( data )
    { int planes= /*nsfb->seed.bpp*/32;
      int sz= bmpWidth * bmpHeight;

      switch( planes )
      { case 16:
        { byte * src= (byte *)data;
          word * dst= (word *)data;

          while( sz-- )
          { word b= *src++;              b >>= 3;  // alpha channel
            word g= *src++;  g &= ~0x03; g <<= 3;
            word r= *src++;  r &= ~0x07; r <<= 8;
            src++; *dst++= r | g | b;
        } }
        break;

        case 32: if ( !mask )
        { dword * ptr= (dword *)data;

          while( sz-- )
          { dword back= *ptr;

            byte r= back; back >>= 8;
            byte g= back; back >>= 8;
            byte b= back; back >>= 8;

                        back  = r;
            back <<= 8; back |= g;
            back <<= 8; back |= b;

            *ptr++= back;
          } }
          break;
      }


      map->image= CreateBitmap( bmpWidth, bmpHeight
                              , 1, planes
                              , data );
      map->mask=  mask
               ? CreateBitmap( bmpWidth, bmpHeight   /* Size   */
                              , 1, 1                  /* deep   */
                              , mask ) : NULL;               /* raster */
  } }

  return( 0 );
}


/*
static int getImageWidth( ImageMap * map )
{ BITMAP info;

  if ( !GetObject( map->image                  // Need be in memory
                 , sizeof( info )
                 ,        &info ))
  { return( 0 );
  }

  return( info.bmWidth );
}

static int getImageHeight( ImageMap * map )
{ BITMAP info;

  if ( !GetObject( map->image                  // Need be in memory
                 , sizeof( info )
                 ,        &info ))
  { return( 0 );
  }

  return( info.bmHeight );
}
*/


/*
 *
 */
NsfbSurfaceRtns * newNode( const char * mode )
{ static int dispIdx= 0;

  struct w32Priv * node= ( struct w32Priv * ) CALLOC( sizeof( struct w32Priv ));

  if ( node )
  { node->rtns.type      = NSFB_SURFACE_WIN32 | ( dispIdx++ << 16 );
    node->rtns.name      = strdup( mode );
    node->rtns.pan       = w32Pan;
    node->rtns.events    = w32Events;
    node->rtns.claim     = w32Claim;
    node->rtns.update    = w32Update;
    node->rtns.cursor    = w32Cursor;
    node->rtns.finalise  = w32Finalise;
    node->rtns.initialise= w32Initialise;

    node->rtns.pixmap     = w32Pixmap;

  //  node->rtns.geometry  = w32SetGeometry;
    node->rtns.dataSize  = sizeof( struct w32List );

    return( initW32Display( node ) );  /* Unique identificator */
  }

  return( NULL );
}













