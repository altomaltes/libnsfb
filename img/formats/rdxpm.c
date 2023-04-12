/*
 * xvxpm.c - load routine for X11 XPM v3 format pictures
 *
 * LoadXPM(fname,pinfo)
 * WriteXPM(fp,pic,ptype,w,h,rp,gp,bp,nc,col,name)
 */

/*
 * Written by Chris P. Ross (cross@eng.umd.edu) to support XPM v3
 * format images.
 *
 * Thanks go to Sam Yates (syates@spam.maths.adelaide.edu.au) for
 * provideing inspiration.
 */

#include <stdio.h>
#include <malloc.h>

#ifndef _WIN32
  #include <alloca.h>
#endif

#include <string.h>

#define VALUES_LEN 80      /* Max length of values line */
#define TOKEN_LEN  8       /* Max length of color token */
#define PIC8       8
#define PIC24     24

#define F_BWDITHER   1
#define F_FULLCOLOR 24
#define F_GREYSCALE  8

#define DEBUG 10


#include "../../nsfb.h"
#include "../images.h"

/*
 * File format:
 *  (A file format def should go here)
 *
 */

typedef struct hent
{ struct hent * next;
  char token[ TOKEN_LEN ];
  unsigned char      index;
} hentry;

#define cv_index        color_val.index

/* Local (Global) Variables */

static hentry **hashtab;        /* Hash table                          */
static int      hash_len;       /* number of hash buckets              */
static int      bufchar;        /* Buffered character from XpmGetc     */
static short    in_quote;       /* Is the current point in the file in */



/***************************************/
/*         hashing functions           */
/***************************************/


/*
 *
 *
 */
static int hash( char *token )
{ int i, sum;

  for (i=sum=0; token[i] != '\0'; i++)
  { sum += token[i];
  }

  sum = sum % hash_len;
  return (sum);
}


/*
 * hash_init() - This function takes an arg, but doesn't do anything with
 * it.  It could easily be expanded to figure out the "optimal" number of
 * buckets.  But, for now, a hard-coded 257 will do.  (Until I finish the
 * 24-bit image writing code.  :-)
 */
static int hash_init( int hsize )
{ int i;

  hash_len = 257;

  hashtab = (hentry **) calloc( sizeof(hentry *) , hash_len);
  if ( !hashtab )
  { return 0;
  }

  for( i = 0
     ; i < hash_len
     ; i++ )
  { hashtab[i] = NULL;
  }

  return 1;
}


/*
 *
 *
 */
static hentry *hash_search( char *token )
{ int     key;
  hentry *tmp;

  key = hash(token);
  tmp = hashtab[key];

  while (tmp && strcmp(token, tmp->token))
  { tmp = tmp->next;
  }

  return tmp;
}

/*
 *
 *
 */
static int hash_insert( hentry *entry )
{ int     key;
  hentry *tmp;

  key = hash(entry->token);

  tmp = (hentry *) calloc(sizeof(hentry), 1);
  if (!tmp)
  { return 0;
  }

  memcpy( (char *)tmp
        , (char *)entry
        , sizeof(hentry));

  tmp->next= hashtab[ key ] ? hashtab[key] : NULL;
  hashtab[key] = tmp;

  return 1;
}

/*
 *
 *
 */
static void hash_destroy()
{ int     i;
  hentry *tmp;

  for (i=0; i<hash_len; i++)
  { while (hashtab[i])
    { tmp = hashtab[i]->next;
      free(hashtab[i]);
      hashtab[i] = tmp;
  } }

  free( hashtab );
  return;
}


/*
 *
 *
 */
static int XpmGetc( FILE *f )
{ int   c, d, lastc;

/* The last invocation of this routine read the character... */

  if (bufchar != -2)
  { c = bufchar;
    bufchar = -2;
    return(c);
  }

  if ((c = getc(f)) == EOF)
  { return(EOF);
  }

  if (c == '"')
  { in_quote = !in_quote;
  }
  else if (!in_quote && c == '/')      /* might be a C-style comment */
  { if ((d = getc(f)) == EOF)
    {  return(EOF);
    }

    if (d == '*')                              /* yup, it *is* a comment */
    { if ((lastc = getc(f)) == EOF)
      { return(EOF);
      }
      do
      { if ((c = getc(f)) == EOF)                               /* skip through to the end */
          return(EOF);
        if (lastc != '*' || c != '/')   /* end of comment */
          lastc = c;
      } while (lastc != '*' || c != '/');
      if ((c = getc(f)) == EOF)
        return(EOF);
    } else                                      /* nope, not a comment */
      { bufchar = d;
  }   }
  return(c);
}


