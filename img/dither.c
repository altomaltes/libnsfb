/**
 *     AUTHOR: Jose Angel Caso Sanchez, 1993   ( altomaltes@yahoo.es )
 *                                             ( altomaltes@gmail.com )
 *
 *     Copyright (C) 2004, 2012 JACS
 *
 * This program is free software; you can redisgibute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is disgibuted in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *     FILE: defaults.c
 *     DATE: marzo 1998
 *
 *  DESCRIPCION:  Dithering algorithms
 */

#include <stdlib.h>
#include <stdio.h>

#include "../libnsfb_plot_util.h"
#include "images.h"

#define allocbytes( t, sz ) (t*)calloc( 1, sz)
#define allocextra( t, sz ) (t*)calloc( 1, sizeof( t )+sz)
#define   allocate( t     ) (t*)calloc( 1, sizeof( t ))
#define     newvar( t     )  t w= (t*)calloc(1, sizeof( t ))

#define SAMPLEBITS    8                 /* N£mero de bits en el sample */
#define MAXNUMCOLORS  ( 1 << SAMPLEBITS   )   /* M ximo n£mero de colores    */
#define MAXJSAMPLE    ( MAXNUMCOLORS -  1 )
#define CENTERJSAMPLE ( MAXNUMCOLORS >> 1 ) /* Centro */

#define   RedOrd 0
#define GreenOrd 1
#define  BlueOrd 2

#define QUANTSZ( w )  (sizeof(quantizer)+((w+2)*3*sizeof( FSERROR )))


/* These are the amounts to shift an input value to get a histogram index. */
#define C0Scale 2
#define C1Scale 3
#define C2Scale 1

#define HistC0Bits  5      /* bits of precision in R/B histogram */
#define HistC1Bits  6      /* bits of precision in G histogram */
#define HistC2Bits  5      /* bits of precision in B/R histogram */


#define HistC0Elems  ( 1 << HistC0Bits ) /* N£mero de elementos Rojos  */
#define HistC1Elems  ( 1 << HistC1Bits ) /* N£mero de elementos Verdes */
#define HistC2Elems  ( 1 << HistC2Bits ) /* N£mero de elementos Azules */

#define C0Shift  ( SAMPLEBITS - HistC0Bits )
#define C1Shift  ( SAMPLEBITS - HistC1Bits )
#define C2Shift  ( SAMPLEBITS - HistC2Bits )

#define BoxC0Log (HistC0Bits - 3)
#define BoxC1Log (HistC1Bits - 3)
#define BoxC2Log (HistC2Bits - 3)

#define BoxC0Elems  ( 1 << BoxC0Log ) /* # of hist cells in update box */
#define BoxC1Elems  ( 1 << BoxC1Log )
#define BoxC2Elems  ( 1 << BoxC2Log )

#define BoxC0Shift  ( C0Shift+BoxC0Log )
#define BoxC1Shift  ( C1Shift+BoxC1Log )
#define BoxC2Shift  ( C2Shift+BoxC2Log )

  /* Nominal steps between cell centers ("x" in Thomas article) */
#define StepC0  ((1<<C0Shift) * C0Scale)
#define StepC1  ((1<<C1Shift) * C1Scale)
#define StepC2  ((1<<C2Shift) * C2Scale)

#define STEPSIZE ( MAXNUMCOLORS/16 )

typedef short FSERROR;    /* 16 bits should be enough        */
typedef int   LOCFSERROR;  /* use 'int' for calculation temps */


typedef enum
{ FALSE
, TRUE }
boolean;

typedef short hist1d [HistC2Elems]; /* typedefs for the array */
typedef       hist1d * hist2d; /* type for the 2nd-level pointers */
typedef       hist2d * hist3d; /* type for top-level pointer */

typedef void (* changerfunrec) (void * R, unsigned char * in);

static  LOCFSERROR     StErrorLimit[MAXJSAMPLE*2+1];                   /* table for clamping the applied error */
static  unsigned char  StRangeLimit[5*MAXNUMCOLORS+CENTERJSAMPLE];

