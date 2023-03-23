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

typedef struct ImageMap_s
{ void * image;
  void * mask;
} ImageMap;



#ifdef __cplusplus
  #define PUBLIC extern "C"
  #define ANSIC extern "C"
#else
  #define PUBLIC
  #define ANSIC
#endif

#define ARGUSED __attribute__((unused))
#define UNUSED(x) ((x) = (x))


typedef struct NsfbPalette        NsfbPalette;
typedef struct NsfbCursorSt       NsfbCursor;
typedef struct NsfbSt             Nsfb;
typedef struct NsfbSurfaceRtnsSt  NsfbSurfaceRtns;
typedef struct FontListRecSt      FontListRec;

typedef struct DeviceImageRec_s  DeviceImageRec;
typedef struct HistRec_s         HistRec;

/** co-ordinate for plotting operations
 */
typedef struct NsfbPointStruct
{ int x;
  int y;
} NsfbPoint;

typedef struct NsfbSizeStruct
{ int w;
  int h;
} NsfbSize;

/** co-ordinate for plotting operations
 */
typedef struct NsfbAreaStruct
{ NsfbPoint posi;
  NsfbSize  size;
} NsfbArea;

/** bounding box for plotting operations
 */
typedef struct nsfb_bbox_s
{ int x0; int y0;
  int x1; int y1;
} NsfbBbox;


/** Icon construction
 */
typedef struct ImgPaletteStruct
{ unsigned char red;
  unsigned char green;
  unsigned char blue;
  unsigned char alpha;
} ImgPalette;

typedef struct IcoRecStruct
{ unsigned char  * pic;   /* Points to the image data */
  unsigned short * frm;   /* Frame latency            */
  unsigned short   wNat;  /* Native size              */
  unsigned short   hNat;
  unsigned short   pics;  /* Tile */
  unsigned int     nCol;  /* Number or colors         */
  ImgPalette      * pal;
} IcoRec;

typedef struct  VectorRecStruct
{ int width, height;

	 struct svgtiny_shape *shape;
	 unsigned int shape_count;

	 unsigned short error_line;
	 const char *error_message;
} VectorRec ;


/** representation of a colour.
 *
 * The colour value comprises of four components arranged in the order ABGR:
 * bits 24-31 are the alpha value and represent the opacity. 0 is
 *   transparent i.e. there would be no change in the target surface if
 *   this colour were to be used and 0xFF is opaque.
 *
 * bits 16-23 are the Blue component of the colour.
 *
 * bits 8-15 are the Green component of the colour.
 *
 * bits 0-7 are the Red component of the colour.
 */
typedef unsigned int NSFBCOLOUR;


#define    NOCOLOR 0xFFFFFFFF
#define    CPCOLOR 0xEFFFFFFF
#define    PRCOLOR 0xDFFFFFFF
#define ALPHACOLOR 0xFF000000

enum
{ NSFB_FONT_FIXEDSYS = 0
, NSFB_FONT_COURIER  = 1
, NSFB_FONT_LUCIDA   = 2
, NSFB_FONT_ANDALE   = 3
, NSFB_FONT_TERMINAL = 4
, NSFB_FONT_HELVETICA= 5
, NSFB_FONT_TIMES    = 6
, NSFB_FONT_UTOPIA   = 7 };

enum NsfbKeys
{ kbTab      = 0x10000  /* One more that possible unicodes */
, kbPause               /* Pause, hold                     */
, kbEsc
, kbDelete              /* Delete, rubout                  */
, kbPrnScr
, kbBackSpace           /* back space, back char           */
, kbUp                  /* Move up, up arrow               */
, kbEnd                 /* EOL                             */
, kbDown                /* Move down, down arrow           */
, kbHome
, kbLeft                /* Move left, left arrow           */
, kbRight               /* Move right, right arrow         */
, kbPgUp
, kbPgDn
, kbInsert              /* Insert, insert here             */
, kbF1
, kbF2
, kbF3
, kbF4
, kbF5
, kbF6
, kbF7
, kbF8
, kbF9
, kbF10
, kbF11
, kbF12

, kbSysReqPress
, kbCancel          /* Clear to home */

, kbEnter= '\r'
};


