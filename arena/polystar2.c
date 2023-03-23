/* libnsfb ploygon plotter test program */

#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 199506L
#endif




#include <math.h>

#include <stdarg.h>
#include "../nsfb.h"
#include "../nsfbPlot.h"


#include <time.h>

static void sleepMilli(long ms)
{ const struct timespec ts = {ms / 1000, ms % 1000 * 1000000};
  nanosleep(&ts, NULL);
}

#define M_PI 3.141592654

int main(int argc, char **argv)
{ const char *fename;
  enum NsfbType fetype;
  Nsfb *nsfb;

  int waitloop = 3;
  NsfbBbox box;
  int fbstride;

  int sides;
  int radius;
  NsfbPoint *points;
  int loop;
  int counter;
  int colour;

  if (argc < 2)
  { fename= ":0.0#800x600x32.N@x11";
  }
  else
  { fename= argv[1];
  }

  if (!( nsfb= nsfbOpenAscii( fename )))
  { fprintf(stderr, "Unable to convert \"%s\" to nsfb surface type\n", fename);
    return 1;
  }

/* get the geometry of the whole screen
 */
  box.x0 = box.y0 = 0;

  nsfbGetGeometry( nsfb, &box.x1, &box.y1
                   , NULL );

  radius = (box.x1 / 80);
  sides = 5;
  counter = 0;

 // nsfbSetPan( nsfb, NSFB_PAN_BSTORE | NSFB_ROTATE_EAST );  /* Start bstore */



  for ( counter = 0    /* claim the whole screen for update */
      ; counter < 600
      ; counter++)
  { nsfbClaim( nsfb, &box );

    nsfbPlotclg( nsfb, 0xffffffff );

    points = alloca( sizeof(NsfbPoint) * sides );

    for (loop = 0; loop < sides;loop++)
    { points[(2 * loop) % sides].x = (box.x1 / 10) + (radius * 4 * cos(loop * 2 * M_PI / sides));
      points[(2 * loop) % sides].y = (box.y1 / 10) + (radius * 2 * sin(loop * 2 * M_PI / sides));
    }

     if      ( counter % 3 == 0 ) colour = 0xffff0000;
     else if ( counter % 3 == 1 ) colour = 0xff00ff00;
     else                         colour = 0xff0000ff;

    nsfbPlotpolygon(nsfb, (const int *)points, sides, colour);

    sides += 2;

    box.x0= 0;
    box.y0= 0;
    box.x1= 100;
    box.y1= 100;

    nsfbPlotrectangleFill( nsfb, &box, 0xFF);
    nsfbPlotrectangle( nsfb, &box, 5, 0xFF00, 1, 1 );

    nsfbSetPan( nsfb, NSFB_PAN_SWITCH );  /* Dump bstore */

    sleepMilli(40000);
  }

  nsfbFree( nsfb );

  puts("end");
  getchar();

  return 0;
}