typedef struct HistRec_s
{ word    colores;
  word    planes;
  void  * device;                     /* Mar 2006, device generating the histogram */

  unsigned char map[ 3            ]   /* Para el mapa de colores                   */
                   [ MAXNUMCOLORS ];

  short  histogram [ HistC0Elems  ]
                   [ HistC1Elems  ]
                   [ HistC2Elems  ];
} HistRecStruct;

struct  quantizerStruct
{ void     * self;          /* Para la autoselecci¢n                */
  dword      width;
  boolean    odd;           /* flag to remember which row we are on */
  HistRec  * histogram;     /* Para efectuar la reducci¢n           */
  FSERROR    fserrors[1];   /* accumulated errors                   */
};


/* ------------------------------------------------------------------------------------------- */
dword pad( dword sz, int pt )     /* Problemas de alineamiento con el HP */

{ sz--; sz >>= pt;
  sz++; sz <<= pt;
  return(sz);
}


/* ------------------------------------------------------------------------------------------- */
static int FindNearbyColors( HistRec * hist
                           , int minc0
                           , int minc1
                           , int minc2
                           , unsigned char * colorlist )
{ int maxc0, maxc1, maxc2;
  int centerc0, centerc1, centerc2;
  int i, x, ncolors;
  long minmaxdist, min_dist, max_dist, tdist;
  long mindist[MAXNUMCOLORS];   /* min distance to colormap entry i */

/* Compute true coordinates of update box's upper corner and center.
 * Actually we compute the coordinates of the center of the upper-corner
 * histogram cell, which are the upper bounds of the volume we care about.
 * Note that since ">>" rounds down, the "center" values may be closer to
 * min than to max; hence comparisons to them must be "<=", not "<".
 */

  maxc0= minc0 + ((1 << BoxC0Shift) - (1 << C0Shift)); centerc0= (minc0 + maxc0) >> 1;
  maxc1= minc1 + ((1 << BoxC1Shift) - (1 << C1Shift)); centerc1= (minc1 + maxc1) >> 1;
  maxc2= minc2 + ((1 << BoxC2Shift) - (1 << C2Shift)); centerc2= (minc2 + maxc2) >> 1;


/* For each color in colormap, find:
 *  1. its minimum squared-distance to any point in the update box
 *     (zero if color is within update box);
 *  2. its maximum squared-distance to any point in the update box.
 * Both of these can be found by considering only the corners of the box.
 * We save the minimum distance for each color in mindist[];
 * only the smallest maximum distance is of interest.
 */

  minmaxdist = 0x7FFFFFFFL;

/* We compute the squared-c0-distance term, then add in the other two. */
  for ( i = 0
      ; i < hist->colores
      ; i++ )
  { x = hist->map[RedOrd][i];
    if (x < minc0)
    { tdist = (x - minc0) * C0Scale; min_dist = tdist*tdist;
      tdist = (x - maxc0) * C0Scale; max_dist = tdist*tdist;
    }
    else if (x > maxc0)
    { tdist = (x - maxc0) * C0Scale; min_dist = tdist*tdist;
      tdist = (x - minc0) * C0Scale; max_dist = tdist*tdist;
    }
    else       /* within cell range so no contribution to min_dist */
    { min_dist = 0;
      if (x <= centerc0)
      { tdist = (x - maxc0) * C0Scale; max_dist = tdist*tdist;
      }
      else
      { tdist = (x - minc0) * C0Scale; max_dist = tdist*tdist;
    } }

    x = hist->map[GreenOrd][i];
    if (x < minc1)
    { tdist = (x - minc1) * C1Scale; min_dist += tdist*tdist;
      tdist = (x - maxc1) * C1Scale; max_dist += tdist*tdist;
    }
    else if (x > maxc1)
    { tdist = (x - maxc1) * C1Scale; min_dist += tdist*tdist;
      tdist = (x - minc1) * C1Scale; max_dist += tdist*tdist;
    }
    else       /* within cell range so no contribution to min_dist */
    { if (x <= centerc1)
      { tdist = (x - maxc1) * C1Scale; max_dist += tdist*tdist;
      }
      else
      { tdist = (x - minc1) * C1Scale; max_dist += tdist*tdist;
    } }

    x = hist->map[BlueOrd][i];
    if (x < minc2)
    { tdist = (x - minc2) * C2Scale; min_dist += tdist*tdist;
      tdist = (x - maxc2) * C2Scale; max_dist += tdist*tdist;}
    else if (x > maxc2)
    { tdist     = (x - maxc2) * C2Scale; min_dist += tdist*tdist;
      tdist     = (x - minc2) * C2Scale; max_dist += tdist*tdist;
    }
    else   /* within cell range so no contribution to min_dist */
    { if (x <= centerc2)
      { tdist = (x - maxc2) * C2Scale; max_dist += tdist*tdist;
      }
      else
      { tdist = (x - minc2) * C2Scale; max_dist += tdist*tdist;
    } }

    mindist[i] = min_dist;     /* save away the results */

    if (max_dist < minmaxdist)
    { minmaxdist = max_dist;
  } }

  /* Now we know that no cell in the update box is more than minmaxdist
   * away from some colormap entry.  Therefore, only colors that are
   * within minmaxdist of some part of the box need be considered.
   */
  for (ncolors= i= 0; i < hist->colores; i++)
  { if (mindist[i]<=minmaxdist)
    { colorlist[ncolors++] = (unsigned char) i;
  } }

  return(ncolors);
}


