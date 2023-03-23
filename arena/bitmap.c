/* libnsfb plotter test program */

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#include "../nsfb.h"
#include "../nsfbPlot.h"

#define UNUSED(x) ((x) = (x))

extern const struct
{ unsigned int  width;
  unsigned int  height;
  unsigned int  bytes_per_pixel; /* 3:RGB, 4:RGBA */
  unsigned char pixel_data[132 * 135 * 4 + 1];
} nsglobe;

static bool dump(Nsfb *nsfb, const char *filename)
{ int fd;

  if ( !filename )
  	return false;

  fd = open(filename, O_RDWR | O_CREAT | O_TRUNC, S_IRWXU);
  if (fd < 0)
  	return false;

  //nsfbDump(nsfb, fd);

  close(fd);

  return true;
}

int doWindow( const char *fename )
{ enum NsfbType fetype;
  Nsfb * nsfb;
  int waitloop = 3;

  NsfbBbox box;
  NsfbBbox box2;
  NsfbBbox box3;
  uint8_t *fbptr;
  int fbstride;


  if ( 0 )
  { fetype= nsfbTypeFromName( fename );

    if ( fetype == NSFB_SURFACE_NONE )
    { fprintf(stderr, "Unable to convert \"%s\" to nsfb surface type\n", fename);
      return 1;
    }


    nsfb= nsfbNew( fetype );
    
    if ( !nsfb )
    { printf("Unable to allocate \"%s\" nsfb surface\n", fename);
	     return EXIT_FAILURE;
    }

    nsfbSetGeometry( nsfb
                   , 800
                   , 600
                   , 32 );
    if ( nsfbInit( nsfb ))
    { printf("Unable to initialise nsfb surface\n");
	     nsfbFree( nsfb );
  	   return EXIT_FAILURE;
  } }
  else
  { if (!( nsfb= nsfbOpenAscii( fename )))
  	 { return( EXIT_FAILURE );
  } }
  
  nsfbClaim(   nsfb, &box );
  nsfbPlotclg( nsfb, 0xffffffff );


  VectorRec * vec= loadImgSvgFile( "tiger.svg", 800, 600 );
  nsfbRenderDeviceVects( nsfb, vec, 0, 0, 400, 300 );
  
  nsfbSnap( nsfb );  // Swap buffers
  return;
  
//  DeviceImageRec * pix= LoadImgFile( "/tmp/radar.gif", 0,0 );
//  getDeviceBitmap( nsfb, pix );
  
  //DeviceImageRec * pix= nsfbNewPixmap( nsfb
    //                                 , nsglobe.width
      //                               , nsglobe.height
        //                             , nsglobe.pixel_data
          //                           , NSFB_FMT_ABGR8888 );


//  bmp= nsfbNew( NSFB_SURFACE_RAM ); slow for non memory based surfaces
//  nsfbInit( bmp );
//  nsfbSetGeometry( bmp
//                 , nsglobe.width
//                 , nsglobe.height
//                 , NSFB_FMT_ABGR8888 );
//  nsfbGetBuffer( bmp, &fbptr, &fbstride);
//
//  memcpy( fbptr
//        , nsglobe.pixel_data
//        , nsglobe.width * nsglobe.height * 4);


/* get the geometry of the whole screen
 */
  box.x0 = box.y0 = 0;
  nsfbGetGeometry(nsfb, &box.x1, &box.y1, NULL);

/* claim the whole screen for update
 */

  box3.x0= 0;   box3.y0= 0;
  box3.x1= 132; box3.y1= 135;

//  nsfbPutIconImage( nsfb, pix, 0, 0, 0, 0 );

//  nsfbPlotPixmap( nsfb, pix, box3.x0, box3.y0, 0xFFFFFFFF );

  /*
  box3.x0 = 132;    box3.y0 = 135;
  box3.x1 = box3.x0 + 264;    box3.y1 = box3.y0 + 135;

//  nsfbPlotcopy(bmp, &box3, nsfb, &box3);
  nsfbPutPixmap( pix, box3.x0, box3.y0 );

  box3.x0 = 396;    box3.y0 = 270;
  box3.x1 = box3.x0 + 264;    box3.y1 = box3.y0 + 270;

//  nsfbPlotcopy(bmp, &box3, nsfb, &box3);
  nsfbPutPixmap( pix, box3.x0, box3.y0 );

  box2.x0 = 64;  box2.y0 = 64;
  box2.x1 = 128; box2.y1 = 128;

  box3.x0 = 270;           box3.y0 = 270;
  box3.x1 = box3.x0 + 64;  box3.y1 = box3.y0 + 64;

//  nsfbPlotcopy(bmp, &box3, nsfb, &box3);
  nsfbPutPixmap( pix, box3.x0, box3.y0 );

             */

  //nsfbSnap( nsfb );  // Swap buffers

//  dump(nsfb, dumpfile);
//  nsfbFree( bmp  );
//  nsfbFree( nsfb );

}


int main( int argc, char **argv )
{// doWindow( "fb#320x240x32.W;300,400@vga" );
 // doWindow( "fb#320x240x32.N;600,300@vga" );
 // doWindow( "fb#300x200x32.S;700,10@vga" );

//  doWindow( "10.9.22.165:0#300x200x32.S;700,10@X11" );
  doWindow( ":0#600x400x32.N@X11" );
  doWindow( ":0#300x200x32.N@X11" );


  //doWindow( ":0.0#800x600x32.N@w32" );
  //doWindow( ":0.0#800x600x32.N@w32" );



  getchar();
  return( 0 );
}

