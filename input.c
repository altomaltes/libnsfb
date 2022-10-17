/* libnsfb plotter test program
 */

#include <limits.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/input.h>



typedef struct
{ struct input_id id;

  char name[ 32 ];
  char phys[ 32 ];
  char uniq[ 32 ];


  unsigned bits[ 32 ];

} InputPtr;



InputPtr * ptr;



static void loadDevices( int devnr )
{ int i,bit,rc;
  char filename[ PATH_MAX ];
  struct stat statbuf;

  for (i = 0; i < 32; i++)
  { snprintf(filename, sizeof(filename)
                     , "/dev/input/event%d", i );

    if ( !stat(filename, &statbuf))
    { int fd= open(filename,O_RDONLY);

      if ( fd >= 0 )
      { int version;

        if ( ioctl(fd,EVIOCGVERSION, &version) >=0 )
        { if ( version == EV_VERSION )
          { if ( ioctl(fd,EVIOCGID,&ptr->id) >=0 )
            { if ( ioctl(fd,EVIOCGNAME(sizeof(ptr->name)),ptr->name) >= 0 )
              { if ( ioctl(fd,EVIOCGPHYS(sizeof(ptr->phys)),ptr->phys) >= 0 )
                { if ( ioctl(fd,EVIOCGUNIQ(sizeof(ptr->uniq)),ptr->uniq))
                  { if ( ioctl(fd,EVIOCGBIT(0,sizeof(ptr->bits)),ptr->bits) >= 0 )
                    { for ( bit = 0; bit < rc*8 && bit < EV_MAX; bit++)
                      { if (test_bit(bit,bits))
                        { fprintf(stderr," %s", EV_NAME[bit]);
} } } } } } } } } } } } }