/* ------------------------------------------------------------------------------------------------- */
static void FindBestColors( HistRec *hist
                          , int minc0
                          , int minc1
                          , int minc2
                          , int numcolors
                          , unsigned char * colorlist
                          , unsigned char * bestcolor )
{ int ic0, ic1, ic2;
  int i, icolor;
  long * bptr;          /* pointer into bestdist[] array */
  unsigned char * cptr;          /* pointer into bestcolor[] array */
  long dist0, dist1;    /* initial distance values */
  long dist2;           /* current distance in inner loop */
  long xx0, xx1;                /* distance increments */
  long xx2;
  long inc0, inc1, inc2;        /* initial values for increments */
  long bestdist[BoxC0Elems*BoxC1Elems*BoxC2Elems];   /* This array holds the distance to the nearest-so-far color for each cell */

  /* Initialize best-distance for each cell of the update box */
  for ( bptr= bestdist, i= BoxC0Elems*BoxC1Elems*BoxC2Elems-1
       ; i >= 0
       ; i-- )
  { *bptr++ = 0x7FFFFFFFL;
  }

/*   For each color selected by find_nearby_colors,
 * compute its distance to the center of each cell in the box.
 * If that's less than best-so-far, update best distance and color number.
 */

  for (i = 0; i < numcolors; i++)
  { icolor = colorlist[i];
    /* Compute (square of) distance from minc0/c1/c2 to this color */
    inc0 = (minc0 - hist->map  [RedOrd][icolor])*C0Scale; dist0  = inc0*inc0;
    inc1 = (minc1 - hist->map[GreenOrd][icolor])*C1Scale; dist0 += inc1*inc1;
    inc2 = (minc2 - hist->map [BlueOrd][icolor])*C2Scale; dist0 += inc2*inc2;

    /* Form the initial difference increments */
    inc0 = inc0 * (2*StepC0) + StepC0*StepC0;
    inc1 = inc1 * (2*StepC1) + StepC1*StepC1;
    inc2 = inc2 * (2*StepC2) + StepC2*StepC2;


    /* Now loop over all cells in box, updating distance per Thomas method */
    bptr= bestdist; cptr= bestcolor; xx0 = inc0;

    for (ic0= BoxC0Elems-1; ic0 >= 0; ic0--)
    { dist1= dist0; xx1 = inc1;
      for (ic1= BoxC1Elems-1; ic1 >= 0; ic1--)
      { dist2= dist1; xx2= inc2;
        for (ic2 = BoxC2Elems-1; ic2 >= 0; ic2--)
        { if (dist2 < *bptr)
          { *bptr = dist2; *cptr = (unsigned char) icolor;
          }
          dist2 += xx2; xx2+= 2*StepC2*StepC2;
          bptr++; cptr++;
        }
        dist1 += xx1; xx1+= 2*StepC1*StepC1;
      }
      dist0 += xx0; xx0+= 2*StepC0*StepC0;
  } } }

