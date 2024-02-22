AUTOMAKE_OPTIONS=gnu
ACLOCAL_AMFLAGS = -I m4

SUBDIRS =  . arena

AM_CPPFLAGS= ${FT2_CFLAGS}

AM_LDFLAGS=   ${LDFLAGS} -release $(VERSION) -shared -L$(top_builddir)/.libs
MODULE_FLAGS=  $(AM_LDFLAGS) -module -lnsfb-$(VERSION) -no-undefined

lib_LTLIBRARIES=  libnsfb.la
lib_LTLIBRARIES+= nsfb-gif.la nsfb-png.la nsfb-jpg.la nsfb-svg.la

library_includedir= $(includedir)
library_include_HEADERS=libnsfb.h nsfbPlot.h libnsfb_cursor.h

libnsfb_la_SOURCES= libnsfb.c surface/ram.c surface/surface.c plot/generic.c plot/util.c plot/24bpp.c plot/8bpp.c plot/32bpp-xrgb8888.c plot/32bpp-xbgr8888.c  plot/16bpp.c  plot/api.c cursor.c palette.c  dump.c nsglobe.c
libnsfb_la_SOURCES+=img/dither.c img/resize.c img/parsec.c img/printer.c img/icons.c img/freetype.c img/formats/rdico.c img/formats/rdxpm.c img/vectors.c src/pointers.c

libnsfb_la_LDFLAGS= $(AM_LDFLAGS) ${FT2_LIBS}


if WINDOWS
  libnsfb_la_SOURCES+= plot/w32.c surface/w32.c
  libnsfb_la_LDFLAGS+= -lgdi32 -no-undefined
endif

if LINUX

  lib_LTLIBRARIES+= nsfb-X11.la nsfb-drm.la nsfb-vga.la

  nsfb_X11_la_SOURCES= surface/x11.c plot/x11.c
  nsfb_drm_la_SOURCES= surface/drm.c
  nsfb_vga_la_SOURCES= surface/linux.c surface/vt.c

  nsfb_X11_la_LDFLAGS= $(MODULE_FLAGS) -lX11
  nsfb_drm_la_LDFLAGS= $(MODULE_FLAGS) -ldrm
  nsfb_vga_la_LDFLAGS= $(MODULE_FLAGS)

  nsfb_X11_la_CFLAGS= -DBUILDING_SO_MODULE $(AM_CFLAGS)
  nsfb_drm_la_CFLAGS= -DBUILDING_SO_MODULE $(AM_CFLAGS) -I/usr/include/drm -I/usr/include/libdrm
  nsfb_vga_la_CFLAGS= -DBUILDING_SO_MODULE $(AM_CFLAGS)

  libnsfb_la_LDFLAGS+= -ldl

endif


#libnsfb_la_SOURCES+=arena/testgif.c

nsfb_gif_la_SOURCES= img/formats/rdgif.c
nsfb_png_la_SOURCES= img/formats/rdpng.c
nsfb_jpg_la_SOURCES= img/formats/rdjpeg.c
nsfb_svg_la_SOURCES= img/formats/rdsvg/color2.c img/formats/rdsvg/gradient.c img/formats/rdsvg/list.c img/formats/rdsvg/svgtiny.c img/formats/rdsvg/xml2dom.c  img/formats/rdsvg/rdsvg.c


nsfb_gif_la_CFLAGS= -DBUILDING_SO_MODULE $(AM_CFLAGS)
nsfb_png_la_CFLAGS= -DBUILDING_SO_MODULE $(AM_CFLAGS)
nsfb_jpg_la_CFLAGS= -DBUILDING_SO_MODULE $(AM_CFLAGS)
nsfb_svg_la_CFLAGS= -DBUILDING_SO_MODULE $(AM_CFLAGS) $(XML_CFLAGS)

EXTRA_DIST = configure.ac


nsfb_gif_la_LDFLAGS= $(MODULE_FLAGS) -lgif
nsfb_png_la_LDFLAGS= $(MODULE_FLAGS) -lpng
nsfb_jpg_la_LDFLAGS= $(MODULE_FLAGS) -ljpeg
nsfb_svg_la_LDFLAGS= $(MODULE_FLAGS) $(XML_LIBS)

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libnsfb.pc

# Get rid of these, you may also preach about its utility
#
install-exec-hook:
	@(cd $(DESTDIR)$(libdir) && ls && pwd && $(RM) $(lib_LTLIBRARIES))

if BUILDING_SLACK


slack:
	make install DESTDIR=$(abs_builddir)/pkg.${build_cpu}
	$(MKDIR_P) $(abs_builddir)/pkg.${build_cpu}/install
	cp ${top_srcdir}/slack-desc $(abs_builddir)/pkg.${build_cpu}/install
	cd $(abs_builddir)/pkg.${build_cpu}; ${SLACKBUILD} -l y -c n $(abs_builddir)/${PACKAGE}-${VERSION}-${build_cpu}-${SVN_BUILD}.txz

else

slack:
	echo "No slackware package requested ${PACKAGE}-${VERSION}-${SVN_BUILD}"

endif




