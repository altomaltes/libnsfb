/*****************************************************************************
*   "Gif-Lib" - Yet another gif library.				     *
*									     *
* Written by:  Gershon Elber				Ver 0.1, Jul. 1989   *
******************************************************************************
* Program to clip an image and dump out only portion of it.		     *
* Options:								     *
* -q : quiet printing mode.						     *
* -i left top width bottom : clipping information for first image.	     *
* -n n left top width bottom : clipping information for nth image.	     *
* -c complement; remove the bands specified by -i or -n			     *
* -h : on-line help							     *
******************************************************************************
* History:								     *
* 8 Jul 89 - Version 1.0 by Gershon Elber.				     *
*****************************************************************************/

#include "../include/config.h"
#include "../include/misc.h"

#include <stdlib.h>
#include <string.h>

#include "gif_lib.h"

/* Width and depth of clipped image.
 */
//  if ( !Complement )	ImageWidth = ImageX2 - ImageX1 + 1;
//                else	ImageWidth = in->SWidth - (ImageX2 != ImageX1) * (ImageX2 - ImageX1 + 1);
//
//  if ( !Complement )	ImageDepth = ImageY2 - ImageY1 + 1;
//                else	ImageDepth = in->SHeight - (ImageY2 != ImageY1) * (ImageY2 - ImageY1 + 1);

#define FALSE 0


/******************************************************************************
* Close both input and output file (if open), and exit.			      *
******************************************************************************/
static void QuitGifError(GifFileType *GifFileIn, GifFileType *GifFileOut)
{ int err;

 // PrintGifError();
  if ( GifFileIn  ) DGifCloseFile(GifFileIn , &err );
  if ( GifFileOut ) EGifCloseFile(GifFileOut, &err );
  exit( -1 );
}


