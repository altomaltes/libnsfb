/* libnsfb ploygon plotter test program */

#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 199506L
#endif


#include <stdio.h>
#include <math.h>

#include <stdarg.h>
#include "../nsfb.h"
#include "../nsfbPlot.h"

#define M_PI 3.1415926


#include <time.h>

static void sleepMilli(long ms)
{ const struct timespec ts = {ms / 1000, ms % 1000 * 1000000};
  nanosleep(&ts, NULL);
}

int main(int argc, char **argv)
{ const char *fename;
  enum NsfbType fetype;
  Nsfb *nsfb;

  int waitloop = 3;

  NsfbBbox box;
  byte *fbptr;
  int fbstride;

  int sides, rc;
  int radius;
  NsfbPoint *points;
  int loop;
  NsfbPlotpen pen;

  double rotate;

//  if (!( nsfb= nsfbOpenAscii( ":0.0#800x600x32.N@X11" )))
  if (!( nsfb= nsfbOpenAscii( ":0.0#800x600x32.N@w32" )))
  { fprintf(stderr, "Unable to initialise nsfb surface\n");
    nsfbFree( nsfb );
    return( 4);
  }

    /* get the geometry of the whole screen */
  box.x0 = box.y0 = 0;
  nsfbGetGeometry(nsfb, &box.x1, &box.y1, NULL);

  nsfbGetBuffer(nsfb, &fbptr, &fbstride);


  pen.strokeColour = 0xffFF0000;
  pen.strokeType = NFSB_PLOT_OPTYPE_SOLID;


  for ( rotate =0
      ; rotate < (2 * M_PI)
      ; rotate += (M_PI / 8) )
  { nsfbClaim(    nsfb, &box);     /* claim the whole screen for update */
    nsfbPlotclg( nsfb, 0x00 );

    radius = (box.y1 / 2);

   for (sides = 18; sides >=9; sides-=2)
   { points = alloca(sizeof(NsfbPoint) * sides);

    for (loop = 0; loop < sides;loop+=2)
    { points[ loop  ].x = (box.x1 / 2) +  (radius * cos((loop * 2 * M_PI / sides) + rotate));
      points[ loop  ].y = (box.y1 / 2) +  (radius * sin((loop * 2 * M_PI / sides) + rotate));
      points[ loop+1].x = (box.x1 / 2) + ((radius / 3) * cos(((loop+1) * 2 * M_PI / sides) + rotate));
      points[ loop+1].y = (box.y1 / 2) + ((radius / 3) * sin(((loop+1) * 2 * M_PI / sides) + rotate));
    }

     rc= nsfbPlotpolygon( nsfb
                    , (const int *)points, sides
                    , 0xff000000 | (0xffffff / (sides * 2)));

     rc= nsfbPlotpolylines( nsfb
                      , sides, points
                      , &pen);

     radius -= 40;
   }

   nsfbUpdate( nsfb, &box );

   sleepMilli(500);


  }

  nsfbFree( nsfb );

  return( 0 );
}

/*
 * Local variables:
 *  c-basic-offset: 4
 *  tab-width: 8
 * End:
 */
