/*
 * Jose Angel Sánchez Caso (C) 2003.  (JASC)
 *
 *  COVERER by the GNU GPL license.
 *
 *  altomaltes@yahoo.es
 *  altomaltes@gmail.com
 *
 *  COVERER
 *
 *  Jul-2004
 *
 */

#ifndef IMAGES_INCLUDED
#define IMAGES_INCLUDED

//#ifndef WINGDIAPI
//  #define HBITMAP void *
//#endif


typedef void ( * ImgResizeProc )( void                * userData
                                , const unsigned long * line
                                , unsigned      wide
                                , unsigned      row
                                , unsigned      coef );  /* File/column crossing */


/*
 * Type hidding
 */

//  typedef struct HistRecStruct    HistRec;
  typedef struct quantizerStruct  quantizer;
  typedef struct ChangerRecStruct ChangerRec;

typedef struct
{ unsigned w;
  unsigned h;

  unsigned char * picture;
  unsigned char * mask;

} PicInfo;

#define ICOREC_MIRROR_PALETTE 0x80000000

typedef struct DeviceImageRec_s
{ struct DeviceImageRec * next;  /* To do a linked list         */
  IcoRec                * iden;  /* Image identifier            */
  HistRec               * hist;  /* Associated histogram        */

  ImageMap map;                  /* OS depandent mask and image */

  unsigned short width;
  unsigned short height;
  unsigned short pics;

  unsigned char * mask;          /* Points forward, to possible mask     */
  unsigned char   image[ 4 ];
} DeviceImageRec;



  typedef int (* ChangerFun) ( void *, unsigned char *, unsigned char * );