int clipGif( GifFileType * out
           , GifFileType * in
           , int ImageX1   , int  ImageY1
           , int ImageWidth, int ImageDepth )
{
  int ImageNum = 0;
  int ImageN   = 1;		    /* Its first image we are after. */
  int error;
  int i, codeSize, extCode;

  GifRecordType RecordType;
  GifByteType * Extension
            , * CodeBlock;

  GifRowType Line;

//  GifFileType * out= EGifOpenFileHandle( outFd, error );
//  GifFileType * in=  DGifOpenFileHandle( inFd , error );

  int ImageX2= ImageX1 + ImageWidth - 1;
  int ImageY2= ImageY1 + ImageDepth - 1;


/* And dump out exactly same screen information:
 */
  if ( EGifPutScreenDesc( out
                        , in->SWidth, in->SHeight
                        ,	in->SColorResolution
                        , in->SBackGroundColor
                        , in->SColorMap) == GIF_ERROR )
	 { return( -1 );
  }

/* Scan the content of the GIF file and load the image(s) in:
  */
  do
  { if ( DGifGetRecordType( in, &RecordType) == GIF_ERROR )
	   { return( -2 );
    }

   	switch (RecordType)
    { case IMAGE_DESC_RECORD_TYPE:
      		if (DGifGetImageDesc(in) == GIF_ERROR)
		      { return( -3 );
        }

      		if ( ++ImageNum == ImageN )  /* We can handle only non interlaced images here: */
        { if (in->Image.Interlace)
       			{ puts( "Image to clip is interlaced - use GifInter first." ); return( -30 );
          }

/*
 * This is the image we should clip - test sizes and  dump out new clipped screen descriptor if o.k.
 */
		        if ( in->Image.Width  <= ImageX2
            ||	in->Image.Height <= ImageY2 )
  			     { puts("Image is smaller than given clip dimensions."); return( -31 );
          }

/* Put the image descriptor to out file:
 */
		        if ( EGifPutImageDesc( out
                               ,	in->Image.Left, in->Image.Top
                               ,	ImageWidth, ImageDepth
                               ,	FALSE, in->Image.ColorMap) == GIF_ERROR )
  			     { return( -4 );
          }

/* o.k. - read the image and clip it:
  */
		        Line = (GifRowType) alloca( in->Image.Width
                                    * sizeof(GifPixelType) );

//		        printf( "\n%s: Image %d at (%d, %d) [%dx%d]:     "
  //                  , "argv0", ImageNum
    //                ,	in->Image.Left, in->Image.Top
      //              ,	in->Image.Width, in->Image.Height );

/* Skip lines below ImageY1:
 */
  		      for ( i = 0
              ; i < ImageY1
              ; i++ )
          { if (DGifGetLine( in, Line, in->Image.Width)== GIF_ERROR )
   			      {  return( -5 );
            }
		        }

/* Clip the lines from ImageY1 to ImageY2 (to X1 - X2):
 */
		        for ( i  = ImageY1
              ; i <= ImageY2
              ; i++ )
          { if (DGifGetLine( in, Line, in->Image.Width) == GIF_ERROR)
		  	       {  return( -9 );
            }

			         if (EGifPutLine( out
                           , &Line[ ImageX1 ]
                           , ImageWidth) == GIF_ERROR)
  				      {  return( -10 );
            }
		        }

/* Skip lines above ImageY2:
 */
  		      for ( i = ImageY2 + 1
              ; i < in->Image.Height
              ; i++)
          { if (DGifGetLine(in, Line, in->Image.Width) == GIF_ERROR)
		  	       {  return( -11 );
      } }  	}
  		  else /* Copy the image as is (we dont modify this one): */
      { if ( EGifPutImageDesc( out
                             ,	in->Image.Left,  in->Image.Top
                             , in->Image.Width, in->Image.Height
                             ,	in->Image.Interlace
                             ,	in->Image.ColorMap) == GIF_ERROR )
    			 {  return( -15 );
        }

/*
 *   Now read image itself in decoded form as we dont  really care what is there,
 * and this is much faster.
 */
		      if ( DGifGetCode( in, &codeSize, &CodeBlock) == GIF_ERROR
  		      || EGifPutCode( out, codeSize,  CodeBlock) == GIF_ERROR )
  			   {  return( -16 );
        }
		      while (CodeBlock != NULL)
  			   { if ( DGifGetCodeNext( in, &CodeBlock) == GIF_ERROR
            || EGifPutCodeNext( out, CodeBlock) == GIF_ERROR)
          {  return( -17 );
      } }	}
		  break;

    case EXTENSION_RECORD_TYPE: /* Skip any extension blocks in file: */
      if (DGifGetExtension( in
                          , &extCode
                          , &Extension) == GIF_ERROR)
      {  return( -18 );
      }
    		if (EGifPutExtension(out, extCode, Extension[0],	Extension) == GIF_ERROR)
      {  return( -23 );
      }

	     while ( Extension) /* No support to more than one extension blocks, so discard: */
      { if ( DGifGetExtensionNext(in, &Extension) == GIF_ERROR)
 			    {  return( -19 );
    		} }
  	 break;

    case TERMINATE_RECORD_TYPE: break;
	   default:break;		    /* Should be traps by DGifGetRecordType. */
	 } }
  while ( RecordType != TERMINATE_RECORD_TYPE );

  if ( DGifCloseFile( in , &error ) == GIF_ERROR)
	 {  return( -20 );
  }

  if ( EGifCloseFile( out, &error ) == GIF_ERROR)
  {  return( -21 );
  }

  return 0;
}



/*
 * Interpret the command line and scan the given GIF file.		      *
 *
 */
int cropGifFile( const char * out
               , const char * in
               , int x, int y
               , int w, int h )
{ GifFileType * GifFileOut;
  GifFileType * GifFileIn;

  int err;


  if ((GifFileIn = DGifOpenFileName( in, &err )) == NULL)
	 { QuitGifError(GifFileIn, GifFileOut);
  }

/* Open stdout for the output file:
 */
  if ((GifFileOut = EGifOpenFileName( out, 0, &err )) == NULL)
	 { QuitGifError(GifFileIn, GifFileOut);
  }

  return( clipGif( GifFileOut, GifFileIn, x, y, w ,h ) );
}


/*
 *
 */
static int inputMemo( GifFileType * fl
                    , GifByteType * bt
                    , int sz )
{ memcpy( bt, fl->UserData, sz );

  fl->UserData += sz;

  return( sz );
}


PUBLIC void * gifFileOtHandle( const char * fname, int tst ){ int err; return( EGifOpenFileName( fname, tst     , &err )); }
PUBLIC void * gifFileInHandle( const char * fname )         { int err; return( DGifOpenFileName( fname          , &err )); }
PUBLIC void * gifMemoOtHandle( const void * memo )          { int err; return( EGifOpen( (void*)memo, inputMemo , &err )); }
PUBLIC void * gifMemoInHandle( const void * memo )          { int err; return( DGifOpen( (void*)memo, inputMemo , &err )); }



