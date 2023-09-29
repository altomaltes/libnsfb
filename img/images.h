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
 *  From cropgif.c
 *
 */
ANSIC void * gifMemoOtHandle( const void * memo  );
ANSIC void * gifMemoInHandle( const void * memo  );
ANSIC void * gifFileOTHandle( const char * fname );
ANSIC void * gifFileOtHandle( const char * fname );
ANSIC void * gifFileInHandle( const char * fname );
ANSIC void * gifDescOtHandle( int          hnd   );
ANSIC void * gifDescInHandle( int          hnd   );

  ANSIC int clipGif( void * out
                   , void * in
                   , int ImageX1   , int  ImageY1
                   , int ImageWidth, int ImageDepth );




/*
 *  From icons.c
 *
 */
ANSIC DeviceImageRec * openIco( IcoRec * ico );



/*  From images.c
 */

ANSIC DeviceImageRec * LoadPng ( const char * fName );
ANSIC DeviceImageRec * LoadJfif( const char * fName );
ANSIC DeviceImageRec * LoadImg ( const char * fName );

/*
 * from resource.c
 */

ANSIC DeviceImageRec * LoadICO
                   ( unsigned char * file    /* Carga de icono de tamaño dado */
                   , int           * sz  );

ANSIC DeviceImageRec * LoadXBM
                   ( unsigned char       * data    /* Carga un bitmap a un tamaño */
                   , int        * szx
                   , int        * szy
                   , unsigned long        fg      /* Foreground color */
                   , unsigned long        bg );



/*  From parsec.c
 */

ANSIC int parseColorName( const char * name
                    , unsigned char * r
                    , unsigned char * g
                    , unsigned char * b );

/*
 *  Returns a 32 bit index based in a color name
 */

/*
 *  From dither.c
 */

ANSIC  int getHistDeep      ( const HistRec * hist );
ANSIC int getHistPad       ( const HistRec * hist );
ANSIC int getHistWide       ( const HistRec * hist );
ANSIC void * getHistDevice    ( const HistRec * hist );
ANSIC unsigned long  getHistBackground( const HistRec * hist );

ANSIC dword   HistColor( HistRec * hist
                   , dword  c0
                   , dword  c1
                   , dword  c2 );

ANSIC  void  HistogramColor( HistRec * hist
                       , dword index
                       , unsigned char * c0
                       , unsigned char * c1
                       , unsigned char * c2 );

ANSIC HistRec * NewHistogram( int planes              /* Fabrica un nuevo histograma */
                                );         /* Extra space */
ANSIC  void ChangeImageAddRGB( ChangerRec * changerArr[]
                         ,       unsigned char * src  );


/*
 * From resize.c
 */

ANSIC int changerSize( int wd, int pl );
ANSIC void * changerLine( ChangerRec * changer );   /* Process info               */

#define allocChanger( pl, wd ) alloca( changerSize( wd, pl ))

ANSIC void * getAlpha( unsigned char * mask
               , int w, int h, int coef
               , unsigned char * alpha );

ANSIC void * getCol16( unsigned short * image
               , int w, int h
               , unsigned char * array );

ANSIC void * getCol32( dword * image
               , int w, int h
               , unsigned char * array );

ANSIC void AddHistogram( HistRec * hist
                 , unsigned char R
                 , unsigned char G
                 , unsigned char B );  // Fabrica un nuevo histograma




#define QUANTIZE( sz, q ) (sz ? (( sz - 1 ) >> q ) + 1 : 0)
#define GETALPHA( w,h,a) getAlpha( (unsigned char*)alloca( QUANTIZE( w , 4 ) * h * sizeof( word )), w,h,0x80,a )
#define GETCOL16( w,h,a) getCol16( (word*)alloca( 4 * w * h * sizeof( word )), w,h,a )  // 4 ???
#define GETCOL32( w,h,a) getCol32((dword*)alloca( 8 * w * h * sizeof(dword )), w,h,a )








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
  void    changeImageAddRGB( ChangerRec * changer ); /* Line */

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

