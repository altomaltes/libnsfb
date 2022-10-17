/**
 *     AUTHOR: Jose Angel Caso Sanchez, 2012   ( jascaso@gijon.es )
 *
 *
 *     FILE: vt.c
 *     DATE: ene 2019
 *
 *  DESCRIPCION:
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h>

#include <linux/vt.h>
#include <linux/kd.h>

#define SWITCHSIG	SIGXCPU        /* Just a random signal which is rarely used */
#define CONSOLE_DEV "/dev/tty0"

static void * userData;

int linuxFbOutput( int active, void * );


static int TTY;

static void changeVt( int i, siginfo_t * inf, void * v)
{ if ( ioctl( TTY, VT_RELDISP, 1 ) < 0 ) /* Try to release first */
  { ioctl( TTY, VT_RELDISP, VT_ACKACQ );
    linuxFbOutput( 1, userData );  
  }
  else
  { linuxFbOutput( 0, userData ); 
} }

static struct vt_stat vtstat;

static void restore( void )
{ ioctl( TTY, VT_RELDISP, 1 ); /* Give screen to kernel */
  if ( ioctl( TTY, VT_ACTIVATE, vtstat.v_active ) < 0 )
  { error( 1, errno, "failed to switch to former console"); 
  }

  close( TTY );
}

int openVt( int flags, void * user )
{ char filename[ 256 ];
  int vtConsole;
  struct sigaction sa;
  struct vt_mode vtmode;

  int tty0= open( CONSOLE_DEV, O_WRONLY | O_CLOEXEC );
  
  if (tty0 < 0)
  { error( 1, errno, "could not open tty0" );
    return( -1 ); 
  }

  ioctl( tty0, VT_GETSTATE, &vtstat ); // Get current console

  if ( ioctl( tty0, VT_OPENQRY, &vtConsole ) < 0 || vtConsole < 0 )
  { error( 1, errno, "failed to find non-opened console"); 
    return( -2 ); 
  } 
  close( tty0 );   // Not needed from now

  snprintf( filename, sizeof( filename )
          , "/dev/tty%d", vtConsole );

  TTY= open( filename, O_RDWR | O_CLOEXEC );

  if ( TTY < 0 )
  { error( 1, errno, "failed to open vconsole %d", filename ); 
  } 

  userData= user;                        /* Very important. BEFORE interrupt handler */

  if ( ioctl( TTY, KDSETMODE, KD_GRAPHICS ) < 0 )
  { error( 1, errno, "failed to switch to graph"); 
  } 

/* Setup the sigal handler
 */
  sigemptyset( &sa.sa_mask );
  sigfillset(  &sa.sa_mask );

  sa.sa_flags= SA_RESTART | SA_SIGINFO;
  sa.sa_sigaction= changeVt; sigaction( SWITCHSIG, &sa, NULL );

  vtmode.mode  = VT_PROCESS;
  vtmode.waitv = 0;
  vtmode.relsig= SWITCHSIG;
  vtmode.acqsig= SWITCHSIG;


  if ( ioctl( TTY, VT_SETMODE, &vtmode  ) < 0 )
  { error( 1, errno, "failed manage graph VT mode"); 
  } 

  if ( flags )  /* Want to switch to new created console */
  { if ( ioctl( TTY, VT_ACTIVATE, vtConsole  ) < 0 )
    { error( 1, errno, "failed to switch to new console"); 
  } } 

  return( atexit( restore ) );
} 

#ifdef TESTMAIN

int main()
{ printf("Hello world!\n");          
  openVt( 1 );
  getchar();
  return 0;
}

#endif
