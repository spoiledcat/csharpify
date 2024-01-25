# ===============================================================================
#  https://www.gnu.org/software/autoconf-archive/ax_prog_dotnetcore_version.html
# ===============================================================================
#
# SYNOPSIS
#
#   AX_PROG_DOTNET([ACTION-IF-TRUE],[ACTION-IF-FALSE])
#   AX_PROG_DOTNET_VERSION([VERSION],[ACTION-IF-TRUE],[ACTION-IF-FALSE])
#
# DESCRIPTION
#
#   Makes sure that .NET Core supports the version indicated. If true the
#   shell commands in ACTION-IF-TRUE are executed. If not the shell commands
#   in ACTION-IF-FALSE are run. The $dotnetcore_version variable will be
#   filled with the detected version.
#
#   AX_PROG_DOTNET will check if a version of dotnet is usable - use this
#   if you have a global.json file that sets the version you want.
#
#   AX_PROG_DOTNET_VERSION will check the given version(s) of dotnet
#
#   This macro uses the $DOTNET variable to perform the check. If
#   $DOTNET is not set prior to calling this macro, the macro will fail.
#
#
#   Example:
#
#     AC_PATH_PROG([DOTNET],[dotnet])
#     AC_PROG_DOTNET([ ... ],[ ... ])
#     
#     or
#     
#     AC_PROG_DOTNET_VERSION([1.0.2],[ ... ],[ ... ])
#
#   Searches for .NET Core, then checks if at least version 1.0.2 is
#   present.
#
# LICENSE
#
#   Copyright (c) 2024 Andreia Gaita <shana@spoiledcat.net>
#   Copyright (c) 2016 Jens Geyer <jensg@apache.org>
#
#   Copying and distribution of this file, with or without modification, are
#   permitted in any medium without royalty provided the copyright notice
#   and this notice are preserved. This file is offered as-is, without any
#   warranty.

#serial 2

AC_DEFUN([AX_PROG_DOTNET],[
    AC_REQUIRE([AC_PROG_SED])
    AC_REQUIRE([AC_PATH_PROG(HEAD, head)])

    AS_IF([test -n "$DOTNET"],[
        AC_MSG_CHECKING([for .NET SDK])
        sdk_version=`$DOTNET --version 2>&1 | $HEAD -n1 | $SED -e 's/The command could not be loaded.*/failed/'`
        AC_MSG_RESULT($sdk_version)

        dotnet_version=`$SED -e 's/\(@<:@0-9@:>@*\.@<:@0-9@:>@*\)\.@<:@0-9@:>@*/\1/' <<< $sdk_version`.`$SED -e 's/.*\(@<:@0-9@:>@\)/\1/' <<< $sdk_version`

        AC_SUBST([DOTNETSDK_VERSION],[$sdk_version])
        AC_SUBST([DOTNET_VERSION],[$dotnet_version])
        if test x$dotnetcore_version = xfailed; then
            $2
        else
            AC_MSG_CHECKING([for .NET Core])
            AC_MSG_RESULT($dotnet_version)
            $1
        fi
    ],[
        AC_MSG_WARN([could not find .NET Core])
        $3
    ])
])

AC_DEFUN([AX_PROG_DOTNET_VERSION],[
    AC_REQUIRE([AC_PROG_SED])

    AS_IF([test -n "$DOTNET"],[
        ax_dotnetcore_version="$1"

        AC_MSG_CHECKING([for .NET Core version])
        dotnetcore_version=`$DOTNET --version 2>&1 | $SED -e 's/\(@<:@0-9@:>@*\.@<:@0-9@:>@*\.@<:@0-9@:>@*\)\(.*\)/\1/'`
        AC_MSG_RESULT($dotnetcore_version)

	    AC_SUBST([DOTNET_VERSION],[$dotnetcore_version])

        AX_COMPARE_VERSION([$ax_dotnetcore_version],[le],[$dotnetcore_version],[
	    :
            $2
        ],[
	    :
            $3
        ])
    ],[
        AC_MSG_WARN([could not find .NET Core])
        $3
    ])
])