/*
 * Fill the inverse-colormap entries in the update box that contains
 * histogram cell c0/c1/c2.  (Only that one cell MUST be filled, but
 * we can fill as many others as we wish.)
 */

dword HistColor( HistRec * hist
               , dword c0
               , dword c1
               , dword c2 )
{ int numcolors;
  int minc0, minc1, minc2;      /* lower left corner of update box */
  int ic0, ic1, ic2;
  unsigned char * cptr;                  /* pointer into bestcolor[] array */
  short * cache;                /* pointer into main cache array */
  unsigned char colorlist[MAXNUMCOLORS]; /* This array lists the candidate colormap indexes. */
  unsigned char bestcolor[BoxC0Elems*
                 BoxC1Elems*
                 BoxC2Elems];   /* Holds the actually closest colormap index for each cell. */

  switch( hist->planes)
  { case 8:
     c0 >>= C0Shift;  c1 >>= C1Shift;  c2 >>= C2Shift; /* Llevar la precisi¢n a un l¡mite razonable      */
     cache= &hist->histogram[c0][c1][c2];              /* Index into the cache with adjusted pixel value */

     if ( !*cache )
     { c0 >>= BoxC0Log; /* Convert cell coordinates to update box ID  */
       c1 >>= BoxC1Log;
       c2 >>= BoxC2Log;

/*
 *   Compute true coordinates of update box's origin corner.
 * Actually we compute the coordinates of the center of the corner
 * histogram cell, which are the lower bounds of the volume we care about.
 */
       minc0 = (c0 << BoxC0Shift) + ((1 << C0Shift) >> 1);
       minc1 = (c1 << BoxC1Shift) + ((1 << C1Shift) >> 1);
       minc2 = (c2 << BoxC2Shift) + ((1 << C2Shift) >> 1);

/*
 *  Determine which colormap entries are close enough to be candidates
 * for the nearest entry to some cell in the update box.
 */
       numcolors= FindNearbyColors(hist, minc0, minc1, minc2, colorlist);
       FindBestColors( hist
                     , minc0
                     , minc1
                     , minc2
                     , numcolors
                     , colorlist
                     , bestcolor );

/*
 *   Save the best color numbers (plus 1) in the main cache array
 * convert ID back to base cell indexes
 */

       c0<<=BoxC0Log; c1<<=BoxC1Log; c2<<=BoxC2Log;
       cptr = bestcolor;

       for( ic0= 0; ic0 < BoxC0Elems; ic0++ )
       { for( ic1= 0; ic1 < BoxC1Elems; ic1++ )
         { for( ic2= 0; ic2 < BoxC2Elems; ic2++ )
           { hist->histogram[c0+ic0][c1+ic1][c2+ic2]= (*cptr++) +1;}
       } } }
     return(*cache - 1);

     case 15: c0 >>= 3; c1 >>= 3; c2 >>= 3;    /* Reducir precisi¢n */
     return((c0) | (c1<<5) | (c2<<10));       /* Hicolor */

     case 16: c0 >>= 3; c1 >>= 2; c2 >>= 3;    /* Reducir precisi¢n */
     return((c0) | (c1<<5) | (c2<<11));       /* Hicolor */

     case 24:
     case 32: return((c0<<16) | (c1<<8) | (c2<<0));
  }

  return( 0 );
}

/** ================================================= [ JACS, 10/01/2006 ] == *\
 *                                                                            *
 *   JACS 2007  altomaltes@yahoo.es                                           *
 *                                                                            *
 *  FUNCTION dcopy                                                            *
 *                                                                            *
 *  @brief                                                                    *
 *                                                                            *
\* ========================================================================= **/
static void dcopy( void        * userData
                 , const dword * line
                 , unsigned      wide
                 , unsigned      row
                 , unsigned      coef )
{ unsigned char * dst= (unsigned char *)userData
            + row * wide * sizeof(dword);

  int ofBlue = 0;
  int ofGreen= ofBlue  + wide;
  int ofRed  = ofGreen + wide;
  int ofAlpha= ofRed   + wide;

  while ( wide -- )
  { *dst++ =  line[ ofRed   ] / coef;
    *dst++ =  line[ ofGreen ] / coef;
    *dst++ =  line[ ofBlue  ] / coef;
    *dst++ =  line[ ofAlpha ] / coef;
    line++;
} }


