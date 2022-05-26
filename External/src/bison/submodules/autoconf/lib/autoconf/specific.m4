# This file is part of Autoconf.			-*- Autoconf -*-
# Macros that test for specific, unclassified, features.
#
# Copyright (C) 1992-1996, 1998-2017, 2020 Free Software Foundation,
# Inc.

# This file is part of Autoconf.  This program is free
# software; you can redistribute it and/or modify it under the
# terms of the GNU General Public License as published by the
# Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# Under Section 7 of GPL version 3, you are granted additional
# permissions described in the Autoconf Configure Script Exception,
# version 3.0, as published by the Free Software Foundation.
#
# You should have received a copy of the GNU General Public License
# and a copy of the Autoconf Configure Script Exception along with
# this program; see the files COPYINGv3 and COPYING.EXCEPTION
# respectively.  If not, see <https://www.gnu.org/licenses/>.

# Written by David MacKenzie, with help from
# Franc,ois Pinard, Karl Berry, Richard Pixley, Ian Lance Taylor,
# Roland McGrath, Noah Friedman, david d zuhn, and many others.


## ------------------------- ##
## Checks for declarations.  ##
## ------------------------- ##


# AC_DECL_SYS_SIGLIST
# -------------------
AN_IDENTIFIER([sys_siglist],     [AC_CHECK_DECLS([sys_siglist])])
AU_DEFUN([AC_DECL_SYS_SIGLIST],
[AC_CHECK_DECLS([sys_siglist],,,
[#include <signal.h>
/* NetBSD declares sys_siglist in unistd.h.  */
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif
])
])# AC_DECL_SYS_SIGLIST




## -------------------------------------- ##
## Checks for operating system services.  ##
## -------------------------------------- ##


# AC_SYS_INTERPRETER
# ------------------
AC_DEFUN([AC_SYS_INTERPRETER],
[AC_CACHE_CHECK(whether @%:@! works in shell scripts, ac_cv_sys_interpreter,
[echo '#! /bin/cat
exit 69
' >conftest
chmod u+x conftest
(SHELL=/bin/sh; export SHELL; ./conftest >/dev/null 2>&1)
if test $? -ne 69; then
   ac_cv_sys_interpreter=yes
else
   ac_cv_sys_interpreter=no
fi
rm -f conftest])
interpval=$ac_cv_sys_interpreter
])


