/*
 * This file is part of Libsvgtiny
 * Licensed under the MIT License,
 *                http://opensource.org/licenses/mit-license.php
 * Copyright 2008 James Bursa <james@semichrome.net>
 */

#ifndef SVGTINY_INTERNAL_H
#define SVGTINY_INTERNAL_H

#include <stdbool.h>

#include "xml2dom.h"

#ifndef UNUSED
#define UNUSED(x) ((void) (x))
#endif

struct svgtiny_gradient_stop
{ float offset;
	 NSFBCOLOUR color;
};

#define svgtiny_MAX_STOPS 10

struct svgtinyParseState
{ VectorRec * diagram;

	 dom_document *document;

	 float viewport_width;
	 float viewport_height;

	/* current transformation matrix */
	 struct
	 { float a, b, c, d, e, f;
  } ctm;

	/*struct css_style style;*/

	/* paint attributes
	 */
	NSFBCOLOUR fill;
	NSFBCOLOUR stroke;
	// int stroke_width;
	float stroke_width;

	/* gradients
	 */

	unsigned int linear_gradient_stop_count;
	dom_string *gradient_x1, *gradient_y1, *gradient_x2, *gradient_y2;
	struct svgtiny_gradient_stop gradient_stop[svgtiny_MAX_STOPS];
	bool gradient_user_space_on_use;

	struct
	{ float a, b, c, d, e, f;
	} gradient_transform;

	/* Interned strings */
#define SVGTINY_STRING_ACTION2(n,nn) dom_string *interned_##n;
#include "svgtiny_strings.h"
#undef SVGTINY_STRING_ACTION2

};

struct svgtiny_list;

/* svgtiny.c
 */
float svgtiny_parse_length( dom_string *s
                          , int viewport_size
                          ,	const struct svgtinyParseState state );

void svgtinyParseColor( dom_string *s, NSFBCOLOUR *c
                      ,	struct svgtinyParseState *state );

void svgtinyParseTransform( char *s, float *ma, float *mb
                          ,	float *mc, float *md, float *me, float *mf);

struct svgtiny_shape * svgtinyAddShape(struct svgtinyParseState *state);

void svgtinyTransformPath( float *p, unsigned int n
                           ,	struct svgtinyParseState *state );

#if (defined(_GNU_SOURCE) && !defined(__APPLE__) || defined(__amigaos4__) || defined(__HAIKU__) || (defined(_POSIX_C_SOURCE) && ((_POSIX_C_SOURCE - 0) >= 200809L)))
#define HAVE_STRNDUP
#else
#undef HAVE_STRNDUP
char *svgtiny_strndup(const char *s, size_t n);
#define strndup svgtiny_strndup
#endif

/* svgtiny_gradient.c
 */
void svgtiny_find_gradient( const char *id, struct svgtinyParseState *state );
svgtiny_code svgtinyAddPathLinearGradient(float *p, unsigned int n,
		struct svgtinyParseState *state);

/* svgtiny_list.c
 */
struct svgtiny_list *svgtinyListCreate(size_t item_size);
unsigned int svgtiny_list_size(struct svgtiny_list *list);
svgtiny_code svgtiny_list_resize(struct svgtiny_list *list,
		unsigned int new_size);
void *svgtinyListGet(struct svgtiny_list *list,
		unsigned int i);
void *svgtinyListPush(struct svgtiny_list *list);
void svgtinyListFree(struct svgtiny_list *list);

/* colors.gperf */
const struct svgtiny_named_color *
		svgtiny_color_lookup(register const char *str,
				register unsigned int len);

#endif