/** ================================================= [ JACS, 10/01/2006 ] == *\
 *                                                                            *
 *   JACS 2007  altomaltes@yahoo.es                                           *
 *                                                                            *
 *  FUNCTION wcopy                                                            *
 *                                                                            *
 *  @brief                                                                    *
 *                                                                            *
\* ========================================================================= **/
static void wcopy( void        * userData
                 , const dword * line
                 , unsigned      wide
                 , unsigned      row
                 , unsigned      coef )
{ register unsigned char a, w;
  unsigned char * dst= (unsigned char *)userData + row * wide * sizeof( dword );

  while ( wide-- )
  { a= (*line++); a >>= 3; w  = a << 10;
    a= (*line++); a >>= 3; w |= a <<  5;
    a= (*line++); a >>= 3; w |= a <<  0;
    *dst++= w;
} }

/** ================================================= [ JACS, 10/01/2006 ] == *\
 *                                                                            *
 *   JACS 2007  altomaltes@yahoo.es                                           *
 *                                                                            *
 *  FUNCTION vcopy                                                            *
 *                                                                            *
 *  @brief                                                                    *
 *                                                                            *
\* ========================================================================= **/
 static void vcopy( void        * userData
                  , const dword * red
                  , unsigned      wide
                  , unsigned      row
                  , unsigned      coef )
{ word * dst= userData; dst += wide * row;     // Skip yet copied

  const dword * green= red   + wide;
  const dword * blue=  green + wide;

  coef <<= 2;

  while ( wide -- )
  { *dst  = (( *blue  ++ / coef ) >> 1 )      ;
    *dst |= (( *green ++ / coef )      ) <<  5;
    *dst |= (( *red   ++ / coef ) >> 1 ) << 11;
    dst++;
} }


