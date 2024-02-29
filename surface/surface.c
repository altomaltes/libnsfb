/*
 * Copyright 2009 Vincent Sanders <vince@simtec.co.uk>
 *
 * This file is part of libnsfb, http://www.netsurf-browser.org/
 * Licenced under the MIT License,
 *                http://www.opensource.org/licenses/mit-license.php
 */

#define _GNU_SOURCE

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>

#include "../surface.h"
#include "../plot.h"
#include "../nsfb.h"

#define PATH_SIZE 256

typedef NsfbSurfaceRtns * ( *NodeFun ) ( const char * mode                           ); /* dll mouter */
typedef IcoRec          * ( *IcoFun  ) ( const char * name, int wtarget, int htarget ); /* dll mouter */
typedef DeviceImageRec  * ( *ImgFun  ) ( const char * name, int wtarget, int htarget ); /* dll mouter */
typedef               int ( *DumpFun ) ( Nsfb *, const char * name  );                  /* dll mouter */


static NsfbSurfaceRtns * surfaceSeed= NULL;

/** ================================================ [ JACS, 10/12/2009 ] == **\
 **                                                                          **
 **   JACS 2009 (altomaltes@yahoo.es)                                        **
 **             (altomaltes@gmail.com)                                       **
 **                                                                          **
 **  FUNCION: loadModuleSymbolNsfb                                               **
 **                                                                          **
 **  @brief Loads a symbol from a plugin on the same path of symbol.         **
 **                                                                          **
\** ======================================================================== **/
#if defined( __unix__ )
#include <dlfcn.h>

static void * loadModuleSymbolNsfb( const char * name
                                  , const char * dll, ...  )
{ Dl_info info;

  if ( dladdr( loadModuleSymbolNsfb, &info ) )
  { if ( info.dli_fname )
    { char path[ PATH_SIZE ];

      strcpy( path, info.dli_fname ); char * mark= strrchr( path, '/' );

      if ( mark )
      {
        mark++;              /* Cut path, but leave "/" */
        void * addr;
        void * hOle2Dll;

        va_list ap; va_start( ap, dll);
        vsprintf( mark, dll, ap );
        va_end( ap );
        strcat( mark, ".so" );

        if (( hOle2Dll= dlopen( path, RTLD_NOW )))
        { if (( addr= dlsym( hOle2Dll, name )))
          { fprintf( stderr, "Open %s in %s sucess\n", name, path );
            return( addr );
        } }

        fprintf( stderr, "Not able to open %s in %s -> %s\n", name, path, dlerror() );

  } } }

  return( NULL );
}

#elif defined( __WIN32 )

#undef byte            // namespace corruption
  #include "windows.h"

ANSIC void * loadModuleSymbolNsfb( const char * name
                                 , const char * dll, ...  )
{ char lib[ 1024 ];
  void * hOle2Dll;

  if ( dll )
  { va_list ap; va_start( ap, dll);
    vsprintf( lib, dll, ap );
    va_end( ap );

    if ( !strstr( dll, ".DLL" ) )
    { if ( !strstr( dll, ".dll" ) )
      { strcat( lib, ".DLL" );
    } }

    hOle2Dll= LoadLibrary( dll= lib );
  }
  else
  { hOle2Dll= GetModuleHandle( name );
  }

  if ( hOle2Dll )
  { if ( name )                               /* A symbol was requested */
    { void * addr= GetProcAddress( hOle2Dll, name );

      if ( addr )
      { return( addr );
    } }

    FreeLibrary( hOle2Dll );
  }
  return( NULL );
}

#endif


/* internal routine which lets surfaces register their presence at runtime
 */
void _nsfb_register_surface( NsfbSurfaceRtns * rtns )
{ rtns->next= surfaceSeed;
  surfaceSeed= rtns;
}


struct NsfbSurfaceRtnsSt * nsfbFindSurface( int type )
{ NsfbSurfaceRtns * ptr;

  for( ptr= surfaceSeed
     ; ptr
     ; ptr= ptr->next )
  { if ( ptr->type == type )
    { return(( struct NsfbSurfaceRtnsSt *)ptr );
  } }

  return( NULL );
}

/* default surface implementations
 */

static int surfaceClaim( Nsfb *nsfb, NsfbBbox *box )
{ UNUSED( nsfb );
  UNUSED( box  );

  return( 0 );
}

/* memory updates
 */
