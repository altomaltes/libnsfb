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
 *     FILE: printer.c
 *     DATE: jul 2004
 *
 *  DESCRIPCION:  general format printer
 */

#include <ctype.h>
#include <stdarg.h>
#include <stdlib.h>

#define NAM_LONG 0x80000000
#define dword unsigned int

static void outerSz( void * dst, unsigned char c )
{ int * out= (int *) dst;
  *out= *out+1;
}


/** ================================================= [ JACS, 10/01/2006 ] == *\
**                                                                            **
**  JACS feb 2009                                                             **
**                                                                            **
**  FUNCTION: Printer.                                                        **
**                                                                            **
**   TX's RAM ubicated string, by means a circular buffer.                    **
**                                                                            **
\* ========================================================================== */
char Printer( void (*outer)( void *, unsigned char )
            , void * args
            , const char * fmt, va_list list )   // String version
{ int sz= 0;
  int write= 1;
  char filler;  /* Feb 2017, filler support */

  if ( !fmt )
  { return( -1 );
  }

  if ( !outer )
  { outer= outerSz;
    write= 0;
  }

  while( *fmt )
  { if ( *fmt== '%' )
    { unsigned char store[ 16 ];           // entities formation
      unsigned u;
      unsigned u1;
      signed char sgn= 0;  // sign
      signed char prc= 0;  // Integer part
      signed char dec= 0;  // Decimals
             char dpt= 0;  // Decimal point

      unsigned char * s= store             // String operations
             + ( sizeof( store ) - 1 );
      *s= 0;

      fmt++; filler= ' ';
      if ( *fmt == '0' )
      { filler= *fmt++;
      }

      while( isdigit( *fmt ))
      { prc *= 10;
        prc += ( *fmt++ ) - '0';
      }

      if ( *fmt == '.' )               // Decimal point found
      { fmt++; while( isdigit( *fmt ))
        { dec *= 10;
          dec += ( *fmt++ ) - '0';
      } }

      switch( *fmt++ )
      { case '%':
         s= store; s[ 0 ]= '%'; s[ 1 ]= 0;
        break;

        case 'Q':
          u= write ? va_arg( list, int ) : 0; // Pointer to a fixed point

          while( dec-- )
          { s--; if ( s <= store )
            { break;
            }
            *s= '-'; dpt= filler; if ( u != NAM_LONG )
            { dpt= '.'; *s= ( u % 10 ) + '0'; u /= 10;
          } }

          if ( dpt )
          { s--; *s= dpt;
          }

          do
          { s--; prc--;
            if ( s <= store )
            { break;
            }
            *s= '-'; if ( u != NAM_LONG )
            { *s= ( u % 10 ) + '0'; u /= 10;
          } }
          while( u && prc );
        break;

        case 's':                  // Copy strings
          s= write ? va_arg( list, unsigned char * ) : (unsigned char*)"NULL"; // Copy string, fall out
        break;

        case 'c':
          u= write ? va_arg( list, unsigned ) : 0;
          outer( args, u ); sz++;
        continue;

        case 'd':
          sgn ++;

        case 'u':
          u= write ? va_arg( list, unsigned ) : 0;

          if ( sgn )
          { if ( u == NAM_LONG )
            { break;
            }

            if ( (int)u < 0 )
            { outer( args, '-' ); sz++; prc--; u= -u;          // Room for minus sign
          } }

          do
          { prc--; s--;
            if ( s <= store )
            { break;
            }
            *s= ( u % 10 ) + '0';
            u /= 10;
          }
          while( u );
        break;

        case 'l':
        { dword d= write ? va_arg( list, dword ) : 0;

          switch( *fmt++ )
          { case 'u':
              do
              { prc--; s--;
                if ( s <= store )
                { outer( args, '?'); sz++;
                  break;
                }
                *s= ( d % 10 ) + '0';
                d /= 10;
              }
              while( d );
            break;
        } }

        case 'x':
        case 'X':
        { dword d= write ? va_arg( list, dword ) : 0;
          do
          { prc--; s--;
            if ( s <= store )
            { break;
            }
            *s= ( d & 0x0F ) + '0';
            if ( *s > '9' )         // Start capital letters
            { *s += ( '@'-'9') ;
            }
            d >>= 4;
          }
          while( d );
          while( prc > 0 ) { outer( args, '0' );  sz++; prc--; }
        }
        break;

        case 'D':    /* From 16 bit to degrees */
          u= write ? va_arg( list, unsigned ) : 0;   /* 0 .. 360 ( no 180 .. -180  )*/

          u *= 360 * 60 ; u /= 0x10000;    /* To minutes */

          u1 = u % 60;   /* Minutes */
          u  = u / 60;   /* Degrees */

          s--; *s= ( u1 % 10 ) + '0'; u1 /= 10;
          s--; *s= ( u1 % 10 ) + '0'; u1 /= 10;
          s--; *s=':';
          s--; *s= ( u  % 10 ) + '0'; u  /= 10;
          s--; *s= ( u  % 10 ) + '0'; u  /= 10;
          s--; *s= ( u  % 10 ) + '0'; u  /= 10;

        break;

        case 'M':   /* Minutes to hours .. minutes */
          u= write ? va_arg( list, int ) : 0; // Write or inquire size

          int h= u / 60; int m = u % 60;       // Split in hours and minutes

          s--; *s= ( u == NAM_LONG ) ? '-' : ( m % 10 ) + '0'; m /= 10;
          s--; *s= ( u == NAM_LONG ) ? '-' : ( m % 10 ) + '0'; m /= 10;
          s--; *s= ':';

          do
          { s--; prc--;
            if ( s <= store )
            { break;
            }
            if ( u != NAM_LONG )
            { *s= ( h % 10 ) + '0'; h /= 10;
          } }
          while( h && prc );

        break;

        case 0:
        return( 0 );

        default:
        continue;
      }

      while( prc > 0 ) { outer( args, filler  );  sz++; prc--; }

     if ( !s) s= (unsigned char*)"NNN";
      while( *s      ) { outer( args, *s++ );  sz++;        }  // RAM located, Copy result string

      continue;
    }
    outer( args, *fmt++ );  sz++;
  }

  return( sz );
}

int printerSize( const char * fmt )
{ int sz= 0;

  va_list voidList;

  Printer( NULL, &sz, fmt, voidList );

  return( sz );
}