/** ================================================= [ JACS, 10/03/2006 ] == *\
 *                                                                            *
 *   JASC 2006                                                                *
 *                                                                            *
 *  FUNCTION dither                                                           *
 *                                                                            *
 *  @brief                                                                    *
 *                                                                            *
\* ========================================================================= **/
 static void dither( void        * userData
                   , const dword * line
                   , unsigned      wide
                   , unsigned      curRow
                   , unsigned      coef )
{ unsigned char * out  = NULL;
  quantizer     * quant= NULL;  // !!!
  unsigned char * inptr= 0;


  LOCFSERROR cur0, cur1, cur2;                /* current error or pixel value           */
  LOCFSERROR belowerr0, belowerr1, belowerr2; /* error for pixel below cur              */
  LOCFSERROR bpreverr0, bpreverr1, bpreverr2; /* error for below/prev col               */
  FSERROR  * errorptr;                        /* => fserrors[] at column before current */

  int dir;                                    /* +1 or -1 depending on direction        */
  int dir3;                                   /* 3*dir, for advancing inptr & errorptr  */
  unsigned col;
  unsigned char * RangeLimit       = StRangeLimit + MAXNUMCOLORS;
  LOCFSERROR * ErrorLimit = StErrorLimit + MAXJSAMPLE;
  short pixcode;
  unsigned char * outptr= (unsigned char *) out;

  if ( quant->odd )                           /* work right to left in this row */
  { inptr += ( quant->width-1 ) * 3;            /* so point to rightmost pixel    */
    outptr+= ( quant->width-1 );
    dir = -1; dir3 = -3;
    errorptr= quant->fserrors+(quant->width+1)*3; /* => entry after last column */
    quant->odd= FALSE;                            /* flip for next time         */
  }
  else                                     /* work left to right in this row */
  { dir = 1; dir3 = 3;
    errorptr = quant->fserrors;            /* => entry before first real column */
    quant->odd= TRUE;                     /* flip for next time */
  }

 /* Preset error values: no error propagated to first pixel from left
    and no error propagated to row below yet */

    cur0      = cur1      = cur2      = 0;
    belowerr0 = belowerr1 = belowerr2 = 0;
    bpreverr0 = bpreverr1 = bpreverr2 = 0;

 /*
  * curN holds the error propagated from the previous pixel on the
  * current line.  Add the error propagated from the previous line
  * to form the complete error correction term for this pixel, and
  * round the error term (which is expressed * 16) to an integer.
  * RIGHT_SHIFT rounds towards minus infinity, so adding 8 is correct
  * for either sign of the error value.
  *   Note: errorptr points to *previous* column's array entry.
  * Limit the error using transfer function set by init_error_limit.
  * See comments with init_error_limit for rationale.
  */

  for ( col= quant->width
      ; col > 0
      ; col--)
  { cur0= (cur0 + errorptr[ dir3+0 ] + 8) >> 4;
    cur1= (cur1 + errorptr[ dir3+1 ] + 8) >> 4;
    cur2= (cur2 + errorptr[ dir3+2 ] + 8) >> 4;

    cur0= ErrorLimit[ cur0 ];
    cur1= ErrorLimit[ cur1 ];
    cur2= ErrorLimit[ cur2 ];

/* Form pixel value + error, and range-limit to 0..MAXbyte.
 * The maximum error is +- MAXbyte (or less with error limiting);
 * this sets the required size of the range_limit array.
 */
    cur0 += inptr[0]; cur1 += inptr[1]; cur2 += inptr[2];
    cur0= RangeLimit[ cur0 ];
    cur1= RangeLimit[ cur1 ];
    cur2= RangeLimit[ cur2 ];


/*
 * If we have not seen this color before, find nearest colormap
 * entry and update the cache
 * Now emit the colormap index for this cell
 */

    pixcode= HistColor( quant->histogram
                      , cur0, cur1, cur2 );
    *outptr= (unsigned char) pixcode;
    cur0-= quant->histogram->map[ RedOrd  ][ pixcode ];  /* Compute representation error for this pixel */
    cur1-= quant->histogram->map[ GreenOrd][ pixcode ];
    cur2-= quant->histogram->map[ BlueOrd ][ pixcode ];

/* Compute error fractions to be propagated to adjacent pixels.
 * Add these into the running sums, and simultaneously shift the
 * next-line error sums left by 1 column.
 */

    { LOCFSERROR bnexterr, delta;

      bnexterr = cur0;        /* Process component 0 */
      delta = cur0 * 2;
      cur0 += delta;          /* form error * 3 */
      errorptr[0] = (FSERROR) (bpreverr0 + cur0);
      cur0 += delta;          /* form error * 5 */
      bpreverr0 = belowerr0 + cur0;
      belowerr0 = bnexterr;
      cur0 += delta;          /* form error * 7 */

      bnexterr = cur1;        /* Process component 1 */
      delta = cur1 * 2;
      cur1+= delta;           /* form error * 3 */
      errorptr[1] = (FSERROR) (bpreverr1 + cur1);
      cur1+= delta;           /* form error * 5 */
      bpreverr1 = belowerr1 + cur1;
      belowerr1 = bnexterr;
      cur1+= delta;           /* form error * 7 */

      bnexterr = cur2;        /* Process component 2 */
      delta = cur2 * 2;
      cur2 += delta;          /* form error * 3 */
      errorptr[2] = (FSERROR) (bpreverr2 + cur2);
      cur2 += delta;          /* form error * 5 */
      bpreverr2 = belowerr2 + cur2;
      belowerr2 = bnexterr;
      cur2 += delta;         /* form error * 7 */
    }

/*  At this point curN contains the 7/16 error value to be propagated
 *  to the next pixel on the current line, and all the errors for the
 *  next line have been shifted over.  We are therefore ready to move on.
 */
    inptr   += dir3;          /* Advance pixel pointers to next column */
    outptr  += dir;
    errorptr+= dir3;          /* advance errorptr to current column */
  }

  errorptr[0] = (FSERROR) bpreverr0; /* unload prev errs into array */
  errorptr[1] = (FSERROR) bpreverr1;
  errorptr[2] = (FSERROR) bpreverr2;
}