static int surfaceUpdate( Nsfb     * nsfb
                        , NsfbBbox * box )
{ if ( nsfb && box )
  { return( nsfb->plotterFns->moverect( nsfb
                                      , box->x1 - box->x0
                                      , box->y1 - box->y0
                                      , box->x0 , box->y0 ));
  }

  return( -1 );
}

ANSIC int nsfbSnap( Nsfb       * nsfb  )
{ if ( nsfb  )
  { return( nsfb->plotterFns->moverect( nsfb
                                      , nsfb->width
                                      , nsfb->height
                                      , 0 , 0 ));
  }

  return( 0 );
}

static int surfaceCursor(Nsfb *nsfb, struct NsfbCursorSt *cursor)
{ UNUSED( nsfb   );
  UNUSED( cursor );
  return 0;
}

static int surfaceGeometry( Nsfb * nsfb
                          , int width, int height
                          , enum NsfbFormat format )
{ nsfb->width = width;
  nsfb->height= height;
  nsfb->format= format;

  return( 0 );
}

ANSIC enum NsfbType nsfbGetSurfaceType( Nsfb *nsfb )
{ if ( nsfb )
  { if ( nsfb->surfaceRtns )
    { return( nsfb->surfaceRtns->type );
  } }

  return( NSFB_SURFACE_NONE );
}

/* exported interface documented in surface.h
 */
/* surface type must match and have a initialisor, finaliser
 * and input method
 */
/*    The rest may be empty but to avoid the null check every time
 * provide default implementations.
 */

NsfbSurfaceRtns * nsfbSurfaceDefaultRtns( NsfbSurfaceRtns * rtns )
{ if (( rtns->initialise )
  &&  ( rtns->finalise   ))
  { if ( !rtns->claim    ) { rtns->claim   = surfaceClaim;	   }
    if ( !rtns->update   ) { rtns->update  = surfaceUpdate;	  }
    if ( !rtns->cursor   ) { rtns->cursor  = surfaceCursor;	  }
    if ( !rtns->geometry ) { rtns->geometry= surfaceGeometry; }
//    if ( !rtns->clg      ) { rtns->geometry= surfaceClg;      }

    if ( !rtns->theDepth) { rtns->theDepth= 32; }

    return( rtns );
  }

  return( NULL );
}


/*   EXAMPLE ":0#800x600x24.N@X11" -> ":0" "800x600x24.N" "X11"
 */
ANSIC Nsfb * nsfbNewSurface( enum NsfbType type
                           , int w, int h, int plan
                           , int x, int y, int geo
                           , const char * title )
{ Nsfb * newfb= nsfbNew( type );

  if ( !newfb )
  { fprintf( stderr, "Unable to allocate \"%d %d %s\" new nsfb surface\n", w, h, title );
  }
  else
  { nsfbSetGeometry( newfb         /* Before nsfbInit, because of the size */
                   , w, h, plan );

    nsfbSetAttrib(  newfb
                 ,  title );
    nsfbInit( newfb );
    nsfbSetPosition( newfb
                   , x, y, geo );
  }

  return( newfb );
}

/*
 */
ANSIC Nsfb * nsfbNewConsole( enum NsfbType type
                           , const char * ascii )
{ const char * mk= (const char *)strchr( ascii, '#' );

  mk= mk ? mk +1  : ascii;

  int w= 800;  /* Some default values */
  int h= 600;
  int x= 0
    , y= 0;

  int plan=  32;
  char geo= 'N';

  sscanf( mk, "%dx%dx%d.%c;%d,%d"
        , &w, &h, &plan, &geo, &x, &y );

  return( nsfbNewSurface( type
                        , w, h, plan
                        , x, y, geo
                        , "nsfb library" ));

}

/*
 */
