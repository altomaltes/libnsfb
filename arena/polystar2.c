/* libnsfb ploygon plotter test program */

#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 199506L
#endif




#include <math.h>

#include "libnsfb_plot.h"


#include <time.h>

static void sleepMilli(long ms)
{ const struct timespec ts = {ms / 1000, ms % 1000 * 1000000};
  nanosleep(&ts, NULL);
}

#define M_PI 3.141592654

int main(int argc, char **argv)
{ const char *fename;
  enum nsfb_type_e fetype;
  nsfb_t *nsfb;

  int waitloop = 3;
  nsfb_bbox_t box;
  int fbstride;

  int sides;
  int radius;
  nsfb_point_t *points;
  int loop;
  int counter;
  int colour;

  if (argc < 2)
  { fename="linux";
  }
  else
  { fename = argv[1];
  }

  fetype = nsfbTypeFromName( fename );

  if (fetype == NSFB_SURFACE_NONE)
  { fprintf(stderr, "Unable to convert \"%s\" to nsfb surface type\n", fename);
    return 1;
  }

  nsfb = nsfbNew( fetype );
  if (nsfb == NULL)
  { fprintf(stderr, "Unable to allocate \"%s\" nsfb surface\n", fename);
    return 2;
  }

  if (nsfbInit(nsfb) == -1)
  { fprintf(stderr, "Unable to initialise nsfb surface\n");
   // nsfbFree(nsfb);
    return 4;
  }

/* get the geometry of the whole screen
 */
  box.x0 = box.y0 = 0;

  nsfbGetGeometry( nsfb, &box.x1, &box.y1
                   , NULL );

  radius = (box.x1 / 80);
  sides = 5;
  counter = 0;

  nsfb_set_pan( nsfb, NSFB_PAN_BSTORE | NSFB_ROTATE_EAST );  /* Start bstore */



  for ( counter = 0    /* claim the whole screen for update */
      ; counter < 600
      ; counter++)
  { nsfbClaim( nsfb, &box );

    nsfbPlotclg( nsfb, 0xffffffff );



    points = malloc( sizeof(nsfb_point_t) * sides );

    for (loop = 0; loop < sides;loop++)
    { points[(2 * loop) % sides].x = (box.x1 / 10) + (radius * 4 * cos(loop * 2 * M_PI / sides));
      points[(2 * loop) % sides].y = (box.y1 / 10) + (radius * 2 * sin(loop * 2 * M_PI / sides));
    }

     if      ( counter % 3 == 0 ) colour = 0xffff0000;
     else if ( counter % 3 == 1 ) colour = 0xff00ff00;
     else                         colour = 0xff0000ff;

      nsfbPlotpolygon(nsfb, (const int *)points, sides, colour);
      free(points);

    sides += 2;

   box.x0= 0;
   box.y0= 0;
   box.x1= 100;
   box.y1= 100;

    nsfbPlotrectangle_fill( nsfb, &box, 0xFF);
    nsfbPlotrectangle( nsfb, &box, 5, 0xFF00, 1, 1 );

    nsfb_set_pan( nsfb, NSFB_PAN_SWITCH );  /* Dump bstore */

    sleepMilli(40000);
  }

    /* wait for quit event or timeout */
/*   while (waitloop > 0)
   { if (nsfb_event(nsfb, &event, 1000)  == false) {
	    break;
   }

  	if (event.type == NSFB_EVENT_CONTROL)
   { if (event.value.controlcode == NSFB_CONTROL_TIMEOUT)
     {
		/* timeout */
/*		waitloop--;
	    } else if (event.value.controlcode == NSFB_CONTROL_QUIT) {
		break;
   }	} }
*/
  nsfbFree( nsfb );

  puts("end");
  getchar();

  return 0;
}