AU_DEFUN([AC_HAVE_POUNDBANG],
[AC_SYS_INTERPRETER],
[Remove this warning when you adjust your code to use
`AC_SYS_INTERPRETER'.])


AU_DEFUN([AC_ARG_ARRAY], [],
[$0 is no longer implemented: don't do unportable things
with arguments. Remove this warning when you adjust your code.])


# _AC_SYS_LARGEFILE_TEST_INCLUDES
# -------------------------------
m4_define([_AC_SYS_LARGEFILE_TEST_INCLUDES],
[@%:@include <sys/types.h>
 /* Check that off_t can represent 2**63 - 1 correctly.
    We can't simply define LARGE_OFF_T to be 9223372036854775807,
    since some C++ compilers masquerading as C compilers
    incorrectly reject 9223372036854775807.  */
@%:@define LARGE_OFF_T (((off_t) 1 << 31 << 31) - 1 + ((off_t) 1 << 31 << 31))
  int off_t_is_large[[(LARGE_OFF_T % 2147483629 == 721
		       && LARGE_OFF_T % 2147483647 == 1)
		      ? 1 : -1]];[]dnl
])


# _AC_SYS_LARGEFILE_MACRO_VALUE(C-MACRO, VALUE,
#				CACHE-VAR,
#				DESCRIPTION,
#				PROLOGUE, [FUNCTION-BODY])
# --------------------------------------------------------
m4_define([_AC_SYS_LARGEFILE_MACRO_VALUE],
[AC_CACHE_CHECK([for $1 value needed for large files], [$3],
[while :; do
  m4_ifval([$6], [AC_LINK_IFELSE], [AC_COMPILE_IFELSE])(
    [AC_LANG_PROGRAM([$5], [$6])],
    [$3=no; break])
  m4_ifval([$6], [AC_LINK_IFELSE], [AC_COMPILE_IFELSE])(
    [AC_LANG_PROGRAM([@%:@define $1 $2
$5], [$6])],
    [$3=$2; break])
  $3=unknown
  break
done])
case $$3 in #(
  no | unknown) ;;
  *) AC_DEFINE_UNQUOTED([$1], [$$3], [$4]);;
esac
rm -rf conftest*[]dnl
])# _AC_SYS_LARGEFILE_MACRO_VALUE


# AC_SYS_LARGEFILE
# ----------------
# By default, many hosts won't let programs access large files;
# one must use special compiler options to get large-file access to work.
# For more details about this brain damage please see:
# http://www.unix.org/version2/whatsnew/lfs20mar.html
# Additionally, on Linux file systems with 64-bit inodes a file that happens
# to have a 64-bit inode number cannot be accessed by 32-bit applications on
# Linux x86/x86_64.  This can occur with file systems such as XFS and NFS.
AC_DEFUN([AC_SYS_LARGEFILE],
[AC_ARG_ENABLE(largefile,
	       [  --disable-largefile     omit support for large files])
if test "$enable_largefile" != no; then

  AC_CACHE_CHECK([for special C compiler options needed for large files],
    ac_cv_sys_largefile_CC,
    [ac_cv_sys_largefile_CC=no
     if test "$GCC" != yes; then
       ac_save_CC=$CC
       while :; do
	 # IRIX 6.2 and later do not support large files by default,
	 # so use the C compiler's -n32 option if that helps.
	 AC_LANG_CONFTEST([AC_LANG_PROGRAM([_AC_SYS_LARGEFILE_TEST_INCLUDES])])
	 AC_COMPILE_IFELSE([], [break])
	 CC="$CC -n32"
	 AC_COMPILE_IFELSE([], [ac_cv_sys_largefile_CC=' -n32'; break])
	 break
       done
       CC=$ac_save_CC
       rm -f conftest.$ac_ext
    fi])
  if test "$ac_cv_sys_largefile_CC" != no; then
    CC=$CC$ac_cv_sys_largefile_CC
  fi

  _AC_SYS_LARGEFILE_MACRO_VALUE(_FILE_OFFSET_BITS, 64,
    ac_cv_sys_file_offset_bits,
    [Number of bits in a file offset, on hosts where this is settable.],
    [_AC_SYS_LARGEFILE_TEST_INCLUDES])
  if test $ac_cv_sys_file_offset_bits = unknown; then
    _AC_SYS_LARGEFILE_MACRO_VALUE(_LARGE_FILES, 1,
      ac_cv_sys_large_files,
      [Define for large files, on AIX-style hosts.],
      [_AC_SYS_LARGEFILE_TEST_INCLUDES])
  fi
fi
])# AC_SYS_LARGEFILE


# AC_SYS_LONG_FILE_NAMES
# ----------------------
# Security: use a temporary directory as the most portable way of
# creating files in /tmp securely.  Removing them leaves a race
# condition, set -C is not portably guaranteed to use O_EXCL, so still
# leaves a race, and not all systems have the `mktemp' utility.  We
# still test for existence first in case of broken systems where the
# mkdir succeeds even when the directory exists.  Broken systems may
# retain a race, but they probably have other security problems
# anyway; this should be secure on well-behaved systems.  In any case,
# use of `mktemp' is probably inappropriate here since it would fail in
# attempting to create different file names differing after the 14th
# character on file systems without long file names.
AC_DEFUN([AC_SYS_LONG_FILE_NAMES],
[AC_CACHE_CHECK(for long file names, ac_cv_sys_long_file_names,
[ac_cv_sys_long_file_names=yes
# Test for long file names in all the places we know might matter:
#      .		the current directory, where building will happen
#      $prefix/lib	where we will be installing things
#      $exec_prefix/lib	likewise
#      $TMPDIR		if set, where it might want to write temporary files
#      /tmp		where it might want to write temporary files
#      /var/tmp		likewise
#      /usr/tmp		likewise
for ac_dir in . "$TMPDIR" /tmp /var/tmp /usr/tmp "$prefix/lib" "$exec_prefix/lib"; do
  # Skip $TMPDIR if it is empty or bogus, and skip $exec_prefix/lib
  # in the usual case where exec_prefix is '${prefix}'.
  case $ac_dir in #(
    . | /* | ?:[[\\/]]*) ;; #(
    *) continue;;
  esac
  test -w "$ac_dir/." || continue # It is less confusing to not echo anything here.
  ac_xdir=$ac_dir/cf$$
  (umask 077 && mkdir "$ac_xdir" 2>/dev/null) || continue
  ac_tf1=$ac_xdir/conftest9012345
  ac_tf2=$ac_xdir/conftest9012346
  touch "$ac_tf1" 2>/dev/null && test -f "$ac_tf1" && test ! -f "$ac_tf2" ||
    ac_cv_sys_long_file_names=no
  rm -f -r "$ac_xdir" 2>/dev/null
  test $ac_cv_sys_long_file_names = no && break
done])
if test $ac_cv_sys_long_file_names = yes; then
  AC_DEFINE(HAVE_LONG_FILE_NAMES, 1,
	    [Define to 1 if you support file names longer than 14 characters.])
fi
])


# AC_SYS_RESTARTABLE_SYSCALLS
# ---------------------------
# If the system automatically restarts a system call that is
# interrupted by a signal, define `HAVE_RESTARTABLE_SYSCALLS'.
AC_DEFUN([AC_SYS_RESTARTABLE_SYSCALLS],
[m4_warn([obsolete],
[$0: AC_SYS_RESTARTABLE_SYSCALLS is useful only when supporting very
old systems that lack `sigaction' and `SA_RESTART'.  Don't bother with
this macro unless you need to support very old systems like 4.2BSD and
SVR3.])dnl
AC_REQUIRE([AC_HEADER_SYS_WAIT])dnl
AC_CACHE_CHECK(for restartable system calls, ac_cv_sys_restartable_syscalls,
[AC_RUN_IFELSE([AC_LANG_SOURCE(
[/* Exit 0 (true) if wait returns something other than -1,
   i.e. the pid of the child, which means that wait was restarted
   after getting the signal.  */

AC_INCLUDES_DEFAULT
#include <signal.h>
#ifdef HAVE_SYS_WAIT_H
# include <sys/wait.h>
#endif

/* Some platforms explicitly require an extern "C" signal handler
   when using C++. */
#ifdef __cplusplus
extern "C" void ucatch (int dummy) { }
#else
void ucatch (dummy) int dummy; { }
#endif

int
main (void)
{
  int i = fork (), status;

  if (i == 0)
    {
      sleep (3);
      kill (getppid (), SIGINT);
      sleep (3);
      return 0;
    }

  signal (SIGINT, ucatch);

  status = wait (&i);
  if (status == -1)
    wait (&i);

  return status == -1;
}])],
	       [ac_cv_sys_restartable_syscalls=yes],
	       [ac_cv_sys_restartable_syscalls=no])])
