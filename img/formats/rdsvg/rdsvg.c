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
{ struct stat sb;
  VectorRec * diagram;
  
	 FILE * fd= fopen( name, "rb");
	 if ( !fd ) 
	 { perror( name );
		  return 1;
 	} 

 	 if (stat(name, &sb)) 
 	 { perror( name );
		   return 1;
	  }
 	 int size = sb.st_size;

 	 char * buffer = malloc(size);
	  if ( !buffer ) 
	  { fprintf(stderr, "Unable to allocate %lld bytes\n"
	                 ,	(long long) size );
   		return 1;
	  }

	  int n= fread(buffer, 1, size, fd);
	  if (n != size) 
	  { perror(name);
   		return 1;
	  }

	  fclose( fd );

/* create svgtiny object
*/
	  diagram = svgtinyCreate();
   if (! diagram )
   { fprintf(stderr, "svgtinyCreate failed\n");
   		return( NULL );
	  }

	/* parse
	 */

	  int code= svgtiny_parse( diagram, buffer, size, name
	                         , wtarget, htarget );

	  if (code != svgtiny_OK)
	  { fprintf(stderr, "svgtiny_parse failed: ");
	  
   		switch (code) 
   		{ case svgtiny_OUT_OF_MEMORY:	fprintf( stderr, "svgtiny_OUT_OF_MEMORY");			break;
     		case svgtiny_LIBXML_ERROR:		fprintf( stderr, "svgtiny_LIBXML_ERROR" );			break;
		     case svgtiny_NOT_SVG:			    fprintf( stderr, "svgtiny_NOT_SVG"      );			break;
  		   case svgtiny_SVG_ERROR:		  	fprintf( stderr, "svgtiny_SVG_ERROR: line %i: %s"
		                                                ,	diagram->error_line
		                                                ,	diagram->error_message );
      	break;

		     default:
			      fprintf(stderr, "unknown svgtiny_code %i", code);
		    	break;
		  }
		  fprintf(stderr, "\n");
	  }

	  free( buffer );

	 // VectorsFree( diagram );

	  return( diagram );
}



