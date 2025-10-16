/* libnsfb framebuffer surface support */

#include "nsfb.h"
#include "nsfbPlot.h"
#include "img/images.h"

/* surface default options
 */
typedef int (NsfbSurfacefnDefaults)( Nsfb * );

/* surface init
 */
typedef int (NsfbSurfacefnInit)( Nsfb * );

/* surface finalise
 */
typedef int (NsfbSurfacefnFini)( Nsfb * );

/* surface set geometry
 */
typedef int (NsfbSurfacefnGeometry)( Nsfb *
                                   , int width, int height
                                   , enum NsfbFormat format );

/* surface set parameters
 */
typedef int (NsfbSurfacefnParameters)(Nsfb *, const char *parameters);

/* surface area claim
 */
typedef int (NsfbSurfacefnClaim)(Nsfb *, NsfbBbox * );

/* surface area update
 */
typedef int (NsfbSurfacefnUpdate)(Nsfb *, NsfbBbox * );

/* surface cursor display
 */
typedef int (NsfbSurfacefnCursor)( struct NsfbSurfaceRtnsSt * );

/* surface finalise
 */
typedef int (NsfbSurfacefnPan)(Nsfb *, int type );

/** Create pixmapp
 */
typedef int( *NsfbPlotfnPixmap )( NsfbSurfaceRtns * surf
                                , ImageMap  * map
                                , void * img, void * msk
                                , int w, int h  );



typedef struct NsfbSurfaceRtnsSt
{ struct NsfbSurfaceRtnsSt * next;       /** List of registered surfaces */
  struct NsfbSt            * clients;    /** List of surface clients     */
  struct RenderListSt      * renderList; /** Cache of fonts              */
  struct NsfbCursorSt      * pointer;    /** cursor                  */

  enum NsfbType   type;       /* JACS, octy 2022 */
  const char    * name;
  enum NsfbFormat format;     /** Native Framebuffer format */

  int fd;                     /** device handler          */
  int dataSize;               /** Variable allocated client data */

  int panType;                /** Framebuffer outputing                   */

  int    buffSize;
  void * buffStart;

  int stride
    , theWidth, theHeigth
    , theDepth, theGeo;


/* Those available to caller
 */
  NsfbSurfacefnInit     * initialise;
  NsfbSurfacefnFini     * finalise;
  NsfbSurfacefnGeometry * geometry;
  NsfbSurfacefnClaim    * claim;
  NsfbSurfacefnUpdate   * update;
  NsfbSurfacefnCursor   * cursor;
  NsfbSurfacefnPan      * pan;

  NsfbPlotfnPixmap      pixmap;


/* Must be called by client on events
 */

  NsfbSurfacefnEvents     events;

} NsfbSurfaceRtns;

ANSIC void _nsfb_register_surface(  NsfbSurfaceRtns * );


/* macro which adds a builtin command with no argument limits
 */
#define NSFB_SURFACE_DEF( __rtns )                       \
  static void __rtns##_register_surface(void) __attribute__((constructor)); \
  void __rtns##_register_surface(  )      \
  { _nsfb_register_surface( &__rtns );               \
  }

/** Obtain routines for a surface
 *
 * Obtain a vlist of methods for a surface type.
 *
 * @param type The surface type.
 * @return A vtable of routines which teh caller must deallocate or
 *         NULL on error
 */
//NsfbSurfaceRtns * nsfbSurfaceGet Rtns( enum NsfbType type );