if test $ac_cv_sys_restartable_syscalls = yes; then
  AC_DEFINE(HAVE_RESTARTABLE_SYSCALLS, 1,
	    [Define to 1 if system calls automatically restart after
	     interruption by a signal.])
fi
])# AC_SYS_RESTARTABLE_SYSCALLS


# AC_SYS_POSIX_TERMIOS
# --------------------
AC_DEFUN([AC_SYS_POSIX_TERMIOS],
[AC_CACHE_CHECK([POSIX termios], ac_cv_sys_posix_termios,
[AC_LINK_IFELSE([AC_LANG_PROGRAM([[#include <sys/types.h>
#include <unistd.h>
#include <termios.h>
]],
	     [/* SunOS 4.0.3 has termios.h but not the library calls.  */
   tcgetattr(0, 0);])],
	     ac_cv_sys_posix_termios=yes,
	     ac_cv_sys_posix_termios=no)])
])# AC_SYS_POSIX_TERMIOS




## ------------------------------------ ##
## Checks for not-quite-Unix variants.  ##
## ------------------------------------ ##


# AC_GNU_SOURCE
# -------------
AU_DEFUN([AC_GNU_SOURCE], [AC_USE_SYSTEM_EXTENSIONS])


# AC_CYGWIN
# ---------
# Check for Cygwin.  This is a way to set the right value for
# EXEEXT.
AU_DEFUN([AC_CYGWIN],
[AC_CANONICAL_HOST
case $host_os in
  *cygwin* ) CYGWIN=yes;;
	 * ) CYGWIN=no;;
