AUTOMAKE_OPTIONS=gnu
ACLOCAL_AMFLAGS = -I m4
lib_LTLIBRARIES= libnsfb.la

library_includedir= $(includedir)/altomaltes
library_include_HEADERS=libnsfb.h libnsfb_plot.h libnsfb_cursor.h

libnsfb_la_SOURCES= libnsfb.c surface/linux.c surface/ram.c surface/surface.c surface/vt.c plot/generic.c plot/util.c plot/24bpp.c plot/8bpp.c plot/32bpp-xrgb8888.c plot/32bpp-xbgr8888.c  plot/16bpp.c  plot/api.c cursor.c palette.c  surface/drm.c dump.c nsglobe.c
libnsfb_la_LDFLAGS= -release $(VERSION) -ldrm

EXTRA_DIST = configure.ac
# SUBDIRS =  arena .

if BUILDING_SLACK


slack:
	make install DESTDIR=$(abs_builddir)/pkg.${build_cpu}
	$(MKDIR_P) $(abs_builddir)/pkg.${build_cpu}/install
	cp ${top_srcdir}/slack-desc $(abs_builddir)/pkg.${build_cpu}/install
#	cd $(abs_builddir)/pkg.${build_cpu}; ${SLACKBUILD} -l y -c n /var/cache/packages/${build_cpu}/extra/${PACKAGE}-${VERSION}-${build_cpu}-${SVN_BUILD}.txz
	cd $(abs_builddir)/pkg.${build_cpu}; ${SLACKBUILD} -l y -c n $(abs_builddir)/../../${build_cpu}/${PACKAGE}-${VERSION}-${build_cpu}-${SVN_BUILD}.txz

else

slack:
	echo "No slackware package requested ${PACKAGE}-${VERSION}-${SVN_BUILD}"

endif




