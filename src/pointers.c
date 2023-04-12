/**
 *     AUTHOR: Jose Angel Caso Sanchez, 2023   ( altomaltes@yahoo.es )
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
 *     FILE: pointers.c
 *     DATE: mar 2023
 *
 *  DESCRIPCION: hardcoded pointers
 *
 */

#include "../nsfb.h"
//#include "../cursor.h"



#define DEFAULT_CURSOR_H 19
#define DEFAULT_CURSOR_W 11

static unsigned char cursDefault[ 1 ]
                                [ DEFAULT_CURSOR_H ]
                                [ DEFAULT_CURSOR_W ] =
{{{ 1,0,0,0,0,0,0,0,0,0,0 }
 ,{ 1,1,0,0,0,0,0,0,0,0,0 }
 ,{ 1,2,1,0,0,0,0,0,0,0,0 }
 ,{ 1,2,2,1,0,0,0,0,0,0,0 }
 ,{ 1,2,2,2,1,0,0,0,0,0,0 }
 ,{ 1,2,2,2,2,1,0,0,0,0,0 }
 ,{ 1,2,2,2,2,2,1,0,0,0,0 }
 ,{ 1,2,2,2,2,2,2,1,0,0,0 }
 ,{ 1,2,2,2,2,2,2,2,1,0,0 }
 ,{ 1,2,2,2,2,2,2,2,0,1,0 }
 ,{ 1,2,2,2,2,2,1,1,1,1,1 }
 ,{ 1,2,2,1,2,2,1,0,0,0,0 }
 ,{ 1,2,1,0,1,2,2,1,0,0,0 }
 ,{ 1,1,0,0,1,2,2,1,0,0,0 }
 ,{ 1,0,0,0,0,1,2,2,1,0,0 }
 ,{ 0,0,0,0,0,1,2,2,1,0,0 }
 ,{ 0,0,0,0,0,0,1,2,2,1,0 }
 ,{ 0,0,0,0,0,0,1,2,2,1,0 }
 ,{ 0,0,0,0,0,0,0,1,1,1,0 }}};


static ImgPalette cusrDefaultPal[]=
{{    0,     0,    0, 0xFF  }
,{ 0x00,  0x00, 0x00, 0x00  }
,{ 0xFF,  0xFF, 0xFF, 0x00  }};

IcoRec cursorDefault=
{ .pic= cursDefault
, .frm= NULL
, .wNat = DEFAULT_CURSOR_W, .hNat= DEFAULT_CURSOR_H   /* Native size    */
, .pics= 1   /* Native size    */
, .nCol= 0x003         /*  Number of frames and palettes  */
, .pal= cusrDefaultPal }; /*  Color palette                  */

static unsigned char rightIcoData[1][9][6] =
{{{ 1,  0,  1,  1,  1,  1 }
 ,{ 1,  0,  0,  1,  1,  1 }
 ,{ 1,  0,  0,  0,  1,  1 }
 ,{ 1,  0,  0,  0,  0,  1 }
 ,{ 1,  0,  0,  0,  0,  0 }
 ,{ 1,  0,  0,  0,  0,  1 }
 ,{ 1,  0,  0,  0,  1,  1 }
 ,{ 1,  0,  0,  1,  1,  1 }
 ,{ 1,  0,  1,  1,  1,  1 }}};

static ImgPalette rightIcoPal[]=
{{    0,     0,    0, 0x00  }
,{ 0xFF,  0xFF, 0xFF, 0xFF  }};

IcoRec cursorRight=
{ .pic= cursDefault
, .frm= NULL
, .wNat = 6, .hNat= 9   /* Native size    */
, .pics= 1              /* Native size    */
, .nCol= 0x02           /*  Number of frames and palettes  */
, .pal= rightIcoPal };  /*  Color palette                  */