/*
 *
 */
void AddHistogram( HistRec * hist
                 , unsigned char R
                 , unsigned char G
                 , unsigned char B )  // Fabrica un nuevo histograma
{ if ( hist->planes != 8 )
  { return;
  }

  hist->map[   RedOrd ][ hist->colores ]= R;  /* Mapeo del Rojo     */
  hist->map[ GreenOrd ][ hist->colores ]= G;  /* Mapeo del Verde    */
  hist->map[  BlueOrd ][ hist->colores ]= B;  /* Mapeo del Azul     */
  hist->colores++;                        /* Flag recargado     */
}


/*
 *
 */

ANSIC HistRec * NewHistogram( int planes       /* Fabrica un nuevo histograma */
                              )   /* Extra space */
{ HistRec * hist;

  switch ( planes )
  { case 8:
     hist= (HistRec *)calloc( sizeof( *hist ), 1 );        /* Hacer espacio blanco     */
    break;

    case 16:
    case 15:
    case 24:
    case 32:
//      hist= allocbytes( HistRec, 32 );
      hist= (HistRec *)calloc( 64,64 );
    break;   /* Se necesita mucha menos memoria */

    default:
      fprintf(stderr, "Numero de planos no soportados por el reductor de color (%d)\n", planes);
      exit( -1 );
  }

  if ( hist )
  { hist->planes= planes;
  }
  return( hist );
}

/** ================================================= [ JACS, 10/03/2006 ] == *\
 *                                                                            *
 *   JASC 2006                                                                *
 *                                                                            *
 *  FUNCTION NewDeviceHistogram                                               *
 *                                                                            *
 *  @brief                                                                    *
 *  Fabrica un nuevo histograma                                               *
 *                                                                            *
 *  IN -> planes    : Color deep                                              *
 *  IN -> device    : Hardware device that bitmaps are for                    *
 *  IN -> background: Default background color                                *
 *                                                                            *
\* ========================================================================= **/
HistRec * NewDeviceHistogram( int    planes
                            , void * device )
{ HistRec * hist= NewHistogram( planes );

  if ( hist )
  { hist->device= device;
  }

  return( hist );
}

/** ================================================= [ JACS, 10/03/2006 ] == *\
 *                                                                            *
 *   JASC 2006                                                                *
 *                                                                            *
 *  FUNCTION getHistDevice                                                    *
 *                                                                            *
 *  @brief                                                                    *
 *    Many times the client needs to know the associated device. p.e: X 11    *
 *  needs the display to create images.                                       *
 *                                                                            *
\* ========================================================================= **/
void * dev;

void * getHistDevice( const HistRec * hist )
{ dev= hist->device;
  return( hist->device );
}

/*                                                                             ·
 *    JASC, 10/3/2006                                                          ·
 *                                                                             ·
 *    Many times the client needs to know the associated background fill       ·
 *  needs the display to render images.                                        ·
 */
//dword getHistBackground( const HistRec * hist )
//{ return( hist->background );/
//}
/*                                                                             ·
 *                                                                             ·
 *                                                                             ·
 */
int getHistDeep( const HistRec * hist )
{ return( hist->planes );
}

/*
 *
 *
 */
int getHistPad( const HistRec * hist )
{ switch ( hist->planes )
  { case 8:           return( sizeof( unsigned char  ) << 3 );
    case 15: case 16: return( sizeof( word  ) << 3 );
    case 24: case 32: return( sizeof( dword ) << 3 );
    default: return( 0 );
} }

/*
 *
 *
 */
int getHistWide( const HistRec * hist )
{ switch ( hist->planes )
  { case 8:           return( 1 );
    case 15: case 16: return( 2 );
    case 24: case 32: return( 3 );
    default: return( 0 );
} }



