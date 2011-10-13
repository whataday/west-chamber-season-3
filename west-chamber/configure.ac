
AC_INIT([west-chamber], [0.0.2])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])
AC_PROG_INSTALL
AM_INIT_AUTOMAKE([1.10b -Wall foreign subdir-objects])
AC_PROG_CC
AM_PROG_CC_C_O
AC_DISABLE_STATIC
AC_PROG_LIBTOOL

AC_ARG_WITH([kbuild],
	AS_HELP_STRING([--with-kbuild=PATH],
	[Path to kernel build directory [[/lib/modules/CURRENT/build]]]),
	[kbuilddir="$withval"],
	[kbuilddir="/lib/modules/$(uname -r)/build"])
#
# check for --without-kbuild
#
if [[ "$kbuilddir" == no ]]; then
	kbuilddir="";
fi

AC_CHECK_HEADERS([linux/netfilter/x_tables.h], [],
	[AC_MSG_ERROR([You need to have linux/netfilter/x_tables.h, see INSTALL file for details])])
PKG_CHECK_MODULES([libxtables], [xtables >= 1.4.3])
xtlibdir="$(pkg-config --variable=xtlibdir xtables)"
PKG_CHECK_MODULES([libmnl], [libmnl >= 1], [:], [:])

AC_ARG_WITH([xtlibdir],
	AS_HELP_STRING([--with-xtlibdir=PATH],
	[Path where to install Xtables extensions [[autodetect]]]]),
	[xtlibdir="$withval"])
AC_MSG_CHECKING([Xtables module directory])
AC_MSG_RESULT([$xtlibdir])

regular_CPPFLAGS="-D_LARGEFILE_SOURCE=1 -D_LARGE_FILES -D_FILE_OFFSET_BITS=64 \
	-D_REENTRANT -I\${XA_TOPSRCDIR}/include"
regular_CFLAGS="-Wall -Waggregate-return -Wmissing-declarations \
	-Wmissing-prototypes -Wredundant-decls -Wshadow -Wstrict-prototypes \
	-Winline -pipe";

if test -n "$kbuilddir"; then
	AC_MSG_CHECKING([kernel version that we will build against])
	krel="$(make -sC "$kbuilddir" M=$PWD kernelrelease)";
	kmajor="${krel%%[[^0-9]]*}";
	kmajor="$(($kmajor+0))";
	krel="${krel:${#kmajor}}";
	krel="${krel#.}";
	kminor="${krel%%[[^0-9]]*}";
	kminor="$(($kminor+0))";
	krel="${krel:${#kminor}}";
	krel="${krel#.}";
	kmicro="${krel%%[[^0-9]]*}";
	kmicro="$(($kmicro+0))";
	krel="${krel:${#kmicro}}";
	krel="${krel#.}";
	kstable="${krel%%[[^0-9]]*}";
	kstable="$(($kstable+0))";
	if test -z "$kmajor" -o -z "$kminor" -o -z "$kmicro"; then
		echo "WARNING: Version detection did not succeed. Continue at own luck.";
	else
		echo "$kmajor.$kminor.$kmicro.$kstable in $kbuilddir";
		if test "$kmajor" -gt 3 -o "$kmajor" -eq 3 -a "$kminor" -gt 1; then
			echo "WARNING: You are trying a newer kernel. Results may vary. :-)";
		elif test "$kmajor" -eq 3; then
			:;
		elif test "$kmajor" -eq 2 -a "$kminor" -eq 6 -a "$kmicro" -ge 29; then
			:;
		else
			echo "WARNING: That kernel version is not supported.";
		fi;
	fi;
fi;

AC_SUBST([regular_CPPFLAGS])
AC_SUBST([regular_CFLAGS])
AC_SUBST([kbuilddir])
AC_SUBST([xtlibdir])
AC_CONFIG_FILES([Makefile Makefile.iptrules Makefile.mans
	extensions/Makefile])
AC_OUTPUT