const char * nsfbDemangleName( const char * name
                             , char       * display
                             , int * w, int * h, int * bpp
                             , int * x, int * y, int * geo )
{ if ( name )
  { int thisW= -1;
    int thisH= -1;   /* default values */
    int thisX= -1;
    int thisY= -1;
    int thisBpp= 32;
    int thisGeo= NSFB_ROTATE_NORTH;

    const char * thisDriver= NULL;

/* Extract display
 */
    const char * src= name;
    char * dst= display ? display : (char*)alloca( 256 );

    while(( *src != '#' ) && ( *src != '@' ))
    { if ( *src == 0)         /* Invalid stream    */
      { //if ( *name == ':' )    /* Last resort, tell most probable */

        if ( dst )
        { *dst= 0; }
        #ifdef __unix__
          return( "X11" );
        #else
          return( "w32" );
        #endif
      }
      if ( dst )
      { *dst++= *src;
      } src++;
    }; if ( dst ) { *dst= 0; }             /* Terminate display */

    switch( *src )
    { case '@':                         /* No display info passed */
        src= name;
        dst= display;
      break;

      case '#':                         /* Resolution information */
        *dst++= '@';                    /* */
        src++;                          /* Point to resolution    */
      break;

      default: return( NULL );
    }

    sscanf( src, "%dx%dx%d"
               , &thisW
               , &thisH
               , &thisBpp );
    while( isalnum( *src ))          /* Skip alphanumeric */
    { src++;
    }

    if ( *src== '.' )                  /* Orientation information */
    { src++; switch( *src )
      { case 'S': case 's': thisGeo= NSFB_ROTATE_SOUTH; break;
        case 'E': case 'e': thisGeo= NSFB_ROTATE_EAST;  break;
        case 'W': case 'w': thisGeo= NSFB_ROTATE_WEST;  break;
        case 'N': case 'n': thisGeo= NSFB_ROTATE_NORTH; break;
      }; src++;
    }

    if ( *src== ';' )                /* Placement information */
    { src++; sscanf( src, "%d,%d"
                        , &thisX
                        , &thisY );
    }

    while( *src++ != '@' )   /* Point to the driver */
    { if ( ! *src )
      { src= NULL;
        break;
    } }

    thisDriver= src;

     /* Driver at the end, ergo terminator valid */

    while( *src )            /* Add the driver */
    { *dst++= *src++;
    }
    *dst=0;

    if (( w   ) && ( thisW   != -1 )) { *w= thisW; }
    if (( h   ) && ( thisH   != -1 )) { *h= thisH; }
    if (( x   ) && ( thisX   != -1 )) { *x= thisX; }
    if (( y   ) && ( thisY   != -1 )) { *y= thisY; }

    if (( bpp ) && ( thisBpp != -1 )) { *bpp= thisBpp; }
    if (( geo ) && ( thisGeo != -1 )) { *geo= thisGeo; }

    return( thisDriver );
  }

  return( NULL );
}

ANSIC enum NsfbType nsfbTypeFromName( const char * name )
{ if ( name )
  { int try= 2;

    int theX= 0, theY= 0, theGeo= NSFB_ROTATE_NORTH;
    int theW= 0, theH= 0, theBpp= 32;

    char display[ 256 ];

    const char * drv= nsfbDemangleName( name
                                      , display
                                      , &theW, &theH, &theBpp
                                      , &theX, &theY, &theGeo );

    if ( drv )               /* Driver specified */
    { NsfbSurfaceRtns * ptr;

/* Try internally loaded
 */
      while( try-- )                   /* Only a try */
      { for( ptr = surfaceSeed
           ; ptr
           ; ptr= ptr->next )
        { if ( !strcmp( ptr->name, display ) )
          { ptr->theGeo= theGeo;
            ptr->theDepth= theBpp;
            return( ptr->type );
        } }

/* Load from shared lib if not, this registers surface also
 */
        NodeFun launch= loadModuleSymbolNsfb( "newNode"
                                            , "nsfb-%s-%s", drv, DLLVERSION );
        if ( launch )
        { if (( ptr= launch( display )))
          { _nsfb_register_surface( ptr );
            ptr->theGeo  = theGeo;
            ptr->theDepth= theBpp;
            return( ptr->type );
  } } } } }

  return( NSFB_SURFACE_NONE );
}

/** ========================================= [ JACS, 10/02/2023 ] == *\
 *                                                                    *
 *   JASC 2023                                                        *
 *                                                                    *
 *  FUNCTION loadIcoGifFile                                           *
 *           saveIcoGifFile                                           *
 *           loadIcoPngFile                                           *
 *           loadIcoJpgFile                                           *
 *                                                                    *
 *           loadImgGifFile                                           *
 *           loadImgPngFile                                           *
 *           loadImgJpgFile                                           *
 *                                                                    *
 *  @brief                                                            *
 *                                                                    *
\* ================================================================= **/
ANSIC IcoRec * loadIcoGifFile( const char * fname, int wtarget, int htarget  )
{ static IcoFun launcher;

  if ( !launcher )  /* Search for the magician in the dll*/
  {
    launcher= loadModuleSymbolNsfb( "loadIcoFile"
                                  , "nsfb-gif-%s", DLLVERSION );
  }

  return( launcher ? launcher( fname, wtarget, htarget ) : NULL );
}

