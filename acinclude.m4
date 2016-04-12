# Custom m4 macros for Open Source Tripwire 2.4.3
# a non-symlinky variant of AC_PROG_LN_S, via: http://www.opensource.apple.com/source/zsh/zsh-34/zsh/aclocal.m4
#
AC_DEFUN([AC_PROG_LN],
[AC_MSG_CHECKING(whether ln works)
AC_CACHE_VAL(ac_cv_prog_LN,
[rm -f conftestdata conftestlink
echo > conftestdata
if ln conftestdata conftestlink 2>/dev/null
then
  rm -f conftestdata conftestlink
  ac_cv_prog_LN="ln"
else
  rm -f conftestdata
  ac_cv_prog_LN="cp"
fi])dnl
LN="$ac_cv_prog_LN"
if test "$ac_cv_prog_LN" = "ln"; then
  AC_MSG_RESULT(yes)
else
  AC_MSG_RESULT(no)
fi
AC_SUBST(LN)dnl
])