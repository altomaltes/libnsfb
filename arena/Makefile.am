#
#     José Angel Caso Sanchez (altomaltes@gmail.com)
#
#    jul 2004
#
#    jul 2017 -> unthreaded comm library
#    jul 2022 -> taken as template
#

noinst_PROGRAMS= bitmap polystar polystar2 tspeed

AM_CPPFLAGS= -I$(top_srcdir)/

tspeed_CPPFLAGS = -I/usr/include/freetype2
tspeed_LDFLAGS  = -lfreetype


LDADD= $(top_builddir)/libnsfb.la -lm

bitmap_sources=    bitmap.c
polystar_sources=  polystar.c
polystar2_sources= polystar2.c
tspeed_sources=    text-speed.c