enum ButtonKeyModifiers
{ kmNone    = 0x0000
, kmLShift  = 0x0001
, kmRShift  = 0x0002
, kmShift   = kmLShift | kmRShift
, kmLCtrl   = 0x0004
, kmRCtrl   = 0x0008
, kmCtrl    = kmLCtrl  | kmRCtrl
, kmAltL    = 0x0010
, kmAltR    = 0x0020
, kmAlt     = kmAltR   | kmAltL
, kmCaps    = 0x0040
, kmScroll  = 0x0080
, kmNumLock = 0x0100
, kmWindows = 0x0200
, kmInsert  = 0x0400

, kmButton_1  = 0x0800
, kmButton_2  = 0x1000
, kmButton_3  = 0x2000
, kmButton_4  = 0x4000
, kmButton_5  = 0x8000

, kmAltered   = kmCtrl | kmAlt | kmWindows


};

enum MouseButtons
{ HBUTTON_NONE= 0
, HBUTTON_1   = 1
, HBUTTON_2   = 2
, HBUTTON_3   = 3
, HBUTTON_4   = 4
, HBUTTON_5   = 5
};



/** The type of framebuffer surface.
 */
enum NsfbType
{ NSFB_SURFACE_NONE = 0x0000   /** No surface                */
, NSFB_SURFACE_RAM  = 0x0001   /** RAM surface               */
, NSFB_SURFACE_SDL  = 0x0002   /** SDL surface               */
, NSFB_SURFACE_LINUX= 0x0004   /** Linux framebuffer surface */
, NSFB_SURFACE_VNC  = 0x0008   /** VNC surface               */
, NSFB_SURFACE_ABLE = 0x0010   /** ABLE framebuffer surface  */
, NSFB_SURFACE_X11  = 0x0020   /** X windows surface         */
, NSFB_SURFACE_WL   = 0x0040   /** Wayland surface           */
, NSFB_SURFACE_FBUFF= 0x0080   /** Internal framebuffer      */
, NSFB_SURFACE_DRM  = 0x0100   /** borrowed from <david.rheinsberg@gmail.com>  */
, NSFB_SURFACE_EGL  = 0x0200   /** borrowed from <david.rheinsberg@gmail.com>   */
, NSFB_SURFACE_WIN32= 0x0400   /** ms surface       */

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

enum NsfbPanning
{ NSFB_PAN_START
, NSFB_PAN_SWITCH
, NSFB_PAN_BSTORE
, NSFB_PAN_DUMP
};

enum NsfbEvents
{ NSFB_EVT_SHOW
, NSFB_EVT_FOCUSIN
, NSFB_EVT_FOCUSOT
, NSFB_EVT_UNMAP
, NSFB_EVT_MAP
, NSFB_EVT_EXPOSE
, NSFB_EVT_NEXPOSE
, NSFB_EVT_SCLEAR
, NSFB_EVT_SREQ
, NSFB_EVT_PROPERT
, NSFB_EVT_DESTROY
, NSFB_EVT_RESIZE
, NSFB_EVT_ENTER
, NSFB_EVT_LEAVE
, NSFB_EVT_MOVE     /* Move pointer */
, NSFB_EVT_PRESS    /* Press button */
, NSFB_EVT_REL      /* RElease button */
, NSFB_EVT_CLICK

, NSFB_EVT_KPRESS   /* Press key  */
, NSFB_EVT_KREL     /* Release key */
, NSFB_EVT_KCLICK   /* Click key */

, NSFB_MAP_NOT

, NSFB_EVT_CREATE_WIN
, NSFB_EVT_MAP_REQ
, NSFB_EVT_REPARENT_NOT
, NSFB_EVT_COFIGURE_REQ
, NSFB_EVT_KEYPRESS
, NSFB_EVT_KEYRELEASE
, NSFB_EVT_RESIZE_REQ
, NSFB_EVT_CLIENT_MESS
, NSFB_EVT_GRAPH_EXPOSE
, NSFB_EVT_GRAVITY_NOT
, NSFB_EVT_CIRCULATE_NOT
, NSFB_EVT_CIRCULATE_REQ
, NSFB_EVT_SELECTION_NOT
, NSFB_EVT_COLORMAP_NOT

, NSFB_EVT_JOYSTICK

};


enum NsfbRotate
{ NSFB_ROTATE_NORTH= 0x00
, NSFB_ROTATE_EAST = 0x01
, NSFB_ROTATE_SOUTH= 0x02
, NSFB_ROTATE_WEST = 0x03
, NSFB_ROTATE_KEEP = -1
};


#define GETFMITEM( l, o, i  ) (( l | ( o << 1 )) << ( i << 3 ))


enum NsfbFormat
{ NSFB_FMT_ANY     =          0  /* No specific format - use surface default */
, NSFB_FMT_XBGR8888= 0x30081828  /* 32bpp Blue Green Red */
, NSFB_FMT_XRGB8888= 0x30281808  /* 32bpp Red Green Blue */
, NSFB_FMT_ABGR8888= 0x38081828  /* 32bpp Alpha Blue Green Red */
, NSFB_FMT_ARGB8888= 0x38281808  /* 32bpp Alpha Red Green Blue */
, NSFB_FMT_RGB888  = 0x00081828  /* 24 bpp Alpha Red Green Blue */
, NSFB_FMT_BRB888  = 0x00281808  /* 24 bpp Alpha Red Green Blue */
, NSFB_FMT_ARGB1555= 0x31251505  /* 16 bpp 555 */
, NSFB_FMT_RGB565  = 0x00251505  /* 16 bpp 565 */
, NSFB_FMT_I8      =          8  /* 8bpp indexed */
, NSFB_FMT_I4      =          4   /* 4bpp indexed */
, NSFB_FMT_I1      =          1   /* black and white */

, NSFB_FMT_MASK    = 0x3FFFFFFF  /* JACS, format mask */

, NSFB_FMT_ACTIVATE= 0x40000000  /* JACS, add flags */
};

#define APPLY_ROTATE1( holder, xd, yd, xs, ys ) \
{ switch( holder->theGeo )                                      \
  { case NSFB_ROTATE_NORTH: xd=               xs; yd=                ys; break; \
    case NSFB_ROTATE_WEST:  xd=               ys; yd= holder->height- xs; break; \
    case NSFB_ROTATE_SOUTH: xd= holder->width-xs; yd= holder->height-ys; break; \
    case NSFB_ROTATE_EAST : xd= holder->width-ys; yd=                xs; break; \
} }





/** Select frontend type from a name.
 *
 * @param name The name to select a frontend ( console altomaltes ).
 * @return The surface type or NSFB_SURFACE_NONE if frontend with specified
 *         name was not available
 */
ANSIC enum NsfbType nsfbTypeFromName( const char *name );
ANSIC const char * nsfbDemangleName(  const char * name
                                   , char * display
                                   , int * w, int * h, int * bpp
                                   , int * x, int * y, int * geo );


/** Create a nsfb context.
 *
 * This creates a framebuffer surface context.
 *
 * @param surface_type The type of surface to create a context for.
 */
ANSIC Nsfb *nsfbNew( const enum NsfbType );

/** Initialise selected surface context.
 *
 * @param nsfb The context returned from ::nsfbInit
 */
ANSIC    int   nsfbInit( Nsfb * );

//ANSIC Nsfb * nsfbOpen( const char * mode
  //                    , int offx, int offy
    //                  , int   w, int h, int  z );     // JACS
ANSIC Nsfb * nsfbOpenAscii( const char * mod ); // JACS

/** Free nsfb context.
 *
 * This shuts down and releases all resources associated with an nsfb context.
 *
 * @param nsfb The context returned from ::nsfbNew to free
 */
ANSIC int nsfbFree( Nsfb * );

/** Claim an area of screen to be redrawn.
 *
 * Informs the nsfb library that an area of screen will be directly
 * updated by the user program. This is neccisarry so the library can
 * ensure the soft cursor plotting is correctly handled. After the
 * update has been perfomed ::nsfbUpdate should be called.
 *
 * @param box The bounding box of the area which might be altered.
 */
ANSIC int nsfbClaim( Nsfb *, NsfbBbox * box );

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
ANSIC int nsfbUpdate( Nsfb *nsfb, NsfbBbox * );

/** Obtain the geometry of a nsfb context.
 *
 * @param width a variable to store the framebuffer width in or NULL
 * @param height a variable to store the framebuffer height in or NULL
 * @param format a variable to store the framebuffer format in or NULL
 */
ANSIC const char * nsfbSetAttrib( Nsfb * nsfb
                                 , const char * title );

ANSIC void * nsfbGetBackStore( Nsfb * nsfb                // JACS, 2022
                              , int  * width, int * height
                              , enum NsfbFormat * format );


/** Alter the geometry of a surface
 *
 * @param nsfb The context to alter.
 * @param width The new display width.
 * @param height The new display height.
 * @param format The desired surface format.
 */
ANSIC int nsfbSetGeometry( Nsfb *
                          , int width, int height
                          , enum NsfbFormat format );


/** Alter the position of a surface
 *
 * @param nsfb The context to alter.
 * @param x The new display width.
 * @param y The new display height.
 * @param geo The desired orientation
 */
ANSIC int nsfbSetPosition( Nsfb *
                          , int x, int y, int geo );

ANSIC Nsfb * nsfbNewConsole( enum NsfbType type
                            , const char * ascii );

ANSIC Nsfb * nsfbNewSurface( enum NsfbType type
                            , int w, int h, int plan
                            , int x, int y, int geo
                            , const char * title );
/* altomaltes
 */
ANSIC  DeviceImageRec * nsfbNewImage( DeviceImageRec img );

ANSIC  bool   nsfbPlotImage( Nsfb * nsfb
                           , int x, int y
                           , DeviceImageRec * );

ANSIC int  nsfbKillImage( DeviceImageRec * );


/* user must feed mouse and so events, to avoid dependencies
 */
typedef int ( *NsfbSurfacefnEvents )( int theDisp, void * userData
                                    , int      sz, void * list  );

ANSIC int                 nsfbSetPan(          Nsfb *, int type );

ANSIC int                 nsfbGetEventHandler( struct NsfbSurfaceRtnsSt * );
ANSIC NsfbSurfacefnEvents nsfbGetEventSinker(  struct NsfbSurfaceRtnsSt * );
ANSIC NsfbSurfacefnEvents nsfbGetEventCursor(  struct NsfbSurfaceRtnsSt * );
ANSIC int                 nsfbGetDepth(        struct NsfbSurfaceRtnsSt * );
ANSIC int                 nsfbGetWidth(        struct NsfbSurfaceRtnsSt * );
ANSIC int                 nsfbGetHeight(       struct NsfbSurfaceRtnsSt * );
ANSIC int                 nsfbGetHard(         struct NsfbSurfaceRtnsSt * );


ANSIC void *              nsfbSetEventSourcer( Nsfb * nsfb, void * code, void * data );




/** Obtain the buffer memory base and stride.
 *
 * @param nsfb The context to read.
 */
ANSIC int nsfbGetBuffer( Nsfb *
                        , unsigned char **ptr
                        , int * linelen );


/** Obtain the buffer memory stride.
 *
 * @param nsfb The context to read.
 */
ANSIC int nsfbGetStride( Nsfb * nsfb );

ANSIC void * nsfbGetGeometry( Nsfb *
                            , int  * width
                            , int  * height
                            , enum NsfbFormat * format);



/** Dump the surface to fd in PPM format
 */
ANSIC bool nsfbDump( Nsfb *nsfb, int fd );

ANSIC enum NsfbType nsfbGetSurfaceType( Nsfb * );

ANSIC int vtswitchopen(  );


/*
 */

ANSIC NsfbSurfaceRtns * nsfbSurfaceDefaultRtns( NsfbSurfaceRtns * );
ANSIC NsfbSurfaceRtns * nsfbFindSurface( int type );

ANSIC int nsfbGetImageGeometry( DeviceImageRec * img
                               , int * w, int * h, int * p );

ANSIC unsigned long parsedColor( const char * name );

ANSIC int printerSize( const char * fmt );
ANSIC char Printer( void (*outer)( void *, unsigned char )
                  , void * args
                  , const char * fmt, va_list list );   // String version


/* Image loading
 */
ANSIC int    getDeviceImage( NsfbSurfaceRtns  *
                           , ImageMap         *
                           , void * img, void * msk
                           , int w, int h );

ANSIC DeviceImageRec * getDeviceBitmap( Nsfb           *
                                      , DeviceImageRec * );

ANSIC DeviceImageRec * getIconImage( NsfbSurfaceRtns * surf
                                   , IcoRec  * ico
                                   , NSFBCOLOUR hue );

ANSIC HistRec * NewDeviceHistogram( int    planes
                                  , void * device );

ANSIC         IcoRec * loadIcoXpmFile( const char * fName, int wide, int height );
ANSIC         IcoRec * loadIcoGifFile( const char * fName, int wide, int height );
ANSIC         IcoRec * loadIcoIcoFile( const char * fName, int wide, int height );
ANSIC         IcoRec * loadIcoXpmFile( const char * fName, int wide, int height );
ANSIC         IcoRec * loadIcoJpgFile( const char * fName, int wide, int height );
ANSIC         IcoRec * loadIcoPngFile( const char * fName, int wide, int height );
ANSIC         IcoRec * loadIcoSvgFile( const char * fName, int wide, int height );
ANSIC         IcoRec * loadIcoVecFile( const char * fName, int wide, int height ); /* Vector to raster */

ANSIC DeviceImageRec * loadImgXpmFile( const char * fName, int wide, int height );
ANSIC DeviceImageRec * loadImgGifFile( const char * fName, int wide, int height );
ANSIC DeviceImageRec * loadImgIcoFile( const char * fName, int wide, int height );
ANSIC DeviceImageRec * loadImgXpmFile( const char * fName, int wide, int height );
ANSIC DeviceImageRec * loadImgJpgFile( const char * fName, int wide, int height );
ANSIC DeviceImageRec * loadImgPngFile( const char * fName, int wide, int height );
ANSIC DeviceImageRec * loadImgSvgFile( const char * fName, int wide, int height );

ANSIC IcoRec         * LoadIcoFile(    const char * fName, int wide, int height ); /* general */
ANSIC DeviceImageRec * LoadImgFile(    const char * fName, int wide, int height ); /* general */
ANSIC VectorRec      * LoadVecFile(    const char * fName, int wide, int height ); /* general */




/* Font rendering
 */
ANSIC  FontListRec * getFreeFont( int fontIdx
                               , int w, int h /*, dword outline */ );

ANSIC int nsfbFillString( Nsfb        * nsfb
                        , FontListRec * font
                        , const int x, const int y      // Draws "chess" text
                        , const char * str
                        , NSFBCOLOUR fore
                        , NSFBCOLOUR back
                        , NSFBCOLOUR outline  );


ANSIC  void ** getChar(   void *, unsigned char idx );

ANSIC int nsfbAttEvent( int sk, void * userData
                      , int sz, void * stream  );



#define CALLOC( sz  ) calloc( sz, 1 )
#define FREE(   ptr ) if ( ptr ) { free( ptr); ptr= NULL; }


#endif

/*
 * Local variables:
 *  c-basic-offset: 4
 *  tab-width: 8
 * End:
 */