/** ========================================= [ JACS, 10/01/2012 ] == *\
 *                                                                    *
 *   JASC 2012                                                        *
 *                                                                    *
 *  FUNCTION loadXpmFile                                              *
 *                                                                    *
 *  @brief                                                            *
 *                                                                    *
\* ================================================================= **/
ANSIC IcoRec * loadIcoXpmFile( const char * bname, int wtarget, int htarget )
{ FILE * fp;
  unsigned char   * picture;

  hentry   item;
  int      c;
  char     values[VALUES_LEN];
  int      w,h,nc, cpp, line_pos;
  short    i, j, k;             /* for() loop indexes */

  hentry  * clmp;    /* colormap hash-table            */
  hentry  * c_sptr;  /* cmap hash-table search pointer */

//  unsigned char * dst;

  IcoRec * icon;

  fp= fopen( bname, "r");
  if ( !fp )
  { return( NULL );  /* return( "couldn't open file" ); */
  }


  bufchar = -2;
  in_quote= 0;

/*
 * Read in the values line.  It is the first string in the
 * xpm, and contains four numbers.  w, h, num_colors, and
 * chars_per_pixel.
 */

  /* First, get to the first string */

  while (((c = XpmGetc( fp ))!=EOF) && (c != '"')) ;
  line_pos = 0;

  /* Now, read in the string */
  while (((c = XpmGetc( fp))!=EOF) && (line_pos < VALUES_LEN) && (c != '"'))
  { values[line_pos++] = c;
  }

  if (c != '"')
  { /* return( "error parsing values line" ); */  return( NULL );
  }

  values[line_pos] = '\0';
  sscanf( values
        , "%d%d%d%d"
        , &w, &h
        , &nc, &cpp );

  if ( nc <= 0 || cpp <= 0 )
  { /* return( "No colours in Xpm?" ); */  return( NULL );
  }

  /* info->type = (nc > 256) ? PIC24 : PIC8; */

  if (!hash_init( nc ))
  { /* return( "Not enough memory to hash colormap" ); */  return( NULL  );
  }


  clmp= (hentry *) alloca(nc * sizeof(hentry)); /* Holds the colormap */

  if ( !clmp  )
  { /* return( "Not enough memory to load pixmap" ); */  return( NULL  );
  }

  icon= ( IcoRec * ) calloc( sizeof( IcoRec )             /* Static size       */
                           + nc * sizeof( icon->pal ) /* Holds colormap    */
                           + w * h                        /* Holds image data  */
                           , 1 );
  icon->pal->red=
  icon->pal->blue=
  icon->pal->green= 0x7F;
  picture= icon->pic= (unsigned char *) ( &icon->pal[ nc+1 ]  );                   /* Points to image data */
  icon->wNat= w; icon->hNat= h; icon->nCol= 255;

  c_sptr= clmp;

/* pinfo->colType = F_BWDITHER;  Now, we need to read the colormap. */

  for ( i = 1
      ; i <= nc
      ; i++, c_sptr++ )
  { while (((c = XpmGetc( fp ))!=EOF) && (c != '"')) ;
    if (c != '"')
    { /* return( "Error reading colormap" ); */  return( NULL  );
    }

    for( j = 0
       ; j < cpp
       ; j++ )
    { c_sptr->token[ j ] = XpmGetc( fp );
    }

    c_sptr->token[j] = '\0';

    while (((c = XpmGetc( fp ))!=EOF) && ((c == ' ') || (c == '\t')));

    if (c == EOF)               /* The failure condition of getc() */
    { /* return( "Error parsing colormap line" ); */  return( NULL  );
    }


    do
    { char  key  [  3 ];
      char  color[ 40 ];  /* Need to figure a good size for this... */
    //  short hd;           /* Hex digits per R, G, or B */

      for( j=0
         ; j<2 && (c != ' ') && (c != '\t') && (c != EOF)
         ; j++ )
      { key[j] = c;
        c = XpmGetc( fp );
      }
      key[j] = '\0';

      while (((c = XpmGetc( fp ))!=EOF) && ((c == ' ') || (c == '\t'))) ;
      if (c == EOF)     /* The failure condition of getc() */
      { return( NULL );  /* return( "Error parsing colormap line" ); */
      }

      for ( j=0
          ; j<39
            && ( c != ' ' )
            && ( c != '\t')
            && ( c != '"' )
            && ( c != EOF )
          ; j++ )
      { color[ j ] = c;
        c = XpmGetc( fp );
      }
      color[j]= 0;

      while ((c == ' ') || (c == '\t'))
      { c = XpmGetc( fp );
      }

      if (key[0] == 's')        /* Don't find a color for a symbolic name */
      { continue;
      }

      c_sptr->index= parseColorName( color                  /* Mask   */
                                   , &icon->pal[ i ].red
                                   , &icon->pal[ i ].green
                                   , &icon->pal[ i ].blue  )
                    ? i : 0;
      if ( !c_sptr->index )  /* Has transparenrt color */
      { // icon->alpha= 0;
      }

      memcpy( (char *) &item
            , (char *) c_sptr
            , sizeof(item) );
      hash_insert( &item );


      if (*key == 'c')         /* This is the color entry, keep it. */
      { while (c!='"' && c!=EOF)
        { c = XpmGetc( fp );
        }
        break;
      }
    } while (c != '"');
  } /* for */


/*
 * Now, read the pixmap.
 */

  for ( i = 0
      ; i < h
      ; i++ )
  { while (((c = XpmGetc( fp ))!=EOF) && (c != '"')) ;

    if (c != '"')
    { /* return( "Error reading colormap" ); */  return( NULL  );
    }

    for( j = 0
       ; j < w
       ; j++ )
    { char pixel[TOKEN_LEN];
      hentry *mapentry;

      for ( k = 0
          ; k < cpp
          ; k++ )
      { pixel[ k ] = XpmGetc( fp);
      }

      pixel[ k ] = 0;
      mapentry = (hentry *) hash_search( pixel );

      *picture ++= mapentry
                 ? mapentry->index
                 : 0;               /* No colormap entry found.  What do we do?  Transparent is a good idea */

    }  /* for ( j < w ) */

    XpmGetc( fp );          /* Throw away the close " */
  }  /* for ( i < h ) */

  hash_destroy();

  if ( fp != stdin)
  { fclose( fp );
  }

  return( icon );
}


DeviceImageRec * loadImgXpmFile( const char * bname, int w, int h )
{ return( NULL );
}



