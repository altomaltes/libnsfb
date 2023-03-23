/*
 * This file is part of Libsvgtiny
 * Licensed under the MIT License,
 *                http://opensource.org/licenses/mit-license.php
 * Copyright 2008 James Bursa <james@semichrome.net>
 */

#ifndef SVGTINY_H
#define SVGTINY_H

// To get the definition of size_t
#include <stdlib.h>

#include "../../../libnsfb.h"
#include "../../../nsfbPlot.h"

typedef unsigned int NSFBCOLOUR;

#ifdef __riscos__
  #define svgtiny_RGB(r, g, b) (0xff << 24 | (b) << 16 | (g) << 8 | (r))
  #define svgtiny_RED(c) ((c) & 0xff)
  #define svgtiny_GREEN(c) (((c) >> 8) & 0xff)
  #define svgtiny_BLUE(c) (((c) >> 16) & 0xff)
#else
  #define svgtiny_RGB(r, g, b ) (0x00 << 24 | (r) << 16 | (g) << 8 | (b))
  #define svgtiny_RED(      c ) (((c) >> 16) & 0xff)
  #define svgtiny_GREEN(    c ) (((c) >> 8) & 0xff)
  #define svgtiny_BLUE(     c ) ((c) & 0xff)
#endif

#define svgtiny_ALPHA(c) (((c) >> 24) & 0xff)

// Use these colors to indicate special.
// They are legal, but unlikely, black that's almost transparent.
// TODO: in a future commit, we'll malloc the gradient description into the _internal_extensions
// field of the svgtiny_shape, and we'll lopk at that to decide if an object has a gradient.


struct svgtiny_shape
{ float * path;
	 unsigned int path_length;
	 char  * text;
	 float text_x, text_y;
	 NSFBCOLOUR fill;
	 NSFBCOLOUR stroke;
	 float stroke_width;

//  void *_internal_extensions;  // TODO: if non-NULL, points to an allocated on the heap extension block. (gradients, fonts)
};

typedef enum
{ svgtiny_OK
, svgtiny_OUT_OF_MEMORY
,	svgtiny_LIBDOM_ERROR
,	svgtiny_NOT_SVG
,	svgtiny_SVG_ERROR
,	svgtiny_LIBXML_ERROR
} svgtiny_code;

/*
enum
{ NFSB_PLOT_PATHOP_MOVE
,	NFSB_PLOT_PATHOP_CLOSE
,	NFSB_PLOT_PATHOP_LINE
,	NFSB_PLOT_PATHOP_QUAD
};
  */
struct svgtiny_named_color
{
 	const char *name;
	 NSFBCOLOUR color;
};


VectorRec * svgtinyCreate( void );

svgtiny_code svgtiny_parse( VectorRec *diagram
                          , const char *buffer, size_t size, const char *url
                          ,	int width, int height);

// Gets the width and size from the buffer.
svgtiny_code svgtiny_parse0( VectorRec *diagram
                           , const char *buffer, size_t size );


void VectorsFree( VectorRec *svg );

#endif
