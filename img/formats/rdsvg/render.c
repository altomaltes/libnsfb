/*
 * This file is part of Libsvgtiny
 * Licensed under the MIT License,
 *                http://opensource.org/licenses/mit-license.php
 * Copyright 2009-2010 James Bursa <james@semichrome.net>
 */

/*
 * This example loads an SVG using libsvgtiny and then displays it in an X11
 * window using cairo.
 *
 * Functions of interest for libsvgtiny use are:
 *  main() - loads an SVG using svgtiny_create() and svgtiny_parse()
 *  event_diagram_expose() - renders the SVG by stepping through the shapes
 *
 * Compile using:
 *  gcc -g -W -Wall -o svgtiny_display_x11 svgtiny_display_x11.c \
 *          `pkg-config --cflags --libs libsvgtiny cairo` -lX11
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


struct svgtiny_diagram *diagram;
char *svg_path;
float scale = 1.0;



	/* load file into memory buffer */
	fd = fopen(svg_path, "rb");
	if (!fd) {
		perror(svg_path);
		return 1;
	}

	if (stat(svg_path, &sb)) {
		perror(svg_path);
		return 1;
	}
	size = sb.st_size;

	buffer = malloc(size);
	if (!buffer) {
		fprintf(stderr, "Unable to allocate %lld bytes\n",
				(long long) size);
		return 1;
	}

	n = fread(buffer, 1, size, fd);
	if (n != size) {
		perror(svg_path);
		return 1;
	}

	fclose(fd);

/* create svgtiny object
*/
	 diagram = svgtiny_create();
  if (!diagram)
  { fprintf(stderr, "svgtiny_create failed\n");
  		return 1;
	 }

	/* parse
	 */

	code= svgtiny_parse(diagram, buffer, size, svg_path, 240, 320);

	if (code != svgtiny_OK)
	{
		fprintf(stderr, "svgtiny_parse failed: ");
		switch (code) {
		case svgtiny_OUT_OF_MEMORY:			fprintf(stderr, "svgtiny_OUT_OF_MEMORY");			break;
		case svgtiny_LIBXML_ERROR:			fprintf(stderr, "svgtiny_LIBXML_ERROR");			break;
		case svgtiny_NOT_SVG:			fprintf(stderr, "svgtiny_NOT_SVG");			break;
		case svgtiny_SVG_ERROR:			fprintf(stderr, "svgtiny_SVG_ERROR: line %i: %s"
		                                     ,					diagram->error_line,
                              					diagram->error_message);
 	break;

		default:
			fprintf(stderr, "unknown svgtiny_code %i", code);
			break;
		}
		fprintf(stderr, "\n");
	}

	free( buffer );

	svgtiny_free(diagram);

	return 0;
}






/**
 * Render an svgtiny path using cairo.
 */
void render_path(cairo_t *cr, float scale, struct svgtiny_shape *path)
{	unsigned int j;

	 cairo_new_path(cr);

	 for ( j = 0
	     ; j != path->path_length; )
	 { switch ((int) path->path[j])
	   { case svgtiny_PATH_MOVE:
     			cairo_move_to( scale * path->path[j + 1]
     			             ,	scale * path->path[j + 2] );
      			j += 3;
    		break;

		    case svgtiny_PATH_CLOSE:
			     cairo_close_path();
			     j += 1;
			   break;

		    case svgtiny_PATH_LINE:
     			cairo_line_to( scale * path->path[j + 1]
     			             ,	scale * path->path[j + 2]);
     			j += 3;
   			break;

		    case svgtiny_PATH_BEZIER:
			     cairo_curve_to( scale * path->path[j + 1]
			                   ,	scale * path->path[j + 2]
			                   ,	scale * path->path[j + 3]
			                   ,	scale * path->path[j + 4]
			                   ,	scale * path->path[j + 5]
			                   ,	scale * path->path[j + 6] );
			     j += 7;
			   break;

		    default:
			     printf("error ");
			j += 1;
		}
	}

	if (path->fill != svgtiny_TRANSPARENT)
	{ cairo_set_source_rgb( cr
	                      , svgtiny_RED(path->fill) / 255.0
	                      ,	svgtiny_GREEN(path->fill) / 255.0
	                      , svgtiny_BLUE(path->fill) / 255.0 );
 		cairo_fill_preserve(cr);
	}

	if (path->stroke != svgtiny_TRANSPARENT)
	{ cairo_set_source_rgb( cr
	                      , svgtiny_RED(   path->stroke ) / 255.0
	                      ,	svgtiny_GREEN( path->stroke ) / 255.0
	                      ,	svgtiny_BLUE(  path->stroke ) / 255.0 );
		 cairo_set_line_width( cr, scale * path->stroke_width );
		 cairo_stroke_preserve( cr );
	}

}

/**
 * Handle an X11 Expose event of the diagram window.
 */
void event_diagram_expose( const XExposeEvent *expose_event )
{ cairo_set_source_rgb(cr, 1, 1, 1);
	 cairo_paint(cr);

	 for ( i = 0
	     ; i != diagram->shape_count
	     ; i++ )
	 { if ( diagram->shape[i].path )
	   { render_path(cr, scale, &diagram->shape[i]);
  		}
  		else if (diagram->shape[i].text)
  		{ cairo_set_source_rgb(cr,
				  svgtiny_RED(   diagram->shape[i].stroke) / 255.0,
				  svgtiny_GREEN( diagram->shape[i].stroke) / 255.0,
				  svgtiny_BLUE(  diagram->shape[i].stroke) / 255.0);

   			cairo_move_to(cr, scale * diagram->shape[i].text_x
   			                ,	scale * diagram->shape[i].text_y );
   			cairo_show_text(cr, diagram->shape[i].text);
		  }
	}
}


