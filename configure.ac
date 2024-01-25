AC_INIT([csharpify], [1.0])
AC_CONFIG_SRCDIR([README.md])
AC_CANONICAL_TARGET
AM_INIT_AUTOMAKE([foreign subdir-objects])
AM_MAINTAINER_MODE

AC_CONFIG_MACRO_DIRS([m4])

AC_PROG_CXX([clang++ g++ c++])
FILTER_VALUES(CXX, $CXX, -std=c++11 -std=gnu++11)

AX_CXX_COMPILE_STDCXX(17, , mandatory)
AC_PROG_CC

AC_PROG_INSTALL
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

AC_PATH_PROG(DOTNET, dotnet, no)
if test "x$DOTNET" = "xno" ; then
	AC_MSG_ERROR([dotnet not found. Install dotnet.])
fi
AX_PROG_DOTNET([AC_MSG_CHECKING([success])],[AC_MSG_ERROR([dotnet not found. Install dotnet.])])

AC_SUBST([RM], ["rm -f"])
AC_SUBST([RM_R], ["rm -rf"])
AC_SUBST([CP], [cp])


AC_SUBST([INCLUDE_MAC], [1])
#AC_SUBST([INCLUDE_WIN], [1])
#AC_SUBST([INCLUDE_LINUX], [1])
#AC_SUBST([INCLUDE_ANDROID], [1])

AM_CONDITIONAL([CORECLR_RUNTIME], [true])


case "$target" in
	*-*-darwin* )
		target_darwin=yes
		AC_SUBST([EXTRA2_LDFLAGS],["-framework Cocoa -framework IOKit -framework CoreVideo"])
	;;
esac


AC_CONFIG_FILES([
	Makefile
	package-download/Makefile
	src/Makefile
	src/managed/Makefile
])
AC_CONFIG_COMMANDS([default],[[ echo timestamp > stamp-h ]],[[]])
AC_OUTPUT

cat <<EOF

Configuration summary
---------------------

   * Installation prefix: ${prefix}

EOF