esac
], [$0 is obsolete: use AC_CANONICAL_HOST and check if $host_os
matches *cygwin*])# AC_CYGWIN


# AC_EMXOS2
# ---------
# Check for EMX on OS/2.  This is another way to set the right value
# for EXEEXT.
AU_DEFUN([AC_EMXOS2],
[AC_CANONICAL_HOST
case $host_os in
  *emx* ) EMXOS2=yes;;
      * ) EMXOS2=no;;
esac
], [$0 is obsolete: use AC_CANONICAL_HOST and check if $host_os
matches *emx*])# AC_EMXOS2


# AC_MINGW32
# ----------
# Check for mingw32.  This is another way to set the right value for
# EXEEXT.
AU_DEFUN([AC_MINGW32],
[AC_CANONICAL_HOST
case $host_os in
  *mingw32* ) MINGW32=yes;;
	  * ) MINGW32=no;;
esac
], [$0 is obsolete: use AC_CANONICAL_HOST and check if $host_os
matches *mingw32*])# AC_MINGW32


# AC_USE_SYSTEM_EXTENSIONS
# ------------------------
# Enable extensions on systems that normally disable them,
# typically due to standards-conformance issues.
# We unconditionally define as many of the known feature-enabling
# as possible, reserving conditional behavior for macros that are
# known to cause problems on some platforms (such as __EXTENSIONS__).
AC_DEFUN_ONCE([AC_USE_SYSTEM_EXTENSIONS],
[AC_BEFORE([$0], [AC_PREPROC_IFELSE])dnl
AC_BEFORE([$0], [AC_COMPILE_IFELSE])dnl
AC_BEFORE([$0], [AC_LINK_IFELSE])dnl
AC_BEFORE([$0], [AC_RUN_IFELSE])dnl
AC_BEFORE([$0], [AC_CHECK_INCLUDES_DEFAULT])dnl
dnl #undef in AH_VERBATIM gets replaced with #define by AC_DEFINE.
dnl Use a different key than __EXTENSIONS__, as that name broke existing
dnl configure.ac when using autoheader 2.62.
dnl The macros below are in alphabetical order ignoring leading _ or __
dnl prefixes.
AH_VERBATIM([USE_SYSTEM_EXTENSIONS],
[/* Enable extensions on AIX 3, Interix.  */
#ifndef _ALL_SOURCE
# undef _ALL_SOURCE
#endif
/* Enable general extensions on macOS.  */
#ifndef _DARWIN_C_SOURCE
# undef _DARWIN_C_SOURCE
#endif
/* Enable general extensions on Solaris.  */
#ifndef __EXTENSIONS__
# undef __EXTENSIONS__
#endif
/* Enable GNU extensions on systems that have them.  */
#ifndef _GNU_SOURCE
# undef _GNU_SOURCE
#endif
/* Enable X/Open compliant socket functions that do not require linking
   with -lxnet on HP-UX 11.11.  */
#ifndef _HPUX_ALT_XOPEN_SOCKET_API
# undef _HPUX_ALT_XOPEN_SOCKET_API
#endif
/* Identify the host operating system as Minix.
   This macro does not affect the system headers' behavior.
   A future release of Autoconf may stop defining this macro.  */
#ifndef _MINIX
# undef _MINIX
#endif
/* Enable general extensions on NetBSD.
   Enable NetBSD compatibility extensions on Minix.  */
#ifndef _NETBSD_SOURCE
# undef _NETBSD_SOURCE
#endif
/* Enable OpenBSD compatibility extensions on NetBSD.
   Oddly enough, this does nothing on OpenBSD.  */
#ifndef _OPENBSD_SOURCE
# undef _OPENBSD_SOURCE
#endif
/* Define to 1 if needed for POSIX-compatible behavior.  */
#ifndef _POSIX_SOURCE
# undef _POSIX_SOURCE
#endif
/* Define to 2 if needed for POSIX-compatible behavior.  */
#ifndef _POSIX_1_SOURCE
# undef _POSIX_1_SOURCE
#endif
/* Enable POSIX-compatible threading on Solaris.  */
#ifndef _POSIX_PTHREAD_SEMANTICS
# undef _POSIX_PTHREAD_SEMANTICS
#endif
/* Enable extensions specified by ISO/IEC TS 18661-5:2014.  */
#ifndef __STDC_WANT_IEC_60559_ATTRIBS_EXT__
# undef __STDC_WANT_IEC_60559_ATTRIBS_EXT__
#endif
/* Enable extensions specified by ISO/IEC TS 18661-1:2014.  */
#ifndef __STDC_WANT_IEC_60559_BFP_EXT__
# undef __STDC_WANT_IEC_60559_BFP_EXT__
#endif
/* Enable extensions specified by ISO/IEC TS 18661-2:2015.  */
#ifndef __STDC_WANT_IEC_60559_DFP_EXT__
# undef __STDC_WANT_IEC_60559_DFP_EXT__
#endif
/* Enable extensions specified by ISO/IEC TS 18661-4:2015.  */
#ifndef __STDC_WANT_IEC_60559_FUNCS_EXT__
# undef __STDC_WANT_IEC_60559_FUNCS_EXT__
#endif
/* Enable extensions specified by ISO/IEC TS 18661-3:2015.  */
#ifndef __STDC_WANT_IEC_60559_TYPES_EXT__
# undef __STDC_WANT_IEC_60559_TYPES_EXT__
#endif
/* Enable extensions specified by ISO/IEC TR 24731-2:2010.  */
#ifndef __STDC_WANT_LIB_EXT2__
# undef __STDC_WANT_LIB_EXT2__
#endif
/* Enable extensions specified by ISO/IEC 24747:2009.  */
#ifndef __STDC_WANT_MATH_SPEC_FUNCS__
# undef __STDC_WANT_MATH_SPEC_FUNCS__
#endif
/* Enable extensions on HP NonStop.  */
#ifndef _TANDEM_SOURCE
# undef _TANDEM_SOURCE
#endif
/* Enable X/Open extensions.  Define to 500 only if necessary
   to make mbstate_t available.  */
#ifndef _XOPEN_SOURCE
# undef _XOPEN_SOURCE
#endif
])dnl

  AC_REQUIRE([AC_CHECK_INCLUDES_DEFAULT])dnl
  _AC_CHECK_HEADER_ONCE([wchar.h])
  _AC_CHECK_HEADER_ONCE([minix/config.h])

dnl Defining __EXTENSIONS__ may break the system headers on some systems.
dnl (FIXME: Which ones?)
  AC_CACHE_CHECK([whether it is safe to define __EXTENSIONS__],
    [ac_cv_safe_to_define___extensions__],
    [AC_COMPILE_IFELSE(
       [AC_LANG_PROGRAM([[
#         define __EXTENSIONS__ 1
          ]AC_INCLUDES_DEFAULT])],
       [ac_cv_safe_to_define___extensions__=yes],
       [ac_cv_safe_to_define___extensions__=no])])

dnl HP-UX 11.11 defines mbstate_t only if _XOPEN_SOURCE is defined to
dnl 500, regardless of whether compiling with -Ae or -D_HPUX_SOURCE=1.
dnl But defining _XOPEN_SOURCE may turn *off* extensions on platforms
dnl not covered by turn-on-extensions macros (notably Dragonfly, Free,
dnl and OpenBSD, which don't have any equivalent of _NETBSD_SOURCE) so
dnl it should only be defined when necessary.
  AC_CACHE_CHECK([whether _XOPEN_SOURCE should be defined],
    [ac_cv_should_define__xopen_source],
    [ac_cv_should_define__xopen_source=no
    AS_IF([test $ac_cv_header_wchar_h = yes],
      [AC_COMPILE_IFELSE(
        [AC_LANG_PROGRAM([[
          #include <wchar.h>
          mbstate_t x;]])],
        [],
        [AC_COMPILE_IFELSE(
          [AC_LANG_PROGRAM([[
            #define _XOPEN_SOURCE 500
            #include <wchar.h>
            mbstate_t x;]])],
          [ac_cv_should_define__xopen_source=yes])])])])

  AC_DEFINE([_ALL_SOURCE])
  AC_DEFINE([_DARWIN_C_SOURCE])
  AC_DEFINE([_GNU_SOURCE])
  AC_DEFINE([_HPUX_ALT_XOPEN_SOCKET_API])
  AC_DEFINE([_NETBSD_SOURCE])
  AC_DEFINE([_OPENBSD_SOURCE])
  AC_DEFINE([_POSIX_PTHREAD_SEMANTICS])
  AC_DEFINE([__STDC_WANT_IEC_60559_ATTRIBS_EXT__])
  AC_DEFINE([__STDC_WANT_IEC_60559_BFP_EXT__])
  AC_DEFINE([__STDC_WANT_IEC_60559_DFP_EXT__])
  AC_DEFINE([__STDC_WANT_IEC_60559_FUNCS_EXT__])
  AC_DEFINE([__STDC_WANT_IEC_60559_TYPES_EXT__])
  AC_DEFINE([__STDC_WANT_LIB_EXT2__])
  AC_DEFINE([__STDC_WANT_MATH_SPEC_FUNCS__])
  AC_DEFINE([_TANDEM_SOURCE])
  AS_IF([test $ac_cv_header_minix_config_h = yes],
    [MINIX=yes
    AC_DEFINE([_MINIX])
    AC_DEFINE([_POSIX_SOURCE])
    AC_DEFINE([_POSIX_1_SOURCE], [2])],
    [MINIX=])
  AS_IF([test $ac_cv_safe_to_define___extensions__ = yes],
    [AC_DEFINE([__EXTENSIONS__])])
  AS_IF([test $ac_cv_should_define__xopen_source = yes],
    [AC_DEFINE([_XOPEN_SOURCE], [500])])
])# AC_USE_SYSTEM_EXTENSIONS



## -------------------------- ##
## Checks for UNIX variants.  ##
## -------------------------- ##


# These are kludges which should be replaced by a single POSIX check.
# They aren't cached, to discourage their use.

# AC_AIX
# ------
AU_DEFUN([AC_AIX], [AC_USE_SYSTEM_EXTENSIONS])


# AC_MINIX
# --------
AU_DEFUN([AC_MINIX], [AC_USE_SYSTEM_EXTENSIONS])


# AC_ISC_POSIX
# ------------
AU_DEFUN([AC_ISC_POSIX], [AC_SEARCH_LIBS([strerror], [cposix])])


# AC_XENIX_DIR
# ------------
AU_DEFUN([AC_XENIX_DIR],
[AC_MSG_CHECKING([for Xenix])
AC_EGREP_CPP([yes],
[#if defined M_XENIX && ! defined M_UNIX
  yes
@%:@endif],
	     [AC_MSG_RESULT([yes]); XENIX=yes],
	     [AC_MSG_RESULT([no]); XENIX=])

AC_HEADER_DIRENT[]dnl
],
[You shouldn't need to depend upon XENIX.  Remove the
`AC_MSG_CHECKING', `AC_EGREP_CPP', and this warning if this part
of the test is useless.])


# AC_DYNIX_SEQ
# ------------
AU_DEFUN([AC_DYNIX_SEQ], [AC_FUNC_GETMNTENT])


# AC_IRIX_SUN
# -----------
AU_DEFUN([AC_IRIX_SUN],
[AC_FUNC_GETMNTENT
AC_CHECK_LIB([sun], [getpwnam])])


# AC_SCO_INTL
# -----------
AU_DEFUN([AC_SCO_INTL], [AC_FUNC_STRFTIME])