#ifdef __cplusplus
  extern "C"  {
#endif

  #define QUANTIZE( sz, q ) (sz ? (( sz - 1 ) >> q ) + 1 : 0)
  #define ALIGN( w, b ) { w --; w >>= b; w ++; w <<= b; }

/* Image alignement depends
 */

#ifdef _WIN32
  #define IMAGEALIGN 2
  #define IMAGELINE  sizeof( dword )
#else
  #define IMAGEALIGN 1
  #define IMAGELINE  sizeof( word )
#endif

/*
 * Pertenecientes a rdxpm.c
 */
/*  const char * LoadXPM( PicInfo    * data
                      , const char * filename
                      , int        * szx
                      , int        * szy
                      , HistRec    * hist );
  */

  IcoRec * LoadXpmFile( const char * fName );
  IcoRec * LoadGifFile( const char * fName );
  IcoRec * LoadICOFile( const char * fName );
  IcoRec * LoadIcoFile( const char * fName );



/*
 *  From cropgif.c
 *
 */
  ANSIC void * gifFileInHandle( const char * fname )         ;
  ANSIC void * gifFileOtHandle( const char * fname, int tst );
  ANSIC void * gifMemoInHandle( const void * memo )          ;

  ANSIC int clipGif( void * out
                    , void * in
                    , int ImageX1   , int  ImageY1
                    , int ImageWidth, int ImageDepth );




/*
 *  From icons.c
 *
 */
 DeviceImageRec * openIcoFromData2
                  ( int           deep       /* Histogram          */
                  , int pics, int cols       /* Numbero of images  */
                  , int wDst, int hDst       /* desired width and height  */
                  , int wOrg, int hOrg       /* stored  width and height  */
                  , unsigned char * picture  /* Image data         */
                  , ImgPalette    * pal );   /* Returns image mask */

DeviceImageRec * openIcoFromData1
                  ( unsigned char * picture    /* Image data               */
                  , int wDst, int hDst         /* desired width and height */
                  , int wOrg, int hOrg         /* stored  width and height */
                  , int pics, int pals         /* Numbero of palettes      *//* Number of pictures       */
                  , ImgPalette    * palette ); /* Returns image mask       */


  DeviceImageRec * openIco
                  ( IcoRec * src, int deep  /* Histogram              */
                  , int wDst, int hDst );   /* IN ->  Icon size       */


/*  From images.c
 */

  DeviceImageRec * LoadPng ( const char * fName, int * szW, int * szH );
  DeviceImageRec * LoadJfif( const char * fName, int * szW, int * szH );
  DeviceImageRec * LoadImg ( const char * fName, int * szW, int * szH );

/*
 * from resource.c
 */

  DeviceImageRec * LoadICO
                   ( unsigned char       * data    /* Carga de icono de tamaño dado */
                   , int        * sz  );

  DeviceImageRec * LoadXBM
                   ( unsigned char       * data    /* Carga un bitmap a un tamaño */
                   , int        * szx
                   , int        * szy
                   , unsigned long        fg      /* Foreground color */
                   , unsigned long        bg );



/*  From parsec.c
 */

  int parseColorName( const char * name
                    , unsigned char * r
                    , unsigned char * g
                    , unsigned char * b );

/*
 *  Returns a 32 bit index based in a color name
 */

/*
 *  From dither.c
 */

  int getHistDeep      ( const HistRec * hist );
  int getHistPad       ( const HistRec * hist );
  int getHistWide       ( const HistRec * hist );
  void * getHistDevice    ( const HistRec * hist );
  unsigned long  getHistBackground( const HistRec * hist );

  dword   HistColor( HistRec * hist
                   , dword  c0
                   , dword  c1
                   , dword  c2 );

   void  HistogramColor( HistRec * hist
                       , dword index
                       , unsigned char * c0
                       , unsigned char * c1
                       , unsigned char * c2 );

  ANSIC HistRec * NewHistogram( int planes              /* Fabrica un nuevo histograma */
                                );         /* Extra space */
  void ChangeImageAddRGB( ChangerRec * changerArr[]
                         ,       unsigned char * src  );


/*
 * From resize.c
 */

int changerSize( int wd, int pl );
void * changerLine( ChangerRec * changer );   /* Process info               */

#define allocChanger( pl, wd ) alloca( changerSize( wd, pl ))


  /*
   * Codec for bitmaps
   */

  void giveBitmap( void        * userData
                 , const dword * line
                 , unsigned      wide
                 , unsigned      row
                 , unsigned      coef );

  ChangerRec * initChanger( ChangerRec * changer  /* Process info                */
                          , int planes         /* Number of scanlines */
                          , ImgResizeProc given, void * data
                          , int wDst, int hDst    /* Destination wide and height */
                          , int wOrg, int hOrg ); /* Original wide and height    */


  void * initChangerByte( ChangerRec * changer /* Process info */
                        , int wDst             /* Destination wide    */
                        , int hDst             /* Destination height  */
                        , int wOrg             /* Original wide       */
                        , int hOrg  );         /* Original height     */

  void * initChangerBit ( ChangerRec * changer /* Process info */
                        , int wDst             /* Destination wide    */
                        , int hDst             /* Destination height  */
                        , int wOrg             /* Original wide       */
                        , int hOrg  );         /* Original height     */

  void   changeImageAddLine( ChangerRec * changer );

  void * changeImageSize( ChangerRec * changer );

  void * initChangerSize( int     * wDst             /* Destination wide   */
                         , int     * hDst             /* Destination height */
                         , int       wOrg             /* Original wide      */
                         , int       hOrg  );         /* Original height    */


  DeviceImageRec * initImageMap
                 ( ChangerRec * changerRgb
                 , ChangerRec * changerMask
                 , int deep, int pics   /* Number of planes   */
                 , int wDst, int hDst   /* Destination wide and  height */
                 , int wOrg, int hOrg ); /* Original wide and height    */


  DeviceImageRec * initAlphaMap
                 ( ChangerRec *
                 , int pics              /* Number of planes   */
                 , int wDst, int hDst    /* Destination wide and  height */
                 , int wOrg, int hOrg ); /* Original wide and height    */


#ifdef __cplusplus
  }
#endif


#endif

