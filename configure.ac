AC_PREREQ(2.53)
AC_INIT(libphone-ui-shr, 0.0.1, http://www.shr-project.org/)
AM_INIT_AUTOMAKE()
AM_CONFIG_HEADER(config.h)
AM_MAINTAINER_MODE

AC_GNU_SOURCE
AC_ISC_POSIX
AC_PROG_CC
AC_STDC_HEADERS
AC_PROG_LIBTOOL

# base deps
PKG_CHECK_MODULES(LIBPHONEUI, libphone-ui)
PKG_CHECK_MODULES(LIBPHONE_UTILS, libphone-utils)
PKG_CHECK_MODULES(GLIB, glib-2.0)
PKG_CHECK_MODULES(FSO_GLIB, fso-glib-1.0)
PKG_CHECK_MODULES(FSO_FRAMEWORK, fsoframework-2.0)

PKG_CHECK_MODULES(E, [
	evas
	ecore
	ecore-evas
	edje
	elementary
])

AC_SUBST(LIBPHONEUI_CFLAGS)
AC_SUBST(LIBPHONEUI_LIBS)
AC_SUBST(GLIB_CFLAGS)
AC_SUBST(GLIC_LIBS)
AC_SUBST(E_CFLAGS)
AC_SUBST(E_LIBS)

AC_ARG_WITH(edje-cc,
[  --with-edje-cc=PATH              specify a specific path to edje_cc],
[
  v=$withval;
  edje_cc=$v
  echo "  Enlightenment edje_cc explicitly set to "$edje_cc;
],[
  edje_cc=$(pkg-config --variable=prefix edje)/bin/edje_cc
])
AC_SUBST(edje_cc)

AC_SUBST(DATADIR, $datadir)

ALL_LINGUAS="ar ca de es fi gl pl ru sv cs"
AC_SUBST(ALL_LINGUAS)
AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION([0.12.1])

# output stuff
AC_OUTPUT([
Makefile
data/Makefile
data/idle_screen/Makefile
data/widgets/Makefile
src/Makefile
po/Makefile.in
])

