/* libnsfb ploygon plotter test program */

#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 199506L
#endif


#include <stdio.h>
#include <math.h>

#include "../libnsfb_plot.h"

#define M_PI 3.1415926


#include <time.h>

static void sleepMilli(long ms)
{ const struct timespec ts = {ms / 1000, ms % 1000 * 1000000};
  nanosleep(&ts, NULL);
}

int main(int argc, char **argv)
{ const char *fename;
  enum nsfb_type_e fetype;
  nsfb_t *nsfb;

  int waitloop = 3;

  nsfb_bbox_t box;
  byte *fbptr;
  int fbstride;

  int sides;
  int radius;
  nsfb_point_t *points;
  int loop;
  nsfbPlotpen_t pen;

  double rotate;

  if (nsfbOpen( "fb#1280x1024x32.N@vga" ) == -1)
  { fprintf(stderr, "Unable to initialise nsfb surface\n");
    nsfbFree(nsfb);
    return 4;
  }

    /* get the geometry of the whole screen */
  box.x0 = box.y0 = 0;
  nsfbGetGeometry(nsfb, &box.x1, &box.y1, NULL);

  nsfb_get_buffer(nsfb, &fbptr, &fbstride);


  pen.stroke_colour = 0xffFF0000;
  pen.stroke_type = NFSB_PLOT_OPTYPE_SOLID;

  nsfb_set_pan( nsfb, NSFB_PAN_BSTORE ); /* Defaul draw method,backing store */



  for (rotate =0; rotate < (2 * M_PI); rotate += (M_PI / 8))
  { nsfbClaim(    nsfb, &box);     /* claim the whole screen for update */
    //nsfbPlotclg( nsfb, 0xffffffff );
    nsfbPlotclg( nsfb, 0x00 );

    radius = (box.y1 / 2);

   for (sides = 18; sides >=9; sides-=2)
   { points = alloca(sizeof(nsfb_point_t) * sides);

    for (loop = 0; loop < sides;loop+=2)
    { points[loop  ].x = (box.x1 / 2) +  (radius * cos((loop * 2 * M_PI / sides) + rotate));
      points[loop  ].y = (box.y1 / 2) +  (radius * sin((loop * 2 * M_PI / sides) + rotate));
      points[loop+1].x = (box.x1 / 2) + ((radius / 3) * cos(((loop+1) * 2 * M_PI / sides) + rotate));
      points[loop+1].y = (box.y1 / 2) + ((radius / 3) * sin(((loop+1) * 2 * M_PI / sides) + rotate));
    }

     nsfbPlotpolygon( nsfb
                      , (const int *)points, sides
                      , 0xff000000 | (0xffffff / (sides * 2)));

     nsfbPlotpolylines(nsfb, sides, points, &pen);

     radius -= 40;
   }

     nsfb_update(nsfb, &box);
     nsfb_set_pan( nsfb, NSFB_PAN_SWITCH );     // Defaul draw method,backing store

      sleepMilli(100);
    }

    /* wait for quit event or timeout */
/*    nsfb_event_t event;
    while (waitloop > 0) {
	if (nsfb_event(nsfb, &event, 1000)  == false) {
	    break;
	}
	if (event.type == NSFB_EVENT_CONTROL) {
	    if (event.value.controlcode == NSFB_CONTROL_TIMEOUT) {
		/* timeout */
	/*	waitloop--;
	    } else if (event.value.controlcode == NSFB_CONTROL_QUIT) {
		break;
	    }
	}
    }
*/
    nsfbFree(nsfb);

    return 0;
}

/*
 * Local variables:
 *  c-basic-offset: 4
 *  tab-width: 8
 * End:
 */
