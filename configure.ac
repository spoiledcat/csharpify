AC_INIT([csharpify], [1.0])
AC_CONFIG_SRCDIR([README.md])
AC_CANONICAL_TARGET
AM_INIT_AUTOMAKE([foreign])
AM_MAINTAINER_MODE
AC_PROG_INSTALL
AC_PROG_CC
AC_PROG_CXX
AC_PROG_SED

AC_PATH_PROG(SDL2CONFIG, sdl2-config, no)
if test "x$SDL2CONFIG" = "xno" ; then
	AC_MSG_ERROR([sdl2-config not found. Install sdl2.])
fi

AC_SUBST([SDL2_CFLAGS],[$( sdl2-config --cflags )])
AC_SUBST([SDL2_LDFLAGS], [$( sdl2-config --libs )])

AC_SUBST([MOLTENVK_LDFLAGS], [-lMoltenVK])

AC_PATH_PROG(BREW, brew, no)
AC_SUBST([EXTRA1_LDFLAGS],[])
AC_SUBST([EXTRA1_CFLAGS],[])
if test "x$BREW" != "xno" ; then
	AC_SUBST([EXTRA1_LDFLAGS],[-L$( brew --prefix )/lib])
	AC_SUBST([EXTRA1_CFLAGS],[-I$( brew --prefix )/include])
fi

AC_SUBST([EXTRA2_LDFLAGS],[])

case "$target" in
	*-*-darwin* )
		target_darwin=yes
		AC_SUBST([EXTRA2_LDFLAGS],["-framework Cocoa -framework IOKit -framework CoreVideo"])
	;;
esac


AC_CONFIG_FILES([
	Makefile
	src/Makefile
])
AC_CONFIG_COMMANDS([default],[[ echo timestamp > stamp-h ]],[[]])
AC_OUTPUT

cat <<EOF

Configuration summary
---------------------

   * Installation prefix: ${prefix}

EOF