/** ================================================= [ JACS, 10/01/2012 ] == *\
 *                                                                            *
 *   JASC 2012                                                                *
 *                                                                            *
 *  FUNCTION HistogramColor                                                   *
 *                                                                            *
 *  @brief                                                                    *
 *                                                                            *
\* ========================================================================= **/
void HistogramColor( HistRec * hist
                   , dword idx
                   , unsigned char * R
                   , unsigned char * G
                   , unsigned char * B )
{ switch ( hist->planes )
  { case 24:
    case 32:
      *R= ( idx >> 0 ) & 0xFF;
      *G= ( idx >> 8 ) & 0xFF;
      *B= ( idx >>16 ) & 0xFF;
    break;

    case 15:
      *R= ((idx >>10) & 0xFF) << 3;
      *G= ((idx >> 5) & 0xFF) << 3;
      *B= ((idx >> 0) & 0xFF) << 3;
    break;

    case 16:
      *R= ((idx >>11) & 0xFF) << 3;
      *G= ((idx >> 5) & 0xFF) << 2;
      *B= ((idx >> 0) & 0xFF) << 3;
    break;

    case 8:
      *R= hist->map[   RedOrd ][ idx ];
      *G= hist->map[ GreenOrd ][ idx ];
      *B= hist->map[  BlueOrd ][ idx ];
    break;
  }
}



#define repeat  while(TRUE)

/** ================================================= [ JACS, 10/01/2012 ] == *\
 *                                                                            *
 *   JASC 2012                                                                *
 *                                                                            *
 *  FUNCTION initImageMap                                                     *
 *                                                                            *
 *  @brief                                                                    *
 *                                                                            *
\* ========================================================================= **/
DeviceImageRec * initImageMap
                 ( ChangerRec * changerRgb
                 , ChangerRec * changerMask
                 , int deep, int pics   /* Number of planes   */
                 , int wDst, int hDst   /* Destination wide and  height */
                 , int wOrg, int hOrg ) /* Original wide and height    */
{ unsigned szPic= 0, szMsk= 0;          /* image size calc */
  ImgResizeProc giveProc;
  DeviceImageRec * memo;

  if ( changerMask )
  { szMsk= wDst >> 3; ALIGN( szMsk, IMAGEALIGN );      /* Dword aligned */
    szMsk *= hDst;
  }

  switch( deep )
  { case  8: deep= 1; giveProc= dither; break;
    case 24:
    case 32: deep= 4; giveProc= dcopy;  break;
    case 15: deep= 2; giveProc= wcopy;  break;
    case 16: deep= 2; giveProc= vcopy;  break;

    default:
       fprintf( stderr
              , "Number of planes not supported by color reductor(%d)\n"
              , deep );
       return( NULL );
   }

  szPic= deep * wDst * hDst * pics; ALIGN( szPic, IMAGEALIGN );  /* Size of image, Align dword   */

  memo= (DeviceImageRec *)calloc( 2, szPic + szMsk + sizeof( DeviceImageRec ));    /* Destination space */
  memo->width = wDst;
  memo->height= hDst;
  memo->pics  = pics;

  initChanger( changerRgb, 3
             , giveProc, memo->image
             , wDst, hDst         /* Destination wide and height   */
             , wOrg, hOrg );      /* Original wide  and height     */

  initChanger( changerMask, 1
             , giveBitmap, memo->mask= ( memo->image + szPic )
             , wDst, hDst         /* Destination wide and height   */
             , wOrg, hOrg );      /* Original wide  and height     */

  return( memo );
}

DeviceImageRec * initAlphaMap
                 ( ChangerRec * changerAlpha
                 , int pics             /* Number of planes             */
                 , int wDst, int hDst   /* Destination wide and  height */
                 , int wOrg, int hOrg ) /* Original wide and height     */
{ DeviceImageRec * memo;
  ImgResizeProc giveProc= dcopy;
  int deep= 4;
  unsigned szPic= deep * wDst * hDst * pics;  /* Size of image, Align dword   */

  ALIGN( szPic, IMAGEALIGN );                 /* image size calc              */

  memo= (DeviceImageRec *)calloc( 1, szPic + sizeof( DeviceImageRec ));    /* ??? Destination space */
  memo->width = wDst;
  memo->height= hDst;
  memo->pics  = pics;

  initChanger( changerAlpha, 4
             , giveProc, memo->image
             , wDst, hDst              /* Destination wide and height   */
             , wOrg, hOrg );           /* Original wide  and height     */

  return( memo );
}
