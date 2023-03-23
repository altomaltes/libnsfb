/*
 * This file is part of Libsvgtiny
 * Licensed under the MIT License,
 *                http://opensource.org/licenses/mit-license.php
 * Copyright 2009-2010 James Bursa <james@semichrome.net>
 */

/*
 * This example loads an SVG using libsvgtiny and then displays it in an X11
 * window using nfsb.
 *
 * Functions of interest for libsvgtiny use are:
 *  main() - loads an SVG using svgtinyCreate() and svgtiny_parse()
 *  event_diagram_expose() - renders the SVG by stepping through the shapes
 *
 * Compile using:
 *  gcc -g -W -Wall -o svgtiny_display_x11 svgtiny_display_x11.c \
 *          `pkg-config --cflags --libs libsvgtiny nfsb` -lX11
 */


#include <libgen.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "svgtiny.h"

#include "../../../nsfb.h"










/**
 * Handle an X11 Expose event of the diagram window.
 */
#include "../../../nsfb.h"
#include "../../../nsfbPlot.h"


/** ====================================================[ JACS 1997-11-11 ]== *\
 *                                                                            *
 *   JACS 2011                                                                *
 *                                                                            *
 *  FUNCTION: loadImgFile                                                     *
 *                                                                            *
 *  @brief Loads a png image from disk.                                       *
 *                                                                            *
\* ========================================================================= **/
ANSIC VectorRec * loadVectors( const char * name
                             , int wtarget, int htarget )
{ FILE * fd= fopen( name, "rb");

  if ( fd )
  { struct stat sb;

    if ( !stat( name, &sb ))
    { int size = sb.st_size;

      char * buffer= alloca( size );

      if ( !buffer )
      { close( fd );
        return( NULL );
      }

       int n= fread( buffer
                    , 1
                    , size, fd );
       close( fd );

       if ( n == size)
       { VectorRec * diagram= svgtinyCreate();

         if ( diagram ) /* create svgtiny object */
         { int code= svgtiny_parse( diagram, buffer, size, name /* parse */
                                  , wtarget, htarget );
           switch ( code )
           { case svgtiny_OK: return( diagram );

	            case svgtiny_NOT_SVG:	      fprintf( stderr, "svgtiny_NOT_SVG\n"      ); break;
  		         case svgtiny_OUT_OF_MEMORY: fprintf( stderr, "svgtiny_OUT_OF_MEMORY\n"); break;
      	      case svgtiny_LIBXML_ERROR:  fprintf( stderr, "svgtiny_LIBXML_ERROR\n" ); break;
  	 	        case svgtiny_SVG_ERROR:     fprintf( stderr, "svgtiny_SVG_ERROR: line %i: %s\n"
                                                , diagram->error_line
                                                , diagram->error_message );
             break;

             default: fprintf(stderr, "unknown svgtiny_code %i", code);
       } }
       VectorsFree( diagram );
  } } }

  return( NULL );
}