/** ------------------------------------------------------------------------- */
    ANSIC IcoRec * loadIcoPngFile( const char * fname, int wtarget, int htarget  )
/** ------------------------------------------------------------------------- */
{ static IcoFun launcher;

  if ( !launcher )  /* Search for the magician in the dll*/
  {
    launcher= loadModuleSymbolNsfb( "loadIcoFile"
                                  , "nsfb-png-%s", DLLVERSION );
  }

  return( launcher ? launcher( fname, wtarget, htarget ) : NULL );
}

/*
 *
 */
ANSIC IcoRec * loadIcoJpgFile( const char * fname, int wtarget, int htarget  )
{ static IcoFun launcher;

  if ( !launcher )  /* Search for the magician in the dll*/
  {
    launcher= loadModuleSymbolNsfb( "loadIcoFile"
                                  , "nsfb-jpg-%s", DLLVERSION );
  }

  return( launcher ? launcher( fname, wtarget, htarget ) : NULL );
}


/*
 *
 */
ANSIC IcoRec * loadIcoSvgFile( const char * fname, int wtarget, int htarget  )
{ static IcoFun launcher;

  if ( !launcher )  /* Search for the magician in the dll*/
  {
    launcher= loadModuleSymbolNsfb( "loadIcoFile"
                                  , "nsfb-svg-%s", DLLVERSION );
  }

  return( launcher ? launcher( fname, wtarget, htarget ) : NULL );
}


/** ------------------------------------------------------------------------- */
    ANSIC DeviceImageRec * loadImgGifFile( const char * fname, int wtarget, int htarget  )
/** ------------------------------------------------------------------------- */
{ static ImgFun launcher;

  if ( !launcher )  /* Search for the magician in the dll*/
  {
    launcher= loadModuleSymbolNsfb( "loadImgFile"
                                  , "nsfb-gif-%s", DLLVERSION );
  }

  return( launcher ? launcher( fname, wtarget, htarget ) : NULL );
}

/** ------------------------------------------------------------------------- */
    ANSIC int dumpImgGifFile( Nsfb * nsfb, const char * fname  )
/** ------------------------------------------------------------------------- */
{ static DumpFun launcher;

  if ( !launcher )  /* Search for the magician in the dll*/
  {
    launcher= loadModuleSymbolNsfb( "dumpImgFile"
                                  , "nsfb-gif-%s", DLLVERSION );
  }

  return( launcher ? launcher( fname, nsfb ) : NULL );
}

/** ------------------------------------------------------------------------- */
    ANSIC DeviceImageRec * loadImgPngFile( const char * fname, int wtarget, int htarget  )
/** ------------------------------------------------------------------------- */
{ static ImgFun launcher;

  if ( !launcher )  /* Search for the magician in the dll*/
  {
    launcher= loadModuleSymbolNsfb( "loadImgFile"
                                  , "nsfb-png-%s", DLLVERSION );
  }

  return( launcher ? launcher( fname, wtarget, htarget ) : NULL );
}

/** ------------------------------------------------------------------------- */
    ANSIC DeviceImageRec * loadImgJpgFile( const char * fname, int wtarget, int htarget  )
/** ------------------------------------------------------------------------- */
{ static ImgFun launcher;

  if ( !launcher )  /* Search for the magician in the dll*/
  {
    launcher= loadModuleSymbolNsfb( "loadImgFile"
                                  , "nsfb-jpg-%s", DLLVERSION );
  }

  return( launcher ? launcher( fname, wtarget, htarget ) : NULL );
}

/** ------------------------------------------------------------------------- */
    ANSIC DeviceImageRec * loadImgSvgFile( const char * fname, int wtarget, int htarget  )
/** ------------------------------------------------------------------------- */
{ static ImgFun launcher;

  if ( !launcher )  /* Search for the magician in the dll*/
  {
    launcher= loadModuleSymbolNsfb( "loadVectors"
                                  , "nsfb-svg-%s", DLLVERSION );
  }

  return( launcher ? launcher( fname, wtarget, htarget ) : NULL );
}






