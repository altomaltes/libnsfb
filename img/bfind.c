/**
 *     AUTHOR: Jose Angel Caso Sanchez, 1993   ( altomaltes@yahoo.es )
 *                                             ( altomaltes@gmail.com )
 *
 *     Copyright (C) 1993, 2012 JACS
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
 *     FILE: bfind.c
 *     DATE: jul 1993
 *
 *  DESCRIPCION: Data manipulation routines
 *
 */


#include <string.h>

/** ====================================================[ JACS 1993-07-08 ]== *\
 *                                                                            *
 *   JACS 2011                                                                *
 *                                                                            *
 *  FUNCTION: bfind                                                           *
 *                                                                            *
 *  @brief busca elemento en array ordenado y lo mete en su sitio si no esta. *
 *                                                                            *
\* ========================================================================= **/
void * bfind( const void * key
            , const void * base
            , int * nmemb, int size
            , int (*compar)(const void *, const void *))
{ int l, u, idx;
  void * ptr= NULL;
  int comparison;

  idx= l = 0; u = *nmemb;

  while (l < u)
  { idx = (l + u) / 2;
    ptr = (((char *) base) + (idx * size));
    comparison = (*compar)(key, ptr );
    if ( comparison < 0 )
    { u = idx;
    }
    else if (comparison > 0)
    { l = idx + 1;
    }
    else
    { return( ptr );
  } }

  ptr= (((char *) base) + (l * size));

  idx= *nmemb - l;              /* Calcular los elementos a mover     */

  if ( idx>0 )
  { memmove(ptr+size, ptr, idx*size);  /* Hacer sitio para el nuevo elemento */
  }
  memcpy(ptr, key, size);              /* Copiar nuevo elemento              */

  (*nmemb)++;                          /* Indicar que hay un nuevo elemento  */

  return( ptr );
}




